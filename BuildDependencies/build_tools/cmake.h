#ifndef CMAKE_PREFIX
#define CMAKE_PREFIX
#endif // !CMAKE_PREFIX

#include "../defs.h"
#include "../nob.h"

//=========================================macros=================================================
#ifndef CMAKE_VERSION
#define CMAKE_VERSION "4.4.3"
#endif // !CMAKE_VERSION
#define CMAKE_ARCHIVE_NAME "cmake-" CMAKE_VERSION"-linux-x86_64"
#define CMAKE_ARCHIVE CMAKE_ARCHIVE_NAME".tar.gz"
#define CMAKE_MIRROR \
    "https://github.com/Kitware/CMake/releases/download/v"CMAKE_VERSION"/"CMAKE_ARCHIVE

#ifndef CMAKE_TYPES
#define CMAKE_TYPES
//=========================================types==================================================
typedef enum
{
   CmakeGenerator_Ninja                 = 0, //INFO: default
   CmakeGenerator_UnixMakefiles,
   CmakeGenerator_FASTBuild,

   _CmakeGenerator__Count,
}CmakeGenerator;

typedef struct
{
    CmakeGenerator generator;
    ArrayViewGDef global_defs;
}CmakeConfigureOpt;

typedef struct
{
    bool verbose;
    size_t jobs;
    Procs* async;
}CmakeBuildOpt;

#endif // !CMAKE_TYPES

//===================================declarations================================================

CMAKE_PREFIX bool
_cmake_configure(const char* CMakeLists_path, const char* build_dir, const CmakeConfigureOpt opt);
#define cmake_configure(CMAKELISTS_PATH, BUILD_DIR, ...) \
    _cmake_configure( (CMAKELISTS_PATH), (BUILD_DIR), ((CmakeConfigureOpt) {__VA_ARGS__}))

CMAKE_PREFIX bool
_cmake_build(const char* build_dir, const CmakeBuildOpt opt);
#define cmake_build(BUILD_DIR, ...) _cmake_build( (BUILD_DIR), ((CmakeBuildOpt) {__VA_ARGS__}))

CMAKE_PREFIX const char* cmake_get(void);

// #define CMAKE_IMPLEMENTATION //enable for debugging
#ifdef CMAKE_IMPLEMENTATION
//===================================implementation==============================================

#include "../dependency.h"

//==================================internal declarations========================================
CMAKE_PREFIX const char* _cmake_generator_to_str(const CmakeGenerator gen);
CMAKE_PREFIX const char* _cmake_generator_prog(const CmakeGenerator gen);
CMAKE_PREFIX const char* cmake_get(void);

CMAKE_PREFIX bool
_cmake_configure(const char* CMakeLists_path, const char* build_dir, const CmakeConfigureOpt opt)
{
    bool res = false;
    const char* generator = _cmake_generator_to_str(opt.generator);
    Cmd cmd = {0};
    const char* cmake = NULL;

    if ( 
            !CMakeLists_path ||
            !build_dir ||
            !(cmake= cmake_get())
       )
    {
        goto end;
    }

    cmd_append(&cmd, cmake);

    cmd_append(&cmd, "-S", CMakeLists_path);
    cmd_append(&cmd, "-B", build_dir);

    cmd_append(&cmd, "-G");
    if ( check_dependency(_cmake_generator_prog(opt.generator)) )
    {
        cmd_append(&cmd, generator);
    }
    else
    {
        generator = _cmake_generator_to_str(CmakeGenerator_UnixMakefiles);
        if ( check_dependency(_cmake_generator_prog(CmakeGenerator_UnixMakefiles)) )
        {
            cmd_append(&cmd, generator);
        }
        else
        {
            nob_log(ERROR, 
                    "cmake: both the generators: "
                    "%s and %s are not available in your system. Abort",
                    _cmake_generator_to_str(opt.generator),
                    _cmake_generator_to_str(CmakeGenerator_UnixMakefiles));
        }
    }

    apply_global_definitions(&cmd, opt.global_defs);

    res = cmd_run(&cmd);

end:
    cmd_free(cmd);
    return res;
}

CMAKE_PREFIX bool _cmake_build(const char* build_dir, const CmakeBuildOpt opt)
{
    bool res = false;
    Cmd cmd = {0};
    const size_t jobs = opt.jobs && opt.jobs < (size_t) nprocs() ? opt.jobs : (size_t) nprocs();
    const char* cmake = NULL;

    if ( !build_dir || !(cmake = cmake_get()) )
    {
        goto end;
    }

    cmd_append(&cmd, cmake);
    cmd_append(&cmd, "--build", build_dir);
    if ( opt.verbose ) cmd_append(&cmd, "--verbose");
    cmd_append(&cmd, "-j", temp_sprintf("%zu", jobs));

    res = cmd_run(&cmd, .async = opt.async);

end:
    cmd_free(cmd);
    return res;
}

//==================================internal implementation======================================

CMAKE_PREFIX const char* _cmake_generator_to_str(const CmakeGenerator gen)
{
    switch (gen)
    {
        case CmakeGenerator_Ninja:                  return "Ninja";
        case CmakeGenerator_UnixMakefiles:          return "Unix Makefiles";
        case CmakeGenerator_FASTBuild:              return "fbuild";
        case _CmakeGenerator__Count:                NOB_ASSERT( 0 && "unreachable");
    }
    NOB_ASSERT( 0 && "unreachable");
}

CMAKE_PREFIX const char* _cmake_generator_prog(const CmakeGenerator gen)
{
    switch (gen)
    {
        case CmakeGenerator_Ninja:                  return "ninja";
        case CmakeGenerator_UnixMakefiles:          return "make";
        case CmakeGenerator_FASTBuild:              return "FASTBuild";
        case _CmakeGenerator__Count:                NOB_ASSERT( 0 && "unreachable");
          break;
    }
    NOB_ASSERT( 0 && "unreachable");
}

CMAKE_PREFIX bool _cmake_installer(const char* work_dir, const char* bin_dst_path)
{
    static char temp_buffer_in[1024] = {0};
    static char temp_buffer_out[1024] = {0};
    bool res = false;
    const char* tar = NULL;
    Cmd cmd = {0};

    NOB_ASSERT( work_dir );
    NOB_ASSERT( bin_dst_path );

    if ( !(tar = check_dependency("tar")) ) goto end;

    snprintf(temp_buffer_in, sizeof(temp_buffer_in), "%s/%s", work_dir, CMAKE_ARCHIVE);
    cmd_append(&cmd, tar);
    cmd_append(&cmd, "-C", work_dir);
    cmd_append(&cmd, "-xf", temp_buffer_in);
    if ( !(res = cmd_run(&cmd)) ) goto end;

    snprintf(temp_buffer_in, sizeof(temp_buffer_in), "%s/%s", work_dir, CMAKE_ARCHIVE_NAME);
    snprintf(temp_buffer_out, sizeof(temp_buffer_out), "%s", bin_dst_path);
    if ( !nob_copy_directory_recursively(temp_buffer_in, temp_buffer_out) ) 
    {
        goto end;
    }

    res = true;
end:
    cmd_free(cmd);
    return res;
}

CMAKE_PREFIX const char* cmake_get(void)
{
    return check_dependency(
                "cmake",
                .download_mirror = CMAKE_MIRROR,
                .installer_f = _cmake_installer,
                );
}

#endif //CMAKE_IMPLEMENTATION
