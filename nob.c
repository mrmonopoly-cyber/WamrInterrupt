#include <string.h>
#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define FAKE_BOARD_IMPLEMENTATION
#include "BuildDependencies/fake_board.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"

#include "BuildDependencies/c_cli.h"

typedef struct CCliUserArgs{
    bool help;
    bool verbose;
    bool test;
    bool run;
    bool build;
    bool clean;
}CliArgs;

static CliArgs args;

static inline bool cli_parse(CliArgs* args, const int argc, char** argv);

static bool f_build_wamr(void)
{
    static char wamr_root_dir[128] = {0};

    bool res = false;
    Cmd cmd = {0};

    const char* pwd = get_current_dir_temp();
    const char* wamr_path = "./ThirdParty/wamr";
    snprintf(wamr_root_dir, sizeof(wamr_root_dir),
            "%s/ThirdParty/wamr/wasm-micro-runtime", pwd);

    nob_log(INFO, "wamr root is at %s", wamr_root_dir);

    const GDef build_gen_defs [] =
    {
        {"WAMR_BUILD_PLATFORM"                  , "linux"},
        {"WAMR_BUILD_TARGET"                    , "X86_64"},
        {"WAMR_ROOT_DIR"                        , wamr_root_dir},
        {"WAMR_BUILD_INTERP"                    , "0"},
        {"WAMR_BUILD_FAST_INTERP"               , "0"},
        {"WAMR_BUILD_AOT"                       , "1"},
        {"WAMR_BUILD_LIBC_BUILTIN"              , "1"},
        {"WAMR_BUILD_LIBC_WASI"                 , "1"},
        {"WAMR_BUILD_SIMD"                      , "0"},
        {"WAMR_BUILD_REF_TYPES"                 , "1"},
        {"WAMR_BUILD_THREAD_MGR"                , "1"},
        {"WAMR_BUILD_SHARED_MEMORY"             , "1"},
        {"WAMR_BUILD_LIB_PTHREAD"               , "1"},
        {"WAMR_BUILD_LIB_WASI_THREADS"          , "1"},
        {"WAMR_BUILD_LINUX_PERF"                , "0"},
        {"WAMR_BUILD_DEBUG_INTERP"              , "0"},
        {"WAMR_BUILD_LOAD_CUSTOM_SECTION"       , "1"},
        {"WAMR_BUILD_CUSTOM_NAME_SECTION"      , "1"},

        {"CMAKE_EXPORT_COMPILE_COMMANDS"        ,"ON"},
        {"CMAKE_BUILD_TYPE"                     ,"Release"},
    };

    const GDef feature_gen_defs [] =
    {
        {"WASM_ENABLE_THREAD_MGR"               , "1"},
        {"WASM_ENABLE_CUSTOM_NAME_SECTION"      , "1"},
    };

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "-S", wamr_path);
    cmd_append(&cmd, "-B", BUILD_DIR"/wamr");
    cmd_append(&cmd, "-G", "Ninja");

    apply_global_definitions(&cmd, (ArrayViewGDef) FAT_ARRAY_INIT(build_gen_defs));

    if ( !( res = cmd_run(&cmd) ) ) goto end;

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "--build", BUILD_DIR"/wamr");

    if ( args.verbose ) cmd_append(&cmd, "--verbose");

    if ( !( res = cmd_run(&cmd) ) ) goto end;

end:
    cmd_free(cmd);
    return res;
}

static bool f_compile(Walk_Entry entry)
{
    bool res=true;
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;

    if(entry.type == FILE_REGULAR && !strcmp(suffix, ".c"))
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);

        if ( args.test && !strcmp(file_name, "main.c" ))
        {
            entry.path = "./BuildDependencies/dummy_main.c";
        }

        cmd_append(&cmd, CC);

        apply_all_defualt_compile_opts(&cmd);

        if ( args.test )
        {
            cmd_append(&cmd, "-DENABLE_TESTS");
        }

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));

        cmd_append(&cmd, entry.path);

        res = cmd_run(&cmd);

        cmd_free(cmd);
    }

    return res;
}

static bool f_link(void)
{
    Dir_Entry dir = {0};
    Cmd cmd = {0};
    bool res = true;

    if(!dir_entry_open(BUILD_DIR, &dir)) return false;

    cmd_append(&cmd, CC);


    cmd_append(&cmd, "-o", O_FILE);

    while(dir_entry_next(&dir))
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (FILE_REGULAR == get_file_type(file_path))
        {
            printf("found %s\n", file_path);
            cmd_append(&cmd, file_path);
        }
    }

    apply_all_defualt_linker_opts(&cmd);

    res = cmd_run(&cmd);

    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

static bool f_run(void)
{
    bool res = false;
    Cmd cmd = {0};

    if ( !args.test && !f_build_fakeboard(args.verbose, "./fake_board_src/main.c") )
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
    GO_REBUILD_URSELF_PLUS(argc, argv,
            "./BuildDependencies/fake_board.h",
            "./BuildDependencies/c_cli.h",
            "./BuildDependencies/defs.h"
            );

    if ( !cli_parse(&args, argc, argv) )
    {
        return 1;
    }

    nob_log(INFO, "build directory: %s", BUILD_DIR);
    nob_log(INFO, "output file: %s", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

    if(args.build)
    {
        //wamr
        if(!f_build_wamr())
        {
            nob_log(ERROR, "failed building wamr");
            return 1;
        }

        //source directories
        FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
        {
            if(dir)
            {
                nob_log(INFO, "compiling sources in src: %s", dir);
                if(!walk_dir(dir, f_compile))
                {
                    nob_log(ERROR, "failed compiling sources in %s", dir);
                    return 1;
                }
            }
        }

        if(!f_link())
        {
            nob_log(ERROR, "failed liking");
            return 1;
        }
    }


    if( args.run && !f_run() )
    {
        nob_log(ERROR, "failed running");
        return 1;
    }

    if ( args.clean )
    {
        walk_dir(BUILD_DIR, walk_delete, .post_order = true);
    }


  return 0;
}


#define CCLI_IMPLEMENTATION
#include "BuildDependencies/c_cli.h"

CCLI_PARSER_DECLARE(test);
CCLI_PARSER_DECLARE(build);
CCLI_PARSER_DECLARE(run);
CCLI_PARSER_DECLARE(clean);

static const CCliArgDef defs[] = 
{
    //--test, -t
    {
        .f_long = CCLI_LONG_FLAG(test),
        .f_short = CCLI_SHORT_FLAG(t),
        .f_args = CCLI_NO_ARG,
        .f_description = "run the tests",
        .f_parser = CCLI_PARSER_NAME(test),
    },

    //--build, -b
    {
        .f_long = CCLI_LONG_FLAG(build),
        .f_short = CCLI_SHORT_FLAG(b),
        .f_args = CCLI_NO_ARG,
        .f_description = "build the sources",
        .f_parser = CCLI_PARSER_NAME(build),
    },

    //--run, -r
    {
        .f_long = CCLI_LONG_FLAG(run),
        .f_short = CCLI_SHORT_FLAG(r),
        .f_args = CCLI_NO_ARG,
        .f_description = "run the sources",
        .f_parser = CCLI_PARSER_NAME(run),
    },

    //--clean, -c
    {
        .f_long = CCLI_LONG_FLAG(clean),
        .f_short = CCLI_SHORT_FLAG(c),
        .f_args = CCLI_NO_ARG,
        .f_description = "clean the sources",
        .f_parser = CCLI_PARSER_NAME(clean),
    },
};

static void cli_default(CliArgs* const restrict args)
{
    args->test = false;
    args->build = true;
    args->run = true;
}

static inline bool cli_parse(CliArgs* args, const int argc, char** argv)
{
    return c_cli_parse(defs, CCLI_ARRAYSIZE(defs), args, argc, argv, cli_default);
}

CCLI_PARSER_DECLARE_FULL(test, args, ctx)
{
    args->build = true;
    args->run= true;
    args->test = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(build, args, ctx)
{
    args->build = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(run, args, ctx)
{
    args->run = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(clean, args, ctx)
{
    args->clean = true;
    return CCliActionOK;
}
