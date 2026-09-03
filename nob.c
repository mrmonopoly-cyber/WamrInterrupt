#include <stdio.h>


#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"


static bool f_compile(Walk_Entry entry)
{
    bool res=true;

    if(entry.type == FILE_REGULAR)
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CC);

        //compiler options
        FOR_EACH_FAT_ARRAY_STR(default_compiler_opts(), opt)
        {
            if(opt) cmd_append(&cmd, opt);
        }

        //include path
        FOR_EACH_FAT_ARRAY_STR(default_include_path_opts(), path)
        {
            if(path) cmd_append(&cmd, temp_sprintf("-I%s", path));
        }

        //global definitions
        FOR_EACH_FAT_ARRAY(default_global_defs_opts(), def)
        {
            if(def && def->def)
            {
                if(def->val)
                {
                    cmd_append(&cmd, temp_sprintf("-D%s=%s", def->def, def->val));
                }
                else
                {
                    cmd_append(&cmd, temp_sprintf("-D%s", def->def));
                }
            }
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

    //linker options
    FOR_EACH_FAT_ARRAY_STR(default_linker_opts(), opt)
    {
        if(opt) cmd_append(&cmd, opt);
    }

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

    res = cmd_run(&cmd);

    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    printf("build directory: %s\n", BUILD_DIR);
    printf("output file: %s\n", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

    //source directories
    FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
    {
        if(dir)
        {
            printf("compiling sources in src: %s\n", dir);
            if(!walk_dir(dir, f_compile))
            {
                fprintf(stderr, "failed compiling sources in %s\n", dir);
                return 1;
            }
        }
    }

    if(!f_link())
    {
        fprintf(stderr, "failed liking\n");
        return 1;
    }

  return 0;
}
