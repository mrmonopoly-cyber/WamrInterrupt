#pragma once

#include <stdbool.h>

#include "nob.h"

bool f_build_wamr(bool verbose, Procs* procs);

#ifdef WAMR_IMPLEMENTATION

#include "defs.h"

bool f_build_wamr(bool verbose, Procs* procs)
{
    static char wamr_root_dir[128] = {0};

    bool res = false;
    Cmd cmd = {0};

    const char* pwd = get_current_dir_temp();
    const char* wamr_path = "./ThirdParty/wamr";
    snprintf(wamr_root_dir, sizeof(wamr_root_dir),
            "%s/ThirdParty/wamr/wasm-micro-runtime", pwd);

    nob_log(INFO, "wamr root is at %s", wamr_root_dir);

    const GDef build_gen_defs [] =
    {
        {"WAMR_BUILD_PLATFORM"                  , "linux"},
        {"WAMR_BUILD_TARGET"                    , "X86_64"},
        {"WAMR_ROOT_DIR"                        , wamr_root_dir},
        {"WAMR_BUILD_INTERP"                    , "0"},
        {"WAMR_BUILD_FAST_INTERP"               , "0"},
        {"WAMR_BUILD_AOT"                       , "1"},
        {"WAMR_BUILD_LIBC_BUILTIN"              , "1"},
        {"WAMR_BUILD_LIBC_WASI"                 , "1"},
        {"WAMR_BUILD_SIMD"                      , "0"},
        {"WAMR_BUILD_REF_TYPES"                 , "1"},
        {"WAMR_BUILD_THREAD_MGR"                , "1"},
        {"WAMR_BUILD_SHARED_MEMORY"             , "1"},
        {"WAMR_BUILD_LIB_PTHREAD"               , "1"},
        {"WAMR_BUILD_LIB_WASI_THREADS"          , "1"},
        {"WAMR_BUILD_LINUX_PERF"                , "0"},
        {"WAMR_BUILD_DEBUG_INTERP"              , "0"},
        {"WAMR_BUILD_LOAD_CUSTOM_SECTION"       , "1"},
        {"WAMR_BUILD_CUSTOM_NAME_SECTION"      , "1"},

        {"CMAKE_EXPORT_COMPILE_COMMANDS"        ,"ON"},
        {"CMAKE_BUILD_TYPE"                     ,"Release"},
    };

    if ( !program_exsists_on_path("cmake") )
    {
        nob_log( ERROR, "cmake is not present in your system: compilation aborted");
        return false;
    }

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "-S", wamr_path);
    cmd_append(&cmd, "-B", BUILD_DIR"/wamr");
    cmd_append(&cmd, "-G", "Ninja");

    apply_global_definitions(&cmd, (ArrayViewGDef) FAT_ARRAY_INIT(build_gen_defs));

    if ( !( res = cmd_run(&cmd) ) ) goto end;

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "--build", BUILD_DIR"/wamr");

    if ( verbose ) cmd_append(&cmd, "--verbose");

    if ( !( res = cmd_run(&cmd, .async = procs) ) ) goto end;

end:
    cmd_free(cmd);
    return res;
}

#endif // WAMR_IMPLEMENTATION
