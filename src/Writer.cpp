#include "Writer.hpp"

Writer &Writer::operator << (const uint8_t b) {
    write(b);
    return *this;
}

Writer &Writer::operator << (const uint16_t i) {
    write(i >> 8);
    write(i);
    return *this;
}

Writer &Writer::operator << (const uint32_t i) {
    write(i >> 24);
    write(i >> 16);
    write(i >> 8);
    write(i);
    return *this;
}

Writer &Writer::operator << (const char *string) {
    if (string != nullptr) {
        uint8_t c = *string;
        while (c) {
            write(c);
            string++;
            c = *string;
        }
    }

    return *this;
}

Writer &Writer::operator << (Decimal<uint8_t> v) {
    if (v.value > 99) {
        write('0' + (v.value / 100));
        v.value %= 100;
        write('0' + (v.value / 10));
        v.value %= 10;
    } else if (v.value > 9) {
        write('0' + (v.value / 10));
        v.value %= 10;
    }
    write('0' + v.value);
    return *this;
}

Writer &Writer::operator << (Decimal<int8_t> v) {
    if (v.value < 0) {
        write('-');
        *this << dec(uint8_t(-v.value));
    } else {
        *this << dec(uint8_t(v.value));
    }
    return *this;
}

Writer &Writer::operator << (const Decimal<uint16_t> v) {
    const uint16_t n = v.value;

    uint8_t d4, d3, d2, d1, q;
    uint16_t d0;

    d0 = n       & 0xF;
    d1 = (n>>4)  & 0xF;
    d2 = (n>>8)  & 0xF;
    d3 = (n>>12);

    d0 = 6*(d3 + d2 + d1) + d0;
    q = d0 / 10;
    d0 = d0 % 10;

    d1 = q + 9*d3 + 5*d2 + d1;
    if (d1 != 0) {
        q = d1 / 10;
        d1 = d1 % 10;

        d2 = q + 2*d2;
        if ((d2 != 0) || (d3 != 0)) {
            q = d2 / 10;
            d2 = d2 % 10;

            d3 = q + 4*d3;
            if (d3 != 0) {
                q = d3 / 10;
                d3 = d3 % 10;

                d4 = q;

                if (d4 != 0) {
                    write( d4 + '0' );
                }
                write( d3 + '0' );
            }
            write( d2 + '0' );
        }
        write( d1 + '0' );
    }
    write( d0 + '0' );

    return *this;
}

Writer &Writer::operator << (Decimal<int16_t> v) {
    if (v.value < 0) {
        write('-');
        *this << dec(uint16_t(-v.value));
    } else {
        *this << dec(uint16_t(v.value));
    }
    return *this;
}

