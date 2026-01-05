#ifndef LTDL_H
#define LTDL_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * lt_dlhandle;
typedef void * lt_ptr;
typedef void * lt_user_data;
typedef void * lt_module;

int lt_dlinit (void);
int lt_dlexit (void);
lt_dlhandle lt_dlopen (const char *filename);
lt_dlhandle lt_dlopenext (const char *filename);
int lt_dlclose (lt_dlhandle handle);
lt_ptr lt_dlsym (lt_dlhandle handle, const char *symbol);
const char * lt_dlerror (void);

int lt_dladdsearchdir (const char *search_dir);
int lt_dlsetsearchpath (const char *search_path);
const char * lt_dlgetsearchpath (void);

typedef struct {
  char *filename;
  char *name;
  int ref_count;
  unsigned int is_resident : 1;
  unsigned int is_symglobal : 1;
  unsigned int is_local : 1;
} lt_dlinfo;

const lt_dlinfo * lt_dlgetinfo (lt_dlhandle handle);

typedef int lt_dlforeach_callback (lt_dlhandle handle, lt_ptr data);
int lt_dlforeach (lt_dlforeach_callback *func, lt_ptr data);

int lt_dlmakeresident (lt_dlhandle handle);
int lt_dlisresident (lt_dlhandle handle);

#ifdef __cplusplus
}
#endif

#endif /* LTDL_H */
