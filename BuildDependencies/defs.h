
#include <assert.h>
#include <stddef.h>

#include "nob.h"

//==================================macros======================================================
#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#define CC "cc"

#ifndef VI_PROJET_ROOT
#pragma message "no VI_PROJET_ROOT passed using default value: \".\""
#define VI_PROJET_ROOT "."
#endif // !VI_PROJET_ROOT

#define BUILD_DIR "build"
#define VI_THIRDPARTY VI_PROJET_ROOT"/ThirdParty"

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
#ifndef DEFS_TYPES
#define DEFS_TYPES
typedef struct GDef{
    const char* def;
    const char* val;
}GDef;

typedef FAT_ARRAY_TEMPLATE(void)    ArrayViewVoid;
typedef FAT_ARRAY_TEMPLATE(char*)   ArrayViewString;
typedef FAT_ARRAY_TEMPLATE(GDef)    ArrayViewGDef;
#endif // !DEFS_TYPES

//==================================functions declarations======================================

void vi_apply_global_definitions(Cmd* cmd, ArrayViewGDef defs);

ArrayViewString vi_default_src_dir_opts(void);
ArrayViewString vi_default_compiler_opts(void);
ArrayViewString vi_default_linker_opts(void);
ArrayViewString vi_default_include_path_opts(void);
ArrayViewGDef vi_default_global_defs_opts(void);

void vi_apply_all_defualt_compile_opts(Cmd* cmd);
void vi_apply_all_defualt_linker_opts(Cmd* cmd);

bool vi_file_has_suffix(
        const char* const restrict file_name, const size_t len_file_name,
        const char* const restrict suffix, const size_t len_suffix);

bool vi_file_has_suffix_with_null(
        const char* const restrict file_name,
        const char* const restrict suffix);

bool vi_program_exsists_on_path(const char* program_name);

//================================implementation================================================

#ifdef DEFS_IMPLEMENTATION

void vi_apply_global_definitions(Cmd* cmd, ArrayViewGDef defs)
{
    assert(cmd);

    FOR_EACH_FAT_ARRAY(defs, def)
    {
        if(def && def->val)
        {
            cmd_append(cmd, temp_sprintf("-D%s=%s", def->def, def->val));
        }
        else
        {
            cmd_append(cmd, temp_sprintf("-D%s", def->def));
        }
    }


}

void vi_apply_all_defualt_compile_opts(Cmd* cmd)
{
    assert(cmd);

    //compiler options
    FOR_EACH_FAT_ARRAY_STR(vi_default_compiler_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

    //include path
    FOR_EACH_FAT_ARRAY_STR(vi_default_include_path_opts(), path)
    {
        if(path) cmd_append(cmd, temp_sprintf("-I%s", path));
    }

    //global definitions
    vi_apply_global_definitions(cmd, vi_default_global_defs_opts());
}

void vi_apply_all_defualt_linker_opts(Cmd* cmd)
{
    assert(cmd);

    FOR_EACH_FAT_ARRAY_STR(vi_default_linker_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

}

bool vi_program_exsists_on_path(const char* program_name)
{
    bool res=false;
    Cmd cmd = {0};

    cmd_append(&cmd, "bash");
    cmd_append(&cmd, "-c");
    cmd_append(&cmd, temp_sprintf("command -v %s", program_name));

    res = cmd_run(&cmd);

    cmd_free(cmd);
    return res;
}

ArrayViewString vi_default_src_dir_opts(void)
{
    static const char* opts[] = 
    {
        VI_PROJET_ROOT"/src/virtual_interrupt",
        //add here your sources directory like ThirdParty dependencies sources
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString vi_default_compiler_opts(void)
{
    static const char* opts[] = 
    {
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-ggdb",
        "-pedantic",
        "-fsanitize=address,undefined",
        //add here your compiler options: -c, -ggdb, -O2, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString vi_default_linker_opts(void)
{
    static const char* opts[] = 
    {
        "-L"BUILD_DIR"/wamr",
        "-lvmlib",
        "-lm",
        "-ggdb",
        "-fsanitize=address,undefined",
        //add here your compiler options: -lm, -lgdb, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewString vi_default_include_path_opts(void)
{
    static const char* opts[] = 
    {
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/include",
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/libraries/thread-mgr",
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils",
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils/uncommon",
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/include",
        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/linux",

        VI_THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/interpreter",

        //add here your include path: -I...
        //consider the root of the project the starting source path
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

ArrayViewGDef vi_default_global_defs_opts(void)
{
    static const GDef opts[] = 
    {
        //wamr features
        {"WASM_ENABLE_THREAD_MGR"               , "0"},
        {"WASM_ENABLE_CUSTOM_NAME_SECTION"      , "1"},
        {"_GNU_SOURCE",                              },

        //add here your global definitions: -DVAR=VALUE == (GDef) {.def="VAR", .val="VALUE"}
    };

    return (ArrayViewGDef) FAT_ARRAY_INIT(opts);
}

bool vi_file_has_suffix(
        const char* file_name, const size_t len_file_name,
        const char* suffix, const size_t len_suffix)
{
    const char* file_name_suffix = file_name + len_file_name - len_suffix;

    return !strcmp(file_name_suffix, suffix);
}

bool vi_file_has_suffix_with_null(
        const char* const restrict file_name,
        const char* const restrict suffix)
{
    return vi_file_has_suffix(file_name, strlen(file_name), suffix, strlen(suffix));
}

#endif // DEFS_IMPLEMENTATION
