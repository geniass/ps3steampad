#include "xpad.h"

#include <stdlib.h>
#include <string.h>
#include <sys/synchronization.h>
#include <sys/ppu_thread.h>

#include "utils.h"


// Sync
sys_mutex_t xpad_mutex, ringbuf_mutex;


static void block(sys_mutex_t mutex) {
	int32_t r;

	if ((r = sys_mutex_lock(mutex, 0)) != CELL_OK) {
		ppu_thread_exit();
	}
}

static void unblock(sys_mutex_t mutex) {
	int32_t r;

	if ((r = sys_mutex_unlock(mutex)) != CELL_OK) {
		ppu_thread_exit();
	}
}


int32_t init_xpad_mutex() {
	int r;
	sys_mutex_attribute_t mutex_attr;

	sys_mutex_attribute_initialize(mutex_attr);
	return sys_mutex_create(&xpad_mutex, &mutex_attr);
}
int32_t destroy_xpad_mutex() {
	return sys_mutex_destroy(xpad_mutex);
}
void block_xpad() {
	block(xpad_mutex);
}
void unblock_xpad() {
	unblock(xpad_mutex);
}

int32_t init_ringbuf_mutex() {
	int r;
	sys_mutex_attribute_t mutex_attr;

	sys_mutex_attribute_initialize(mutex_attr);
	return sys_mutex_create(&ringbuf_mutex, &mutex_attr);
}
int32_t destroy_ringbuf_mutex() {
	return sys_mutex_destroy(ringbuf_mutex);
}
void block_ringbuf() {
	block(ringbuf_mutex);
}
void unblock_ringbuf() {
	unblock(ringbuf_mutex);
}


void unit_free(XPAD_UNIT_t *unit) {
	if (unit) {
		free(unit);
	}
}

XPAD_UNIT_t *unit_alloc(int32_t dev_id, int32_t payload, uint8_t ifnum, uint8_t as, uint8_t xtype) {
	XPAD_UNIT_t *unit;
	int32_t i;
	if ((unit = (XPAD_UNIT_t *)malloc(sizeof(XPAD_UNIT_t) + payload)) != NULL) {
		memset(unit, 0, sizeof(XPAD_UNIT_t));
		unit->dev_id = dev_id;
		unit->payload = payload;
		unit->ifnum = ifnum;
		unit->as = as;
		unit->tcount = 0;
		unit->rp = 0;
		unit->wp = 0;
		unit->rblen = 0;
		unit->xtype = xtype;
		if (xtype == XTYPE_XBOX360) {
			unit->read_input = xpad_read_input;
			unit->set_led = xpad_set_led;
			unit->set_rumble = xpad_set_rumble;
		}

		// TODO: WIRELESS
		//else if (xtype == XTYPE_XBOX360W) {
		//	unit->read_input = xpadw_read_input;
		//	unit->set_led = xpadw_set_led;
		//	unit->set_rumble = xpadw_set_rumble;
		//}

		//block(xpad_mutex);
		//unit->number = XPAD.next_number;
		//for (i = 0; i < MAX_XPAD_NUM; i++) {
		//	++XPAD.next_number;
		//	if (XPAD.next_number >= MAX_XPAD_NUM) {
		//		XPAD.next_number = 0;
		//	}
		//	if (XPAD.con_unit[XPAD.next_number] == NULL) {
		//		break;
		//	}
		//}
		//unblock(xpad_mutex);
		//if (i >= MAX_XPAD_NUM) {
		//	free(unit);
		//	return(NULL);
		//}
	}
	return(unit);
}

int32_t xpad_read_input(int32_t id, void *data) {
	unsigned char *p;
	int32_t i;
	unsigned char *xpadbuf;
	XPAD_UNIT_t *unit;

	//p = (unsigned char *)data;
	//if (id > MAX_XPAD_NUM) {
	//	return(-1);
	//}
	//if ((unit = XPAD.con_unit[id]) == NULL) {
	//	return(-1);
	//}

	//// get from ringbuffer
	//block(ringbuf_mutex);
	//if (unit->rblen > 0) {
	//	xpadbuf = &unit->ringbuf[unit->rp][0];
	//	*p++ = xpadbuf[0]; // count
	//	*p++ = xpadbuf[1]; // size
	//	memcpy(p, &xpadbuf[2], unit->payload);
	//	if (++unit->rp >= RINGBUF_SIZE) {
	//		unit->rp = 0;
	//	}
	//	XBOX360_IN_REPORT *report = (XBOX360_IN_REPORT *)p;
	//	if ((report->header.command == inReport) && (report->header.size == sizeof(XBOX360_IN_REPORT))) {
	//		xpad_read_report(unit->number, p);
	//	}
	//	unit->rblen--;
	//}
	//else {
	//	*p++ = 0; // count
	//	memset(p, 0, unit->payload);
	//}
	//unblock(ringbuf_mutex);
	return(0);
}

static int32_t xpad_set_led(int32_t id, uint8_t led) {
	//uint8_t out[3] = { 0x01, 0x03, led };

	//if (write_xpad(id, out, 3) < 0) {
	//	return(-1);
	//}
	return(CELL_OK);
}

static int32_t xpad_set_rumble(int32_t id, uint8_t lval, uint8_t rval) {
	//uint8_t out[8] = { 0x00, 0x08, 0x00, lval, rval, 0x00, 0x00, 0x00 };

	//if (write_xpad(id, out, 8) < 0) {
	//	return(-1);
	//}
	return(CELL_OK);
}