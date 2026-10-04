#if 0
if [[ ! -f ./nob ]]
then
echo "Bootrap: nob is not present"
cc -o nob nob.c;
fi
exec ./nob "$@"
exit 0
#endif

#include "BuildDependencies/build_dependencies.h"
#include "BuildDependencies/user/user.h"

static CliArgs args;

static bool f_link(void)
{
    BuilderLinkerOptions linker_opts = {0};
    ArrayViewString def_linker_opts = default_linker_opts();
    da_append_many(&linker_opts, def_linker_opts.data, def_linker_opts.len);

    return builder_link_file_to_obj(O_FILE,
            .linker_options = linker_opts,
            .lsp = args.lsp,
            );
}

static bool f_run()
{
    bool res= false;
    Cmd cmd = {0};
    Procs procs = {0};

    if ( !f_compile_launcher(PROJECT_ROOT"/src/launcher", &procs, args.lsp) ) goto end;
    if ( !f_build_fakeboard(args.verbose, PROJECT_ROOT"/src/fake_board_src/main.c") ) goto end;

    procs_flush(&procs);

    if ( !f_link_launcher(args.lsp) ) goto end;

    cmd_append(&cmd, "./main");
    cmd_append(&cmd, O_FAKE_BOARD_AOT);

    res = cmd_run(&cmd);

end:
    cmd_free(cmd);
    return res;
}

int main(int argc, char **argv)
{
    go_exec_yourself_on_project_root(argc, argv);
    go_rebuild_yourself_check_dir(argc, argv, PROJECT_ROOT"/nob.c", PROJECT_ROOT"/BuildDependencies");

    if ( !cli_parse(&args, argc, argv) ) return 1;

    nob_log(INFO, "build directory: %s", BUILD_DIR);
    nob_log(INFO, "output file: %s", O_FILE);

    if( !file_exists(BUILD_DIR) ) mkdir_if_not_exists(BUILD_DIR);

    if ( args.build || args.run )
    {
        Procs procs = {0};
        Procs* p_procs = args.lsp ? NULL : &procs;
        
        if ( !f_build_wamr(args.verbose, p_procs, args.lsp) )
        {
            return 1;
        }
        if ( !f_compile_vi_interrupt(PROJECT_ROOT"/src/virtual_interrupt", p_procs, args.lsp) )
        {
            return 1;
        }
        procs_flush(&procs);

        if ( !f_link_vi_interrupt(args.lsp) ) return 1;
    }


    if ( args.run && !f_run() )
    {
        nob_log(ERROR, "failed running");
        return 1;
    }

    if ( args.clean )
    {
        f_clean_fakeboard(false);
        if ( file_exists(O_FILE) ) delete_file(O_FILE);
        if ( file_exists(BUILD_DIR) ) clear_dir(BUILD_DIR);
    }
    
    if ( args.clean_all )
    {
        Dir_Entry dir = {0};
        const char* old = temp_sprintf("%s.old", argv[0]);

        UNUSED(delete_file(argv[0]));
        if ( file_exists(old) ) delete_file(old);

        lsp_clean();
        UNUSED(dependency_clear());
        UNUSED(f_clean_fakeboard(true));

        dir_entry_open(".", &dir);
        while( dir_entry_next(&dir) )
        {
            if (
                    get_file_type(dir.name) ==  FILE_REGULAR &&
                    dir.name[0] != '.' &&
                    file_has_suffix_with_null(dir.name, ".log")
               )
            {
                delete_file(dir.name);
            }
        }
        dir_entry_close(dir);

    }

  return 0;
}

#include "BuildDependencies/implementation.h"
