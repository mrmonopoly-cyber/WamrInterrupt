#include <stdbool.h>

#ifndef GOTO_ROOT_PREFIX
#define GOTO_ROOT_PREFIX
#endif // !GOTO_ROOT_PREFIX

//===================================declarations================================================
 /**
  * find the project root, exec itself in there and than goes back to the starting location
  */
GOTO_ROOT_PREFIX void go_exec_yourself_on_project_root(int argc, char** argv);

// #define GOTO_ROOT_IMPLEMENTATION //enable for debugging
#ifdef GOTO_ROOT_IMPLEMENTATION
//===================================implementation==============================================

#include <string.h>

#include "defs.h"
#include "nob.h"

static struct
{
    const char* path;
    const char* root_path;
}ORIGINAL_ROOT;

static const char* _goto_root(void);

GOTO_ROOT_PREFIX void go_exec_yourself_on_project_root(int argc, char** argv)
{
    const char* binary_path = shift(argv, argc);
    const char* exec_name = temp_file_name(binary_path);
    if ( !file_exists(exec_name) )
    {
        const char* nob_root = _goto_root();
        Cmd cmd = {0};
        const char* nob_root_exec = temp_sprintf("%s/%s", nob_root, exec_name);

        if( !nob_root ) exit(1);
        set_current_dir(nob_root);
        
        cmd_append(&cmd, temp_sprintf("./%s", exec_name));
        da_append_many(&cmd, argv, argc);

        UNUSED(cmd_run(&cmd));
        exit(0);
    }

}

static const char* _goto_root(void)
{
    const char* temp_pwd = ORIGINAL_ROOT.path;
    const char* pwd = get_current_dir_temp();
    bool quit = false;

    if ( ORIGINAL_ROOT.path ) //root already set
    {
        return ORIGINAL_ROOT.root_path;
    }

    temp_pwd = pwd;

    while ( !quit )
    {
        size_t temp_mark = temp_save();
        const char* nob_path = temp_sprintf("%s/nob.c", temp_pwd );

        if ( file_exists(nob_path) )
        {
            ORIGINAL_ROOT.root_path = temp_pwd;
            break;
        }

        if ( !strcmp(temp_pwd, "/") )
        {
            nob_log(ERROR, "nob.c not found from : %s", pwd);
            return NULL;
        }

        if( !set_current_dir("..") )
        {
            nob_log(ERROR, "fail to go to parent: %s", temp_pwd);
            temp_rewind(temp_mark);
            return NULL;
        }

        temp_rewind(temp_mark);
        temp_pwd = get_current_dir_temp();
    }

    return ORIGINAL_ROOT.root_path;
}

#endif //GOTO_ROOT_IMPLEMENTATION
