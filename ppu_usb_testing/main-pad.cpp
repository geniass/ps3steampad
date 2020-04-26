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

	for (i = 0; i<MAX_PAD; i++)
	{
		memset(&actprm[i], 0x00, sizeof(CellPadActParam));
		cellPadSetPortSetting(i, CELL_PAD_SETTING_PRESS_ON | CELL_PAD_SETTING_SENSOR_ON);
	}

	printf("Gamepad Sample\n");

	/*E ----- Main loop -----*/
	while (1)
	{
		if ((ret = cellPadGetInfo2(&PadInfo)) != CELL_OK)
		{
			printf("Error(%08X) : cellPadGetPadInfo2\n", ret);
			continue;
		}

		/*E Check info field for monitoring the INTERCEPTED state. */
		if ((PadInfo.system_info & CELL_PAD_INFO_INTERCEPTED) &&
			(!(old_info & CELL_PAD_INFO_INTERCEPTED))) {
			printf("This program lost the ownership of the game pad data\n");
			old_info = PadInfo.system_info;
		}
		else if ((!(PadInfo.system_info & CELL_PAD_INFO_INTERCEPTED)) &&
			(old_info & CELL_PAD_INFO_INTERCEPTED)){
			printf("This program got the ownership of the game pad data\n");
			old_info = PadInfo.system_info;
		}

		for (i = 0; i < MAX_PAD; i++)
		{
			if (PadInfo.port_status[i] & CELL_PAD_STATUS_ASSIGN_CHANGES)
			{
				if ((PadInfo.port_status[i] & CELL_PAD_STATUS_CONNECTED) == 0)
				{
					printf("gamepad %d removal\n", i);
					continue;
				}

				/*E  Gamepad i has been connected */
				if ((PadInfo.port_status[i] & CELL_PAD_STATUS_CONNECTED) > 0)
				{
					/*E  the previous data shows that gamepad was not
					*E  yet connected, so it is a new connection
					*/
					printf("New gamepad [%d] is connected\n", i);
					printf("   Capability  :%08x\n", PadInfo.device_capability[i]);
					if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_PS3_CONFORMITY)
						printf("   + PS3_CONFORMITY\n");
					if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_PRESS_MODE)
						printf("   + PRESS_MODE\n");
					if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_SENSOR_MODE)
						printf("   + SENSOR_MODE\n");
					if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_HP_ANALOG_STICK)
						printf("   + HP_ANALOG_STICK\n");
					if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_ACTUATOR)
						printf("   + ACTUATOR\n");
					printf("   Port Setting:%08x\n", PadInfo.port_setting[i]);
					if (PadInfo.port_setting[i] & CELL_PAD_SETTING_PRESS_ON)
						printf("   + PRESS_MODE ON\n");
					if (PadInfo.port_setting[i] & CELL_PAD_SETTING_SENSOR_ON)
						printf("   + SENSOR_MODE ON\n");
					printf("   Device Type :%08x\n", PadInfo.device_type[i]);
				}
			}

			if ((PadInfo.port_status[i] & CELL_PAD_STATUS_CONNECTED) == 0)
			{
				continue;
			}

			ret = cellPadGetData(i, &PadData);

			if (0 != ret) {
				continue;
			}

			if (PadData.len > 0)
			{
#if 0
				/* all data dump */
				printf("[%d] DATA:", i);
				for (j = 0; j < PadData.len; j++)
					printf("%02x ", PadData.button[j]);
				printf("\n");
#else
				printf("[%d] DATA:", i);
				/* controler id */
				printf("[");
				for (j = 0; j < 2; j++)
					printf("%02x", PadData.button[j]);
				printf("]");
				/* digital button */
				printf("[");
				for (j = 2; j < 4; j++)
					printf("%02x", PadData.button[j]);
				printf("]");
				/* analog stick */
				printf("[");
				for (j = 4; j < 8; j++)
					printf("%02x", PadData.button[j]);
				printf("]");
				/* press button data */
				if (PadData.len>8)
				{
					printf("[");
					for (j = 8; j < 20; j++)
						printf("%02x", PadData.button[j]);
					printf("]");
				}
				/* sensor data */
				if (PadData.len>20)
				{
					printf("[");
					for (j = 20; j < 24; j++)
						printf("%03x", PadData.button[j]);
					printf("]");
				}
				printf("\n");
#endif
				if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_ACTUATOR)
				{
					if (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1)
					{
						actprm[i].motor[0] = 1;
					}
					else actprm[i].motor[0] = 0;

					if (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2)
					{
						if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_PRESS_MODE)
						{
							actprm[i].motor[1] = PadData.button[CELL_PAD_BTN_OFFSET_PRESS_R2];
						}
						else {
							actprm[i].motor[1] = 128;
						}
					}
					else actprm[i].motor[1] = 0;
				}
			}

			if (PadInfo.device_capability[i] & CELL_PAD_CAPABILITY_ACTUATOR)
			{
				cellPadSetActDirect(i, &actprm[i]);
			}
		}

		/* E Poll sysutil to see if it has any callbacks to process */
		cellSysutilCheckCallback();
		if (is_exit_game)
		{
			break;
		}

		/*E  wait for the vsync, or next interval */
		sys_timer_usleep(1000 * 10);
		frame++;

	} /*E end of Main loop */
	return (CELL_OK);
}

static int32_t initializeModule(void)
{
	int32_t ret;

	if (MAX_PAD>CELL_PAD_MAX_PORT_NUM)
	{
		printf("Invalid MAX_PAD value :%d\n", MAX_PAD);
		return (CELL_OK);
	}

	ret = cellPadInit(MAX_PAD);
	if (ret != CELL_OK){
		printf("cellPadInit failed 0x%08x\n", ret);
		return(ret);
	}
	return(ret);
}

static int32_t finalizeModule(void)
{
	int32_t ret;
	ret = cellPadEnd();
	if (ret != CELL_OK){
		printf("cellPadEnd failed 0x%08x\n", ret);
	}

	return(ret);
}

/* ---------------------------------------------
Function Name : main
function      : main function
Input Data    : none
Output Data   : none
Return Value  : none
----------------------------------------------*/
int32_t main(void)
{
	int32_t ret;

	/*E Register with sysutil, to be notified if this application needs to quit */
	if ((ret = cellSysutilRegisterCallback(0, (CellSysutilCallback)sysutil_callback, NULL)) != CELL_OK)
	{
		printf("ERROR : cellSysutilRegisterCallback() = 0x%x\n", ret);
		return(ret);
	}

	ret = initializeModule();
	if (ret != CELL_OK){
		printf("initializeModule failed 0x%08x", ret);
		return(ret);
	}

	run_Gamepad();

	ret = finalizeModule();
	if (ret != CELL_OK){
		printf("finalizeModule failed 0x%08x", ret);
		return(ret);
	}

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

