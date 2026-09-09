#include <stdbool.h>

#include "nob.h"

#ifndef WASI_SDK
#define WASI_SDK "../wasi-sdk-27.0-x86_64-linux"
#endif // !WASI_SDK

bool f_build_fakeboard(bool verbose, const char* path_main);

//======================================implementation==========================================

#ifdef FAKE_BOARD_IMPLEMENTATION
#include "defs.h"
bool f_build_fakeboard(bool verbose, const char* path_main)
{
    const char* exported_functions[] = 
    {
        "board_main",
        "led_value",
        "new_led_value",
        "led_value_i1",
        "led_value_i2",
    };

    bool res = false;
    Cmd cmd = {0};

    cmd_append(&cmd, WASI_SDK"/bin/clang");
    cmd_append(&cmd, "--sysroot="WASI_SDK"/share/wasi-sysroot");
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
