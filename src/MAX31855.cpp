// Maxim MAX31855 Library
// by Andy Gock

#include "MAX31855.h"

Max31855::Max31855(uint8_t pin) {
	Max31855(pin, 1);
}

Max31855::Max31855(uint8_t pin, uint8_t samples) {
	this->_pin_select = pin;
	this->_samples = samples;
	
	pinMode(this->_pin_select, OUTPUT);
	digitalWrite(this->_pin_select, 1); // hold select line high

	// Set up SPI bus
	//SPI.setBitOrder(MSBFIRST);
	//SPI.setClockDivider(SPI_CLOCK_DIV4)
	//SPI.setDataMode(SPI_MODE0 )
	SPI.begin();

	Serial.begin(115200);
}


void Max31855::select(void) {
	digitalWrite(this->_pin_select, 0);
}

void Max31855::deselect(void) {
	digitalWrite(this->_pin_select, 1);
}

void Max31855::_read(uint8_t samples) {
	uint8_t n;
	float sum_temperature = 0;
	float sum_cold_junction = 0;

	for (n = 1; n < samples; n++) {
		this->_read();
		sum_temperature += this->_info.temperature;
		sum_cold_junction += this->_info.cold_junction_temperature;
	}

	this->_info.temperature = sum_temperature / samples;
	this->_info.cold_junction_temperature = sum_cold_junction / samples;

#if 0
	Serial.print(F("sum_temperature: "));
	Serial.println(sum_temperature);
#endif

}

void Max31855::_read(void) {
	uint32_t data = 0;
	uint8_t c;

	this->select();
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: Selected\n"));
#endif	

	c = SPI.transfer(0x00);
	data += (uint32_t) c << 24;
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: SPI Read Byte 0 => "));
	Serial.println(c,BIN);
#endif

	c = SPI.transfer(0x00);
	data += (uint32_t) c << 16;
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: SPI Read Byte 1 => "));
	Serial.println(c,BIN);
#endif

	c = SPI.transfer(0x00);
	data += (uint32_t) c << 8;
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: SPI Read Byte 2 => "));
	Serial.println(c,BIN);
#endif

	c = SPI.transfer(0x00);
	data += (uint32_t) c;
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: SPI Read Byte 3 => "));
	Serial.println(c,BIN);
#endif

	deselect();
#if defined(MAX31855_DEBUG)
	Serial.print(F("MAX31855: Deselected\n"));
	Serial.print(F("MAX31855: 32-bit data = "));
	Serial.println(data, HEX);
#endif	

	// get temperature
	this->_info.temperature = (float) ((data >> 18) & 0b1111111111111) / 4; // 1 sign + 13 data bits
	this->_info.temperature *= ( (data >> 31) ? -1 : 1 );

	// get cold /reference junction temperature
	this->_info.cold_junction_temperature = (float) ((data >> 4) & 0b11111111111) / 16; // 1 sign + 11 data bits
	this->_info.cold_junction_temperature *= ( (data >> 15) & 0x01 ? -1 : 1 );

	// get fault bits
	_info.error = MAX31855_NO_ERROR; // reset fault bits to zero (no fault)
	if ((data >> 16) & 0x01) {
		// fault bit (D16) is set
		if (data & 0b001) {
			// open circuit
			this->_info.error = MAX31855_OPEN_CIRCUIT;
		} else if (data & 0b010) {
			// short to GND
			this->_info.error = MAX31855_SHORT_GND;
		} else if (data & 0b100) {
			// short to VCC
			this->_info.error = MAX31855_SHORT_VCC;
		} else {
			// unknown fault
			this->_info.error = MAX31855_FAULT_UNKNOWN;
		}
	} else {
		this->_info.error = MAX31855_NO_ERROR;
	}
	
}

void Max31855::update(void) {
	this->_read(this->_samples);
}

void Max31855::dummy_read(uint8_t count) {
	uint8_t n;
	for (n = 0; n < count; n++) {
		this->_read();
	}
}

void Max31855::set_oversampling(uint16_t samples) {
	this->_samples = (samples - 1) & 0xff;
}

Max31855_Info Max31855::get_info(void) {
	update();
	return this->_info;
}

float Max31855::get_temperature(void) {
	return this->_info.temperature;
}

float Max31855::get_cold_junction_temperature(void) {
	return this->_info.cold_junction_temperature;
}

uint8_t Max31855::get_error(void) {
	return this->_info.error;
}
