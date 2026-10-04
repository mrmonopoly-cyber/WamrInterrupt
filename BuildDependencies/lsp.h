#include <stdbool.h>

#ifndef LSP_PREFIX
#define LSP_PREFIX
#endif // !LSP_PREFIX

#include "nob.h"

LSP_PREFIX bool lsp_configure(const Cmd* build_command);
LSP_PREFIX void lsp_clean(void);

// #define LSP_IMPLEMENTATION //enable for debugging
#ifdef LSP_IMPLEMENTATION

#include "dependency.h"

LSP_PREFIX bool lsp_configure(const Cmd* build_command)
{
    bool res = false;
    Cmd cmd = {0};
    const char* bear_path = NULL;

    if ( (bear_path = check_dependency("bear")) )
    {
        cmd_append(&cmd, bear_path);

        if ( file_exists("./compile_commands.json") )
        {
            cmd_append(&cmd, "--append");
        }

        cmd_append(&cmd, "--");

        da_foreach(const char*, command_arg, build_command)
        {
            cmd_append(&cmd, *command_arg);
        }

        res = cmd_run(&cmd);
    }
    else
    {
        nob_log(ERROR, "bear not found in your system. Abort");
    }

    cmd_free(cmd);
    return res;
}

LSP_PREFIX void lsp_clean(void)
{
    if ( file_exists("compile_commands.json") )
    {
        delete_file("compile_commands.json");
    }
}

#endif // LSP_IMPLEMENTATION
