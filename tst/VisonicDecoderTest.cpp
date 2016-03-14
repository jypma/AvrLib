#include <gtest/gtest.h>
#include <type_traits>
#include "Visonic/VisonicDecoder.hpp"
#include <iostream>

namespace VisonicDecoderTest {

using namespace Serial;
using namespace Visonic;

struct MockPulseCounter {
    typedef uint16_t count_t;
    struct comparator_t {
        typedef uint16_t value_t;

        static constexpr uint8_t prescalerPower2 = 3; // prescaler of 2^3 = 8

        template <uint32_t usecs, typename return_t>
        static constexpr return_t microseconds2counts() {
            return (F_CPU >> prescalerPower2) / 1000 * usecs / 1000;
        }

        static uint16_t us2counts(uint16_t usecs) {
            return (F_CPU >> prescalerPower2) / 1000 * usecs / 1000;
        }
    };
};

template <typename T>
void sendData(T &decoder, const uint16_t *seq, uint16_t length) {
    bool high = true;
    for (uint8_t i = 0; i < length; i++) {
        Pulse event = { high, seq[i] };
        decoder.apply(event);
        high = !high;
    }
    decoder.apply(Pulse::empty());
}

TEST(VisonicDecoderTest, visonic_decoder_can_decode_correct_bit_sequences) {
    VisonicDecoder<MockPulseCounter> decoder;

    const uint16_t seq1[] = { 426,258,268,231,905,702,1659,1625,821,821,1598,884,1552,1716,744,1696,765,1667,774,874,1558,928,1515,1735,724,1707,749,893,1536,1720,743,1692,764,1678,777,871,1563,1706,744,892,1540,1726,736,906,1526,1729,736,911,1523,943,1506,1735,724,912,1527,934,1509,961,1490,956,1484,1752,715,1710,744,887,1544,926,1522,1736,725,1695,757,897,1551,1710,743 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    VisonicPacket pkt;
    EXPECT_TRUE(decoder.read(&pkt));

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
    // 141, 137, 106, 207, 4

    const uint16_t seq2[] = { 236,1114,1538,932,1595,889,1637,872,832,1666,1724,789,889,1622,915,1600,1760,775,1721,801,888,1618,1752,782,1706,815,1696,823,1697,823,858,1652,907,1598,953,1569,941,1585,1742,792,895,1618,1722,797,893,1633,1738,780,1731,794,882,1613,1759,785,888,1635,919,1592,957,1544,1772,778,884,1624,939,1576,1751,783,896,1619,924,1595,1760,762 };
    sendData(decoder, seq2, std::extent<decltype(seq1)>::value);

    EXPECT_TRUE(decoder.read(&pkt));

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
    // 111, 194, 43, 221, 6
}

TEST(VisonicDecoderTest, visonic_decoder_can_decode_more_bit_sequences) {
    VisonicDecoder<MockPulseCounter> decoder;

    const uint16_t seq1[] = { 500, 828, 500, 396, 904, 868, 484, 408, 892, 452, 872, 884, 476, 844, 496, 832, 500, 396, 900, 868, 488, 840, 492, 400, 900, 868, 484, 840, 496, 832, 500, 828, 496, 832, 504, 388, 904, 444, 876, 460, 868, 884, 480, 840, 492, 836, 496, 400, 900, 868, 484, 408, 896, 448, 872, 460, 868, 884, 480, 408, 892, 456, 868, 884, 480, 412, 892, 452, 872, 880, 480 };
    constexpr uint16_t count = std::extent<decltype(seq1)>::value;
    uint16_t seq1c[count];
    for (int i = 0; i < count; i++) {
        seq1c[i] = MockPulseCounter::comparator_t::us2counts(seq1[i]);
        //std::cout << seq1c[i] << ',';
    }
    std::cout << int(count) << " pulses." << endl;

    sendData(decoder, seq1c, std::extent<decltype(seq1)>::value);

    VisonicPacket pkt;
    EXPECT_TRUE(decoder.read(&pkt));

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
}


}
