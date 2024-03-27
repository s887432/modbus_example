#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <modbus/modbus.h>

#define MODBUS_PORT	1503
#define MODBUS_CONNECTIONS	5

#define MODBUS_DATA_POSITION_LED_RED	0
#define MODBUS_DATA_POSITION_LED_GREEN	1
#define MODBUS_DATA_POSITION_LED_BLUE	2

int modbusSendData(modbus_t *ctx, int16_t *regs, int nb)
{
	int rc;
	
	rc = modbus_write_registers(ctx, 1, nb, regs);
	if (rc != nb) 
	{
		printf("ERROR modbus_write_registers (%d)\n", rc);
		printf("Address = %d, nb = %d\n", 1, nb);
		return -1;
	} 

	rc = modbus_read_registers(ctx, 1, nb, regs);
	if (rc != nb)
	{
		printf("ERROR modbus_read_registers (%d)\n", rc);
		return -1;
	} 

	return 0;
}

int main(int argc, char **argv)
{
	int i, j;
	modbus_t *ctx;
	int16_t regs[10];
	int nb = sizeof(regs)/sizeof(int16_t);

	if( argc != 2 )
	{
		printf("USAGE: simple_client_02 SERVERIP\r\n");
		return -1;
	}
	
	memset(regs, 0, sizeof(regs));
	
	// connect to server
	ctx = modbus_new_tcp(argv[1], MODBUS_PORT); 
	if (modbus_connect(ctx) == -1) {
		fprintf(stderr,"Connection failed: %s\n",modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	modbus_set_debug(ctx, FALSE);

	// write regs
	regs[MODBUS_DATA_POSITION_LED_RED] = 0;
	regs[MODBUS_DATA_POSITION_LED_GREEN] = 0;
	regs[MODBUS_DATA_POSITION_LED_BLUE] = 0;
	modbusSendData(ctx, regs, nb);
	
	regs[MODBUS_DATA_POSITION_LED_RED] = 1;
	regs[MODBUS_DATA_POSITION_LED_GREEN] = 1;
	regs[MODBUS_DATA_POSITION_LED_BLUE] = 1;
	modbusSendData(ctx, regs, nb);

	return 0;
}

// end of file

