
#include "GardenSensor/GardenSensor.hpp"
#include <gtest/gtest.h>
#include "EEPROMTest.hpp"
#include "invoke.hpp"

namespace GardenSensorTest {

using namespace Time;

TEST(GardenSensorTest, should_measure) {
	// INT_PIN PinPD2 pull high
	PIND |= _BV(2);
	// Simulate USART TX is ready
	UCSR0A |= _BV(TXC0);

	GardenSensor::GardenSensor<&EEPROM::bandgapVoltage, &EEPROM::data> sensor;

    onSleep_cpu = [&] {
        // when we go to sleep, we pretend enough time has passed to read out our sensors
    	constexpr int counts = toCountsOn<decltype(sensor.rt)>(1000_ms).getValue();
    	for (int i = 0; i < counts; i++) {
    		invoke<decltype(sensor.timer1)::INT>(sensor.rt);
    	}
    };

	sensor.loop();
}

}
