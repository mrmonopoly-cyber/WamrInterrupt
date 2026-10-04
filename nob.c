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

    cmd_append(&cmd, "./"O_FILE);
    res = cmd_run(&cmd);

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
        BuilderCompilerOptions comp_opts = {0};
        ArrayViewString def_comp_opts = default_compiler_opts();
        da_append_many(&comp_opts, def_comp_opts.data, def_comp_opts.len);

        //source directories
        FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
        {
            if( dir )
            {
                nob_log(INFO, "compiling sources in src: %s", dir);
                if(
                        !builder_compile_dir_files_to_obj(dir,
                            .suffix = ".c",
                            .comp_opt =
                            {
                            .async = &procs,
                            .lsp = args.lsp,
                            .compiler_options = comp_opts,
                            })
                  )
                {
                    nob_log(ERROR, "failed compiling sources in %s", dir);
                    return 1;
                }
            }
        }

        procs_flush(&procs);

        if( !f_link() )
        {
            nob_log(ERROR, "failed liking");
            return 1;
        }
    }


    if ( args.run && !f_run() )
    {
        nob_log(ERROR, "failed running");
        return 1;
    }

    if ( args.clean )
    {
        if ( file_exists(O_FILE) ) delete_file(O_FILE);
        if ( file_exists(BUILD_DIR) ) clear_dir(BUILD_DIR);
    }
    
    if ( args.clean_all )
    {
        const char* old = temp_sprintf("%s.old", argv[0]);

        UNUSED(delete_file(argv[0]));
        if ( file_exists(old) ) delete_file(old);

        lsp_clean();
        UNUSED(dependency_clear());
    }

  return 0;
}

#include "BuildDependencies/implementation.h"
