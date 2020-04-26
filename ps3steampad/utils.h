#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus  
extern "C" {
#endif 

	// for some reason the built-in sys_ppu_thread_exit freezes the console
	//sys_ppu_thread_exit(0);
	inline void ppu_thread_exit()
	{
		system_call_1(41, 0); //ppu_thread_exit
	}

	/* vsh methods */
	// used to find malloc, free, and show notification
	// returns a void* to be casted to a function pointer
	void *getNIDfunc(const char *vsh_module, uint32_t fnid, int32_t offset);

	/* Higher-level functions */
	// displays a notification on the PS3
	void show_msg(char *msg);

	void beep3();

	int snprintf(char *_Restrict s, size_t n, const char *_Restrict fmt, ...);

	inline int print_error(char* name, int32_t err) {
		return printf("ERROR: %s = 0x%x (%d)\n", name, err, err);
	}

#ifdef __cplusplus 
}
#endif 