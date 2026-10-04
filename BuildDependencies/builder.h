#ifndef BUILDER_PREFIX
#define BUILDER_PREFIX
#endif // !BUILDER_PREFIX

#include <stdbool.h>
#include <stddef.h>

#include "nob.h"

//=========================================macros=================================================

//=========================================types==================================================
#ifndef BUILDER_TYPES
#define BUILDER_TYPES
typedef struct
{
    const char** items;
    size_t count;
    size_t capacity;
}BuilderCompilerOptions;

typedef struct
{
    const char* compiler;
    const char* build_dir;
    const char* o_file;
    BuilderCompilerOptions compiler_options;
    Procs* async;
    bool lsp;
}BuilderCompileFileOpt;

typedef struct
{
    BuilderCompileFileOpt comp_opt;
    const char* suffix;
}BuilderCompileDirFilesOpt;

typedef struct
{
    char** items;
    size_t count;
    size_t capacity;
}BuilderLinkerOptions;

typedef struct
{
    const char* linker;
    const char* build_dir;
    BuilderLinkerOptions linker_options;
    Procs* async;
    bool lsp;
}BuilderLinkFileOpt;
#endif // !BUILDER_TYPES

//===================================declarations================================================

BUILDER_PREFIX bool
_builder_compile_file_to_obj(const char* file_path, BuilderCompileFileOpt opt);
#define builder_compile_file_to_obj(FILE_PATH, ...) \
    _builder_compile_file_to_obj((FILE_PATH), ((BuilderCompileFileOpt) {__VA_ARGS__}))

BUILDER_PREFIX bool
_builder_compile_dir_files_to_obj(const char* dir_path, BuilderCompileDirFilesOpt opt);
#define builder_compile_dir_files_to_obj(FILE_PATH, ...) \
    _builder_compile_dir_files_to_obj((FILE_PATH), ((BuilderCompileDirFilesOpt) {__VA_ARGS__}))

BUILDER_PREFIX bool
_builder_link_obj_files(const char* o_file, BuilderLinkFileOpt opt);
#define builder_link_file_to_obj(O_FILE, ...) \
    _builder_link_obj_files((O_FILE), ((BuilderLinkFileOpt){__VA_ARGS__}))

//===================================implementation==============================================
// #define BUILDER_IMPLEMENTATION //enable for debugging
#ifdef BUILDER_IMPLEMENTATION

#include "lsp.h"
#include "defs.h"

BUILDER_PREFIX bool
_builder_compile_file_to_obj(const char* file_path, BuilderCompileFileOpt opt)
{
    const size_t mark = temp_save();
    bool res = false;
    Cmd cmd = {0};

    const char* file_name = NULL;
    const char* cc = opt.compiler ? opt.compiler : CC;
    const char* obj_file = opt.o_file;
    const char* build_dir = opt.build_dir ? opt.build_dir : BUILD_DIR;

    if ( !file_path ) goto end;

    file_name = temp_strdup(temp_file_name(file_path));
    if ( !obj_file )
    {
        obj_file = temp_strdup(temp_sprintf("%s/%s.o", build_dir, file_name));
    }

    NOB_ASSERT( obj_file );
    NOB_ASSERT( file_name );
    NOB_ASSERT( cc );
    NOB_ASSERT( build_dir );

    if ( !file_exists(build_dir) && !mkdir_if_not_exists(build_dir) ) goto end;

    res = true;
    if ( opt.lsp || needs_rebuild1(obj_file, file_path) )
    {
        cmd_append(&cmd, cc);
        cmd_append(&cmd, "-c");
        da_append_many(&cmd, opt.compiler_options.items, opt.compiler_options.count);
        cmd_append(&cmd, "-o");
        cmd_append(&cmd, obj_file);

        cmd_append(&cmd, file_path);

        if ( opt.lsp )
        {
            res = lsp_configure(&cmd);
        }
        else
        {
            res = cmd_run(&cmd, .async = opt.async);
        }

    }

end:
    temp_rewind(mark);
    cmd_free(cmd);
    return res;
}

static bool _builder_walk_compile(Walk_Entry entry)
{
    BuilderCompileDirFilesOpt* comp_args = entry.data;
    bool res=true;
    bool valid_suffix =
        !comp_args->suffix ||
        file_has_suffix_with_null(entry.path, comp_args->suffix);

    NOB_ASSERT( comp_args );

    if( entry.type == FILE_REGULAR && valid_suffix )
    {
        res = _builder_compile_file_to_obj(entry.path, comp_args->comp_opt);
    }

    return res;
}

BUILDER_PREFIX bool
_builder_compile_dir_files_to_obj(const char* dir_path, BuilderCompileDirFilesOpt opt)
{
    bool res = false;

    if ( !dir_path ) goto end;
    res = walk_dir(dir_path, _builder_walk_compile, .data = &opt);

end:
    return res;
}

BUILDER_PREFIX bool
_builder_link_obj_files(const char* o_file, BuilderLinkFileOpt opt)
{
    bool res = false;
    size_t mark = temp_save();
    Cmd cmd = {0};
    Dir_Entry dir = {0};
    const char* linker = opt.linker ? opt.linker : CC;
    const char* build_dir = opt.build_dir ? opt.build_dir : BUILD_DIR;

    NOB_ASSERT( o_file );
    NOB_ASSERT( linker );
    NOB_ASSERT( build_dir );

    if ( !file_exists(build_dir) && !mkdir_if_not_exists(build_dir) ) goto end;

    if( !dir_entry_open(build_dir, &dir) ) goto end;

    cmd_append(&cmd, linker);
    cmd_append(&cmd, "-o", o_file);

    while( dir_entry_next(&dir) )
    {
        const char* file_path = temp_sprintf("%s/%s", build_dir, dir.name);
        if (
                get_file_type(file_path) ==  FILE_REGULAR &&
                file_has_suffix_with_null(file_path, ".o")
           )
        {
            cmd_append(&cmd, file_path);
        }
    }

    da_append_many(&cmd, opt.linker_options.items, opt.linker_options.count);

    if ( opt.lsp )
    {
        res = lsp_configure(&cmd);
    }
    else
    {
        res = cmd_run(&cmd, .async = opt.async);
    }


end:
    dir_entry_close(dir);
    temp_rewind(mark);
    cmd_free(cmd);
    return res;
}

#endif //BUILDER_IMPLEMENTATION
