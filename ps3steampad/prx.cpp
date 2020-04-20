#include "stdafx.h"

#include <cellstatus.h>
#include <sys/prx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

SYS_MODULE_INFO( ps3steampad, 0, 1, 1);
SYS_MODULE_START( _ps3steampad_prx_entry );

SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );
SYS_LIB_EXPORT( _ps3steampad_export_function, LIBNAME );

#define THREAD_NAME "xpaddt"
#define STOP_THREAD_NAME "xpadds"


static sys_ppu_thread_t thread_id = 1;
static volatile uint8_t running;

// function declarations
static void xpadd_stop_thread(uint64_t arg);
static void xpadd_thread(uint64_t arg);



void ppu_thread_exit()
{
	system_call_1(41, 0); //ppu_thread_exit
}


// An exported function is needed to generate the project's PRX stub export library
extern "C" int _ps3steampad_export_function(void)
{
    return CELL_OK;
}

extern "C" int _ps3steampad_prx_entry(void)
{
	system_call_3(392, 0x1004, 0xa, 0x1b6);

	//show_msg((char*) "WAT!!!");

	// start a background thread and exit
	//sys_ppu_thread_create(&thread_id, xpadd_thread, NULL, -0x1d8, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME);

	// for some reason the built-in sys_ppu_thread_exit freezes the console
	//sys_ppu_thread_exit(0);
	ppu_thread_exit();

	return SYS_PRX_RESIDENT;
}

extern "C" int _PS3_PPU_Project1_prx_exit(void)
{
	//sys_ppu_thread_t t;
	//uint64_t exit_code;

	//sys_ppu_thread_create(&t, xpadd_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
	//sys_ppu_thread_join(t, &exit_code);
	//sys_ppu_thread_exit(0);
	return(SYS_PRX_STOP_OK);
}

//// called when xpad must shutdown
//static void xpadd_stop_thread(uint64_t arg) {
//	uint64_t exit_code;
//
//	running = 0;
//	sys_timer_usleep(500000);
//	if (thread_id != (sys_ppu_thread_t)-1) {
//		sys_ppu_thread_join(thread_id, &exit_code);
//	}
//	show_msg("XPAD Unloaded!");
//	sys_ppu_thread_exit(0);
//}
//
//static void xpadd_thread(uint64_t arg) {
//	int32_t i, r;
//	/*unsigned char xpad_data[MAX_XPAD_DATA_LEN];
//	XPAD_UNIT_t *unit;
//
//	r = init_usb();
//	if (r < 0) {
//		sys_ppu_thread_exit(0);
//	}*/
//
//	// wait until we're back in xmb
//	sys_timer_sleep(10);
//	show_msg((char *)"XPAD Loaded!");
//	running = 1;
//	//while (running) {
//	//	sys_timer_usleep(1000 * 10);
//	//	block(xpad_mutex);
//	//	for (i = 0; i < MAX_XPAD_NUM; i++) {
//	//		if (XPAD.is_connected[i] > 0) {
//	//			unit = XPAD.con_unit[i];
//	//			unit->read_input(i, xpad_data);
//	//		}
//	//	}
//	//	check_pad_status();
//	//	unblock(xpad_mutex);
//	//}
//
//	//// exiting...
//	//xpad_detach_all();
//	//xpadw_detach_all();
//	//shutdown_usb();
//	sys_ppu_thread_exit(0);
//}
