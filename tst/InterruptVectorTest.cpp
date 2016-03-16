#include <gtest/gtest.h>
#include "avr/common.h"

#include "HAL/Atmel/InterruptVectors.hpp"

#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, USART_RX_, USART_UDRE_, INT0_)


namespace InterruptVectorTest {

struct TypeWithHandler {
    typedef TypeWithHandler This;
    bool invoked = false;

    void onUSART_RX() {
        invoked = true;
    }

    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_RX), onUSART_RX);
};

struct TypeWithTwoHandlers {
    typedef TypeWithTwoHandlers This;
    bool invokedRX = false;
    bool invokedUDRE = false;

    void onUSART_RX() {
        invokedRX = true;
    }
    void onUSART_UDRE() {
        invokedUDRE = true;
    }

    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_RX), onUSART_RX);
    INTERRUPT_HANDLER2(INTERRUPT_VECTOR(USART_UDRE), onUSART_UDRE);
};

struct WrappingVector {
    typedef INTERRUPT_VECTOR(INT0) VECT;
    static bool invoked;

    template <typename body_t>
    static void wrap(body_t body) {
        invoked = true;
        body();
    }
};

bool WrappingVector::invoked = false;

struct TypeWithWrappedVector {
    typedef TypeWithWrappedVector This;
    bool invoked = false;

    void onINT0() {
        invoked = true;
    }

    INTERRUPT_HANDLER1(WrappingVector, onINT0);
};

TypeWithHandler testPin1;
TypeWithHandler testPin2;
TypeWithTwoHandlers testPin3;
TypeWithWrappedVector testWrapped;

mkISRS(testPin1, testPin2, testPin3, testWrapped);

TEST(InterruptVectorTest, invoking_interrupt_handler_should_forward_to_method_and_can_chain) {
    testPin1.invoked = false;
    testPin2.invoked = false;
    USART_RX_vect();
    EXPECT_TRUE(testPin1.invoked);
    EXPECT_TRUE(testPin2.invoked);
}

TEST(InterruptVectorTest, two_different_interrupt_handlers_can_be_registered_and_invoked) {
    testPin3.invokedRX = false;
    testPin3.invokedUDRE = false;
    USART_RX_vect();
    EXPECT_TRUE(testPin3.invokedRX);
    EXPECT_FALSE(testPin3.invokedUDRE);

    testPin3.invokedRX = false;
    USART_UDRE_vect();
    EXPECT_FALSE(testPin3.invokedRX);
    EXPECT_TRUE(testPin3.invokedUDRE);
}

TEST(InterruptVectorTest, wrapping_vector_is_recognized_and_applied) {
    WrappingVector::invoked = false;
    testWrapped.invoked = false;
    INT0_vect();
    EXPECT_TRUE(WrappingVector::invoked);
    EXPECT_TRUE(testWrapped.invoked);
}

}

