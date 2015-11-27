#include <gtest/gtest.h>
#include "Streams/Streamable.hpp"
#include "Streams/Scanner.hpp"

using namespace Streams;

namespace ScannerTest {

TEST(ScannerTest, scan_can_find_a_token_in_a_fifo) {
    struct T: public Streamable<T> {
        uint8_t ch = 0;
        bool invoked = false;

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "abcdef";

            scan(fifo, *this, [this] (auto s) {
                on<Format<Token<STR("abd")>>>(s, [] { FAIL(); });
                on<Format<Token<STR("cde")>, Binary<uint8_t, &T::ch>>>(s, [this] { invoked = true; });
                on<Format<Token<STR("e")>>>(s, [] { FAIL(); });
            });

            EXPECT_EQ('f', ch);
            EXPECT_EQ(0, fifo.getSize());
            EXPECT_TRUE(invoked);
        }
    } t;
}


TEST(ScannerTest, chunk_with_prefix_and_separator_can_be_read) {
    struct T: public Streamable<T> {
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+++DATA5:abcde+++";

            scan(testdata, *this, [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {
                    invoked = true;
                });
            });

            EXPECT_TRUE(invoked);
            EXPECT_EQ(6, fifo.getSize());     // "abcde" has been put into the chunked fifo, plus length
            EXPECT_EQ(3, testdata.getSize()); // "+++" at the end is still left
        }
    } t;
}

TEST(ScannerTest, chunk_with_two_digit_length_can_be_read) {
    struct T: public Streamable<T> {
        Fifo<40> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        T() {
            auto testdata = Fifo<40>();
            testdata.out() << "+++DATA10:abcdefghij+++";

            scan(testdata, *this, [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {
                    invoked = true;
                });
            });

            EXPECT_TRUE(invoked);
            EXPECT_EQ(11, fifo.getSize());     // "abcdefghij" has been put into the chunked fifo, plus length
            EXPECT_EQ(3, testdata.getSize());  // "+++" at the end is still left
        }
    } t;
}

TEST(ScannerTest, chunk_with_three_digit_length_thats_too_large_for_output_fifo_is_read_and_discarded) {
    struct T: public Streamable<T> {
        Fifo<40> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        T() {
            auto testdata = Fifo<254>();
            testdata.out() << "DATA240:";
            for (int i = 0; i < 240; i++) testdata.write(i);

            scan(testdata, *this, [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {
                    invoked = true;
                });
            });

            EXPECT_TRUE(invoked);
            EXPECT_EQ(0, fifo.getSize()); // Chunk was discarded since it's too big
            EXPECT_EQ(0, testdata.getSize()); // All test data was read
        }
    } t;
}

TEST(ScannerTest, chunk_is_not_read_on_incorrect_separator) {
    struct T: public Streamable<T> {
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+++DATA5_abcde+++";

            scan(testdata, *this, [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {
                    FAIL();
                });
            });

            EXPECT_TRUE(fifo.isEmpty());
        }
    } t;
}

TEST(ScannerTest, incomplete_chunk_is_ignored_until_data_is_available) {
    struct T: public Streamable<T> {
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;

        T() {
            auto matcher = [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {

                });
            };

            auto testdata = Fifo<24>();
            testdata.out() << "+++DA";
            scan(testdata, *this, matcher);

            EXPECT_EQ(2, testdata.getSize()); // "+++" has been eaten

            testdata.out() << "TA5:abc";
            scan(testdata, *this, matcher);

            EXPECT_EQ(9, testdata.getSize());

            testdata.out() << "de+++";
            scan(testdata, *this, matcher);

            EXPECT_EQ(3, testdata.getSize()); // the remaining +++ at the end
            EXPECT_EQ(6, storage.getSize());  // "abcde" + length
        }
    } t;

}


TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer) {
    struct T: public Streamable<T> {
        bool invoked = false;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+DATA";

            scan(testdata, [this] (auto s) {
                on<Format<Token<STR("DATA")>>>(s, [this] {
                    invoked = true;
                });
                on<Format<Token<STR("BOOHOO")>>>(s, [this] {
                    FAIL();
                });
            });

            EXPECT_TRUE(invoked);
        }
    } t;
}

TEST(ScannerTest, scan_does_not_eat_chars_that_are_correct_prefix) {
    struct T: public Streamable<T> {
        bool invoked = false;

        T() {
            auto matcher = [this] (auto s) {
                on<Format<Token<STR("DATA")>>>(s, [this] {
                    invoked = true;
                });
            };

            auto testdata = Fifo<24>();
            testdata.out() << "+DA";
            scan(testdata, matcher);
            EXPECT_EQ(2, testdata.getSize()); // "+" has been eaten since it's not part of the token
            testdata.out() << "TA";
            scan(testdata, matcher);
            EXPECT_TRUE(invoked);
        }
    } t;
}

TEST(ScannerTest, scan_does_not_eat_chars_when_receiving_chunk_one_by_one) {
    struct T: public Streamable<T> {
        AbstractFifo testdata = Fifo<24>();
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        void loop() {
            scan(testdata, *this, [this] (auto s) {
                on<Format<
                    Token<STR("DATA")>,
                    Chunk<&T::fifo, Format<Token<STR(":")>>>>
                >(s, [this] {
                    invoked = true;
                });
            });
        }

        T() {
            testdata.out() << "+";
            loop();
            EXPECT_EQ(0, testdata.getSize());

            testdata.out() << "D";
            loop();
            EXPECT_EQ(1, testdata.getSize());

            testdata.out() << "A";
            loop();
            EXPECT_EQ(2, testdata.getSize());

            testdata.out() << "T";
            loop();
            EXPECT_EQ(3, testdata.getSize());

            testdata.out() << "A";
            loop();
            EXPECT_EQ(4, testdata.getSize());

            testdata.out() << "3";
            loop();
            EXPECT_EQ(5, testdata.getSize());

            testdata.out() << ":";
            loop();
            EXPECT_EQ(6, testdata.getSize());

            testdata.out() << "a";
            loop();
            EXPECT_EQ(7, testdata.getSize());

            testdata.out() << "b";
            loop();
            EXPECT_EQ(8, testdata.getSize());

            testdata.out() << "c";
            loop();
            EXPECT_EQ(0, testdata.getSize());
            EXPECT_TRUE(invoked);
            EXPECT_EQ(4, fifo.getSize());     // "abc" has been put into the chunked fifo, plus length
        }
    } t;
}

TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer_but_matches_prefix) {
    struct T: public Streamable<T> {
        bool invoked = false;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+DATA";

            scan(testdata, [this] (auto s) {
                on<Format<Token<STR("DATA")>>>(s, [this] {
                    invoked = true;
                });
                on<Format<Token<STR("+OOHOO")>>>(s, [this] {
                    FAIL();
                });
            });

            EXPECT_TRUE(invoked);
        }
    } t;
}

}
