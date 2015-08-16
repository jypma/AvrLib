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
                on<Format<Token<'a','b','d'>>>(s, [] { FAIL(); });
                on<Format<Token<'c','d','e'>, Scalar<uint8_t, &T::ch>>>(s, [this] { invoked = true; });
                on<Format<Token<'e'>>>(s, [] { FAIL(); });
            });

            EXPECT_EQ('f', ch);
            EXPECT_EQ(0, fifo.getSize());
            EXPECT_TRUE(invoked);
        }
    } t;
}
/*
TEST(ScannerTest, scan_drops_nonmatched_tokens_until_below_token_length) {
    struct T: public Streamable<T> {
        uint8_t ch = 0;

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "abc";

            scan(fifo, *this, [this] (auto s) {
                on<Format<Token<'c','d','e'>>>(s, [this] { FAIL(); });
            });

            EXPECT_EQ(2, fifo.getSize());
            EXPECT_EQ('b', fifo.peek());
        }
    } t;
}
*/
/*
TEST(ScannerTest, scan_drops_until_token_length_minus_one_if_no_token_matches_partially) {
    struct T: public Streamable<T> {
        uint8_t ch = 0;

        T() {
            auto fifo = Fifo<16>();
            fifo.out() << "abcde";

            scan(fifo, *this, [this] (auto s) {
                on<Format<Token<'c','a'>>>(s, [this] { FAIL(); });
            });

            EXPECT_EQ(1, fifo.getSize());
        }
    } t;
}
*/
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
                    Token<'D','A', 'T', 'A'>,
                    Chunk<&T::fifo, Format<Token<':'>>>>
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
                    Token<'D','A', 'T', 'A'>,
                    Chunk<&T::fifo, Format<Token<':'>>>>
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
                    Token<'D','A', 'T', 'A'>,
                    Chunk<&T::fifo, Format<Token<':'>>>>
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
                    Token<'D','A', 'T', 'A'>,
                    Chunk<&T::fifo, Format<Token<':'>>>>
                >(s, [this] {
                    FAIL();
                });
            });

            //EXPECT_EQ(5, testdata.getSize());  // Since nothing was matched, characters are eaten until remaining length
            //                                   // is one less than the minimum length of a pattern match (which is 6 for the
            //                                   // test case, "DATA" + "1" + ":".
            EXPECT_TRUE(fifo.isEmpty());
        }
    } t;

}

TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer) {
    struct T: public Streamable<T> {
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+DATA";

            scan(testdata, [this] (auto s) {
                on<Format<Token<'D','A', 'T','A'>>>(s, [this] {
                    invoked = true;
                });
                on<Format<Token<'B','O','O','H','O','O'>>>(s, [this] {
                    FAIL();
                });
            });

            EXPECT_TRUE(invoked);
        }
    } t;
}

TEST(ScannerTest, scan_can_match_first_branch_if_second_branch_is_longer_but_matches_prefix) {
    struct T: public Streamable<T> {
        Fifo<24> storage;
        ChunkedFifo fifo = &storage;
        bool invoked = false;

        T() {
            auto testdata = Fifo<24>();
            testdata.out() << "+DATA";

            scan(testdata, [this] (auto s) {
                on<Format<Token<'D','A', 'T','A'>>>(s, [this] {
                    invoked = true;
                });
                on<Format<Token<'+','O','O','H','O','O'>>>(s, [this] {
                    FAIL();
                });
            });

            EXPECT_TRUE(invoked);
        }
    } t;
}

}
