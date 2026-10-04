#ifndef MAKEFILE_PREFIX
#define MAKEFILE_PREFIX
#include <stdio.h>
#endif // !MAKEFILE_PREFIX

#include "../defs.h"

#ifndef MAKEFILE_TYPES
//=========================================types==================================================
#define MAKEFILE_TYPES
typedef struct
{
    const char* command;
    const char* makefile_path;
    ArrayViewGDef defs;
    size_t jobs;
    Procs* async;
    bool ignore_result;
}MakefileRunCommandOpt;
#endif // !MAKEFILE_TYPES

//===================================declarations================================================

MAKEFILE_PREFIX bool _makefile_run_command(const MakefileRunCommandOpt opt);
#define makefile_run_command(...) _makefile_run_command(((MakefileRunCommandOpt) {__VA_ARGS__}))

// #define MAKEFILE_IMPLEMENTATION //enable for debugging
#ifdef MAKEFILE_IMPLEMENTATION
//===================================implementation==============================================

#include "../nob.h"
#include "../dependency.h"

MAKEFILE_PREFIX bool _makefile_run_command(const MakefileRunCommandOpt opt)
{
    bool res = false;
    Cmd cmd = {0};
    const char* make = NULL;

    const size_t jobs = (opt.jobs > 0 && opt.jobs < (size_t) nprocs())
        ? opt.jobs : (size_t) nprocs();
    char buf[32] = {0};

    //TODO: auto install make
    if ( !(make = check_dependency("make")) )
    {
        nob_log(ERROR, "make is not available in your PATH. Abort");
        goto end;
    }

    cmd_append(&cmd, make);

    if ( opt.makefile_path )
    {
        if ( !file_exists(opt.makefile_path) )
        {
            nob_log(ERROR, "Not such file or directory: %s", opt.makefile_path);
            goto end;
        }

        cmd_append(&cmd, "-C", opt.makefile_path);
    }

    if ( opt.command ) cmd_append(&cmd, opt.command);

    if ( jobs )
    {
        snprintf(buf, sizeof(buf), "%zu", jobs);
        cmd_append(&cmd, "-j", buf);
    }


    bool temp_res = cmd_run(&cmd, .async = opt.async);
    res = opt.ignore_result ? true : temp_res;

end:
    cmd_free(cmd);
    return res;
}

#endif // !MAKEFILE_IMPLEMENTATION
