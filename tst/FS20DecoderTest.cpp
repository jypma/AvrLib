#include <gtest/gtest.h>
#include <type_traits>
#include "FS20Decoder.hpp"

struct FS20MockPulseCounter {
    typedef uint8_t count_t;
    struct PulseEvent {
        PulseType type;
        uint8_t length;
        PulseType getType() const { return type; }
        uint8_t getLength() const { return length; }
    };
    struct Timer {
        static constexpr uint8_t prescalerPower2 = 6;
        static constexpr uint16_t microseconds2counts(uint16_t usecs) {
            return (F_CPU >> prescalerPower2) / 1000 * usecs / 1000;
        }
    };
};

template <typename T>
void sendData(T &decoder, const uint8_t *seq, uint16_t length) {
    PulseType type = PulseType::LOW;
    for (uint8_t i = 0; i < length; i++) {
        FS20MockPulseCounter::PulseEvent event = { type, seq[i] };
        decoder.apply(event);
        type = (type == PulseType::LOW) ? PulseType::HIGH : PulseType::LOW;
    }
    FS20MockPulseCounter::PulseEvent event = { PulseType::TIMEOUT, 0 };
    decoder.apply(event);
}

TEST(FS20DecoderTest, fs20_decoder_can_decode_correct_bit_sequence) {
    FS20Decoder<FS20MockPulseCounter> decoder;

    const uint8_t seq1[] = { 115, 85, 107, 93, 101, 95, 100, 97, 103, 97, 99, 96, 97, 101, 96, 102, 94, 108, 89, 106, 92, 103, 142, 154, 93, 106, 95, 102, 94, 104, 139, 160, 137, 153, 93, 105, 141, 155, 141, 154, 92, 106, 140, 155, 141, 154, 143, 153, 142, 154, 141, 156, 140, 155, 140, 157, 139, 155, 93, 105, 91, 106, 91, 106, 93, 104, 93, 104, 92, 107, 90, 106, 92, 106, 92, 104, 92, 107, 91, 105, 91, 106, 92, 105, 92, 105, 92, 105, 92, 105, 93, 105, 91, 106, 93, 104, 92, 106, 92, 104, 142, 155, 92, 104, 92, 105, 92, 106, 91, 105, 92, 106, 141, 155 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    FS20Packet pkt;
    EXPECT_TRUE(decoder.in() >> pkt);
    EXPECT_EQ(27, pkt.houseCodeHi);
    EXPECT_EQ(255, pkt.houseCodeLo);
    EXPECT_EQ(0, pkt.address);
    EXPECT_EQ(0, pkt.command);
}

TEST(FS20DecoderTest, fs20_decoder_ignores_packet_with_parity_error) {
    FS20Decoder<FS20MockPulseCounter> decoder;

    const uint8_t seq1[] = { 115, 85, 107, 93, 101, 95, 100, 97, 103, 97, 99, 96, 97, 101, 96, 102, 94, 108, 89, 106, 92, 103, 142, 154, 93, 106, 95, 102, 94, 104, 139, 160, 137, 153, 93, 105, 141, 155, 141, 154, 92, 106, 100, 100, 141, 154, 143, 153, 142, 154, 141, 156, 140, 155, 140, 157, 139, 155, 93, 105, 91, 106, 91, 106, 93, 104, 93, 104, 92, 107, 90, 106, 92, 106, 92, 104, 92, 107, 91, 105, 91, 106, 92, 105, 92, 105, 92, 105, 92, 105, 93, 105, 91, 106, 93, 104, 92, 106, 92, 104, 142, 155, 92, 104, 92, 105, 92, 106, 91, 105, 92, 106, 141, 155 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    FS20Packet pkt;
    EXPECT_FALSE(decoder.in() >> pkt);
}

TEST(FS20DecoderTest, fs20_decoder_ignores_packet_with_checksum_error_but_correct_parity) {
    FS20Decoder<FS20MockPulseCounter> decoder;

    const uint8_t seq1[] = { 115, 85, 107, 93, 101, 95, 100, 97, 103, 97, 99, 96, 97, 101, 96, 102, 94, 108, 89, 106, 92, 103, 142, 154, 93, 106, 95, 102, 94, 104, 139, 160, 137, 153, 93, 105, 141, 155, 141, 154, 92, 106, 140, 155, 141, 154, 143, 153, 142, 154, 141, 156, 140, 155, 140, 157, 139, 155, 93, 105, 91, 106, 91, 106, 93, 104, 93, 104, 92, 107, 90, 106, 92, 106, 92, 104, 92, 107, 91, 105, 91, 106, 92, 105, 92, 105, 92, 105, 92, 105, 93, 105, 91, 106, 93, 104, 92, 106, 92, 104, 142, 155, 92, 104, 92, 105, 92, 106, 91, 105, 141, 155, 92, 106 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    FS20Packet pkt;
    EXPECT_FALSE(decoder.in() >> pkt);
}
