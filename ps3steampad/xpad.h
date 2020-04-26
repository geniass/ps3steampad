#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/synchronization.h>
#include <cell/pad.h>


#ifdef __cplusplus  
extern "C" {
#endif 

#define STEAM_CONTROLLER_NAME	"STEAM"
#define STEAM_CONTROLLER_VID	0x28de
#define STEAM_CONTROLLER_PID	0x1102

#define XPAD_DATA_LEN 14+2 // +2 for count and size fields
#define XPADW_DATA_LEN 0x13+2
#define MAX_XPAD_DATA_LEN XPADW_DATA_LEN
#define MAX_XPAD_NUM CELL_PAD_MAX_PORT_NUM

#define RINGBUF_SIZE  10

enum XTYPES {
	XTYPE_XBOX360 = 1,
	XTYPE_XBOX360W = 2
};


typedef struct {
	int32_t dev_id; /* Device id */
	int32_t number; /* Xpad number */
	int32_t c_pipe; /* Control pipe id */
	int32_t i_pipe; /* In pipe id */
	int32_t o_pipe; /* Out pipe id */
	int32_t payload; /* Size of payload */
	uint8_t ifnum; /* Interface number */
	uint8_t as; /* Alternate setting number */
	int32_t tcount; /* Transfer counts */
	uint8_t xtype;

	// methods to their respective controllers
	int32_t(*read_input)(int32_t dev_id, void *data);
	int32_t(*set_led)(int32_t dev_id, uint8_t led);
	int32_t(*set_rumble)(int32_t dev_id, uint8_t lval, uint8_t rval);

	/* Ring buffer */
	int32_t rp; /* Read pointer   */
	int32_t wp; /* Write pointer  */
	int32_t rblen; /* Buffer length  */
	unsigned char ringbuf[RINGBUF_SIZE][MAX_XPAD_DATA_LEN]; /* Ring buffer */

	/* Buffer for interrupt transfer */
	unsigned char data[0];

} XPAD_UNIT_t;


int32_t init_xpad_mutex();
int32_t destroy_xpad_mutex();
void block_xpad();
void unblock_xpad();

int32_t init_ringbuf_mutex();
int32_t destroy_ringbuf_mutex();
void block_ringbuf();
void unblock_ringbuf();

void unit_free(XPAD_UNIT_t *unit);
XPAD_UNIT_t *unit_alloc(int32_t dev_id, int32_t payload, uint8_t ifnum, uint8_t as, uint8_t xtype);

// wired Xbox 360 controller methods
int32_t xpad_probe(int32_t dev_id);
int32_t xpad_attach(int32_t dev_id);
int32_t xpad_detach(int32_t dev_id);
int32_t xpad_detach_all(void);
int32_t xpad_read_input(int32_t id, void *data);
void xpad_read_report(int32_t id, uint8_t *readBuf);
int32_t xpad_set_led(int32_t id, uint8_t led);
int32_t xpad_set_rumble(int32_t id, uint8_t lval, uint8_t rval);

#ifdef __cplusplus 
}
#endif 