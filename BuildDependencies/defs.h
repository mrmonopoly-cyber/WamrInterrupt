#pragma once

//==================================macros======================================================

#include <assert.h>
#include <stddef.h>

#include "nob.h"

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
            const char* ELE_NAME = ((ARR).data[__AKAB_I]), *____RUN=(const char*) 1;            \
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

//==================================functions declarations======================================

void apply_global_definitions(Cmd* cmd, ArrayViewGDef defs);

ArrayViewString default_src_dir_opts(void);
ArrayViewString default_compiler_opts(void);
ArrayViewString default_linker_opts(void);
ArrayViewString default_include_path_opts(void);
ArrayViewGDef default_global_defs_opts(void);


void apply_all_defualt_compile_opts(Cmd* cmd);
void apply_all_defualt_linker_opts(Cmd* cmd);

//================================implementation================================================

#ifdef DEFS_IMPLEMENTATION

void apply_global_definitions(Cmd* cmd, ArrayViewGDef defs)
{
    assert(cmd);

    FOR_EACH_FAT_ARRAY(defs, def)
    {
        if(def && def->def)
        {
            cmd_append(cmd, temp_sprintf("-D%s=%s", def->def, def->val));
        }
        else
        {
            cmd_append(cmd, temp_sprintf("-D%s", def->def));
        }
    }


}

void apply_all_defualt_compile_opts(Cmd* cmd)
{
    assert(cmd);

    //compiler options
    FOR_EACH_FAT_ARRAY_STR(default_compiler_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

    //include path
    FOR_EACH_FAT_ARRAY_STR(default_include_path_opts(), path)
    {
        if(path) cmd_append(cmd, temp_sprintf("-I%s", path));
    }

    //global definitions
    apply_global_definitions(cmd, default_global_defs_opts());
}

void apply_all_defualt_linker_opts(Cmd* cmd)
{
    assert(cmd);

    FOR_EACH_FAT_ARRAY_STR(default_linker_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

}

ArrayViewString default_src_dir_opts(void)
{
    static const char* opts[] = 
    {
        "src",
        //add here your sources directory like ThirdParty dependencies sources
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString default_compiler_opts(void)
{
    static const char* opts[] = 
    {
        "-Wall",
        "-Wextra",
        //add here your compiler options: -c, -ggdb, -O2, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString default_linker_opts(void)
{
    static const char* opts[] = 
    {
        //add here your compiler options: -lm, -lgdb, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString default_include_path_opts(void)
{
    static const char* opts[] = 
    {
        //add here your include path: -I...
        //consider the root of the project the starting source path
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewGDef default_global_defs_opts(void)
{
    static const GDef opts[] = 
    {
        //add here your global definitions: -DVAR=VALUE == (GDef) {.def="VAR", .val="VALUE"}
    };

    return (ArrayViewGDef) FAT_ARRAY_INIT(opts);
}

#endif // DEFS_IMPLEMENTATION
