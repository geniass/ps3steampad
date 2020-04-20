#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus  
extern "C" {
#endif 

	/* vsh methods */
	// used to find malloc, free, and show notification
	// returns a void* to be casted to a function pointer
	void *getNIDfunc(const char *vsh_module, uint32_t fnid, int32_t offset);

	/* VSH function signatures for casting */
	typedef int notify_func_type(int, const char *);
	static notify_func_type *vshtask_notify = NULL;

	typedef void* malloc_func_type(unsigned int size);
	static malloc_func_type *vsh_malloc = NULL;

	typedef int free_func_type(void *);
	static free_func_type *vsh_free = NULL;

	/* Higher-level functions */
	// displays a notification on the PS3
	void show_msg(char *msg);

	void beep3();

#ifdef __cplusplus 
}
#endif 