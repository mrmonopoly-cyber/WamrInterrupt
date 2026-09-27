#ifndef DEPENDENCY_PREFIX
#define DEPENDENCY_PREFIX
#include <assert.h>
#endif // !DEPENDENCY_PREFIX

#include "defs.h"
#include "nob.h"

#ifndef DEPENDENCY_LOCAL_PROGRAMS_DB
#define DEPENDENCY_LOCAL_PROGRAMS_DB PROJECT_ROOT"/.local_programs"
#endif // !DEPENDENCY_LOCAL_PROGRAMS_DB

#ifndef DEPENDENCY_LOCAL_PROGRAMS_DB_WORK
#define DEPENDENCY_LOCAL_PROGRAMS_DB_WORK DEPENDENCY_LOCAL_PROGRAMS_DB"/work"
#endif // !DEPENDENCY_LOCAL_PROGRAMS_DB_WORK

#ifndef DEPENDENCY_TYPES
//===================================types=======================================================
#define DEPENDENCY_TYPES
typedef enum
{
    DependencyFetcher_Curl  = 0,    //INFO: default
    DependencyFetcher_Wget,
    DependencyFetcher_Git,

    _DependencyFetcher_Count
}DependencyFetcher;

typedef bool
(DependencyInstaller)(const char* work_dir, const char* bin_dst_path);

typedef struct
{
    const char* download_mirror;
    DependencyFetcher fetcher;
    DependencyInstaller* installer_f;
}DependencyCheckerOpt;

#endif // !DEPENDENCY_TYPES

//===================================declarations================================================
/**
* IF the program DOES EXISTS on PATH it will return program name
* IF the program DOES NOT EXISTS on PATH it will try to install it in DEPENDENCY_LOCAL_PROGRAMS_DB
* and returns full path to the program.
* IF the program DOES NOT EXISTS on PATH and the installation process failed for any reason
* it will return NULL;
* By default it will use Curl to download, it can be changed in the DependencyCheckerOpt struct.
* IF Curl and Wget are not present it will return NULL
* The function may return pointer to a static buffer. It's NOT Thread-Safe
*/
DEPENDENCY_PREFIX const char* _check_dependency(const char* name, const DependencyCheckerOpt opt);
#define check_dependency(NAME, ...) _check_dependency((NAME), ((DependencyCheckerOpt) {__VA_ARGS__}))

// #define DEPENDENCY_IMPLEMENTATION //enable for debugging
#ifdef DEPENDENCY_IMPLEMENTATION
//===================================implementation==============================================

DEPENDENCY_PREFIX bool _program_exists_on_path(const char* program_name);

DEPENDENCY_PREFIX const char* _get_fetcher_name(const DependencyFetcher fetcher);
DEPENDENCY_PREFIX bool _fetcher_download(
        DependencyFetcher fetcher,
        const char* mirror,
        const char* output);

DEPENDENCY_PREFIX bool _db_sarch_program(
        const char* prog_name,
        const char* opt_root,
        char* o_buffer,
        const size_t o_buffer_size);

DEPENDENCY_PREFIX const char* _check_dependency(const char* name, const DependencyCheckerOpt opt)
{
    static char temp_buffer[1024];
    const char* res = NULL;
    const char* fetcher = _get_fetcher_name(opt.fetcher);
    Cmd cmd = {0};

    if ( !name ) goto end;

    if ( !file_exists(DEPENDENCY_LOCAL_PROGRAMS_DB) )
    {
        mkdir_if_not_exists(DEPENDENCY_LOCAL_PROGRAMS_DB);
    }

    if ( _program_exists_on_path(name) )
    {
        nob_log(INFO, "found %s in PATH", name);
        res = name;
        goto end;
    }

    if ( _db_sarch_program(name, NULL, temp_buffer, sizeof(temp_buffer)) )
    {
        goto found;
    }

    nob_log(INFO, "dependency missing: %s. Downloading it", name);
    if ( !_program_exists_on_path(fetcher) ||  !opt.download_mirror || !opt.installer_f )
    {
        nob_log(WARNING ,
                "one of the following is NULL: "
                "fetcher %s, exists: %s, "
                "mirror : %s, "
                "installer: %p, ",
                fetcher,
                _program_exists_on_path(fetcher) ? "true" : "false",
                opt.download_mirror,
                opt.installer_f);
        goto end;
    }

    nob_log(INFO, "clearing work dir: %s", DEPENDENCY_LOCAL_PROGRAMS_DB_WORK);
    clear_dir(DEPENDENCY_LOCAL_PROGRAMS_DB_WORK);
    mkdir_if_not_exists(DEPENDENCY_LOCAL_PROGRAMS_DB_WORK);

    mkdir_if_not_exists(DEPENDENCY_LOCAL_PROGRAMS_DB_WORK);
    if ( !_fetcher_download(opt.fetcher, opt.download_mirror, DEPENDENCY_LOCAL_PROGRAMS_DB_WORK) )
    {
        goto end;
    }

    snprintf(temp_buffer, sizeof(temp_buffer), "%s/%s",DEPENDENCY_LOCAL_PROGRAMS_DB, name);
    nob_log(INFO, "installing %s in: %s", name, temp_buffer);
    mkdir_if_not_exists(temp_buffer);
    if( !opt.installer_f(DEPENDENCY_LOCAL_PROGRAMS_DB_WORK, temp_buffer) )
    {
        goto end;
    }

    if ( !_db_sarch_program(name, temp_buffer, temp_buffer, sizeof(temp_buffer)) )
    {
        goto end;
    }

found:
    res = realpath(temp_buffer, NULL);
    nob_log(INFO, "found %s in local db dir: %s at: %s",
            DEPENDENCY_LOCAL_PROGRAMS_DB, name, res);

end:
    cmd_free(cmd);
    return res;
}

DEPENDENCY_PREFIX bool _program_exists_on_path(const char* program_name)
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

DEPENDENCY_PREFIX const char* _get_fetcher_name(const DependencyFetcher fetcher)
{
    switch (fetcher)
    {
        case DependencyFetcher_Curl:        return "curl";
        case DependencyFetcher_Wget:        return "wget";
        case DependencyFetcher_Git:         return "git";
        case _DependencyFetcher_Count:      assert(0 && "unreachable");
    }

    assert(0 && "unreachable");
}

DEPENDENCY_PREFIX bool _fetcher_download(
        DependencyFetcher fetcher,
        const char* mirror,
        const char* output)
{
    bool res = false;
    Cmd cmd = {0};
    const char* fetcher_prog = _get_fetcher_name(fetcher);

    if ( !mirror || !output )
    {
        goto end;
    }

    cmd_append(&cmd, fetcher_prog);
    switch (fetcher)
    {
        case DependencyFetcher_Curl:
            {
                cmd_append(&cmd, "-fL", "--output-dir", output, "-O");
                cmd_append(&cmd, mirror);
            }
            break;
        case DependencyFetcher_Wget:
            {
                cmd_append(&cmd, "-P", output);
                cmd_append(&cmd, mirror);
            }
            break;
        case DependencyFetcher_Git:
            {
                cmd_append(&cmd, "-C", output);
                cmd_append(&cmd, "clone", "--recursive");
                cmd_append(&cmd, mirror);
            }
            break;
        case _DependencyFetcher_Count:      assert( 0 && "unreachable" );
    }

    res = cmd_run(&cmd);

end:
    cmd_free(cmd);
    return res;
}

typedef struct
{
    const char* prog_name;
    char* o_buffer;
    const size_t o_buffer_size;
    bool found;
}_DepDBChecker;

static bool _db_serach_program_check_file(Walk_Entry entry)
{
    _DepDBChecker* data = entry.data;
    const char* name = temp_file_name(entry.path);

    if ( entry.type == FILE_REGULAR && !strcmp(name, data->prog_name) )
    {
        const size_t path_len = strlen(entry.path);
        if ( path_len <= data->o_buffer_size )
        {
            memcpy(data->o_buffer, entry.path, path_len);
            data->found = true;
        }
        return false;
    }

    return true;
}

DEPENDENCY_PREFIX bool _db_sarch_program(
        const char* prog_name,
        const char* opt_root,
        char* o_buffer,
        const size_t o_buffer_size)
{
    const char* root = opt_root ? opt_root : DEPENDENCY_LOCAL_PROGRAMS_DB;
    _DepDBChecker data = 
    {
        .prog_name = prog_name,
        .o_buffer = o_buffer,
        .o_buffer_size = o_buffer_size,
        .found = false,
    };

    assert( prog_name );
    assert( o_buffer );

    (void) walk_dir(root, _db_serach_program_check_file, .data = &data);

    return data.found;
}

#endif // !DEPENDENCY_IMPLEMENTATION
