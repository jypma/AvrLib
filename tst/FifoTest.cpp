#include <gtest/gtest.h>
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

    uint8_t b;
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(42, b);
    EXPECT_TRUE(fifo.remove(b));
    EXPECT_EQ(84, b);

    EXPECT_TRUE(fifo.isFull());
    EXPECT_FALSE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
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
