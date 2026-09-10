#include <stdbool.h>

#include "nob.h"


#ifndef WASI_SDK_VERSION
#define WASI_SDK_VERSION "34"
#endif // !WASI_SDK_VERSION

#define WASI_SDK_NAME "wasi-sdk-"WASI_SDK_VERSION".0-x86_64-linux"
#define WASI_SDK_TAR WASI_SDK_NAME".tar.gz"
#define WASI_SDK_MIRROR "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-"WASI_SDK_VERSION"/"WASI_SDK_TAR


bool f_build_fakeboard(bool verbose, const char* path_main);

//======================================implementation==========================================

#ifdef FAKE_BOARD_IMPLEMENTATION
#include "defs.h"
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
    };

    if ( !file_exists(WASI_SDK_NAME"/VERSION") )
    {
        if ( !file_exists(WASI_SDK_NAME".tar.gz") )
        {
            cmd_append(&cmd, "wget", WASI_SDK_MIRROR);
            cmd_append(&cmd, "-O", WASI_SDK_TAR);
            if ( !(res = cmd_run(&cmd)) ) goto end;
        }

        cmd_append(&cmd, "tar");
        cmd_append(&cmd, "-xf", WASI_SDK_TAR);
        if ( !(res = cmd_run(&cmd)) ) goto end;
    }


    cmd_append(&cmd, "./"WASI_SDK_NAME"/bin/clang");
    cmd_append(&cmd, "--sysroot=./"WASI_SDK_NAME"/share/wasi-sysroot");
    cmd_append(&cmd, "--target=wasm32-wasip1-threads");
    cmd_append(&cmd, "-pthread");

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
    cmd_append(&cmd, "-o", "fake_board.wasm");

    cmd_append(&cmd, path_main);
    if ( !(res = cmd_run(&cmd)) ) goto end;

    //../wamrc_building/wasm-micro-runtime/wamr-compiler/build/wamrc --emit-custom-sections=name -o fake_board.aot fake_board.wasm

    cmd_append(&cmd, "./wamrc");
    cmd_append(&cmd, "--emit-custom-sections=name");
    cmd_append(&cmd, "-o", "fake_board.aot", "fake_board.wasm");

    if ( !(res = cmd_run(&cmd)) ) goto end;



end:
    return res;
}
#endif // FAKE_BOARD_IMPLEMENTATION
