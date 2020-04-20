#include "utils.h"

void *getNIDfunc(const char * vsh_module, uint32_t fnid, int32_t offset) {
	// from webman-MOD source

	// 0x10000 = ELF
	// 0x10080 = segment 2 start
	// 0x10200 = code start

	uint32_t table = (*(uint32_t *)0x1008C) + 0x984; // vsh table address
	//  uint32_t table = (*(uint32_t*)0x1002C) + 0x214 - 0x10000; // vsh table address
	//  uint32_t table = 0x63A9D4;

	while (((uint32_t)*(uint32_t *)table) != 0) {
		uint32_t *export_stru_ptr = (uint32_t *)*(uint32_t *)table; // ptr to export stub, size 2C, "sys_io" usually... Exports:0000000000635BC0 stru_635BC0:    ExportStub_s <0x1C00, 1, 9, 0x39, 0, 0x2000000, aSys_io, ExportFNIDTa
		const char *lib_name_ptr = (const char *)*(uint32_t *)((char *)export_stru_ptr + 0x10);
		if (strncmp(vsh_module, lib_name_ptr, strlen(lib_name_ptr)) == 0) {
			// we got the proper export struct
			uint32_t lib_fnid_ptr = *(uint32_t *)((char *)export_stru_ptr + 0x14);
			uint32_t lib_func_ptr = *(uint32_t *)((char *)export_stru_ptr + 0x18);
			uint16_t count = *(uint16_t *)((char *)export_stru_ptr + 6); // number of exports
			for (int i = 0; i < count; i++) {
				if (fnid == *(uint32_t *)((char *)lib_fnid_ptr + i * 4)) {
					// take address from OPD
					return (void **)*((uint32_t *)(lib_func_ptr)+i) + offset;
				}
			}
		}
		table = table + 4;
	}
	return(0);
}

// displays a notification on the PS3
void show_msg(char* msg) 
{
	// from webman-MOD
	if (!vshtask_notify) {
		vshtask_notify = (notify_func_type*)getNIDfunc("vshtask", 0xA02D46E7, 0);
	}
	if (strlen(msg) > 200) {
		msg[200] = 0;
	}
	if (vshtask_notify) {
		vshtask_notify(0, msg);
	}
}


void *_malloc(unsigned int size) {

	// vsh export for malloc
	if (!vsh_malloc) {
		vsh_malloc = (malloc_func_type*) getNIDfunc("allocator", 0x759E0635, 0);
	}
	if (vsh_malloc) {
		return vsh_malloc(size);
	}
	return(NULL);
}

void _free(void *ptr) {

	// vsh export for free
	if (!vsh_free) {
		vsh_free = (free_func_type*) getNIDfunc("allocator", 0x77A602DD, 0);
	}
	if (vsh_free) {
		vsh_free(ptr);
	}
}

void beep3()
{
	system_call_3(392, 0x1004, 0xa, 0x1b6);
}
