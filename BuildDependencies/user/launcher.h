#pragma once

#include "../nob.h"
#include "../defs.h"
#include "../builder.h"

#include "wamr.h"
#include "vi_interrupt.h"

#define LAUNCHER_BUILD_DIR BUILD_DIR"/launcher"

static bool f_compile_launcher(const char* launcher_src_root, Procs* procs, bool lsp)
{
    bool res = false;
    BuilderCompilerOptions comp_opts = {0};
    const char* raw_comp_opts[] = 
    {
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-ggdb",
        "-pedantic",
        "-fsanitize=address,undefined",
        "-Wthread-safety",

        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/include",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/libraries/thread-mgr",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils/uncommon",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/include",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/linux",
        "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/interpreter",

        "-DWASM_ENABLE_THREAD_MGR=0",
        "-DWASM_ENABLE_CUSTOM_NAME_SECTION=1",
        "-D_GNU_SOURCE",
    };

    da_append_many(&comp_opts, raw_comp_opts, ArraySize(raw_comp_opts));

    NOB_ASSERT( launcher_src_root );

    res = builder_compile_dir_files_to_obj(PROJECT_ROOT"/src/launcher",
            .comp_opt =
            {
            .build_dir = LAUNCHER_BUILD_DIR,
            .compiler_options = comp_opts,
            .async = procs,
            .lsp = lsp,
            },
            .suffix = ".c",
            );

    return res;
}

static bool f_link_launcher(bool lsp)
{
    bool res = false;
    BuilderLinkerOptions linker_opts = {0};
    const char* raw_linker_opts[] =
    {
        "-L" VI_BUILD_DIR,
        "-L" WAMR_BUILD_DIR,
        "-lvmlib",
        "-lvi",
        "-lm",
        "-ggdb",
        "-fsanitize=address,undefined",
        "-Wl,--rpath=" VI_BUILD_DIR,
    };

    da_append_many(&linker_opts, raw_linker_opts, ArraySize(raw_linker_opts));

    return builder_link_file_to_obj("main",
            .build_dir = LAUNCHER_BUILD_DIR,
            .linker_options = linker_opts,
            .lsp = lsp,
            );
}
