#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <modbus/modbus.h>
#include "modbus_ex.h"

#define LED_RED_DEV			"/sys/class/leds/red/brightness"
#define LED_GREEN_DEV		"/sys/class/leds/green/brightness"
#define LED_BLUE_DEV		"/sys/class/leds/blue/brightness"

static int gLedRedfd = -1;
static int gLedGreenfd = -1;
static int gLedBluefd = -1;

int ledInit(void)
{
	if( (gLedRedfd = open(LED_RED_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_RED_DEV);
		return -1;
	}
	
	if( (gLedGreenfd = open(LED_GREEN_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_GREEN_DEV);
		return -1;
	}
	
	if( (gLedBluefd = open(LED_BLUE_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_BLUE_DEV);
		return -1;
	}
	
	return 0;
}

void ledClose(void)
{
	if( gLedRedfd >= 0 )
		close(gLedRedfd);
	if( gLedGreenfd >= 0 )
		close(gLedGreenfd);
	if( gLedBluefd >= 0 )
		close(gLedBluefd);
}

int ledSet(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "255", 3);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "255", 3);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "255", 3);
			break;
		default:
			return -1;
	}
	
	return 0;
}

int ledClean(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "0", 1);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "0", 1);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "0", 1);
			break;
		default:
			return -1;
	}
	
	return 0;
}

int main(int argc, char*argv[])
{
    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int i;
    uint8_t *query;
    int header_length;
    uint16_t addr;
    uint16_t value;
    pWriteSingleBit_t writeSingleBitValue;
    uint8_t *ptrPackage;
    
    ledInit();

    ctx = modbus_new_tcp(NULL, 1502);
    query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    header_length = modbus_get_header_length(ctx);
    
    printf("MODBUS_TCP_MAX_ADU_LENGTH = %d\r\n", MODBUS_TCP_MAX_ADU_LENGTH);
    printf("header_length = %d\r\n", header_length);

    //modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(
        UT_BITS_ADDRESS + UT_BITS_NB,
        UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
        UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
        UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /** INPUT STATUS **/
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                               UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
                               UT_INPUT_BITS_TAB);

    /** INPUT REGISTERS **/
    for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
        mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
            UT_INPUT_REGISTERS_TAB[i];;
    }

    s = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &s);

    for (;;) {
        do {
            rc = modbus_receive(ctx, query);
            /* Filtered queries return 0 */
        } while (rc == 0);

        if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }

		ptrPackage = query+header_length;
		
		switch( *ptrPackage )
		{
			case 5:	
				printf("*******************************************************************\r\n");
				// write single coil
				writeSingleBitValue = (pWriteSingleBit_t)(ptrPackage+1);				
				printf("%d, %s\r\n", writeSingleBitValue->address, writeSingleBitValue->value==0xFF?"ON":"OFF");
				
				writeSingleBitValue->value==0xFF ? ledSet(LED_RED) : ledClean(LED_RED);
				
				break;
		}
		
        rc = modbus_reply(ctx, query, rc, mb_mapping);
        if (rc == -1) {
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (s != -1) {
       close(s);
    }
    modbus_mapping_free(mb_mapping);
    free(query);
    /* For RTU */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

// end of file
