#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define FAKE_BOARD_IMPLEMENTATION
#include "BuildDependencies/fake_board.h"

#define CLI_IMPLEMENTATION
#include "BuildDependencies/cli.h"

#include "BuildDependencies/virtual_interrupt.h"

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

    mkdir_if_not_exists(BUILD_DIR);

    if( args.build || args.run )
    {
        f_build_virtual_interrupt(args.verbose, args.test);
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

#define WAMR_IMPLEMENTATION
#include "BuildDependencies/wamr.h"

#define VINT_IMPLEMENTATION
#include "BuildDependencies/virtual_interrupt.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"
