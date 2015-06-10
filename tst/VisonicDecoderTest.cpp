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
    };
};

template <typename T>
void sendData(T &decoder, const uint16_t *seq, uint16_t length) {
    PulseType type = PulseType::HIGH;
    for (uint8_t i = 0; i < length; i++) {
        PulseEvent<MockPulseCounter::count_t> event = { type, seq[i] };
        decoder.apply(event);
        type = (type == PulseType::HIGH) ? PulseType::LOW : PulseType::HIGH;
    }
    PulseEvent<MockPulseCounter::count_t> event = { PulseType::TIMEOUT, 0 };
    decoder.apply(event);
}

TEST(VisonicDecoderTest, visonic_decoder_can_decode_correct_bit_sequences) {
    VisonicDecoder<MockPulseCounter> decoder;

    const uint16_t seq1[] = { 426,258,268,231,905,702,1659,1625,821,821,1598,884,1552,1716,744,1696,765,1667,774,874,1558,928,1515,1735,724,1707,749,893,1536,1720,743,1692,764,1678,777,871,1563,1706,744,892,1540,1726,736,906,1526,1729,736,911,1523,943,1506,1735,724,912,1527,934,1509,961,1490,956,1484,1752,715,1710,744,887,1544,926,1522,1736,725,1695,757,897,1551,1710,743 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    VisonicPacket pkt;
    EXPECT_TRUE(decoder.in() >> pkt);

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
    // 141, 137, 106, 207, 4

    const uint16_t seq2[] = { 236,1114,1538,932,1595,889,1637,872,832,1666,1724,789,889,1622,915,1600,1760,775,1721,801,888,1618,1752,782,1706,815,1696,823,1697,823,858,1652,907,1598,953,1569,941,1585,1742,792,895,1618,1722,797,893,1633,1738,780,1731,794,882,1613,1759,785,888,1635,919,1592,957,1544,1772,778,884,1624,939,1576,1751,783,896,1619,924,1595,1760,762 };
    sendData(decoder, seq2, std::extent<decltype(seq1)>::value);

    EXPECT_TRUE(decoder.in() >> pkt);

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
    // 111, 194, 43, 221, 6
}

/*
TEST(VisonicDecoderTest, visonic_decoder_can_decode_long_bit_sequences) {
    VisonicDecoder<MockPulseCounter> decoder;

    const uint16_t seq1[] = { 739,1344,1904,774,1667,804,1654,818,822,1614,1634,819,823,1610,848,1624,875,344,230,970,1680,783,858,1620,900,1519,921,1539,899,1405,1025,1327,231,176,799,1514,6856,230,446,283,533,924,1576,840,817,1559,886,1409,1056,1545,892,1309,1160,1559,905,1573,828,1408,1083,1051,231,271,1669,800,1661,788,1657,809,832,1595,873,1548,947,1284,1936,767,911,1558,875,1575,1682,751,1704,779,1645,783,839,1647,1630,816,852,1574,865,1590,878,259,231,1054,1694,791,823,1628,856,1606,864,1581,874,1005,231,329,923,773,231,534,1048,1401,4207,230,1000,230,2703,873,1593,829,794,1618,1698,817,1618,784,824,1632,872,1551,974,796,231,497,873,1537,889,1540,964,1522,903,1564,866,1566,1693,811,1635,813,1631,760,903,1569,873,1579,881,921,231,438,1636,866,770,1612,889,1580,1693,795,1618,802,1630,821,872,1565,1676,795,904,1522,848,1609,935,1522,1654,828,824,1246,231,175 };
    sendData(decoder, seq1, std::extent<decltype(seq1)>::value);

    VisonicPacket pkt;
    EXPECT_TRUE(decoder.in() >> pkt);

    std::cout << int(pkt.data[0]) << ", " << int(pkt.data[1]) << ", " << int(pkt.data[2]) << ", " << int(pkt.data[3]) << ", " << int(pkt.data[4]) << std::endl;
}
*/


}
