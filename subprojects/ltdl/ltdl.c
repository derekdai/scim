#include "ltdl.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char *last_error = NULL;

int lt_dlinit (void) { return 0; }
int lt_dlexit (void) { return 0; }

int lt_dlmakeresident (lt_dlhandle handle) { return 0; }
int lt_dlisresident (lt_dlhandle handle) { return 0; }

lt_dlhandle lt_dlopen (const char *filename) {
    // If filename is NULL, open self
    lt_dlhandle handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        last_error = dlerror();
    }
    return handle;
}

lt_dlhandle lt_dlopenext (const char *filename) {
    lt_dlhandle handle = lt_dlopen(filename);
    if (handle) return handle;

    // Try appending .so
    if (filename) {
        char *so_name = (char *)malloc(strlen(filename) + 4);
        if (so_name) {
            sprintf(so_name, "%s.so", filename);
            handle = lt_dlopen(so_name);
            free(so_name);
            if (handle) return handle;
        }

        // Also try .la? In a meson build we might not generate .la files,
        // but if the app asks for them, we should probably check if we can map it to .so
        // But usually lt_dlopenext is called without extension.
    }

    return NULL;
}

int lt_dlclose (lt_dlhandle handle) {
    return dlclose(handle);
}

lt_ptr lt_dlsym (lt_dlhandle handle, const char *symbol) {
    lt_ptr ptr = dlsym(handle, symbol);
    if (!ptr) {
        last_error = dlerror();
    }
    return ptr;
}

const char * lt_dlerror (void) {
    return last_error;
}

int lt_dladdsearchdir (const char *search_dir) { return 0; }
int lt_dlsetsearchpath (const char *search_path) { return 0; }
const char * lt_dlgetsearchpath (void) { return ""; }

const lt_dlinfo * lt_dlgetinfo (lt_dlhandle handle) {
    static lt_dlinfo info;
    info.filename = (char*)"mock_filename";
    info.name = (char*)"mock_name";
    info.ref_count = 1;
    return &info;
}

int lt_dlforeach (lt_dlforeach_callback *func, lt_ptr data) {
    return 0;
}
