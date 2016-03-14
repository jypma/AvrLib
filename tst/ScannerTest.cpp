#include <gtest/gtest.h>
#include "Fifo.hpp"
#include "Streams/Scanner.hpp"

namespace ScannerTest {

using namespace Streams;

TEST(ScannerTest, scan_can_find_a_token_in_a_fifo) {
    uint8_t ch = 0;
    bool invoked = false;

    Fifo<16> fifo;
    fifo.write(F("abcdef"));

    scan(fifo, [&] (auto &read) {
        if (read(F("abd"))) { FAIL(); }
        if (read(F("cde"), &ch)) { invoked = true; }
        if (read(F("e"))) { FAIL(); }
    });

    EXPECT_EQ('f', ch);
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(invoked);
}


TEST(ScannerTest, chunk_with_prefix_and_separator_can_be_read) {
    Fifo<24> storage;
    ChunkedFifo fifo = storage;
    bool invoked = false;

    auto testdata = Fifo<24>();
    testdata.write(F("+++DATA5:abcde+++"));

    fifo.writeStart();
    scan(testdata, [&] (auto &read) {
        uint8_t length;
        if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
            fifo.writeEnd();
            invoked = true;
        }
    });

    EXPECT_TRUE(invoked);
    EXPECT_EQ(6, fifo.getSize());     // "abcde" has been put into the chunked fifo, plus length
    //TODO expect read from fifo once ChunkedFifo has read(...)
}

TEST(ScannerTest, chunk_with_two_digit_length_can_be_read) {
    Fifo<40> storage;
    ChunkedFifo fifo = storage;
    bool invoked = false;

    auto testdata = Fifo<40>();
    testdata.write(F("+++DATA10:abcdefghij+++"));

    fifo.writeStart();
    scan(testdata, [&] (auto &read) {
        uint8_t length;
        if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
            fifo.writeEnd();
            invoked = true;
        }
    });

    EXPECT_TRUE(invoked);
    EXPECT_EQ(11, fifo.getSize());     // "abcdefghij" has been put into the chunked fifo, plus length
    //EXPECT_EQ(3, testdata.getSize());  // "+++" at the end is still left
}

TEST(ScannerTest, chunk_with_three_digit_length_thats_too_large_for_output_fifo_is_read_and_discarded) {
    Fifo<40> storage;
    ChunkedFifo fifo = storage;
    bool invoked = false;

    auto testdata = Fifo<254>();
    testdata.write(F("DATA240:"));
    for (uint8_t i = 0; i < 240; i++) testdata.write(i);

    fifo.writeStart();
    scan(testdata, [&] (auto &read) {
        uint8_t length;
        if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
            fifo.writeEnd();
            invoked = true;
        }
    });

    EXPECT_TRUE(invoked);
    EXPECT_EQ(1, fifo.getSize()); // Chunk was discarded, but we still got an empty hit.
    EXPECT_EQ(0, testdata.getSize()); // All test data was read
}

TEST(ScannerTest, chunk_is_not_read_on_incorrect_separator) {
    Fifo<24> storage;
    ChunkedFifo fifo = storage;

    auto testdata = Fifo<24>();
    testdata.write(F("+++DATA5_abcde+++"));

    fifo.writeStart();
    scan(testdata, [&] (auto &read) {
        uint8_t length;
        if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
            fifo.writeEnd();
            FAIL();
        }
    });

    EXPECT_TRUE(fifo.isEmpty());
}

TEST(ScannerTest, incomplete_chunk_is_ignored_until_data_is_available) {
    Fifo<24> storage;
    ChunkedFifo fifo = storage;

    fifo.writeStart();
    auto matcher = [&] (auto &read) {
        uint8_t length;
        if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
            fifo.writeEnd();
        }
    };

    auto testdata = Fifo<24>();
    testdata.write(F("+++DA"));
    scan(testdata, matcher);

    EXPECT_EQ(2, testdata.getSize()); // "+++" has been eaten

    testdata.write(F("TA5:abc"));
    scan(testdata, matcher);

    EXPECT_EQ(9, testdata.getSize());

    testdata.write(F("de+++"));
    scan(testdata, matcher);

    EXPECT_EQ(3, testdata.getSize()); // the remaining +++ at the end
    EXPECT_EQ(6, storage.getSize());  // "abcde" + length
}


TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer) {
    bool invoked = false;

    auto testdata = Fifo<24>();
    testdata.write(F("+DATA"));

    scan(testdata, [&] (auto &read) {
        if (read(F("DATA"))) {
            invoked = true;
        }
        if (read(F("BOOHOO"))) {
            FAIL();
        }
    });

    EXPECT_TRUE(invoked);
}

TEST(ScannerTest, scan_does_not_eat_chars_that_are_correct_prefix) {
    bool invoked = false;

    auto matcher = [&] (auto &read) {
        if (read(F("DATA"))) {
            invoked = true;
        }
    };

    auto testdata = Fifo<24>();
    testdata.write(F("+DA"));
    scan(testdata, matcher);
    EXPECT_EQ(2, testdata.getSize()); // "+" has been eaten since it's not part of the token
    testdata.write(F("TA"));
    scan(testdata, matcher);
    EXPECT_TRUE(invoked);
}

TEST(ScannerTest, scan_does_not_eat_chars_when_receiving_chunk_one_by_one) {
    AbstractFifo testdata = Fifo<24>();
    Fifo<24> storage;
    ChunkedFifo fifo = storage;
    bool invoked = false;
    fifo.writeStart();

    auto loop = [&] {
        scan(testdata, [&] (auto &read) {
            uint8_t length;
            if (read(F("DATA"), Decimal(&length), F(":"), ChunkWithLength(&length, fifo))) {
                invoked = true;
                fifo.writeEnd();
            }
        });
    };

    testdata.write(F("+"));
    loop();
    EXPECT_EQ(0, testdata.getSize());

    testdata.write(F("D"));
    loop();
    EXPECT_EQ(1, testdata.getSize());

    testdata.write(F("A"));
    loop();
    EXPECT_EQ(2, testdata.getSize());

    testdata.write(F("T"));
    loop();
    EXPECT_EQ(3, testdata.getSize());

    testdata.write(F("A"));
    loop();
    EXPECT_EQ(4, testdata.getSize());

    testdata.write(F("3"));
    loop();
    EXPECT_EQ(5, testdata.getSize());

    testdata.write(F(":"));
    loop();
    EXPECT_EQ(6, testdata.getSize());

    testdata.write(F("a"));
    loop();
    EXPECT_EQ(7, testdata.getSize());

    testdata.write(F("b"));
    loop();
    EXPECT_EQ(8, testdata.getSize());

    testdata.write(F("c"));
    loop();
    EXPECT_EQ(0, testdata.getSize());
    EXPECT_TRUE(invoked);
    EXPECT_EQ(4, fifo.getSize());     // "abc" has been put into the chunked fifo, plus length
}

TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer_but_matches_prefix) {
    bool invoked = false;

    auto testdata = Fifo<24>();
    testdata.write(F("+DATA"));

    scan(testdata, [&] (auto &read) {
        if (read(F("DATA"))) {
            invoked = true;
        }
        else if (read(F("+OOHOO"))) {
            FAIL();
        }
    });

    EXPECT_TRUE(invoked);
}

TEST(ScannerTest, scanner_can_read_ints) {
    Fifo<16> fifo;
    for (uint8_t i = 0; i < 14; i++) {
        fifo.write(i);
    }

    scan(fifo, [] (auto &read) {
        uint8_t uint8;
        int8_t int8;
        uint16_t uint16;
        int16_t int16;
        uint32_t uint32;
        int32_t int32;
        if (read(&uint8, &int8, &uint16, &int16, &uint32, &int32)) {
            EXPECT_EQ(0x00, uint8);
            EXPECT_EQ(0x01, int8);
            EXPECT_EQ(0x0302, uint16);   // little endian -> LSByte is read first
            EXPECT_EQ(0x0504, int16);
            EXPECT_EQ(0x09080706, uint32);
            EXPECT_EQ(0x0D0C0B0A, int32);
        } else {
            FAIL();
        }
    });
}

}
