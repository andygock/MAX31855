// Maxim MAX31855 Library
// by Andy Gock

#ifndef MAX31855_H
#define MAX31855_H

//#define MAX31855_DEBUG

#if ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <wiring.h>
	#include "pins_arduino.h"
#endif

#include <SPI.h>

// Max31855_Info error byte codes
#define MAX31855_NO_ERROR      0
#define MAX31855_OPEN_CIRCUIT  1
#define MAX31855_SHORT_GND     2
#define MAX31855_SHORT_VCC     3
#define MAX31855_FAULT_UNKNOWN 4

typedef struct {
	float   temperature;
	float   cold_junction_temperature;
	uint8_t error;
} Max31855_Info;

class Max31855 {
	private:
		uint8_t       _pin_select;
		uint32_t      _data;
		Max31855_Info _info;

		/** Read 4 bytes from SPI and store temp and error
		 *  info into _info structure.
		 */
		void _read(void);

	public:
		/** Constructor, initialise SPI on selected pin */
		Max31855(uint8_t pin);

		/** Select chip (bring CS pin low) */
		void select(void);

		/** Deselect chip (bring CS pin high) */
		void deselect(void);

		/** Performs a _read() */
		void update(void);

		/** Returns _info */
		Max31855_Info get_info(void);

		/** Return hot junction temperature from _info */
		float get_temperature(void);

		/** Return cold junction temperature from _info */
		float get_cold_junction_temperature(void);

		/** Return eror code from _info */
		uint8_t get_error(void);
};

#endif
