#ifndef TEMPLATE_PREFIX
#define TEMPLATE_PREFIX
#endif // !TEMPLATE_PREFIX

#include <stdbool.h>

#include "../defs.h"
#include "../nob.h"

#ifndef TEMPLATE_TYPES
//===================================types=======================================================
typedef struct
{
    const char* cc;
    const char* o_file;
    const char* builder; //INFO: name WITHOUT extensions. Example: Ok: nob, Wrong: nob.c
    struct
    {
        int argc;
        const char** argv;
    }cli_args;
    Procs* async;
}ExternalProjectTemplateOpt;
#define TEMPLATE_TYPES

#endif // !TEMPLATE_TYPES

//===================================declarations================================================

TEMPLATE_PREFIX bool
_external_project_template(
        const char* project_root,
        const char* build_dir,
        const ExternalProjectTemplateOpt opt);
#define external_project_template(ROOT, BUILD_DIR, ...) \
    _external_project_template((ROOT), (BUILD_DIR), ((ExternalProjectTemplateOpt) {__VA_ARGS__}))

#ifdef TEMPLATE_IMPLEMENTATION
//===================================implementation==============================================
#include <assert.h>
#include <stdlib.h>
#include <string.h>

TEMPLATE_PREFIX bool
_external_project_template(
        const char* project_root,
        const char* build_dir,
        const ExternalProjectTemplateOpt opt)
{
    bool res = false;
    Cmd cmd = {0};

    const ArrayViewString argv_ref = {.data = opt.cli_args.argv, .len = opt.cli_args.argc};
    const char* prog_name = temp_file_name(project_root);
    const char* builder = opt.builder ? opt.builder : "nob";
    const char* full_prog_name = strdup(temp_sprintf("./%s/%s_%s", BUILD_DIR, builder, prog_name));
    assert( full_prog_name );

    if ( !project_root || !build_dir ) goto end;

    if ( !file_exists(build_dir) && !mkdir_if_not_exists(build_dir) )
    {
        goto end;
    }

    cmd_append(&cmd, CC);

    if ( opt.cc )
    {
        cmd_append(&cmd, temp_sprintf("-DCC=\"%s\"", opt.cc));
    }

    if ( opt.o_file )
    {
        cmd_append(&cmd, temp_sprintf("-DO_FILE=\"%s\"", opt.o_file));
    }

    cmd_append(&cmd, temp_sprintf("-DPROJECT_ROOT=\"%s\"", project_root));
    cmd_append(&cmd, temp_sprintf("-DBUILD_DIR=\"%s\"", build_dir));

    cmd_append(&cmd, "-o", full_prog_name);
    cmd_append(&cmd, temp_sprintf("%s/%s.c", project_root, builder));
    if ( !(res = cmd_run(&cmd)) ) goto end;

    cmd_append(&cmd, temp_sprintf("./%s", full_prog_name));
    FOR_EACH_FAT_ARRAY_STR(argv_ref, arg)
    {
        cmd_append(&cmd, arg);
    }
    if ( !(res = cmd_run(&cmd, .async = opt.async)) ) goto end;

end:
    cmd_free(cmd);
    if ( full_prog_name )       free((void*)full_prog_name);
    return res;
}

//==================================internal declarations========================================
#endif //TEMPLATE_IMPLEMENTATION
