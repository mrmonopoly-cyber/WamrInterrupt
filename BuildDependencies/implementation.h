#pragma once

#include "build_tools/implementation.h"

#ifndef BUILD_EXCLUDE_BUILDER
#define BUILDER_IMPLEMENTATION
#include "builder.h"
#endif // !BUILD_EXCLUDE_BUILDER

#ifndef BUILD_EXCLUDE_LSP
#define LSP_IMPLEMENTATION
#include "lsp.h"
#endif // !BUILD_EXCLUDE_LSP

#ifndef BUILD_EXCLUDE_DEPENDENCY
#define DEPENDENCY_IMPLEMENTATION
#include "dependency.h"
#endif // !BUILD_EXCLUDE_DEPENDENCY

#ifndef BUILD_EXCLUDE_GOTO_ROOT
#define GOTO_ROOT_IMPLEMENTATION
#include "goto_root.h"
#endif // !BUILD_EXCLUDE_GOTO_ROOT

#ifndef BUILD_EXCLUDE_DEFS
#define DEFS_IMPLEMENTATION
#include "defs.h"
#endif // !BUILD_EXCLUDE_DEFS

#ifndef BUILD_EXCLUDE_CLI
#define CLI_IMPLEMENTATION
#include "cli.h"
#endif // !BUILD_EXCLUDE_CLI

#ifndef BUILD_EXCLUDE_NOB
#define NOB_IMPLEMENTATION
#include "nob.h"
#endif // !BUILD_EXCLUDE_NOB
