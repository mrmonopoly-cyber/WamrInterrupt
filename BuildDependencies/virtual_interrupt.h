#include <stdbool.h>
#include <stddef.h>

#define VI_OLIB_BASE_NAME "virtual_interrupt"
#define VI_OLIB_NAME "lib"VI_OLIB_BASE_NAME

#ifndef VINT_TYPES
#define VINT_TYPES
typedef struct
{
    char** items;
    size_t count;
    size_t capacity;
}SourcesList;

typedef enum
{
   VIOutputFormat_StaticLib,
   VIOutputFormat_DynamicLib,

}VIOutputFormat;
#endif // !VINT_TYPES

bool vi_f_build_virtual_interrupt(bool verbose, bool test, VIOutputFormat format);

#ifdef VINT_IMPLEMENTATION
#include "nob.h"

#define WAMR_IMPLEMENTATION
#include "wamr.h"
#define DEFS_IMPLEMENTATION
#include "defs.h"


typedef struct
{
    Procs* procs;
    bool test;
}FCompileArgs;

static bool vi_f_compile(Walk_Entry entry);
static bool vi_f_check(void);
static bool vi__f_check_append_sources(Walk_Entry entry);
static bool vi_f_link(VIOutputFormat format);

bool vi_f_build_virtual_interrupt(bool verbose, bool test, VIOutputFormat format)
{
    bool res=false;
    Procs procs = {0};
    FCompileArgs args=
    {
        .procs = &procs,
        .test = test,
    };

    if ( !(res = vi_f_check()) )
    {
        goto end;
    }

    //wamr
    if( !(f_build_wamr(verbose, &procs)) )
    {
        goto end;
    }

    //source directories
    FOR_EACH_FAT_ARRAY_STR(vi_default_src_dir_opts(), dir)
    {
        if(dir)
        {
            nob_log(INFO, "compiling sources in src: %s", dir);
            if( !(res=walk_dir(dir, vi_f_compile, .data = &args)) )
            {
                goto end;
            }
        }
    }

    procs_flush(&procs);

    if( !(res=vi_f_link(format)) )
    {
        goto end;
    }

end:
    return res;
}

static bool vi_f_check(void)
{
    bool res= false;
    Cmd cmd = {0};

    SourcesList sources = {0};

    if ( !vi_program_exsists_on_path("clang-tidy") )
    {
        nob_log( WARNING, "clang-tidy is not present in your system: static check is skipped");
        return true;
    }

    cmd_append(&cmd, "clang-tidy");
    cmd_append(&cmd, "--config-file="VI_PROJET_ROOT"/.clang-tidy");
    cmd_append(&cmd, "--warnings-as-errors=*");

    //source directories
    FOR_EACH_FAT_ARRAY_STR(vi_default_src_dir_opts(), dir)
    {
        if ( !(res = walk_dir(dir, vi__f_check_append_sources, .data = &sources)) )
        {
            goto end;
        }
    }

    for (size_t i=0; i<sources.count; i++)
    {
        cmd_append(&cmd, sources.items[i]);
    }

    cmd_append(&cmd, "--");

    //include path
    FOR_EACH_FAT_ARRAY_STR(vi_default_include_path_opts(), path)
    {
        if(path) cmd_append(&cmd, temp_sprintf("-I%s", path));
    }

    res = cmd_run(&cmd);

end:
    for (size_t i=0; i<sources.count; i++)
    {
        free(sources.items[i]);
    }

    da_free(sources);
    cmd_free(cmd);
    return res;
}

static bool vi_f_compile(Walk_Entry entry)
{
    FCompileArgs* args = entry.data;
    bool res=true;
    Cmd cmd = {0};
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;

    if(entry.type == FILE_REGULAR && !strcmp(suffix, ".c"))
    {
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CC);
        vi_apply_all_defualt_compile_opts(&cmd);
        cmd_append(&cmd, "-fPIC");

        if ( args->test )
        {
            cmd_append(&cmd, "-DENABLE_TESTS");
        }

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));


        if ( args->test && !strcmp(file_name, "main.c" ) )
        {
            entry.path = VI_PROJET_ROOT"/BuildDependencies/dummy_main.c";
        }

        cmd_append(&cmd, entry.path);

        res = cmd_run(&cmd, .async = args->procs);

    }

    cmd_free(cmd);
    return res;
}

static bool vi__f_check_append_sources(Walk_Entry entry)
{
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;
    SourcesList* sources = entry.data;

    if(entry.type == FILE_REGULAR && !strcmp(suffix, ".c"))
    {
        da_append(sources, strdup(entry.path));
    }

    return true;
}

static bool vi_f_link(VIOutputFormat format)
{
    Dir_Entry dir = {0};
    Cmd cmd = {0};
    bool res = true;

    if(!dir_entry_open(BUILD_DIR, &dir)) return false;

    switch (format)
    {

        case VIOutputFormat_StaticLib:
            {
                if ( !(res = vi_program_exsists_on_path("ar")) )
                {
                    nob_log( ERROR, "ar is not present in your PATH. abort" );
                    goto end;
                }

                cmd_append(&cmd, "ar");
                cmd_append(&cmd, "rcs");
                cmd_append(&cmd, BUILD_DIR"/"VI_OLIB_NAME".a");
            }
            break;
        case VIOutputFormat_DynamicLib:
            {
                cmd_append(&cmd, CC);
                cmd_append(&cmd, "-o", BUILD_DIR"/"VI_OLIB_NAME".so");

                vi_apply_all_defualt_linker_opts(&cmd);
                cmd_append(&cmd, "-shared");

            }
            break;
    }

    while(dir_entry_next(&dir))
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (FILE_REGULAR == get_file_type(file_path))
        {
            printf("found %s\n", file_path);
            cmd_append(&cmd, file_path);
        }
    }

    res = cmd_run(&cmd);

end:
    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

#endif // VINT_IMPLEMENTATION
