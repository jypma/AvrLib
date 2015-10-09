#include <gtest/gtest.h>
#include "avr/common.h"

#include "HAL/Atmel/InterruptVectors.hpp"

#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, USART_RX_)


namespace InterruptVectorTest {

struct TypeWithHandler {
    typedef TypeWithHandler This;
    bool invoked = false;

    void onUSART_RX() {
        invoked = true;
    }

    INTERRUPT_HANDLER1(INTERRUPT_VECTOR(USART_RX), onUSART_RX);
};

TypeWithHandler testPin1;
TypeWithHandler testPin2;

mkISRS(testPin1, testPin2);

TEST(InterruptVectorTest, invoking_interrupt_handler_should_forward_to_method_and_can_chain) {
    USART_RX_vect();
    EXPECT_TRUE(testPin1.invoked);
    EXPECT_TRUE(testPin2.invoked);
}

}

