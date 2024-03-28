#ifndef __MODBUS_EX_H__
#define __MODBUS_EX_H__

#include <modbus/modbus.h>

#define MODBUS_CMD_WRITE_SIGNLE_BIT		5

typedef struct __WRITE_SINGLE_BIT_STR__
{
	uint16_t address;
	uint8_t value;
}WriteSingleBit_t, *pWriteSingleBit_t;


// device/application depends
const uint16_t UT_BITS_ADDRESS = 0;
const uint16_t UT_BITS_NB = 10;
const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B, 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };

const uint16_t UT_INPUT_BITS_ADDRESS = 0x10;
const uint16_t UT_INPUT_BITS_NB = 3;
const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_REGISTERS_ADDRESS = 0x20;
const uint16_t UT_REGISTERS_NB = 3;
const uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x30;
const uint16_t UT_INPUT_REGISTERS_NB = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

typedef enum __POS_ADDR__
{
	POS_LED_RED = 0,
	POS_LED_GREEN,
	POS_LED_BLUE,
	POS_KEY_RED,
	POS_KEY_GREEN,
	POS_KEY_BLUE
}POS_ADDR;

typedef enum __BIT_VALUE__
{
	VALUE_ON = ON,
	VALUE_OFF = OFF
} BIT_VALUE;

typedef enum __LED_ID__
{
	LED_RED = 1,
	LED_GREEN,
	LED_BLUE
}LED_ID;

#endif // end of __MODBUS_EX_H__

// end of file
