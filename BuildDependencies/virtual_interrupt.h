#include <stdbool.h>
#include <stddef.h>

#ifndef VINT_TYPES
#define VINT_TYPES
typedef struct
{
    char** items;
    size_t count;
    size_t capacity;
}SourcesList;
#endif // !VINT_TYPES

bool f_build_virtual_interrupt(bool verbose, bool test);

#ifdef VINT_IMPLEMENTATION
#include "defs.h"
#include "wamr.h"
#include "nob.h"

typedef struct
{
    Procs* procs;
    bool test;
}FCompileArgs;

static bool f_compile(Walk_Entry entry);
static bool f_check(void);
static bool _f_check_append_sources(Walk_Entry entry);
static bool f_link(void);

bool f_build_virtual_interrupt(bool verbose, bool test)
{
    Procs procs = {0};
    FCompileArgs args=
    {
        .procs = &procs,
        .test = test,
    };
    //wamr
    if( !f_build_wamr(verbose, &procs) )
    {
        nob_log(ERROR, "failed building wamr");
        return 1;
    }

    if ( !f_check() )
    {
        nob_log(ERROR, "failed checking sources");
        return 1;
    }

    //source directories
    FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
    {
        if(dir)
        {
            nob_log(INFO, "compiling sources in src: %s", dir);
            if(!walk_dir(dir, f_compile, .data = &args))
            {
                nob_log(ERROR, "failed compiling sources in %s", dir);
                return 1;
            }
        }
    }

    procs_flush(&procs);

    if(!f_link())
    {
        nob_log(ERROR, "failed liking");
        return 1;
    }

    return 0;
}

static bool f_check(void)
{
    bool res= false;
    Cmd cmd = {0};

    SourcesList sources = {0};

    if ( !program_exsists_on_path("clang-tidy") )
    {
        nob_log( WARNING, "clang-tidy is not present in your system: static check is skipped");
        return true;
    }

    cmd_append(&cmd, "clang-tidy");
    cmd_append(&cmd, "--warnings-as-errors=*");

    //source directories
    FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
    {
        if ( !(res = walk_dir(dir, _f_check_append_sources, .data = &sources)) )
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
    FOR_EACH_FAT_ARRAY_STR(default_include_path_opts(), path)
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

static bool f_compile(Walk_Entry entry)
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
        apply_all_defualt_compile_opts(&cmd);

        if ( args->test )
        {
            cmd_append(&cmd, "-DENABLE_TESTS");
        }

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));


        if ( args->test && !strcmp(file_name, "main.c" ) )
        {
            entry.path = "./BuildDependencies/dummy_main.c";
        }

        cmd_append(&cmd, entry.path);

        res = cmd_run(&cmd, .async = args->procs);

    }

    cmd_free(cmd);
    return res;
}

static bool _f_check_append_sources(Walk_Entry entry)
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

static bool f_link(void)
{
    Dir_Entry dir = {0};
    Cmd cmd = {0};
    bool res = true;

    if(!dir_entry_open(BUILD_DIR, &dir)) return false;

    cmd_append(&cmd, CC);


    cmd_append(&cmd, "-o", O_FILE);

    while(dir_entry_next(&dir))
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (FILE_REGULAR == get_file_type(file_path))
        {
            printf("found %s\n", file_path);
            cmd_append(&cmd, file_path);
        }
    }

    apply_all_defualt_linker_opts(&cmd);

    res = cmd_run(&cmd);

    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

#endif // VINT_IMPLEMENTATION


