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

    uint8_t dequeued = fifo.remove();

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

    dequeued = fifo.remove();

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

    uint8_t dequeued = fifo.remove();

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

    dequeued = fifo.remove();

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

    uint8_t v1 = fifo.remove();

    EXPECT_EQ(42, v1);
    EXPECT_FALSE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_TRUE(fifo.hasContent());
    EXPECT_EQ(1, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());

    uint8_t v2 = fifo.remove();

    EXPECT_EQ(84, v2);
    EXPECT_TRUE(fifo.isEmpty());
    EXPECT_FALSE(fifo.isFull());
    EXPECT_TRUE(fifo.hasSpace());
    EXPECT_FALSE(fifo.hasContent());
    EXPECT_EQ(0, fifo.getSize());
    EXPECT_EQ(2, fifo.getCapacity());
}
