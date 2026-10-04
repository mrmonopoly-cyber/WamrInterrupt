/*
 * author: Alberto Damo
 * repository: https://github.com/mrmonopoly-cyber/c_cli.git
 *
 * GNU GENERAL PUBLIC LICENSE
 *                        Version 2, June 1991
 * 
 *  Copyright (C) 1989, 1991 Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *  Everyone is permitted to copy and distribute verbatim copies
 *  of this license document, but changing it is not allowed.
 * 
 *                             Preamble
 * 
 *   The licenses for most software are designed to take away your
 * freedom to share and change it.  By contrast, the GNU General Public
 * License is intended to guarantee your freedom to share and change free
 * software--to make sure the software is free for all its users.  This
 * General Public License applies to most of the Free Software
 * Foundation's software and to any other program whose authors commit to
 * using it.  (Some other Free Software Foundation software is covered by
 * the GNU Lesser General Public License instead.)  You can apply it to
 * your programs, too.
 * 
 *   When we speak of free software, we are referring to freedom, not
 * price.  Our General Public Licenses are designed to make sure that you
 * have the freedom to distribute copies of free software (and charge for
 * this service if you wish), that you receive source code or can get it
 * if you want it, that you can change the software or use pieces of it
 * in new free programs; and that you know you can do these things.
 * 
 *   To protect your rights, we need to make restrictions that forbid
 * anyone to deny you these rights or to ask you to surrender the rights.
 * These restrictions translate to certain responsibilities for you if you
 * distribute copies of the software, or if you modify it.
 * 
 *   For example, if you distribute copies of such a program, whether
 * gratis or for a fee, you must give the recipients all the rights that
 * you have.  You must make sure that they, too, receive or can get the
 * source code.  And you must show them these terms so they know their
 * rights.
 * 
 *   We protect your rights with two steps: (1) copyright the software, and
 * (2) offer you this license which gives you legal permission to copy,
 * distribute and/or modify the software.
 * 
 *   Also, for each author's protection and ours, we want to make certain
 * that everyone understands that there is no warranty for this free
 * software.  If the software is modified by someone else and passed on, we
 * want its recipients to know that what they have is not the original, so
 * that any problems introduced by others will not reflect on the original
 * authors' reputations.
 * 
 *   Finally, any free program is threatened constantly by software
 * patents.  We wish to avoid the danger that redistributors of a free
 * program will individually obtain patent licenses, in effect making the
 * program proprietary.  To prevent this, we have made it clear that any
 * patent must be licensed for everyone's free use or not licensed at all.
 * 
 *   The precise terms and conditions for copying, distribution and
 * modification follow.
 * 
 *                     GNU GENERAL PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 
 *   0. This License applies to any program or other work which contains
 * a notice placed by the copyright holder saying it may be distributed
 * under the terms of this General Public License.  The "Program", below,
 * refers to any such program or work, and a "work based on the Program"
 * means either the Program or any derivative work under copyright law:
 * that is to say, a work containing the Program or a portion of it,
 * either verbatim or with modifications and/or translated into another
 * language.  (Hereinafter, translation is included without limitation in
 * the term "modification".)  Each licensee is addressed as "you".
 * 
 * Activities other than copying, distribution and modification are not
 * covered by this License; they are outside its scope.  The act of
 * running the Program is not restricted, and the output from the Program
 * is covered only if its contents constitute a work based on the
 * Program (independent of having been made by running the Program).
 * Whether that is true depends on what the Program does.
 * 
 *   1. You may copy and distribute verbatim copies of the Program's
 * source code as you receive it, in any medium, provided that you
 * conspicuously and appropriately publish on each copy an appropriate
 * copyright notice and disclaimer of warranty; keep intact all the
 * notices that refer to this License and to the absence of any warranty;
 * and give any other recipients of the Program a copy of this License
 * along with the Program.
 * 
 * You may charge a fee for the physical act of transferring a copy, and
 * you may at your option offer warranty protection in exchange for a fee.
 * 
 *   2. You may modify your copy or copies of the Program or any portion
 * of it, thus forming a work based on the Program, and copy and
 * distribute such modifications or work under the terms of Section 1
 * above, provided that you also meet all of these conditions:
 * 
 *     a) You must cause the modified files to carry prominent notices
 *     stating that you changed the files and the date of any change.
 * 
 *     b) You must cause any work that you distribute or publish, that in
 *     whole or in part contains or is derived from the Program or any
 *     part thereof, to be licensed as a whole at no charge to all third
 *     parties under the terms of this License.
 * 
 *     c) If the modified program normally reads commands interactively
 *     when run, you must cause it, when started running for such
 *     interactive use in the most ordinary way, to print or display an
 *     announcement including an appropriate copyright notice and a
 *     notice that there is no warranty (or else, saying that you provide
 *     a warranty) and that users may redistribute the program under
 *     these conditions, and telling the user how to view a copy of this
 *     License.  (Exception: if the Program itself is interactive but
 *     does not normally print such an announcement, your work based on
 *     the Program is not required to print an announcement.)
 * 
 * These requirements apply to the modified work as a whole.  If
 * identifiable sections of that work are not derived from the Program,
 * and can be reasonably considered independent and separate works in
 * themselves, then this License, and its terms, do not apply to those
 * sections when you distribute them as separate works.  But when you
 * distribute the same sections as part of a whole which is a work based
 * on the Program, the distribution of the whole must be on the terms of
 * this License, whose permissions for other licensees extend to the
 * entire whole, and thus to each and every part regardless of who wrote it.
 * 
 * Thus, it is not the intent of this section to claim rights or contest
 * your rights to work written entirely by you; rather, the intent is to
 * exercise the right to control the distribution of derivative or
 * collective works based on the Program.
 * 
 * In addition, mere aggregation of another work not based on the Program
 * with the Program (or with a work based on the Program) on a volume of
 * a storage or distribution medium does not bring the other work under
 * the scope of this License.
 * 
 *   3. You may copy and distribute the Program (or a work based on it,
 * under Section 2) in object code or executable form under the terms of
 * Sections 1 and 2 above provided that you also do one of the following:
 * 
 *     a) Accompany it with the complete corresponding machine-readable
 *     source code, which must be distributed under the terms of Sections
 *     1 and 2 above on a medium customarily used for software interchange; or,
 * 
 *     b) Accompany it with a written offer, valid for at least three
 *     years, to give any third party, for a charge no more than your
 *     cost of physically performing source distribution, a complete
 *     machine-readable copy of the corresponding source code, to be
 *     distributed under the terms of Sections 1 and 2 above on a medium
 *     customarily used for software interchange; or,
 * 
 *     c) Accompany it with the information you received as to the offer
 *     to distribute corresponding source code.  (This alternative is
 *     allowed only for noncommercial distribution and only if you
 *     received the program in object code or executable form with such
 *     an offer, in accord with Subsection b above.)
 * 
 * The source code for a work means the preferred form of the work for
 * making modifications to it.  For an executable work, complete source
 * code means all the source code for all modules it contains, plus any
 * associated interface definition files, plus the scripts used to
 * control compilation and installation of the executable.  However, as a
 * special exception, the source code distributed need not include
 * anything that is normally distributed (in either source or binary
 * form) with the major components (compiler, kernel, and so on) of the
 * operating system on which the executable runs, unless that component
 * itself accompanies the executable.
 * 
 * If distribution of executable or object code is made by offering
 * access to copy from a designated place, then offering equivalent
 * access to copy the source code from the same place counts as
 * distribution of the source code, even though third parties are not
 * compelled to copy the source along with the object code.
 * 
 *   4. You may not copy, modify, sublicense, or distribute the Program
 * except as expressly provided under this License.  Any attempt
 * otherwise to copy, modify, sublicense or distribute the Program is
 * void, and will automatically terminate your rights under this License.
 * However, parties who have received copies, or rights, from you under
 * this License will not have their licenses terminated so long as such
 * parties remain in full compliance.
 * 
 *   5. You are not required to accept this License, since you have not
 * signed it.  However, nothing else grants you permission to modify or
 * distribute the Program or its derivative works.  These actions are
 * prohibited by law if you do not accept this License.  Therefore, by
 * modifying or distributing the Program (or any work based on the
 * Program), you indicate your acceptance of this License to do so, and
 * all its terms and conditions for copying, distributing or modifying
 * the Program or works based on it.
 * 
 *   6. Each time you redistribute the Program (or any work based on the
 * Program), the recipient automatically receives a license from the
 * original licensor to copy, distribute or modify the Program subject to
 * these terms and conditions.  You may not impose any further
 * restrictions on the recipients' exercise of the rights granted herein.
 * You are not responsible for enforcing compliance by third parties to
 * this License.
 * 
 *   7. If, as a consequence of a court judgment or allegation of patent
 * infringement or for any other reason (not limited to patent issues),
 * conditions are imposed on you (whether by court order, agreement or
 * otherwise) that contradict the conditions of this License, they do not
 * excuse you from the conditions of this License.  If you cannot
 * distribute so as to satisfy simultaneously your obligations under this
 * License and any other pertinent obligations, then as a consequence you
 * may not distribute the Program at all.  For example, if a patent
 * license would not permit royalty-free redistribution of the Program by
 * all those who receive copies directly or indirectly through you, then
 * the only way you could satisfy both it and this License would be to
 * refrain entirely from distribution of the Program.
 * 
 * If any portion of this section is held invalid or unenforceable under
 * any particular circumstance, the balance of the section is intended to
 * apply and the section as a whole is intended to apply in other
 * circumstances.
 * 
 * It is not the purpose of this section to induce you to infringe any
 * patents or other property right claims or to contest validity of any
 * such claims; this section has the sole purpose of protecting the
 * integrity of the free software distribution system, which is
 * implemented by public license practices.  Many people have made
 * generous contributions to the wide range of software distributed
 * through that system in reliance on consistent application of that
 * system; it is up to the author/donor to decide if he or she is willing
 * to distribute software through any other system and a licensee cannot
 * impose that choice.
 * 
 * This section is intended to make thoroughly clear what is believed to
 * be a consequence of the rest of this License.
 * 
 *   8. If the distribution and/or use of the Program is restricted in
 * certain countries either by patents or by copyrighted interfaces, the
 * original copyright holder who places the Program under this License
 * may add an explicit geographical distribution limitation excluding
 * those countries, so that distribution is permitted only in or among
 * countries not thus excluded.  In such case, this License incorporates
 * the limitation as if written in the body of this License.
 * 
 *   9. The Free Software Foundation may publish revised and/or new versions
 * of the General Public License from time to time.  Such new versions will
 * be similar in spirit to the present version, but may differ in detail to
 * address new problems or concerns.
 * 
 * Each version is given a distinguishing version number.  If the Program
 * specifies a version number of this License which applies to it and "any
 * later version", you have the option of following the terms and conditions
 * either of that version or of any later version published by the Free
 * Software Foundation.  If the Program does not specify a version number of
 * this License, you may choose any version ever published by the Free Software
 * Foundation.
 * 
 *   10. If you wish to incorporate parts of the Program into other free
 * programs whose distribution conditions are different, write to the author
 * to ask for permission.  For software which is copyrighted by the Free
 * Software Foundation, write to the Free Software Foundation; we sometimes
 * make exceptions for this.  Our decision will be guided by the two goals
 * of preserving the free status of all derivatives of our free software and
 * of promoting the sharing and reuse of software generally.
 * 
 *                             NO WARRANTY
 * 
 *   11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
 * FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
 * OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
 * PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
 * TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
 * PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
 * REPAIR OR CORRECTION.
 * 
 *   12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 * WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
 * REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
 * INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
 * OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 * TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
 * YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
 * PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 *                      END OF TERMS AND CONDITIONS
 * 
 *             How to Apply These Terms to Your New Programs
 * 
 *   If you develop a new program, and you want it to be of the greatest
 * possible use to the public, the best way to achieve this is to make it
 * free software which everyone can redistribute and change under these terms.
 * 
 *   To do so, attach the following notices to the program.  It is safest
 * to attach them to the start of each source file to most effectively
 * convey the exclusion of warranty; and each file should have at least
 * the "copyright" line and a pointer to where the full notice is found.
 * 
 *     <one line to give the program's name and a brief idea of what it does.>
 *     Copyright (C) <year>  <name of author>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License along
 *     with this program; if not, write to the Free Software Foundation, Inc.,
 *     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * 
 * Also add information on how to contact you by electronic and paper mail.
 * 
 * If the program is interactive, make it output a short notice like this
 * when it starts in an interactive mode:
 * 
 *     Gnomovision version 69, Copyright (C) year name of author
 *     Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 *     This is free software, and you are welcome to redistribute it
 *     under certain conditions; type `show c' for details.
 * 
 * The hypothetical commands `show w' and `show c' should show the appropriate
 * parts of the General Public License.  Of course, the commands you use may
 * be called something other than `show w' and `show c'; they could even be
 * mouse-clicks or menu items--whatever suits your program.
 * 
 * You should also get your employer (if you work as a programmer) or your
 * school, if any, to sign a "copyright disclaimer" for the program, if
 * necessary.  Here is a sample; alter the names:
 * 
 *   Yoyodyne, Inc., hereby disclaims all copyright interest in the program
 *   `Gnomovision' (which makes passes at compilers) written by James Hacker.
 * 
 *   <signature of Ty Coon>, 1 April 1989
 *   Ty Coon, President of Vice
 * 
 * This General Public License does not permit incorporating your program into
 * proprietary programs.  If your program is a subroutine library, you may
 * consider it more useful to permit linking proprietary applications with the
 * library.  If this is what you want to do, use the GNU Lesser General
 * Public License instead of this License.
 *
 *
*/
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*

* C_CLI
* =====
*
* A header-only, heapless, typed command-line interface library for C99.
*
* C_Cli is designed to be integrated into an existing CLI implementation
* rather than forcing a complete CLI architecture onto the application.
*
* The library provides:
*
* * no dynamic memory allocation
* * typed command-line arguments
* * long and short flag names
* * automatic help formatting and alignment
* * built-in --help/-h and --verbose/-v flags
* * optional default arguments
* * flag attributes, including argument lists
* * input validation for supported argument types
*
*
* QUICK START
* ===========
*
* C_Cli requires the application to provide three things:
*
* 1. a user argument structure
* 2. an array of CCliArgDef describing the supported flags
* 3. one parser function for each flag
*
*
* 1. USER ARGUMENTS
* ---
*
* Define the structure populated by the CLI parsers.
*
* Example:
* 
*  typedef struct CCliUserArgs
*  {
*      bool verbose;
*      bool help;
*      const char* path;
*      uint8_t value;
*  } CCliUserArgs;
* 
* When the built-in help and verbose flags are enabled, the structure must
* contain the corresponding members.
*
*
* 2. FLAG DEFINITIONS
* ---
*
* Each flag is described by a CCliArgDef.
*
* Example:
*
* static const CCliArgDef cli_flags[] =
* {
*     {
*         .f_long = CCLI_LONG_FLAG(file),
*         .f_short = CCLI_SHORT_FLAG(f),
*         .f_args =
*         {
*             CCLI_NEW_ARG(path, CCliArgStr),
*         },
*         .f_description = "use file from path",
*         .f_parser = CCLI_PARSER_NAME(file),
*     },
* };
*
* CCLI_NO_ARG is used when a flag does not accept arguments.
*
* Arguments are described in order. If a flag accepts arguments, the
* definitions occupy consecutive entries starting from f_args[0].
*
*
* 3. FLAG PARSERS
* ---
*
* Each flag has a user-defined parser. The parser receives the user argument
* structure and a parsing context.
*
* Example:
* 
*  CCLI_PREFIX CCLI_PARSER_DECLARE_FULL(file, args, ctx)
*  {
*      return c_cli_parse_next_arg_str(ctx, &args->path);
*  }
* 
* Typed parsing functions are available for the supported argument types.
*
*
* 4. IMPLEMENTATION
* ---
*
* The header is included once to make the declarations available and once
* with CCLI_IMPLEMENTATION defined to deploy the implementation.
*
* The implementation must be deployed exactly once in the program.
*
* Example:
* 
*  #include "c_cli.h"
*   --- user definitions and flag parsers ---
*  #define CCLI_IMPLEMENTATION
*  #include "c_cli.h"
*
* 5. PARSING
* ---
*
* Call c_cli_parse() with the flag definitions, user argument structure,
* argc/argv and an optional default setter.
*
* Example:
*
*  c_cli_parse(cli_flags, CCLI_ARRAYSIZE(cli_flags), &args, argc, argv, default_args);
*
* ARGUMENT TYPES
* ==============
*
* C_Cli supports the following argument types:
*
* * CCliArgBool
* * CCliArgChar
* * CCliArgStr
* * CCliArgU8
* * CCliArgU16
* * CCliArgU32
* * CCliArgU64
* * CCliArgS8
* * CCliArgS16
* * CCliArgS32
* * CCliArgS64
*
* Numeric arguments are parsed as decimal values and are checked against
* the limits of their destination type.
*
*
* FLAG ATTRIBUTES
* ===============
*
* CCliFlagAttribute_None
*
* Normal flag behaviour.
*
* CCliFlagAttribute_ArgsList
*
* Allows the flag to consume a continuous sequence of arguments separated
* by CCLI_ARG_LIST_SEPARATOR.
*
* Example:
*
* --include file1 , file2 , file3
*
* BUILT-IN FLAGS
* =============
*
* Unless disabled at compile time, C_Cli provides:
*
* --help/-h
*     Print the generated help and stop normal parsing.
*
* --verbose/-v
*     Enable the verbose option in the user argument structure.
*
* Built-in flags can be disabled with:
*
* * CCLI_FLAG_NO_HELP
* * CCLI_FLAG_NO_VERBOSE
*
* DEFAULT ARGUMENTS
* =================
*
* c_cli_parse() can receive a CCliDefaultSetter.
*
* Defaults are applied when no recognized user flags are found.
* Passing NULL disables default initialization.
*
* CONFIGURATION
* =============
*
* The following configuration macros can be overridden before including
* c_cli.h:
*
*  CCLI_PREFIX
*      Prefix applied to public API declarations and definitions.
* 
*  CCLI_ARG_LIST_SEPARATOR
*      Character used to separate arguments belonging to an ArgsList flag.
* 
*  CCLI_CHARS_IN_TAB
*      Number of character positions represented by one tab when formatting
*      help output.
* 
*  CCLI_MAX_NUM_ARGS
*      Maximum number of arguments supported by a single flag.
*
* INTEGRATION MODEL
* =================
*
* C_Cli does not require the application's public CLI interface to expose
* this library.
*
* An application can keep its own CLI abstraction and use C_Cli only inside
* its implementation.
*
* This makes it possible to replace or modify the CLI implementation without
* changing the application's public interface.
*
* IMPLEMENTATION NOTES
* ====================
*
* C_Cli is heapless: it does not perform dynamic memory allocation.
*
* Parsed string arguments point directly into the original argv storage.
* The lifetime of these strings is therefore tied to the argv data supplied
* to c_cli_parse().
*/


//============================================public API ========================================

//============================================macros=============================================
#define CCLI_ARRAYSIZE(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#ifndef CCLI_PREFIX
#define CCLI_PREFIX
#endif // !CCLI_PREFIX

#ifndef CCLI_ARG_LIST_SEPARATOR
#define CCLI_ARG_LIST_SEPARATOR ','
#endif // CCLI_ARG_LIST_SEPARATOR

#ifndef CCLI_CHARS_IN_TAB
#define CCLI_CHARS_IN_TAB 8
#endif // !CCLI_CHARS_IN_TAB

#ifndef CCLI_MAX_NUM_ARGS
#define CCLI_MAX_NUM_ARGS 8
#endif // !CCLI_MAX_NUM_ARGS

#define CCLI_END_LINE "\n"

#define CCLI_1_TAB "\t"
#define CCLI_2_TAB CCLI_1_TAB CCLI_1_TAB

//flags names
#define CCLI_LONG_FLAG(NAME) "--"#NAME
#define CCLI_SHORT_FLAG(NAME) "-"#NAME

//flag args
#define CCLI_NEW_ARG(name, type) {#name, type}
#define CCLI_NO_ARG {{NULL, 0}}

//c_cli version
#define CCLI_MAJOR      ( (const uint32_t) 3U )
#define CCLI_MINOR      ( (const uint32_t) 2U )
#define CCLI_PATCH      ( (const uint32_t) 2U )

//flag parsers

#define CCLI_PARSER_NAME(NAME)CCliParser_##NAME

#define CCLI_PARSER_DECLARE_FULL(NAME, args, ctx)                                                 \
    CCliActionReturn CCLI_PARSER_NAME(NAME)(                                                      \
            CCliUserArgsInt* const restrict args,                                                 \
            void* const restrict ctx)

#define CCLI_PARSER_DECLARE(NAME) CCLI_PARSER_DECLARE_FULL(NAME, args, ctx)

//================================================types=========================================

#ifndef CCLI_TYPES
#define CCLI_TYPES

typedef enum{
    CCliArgVoid = 0,

    CCliArgBool,

    CCliArgChar,
    CCliArgStr,

    CCliArgU8,
    CCliArgU16,
    CCliArgU32,
    CCliArgU64,

    CCliArgS8,
    CCliArgS16,
    CCliArgS32,
    CCliArgS64,

    __Count__CCliArg,
}CCliArgType;

typedef enum
{
    CCliActionOK = 0,           /* The argument was successfully parsed. */
    CCliActionMissingInput,     /* The expected argument was not present. */
    CCliActionInvalidInput,     /* An argument was present but could not be parsed as the requested type. */
}CCliActionReturn;

typedef uint32_t CCliFlagAttribute_type;
typedef enum
{
    //No special attributes: default behaviour
    CCliFlagAttribute_None = 0,

    //the flag supports a continuous sequence of arguments divided by CCLI_ARG_LIST_SEPARATOR
    CCliFlagAttribute_ArgsList = 1 << 0,

}CCliFlagAttribute;

struct CCliUserArgs;
typedef struct CCliUserArgs CCliUserArgsInt;

typedef CCliActionReturn
(*CCliParser)(CCliUserArgsInt* const restrict args, void* const restrict ctx);

typedef void
(*CCliDefaultSetter)(CCliUserArgsInt* const restrict args);

typedef struct{
    const char* name;
    CCliArgType type;
}CCliArgSpec;

typedef struct{
    CCliFlagAttribute_type f_attributes;
    const char* f_long;
    const char* f_short;
    const char* f_description;
    CCliParser f_parser;
    CCliArgSpec f_args[CCLI_MAX_NUM_ARGS];
}CCliArgDef;

typedef struct CCliParseCtx{
    CCliFlagAttribute_type attributes;
    bool list_continue;
    int* i;
    int argc;
    char **argv;
}CCliParseCtx;

typedef union __CCliDigitData
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    int8_t s8;
    int16_t s16;
    int32_t s32;
    int64_t s64;
}CCliDigit;

#endif // !CCLI_TYPES

//=============================================declarations=====================================

CCLI_PREFIX uint32_t c_cli_get_version(void);

CCLI_PREFIX bool c_cli_parse(
        const CCliArgDef* defs,
        const size_t n_defs,
        CCliUserArgsInt* const restrict args,
        const int argc,
        char** argv,
        CCliDefaultSetter def_set);

//args
CCLI_PREFIX const char* c_cli_arg_type_to_str(const CCliArgType arg_type);

//pretty printers
CCLI_PREFIX const char* c_cli_bool_to_str(const bool val);
CCLI_PREFIX const char* c_cli_str_arg_to_str(const char* const restrict arg);

//parsing general utility functions
CCLI_PREFIX const char* c_cli_next_arg(CCliParseCtx* const restrict ctx);

//parsing specialized utility functions
CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_char(void* const restrict ctx, char* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_str(void* const restrict ctx, const char** out);

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_udig(
        void* const restrict ctx,
        const uint64_t max,
        CCliDigit* const restrict out);

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_sdig(
        void* const restrict ctx,
        const int64_t min,
        const int64_t max,
        CCliDigit* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_bool(void* const restrict ctx, bool* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_uint8_t(void* const restrict ctx, uint8_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_uint16_t(void* const restrict ctx, uint16_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_uint32_t(void* const restrict ctx, uint32_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_uint64_t(void* const restrict ctx, uint64_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_int8_t(void* const restrict ctx, int8_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_int16_t(void* const restrict ctx, int16_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_int32_t(void* const restrict ctx, int32_t* const restrict out);

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_int64_t(void* const restrict ctx, int64_t* const restrict out);

//=========================================static asserts========================================

#if \
    CCLI_ARG_LIST_SEPARATOR == ' '  || \
    CCLI_ARG_LIST_SEPARATOR == '\0' || \
    CCLI_ARG_LIST_SEPARATOR == '\n' || \
    CCLI_ARG_LIST_SEPARATOR == '\r' 
#error "invliad CCLI_ARG_LIST_SEPARATOR"
#endif

#if CCLI_CHARS_IN_TAB < 1
#error "invalid CCLI_CHARS_IN_TAB"
#endif

#if CCLI_MAX_NUM_ARGS < 1
#error "invalid CCLI_MAX_NUM_ARGS"
#endif


//============================================C_CLI IMPLEMENTATION===============================

#ifdef CCLI_IMPLEMENTATION

// CCLI_IMPLEMENTATION
//types
typedef enum {
    CCliCheckInputDefsRet_Found =0,
    CCliCheckInputDefsRet_NotFound,
    CCliCheckInputDefsRet_Error,
}CCliCheckInputDefsRet;

struct __CCliAlignSizes{
    size_t s_to_l;
    size_t l_to_d;
};

typedef struct{
    const CCliArgDef* addr; 
    const size_t size;
}__CCliBaseDefInfo;

//declarations

CCLI_PREFIX void __c_cli_find_correct_align(
        struct __CCliAlignSizes* align,
        const CCliArgDef* const restrict defs,
        const size_t n_defs);

CCLI_PREFIX size_t __c_cli_fprint_all_args(
        const CCliArgSpec* const restrict f_args,
        const CCliFlagAttribute_type attributes,
        FILE* const restrict out);

CCLI_PREFIX void __c_cli_print_defs_help(
        const CCliArgDef* const restrict defs,
        const size_t n_defs,
        struct __CCliAlignSizes* aligns,
        FILE* const restrict out
        );

CCLI_PREFIX void __c_cli_print_help_full(
        const CCliArgDef* const restrict defs,
        const size_t n_defs,
        const char* const argv_0,
        FILE* const restrict out);

CCLI_PREFIX CCliCheckInputDefsRet __c_cli_check_input_defs(
        const char* const restrict input,
        const CCliArgDef* defs,
        const size_t n_defs,
        bool *found_something,
        CCliUserArgsInt* const restrict args,
        CCliParseCtx* const restrict ctx
        );

CCLI_PREFIX const char* __c_cli_get_prog_name(const char* const restrict argv_0);

#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
CCLI_PREFIX __CCliBaseDefInfo __c_cli_get_base_flags(void);
#endif

#ifndef CCLI_FLAG_NO_VERBOSE
CCLI_PREFIX CCLI_PARSER_DECLARE_FULL(verbose, args, ctx);
#endif // !CCLI_FLAG_NO_VERBOSE

#ifndef CCLI_FLAG_NO_HELP
CCLI_PREFIX CCLI_PARSER_DECLARE_FULL(help, args, ctx);
#endif // !CCLI_FLAG_NO_HELP

//implementations

CCLI_PREFIX void __c_cli_print_help_full(
        const CCliArgDef* const restrict defs,
        const size_t n_defs,
        const char* const argv_0,
        FILE* const restrict out)
{
    const char* prog_name = __c_cli_get_prog_name(argv_0);
#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
    const __CCliBaseDefInfo base_flags = __c_cli_get_base_flags();
#endif
    struct __CCliAlignSizes aligns = {0};

    __c_cli_find_correct_align(&aligns, defs, n_defs);
#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
    __c_cli_find_correct_align(&aligns, base_flags.addr, base_flags.size);
#endif

    fprintf(out, "usage %s [opts]:" CCLI_END_LINE, prog_name);

    //user defs
    __c_cli_print_defs_help(defs, n_defs, &aligns, out);

#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
    //base defs
    __c_cli_print_defs_help(base_flags.addr, base_flags.size, &aligns, out);
#endif
}

CCLI_PREFIX size_t __c_cli_fprint_all_args(
        const CCliArgSpec* const restrict f_args,
        const CCliFlagAttribute_type attributes,
        FILE* dst)
{
    const CCliArgSpec* arg;
    bool empty = true;
    size_t written=0;
    

    for(size_t i=0; i<CCLI_MAX_NUM_ARGS; i++)
    {
        arg = &f_args[i];

        if(arg->name != NULL)
        {
            if(empty)
            {
                written += (size_t) fprintf(dst, "[");
                empty = false;
            }

            if(i>0)
            {
                written += (size_t) fprintf(dst, ", ");
            }

            written += (size_t) fprintf(dst, "%s:%s", arg->name, c_cli_arg_type_to_str(arg->type));
        }
    }

    if(!empty)
    {
        written += (size_t) fprintf(dst, "]");
    }

    if(attributes & CCliFlagAttribute_ArgsList)
    {
        written += (size_t) fprintf(dst, "...");
    }

    return written;
}

CCLI_PREFIX void __c_cli_find_correct_align(
        struct __CCliAlignSizes* align,
        const CCliArgDef* const restrict defs, const size_t n_defs)
{
    const CCliArgSpec *args;
    const CCliArgSpec* arg;
    size_t args_len=0, f_len, tot_len, n_tabs;
    bool empty = true;

    for(size_t i=0; i<n_defs; i++)
    {
        args_len = 0;
        empty = true;
        args = defs[i].f_args;

        for(size_t i=0; i<CCLI_MAX_NUM_ARGS; i++)
        {
            arg = &args[i];

            if(arg->name != NULL)
            {
                if(empty)
                {
                    args_len += strlen("[");
                    empty = false;
                }

                if(i>0)
                {
                    args_len += strlen(", ");
                }
                args_len += strlen(arg->name);
                args_len += strlen(":");
                args_len += strlen(c_cli_arg_type_to_str(arg->type));
            }
        }
        if(!empty)
        {
            args_len += strlen("]");
        }

        if(defs[i].f_attributes & CCliFlagAttribute_ArgsList)
        {
            args_len += strlen("...");
        }

        f_len = defs[i].f_short ? strlen(defs[i].f_short) + 1 : 0;
        tot_len = args_len + f_len;
        n_tabs = (tot_len / CCLI_CHARS_IN_TAB) + ((tot_len % CCLI_CHARS_IN_TAB) > 0);
        if(n_tabs > align->s_to_l) align->s_to_l = n_tabs;

        f_len = defs[i].f_long ? strlen(defs[i].f_long) + 1 : 0;
        tot_len = args_len + f_len;
        n_tabs = (tot_len / CCLI_CHARS_IN_TAB) + ((tot_len % CCLI_CHARS_IN_TAB) > 0);
        if(n_tabs > align->l_to_d) align->l_to_d = n_tabs;
    }

}

CCLI_PREFIX void __c_cli_print_defs_help(
        const CCliArgDef* const restrict defs,
        const size_t n_defs,
        struct __CCliAlignSizes* aligns,
        FILE* const restrict out)
{
    const CCliArgDef* def;

    for(size_t i=0; i<n_defs; i++)
    {
        size_t written=0, to_write;
        to_write = CCLI_CHARS_IN_TAB * (aligns->s_to_l + 1);
        def = &defs[i];

        if(def->f_long) assert(strchr(def->f_long, ' ') == NULL && "flags cannot contain spaces");
        if(def->f_short) assert(strchr(def->f_short, ' ') == NULL && "flags cannot contain spaces");

        fprintf(out, CCLI_2_TAB);

        if(def->f_long)
        {
            written += (size_t) fprintf(out, "%s ", def->f_long);                                // --help
            written += (size_t) __c_cli_fprint_all_args(def->f_args, def->f_attributes, out);    // [...]
        }

        while(written < to_write)
        {
            written += (size_t) fprintf(out, " ");                                               // long padding
        }

        written =0;
        to_write = CCLI_CHARS_IN_TAB * (aligns->l_to_d+1);

        fprintf(out, CCLI_1_TAB);

        if(def->f_short)
        {
            written += (size_t) fprintf(out, "%s ", def->f_short);                               // -h
            written += (size_t) __c_cli_fprint_all_args(def->f_args, def->f_attributes, out);    // [...]
        }

        while(written < to_write)
        {
            written += (size_t) fprintf(out, " ");                                               // short padding
        }

        if(def->f_description)
        {
            fprintf(out, "%s", def->f_description);                                             // description
        }

        fprintf(out, CCLI_END_LINE);
    }
}

CCLI_PREFIX CCliCheckInputDefsRet __c_cli_check_input_defs(
        const char* const restrict input,
        const CCliArgDef* defs,
        const size_t n_defs,
        bool *found_something,
        CCliUserArgsInt* const restrict args,
        CCliParseCtx* const restrict ctx
        )
{
    CCliCheckInputDefsRet res = CCliCheckInputDefsRet_NotFound;
    const CCliArgDef* user_def;
    CCliActionReturn act_res = CCliActionOK;

    for(size_t j=0; j<n_defs; j++)
    {
        user_def = &defs[j];
        ctx->attributes = user_def->f_attributes;

        if(user_def->f_long) assert(strchr(user_def->f_long, ' ') == NULL && "flags cannot contain spaces");
        if(user_def->f_short) assert(strchr(user_def->f_short, ' ') == NULL && "flags cannot contain spaces");

        const char* safe_f_long = user_def->f_long ? user_def->f_long : "";
        const char* safe_f_short = user_def->f_short ? user_def->f_short : "";

        const bool matches_long =
            user_def->f_long && strcmp(user_def->f_long, input) == 0;

        const bool matches_short =
            user_def->f_short && strcmp(user_def->f_short, input) == 0;

        const bool two_flags =
            user_def->f_long && user_def->f_short;


        if( matches_long || matches_short )
        {

            assert(user_def->f_parser);
            do
            {
                ctx->list_continue = false;
                act_res = user_def->f_parser(args, ctx);

                if(act_res == CCliActionOK)
                {
                    res = CCliCheckInputDefsRet_Found;
                    *found_something = true;
                }
                else
                {
                    switch (act_res)
                    {
                        case CCliActionOK:
                            assert(0 && "unreachable");
                            break;
                        case CCliActionMissingInput:
                            {
                                fprintf(stderr, "missing args for flag %s %s %s, expected: ",
                                        two_flags ? "OR" : "",
                                        safe_f_long,
                                        safe_f_short);
                            }
                            break;
                        case CCliActionInvalidInput:
                            {
                                fprintf(stderr, "invalid arg %s for flag %s %s %s, expected: ",
                                        ctx->argv[*ctx->i],
                                        two_flags ? "OR" : "",
                                        safe_f_long,
                                        safe_f_short);
                            }
                            break;
                    }

                    __c_cli_fprint_all_args(user_def->f_args, user_def->f_attributes, stderr);
                    fprintf(stderr, "\n");
                    res = CCliCheckInputDefsRet_Error;
                    goto end;
                }
            }
            while(user_def->f_attributes & CCliFlagAttribute_ArgsList && ctx->list_continue);
        }
    }

end:
    return res;
}

CCLI_PREFIX const char* __c_cli_get_prog_name(const char* const restrict argv_0)
{
    const char* res = argv_0 ? argv_0 : "c_cli";
    if( argv_0 )
    {
        const size_t argv_0_len = strlen(argv_0);
        const char* p_prog_name = argv_0_len > 0 ? &argv_0[argv_0_len-1] : "c_cli";

        while(
                p_prog_name > argv_0 &&
                *p_prog_name != '/' &&
                *p_prog_name != '\\'
             )
        {
            p_prog_name--;
        }

        if( *p_prog_name == '/' || *p_prog_name == '\\' ) p_prog_name++;

        res = p_prog_name;
    }

    return res;
}

CCLI_PREFIX CCLI_PARSER_DECLARE_FULL(verbose, args, ctx)
{
    (void) ctx;
    args->verbose = true;
    return CCliActionOK;
}

#ifndef CCLI_FLAG_NO_HELP
CCLI_PREFIX CCLI_PARSER_DECLARE_FULL(help, args, ctx)
{
    (void) ctx;
    args->help= true;
    return CCliActionOK;
}
#endif // !CCLI_FLAG_NO_HELP

#define __CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT(N_BITS, OUT_PTR)                       \
CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_uint##N_BITS##_t(                             \
        void* const restrict ctx, uint##N_BITS##_t* const restrict OUT_PTR)                     \
{                                                                                               \
    CCliActionReturn res;                                                                       \
    CCliDigit dig = {0};                                                                        \
                                                                                                \
    if ( !(res = c_cli_parse_next_arg_udig(ctx, UINT##N_BITS##_MAX, &dig)) )                    \
    {                                                                                           \
        *(OUT_PTR)= dig.u##N_BITS;                                                              \
    }                                                                                           \
                                                                                                \
    return res;                                                                                 \
}

#define __CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT(N_BITS, OUT_PTR)                         \
CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_int##N_BITS##_t(                              \
        void* const restrict ctx, int##N_BITS##_t* const restrict OUT_PTR)                      \
{                                                                                               \
    CCliActionReturn res;                                                                       \
    CCliDigit dig = {0};                                                                        \
                                                                                                \
    if ( !(res = c_cli_parse_next_arg_sdig(ctx, INT##N_BITS##_MIN, INT##N_BITS##_MAX, &dig)) )  \
    {                                                                                           \
        *(OUT_PTR) = dig.u##N_BITS;                                                             \
    }                                                                                           \
                                                                                                \
    return res;                                                                                 \
}

__CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT(8, out)
__CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT(16, out)
__CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT(32, out)
__CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT(64, out)

__CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT(8, out)
__CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT(16, out)
__CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT(32, out)
__CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT(64, out)

#undef __CCLI_PARSE_NEXT_ARG_UNSIGNED_DIGIT_DECLARE_BIT
#undef __CCLI_PARSE_NEXT_ARG_SIGNED_DIGIT_DECLARE_BIT

#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
CCLI_PREFIX __CCliBaseDefInfo __c_cli_get_base_flags(void)
{
    static const CCliArgDef base_flags[] =
    {
#ifndef CCLI_FLAG_NO_VERBOSE
        { //--verbose, -v
            .f_long = CCLI_LONG_FLAG(verbose),
            .f_short = CCLI_SHORT_FLAG(v),
            .f_args = CCLI_NO_ARG,
            .f_description = "print verbose output",
            .f_parser = CCLI_PARSER_NAME(verbose),
        },
#endif // !CCLI_FLAG_NO_VERBOSE

#ifndef CCLI_FLAG_NO_HELP
        {//--help, -h
            .f_long = CCLI_LONG_FLAG(help),
            .f_short = CCLI_SHORT_FLAG(h),
            .f_args = CCLI_NO_ARG,
            .f_description = "print this help",
            .f_parser = CCLI_PARSER_NAME(help),
        },
#endif // !CCLI_FLAG_NO_HELP
    };

    return (__CCliBaseDefInfo) {.addr = base_flags, .size = CCLI_ARRAYSIZE(base_flags)};
}
#endif

CCLI_PREFIX uint32_t c_cli_get_version(void)
{
    return  (CCLI_MAJOR << 16)  |
            (CCLI_MINOR << 8)   |
            CCLI_PATCH;
}

CCLI_PREFIX bool c_cli_parse(
        const CCliArgDef* defs,
        const size_t n_defs,
        CCliUserArgsInt* const restrict args,
        const int argc,
        char** argv,
        CCliDefaultSetter def_set)
{
    const char* input;
#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
    const __CCliBaseDefInfo base_flags = __c_cli_get_base_flags();
#endif

    if ( argc < 1 || !argv ) return false;

    const char* prog_name = __c_cli_get_prog_name(argv[0]);
    bool found_something = false;
    CCliParseCtx ctx = {
        .attributes = 0,
        .i=NULL,
        .argc = argc,
        .argv = argv,
    };


    for(int i=1;i <argc; i++)
    {
        ctx.i = &i;
        input = argv[i];

        if(defs)
        {
            switch (__c_cli_check_input_defs(input, defs, n_defs, &found_something, args, &ctx))
            {
                case CCliCheckInputDefsRet_Found: continue;
                case CCliCheckInputDefsRet_NotFound: break;
                case CCliCheckInputDefsRet_Error: return false;
            }
        }

#if !defined(CCLI_FLAG_NO_HELP) || !defined(CCLI_FLAG_NO_VERBOSE)
        switch(__c_cli_check_input_defs(
                    input, base_flags.addr, base_flags.size, &found_something, args, &ctx))
        {
            case CCliCheckInputDefsRet_Found: continue;
            case CCliCheckInputDefsRet_NotFound: break;
            case CCliCheckInputDefsRet_Error: return false;
        }
#endif

        fprintf(stderr, "%s: unrecognized flag: %s" CCLI_END_LINE, prog_name, input);
    }

    if(args->help)
    {
        __c_cli_print_help_full(defs, n_defs, argv[0], stderr);
        return false;
    }

    if(found_something) return true;

    if(def_set)
    {
        printf("%s: no arguments provided. Applying default values\n", prog_name);
        def_set(args);
        return true;
    }

    __c_cli_print_help_full(defs, n_defs, argv[0], stderr);

    return false;
}

CCLI_PREFIX const char* c_cli_arg_type_to_str(const CCliArgType arg_type)
{
    static const char* const restrict c_cli_args_types[__Count__CCliArg] =
    {
        [CCliArgVoid]   = "void",

        [CCliArgBool]   = "bool",

        [CCliArgChar]   = "char",
        [CCliArgStr]    = "str",

        [CCliArgU8]     = "u8",
        [CCliArgU16]    = "u16",
        [CCliArgU32]    = "u32",
        [CCliArgU64]    = "u64",

        [CCliArgS8]     = "s8",
        [CCliArgS16]    = "s16",
        [CCliArgS32]    = "s32",
        [CCliArgS64]    = "s64",
    };

    if(arg_type < __Count__CCliArg)
    {
        return c_cli_args_types[arg_type];
    }

    return NULL;
}

//pretty printers

CCLI_PREFIX const char* c_cli_bool_to_str(const bool val)
{
    return val ? "True" : "False";
}

CCLI_PREFIX const char* c_cli_str_arg_to_str(const char* const restrict arg)
{
    return arg ? arg : "(nil)";
}


//parsing general utility functions

CCLI_PREFIX const char* c_cli_next_arg(CCliParseCtx* const restrict ctx)
{
    const char* res = NULL;
    CCliParseCtx* p_ctx = ctx;
    const char separator[2] = 
    {
        CCLI_ARG_LIST_SEPARATOR,
    };

    if(*p_ctx->i + 1 < p_ctx->argc)
    {
        res = p_ctx->argv[++(*p_ctx->i)];
        ctx->list_continue = false;

        if(
                (ctx->attributes & CCliFlagAttribute_ArgsList) &&
                *p_ctx->i + 1 < p_ctx->argc &&
                !strcmp(p_ctx->argv[*p_ctx->i + 1], separator)
          )
        {
            ctx->list_continue = true;
            (*p_ctx->i)++;
        }

    }

    return res;
}

//parsing specialized utility functions

CCLI_PREFIX CCliActionReturn
c_cli_parse_next_arg_char(void* const restrict ctx, char* const restrict out)
{
    const char* raw_arg = c_cli_next_arg(ctx);

    if( raw_arg )
    {
        if ( raw_arg[0] == '\0' || raw_arg[1] != '\0' ) return CCliActionInvalidInput;

        *out = *raw_arg;
        return CCliActionOK;
    }

    return CCliActionMissingInput;
}

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_str(void* const restrict ctx, const char** out)
{
    const char* raw_arg = c_cli_next_arg(ctx);

    if(raw_arg)
    {
        *out = raw_arg;
        return CCliActionOK;
    }

    return CCliActionMissingInput;
}

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_udig(
        void* const restrict ctx,
        const uint64_t max,
        CCliDigit* const restrict out)
{
    const char* raw_arg = c_cli_next_arg(ctx);
    uint64_t res = {0};

    if(!raw_arg) return CCliActionMissingInput;
    if(!*raw_arg) return CCliActionInvalidInput;

    while(*raw_arg)
    {
        char c = *raw_arg;
        uint64_t digit = c - '0';

        if(c < '0' || c > '9') return CCliActionInvalidInput;

        if (
                ( res > max / 10 ) ||
                ( res == max / 10 && digit > max % 10 )

           )
        {
            return CCliActionInvalidInput;
        }
        res = res*10 + digit;

        raw_arg++;
    }


    out->u64 = res;

    return CCliActionOK;
}

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_sdig(
        void* const restrict ctx,
        const int64_t min,
        const int64_t max,
        CCliDigit* const restrict out)
{
    const char* raw_arg = c_cli_next_arg(ctx);
    int64_t res = {0};
    int neg = 1;

    if(!raw_arg) return CCliActionMissingInput;

    if(*raw_arg == '-')
    {
        neg = -1;
        raw_arg++;
    }

    if(!*raw_arg) return CCliActionMissingInput;

    while(*raw_arg)
    {
        char c = *raw_arg;
        int64_t digit = neg * (c - '0');

        if(c < '0' || c > '9') return CCliActionInvalidInput;


        if (
                ( res < min / 10 ) ||
                ( res == min / 10 && digit < min % 10 ) ||

                ( res > max / 10 ) ||
                ( res == max / 10 && digit > max % 10 )

           )
        {
            return CCliActionInvalidInput;
        }

        res = res*10 + digit;

        raw_arg++;
    }


    out->u64 = res;

    return CCliActionOK;
}

CCLI_PREFIX CCliActionReturn c_cli_parse_next_arg_bool(
        void* const restrict ctx,
        bool* const restrict out)
{
    CCliActionReturn res = CCliActionMissingInput;
    const char* raw_arg = c_cli_next_arg(ctx);

    if(!raw_arg)
    {
        res = CCliActionMissingInput;
    }
    else if(!strcmp(raw_arg, "true") || !strcmp(raw_arg, "TRUE"))
    {
        *out = true;
        res = CCliActionOK;
    }
    else if(!strcmp(raw_arg, "false") || !strcmp(raw_arg, "FALSE"))
    {
        *out = false;
        res = CCliActionOK;
    }
    else
    {
        res = CCliActionInvalidInput;
    }

    return res;
}

#endif // CCLI_IMPLEMENTATION
