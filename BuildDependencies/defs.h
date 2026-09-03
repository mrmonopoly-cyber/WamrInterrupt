#pragma once

//==================================macros======================================================

#include <stddef.h>

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#define CC "cc"

#define BUILD_DIR "build"
#define O_FILE "main"

#define FAT_ARRAY_TEMPLATE(T)           \
struct                                  \
{                                       \
    const T* data;                      \
    size_t len;                         \
}

#define FAT_ARRAY_INIT(STATIC_ARR) {.data = (STATIC_ARR), .len = ArraySize( (STATIC_ARR) )}

#define FOR_EACH_FAT_ARRAY_STR(ARR, ELE_NAME)                                                   \
    for(size_t __AKAB_I=0; __AKAB_I < (ARR).len; __AKAB_I++)                                    \
    for(                                                                                        \
            const char* ELE_NAME = ((ARR).data[__AKAB_I]), *____RUN=(void*) 1;                  \
            ____RUN;                                                                            \
            ____RUN = NULL)

#define FOR_EACH_FAT_ARRAY(ARR, ELE_NAME)                                                       \
    for(size_t __AKAB_I=0; __AKAB_I < (ARR).len; __AKAB_I++)                                    \
    for(                                                                                        \
            const __typeof__(ARR.data) ELE_NAME = &((ARR).data[__AKAB_I]), *____RUN=(void*) 1;  \
            ____RUN;                                                                            \
            ____RUN = NULL)

//==================================type definitions===========================================

typedef struct GDef{
    const char* def;
    const char* val;
}GDef;

typedef FAT_ARRAY_TEMPLATE(void)    ArrayViewVoid;
typedef FAT_ARRAY_TEMPLATE(char*)   ArrayViewString;
typedef FAT_ARRAY_TEMPLATE(GDef)    ArrayViewGDef;

static ArrayViewString default_src_dir_opts(void);
static ArrayViewString default_compiler_opts(void);
static ArrayViewString default_linker_opts(void);
static ArrayViewString default_include_path_opts(void);
static ArrayViewGDef default_global_defs_opts(void);

//================================implementation===============================================

#ifdef DEFS_IMPLEMENTATION

static ArrayViewString default_src_dir_opts(void)
{
    static const char* opts[] = 
    {
        "src",
        //add here your sources directory like ThirdParty dependencies sources
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

static ArrayViewString default_compiler_opts(void)
{
    static const char* opts[] = 
    {
        "-Wall",
        "-Wextra",
        //add here your compiler options: -c, -ggdb, -O2, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

static ArrayViewString default_linker_opts(void)
{
    static const char* opts[] = 
    {
        //add here your compiler options: -lm, -lgdb, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

static ArrayViewString default_include_path_opts(void)
{
    static const char* opts[] = 
    {
        //add here your include path: -I...
        //consider the root of the project the starting source path
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

static ArrayViewGDef default_global_defs_opts(void)
{
    static const GDef opts[] = 
    {
        //add here your global definitions: -DVAR=VALUE == (GDef) {.def="VAR", .val="VALUE"}
    };

    return (ArrayViewGDef) FAT_ARRAY_INIT(opts);
}

#endif // DEFS_IMPLEMENTATION
