#pragma once

#include "../build_tools/build_tools.h"

#define WAMR_BUILD_DIR BUILD_DIR"/wamr"

static bool f_build_wamr(bool verbose, Procs* procs, bool lsp)
{
    bool res = false;
    Cmd cmd = {0};

    const char* pwd = get_current_dir_temp();
    const char* wamr_path = THIRDPARTY"/wamr";

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
        {"WAMR_BUILD_CUSTOM_NAME_SECTION"       , "1"},

        {"CMAKE_EXPORT_COMPILE_COMMANDS"        , "ON"},
        {"CMAKE_C_COMPILER"                     , CC},
        {"CMAKE_BUILD_TYPE"                     , "Release"},
    };

    if ( !file_exists(WAMR_BUILD_DIR"/CMakeFiles") && !(res = cmake_configure(
                wamr_path,
                WAMR_BUILD_DIR,
                .global_defs = (ArrayViewGDef) FAT_ARRAY_INIT(build_gen_defs))) )
    {
        nob_log( ERROR, "wamr: failed to configure cmake");
        goto end;
    }

    if ( !(res = cmake_build(WAMR_BUILD_DIR, .verbose = verbose, .async = procs)) ) goto end;
    
    if ( lsp)
    {
        copy_file(WAMR_BUILD_DIR"/compile_commands.json", PROJECT_ROOT"/compile_commands.json");
    }

end:
    cmd_free(cmd);
    return res;
}
