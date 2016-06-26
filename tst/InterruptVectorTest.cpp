#include "avr/common.h"
#include "HAL/Atmel/InterruptHandlers.hpp"
#include "HAL/Atmel/Device.hpp"
#include <gtest/gtest.h>

#define __mk_ALL_ISRS \
    FOR_EACH(__mkISR, USART_RX_, USART_UDRE_, INT0_)

namespace InterruptVectorTest {

using namespace HAL::Atmel;
using namespace InterruptHandlers;

struct TypeWithHandler {
    typedef TypeWithHandler This;
    bool invoked = false;

    void onUSART_RX() {
        invoked = true;
    }

    typedef On<This, Int_USART_RX_, &This::onUSART_RX> Handlers;
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

    typedef On<This, Int_USART_RX_, &This::onUSART_RX,
            On<This, Int_USART_UDRE_, &This::onUSART_UDRE>> Handlers;
};

struct WrappingVector {
    typedef Int_INT0_ INT;
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

    typedef On<This, WrappingVector, &This::onINT0> Handlers;
};

struct TheApp {
    TypeWithHandler testPin1;
    TypeWithHandler testPin2;
    TypeWithTwoHandlers testPin3;
    TypeWithWrappedVector testWrapped;

    typedef TheApp This;
    typedef Delegate<This,TypeWithHandler,&This::testPin1,
            Delegate<This,TypeWithHandler,&This::testPin2,
            Delegate<This,TypeWithTwoHandlers,&This::testPin3,
            Delegate<This,TypeWithWrappedVector,&This::testWrapped>>>> Handlers;
};

TheApp app;
mkISRS

TEST(InterruptVectorTest, invoking_interrupt_handler_should_forward_to_method_and_can_chain) {
    app.testPin1.invoked = false;
    app.testPin2.invoked = false;
    USART_RX_vect();
    EXPECT_TRUE(app.testPin1.invoked);
    EXPECT_TRUE(app.testPin2.invoked);
}

TEST(InterruptVectorTest, two_different_interrupt_handlers_can_be_registered_and_invoked) {
    app.testPin3.invokedRX = false;
    app.testPin3.invokedUDRE = false;
    USART_RX_vect();
    EXPECT_TRUE(app.testPin3.invokedRX);
    EXPECT_FALSE(app.testPin3.invokedUDRE);

    app.testPin3.invokedRX = false;
    USART_UDRE_vect();
    EXPECT_FALSE(app.testPin3.invokedRX);
    EXPECT_TRUE(app.testPin3.invokedUDRE);
}

TEST(InterruptVectorTest, wrapping_vector_is_recognized_and_applied) {
    WrappingVector::invoked = false;
    app.testWrapped.invoked = false;
    INT0_vect();
    EXPECT_TRUE(WrappingVector::invoked);
    EXPECT_TRUE(app.testWrapped.invoked);
}

}

