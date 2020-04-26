#include "usb.h"

#include <stdlib.h>
#include <stdio.h>

#include <sys/synchronization.h>
#include <cell/pad.h>
#include <cell/usbd.h>

#include "utils.h"
#include "xpad.h"


#define SWAP16(x) ((uint16_t)((((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8)))

// synchronization
//static sys_mutex_t xpad_mutex, ringbuf_mutex;

// function signatures for USB LDD callbacks
static int32_t xpad_probe(int32_t dev_id);
static int32_t xpad_attach(int32_t dev_id);
static int32_t xpad_detach(int32_t dev_id);
static CellUsbdLddOps xpad_ops = {
	0,
	xpad_probe,
	xpad_attach,
	xpad_detach
};

int32_t init_usb() {
	int32_t r, i;

	if ((r = init_ringbuf_mutex()) != CELL_OK) {
		return r;
	}
	if ((r = init_xpad_mutex()) != CELL_OK) {
		return r;
	}
	//sys_mutex_attribute_t mutex_attr1;
	//sys_mutex_attribute_t mutex_attr2;

	//sys_mutex_attribute_initialize(mutex_attr1);
	//sys_mutex_attribute_initialize(mutex_attr2);
	//if ((r = sys_mutex_create(&ringbuf_mutex, &mutex_attr1)) != CELL_OK) {
	//	return(r);
	//}
	//if ((r = sys_mutex_create(&xpad_mutex, &mutex_attr2)) != CELL_OK) {
	//	return(r);
	//}

	if ((r = cellUsbdInit()) != CELL_OK) {
		printf("ERROR: cellUsbdInit(): %d\n", r);
		return(r);
	}
	
	//// initialize all controller handlers
	//memset(handle, -1, sizeof(int32_t) * CELL_PAD_MAX_PORT_NUM);
	//memset(&XPAD, 0, sizeof(XPAD));

	// register wired Steam controller
	// TEST
	xpad_ops.name = STEAM_CONTROLLER_NAME;
	if ((r = cellUsbdRegisterExtraLdd(&xpad_ops, STEAM_CONTROLLER_VID, STEAM_CONTROLLER_PID)) != CELL_OK) {
		printf("ERROR REGISTER: %d\n", r);
		return(r);
	}

	//// register wireless Xbox controller device types
	//for (i = 0; i < MAX_XPADW_DEV_NUM; i++) {
	//	xpadw_ops.name = xpadw_info[i].name;
	//	if ((r = cellUsbdRegisterExtraLdd(&xpadw_ops, xpadw_info[i].vid, xpadw_info[i].pid)) != CELL_OK) {
	//		return(r);
	//	}
	//}

	show_msg("Success");

	return(CELL_OK);
}

int32_t shutdown_usb() {
	int32_t r;

	if ((r = cellUsbdUnregisterExtraLdd(&xpad_ops)) != CELL_OK) {
		return(r);
	}
	// TODO: wireless controller
	//if ((r = cellUsbdUnregisterExtraLdd(&xpadw_ops)) != CELL_OK) {
	//	return(r);
	//}

	destroy_ringbuf_mutex();
	destroy_xpad_mutex();
	//if ((r = sys_mutex_destroy(ringbuf_mutex)) != CELL_OK) {
	//	return(r);
	//}
	//if ((r = sys_mutex_destroy(xpad_mutex)) != CELL_OK) {
	//	return(r);
	//}

	return(CELL_OK);
}

/* 
  Wired controller functions 
*/
static int32_t xpad_probe(int32_t dev_id) {
	uint16_t idVendor, idProduct;
	uint32_t i;
	UsbDeviceDescriptor *ddesc;
	UsbInterfaceDescriptor *idesc;

	printf("HELLO!\n");

	//block(xpad_mutex);
	//if (XPAD.n >= MAX_XPAD_NUM) {
	//	unblock(xpad_mutex);
	//	return(CELL_USBD_PROBE_FAILED);
	//}
	//unblock(xpad_mutex);

	// get device descriptor
	if ((ddesc = (UsbDeviceDescriptor *)cellUsbdScanStaticDescriptor(dev_id, NULL, USB_DESCRIPTOR_TYPE_DEVICE)) == NULL) {
		print_error("cellUsbdScanStaticDescriptor", 0);
		return(CELL_USBD_PROBE_FAILED);
	}
	// get interface descriptor
	idesc = (UsbInterfaceDescriptor *)ddesc;
	if ((idesc = (UsbInterfaceDescriptor *)cellUsbdScanStaticDescriptor(dev_id, idesc, USB_DESCRIPTOR_TYPE_INTERFACE)) == NULL) {
		print_error("cellUsbdScanStaticDescriptor (interface)", 0);
		return(CELL_USBD_PROBE_FAILED);
	}

	// check that product id and vendor id are in the table of known devices
	idVendor = SWAP16(ddesc->idVendor);
	idProduct = SWAP16(ddesc->idProduct);
	printf("VID=%X PID=%X\n", idVendor, idProduct);
	return(CELL_USBD_PROBE_SUCCEEDED);
	//for (i = 0; i < MAX_XPAD_DEV_NUM; i++) {
	//	if (xpad_info[i].vid == idVendor && xpad_info[i].pid == idProduct) {
	//		return(CELL_USBD_PROBE_SUCCEEDED);
	//	}
	//}

	//return(CELL_USBD_PROBE_FAILED);
}


static int32_t xpad_attach(int32_t dev_id) {
	int32_t payload, port;
	uint32_t mode, port_setting;
	UsbConfigurationDescriptor *cdesc;
	UsbInterfaceDescriptor *idesc;
	UsbEndpointDescriptor *edesc;
	XPAD_UNIT_t *unit;

	if ((cdesc = (UsbConfigurationDescriptor *)cellUsbdScanStaticDescriptor(dev_id, NULL, USB_DESCRIPTOR_TYPE_CONFIGURATION)) == NULL) {
		print_error("attach: cellUsbdScanStaticDescriptor", 0);
		return (CELL_USBD_ATTACH_FAILED);
	}

	idesc = (UsbInterfaceDescriptor *)cdesc;
	if ((idesc = (UsbInterfaceDescriptor *)cellUsbdScanStaticDescriptor(dev_id, idesc, USB_DESCRIPTOR_TYPE_INTERFACE)) == NULL) {
		print_error("attach: cellUsbdScanStaticDescriptor (interface)", 0);
		return(CELL_USBD_ATTACH_FAILED);
	}

	if ((edesc = (UsbEndpointDescriptor *)cellUsbdScanStaticDescriptor(dev_id, idesc, USB_DESCRIPTOR_TYPE_ENDPOINT)) == NULL) {
		print_error("attach: cellUsbdScanStaticDescriptor (endpoint)", 0);
		return(CELL_USBD_ATTACH_FAILED);
	}

	// expect endpoint = 0x81 (same for xbox controller and steam controller...)
	if (edesc->bEndpointAddress != 0x81) {
		print_error("attach: bEndpointAddress != 0x81: ", edesc->bEndpointAddress);
		return(CELL_USBD_ATTACH_FAILED);
	}

	payload = SWAP16(edesc->wMaxPacketSize);
	if ((unit = unit_alloc(dev_id, payload, idesc->bInterfaceNumber, idesc->bAlternateSetting, XTYPE_XBOX360)) == NULL) {
		print_error("attach: unit_alloc (XTYPE_XBOX360)", 0);
		return(CELL_USBD_ATTACH_FAILED);
	}
	if ((unit->c_pipe = cellUsbdOpenPipe(dev_id, NULL)) < 0) {
		unit_free(unit);
		print_error("attach: cellUsbdOpenPipe (c)", unit->c_pipe);
		return(CELL_USBD_ATTACH_FAILED);
	}
	if ((unit->i_pipe = cellUsbdOpenPipe(dev_id, edesc)) < 0) {
		unit_free(unit);
		print_error("attach: cellUsbdOpenPipe (i)", unit->i_pipe);
		return(CELL_USBD_ATTACH_FAILED);
	}
	edesc->bEndpointAddress = 0x01; // XBox 360 controller out endpoint
	if ((unit->o_pipe = cellUsbdOpenPipe(dev_id, edesc)) < 0) {
		edesc->bEndpointAddress = 0x02; // It is 0x02 for some controllers
		if ((unit->o_pipe = cellUsbdOpenPipe(dev_id, edesc)) < 0) {
			print_error("attach:NO CONTROLLER", 0);

			unit_free(unit);
			return(CELL_USBD_ATTACH_FAILED);
		}
	}

	//// endpoint found, set configuration and add to connected controllers list
	//cellUsbdSetPrivateData(dev_id, unit);
	//cellUsbdSetConfiguration(unit->c_pipe, cdesc->bConfigurationValue, set_config_done, unit);
	//block(xpad_mutex);
	//XPAD.n++;
	//XPAD.is_connected[unit->number] = 1;
	//XPAD.con_unit[unit->number] = unit;
	//register_ldd_controller(unit);
	//unblock(xpad_mutex);

	return(CELL_USBD_ATTACH_SUCCEEDED);
}

static int32_t xpad_detach(int32_t dev_id) {
	//int32_t r;
	//XPAD_UNIT_t *unit;

	//// Xbox controller has been unplugged
	//// disconnect virtual controller associated to it
	//if ((unit = (XPAD_UNIT_t *)cellUsbdGetPrivateData(dev_id)) == NULL) {
	//	return(CELL_USBD_DETACH_FAILED);
	//}
	//block(xpad_mutex);

	//// update common data
	//XPAD.n--;
	//XPAD.is_connected[unit->number] = 0;
	//XPAD.con_unit[unit->number] = NULL;
	//unregister_ldd_controller(unit);
	//unblock(xpad_mutex);
	//unit_free(unit);
	return(CELL_USBD_DETACH_SUCCEEDED);
}

//static int32_t xpad_detach_all(void) {
//	int32_t i;
//	XPAD_UNIT_t *unit;
//
//	// detach all wired controllers
//	block(xpad_mutex);
//	for (i = 0; i < MAX_XPAD_NUM; i++) {
//		if (XPAD.is_connected[i]) {
//			unit = XPAD.con_unit[i];
//			if (unit->xtype == XTYPE_XBOX360) {
//				XPAD.n--;
//				XPAD.is_connected[unit->number] = 0;
//				XPAD.con_unit[unit->number] = NULL;
//				unregister_ldd_controller(unit);
//				unit_free(unit);
//			}
//		}
//	}
//	unblock(xpad_mutex);
//	return(CELL_USBD_DETACH_SUCCEEDED);
//}

static void unit_free(XPAD_UNIT_t *unit) {
	if (unit) {
		free(unit);
	}
}
//
//static XPAD_UNIT_t *unit_alloc(int32_t dev_id, int32_t payload, uint8_t ifnum, uint8_t as, uint8_t xtype) {
//	XPAD_UNIT_t *unit;
//	int32_t i;
//	if ((unit = (XPAD_UNIT_t *) malloc(sizeof(XPAD_UNIT_t) + payload)) != NULL) {
//		memset(unit, 0, sizeof(XPAD_UNIT_t));
//		unit->dev_id = dev_id;
//		unit->payload = payload;
//		unit->ifnum = ifnum;
//		unit->as = as;
//		unit->tcount = 0;
//		unit->rp = 0;
//		unit->wp = 0;
//		unit->rblen = 0;
//		unit->xtype = xtype;
//		if (xtype == XTYPE_XBOX360) {
//			unit->read_input = xpad_read_input;
//			unit->set_led = xpad_set_led;
//			unit->set_rumble = xpad_set_rumble;
//		}
//		//else if (xtype == XTYPE_XBOX360W) {
//		//	unit->read_input = xpadw_read_input;
//		//	unit->set_led = xpadw_set_led;
//		//	unit->set_rumble = xpadw_set_rumble;
//		//}
//		block(xpad_mutex);
//		unit->number = XPAD.next_number;
//		for (i = 0; i < MAX_XPAD_NUM; i++) {
//			++XPAD.next_number;
//			if (XPAD.next_number >= MAX_XPAD_NUM) {
//				XPAD.next_number = 0;
//			}
//			if (XPAD.con_unit[XPAD.next_number] == NULL) {
//				break;
//			}
//		}
//		unblock(xpad_mutex);
//		if (i >= MAX_XPAD_NUM) {
//			free(unit);
//			return(NULL);
//		}
//	}
//	return(unit);
//}
