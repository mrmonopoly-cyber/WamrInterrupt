#include <stdbool.h>

#include "nob.h"


#ifndef WASI_SDK_VERSION
#define WASI_SDK_VERSION "34"
#endif // !WASI_SDK_VERSION

#define WASI_SDK_NAME "wasi-sdk-"WASI_SDK_VERSION".0-x86_64-linux"
#define WASI_SDK_TAR WASI_SDK_NAME".tar.gz"
#define WASI_SDK_MIRROR "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-"WASI_SDK_VERSION"/"WASI_SDK_TAR

#define O_FAKE_BOARD_NAME "fake_board"
#define O_FAKE_BOARD_WASM O_FAKE_BOARD_NAME".wasm"
#define O_FAKE_BOARD_AOT O_FAKE_BOARD_NAME".aot"


bool f_build_fakeboard(bool verbose, const char* path_main);
bool f_clean_fakeboard(void);

//======================================implementation==========================================

// #define FAKE_BOARD_IMPLEMENTATION //enable for debugging
#ifdef FAKE_BOARD_IMPLEMENTATION
#include <string.h>

#include "defs.h"
#include "dependency.h"
#include "build_tools/cmake.h"


bool f_build_fakeboard(bool verbose, const char* path_main)
{
    bool res = false;
    Cmd cmd = {0};
    const char* exported_functions[] = 
    {
        "board_main",
        "led_value",
        "new_led_value",
        "led_value_i1",
        "led_value_i2",
        "led_value_i3",
    };

    if ( !file_exists(WASI_SDK_NAME"/VERSION") )
    {
        if (
                !file_exists(WASI_SDK_TAR) &&
                !fetcher_download(DependencyFetcher_Curl, WASI_SDK_MIRROR)
           )
        {
            nob_log(ERROR, "failed downloading: %s. Abort", WASI_SDK_MIRROR);
            res = false;
            goto end;
        }

        if ( !check_dependency("tar") )
        {
            nob_log( ERROR, "tar is not present in your system: abort");
            res = false;
            goto end;
        }

        cmd_append(&cmd, "tar");
        cmd_append(&cmd, "-xf", WASI_SDK_TAR);
        if ( !(res = cmd_run(&cmd)) ) goto end;
    }

    if ( !file_exists(THIRDPARTY"/wamr/wasm-micro-runtime/wamr-compiler/build/wamrc") )
    {
        bool build_ok = false;
        const char* cmake = cmake_get();

        if ( !check_dependency("git") )
        {
            nob_log( ERROR, "git is not present in your system: abort");
            res = false;
            goto end;
        }

        if ( !cmake )
        {
            nob_log( ERROR, "cmake is not present in your system: abort");
            res = false;
            goto end;
        }

        cmd_append(&cmd, "git");
        cmd_append(&cmd, "-C", THIRDPARTY"/wamr/wasm-micro-runtime");
        cmd_append(&cmd, "apply");
        cmd_append(&cmd, "../wamrc_build.patch");
        if ( !(res = cmd_run(&cmd)) ) goto end;

        {
            const char* pwd = get_current_dir_temp();

            if ( !check_dependency("python") )
            {
                nob_log( ERROR, "python is not present in your system: abort");
                res = false;
                goto end;
            }


            set_current_dir(THIRDPARTY"/wamr/wasm-micro-runtime/wamr-compiler");

            cmd_append(&cmd, "bash");
            cmd_append(&cmd, "-c");
            if ( strcmp(cmake, "cmake") )
            {
                cmd_append(&cmd,
                        temp_sprintf("PATH=\"$(dirname '%s'):$PATH\" ./build_llvm.sh", cmake));
            }
            else
            {
                cmd_append(&cmd, "./build_llvm.sh");
            }

            (void) cmd_run(&cmd);
            build_ok = true;

            if ( build_ok )
            {
                build_ok = cmake_configure(".", "build");
            }

            if ( build_ok )
            {
                build_ok = cmake_build("build");
            }


            set_current_dir(pwd);
        }

        cmd_append(&cmd, "git");
        cmd_append(&cmd, "-C", THIRDPARTY"/wamr/wasm-micro-runtime");
        cmd_append(&cmd, "apply", "-R");
        cmd_append(&cmd, "../wamrc_build.patch");
        if ( !(res = cmd_run(&cmd)) || !build_ok ) goto end;
    }


    cmd_append(&cmd, "./"WASI_SDK_NAME"/bin/clang");
    cmd_append(&cmd, "--sysroot=./"WASI_SDK_NAME"/share/wasi-sysroot");
    cmd_append(&cmd, "--target=wasm32-wasip1-threads");

    cmd_append(&cmd, "-mexec-model=reactor");
    cmd_append(&cmd, "-Wl,--no-entry");

    cmd_append(&cmd, "-Wl,--initial-memory=1048576");
    cmd_append(&cmd, "-Wl,--max-memory=4194304");

    cmd_append(&cmd, "-Wl,--export=__heap_base");
    cmd_append(&cmd, "-Wl,--export=__data_end");
    cmd_append(&cmd, "-Wl,--export-table");

    cmd_append(&cmd, "-Wl,--import-memory");
    cmd_append(&cmd, "-Wl,--export-memory");
    cmd_append(&cmd, "-Wl,--shared-memory");

    FOR_EACH_FAT_ARRAY_STR((ArrayViewString) FAT_ARRAY_INIT(exported_functions), fun)
    {
        cmd_append(&cmd, temp_sprintf("-Wl,--export=%s", fun));
    }

    if(verbose) cmd_append(&cmd, "-v");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", O_FAKE_BOARD_WASM);

    cmd_append(&cmd, path_main);
    if ( !(res = cmd_run(&cmd)) ) goto end;

    //./ThirdParty/wasm-micro-runtime/wamr-compiler/build/wamrc --emit-custom-sections=name -o fake_board.aot fake_board.wasm

    cmd_append(&cmd, "./"THIRDPARTY"/wamr/wasm-micro-runtime/wamr-compiler/build/wamrc");
    cmd_append(&cmd, "--emit-custom-sections=name");
    cmd_append(&cmd, "-o", O_FAKE_BOARD_AOT, O_FAKE_BOARD_WASM);

    if ( !(res = cmd_run(&cmd)) ) goto end;



end:
    return res;
}

bool f_clean_fakeboard(void)
{
    bool res = true;

    if ( file_exists(O_FAKE_BOARD_WASM) ) delete_file(O_FAKE_BOARD_WASM);
    if ( file_exists(O_FAKE_BOARD_AOT) ) delete_file(O_FAKE_BOARD_AOT);

    return res;
}
#endif // FAKE_BOARD_IMPLEMENTATION
