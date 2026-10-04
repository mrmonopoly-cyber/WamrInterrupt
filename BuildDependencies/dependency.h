#ifndef DEPENDENCY_PREFIX
#define DEPENDENCY_PREFIX
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
    bool ignore_path;
}DependencyCheckerOpt;

typedef struct
{
    const char* output_dir;
}FetcherDownloadOpt;

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

DEPENDENCY_PREFIX bool
_fetcher_download(DependencyFetcher fetcher, const char* mirror, FetcherDownloadOpt opt);
#define fetcher_download(FETCHER, MIRROR, ...) \
    _fetcher_download((FETCHER), (MIRROR), ((FetcherDownloadOpt) {__VA_ARGS__}))

DEPENDENCY_PREFIX bool dependency_clear(void);

// #define DEPENDENCY_IMPLEMENTATION //enable for debugging
#ifdef DEPENDENCY_IMPLEMENTATION
//===================================implementation==============================================
#include <string.h>

static char _db_dir[512];
static const char _fetcher_output_dir;
static const char _fetcher_input_mirror;

#define N_MAX_ARGS  (5U)
#define N_ARGS_CURL (5U)
#define N_ARGS_WGET (3U)
#define N_ARGS_GIT  (5U)

static const struct _FetcherInfos
{
    const char* prog_name;
    const char *args[N_MAX_ARGS];
    size_t n_args;
}FETCHER_INFOS [] =
{
    [DependencyFetcher_Curl] = 
    {
        .prog_name = "curl",
        .args =
        {
            "-fL",
            "--output-dir",
            &_fetcher_output_dir,
            "-O",
            &_fetcher_input_mirror,
        },
        .n_args = N_ARGS_CURL,
    },
    [DependencyFetcher_Wget] =
    {
        .prog_name = "wget",
        .args =
        {
            "-p",
            &_fetcher_output_dir,
            &_fetcher_input_mirror,
        },
    },

    [DependencyFetcher_Git] =
    {
        .prog_name = "git",
        .args =
        {
            "-C",
            &_fetcher_output_dir,
            "clone",
            "--recursive",
            &_fetcher_input_mirror,
        },
    }
};

static_assert(N_ARGS_CURL <= N_MAX_ARGS, "out of bounds");
static_assert(N_ARGS_WGET <= N_MAX_ARGS, "out of bounds");
static_assert(N_ARGS_GIT  <= N_MAX_ARGS, "out of bounds");

#undef N_ARGS_CURL
#undef N_ARGS_WGET
#undef N_ARGS_GIT
#undef N_MAX_ARGS

DEPENDENCY_PREFIX bool _program_exists_on_path(const char* program_name);

DEPENDENCY_PREFIX bool _db_sarch_program(
        const char* prog_name,
        const char* opt_root,
        char* o_buffer,
        const size_t o_buffer_size);

DEPENDENCY_PREFIX const char* _check_dependency(const char* name, const DependencyCheckerOpt opt)
{
    static char temp_buffer[1024];
    const char* res = NULL;
    const char* fetcher = FETCHER_INFOS[opt.fetcher].prog_name;
    const char* temp_work_dir = "unset";
    Cmd cmd = {0};

    if ( !name ) goto end;

    if ( !_db_dir[0] )
    {
        const char *pwd = get_current_dir_temp();
        snprintf(_db_dir, sizeof(_db_dir), "%s/%s", pwd, DEPENDENCY_LOCAL_PROGRAMS_DB);
    }

    NOB_ASSERT( _db_dir[0] );
    temp_work_dir = temp_sprintf("%s/work", _db_dir);
    if ( !file_exists(_db_dir) )
    {
        mkdir_if_not_exists(_db_dir);
    }

    if ( !opt.ignore_path && _program_exists_on_path(name) )
    {
        nob_log(INFO, "found %s in PATH", name);
        res = name;
        goto end;
    }

    if ( _db_sarch_program(name, NULL, temp_buffer, sizeof(temp_buffer)) )
    {
        goto found;
    }

    nob_log(INFO, "dependency missing: %s. Downloading it from: %s",
            name, opt.download_mirror);
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
                (void*) (uintptr_t) opt.installer_f);
        goto end;
    }

    nob_log(INFO, "clearing work dir: %s", temp_work_dir);
    clear_dir(temp_work_dir);
    mkdir_if_not_exists(temp_work_dir);

    mkdir_if_not_exists(temp_work_dir);
    if (
            !fetcher_download(
                opt.fetcher,
                opt.download_mirror,
                .output_dir= temp_work_dir)
       )
    {
        goto end;
    }

    snprintf(temp_buffer, sizeof(temp_buffer), "%s/%s", _db_dir, name);
    nob_log(INFO, "installing %s in: %s", name, temp_buffer);
    mkdir_if_not_exists(temp_buffer);
    if( !opt.installer_f(temp_work_dir, temp_buffer) )
    {
        goto end;
    }

    if ( !_db_sarch_program(name, temp_buffer, temp_buffer, sizeof(temp_buffer)) )
    {
        goto end;
    }

found:
    res = realpath(temp_buffer, NULL);
    NOB_ASSERT( res != NULL );
    nob_log(INFO, "found %s in local db dir: %s at: %s", temp_work_dir, name, res);

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

DEPENDENCY_PREFIX bool _fetcher_download(
        DependencyFetcher fetcher,
        const char* mirror,
        FetcherDownloadOpt opt)
{
    bool res = false;
    Cmd cmd = {0};

    const struct _FetcherInfos* infos = NULL;
    const char* output = opt.output_dir ? opt.output_dir : ".";

    if ( !mirror || !file_exists(output) || fetcher >= _DependencyFetcher_Count )
    {
        goto end;
    }
    
    infos = &FETCHER_INFOS[fetcher];

    if ( !_program_exists_on_path(infos->prog_name) )
    {
        nob_log(ERROR, "fetcher %s does not exists on path. Abort", infos->prog_name);
        goto end;
    }

    cmd_append(&cmd, infos->prog_name);

    for (size_t i=0; i<infos->n_args; i++)
    {
        const char* arg = infos->args[i];

        if ( arg == &_fetcher_output_dir )
        {
            cmd_append(&cmd, output);
        }
        else if (arg == &_fetcher_input_mirror)
        {
            cmd_append(&cmd, mirror);
        }
        else
        {
            cmd_append(&cmd, arg);
        }
    }

    res = cmd_run(&cmd);

end:
    cmd_free(cmd);
    return res;
}

DEPENDENCY_PREFIX bool dependency_clear(void)
{
    bool res = true;
    if ( _db_dir[0] && file_exists(_db_dir) )
    {
        res = clear_dir(_db_dir);
    }
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
    NOB_ASSERT( _db_dir[0] );
    const char* root = opt_root ? opt_root : _db_dir;
    _DepDBChecker data = 
    {
        .prog_name = prog_name,
        .o_buffer = o_buffer,
        .o_buffer_size = o_buffer_size,
        .found = false,
    };

    NOB_ASSERT( prog_name );
    NOB_ASSERT( o_buffer );

    (void) walk_dir(root, _db_serach_program_check_file, .data = &data);

    return data.found;
}

#endif // !DEPENDENCY_IMPLEMENTATION
