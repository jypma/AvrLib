#include <gtest/gtest.h>
#include <iostream>
#include "Fifo.hpp"

TEST(FifoTest, empty_fifo_reports_as_empty) {
    Fifo<16> fifo;

    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_EQ(16, fifo.getCapacity());
}

TEST(FifoTest, fifo_size_1_can_be_used_twice) {
    Fifo<1> fifo;
    fifo.append(42);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_TRUE(fifo.isFull());
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(1, fifo.getCapacity());

    uint8_t dequeued;
    EXPECT_TRUE(fifo.remove(dequeued));
    EXPECT_EQ(42, dequeued);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());

    fifo.append(84);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_TRUE(fifo.isFull());
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(1, fifo.getCapacity());

    EXPECT_TRUE(fifo.remove(dequeued));
    EXPECT_EQ(84, dequeued);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
}

TEST(FifoTest, fifo_size_2_can_be_used_twice) {
    Fifo<2> fifo;
    fifo.append(42);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());

    uint8_t dequeued;
    EXPECT_TRUE(fifo.remove(dequeued));
    EXPECT_EQ(42, dequeued);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());

    fifo.append(84);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());

    EXPECT_TRUE(fifo.remove(dequeued));
    EXPECT_EQ(84, dequeued);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
}

TEST(FifoTest, fifo_size_2_is_full_after_two_elements) {
    Fifo<2> fifo;
    fifo.append(42);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());

    fifo.append(84);

    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_TRUE(fifo.isFull());
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());

    uint8_t v1;
    EXPECT_TRUE(fifo.remove(v1));
    EXPECT_EQ(42, v1);
    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());

    uint8_t v2;
    EXPECT_TRUE(fifo.remove(v2));
    EXPECT_EQ(84, v2);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());
}

TEST(FifoTest, peek_shows_first_element_without_removing) {
    Fifo<2> fifo;
    fifo.append(42);

    EXPECT_EQ(42, fifo.peek());
    EXPECT_EQ(1, fifo.getSize());
    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(42, b);
}

TEST(FifoTest, append_during_write_mark_cannot_be_read_until_committed) {
    Fifo<2> fifo;
    fifo.markWrite();
    fifo.append(42);

    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());

    fifo.append(84);

    EXPECT_TRUE(fifo.isFull());   // marked writes DO count towards used space
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());

    fifo.commitWrite();

    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_FALSE(fifo.isEmpty());

    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(42, b);
    EXPECT_EQ(1, fifo.getSize());

    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(84, b);
    EXPECT_EQ(0, fifo.getSize());
}

TEST(FifoTest, writes_during_write_mark_disappear_after_reset) {
    Fifo<2> fifo;
    fifo.markWrite();
    fifo.append(2);
    fifo.append(3);
    fifo.resetWrite();

    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());

    fifo.append(42);
    fifo.append(84);

    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(42, b);
    EXPECT_EQ(1, fifo.getSize());

    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(84, b);
    EXPECT_EQ(0, fifo.getSize());
}

TEST(FifoTest, remove_during_marked_read_does_not_free_up_space_until_committed) {
    Fifo<2> fifo;
    fifo.append(42);
    fifo.append(84);
    fifo.markRead();

    EXPECT_TRUE(fifo.isFull());
    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_TRUE(fifo.isFull());
    EXPECT_EQ(42, b);
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_TRUE(fifo.isFull());
    EXPECT_EQ(84, b);

    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());

    fifo.commitRead();

    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(FifoTest, remove_during_marked_read_can_be_repeated_after_reset) {
    Fifo<2> fifo;
    fifo.append(42);
    fifo.append(84);
    fifo.markRead();
    uint8_t dummy;
    fifo.remove(dummy);
    fifo.remove(dummy);
    fifo.resetRead();

    EXPECT_TRUE(fifo.isFull());
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(2, fifo.getSize());
    EXPECT_FALSE(fifo.isEmpty());

    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(42, b);
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(84, b);

    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_TRUE(fifo.isEmpty());
}

TEST(FifoTest, anonymous_writer_applies_after_leaving_scope) {
    Fifo<16> fifo;
    EXPECT_TRUE(fifo.out() << uint16_t(4200) << uint8_t(84));
    EXPECT_EQ(3, fifo.getSize());
}

TEST(FifoTest, anonymous_unconverted_writer_applies_after_leaving_scope) {
    Fifo<16> fifo;
    fifo.out() << uint16_t(4200) << uint8_t(84);
    EXPECT_EQ(3, fifo.getSize());
}

TEST(FifoTest, fifo_operates_rotating) {
    Fifo<3> fifo;

    for (int loop = 0; loop < 3; loop++) {
        std::cout << "loop " << loop << std::endl;

        uint8_t out;
        int in = 1;

        fifo.append(in++);
        fifo.append(in++);
        fifo.append(in++);

        EXPECT_TRUE(fifo.isFull());
        EXPECT_EQ(3, fifo.getSize());

        EXPECT_TRUE(fifo.remove(out));

        EXPECT_EQ(1, out);
        EXPECT_FALSE(fifo.isFull());
        EXPECT_EQ(2, fifo.getSize());

        EXPECT_TRUE(fifo.append(in++));

        EXPECT_TRUE(fifo.isFull());
        EXPECT_EQ(3, fifo.getSize());
        fifo.remove(out);
        EXPECT_EQ(2, out);

        EXPECT_FALSE(fifo.isFull());
        EXPECT_EQ(2, fifo.getSize());
        fifo.remove(out);
        EXPECT_EQ(3, out);

        EXPECT_FALSE(fifo.isFull());
        EXPECT_EQ(1, fifo.getSize());
        fifo.remove(out);
        EXPECT_EQ(4, out);

        EXPECT_FALSE(fifo.isFull());
        EXPECT_EQ(0, fifo.getSize());
        EXPECT_TRUE(fifo.isEmpty());

    }
}

TEST(FifoTest, resetWrite_on_unmarked_fifo_has_no_effect) {
    Fifo<3> fifo;
    fifo.resetWrite();
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
}

TEST(FifoTest, resetRead_on_unmarked_fifo_has_no_effect) {
    Fifo<3> fifo;
    fifo.resetRead();
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
}

enum class TestEnum: uint8_t { RED, GREEN, YELLOW };

struct TestItem {
    TestEnum color;
    uint16_t amount;
    static void write(Writer &out, const TestItem &evt) {
        out << evt.color;
        out << evt.amount;
    }
    static void read(Reader &in, TestItem &evt) {
        if (in >> evt.color) {
            in >> evt.amount;
        }
    }
};

TEST(FifoTest, writer_rolls_back_on_full_fifo) {
    Fifo<16> fifo;
    TestItem item;
    item.color = TestEnum::GREEN;
    item.amount = 0;
    for (int i = 0; i < 5; i++) {
        item.amount++;
        fifo.out() << item;
    }
    EXPECT_EQ(15, fifo.getSize());
    fifo.out() << item;
    EXPECT_EQ(15, fifo.getSize());
}

TEST(FifoTest, reader_rolls_back_on_incomplete_fifo) {
    Fifo<16> fifo;
    fifo.out() << TestEnum::RED;
    TestItem item;
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_FALSE(fifo.in() >> item);
    EXPECT_EQ(1, fifo.getSize());
}

TEST(FifoTest, reservation_can_be_written_to) {
    Fifo<16> fifo;
    fifo.markWrite();
    volatile uint8_t *ptr = nullptr;
    EXPECT_TRUE(fifo.reserve(ptr));
    *ptr = 42;
    fifo.commitWrite();

    EXPECT_EQ(1, fifo.getSize());
    uint8_t out;
    fifo.remove(out);
    EXPECT_EQ(42, out);
}

TEST(FifoTest, reserve_outside_markWrite_returns_false) {
    Fifo<16> fifo;
    volatile uint8_t *ptr = nullptr;
    EXPECT_FALSE(fifo.reserve(ptr));
}

TEST(FifoTest, reserve_on_full_fifo_returns_false) {
    Fifo<2> fifo;
    fifo.append(1);
    fifo.append(1);
    fifo.markWrite();
    volatile uint8_t *ptr = nullptr;
    EXPECT_FALSE(fifo.reserve(ptr));
}
