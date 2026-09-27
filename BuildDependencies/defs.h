//==================================dependencies================================================

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "nob.h"

//==================================macros======================================================

#ifndef DEFS_PREFIX
#define DEFS_PREFIX
#endif // !DEFS_PREFIX

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#ifndef CC
#define CC "clang"
#endif // !CC

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif // !PROJECT_ROOT

#ifndef O_FILE
#define O_FILE "main"
#endif // !O_FILE

#ifndef BUILD_DIR
#define BUILD_DIR "build"
#endif // !BUILD_DIR

#define THIRDPARTY PROJECT_ROOT"/ThirdParty"
#define BUILD_DEPS PROJECT_ROOT"/BuildDependencies"

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

typedef FAT_ARRAY_TEMPLATE(char*)   ArrayViewString;
typedef FAT_ARRAY_TEMPLATE(GDef)    ArrayViewGDef;
#endif // !DEFS_TYPES
//==================================functions declarations======================================

DEFS_PREFIX void apply_global_definitions(Cmd* cmd, ArrayViewGDef defs);

DEFS_PREFIX ArrayViewString default_src_dir_opts(void);
DEFS_PREFIX ArrayViewString default_compiler_opts(void);
DEFS_PREFIX ArrayViewString default_linker_opts(void);
DEFS_PREFIX ArrayViewString default_include_path_opts(void);
DEFS_PREFIX ArrayViewGDef default_global_defs_opts(void);

DEFS_PREFIX void apply_all_defualt_compile_opts(Cmd* cmd);
DEFS_PREFIX void apply_all_defualt_linker_opts(Cmd* cmd);

DEFS_PREFIX bool file_has_suffix(
        const char* const restrict file_name, const size_t len_file_name,
        const char* const restrict suffix, const size_t len_suffix);

DEFS_PREFIX bool file_has_suffix_with_null(
        const char* const restrict file_name,
        const char* const restrict suffix);

DEFS_PREFIX bool clear_dir(const char* const restrict path);

//================================implementation================================================

// #define DEFS_IMPLEMENTATION //enable for debugging
#ifdef DEFS_IMPLEMENTATION

DEFS_PREFIX void apply_global_definitions(Cmd* cmd, ArrayViewGDef defs)
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

DEFS_PREFIX void apply_all_defualt_compile_opts(Cmd* cmd)
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

DEFS_PREFIX void apply_all_defualt_linker_opts(Cmd* cmd)
{
    assert(cmd);

    FOR_EACH_FAT_ARRAY_STR(default_linker_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

}

DEFS_PREFIX ArrayViewString default_src_dir_opts(void)
{
    static const char* opts[] = 
    {
        PROJECT_ROOT"/src/virtual_interrupt",
        //add here your sources directory like ThirdParty dependencies sources
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewString default_compiler_opts(void)
{
    static const char* opts[] = 
    {
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-ggdb",
        "-pedantic",
        "-fsanitize=address,undefined",
        "-Wthread-safety",
        //add here your compiler options: -c, -ggdb, -O2, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewString default_linker_opts(void)
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

DEFS_PREFIX ArrayViewString default_include_path_opts(void)
{
    static const char* opts[] = 
    {
        THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/include",
        THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/libraries/thread-mgr",
        THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils",
        THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils/uncommon",
        THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/include",
        THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/linux",

        THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/interpreter",

        //add here your include path: -I...
        //consider the root of the project the starting source path
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewGDef default_global_defs_opts(void)
{
    static const GDef opts[] = 
    {
        {"WASM_ENABLE_THREAD_MGR"               , "0"},
        {"WASM_ENABLE_CUSTOM_NAME_SECTION"      , "1"},
        {"_GNU_SOURCE",                              },

        //add here your global definitions: -DVAR=VALUE == (GDef) {.def="VAR", .val="VALUE"}
    };

    return (ArrayViewGDef) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX bool file_has_suffix(
        const char* file_name, const size_t len_file_name,
        const char* suffix, const size_t len_suffix)
{
    const char* file_name_suffix = file_name + len_file_name - len_suffix;

    return !strcmp(file_name_suffix, suffix);
}

DEFS_PREFIX bool file_has_suffix_with_null(
        const char* const restrict file_name,
        const char* const restrict suffix)
{
    return file_has_suffix(file_name, strlen(file_name), suffix, strlen(suffix));
}

DEFS_PREFIX bool _defs__delete(Walk_Entry entry)
{
    delete_file(entry.path);
    return true;
}

DEFS_PREFIX bool clear_dir(const char* const restrict path)
{
    bool res = false;

    if ( get_file_type(path) == FILE_DIRECTORY )
    {
        res = walk_dir(
                path,
                _defs__delete,
                .post_order = true,
                );
    }

    return res;
}

#endif // DEFS_IMPLEMENTATION
