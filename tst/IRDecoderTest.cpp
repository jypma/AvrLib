#include <gtest/gtest.h>
#include <type_traits>
#include "IR/IRDecoder.hpp"

using namespace Serial;

struct MockPulseCounter {
    struct Timer {
        typedef uint16_t value_t;

        static constexpr uint8_t prescalerPower2 = 8;
        template <uint32_t usecs, typename return_t>
        static constexpr return_t microseconds2counts() {
            return (F_CPU >> prescalerPower2) / 1000 * usecs / 1000;
        }
    };
};

template <typename T>
void sendData(T &decoder, const uint16_t *seq, uint16_t length) {
    bool high = true;
    for (uint16_t i = 0; i < length; i++) {
        Pulse event = { high, seq[i] };
        decoder.apply(event);
        high = !high;
    }
    decoder.apply(Pulse::empty());
}

TEST(IRDecoderTest, nec_decoder_can_decode_yamaha_at_prescaler_256) {
    IRDecoder_NEC<MockPulseCounter> decoder;
    const uint16_t seq1[] = { 49926 ,553 ,274 ,27 ,28 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,28 ,29 ,95 ,27 ,98 ,27 ,98 ,27 ,98 ,26 ,28 ,27 ,29 ,29 ,25 ,30 ,95 ,26 ,28 ,27 ,28 ,29 ,25 ,30 ,25 ,29 ,95 ,27 ,98 ,27 ,98 ,26 ,27 ,30 ,25 ,29 ,2542 ,553 ,135 ,26 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    IRCode code;
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq2[] = {49926 ,553 ,274 ,27 ,28 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,28 ,29 ,95 ,27 ,98 ,27 ,98 ,27 ,98 ,26 ,28 ,27 ,29 ,29 ,25 ,30 ,95 ,26 ,28 ,27 ,28 ,29 ,25 ,30 ,25 ,29 ,95 ,27 ,98 ,27 ,98 ,26 ,27 ,30 ,25 ,29 ,2542 ,553 ,135 ,26};
    sendData(decoder, seq2, std::extent<decltype(seq2)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq3[] = {26519 ,553 ,274 ,27 ,28 ,26 ,98 ,30 ,25 ,29 ,95 ,26 ,98 ,29 ,96 ,27 ,98 ,26 ,27 ,30 ,95 ,27 ,27 ,29 ,95 ,27 ,28 ,29 ,25 ,30 ,24 ,29 ,25 ,30 ,95 ,27 ,98 ,26 ,98 ,27 ,98 ,27 ,27 ,26 ,29 ,30 ,25 ,29 ,97 ,25 ,28 ,26 ,28 ,30 ,24 ,30 ,24 ,29 ,95 ,26 ,99 ,27 ,97 ,27 ,28 ,27 ,27 ,27 ,2546 ,553 ,133 ,26};
    sendData(decoder, seq3, std::extent<decltype(seq3)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq4[] = {32764 ,553 ,274 ,27 ,28 ,26 ,98 ,26 ,28 ,30 ,95 ,26 ,98 ,29 ,95 ,30 ,95 ,30 ,24 ,29 ,95 ,30 ,25 ,29 ,95 ,30 ,25 ,29 ,25 ,30 ,25 ,29 ,25 ,30 ,95 ,30 ,95 ,29 ,95 ,29 ,95 ,30 ,25 ,29 ,26 ,30 ,25 ,29 ,95 ,30 ,25 ,29 ,25 ,30 ,25 ,29 ,25 ,30 ,95 ,30 ,95 ,29 ,95 ,29 ,25 ,29 ,25 ,29 ,2543 ,552 ,135 ,27 ,6003 ,553 ,133 ,29 ,6002 ,553 ,133 ,27};
    sendData(decoder, seq4, std::extent<decltype(seq4)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());

    const uint16_t seq5[] = {6656 ,553 ,274 ,27 ,28 ,26 ,98 ,26 ,28 ,27 ,98 ,26 ,98 ,27 ,98 ,27 ,98 ,27 ,27 ,27 ,98 ,27 ,28 ,26 ,98 ,27 ,28 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,98 ,27 ,98 ,26 ,97 ,26 ,98 ,27 ,28 ,26 ,29 ,27 ,28 ,26 ,98 ,27 ,28 ,26 ,28 ,27 ,28 ,26 ,28 ,27 ,98 ,27 ,97 ,26 ,98 ,27 ,28 ,26 ,27 ,26 ,2548 ,551 ,134 ,27 ,6003 ,553 ,133 ,26};
    sendData(decoder, seq5, std::extent<decltype(seq5)>::value);
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(1587667484, code.getCommand());

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Repeat, code.getType());
}

TEST(IRDecoderTest, samsung_can_decode_samsung_remote_at_prescaler_256) {
    IRDecoder_Samsung<MockPulseCounter> decoder;
    IRCode code;

    const uint16_t seq1[] = { 2658  ,276  ,266  ,32  ,94  ,33  ,95  ,35  ,93  ,34  ,24  ,34  ,27  ,33  ,24  ,34  ,25  ,35  ,24  ,34  ,93  ,33  ,94  ,33  ,96  ,33  ,24  ,33  ,24  ,34  ,25  ,34  ,22  ,34  ,24  ,34  ,24  ,35  ,23  ,36  ,25  ,32  ,24  ,34  ,24  ,32  ,95  ,34  ,94  ,34  ,23  ,34  ,93  ,34  ,93  ,35  ,94  ,34  ,93  ,34  ,96  ,33  ,24  ,36  ,23  ,34  ,96  ,32 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772778233, code.getCommand());

    const uint16_t seq2[] = { 20077  ,274  ,268  ,30  ,95  ,32  ,95  ,33  ,96  ,34  ,25  ,30  ,28  ,30  ,27  ,31  ,28  ,31  ,25  ,33  ,94  ,34  ,94  ,32  ,97  ,31  ,27  ,31  ,26  ,32  ,25  ,34  ,24  ,34  ,24  ,34  ,24  ,32  ,25  ,32  ,27  ,31  ,25  ,33  ,25  ,32  ,95  ,33  ,94  ,32  ,27  ,31  ,95  ,35  ,94  ,34  ,94  ,34  ,93  ,33  ,98  ,30  ,26  ,33  ,26  ,34  ,93  ,33 };
    sendData(decoder, seq2, std::extent<decltype(seq2)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772778233, code.getCommand());

    const uint16_t seq3[] = { 42295  ,276  ,266  ,33  ,92  ,36  ,91  ,35  ,93  ,34  ,25  ,33  ,26  ,32  ,24  ,34  ,25  ,33  ,24  ,36  ,92  ,36  ,92  ,33  ,97  ,32  ,25  ,34  ,24  ,35  ,22  ,36  ,23  ,35  ,22  ,35  ,23  ,36  ,22  ,35  ,23  ,34  ,25  ,34  ,24  ,33  ,93  ,35  ,93  ,34  ,24  ,33  ,93  ,35  ,94  ,34  ,93  ,34  ,93  ,35  ,93  ,34  ,23  ,34  ,24  ,34  ,93  ,35  ,2941  ,276  ,266  ,32  ,94  ,35  ,92  ,36  ,92  ,35  ,23  ,35  ,24  ,34  ,24  ,36  ,23  ,35  ,22  ,35  ,93  ,35  ,93  ,35  ,93  ,34  ,24  ,35  ,27  ,32  ,24  ,34  ,24  ,34  ,23  ,34  ,24  ,34  ,23  ,34  ,26  ,34  ,23  ,36  ,21  ,36  ,93  ,32  ,95  ,33  ,25  ,34  ,93  ,33  ,93  ,36  ,93  ,34  ,93  ,34  ,93  ,35  ,23  ,34  ,24  ,35  ,93  ,34  ,2942  ,276  ,266  ,33  ,92  ,35  ,91  ,35  ,93  ,36  ,24  ,35  ,24  ,33  ,24  ,33  ,25  ,36  ,22  ,33  ,93  ,35  ,93  ,36  ,92  ,34  ,24  ,34  ,24  ,35  ,25  ,33  ,25  ,34  ,24  ,35  ,24  ,34  ,23  ,34  ,24  ,34  ,23  ,34  ,24  ,34  ,93  ,35  ,95  ,32  ,23  ,34  ,96  ,32  ,93  ,34  ,93  ,36  ,93  ,34  ,93  ,36  ,23  ,33  ,25  ,35  ,93  ,33 };
    sendData(decoder, seq3, std::extent<decltype(seq3)>::value);

    // apparently, the samsung BN59-00603A doesn't send repeats for this button
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772778233, code.getCommand());
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772778233, code.getCommand());
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772778233, code.getCommand());

    const uint16_t seq4[] = { 60656  ,271  ,271  ,28  ,97  ,29  ,98  ,30  ,98  ,30  ,28  ,28  ,31  ,27  ,30  ,28  ,30  ,28  ,29  ,30  ,99  ,28  ,98  ,30  ,100  ,28  ,29  ,28  ,30  ,29  ,30  ,27  ,31  ,29  ,27  ,29  ,98  ,29  ,99  ,29  ,28  ,29  ,100  ,27  ,29  ,30  ,31  ,29  ,28  ,30  ,28  ,29  ,30  ,30  ,28  ,29  ,99  ,29  ,28  ,29  ,100  ,27  ,100  ,27  ,99  ,30  ,98  ,29  ,2946  ,270  ,271  ,26  ,99  ,30  ,98  ,30  ,98  ,30  ,29  ,27  ,30  ,29  ,29  ,28  ,31  ,28  ,29  ,29  ,99  ,28  ,98  ,29  ,99  ,28  ,29  ,28  ,30  ,29  ,30  ,28  ,31  ,27  ,29  ,29  ,101  ,27  ,99  ,29  ,29  ,30  ,98  ,28  ,31  ,28  ,29  ,29  ,30  ,29  ,29  ,29  ,29  ,29  ,29  ,28  ,102  ,27  ,30  ,27  ,101  ,28  ,99  ,30  ,98  ,29  ,98  ,29  ,2948  ,271  ,271  ,27  ,98  ,29  ,99  ,29  ,99  ,29  ,29  ,30  ,29  ,30  ,27  ,29  ,31  ,26  ,29  ,28  ,101  ,28  ,98  ,29  ,99  ,30  ,29  ,29  ,31  ,28  ,31  ,28  ,28  ,30  ,27  ,29  ,100  ,28  ,99  ,28  ,29  ,29  ,100  ,27  ,30  ,29  ,29  ,28  ,30  ,28  ,31  ,27  ,29  ,31  ,28  ,27  ,99  ,29  ,29  ,28  ,99  ,31  ,99  ,27  ,98  ,29  ,99  ,30  ,2946  ,271  ,271  ,26  ,99  ,31  ,97  ,31  ,98  ,30  ,29  ,28  ,30  ,29  ,29  ,28  ,30  ,28  ,29  ,30  ,99  ,27  ,99  ,30  ,99  ,28  ,29  ,29  ,30  ,30  ,28  ,30  ,29  ,29  ,28  ,29  ,100  ,28  ,99  ,28  ,29  ,31  ,97  ,29  ,31  ,27  ,29  ,29  ,30  ,28  ,30  ,27  ,29  ,31  ,27  ,30  ,99  ,27  ,29  ,28  ,101  ,27  ,99  ,30  ,100  ,27  ,98  ,29  ,2946  ,270  ,272  ,27  ,98  ,29  ,98  ,30  ,98  ,30  ,29  ,28  ,31  ,28  ,29  ,29  ,30  ,28  ,30  ,29  ,99  ,27  ,99  ,30  ,100  ,28  ,30  ,28  ,29  ,29  ,30  ,28  ,31  ,27  ,29  ,28  ,100  ,27  ,99  ,30  ,28  ,30  ,98  ,29  ,30  ,29  ,29  ,29  ,30  ,30  ,30  ,28  ,31  ,29  ,27  ,31  ,97  ,29  ,28  ,29  ,99  ,29  ,99  ,30  ,99  ,29  ,99  ,27  ,2948  ,271  ,271  ,28  ,98  ,29  ,98  ,31  ,98  ,30  ,29  ,28  ,32  ,28  ,30  ,28  ,31  ,27  ,29  ,29  ,99  ,28  ,98  ,29  ,99  ,28  ,29  ,28  ,29  ,29  ,31  ,27  ,29  ,29  ,29  ,29  ,101  ,27  ,99  ,29  ,29  ,28  ,100  ,28  ,31  ,28  ,29  ,30  ,29  ,28  ,29  ,29  ,29  ,29  ,29  ,29  ,101  ,28  ,27  ,29  ,99  ,29  ,98  ,30  ,98  ,29  ,98  ,28 };
    sendData(decoder, seq4, std::extent<decltype(seq4)>::value);
    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772829743, code.getCommand());
}

TEST(IRDecoderTest, samsung_can_decode_universal_remote_at_prescaler_256) {
    IRDecoder_Samsung<MockPulseCounter> decoder;
    IRCode code;

    // Yamaha universal remote sends junk as repeat commands, not sure why.
    const uint16_t seq1[] = { 37750  ,272  ,275  ,26  ,100  ,29  ,100  ,29  ,100  ,30  ,30  ,29  ,30  ,29  ,30  ,29  ,31  ,29  ,30  ,29  ,101  ,29  ,101  ,29  ,100  ,29  ,30  ,30  ,30  ,29  ,30  ,29  ,30  ,29  ,30  ,29  ,100  ,29  ,100  ,29  ,101  ,29  ,31  ,29  ,32  ,28  ,31  ,29  ,30  ,29  ,32  ,27  ,31  ,28  ,32  ,28  ,31  ,27  ,101  ,31  ,99  ,28  ,100  ,29  ,100  ,31  ,99  ,28  ,2906  ,272  ,274  ,27  ,30  ,30  ,6077  ,272  ,274  ,26  ,30  ,29  ,6078  ,272  ,275  ,26  ,30  ,29  ,6077  ,271  ,274  ,27  ,31  ,29 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    EXPECT_TRUE(decoder.in() >> code);
    EXPECT_EQ(IRType::Command, code.getType());
    EXPECT_EQ(3772833823, code.getCommand());
}
