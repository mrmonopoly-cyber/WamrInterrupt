#ifndef PYTHON_PREFIX
#define PYTHON_PREFIX
#endif // !PYTHON_PREFIX

#include "../defs.h"
#include "../nob.h"

//=========================================macros=================================================
#ifndef PYTHON_VERSION
#define PYTHON_VERSION_BASE "3.14"
#define PYTHON_VERSION_EXT ".8"
#define PYTHON_VERSION PYTHON_VERSION_BASE PYTHON_VERSION_EXT
#endif // !PYTHON_VERSION
#define PYTHON_ARCHIVE_NAME "Python-" PYTHON_VERSION
#define PYTHON_ARCHIVE PYTHON_ARCHIVE_NAME".tar.xz"
#define PYTHON_MIRROR \
    "https://www.python.org/ftp/python/" PYTHON_VERSION"/" PYTHON_ARCHIVE

//https://www.python.org/ftp/python/3.14.8/Python-3.14.8.tar.xz

#ifndef PYTHON_TYPES
#define PYTHON_TYPES
//=========================================types==================================================
typedef struct
{
}PythonRunOpt;

#endif // !PYTHON_TYPES

//===================================declarations================================================
PYTHON_PREFIX bool _python_run(const char* script_path, const PythonRunOpt opt);
#define python_run(SCRIPT_PATH, ...) _python_run((SCRIPT_PATH), ((PythonRunOpt){__VA_ARGS__}))

PYTHON_PREFIX const char* get_python(void);

// #define PYTHON_IMPLEMENTATION //enable for debugging
#ifdef PYTHON_IMPLEMENTATION
//===================================implementation==============================================
#include <assert.h>

#include "../dependency.h"
#include "makefile.h"

PYTHON_PREFIX bool _python_run(const char* script_path, const PythonRunOpt opt)
{
    bool res = false;
    const char* python = NULL;
    Cmd cmd = {0};

    if ( !script_path ) goto end;

    if ( !(python = get_python()) )
    {
        nob_log(ERROR, "Python is not present in your system. Abort" );
        goto end;
    }

    cmd_append(&cmd, python);
    cmd_append(&cmd, script_path);
    if ( !(res = cmd_run(&cmd) ) ) goto end;

end:
    return res;
}


static bool _python_installer(const char* work_dir, const char* bin_dst_path);

PYTHON_PREFIX const char* get_python(void)
{
    return check_dependency("python" PYTHON_VERSION_BASE,
            .download_mirror = PYTHON_MIRROR,
            .installer_f = _python_installer);
}

static bool _python_installer(const char* work_dir, const char* bin_dst_path)
{
    bool res = false;
    Cmd cmd = {0};
    const char* tar = {0};
    const char* pwd = get_current_dir_temp();
    char* python_sources = strdup(temp_sprintf("%s/%s", work_dir, PYTHON_ARCHIVE));

    if ( !work_dir || !bin_dst_path || !(tar = check_dependency("tar")) ) goto end;

    cmd_append(&cmd, tar);
    cmd_append(&cmd, "-C", work_dir);
    cmd_append(&cmd, "-xf");
    cmd_append(&cmd, python_sources);
    if ( !(res = cmd_run(&cmd) ) ) goto end;

    if ( !set_current_dir(temp_sprintf("%s/%s", work_dir,PYTHON_ARCHIVE_NAME)) ) goto end;

    cmd_append(&cmd, "./configure");
    cmd_append(&cmd, temp_sprintf("--prefix=%s", bin_dst_path));
    cmd_append(&cmd, "--disable-test-modules");
    if ( !(res = cmd_run(&cmd) ) ) goto end;

    nob_log(INFO, "compiling python");
    if ( !(res = makefile_run_command()) ) goto end;
    nob_log(INFO, "installing python");
    if ( !(res = makefile_run_command(.command = "install")) ) goto end;

    nob_log(INFO, "pythone preparation done");

end:
    set_current_dir(pwd);
    if ( python_sources ) free(python_sources);
    return res;
}

#endif //PYTHON_IMPLEMENTATION
