#include <gtest/gtest.h>
#include <type_traits>
#include "HopeRF/JeeLibRxFifo.hpp"

using namespace HopeRF;

TEST(RFM12JeeLibRxFifo, rx_receives_packet_with_valid_crc) {
    JeeLibRxFifo<5,32,true> fifo;

    uint8_t bytes[] = {5,30,8,82,49,32,32,1,58,251,0,80,200};

    EXPECT_FALSE(fifo.isWriting());
    fifo.writeStart(bytes[0]);
    EXPECT_TRUE(fifo.isWriting());
    for (unsigned int i = 1; i < std::extent<decltype(bytes)>::value; i++) {
        fifo.write(bytes[i]);
    }
    EXPECT_FALSE(fifo.isWriting());
    EXPECT_TRUE(fifo.hasContent());
    auto in = fifo.in();
    EXPECT_EQ(9, in.getReadAvailable()); // 1 byte header + 8 bytes in the original packet.
    uint8_t b;
    in >> b;
    EXPECT_EQ(bytes[1], b); // header
    for (unsigned int i = 3; i < std::extent<decltype(bytes)>::value - 2; i++) {
        in >> b;
        EXPECT_EQ(bytes[i], b);
    }
}
/*
TEST(RFM12JeeLibRxFifo, print_crc) {

    uint8_t bytes[] = {5,30,8,82,49,32,32,1,58,251,0,80,200};

    CRC16 crc;
    for (unsigned int i = 0; i < std::extent<decltype(bytes)>::value - 2; i++) {
        crc.append(bytes[i]);
    }
    std::cout << " crc: " << crc.get() << " lo: " << (crc.get() & 0xFF) << " hi: " << (crc.get() >> 8 & 0xFF) << std::endl;
}
*/
/*
Initialized.
0 41 1 mode: 1cont: false
0 81 1 mode: 1cont: false
[5,30,8,82,49,32,32,1,58,251,0,80,200]
1 111 1 mode: 1cont: false
1 142 1 mode: 1cont: false
1 167 1 mode: 1cont: false
1 196 1 mode: 1cont: false
1 219 1 mode: 1cont: false
[5,30,18,82,49,32,32,4,187,11,4,0,0,0,0,0,0,0,0,0,0,197,234]
2 4 1 mode: 1cont: false
2 28 1 mode: 1cont: false
2 55 1 mode: 1cont: false
2 91 1 mode: 1cont: false
2 127 1 mode: 1cont: false
2 164 1 mode: 1cont: false
2 200 1 mode: 1cont: false
[5,30,18,82,49,32,32,4,188,11,4,0,0,0,0,0,0,0,0,0,0,206,173]
3 236 1 mode: 1cont: false
[5,3,12,82,50,32,32,10,108,8,81,0,72,13,0,150,28]
[5,30,12,82,50,32,32,10,108,8,81,0,72,13,0,139,1]
5 10 1 mode: 1cont: false
5 46 1 mode: 1cont: false
5 83 1 mode: 1cont: false
5 112 1 mode: 1cont: false
5 148 1 mode: 1cont: false
[5,31,18,82,49,32,32,4,189,11,4,0,0,0,0,0,0,0,0,0,0,241,253]
6 174 1 mode: 1cont: false
6 203 1 mode: 1cont: false
6 238 1 mode: 1cont: false
6 21 1 mode: 1cont: false
6 48 1 mode: 1cont: false
[5,3,12,82,54,32,32,10,14,8,35,0,16,14,0,121,194]
[5,30,12,82,54,32,32,10,14,8,35,0,16,14,0,100,223]
8 86 1 mode: 1cont: false
8 117 1 mode: 1cont: false
[5,31,18,82,49,32,32,4,190,11,4,0,0,0,0,0,0,0,0,0,0,244,62]
9 158 1 mode: 1cont: false
9 186 1 mode: 1cont: false
[5,30,10,68,66,32,32,4,69,1,84,0,0,194,1]
10 207 1 mode: 1cont: false
10 231 1 mode: 1cont: false
10 12 1 mode: 1cont: false
10 46 1 mode: 1cont: false
10 72 1 mode: 1cont: false
[5,31,18,82,49,32,32,4,191,11,4,0,0,0,0,0,0,0,0,0,0,246,191]
11 103 1 mode: 1cont: false
[5,184,13,234,11,239,225,52,69,120,32,128,158,118,121,185,33,130]
12 134 1 mode: 1cont: false
12 165 1 mode: 1cont: false
12 196 1 mode: 1cont: false
12 229 1 mode: 1cont: false
12 9 1 mode: 1cont: false
12 38 1 mode: 1cont: false
12 77 1 mode: 1cont: false
12 106 1 mode: 1cont: false
[5,31,18,82,49,32,32,4,192,11,4,0,0,0,0,0,0,0,0,0,0,93,128]
13 130 1 mode: 1cont: false
13 163 1 mode: 1cont: false
[5,3,12,82,52,32,32,10,22,8,62,0,231,13,0,183,123]
[5,3,12,82,53,32,32,10,170,7,52,0,16,14,0,231,176]
[5,30,12,82,53,32,32,10,170,7,52,0,16,14,0,250,173]
16 194 1 mode: 1cont: false
[5,27,12,16,232,46,166,248,3,121,124,168,170,55,147,64,222]
[5,70,12,103,248,54,84,244,200,196,48,4,121,216,195,113,224]
18 238 1 mode: 1cont: false
18 18 1 mode: 1cont: false
18 50 1 mode: 1cont: false
[5,31,18,82,49,32,32,4,193,11,4,0,0,0,0,0,0,0,0,0,0,95,1]
19 82 1 mode: 1cont: false

*/


