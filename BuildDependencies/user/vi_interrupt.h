#pragma once

#include "../nob.h"
#include "../defs.h"
#include "../builder.h"
#include "../dependency.h"
#include "wamr.h"

#define VI_BUILD_DIR BUILD_DIR"/vi"

static const char* _fake_board_include_paths[] =
{
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/include",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/libraries/thread-mgr",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/utils/uncommon",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/include",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/shared/platform/linux",
    "-I" THIRDPARTY"/wamr/wasm-micro-runtime/core/iwasm/interpreter",
};

static bool vi_f_check(const char* vi_src_root);
static bool f_compile_vi_interrupt(const char* vi_src_root, Procs* procs, bool lsp)
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
        "-fpic",
        "-fsanitize=address,undefined",
        "-Wthread-safety",

        "-DWASM_ENABLE_THREAD_MGR=0",
        "-DWASM_ENABLE_CUSTOM_NAME_SECTION=1",
        "-D_GNU_SOURCE",
    };

    if ( !(res = vi_f_check(vi_src_root)) )
    {
        goto end;
    }

    da_append_many(&comp_opts, raw_comp_opts, ArraySize(raw_comp_opts));
    da_append_many(&comp_opts, _fake_board_include_paths, ArraySize(_fake_board_include_paths));

    NOB_ASSERT( vi_src_root );

    res = builder_compile_dir_files_to_obj(PROJECT_ROOT"/src/virtual_interrupt",
            .comp_opt =
            {
            .build_dir = VI_BUILD_DIR,
            .compiler_options = comp_opts,
            .async = procs,
            .lsp = lsp,
            },
            .suffix = ".c",
            );

end:
    return res;
}

static bool f_link_vi_interrupt(bool lsp)
{
    BuilderLinkerOptions linker_opts = {0};
    const char* raw_linker_opts[] =
    {
        "-shared",
        "-L" BUILD_DIR"/wamr",
        "-lvmlib",
        "-lm",
        "-ggdb",
        "-fsanitize=address,undefined",
    };

    da_append_many(&linker_opts, raw_linker_opts, ArraySize(raw_linker_opts));

    bool so_res = builder_link_file_to_obj(VI_BUILD_DIR"/libvi.so",
            .build_dir = VI_BUILD_DIR,
            .linker_options = linker_opts,
            .lsp = lsp,
            );

    return so_res;
}

static bool vi__f_check_append_sources(Walk_Entry entry)
{
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;
    Cmd cmd = {0};
    bool res = true;

    if(entry.type == FILE_REGULAR && file_has_suffix_with_null(entry.path, ".c"))
    {
        cmd_append(&cmd, "clang-tidy");
        cmd_append(&cmd, "--config-file=" PROJECT_ROOT "/.clang-tidy");
        cmd_append(&cmd, "--warnings-as-errors=*");
        cmd_append(&cmd, entry.path);
        cmd_append(&cmd, "--");
        da_append_many(&cmd, _fake_board_include_paths, ArraySize(_fake_board_include_paths));

        res = cmd_run(&cmd);
    }

    return res;
}

static bool vi_f_check(const char* vi_src_root)
{
    bool res= false;

    NOB_ASSERT( vi_src_root );

    if ( !check_dependency("clang-tidy") )
    {
        nob_log( WARNING, "clang-tidy is not present in your system: static check is skipped");
        return true;
    }

    //source directories
    if ( !(res = walk_dir(vi_src_root , vi__f_check_append_sources)) )
    {
        goto end;
    }


end:
    return res;
}
