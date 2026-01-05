#include "ltdl.h"
#include <stddef.h>

int lt_dlinit (void) { return 0; }
int lt_dlexit (void) { return 0; }
lt_dlhandle lt_dlopen (const char *filename) { return NULL; }
lt_dlhandle lt_dlopenext (const char *filename) { return NULL; }
void * lt_dlsym (lt_dlhandle handle, const char *symbol) { return NULL; }
const char * lt_dlerror (void) { return "MOCKED"; }
int lt_dlclose (lt_dlhandle handle) { return 0; }
int lt_dlmakeresident (lt_dlhandle handle) { return 0; }
int lt_dlisresident (lt_dlhandle handle) { return 0; }
const lt_dlinfo * lt_dlgetinfo (lt_dlhandle handle) { return NULL; }

int lt_dladvise_init (lt_dladvise *advise) { return 0; }
int lt_dladvise_destroy (lt_dladvise *advise) { return 0; }
int lt_dladvise_ext (lt_dladvise *advise) { return 0; }
int lt_dladvise_global (lt_dladvise *advise) { return 0; }
lt_dlhandle lt_dlopenadvise (const char *filename, lt_dladvise advise) { return NULL; }
