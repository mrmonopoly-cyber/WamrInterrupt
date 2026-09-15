#include <string.h>
#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define FAKE_BOARD_IMPLEMENTATION
#include "BuildDependencies/fake_board.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"

#define CLI_IMPLEMENTATION
#include "BuildDependencies/cli.h"

static CliArgs args;

typedef struct
{
    char** items;
    size_t count;
    size_t capacity;
}SourcesList;

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
    Cmd cmd = {0};
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;

    if(entry.type == FILE_REGULAR && !strcmp(suffix, ".c"))
    {
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CC);
        apply_all_defualt_compile_opts(&cmd);

        if ( args.test )
        {
            cmd_append(&cmd, "-DENABLE_TESTS");
        }

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));


        if ( args.test && !strcmp(file_name, "main.c" ) )
        {
            entry.path = "./BuildDependencies/dummy_main.c";
        }

        cmd_append(&cmd, entry.path);

        res = cmd_run(&cmd);

    }

end:
    cmd_free(cmd);
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

static bool _f_check_append_sources(Walk_Entry entry)
{
    Cmd cmd = {0};
    const char* name = temp_file_name(entry.path);
    const char* suffix = name + strlen(name) - 2;
    SourcesList* sources = entry.data;

    if(entry.type == FILE_REGULAR && !strcmp(suffix, ".c"))
    {
        da_append(sources, strdup(entry.path));
    }

    return true;
}

static bool f_check(void)
{
    bool res= false;
    Cmd cmd = {0};

    SourcesList sources = {0};

    cmd_append(&cmd, "clang-tidy");
    cmd_append(&cmd, "--warnings-as-errors=*");

    //source directories
    FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
    {
        if ( !(res = walk_dir(dir, _f_check_append_sources, .data = &sources)) )
        {
            goto end;
        }
    }

    for (size_t i=0; i<sources.count; i++)
    {
        cmd_append(&cmd, sources.items[i]);
    }

    cmd_append(&cmd, "--");

    //include path
    FOR_EACH_FAT_ARRAY_STR(default_include_path_opts(), path)
    {
        if(path) cmd_append(&cmd, temp_sprintf("-I%s", path));
    }

    res = cmd_run(&cmd);

end:
    for (size_t i=0; i<sources.count; i++)
    {
        free(sources.items[i]);
    }

    da_free(sources);
    cmd_free(cmd);
    return res;
}

static bool f_run(void)
{
    bool res = false;
    Cmd cmd = {0};

    if ( !args.test && !f_build_fakeboard(args.verbose, "./src/fake_board_src/main.c") )
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

    GO_REBUILD_URSELF_PLUS(argc, argv,
            "./BuildDependencies/fake_board.h",
            "./BuildDependencies/c_cli.h",
            "./BuildDependencies/cli.h",
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
        //wamr
        if(!f_build_wamr())
        {
            nob_log(ERROR, "failed building wamr");
            return 1;
        }

        if ( !f_check() )
        {
            nob_log(ERROR, "failed checking sources");
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
        if ( file_exists(O_FILE) ) delete_file(O_FILE);
    }


  return 0;
}
