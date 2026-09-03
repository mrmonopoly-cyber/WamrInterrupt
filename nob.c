#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"

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

    if ( !( res = cmd_run(&cmd) ) ) goto end;

end:
    cmd_free(cmd);
    return res;
}

static bool f_compile(Walk_Entry entry)
{
    bool res=true;

    if(entry.type == FILE_REGULAR)
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CC);

        apply_all_defualt_compile_opts(&cmd);

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

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF_PLUS(argc, argv,
            "./BuildDependencies/defs.h");

    nob_log(INFO, "build directory: %s\n", BUILD_DIR);
    nob_log(INFO, "output file: %s\n", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

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
            printf("compiling sources in src: %s\n", dir);
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

  return 0;
}
