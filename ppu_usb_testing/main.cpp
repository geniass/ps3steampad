/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 475.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <types.h>
#include <stdio.h>
#include <string.h>
#include <sys/timer.h>
#include <sys/process.h>
#include <cell/pad.h>       /*E Gamepad Library */
#include <sysutil/sysutil_sysparam.h>
#include <cell/sysmodule.h>

#include "../ps3steampad/usb.h"
#include "../ps3steampad/utils.h"

#define MAX_PAD 7

/*E Prototype */
static void sysutil_callback(uint64_t status, uint64_t param, void *userdata);

/*E  Variables */
static CellPadInfo2 PadInfo;/*E  connection Pad Information buffer */
static CellPadData PadData;/*E  Gamapad data buffer */
static volatile uint32_t is_exit_game = 0;
static CellPadActParam actprm[MAX_PAD];
static int frame = 0;

SYS_PROCESS_PARAM(1001, 0x10000);

/*E  This sample main function */
static int32_t run_Gamepad(void)
{
	uint32_t i = 0;
	int32_t j = 0;
	int32_t ret = 0;
	uint32_t old_info = 0;

	show_msg((char *)"SteamPad Loaded!");


	printf("Gamepad Sample\n");

	while (1) {

	}

	return (CELL_OK);
}

//static int32_t initializeModule(void)
//{
//	int32_t ret;
//
//	if (MAX_PAD>CELL_PAD_MAX_PORT_NUM)
//	{
//		printf("Invalid MAX_PAD value :%d\n", MAX_PAD);
//		return (CELL_OK);
//	}
//
//	ret = cellPadInit(MAX_PAD);
//	if (ret != CELL_OK){
//		printf("cellPadInit failed 0x%08x\n", ret);
//		return(ret);
//	}
//	return(ret);
//}
//
//static int32_t finalizeModule(void)
//{
//	int32_t ret;
//	ret = cellPadEnd();
//	if (ret != CELL_OK){
//		printf("cellPadEnd failed 0x%08x\n", ret);
//	}
//
//	return(ret);
//}


int32_t main(void)
{
	int32_t ret;

	/*E Register with sysutil, to be notified if this application needs to quit */
	if ((ret = cellSysutilRegisterCallback(0, (CellSysutilCallback)sysutil_callback, NULL)) != CELL_OK)
	{
		printf("ERROR : cellSysutilRegisterCallback() = 0x%x\n", ret);
		return(ret);
	}

	if ((ret = cellSysmoduleLoadModule(CELL_SYSMODULE_USBD)) != CELL_OK) {
		//Error handling
		printf("ERROR : cellSysmoduleLoadModule() = 0x%x\n", ret);
		return(ret);
	}

	if ((ret = init_usb()) < 0) {
		printf("ERROR: init_usb() = 0x%x\n", ret);
		return ret;
	}

	//ret = initializeModule();
	//if (ret != CELL_OK){
	//	printf("initializeModule failed 0x%08x", ret);
	//	return(ret);
	//}

	printf("SUCCESS: running main loop\n");

	run_Gamepad();

	//ret = finalizeModule();
	//if (ret != CELL_OK){
	//	printf("finalizeModule failed 0x%08x", ret);
	//	return(ret);
	//}

	/*E unregist sysutil callback function */
	ret = cellSysutilUnregisterCallback(0);
	if (ret != CELL_OK) {
		printf("ERROR : cellSysutilUnregisterCallback() = 0x%x\n", ret);
	}

	return (ret);
}

/*E sysutil callback function. registered by cellSysutilRegisterCallback() */
static void sysutil_callback(uint64_t status, uint64_t param, void *userdata)
{
	(void)param;
	(void)userdata;

	switch (status)
	{
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		is_exit_game = 1;
		break;
	default:
		break;
	}

	return;
}

