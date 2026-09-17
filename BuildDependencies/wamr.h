#pragma once

#include <stdbool.h>

#include "nob.h"

bool f_build_wamr(bool verbose, Procs* procs);

#ifdef WAMR_IMPLEMENTATION

#include "defs.h"

bool f_build_wamr(bool verbose, Procs* procs)
{
    bool res = false;
    Cmd cmd = {0};

    nob_log(INFO, "VI_THIRDPARTY: %s", VI_THIRDPARTY);

    const char* pwd = get_current_dir_temp();
    const char* wamr_path = VI_THIRDPARTY"/wamr";

    const GDef build_gen_defs [] =
    {
        {"WAMR_BUILD_PLATFORM"                  , "linux"},
        {"WAMR_BUILD_TARGET"                    , "X86_64"},
        {"WAMR_ROOT_DIR"                        , "./wasm-micro-runtime"},
        {"WAMR_BUILD_INTERP"                    , "0"},
        {"WAMR_BUILD_FAST_INTERP"               , "0"},
        {"WAMR_BUILD_AOT"                       , "1"},
        {"WAMR_BUILD_LIBC_BUILTIN"              , "0"},
        {"WAMR_BUILD_LIBC_WASI"                 , "1"},
        {"WAMR_BUILD_SIMD"                      , "0"},
        {"WAMR_BUILD_REF_TYPES"                 , "1"},
        {"WAMR_BUILD_THREAD_MGR"                , "0"},
        {"WAMR_BUILD_SHARED_MEMORY"             , "1"},
        {"WAMR_BUILD_LIB_PTHREAD"               , "0"},
        {"WAMR_BUILD_LIB_WASI_THREADS"          , "0"},
        {"WAMR_BUILD_LINUX_PERF"                , "0"},
        {"WAMR_BUILD_DEBUG_INTERP"              , "0"},
        {"WAMR_BUILD_LOAD_CUSTOM_SECTION"       , "1"},
        {"WAMR_BUILD_CUSTOM_NAME_SECTION"      , "1"},

        {"CMAKE_EXPORT_COMPILE_COMMANDS"        ,"ON"},
        {"CMAKE_BUILD_TYPE"                     ,"Release"},
    };

    if ( !vi_program_exsists_on_path("cmake") )
    {
        nob_log( ERROR, "cmake is not present in your system: compilation aborted");
        return false;
    }

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "-S", wamr_path);
    cmd_append(&cmd, "-B", BUILD_DIR"/wamr");
    cmd_append(&cmd, "-G", "Ninja");

    vi_apply_global_definitions(&cmd, (ArrayViewGDef) FAT_ARRAY_INIT(build_gen_defs));

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
