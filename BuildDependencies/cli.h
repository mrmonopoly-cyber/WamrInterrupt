#include <stdbool.h>

#ifndef CLI_PREFIX
#define CLI_PREFIX
#endif // !CLI_PREFIX

typedef struct CCliUserArgs{
    bool help;
    bool verbose;
    bool test;
    bool run;
    bool build;
    bool clean;
}CliArgs;

CLI_PREFIX bool cli_parse(CliArgs* args, const int argc, char** argv);

#ifdef CLI_IMPLEMENTATION
#define CCLI_PREFIX CLI_PREFIX
#define CCLI_IMPLEMENTATION
#include "c_cli.h"

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

CLI_PREFIX void cli_default(CliArgs* const restrict args)
{
    args->test = false;
    args->build = true;
    args->run = true;
}

CLI_PREFIX bool cli_parse(CliArgs* args, const int argc, char** argv)
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

#endif
