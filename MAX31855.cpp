// Maxim MAX31855 Library
// by Andy Gock

#include "MAX31855.h"

Max31855::Max31855(uint8_t pin) {
	_pin_select = pin;

	pinMode(_pin_select,OUTPUT);
	digitalWrite(_pin_select,1); // hold select line high

	// Set up SPI bus
	//SPI.setBitOrder(MSBFIRST);
	//SPI.setClockDivider(SPI_CLOCK_DIV4)
	//SPI.setDataMode(SPI_MODE0 )
	SPI.begin();

	Serial.begin(115200);
}

void Max31855::select(void) {
	digitalWrite(_pin_select,0);
}

void Max31855::deselect(void) {
	digitalWrite(_pin_select,1);
}

void Max31855::_read(void) {
	//uint8_t data[4];
	uint32_t data;
	uint8_t c;

	data = 0;

	select();
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
	_info.temperature = (float) ((data >> 18) & 0b1111111111111) / 4; // 1 sign + 13 data bits
	_info.temperature *= ( (data >> 31) ? -1 : 1 );

	// get cold /reference junction temperature
	_info.cold_junction_temperature = (float) ((data >> 4) & 0b11111111111) / 16; // 1 sign + 11 data bits
	_info.cold_junction_temperature *= ( (data >> 15) & 0x01 ? -1 : 1 );

	// get fault bits
	_info.error = MAX31855_NO_ERROR; // reset fault bits to zero (no fault)
	if ((data >> 16) & 0x01) {
		// fault bit (D16) is set
		if (data & 0b001) {
			// open circuit
			_info.error = MAX31855_OPEN_CIRCUIT;
		} else if (data & 0b010) {
			// short to GND
			_info.error = MAX31855_SHORT_GND;
		} else if (data & 0b100) {
			// short to VCC
			_info.error = MAX31855_SHORT_VCC;
		} else {
			// unknown fault
			_info.error = MAX31855_FAULT_UNKNOWN;
		}
	} else {
		_info.error = MAX31855_NO_ERROR;
	}

	
}

void Max31855::update(void) {
	_read();
}

Max31855_Info Max31855::get_info(void) {
	update();
	return _info;
}

float Max31855::get_temperature(void) {
	return _info.temperature;
}

float Max31855::get_cold_junction_temperature(void) {
	return _info.cold_junction_temperature;
}

uint8_t Max31855::get_error(void) {
	return _info.error;
}
