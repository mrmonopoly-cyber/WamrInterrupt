
#ifndef CLI_PREFIX
#define CLI_PREFIX
#endif // !CLI_PREFIX

#define CCLI_PREFIX CLI_PREFIX
#include "c_cli.h"

#ifndef CLI_TYPES
#define CLI_TYPES
typedef struct CCliUserArgs{
    bool verbose;
    bool help;
    bool build;
    bool lsp;
    bool clean;
    bool clean_all;
    bool run;
}CliArgs;
#endif // !CLI_TYPES

CLI_PREFIX bool cli_parse(CliArgs* args, const int argc, char** argv);

//========================================implementation========================================

#ifdef CLI_IMPLEMENTATION

#define CCLI_IMPLEMENTATION
#include "c_cli.h"

CCLI_PARSER_DECLARE(build);
CCLI_PARSER_DECLARE(run);
CCLI_PARSER_DECLARE(lsp);
CCLI_PARSER_DECLARE(clean);
CCLI_PARSER_DECLARE(clean_all);

static CCliArgDef defs [] = 
{
    //--build, -b
    {
        .f_long = CCLI_LONG_FLAG(build),
        .f_short = CCLI_SHORT_FLAG(b),
        .f_args = CCLI_NO_ARG,
        .f_description = "build the program",
        .f_parser = CCLI_PARSER_NAME(build),
    },

    //--run, -r
    {
        .f_long = CCLI_LONG_FLAG(run),
        .f_short = CCLI_SHORT_FLAG(r),
        .f_args = CCLI_NO_ARG,
        .f_description = "run the program",
        .f_parser = CCLI_PARSER_NAME(run),
    },

    //--lsp, -lsp
    {
        .f_long = CCLI_LONG_FLAG(lsp),
        .f_short = CCLI_SHORT_FLAG(lsp),
        .f_args = CCLI_NO_ARG,
        .f_description = "configure the lsp",
        .f_parser = CCLI_PARSER_NAME(lsp),
    },

    //--clean, -c
    {
        .f_long = CCLI_LONG_FLAG(clean),
        .f_short = CCLI_SHORT_FLAG(c),
        .f_args = CCLI_NO_ARG,
        .f_description = "clean the project",
        .f_parser = CCLI_PARSER_NAME(clean),
    },

    //--clean-all, -call
    {
        .f_long = CCLI_LONG_FLAG(clean-all),
        .f_short = CCLI_SHORT_FLAG(call),
        .f_args = CCLI_NO_ARG,
        .f_description = "clean the project, also nob nob.old",
        .f_parser = CCLI_PARSER_NAME(clean_all),
    },
};

CLI_PREFIX void _cli_default(CliArgs* const restrict args)
{
    args->build = true;
}

CLI_PREFIX bool cli_parse(CliArgs* args, const int argc, char** argv)
{
    return c_cli_parse(defs, CCLI_ARRAYSIZE(defs), args, argc, argv, _cli_default);
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

CCLI_PARSER_DECLARE_FULL(lsp, args, ctx)
{
    args->build = true;
    args->lsp = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(clean, args, ctx)
{
    args->clean = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(clean_all, args, ctx)
{
    args->clean = true;
    args->clean_all = true;
    return CCliActionOK;
}

#endif // CLI_IMPLEMENTATION
