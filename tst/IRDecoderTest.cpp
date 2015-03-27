#include <gtest/gtest.h>
#include <type_traits>
#include "IRDecoder.hpp"

struct MockPulseCounter {
    struct PulseEvent {
        PulseType type;
        uint16_t length;
        PulseType getType() const { return type; }
        uint16_t getLength() const { return length; }
    };
    struct Timer {
        static constexpr uint8_t prescalerPower2 = 8;
        static constexpr uint16_t microseconds2counts(uint16_t usecs) {
            return (F_CPU >> prescalerPower2) / 1000 * usecs / 1000;
        }
    };
};

void sendData(IRDecoder_NEC<MockPulseCounter> &decoder, const uint16_t *seq, uint8_t length) {
    PulseType type = PulseType::LOW;
    for (uint16_t i = 0; i < length; i++) {
        MockPulseCounter::PulseEvent event = { type, seq[i] };
        decoder.apply(event);
        type = (type == PulseType::LOW) ? PulseType::HIGH : PulseType::LOW;
    }
    MockPulseCounter::PulseEvent event = { PulseType::TIMEOUT, 0 };
    decoder.apply(event);
}

TEST(IRDecoderTest, nec_decoder_can_decode_yamaha_at_prescaler_256) {
    IRDecoder_NEC<MockPulseCounter> decoder;
    const uint16_t seq1[] = { 49926 ,553 ,274 ,27 ,28 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,28 ,29 ,95 ,27 ,98 ,27 ,98 ,27 ,98 ,26 ,28 ,27 ,29 ,29 ,25 ,30 ,95 ,26 ,28 ,27 ,28 ,29 ,25 ,30 ,25 ,29 ,95 ,27 ,98 ,27 ,98 ,26 ,27 ,30 ,25 ,29 ,2542 ,553 ,135 ,26 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    IRCode code;
    decoder.in() >> code;
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq2[] = {49926 ,553 ,274 ,27 ,28 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,28 ,29 ,95 ,27 ,98 ,27 ,98 ,27 ,98 ,26 ,28 ,27 ,29 ,29 ,25 ,30 ,95 ,26 ,28 ,27 ,28 ,29 ,25 ,30 ,25 ,29 ,95 ,27 ,98 ,27 ,98 ,26 ,27 ,30 ,25 ,29 ,2542 ,553 ,135 ,26};
    sendData(decoder, seq2, std::extent<decltype(seq2)>::value);

    decoder.in() >> code;
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq3[] = {26519 ,553 ,274 ,27 ,28 ,26 ,98 ,30 ,25 ,29 ,95 ,26 ,98 ,29 ,96 ,27 ,98 ,26 ,27 ,30 ,95 ,27 ,27 ,29 ,95 ,27 ,28 ,29 ,25 ,30 ,24 ,29 ,25 ,30 ,95 ,27 ,98 ,26 ,98 ,27 ,98 ,27 ,27 ,26 ,29 ,30 ,25 ,29 ,97 ,25 ,28 ,26 ,28 ,30 ,24 ,30 ,24 ,29 ,95 ,26 ,99 ,27 ,97 ,27 ,28 ,27 ,27 ,27 ,2546 ,553 ,133 ,26};
    sendData(decoder, seq3, std::extent<decltype(seq3)>::value);

    decoder.in() >> code;
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq4[] = {32764 ,553 ,274 ,27 ,28 ,26 ,98 ,26 ,28 ,30 ,95 ,26 ,98 ,29 ,95 ,30 ,95 ,30 ,24 ,29 ,95 ,30 ,25 ,29 ,95 ,30 ,25 ,29 ,25 ,30 ,25 ,29 ,25 ,30 ,95 ,30 ,95 ,29 ,95 ,29 ,95 ,30 ,25 ,29 ,26 ,30 ,25 ,29 ,95 ,30 ,25 ,29 ,25 ,30 ,25 ,29 ,25 ,30 ,95 ,30 ,95 ,29 ,95 ,29 ,25 ,29 ,25 ,29 ,2543 ,552 ,135 ,27 ,6003 ,553 ,133 ,29 ,6002 ,553 ,133 ,27};
    sendData(decoder, seq4, std::extent<decltype(seq4)>::value);

    decoder.in() >> code;
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());
    code = IRCode();
    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());
    code = IRCode();
    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq5[] = {6656 ,553 ,274 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,98 ,27 ,98 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,28 ,26 ,98 ,27 ,28 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,98 ,27 ,98 ,26 ,97 ,26 ,98 ,27 ,28 ,26 ,29 ,27 ,28 ,26 ,98 ,27 ,28 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,27 ,26 ,2548 ,551 ,134 ,27 ,6003 ,553 ,133 ,26};
    sendData(decoder, seq5, std::extent<decltype(seq5)>::value);
    decoder.in() >> code;
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());
    code = IRCode();
    decoder.in() >> code;
    EXPECT_EQ(IRType::Repeat, code.getType());
}
