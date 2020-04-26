#include "utils.h"

#include <stdarg.h>

/* VSH function signatures for casting */
typedef int notify_func_type(int, const char *);
static notify_func_type *vshtask_notify = NULL;

typedef void* malloc_func_type(unsigned int size);
static malloc_func_type *vsh_malloc = NULL;

typedef int free_func_type(void *);
static free_func_type *vsh_free = NULL;

typedef int snprintf_func_type(char *_Restrict, size_t, const char *_Restrict, ...);
static snprintf_func_type *vsh_snprintf = NULL;

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

int snprintf(char *_Restrict s, size_t n, const char *_Restrict fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (!vsh_snprintf) {
		vsh_snprintf = (snprintf_func_type*)getNIDfunc("stdc", 0x3A840AE3, 0);
	}
	if (vsh_snprintf) {
		return vsh_snprintf(s, n, fmt, args);
	}
	return -1;
}