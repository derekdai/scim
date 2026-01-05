#include "ltdl.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* A simple wrapper around dlopen/dlsym to mock libltdl for environments without it */

int lt_dlinit (void) { return 0; }
int lt_dlexit (void) { return 0; }

lt_dlhandle lt_dlopen (const char *filename) {
    if (!filename) return NULL;
    return dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
}

lt_dlhandle lt_dlopenext (const char *filename) {
    if (!filename) return NULL;

    lt_dlhandle handle = lt_dlopen(filename);
    if (handle) return handle;

    /* Try appending .so */
    char *buf = malloc(strlen(filename) + 4);
    if (!buf) return NULL;
    sprintf(buf, "%s.so", filename);
    handle = lt_dlopen(buf);
    free(buf);
    return handle;
}

void * lt_dlsym (lt_dlhandle handle, const char *symbol) {
    return dlsym(handle, symbol);
}

const char * lt_dlerror (void) {
    return dlerror();
}

int lt_dlclose (lt_dlhandle handle) {
    return dlclose(handle);
}

int lt_dlmakeresident (lt_dlhandle handle) { return 0; }
int lt_dlisresident (lt_dlhandle handle) { return 0; }
const lt_dlinfo * lt_dlgetinfo (lt_dlhandle handle) { return NULL; }

int lt_dladvise_init (lt_dladvise *advise) { return 0; }
int lt_dladvise_destroy (lt_dladvise *advise) { return 0; }
int lt_dladvise_ext (lt_dladvise *advise) { return 0; }
int lt_dladvise_global (lt_dladvise *advise) { return 0; }
lt_dlhandle lt_dlopenadvise (const char *filename, lt_dladvise advise) {
    /* Ignore advice for now, just dlopen */
    return lt_dlopenext(filename);
}
