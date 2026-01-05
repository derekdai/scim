#ifndef _LTDL_H
#define _LTDL_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void * lt_dlhandle;
typedef struct lt_dladvise_struct * lt_dladvise;
typedef struct {
    char *filename;
} lt_dlinfo;

int lt_dlinit (void);
int lt_dlexit (void);
lt_dlhandle lt_dlopen (const char *filename);
lt_dlhandle lt_dlopenext (const char *filename);
void * lt_dlsym (lt_dlhandle handle, const char *symbol);
const char * lt_dlerror (void);
int lt_dlclose (lt_dlhandle handle);
int lt_dlmakeresident (lt_dlhandle handle);
int lt_dlisresident (lt_dlhandle handle);
const lt_dlinfo * lt_dlgetinfo (lt_dlhandle handle);

/* Advisory flags */
int lt_dladvise_init (lt_dladvise *advise);
int lt_dladvise_destroy (lt_dladvise *advise);
int lt_dladvise_ext (lt_dladvise *advise);
int lt_dladvise_global (lt_dladvise *advise);
lt_dlhandle lt_dlopenadvise (const char *filename, lt_dladvise advise);


#ifdef __cplusplus
}
#endif

#endif
