//==================================dependencies================================================

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "nob.h"

//==================================macros======================================================

#ifndef DEFS_PREFIX
#define DEFS_PREFIX
#endif // !DEFS_PREFIX

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#ifndef CC
#define CC "cc"
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

DEFS_PREFIX void
go_rebuild_yourself_check_dir(int argc, char** argv, const char *source_path, const char* dir_path);

//================================implementation================================================

// #define DEFS_IMPLEMENTATION //enable for debugging
#ifdef DEFS_IMPLEMENTATION

DEFS_PREFIX void apply_global_definitions(Cmd* cmd, ArrayViewGDef defs)
{
    NOB_ASSERT(cmd);

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
    NOB_ASSERT(cmd);

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
    NOB_ASSERT(cmd);

    FOR_EACH_FAT_ARRAY_STR(default_linker_opts(), opt)
    {
        if(opt) cmd_append(cmd, opt);
    }

}

DEFS_PREFIX ArrayViewString default_src_dir_opts(void)
{
    static const char* opts[] = 
    {
        PROJECT_ROOT"/src",
        //add here your sources directory like ThirdParty dependencies sources
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewString default_compiler_opts(void)
{
    static const char* opts[] = 
    {
        "-Wall",
        "-Wextra",
        "-xc",
        //add here your compiler options: -c, -ggdb, -O2, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewString default_linker_opts(void)
{
    static const char* opts[] = 
    {
        //add here your compiler options: -lm, -lgdb, ...
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewString default_include_path_opts(void)
{
    static const char* opts[] = 
    {
        //add here your include path: -I...
        //consider the root of the project the starting source path
    };

    return (ArrayViewString) FAT_ARRAY_INIT(opts);
}

DEFS_PREFIX ArrayViewGDef default_global_defs_opts(void)
{
    static const GDef opts[] = 
    {
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

struct _DefsAddFileToListArg
{
    bool found_diff;
    const char* binary_path;
};

static inline bool _add_file_to_list(Walk_Entry entry)
{
    struct _DefsAddFileToListArg* data = entry.data;
    NOB_ASSERT( data );

    if (
            entry.type == FILE_REGULAR &&
            !data->found_diff &&
            needs_rebuild1(data->binary_path, entry.path)
       )
    {
        data->found_diff = true;
    }

    return true;
}

DEFS_PREFIX void
go_rebuild_yourself_check_dir(int argc, char** argv, const char *source_path, const char* dir_path)
{
    const char *binary_path = shift(argv, argc);
    Cmd cmd = {0};
    struct _DefsAddFileToListArg arg =
    {
        .found_diff = false,
        .binary_path = binary_path,
    };

#ifdef _WIN32
    // On Windows executables almost always invoked without extension, so
    // it's ./nob, not ./nob.exe. For renaming the extension is a must.
    if (!sv_ends_with_cstr(nob_sv_from_cstr(binary_path), ".exe")) {
        binary_path = temp_sprintf("%s.exe", binary_path);
    }
#endif
    UNUSED(walk_dir(dir_path, _add_file_to_list, .data = &arg));

    if ( !needs_rebuild1(binary_path, source_path) && !arg.found_diff ) // no rebuild is needed
    {
        return;
    }

    const char *old_binary_path = temp_sprintf("%s.old", binary_path);

    if (!nob_rename(binary_path, old_binary_path)) exit(1);
    cmd_append(&cmd, NOB_REBUILD_URSELF(binary_path, source_path));
    Cmd_Opt opt = {0};
    if (!cmd_run_opt(&cmd, opt)) {
        rename(old_binary_path, binary_path);
        exit(1);
    }

#ifdef NOB_EXPERIMENTAL_DELETE_OLD
    // TODO: this is an experimental behavior behind a compilation flag.
    // Once it is confirmed that it does not cause much problems on both POSIX and Windows
    // we may turn it on by default.
    delete_file(old_binary_path);
#endif // NOB_EXPERIMENTAL_DELETE_OLD

    cmd_append(&cmd, binary_path);
    da_append_many(&cmd, argv, argc);
    if (!cmd_run_opt(&cmd, opt)) exit(1);
    exit(0);
}

#endif // DEFS_IMPLEMENTATION
