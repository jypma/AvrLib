#ifndef ETHERNET_MAC_ADDRESS_H
#define ETHERNET_MAC_ADDRESS_H

#include <stdint.h>
#include "Streams/Streamable.hpp"

namespace Espressif {

using namespace Streams;

class EthernetMACAddress: public Streamable<EthernetMACAddress> {
    uint8_t data1, data2, data3, data4, data5, data6;
public:
    EthernetMACAddress(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6):
        data1(d1), data2(d2), data3(d3), data4(d4), data5(d5), data6(d6) {}

    EthernetMACAddress(): data1(0),data2(0),data3(0),data4(0),data5(0),data6(0) {}

    inline uint8_t byte1() const { return data1; }
    inline uint8_t byte2() const { return data2; }
    inline uint8_t byte3() const { return data3; }
    inline uint8_t byte4() const { return data4; }
    inline uint8_t byte5() const { return data5; }
    inline uint8_t byte6() const { return data6; }

    typedef Format<
        Hexadecimal<uint8_t, &EthernetMACAddress::data1>,
        Token<STR(":")>,
        Hexadecimal<uint8_t, &EthernetMACAddress::data2>,
        Token<STR(":")>,
        Hexadecimal<uint8_t, &EthernetMACAddress::data3>,
        Token<STR(":")>,
        Hexadecimal<uint8_t, &EthernetMACAddress::data4>,
        Token<STR(":")>,
        Hexadecimal<uint8_t, &EthernetMACAddress::data5>,
        Token<STR(":")>,
        Hexadecimal<uint8_t, &EthernetMACAddress::data6>
    > Proto;
};

}

#endif
