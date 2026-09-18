#define FAKE_BOARD_IMPLEMENTATION
#include "BuildDependencies/fake_board.h"

#define VINT_IMPLEMENTATION
#include "BuildDependencies/virtual_interrupt.h"

#define CLI_IMPLEMENTATION
#include "BuildDependencies/cli.h"

static bool f_run(bool verbose, bool test)
{
    bool res = false;
    Cmd cmd = {0};

    if ( !test && !f_build_fakeboard(verbose, "./src/fake_board_src/main.c") )
    {
        nob_log(ERROR, "failed fake board");
        return 1;
    }

    cmd_append(&cmd, "./"O_FILE);
    cmd_append(&cmd, "./fake_board.aot");

    res = cmd_run(&cmd);

    cmd_free(cmd);
    return res;
}

static bool walk_delete(Walk_Entry entry)
{
    delete_file(entry.path);
    return true;
}

int main(int argc, char **argv)
{
    set_log_handler(&cancer_log_handler);
    CliArgs args;

    GO_REBUILD_URSELF_PLUS(argc, argv,
            "./BuildDependencies/fake_board.h",
            "./BuildDependencies/c_cli.h",
            "./BuildDependencies/cli.h",
            "./BuildDependencies/wamr.h",
            "./BuildDependencies/virtual_interrupt.h",
            "./BuildDependencies/defs.h"
            );

    if ( !cli_parse(&args, argc, argv) )
    {
        return 1;
    }

    nob_log(INFO, "build directory: %s", BUILD_DIR);
    nob_log(INFO, "output file: %s", O_FILE);

    if ( args.lsp )
    {
        walk_dir(BUILD_DIR, walk_delete, .post_order = true);
        if ( file_exists(O_FILE) ) delete_file(O_FILE);
        if ( file_exists("compile_commands.json") ) delete_file("compile_commands.json");
    }

    mkdir_if_not_exists(BUILD_DIR);

    if ( args.fetch && !vi_f_fetch_deps() )
    {
        nob_log(ERROR, "failed fetching dependencies");
        return 1;
    }

    if( args.build || args.run )
    {
        Cmd cmd = {0};
        const char* main_src = "src/launcher/main.c";
        if ( 
                !vi_f_build_virtual_interrupt(
                    args.verbose,
                    args.test,
                    args.lsp,
                    VIOutputFormat_StaticLib)
           )
        {
            nob_log(ERROR, "failed building");
            return 1;
        }

        if ( args.test )
        {
            const char* main_src = "./BuildDependencies/dummy_main.c";
        }

        cmd_append(&cmd, CC);
        vi_apply_all_defualt_compile_opts(&cmd);
        cmd_append(&cmd, main_src);
        vi_apply_all_defualt_linker_opts(&cmd);
        cmd_append(&cmd, "-o", O_FILE);
        cmd_append(&cmd, "-L", BUILD_DIR);
        cmd_append(&cmd, "-l", VI_OLIB_BASE_NAME);

        if ( !cmd_run(&cmd) )
        {
            nob_log(ERROR, "failed building");
            return 1;
        }
    }

    if( args.run && !f_run(args.verbose, args.test) )
    {
        nob_log(ERROR, "failed running");
        return 1;
    }

    if ( args.clean )
    {
        walk_dir(BUILD_DIR, walk_delete, .post_order = true);
        if ( file_exists(O_FILE) ) delete_file(O_FILE);
    }


  return 0;
}

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"
