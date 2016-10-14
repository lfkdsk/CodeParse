/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT_LITERAL = 258,
     DOUBLE_LITERAL = 259,
     STRING_LITERAL = 260,
     REGEXP_LITERAL = 261,
     IDENTIFIER = 262,
     IF = 263,
     ELSE = 264,
     ELSIF = 265,
     WHILE = 266,
     FOR = 267,
     FOREACH = 268,
     RETURN_T = 269,
     BREAK = 270,
     CONTINUE = 271,
     NULL_T = 272,
     LP = 273,
     RP = 274,
     LC = 275,
     RC = 276,
     LB = 277,
     RB = 278,
     SEMICOLON = 279,
     COLON = 280,
     COMMA = 281,
     ASSIGN_T = 282,
     LOGICAL_AND = 283,
     LOGICAL_OR = 284,
     EQ = 285,
     NE = 286,
     GT = 287,
     GE = 288,
     LT = 289,
     LE = 290,
     ADD = 291,
     SUB = 292,
     MUL = 293,
     DIV = 294,
     MOD = 295,
     TRUE_T = 296,
     FALSE_T = 297,
     EXCLAMATION = 298,
     DOT = 299,
     ADD_ASSIGN_T = 300,
     SUB_ASSIGN_T = 301,
     MUL_ASSIGN_T = 302,
     DIV_ASSIGN_T = 303,
     MOD_ASSIGN_T = 304,
     INCREMENT = 305,
     DECREMENT = 306,
     TRY = 307,
     CATCH = 308,
     FINALLY = 309,
     THROW = 310,
     BOOLEAN_T = 311,
     INT_T = 312,
     DOUBLE_T = 313,
     STRING_T = 314,
     NEW = 315,
     REQUIRE = 316,
     RENAME = 317,
     CLASS_T = 318,
     PUBLIC_T = 319,
     PRIVATE_T = 320,
     VIRTUAL_T = 321,
     OVERRIDE_T = 322
   };
#endif
/* Tokens.  */
#define INT_LITERAL 258
#define DOUBLE_LITERAL 259
#define STRING_LITERAL 260
#define REGEXP_LITERAL 261
#define IDENTIFIER 262
#define IF 263
#define ELSE 264
#define ELSIF 265
#define WHILE 266
#define FOR 267
#define FOREACH 268
#define RETURN_T 269
#define BREAK 270
#define CONTINUE 271
#define NULL_T 272
#define LP 273
#define RP 274
#define LC 275
#define RC 276
#define LB 277
#define RB 278
#define SEMICOLON 279
#define COLON 280
#define COMMA 281
#define ASSIGN_T 282
#define LOGICAL_AND 283
#define LOGICAL_OR 284
#define EQ 285
#define NE 286
#define GT 287
#define GE 288
#define LT 289
#define LE 290
#define ADD 291
#define SUB 292
#define MUL 293
#define DIV 294
#define MOD 295
#define TRUE_T 296
#define FALSE_T 297
#define EXCLAMATION 298
#define DOT 299
#define ADD_ASSIGN_T 300
#define SUB_ASSIGN_T 301
#define MUL_ASSIGN_T 302
#define DIV_ASSIGN_T 303
#define MOD_ASSIGN_T 304
#define INCREMENT 305
#define DECREMENT 306
#define TRY 307
#define CATCH 308
#define FINALLY 309
#define THROW 310
#define BOOLEAN_T 311
#define INT_T 312
#define DOUBLE_T 313
#define STRING_T 314
#define NEW 315
#define REQUIRE 316
#define RENAME 317
#define CLASS_T 318
#define PUBLIC_T 319
#define PRIVATE_T 320
#define VIRTUAL_T 321
#define OVERRIDE_T 322




/* Copy the first part of user declarations.  */
#line 1 "diksam.y"

#include <stdio.h>
#include "diksamc.h"
#define YYDEBUG 1


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 6 "diksam.y"
{
    char                *identifier;
    PackageName         *package_name;
    RequireList         *require_list;
    RenameList          *rename_list;
    ParameterList       *parameter_list;
    ArgumentList        *argument_list;
    Expression          *expression;
    ExpressionList      *expression_list;
    Statement           *statement;
    StatementList       *statement_list;
    Block               *block;
    Elsif               *elsif;
    AssignmentOperator  assignment_operator;
    TypeSpecifier       *type_specifier;
    DVM_BasicType       basic_type_specifier;
    ArrayDimension      *array_dimension;
}
/* Line 193 of yacc.c.  */
#line 255 "diksam.tab.c"
        YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 268 "diksam.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (To)[yyi] = (From)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                                        \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack, Stack, yysize);                          \
        Stack = &yyptr->Stack;                                          \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   486

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  150
/* YYNRULES -- Number of states.  */
#define YYNSTATES  260

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    13,    15,    17,    19,
      22,    26,    28,    32,    34,    37,    42,    44,    46,    48,
      50,    52,    54,    56,    58,    62,    69,    75,    82,    88,
      91,    96,    98,   102,   104,   107,   109,   113,   115,   119,
     121,   123,   125,   127,   129,   131,   133,   137,   139,   143,
     145,   149,   153,   155,   159,   163,   167,   171,   173,   177,
     181,   183,   187,   191,   195,   197,   200,   203,   205,   207,
     212,   216,   221,   225,   228,   231,   235,   237,   239,   241,
     243,   245,   247,   249,   251,   253,   257,   262,   266,   271,
     273,   276,   280,   283,   287,   288,   290,   294,   297,   299,
     301,   303,   305,   307,   309,   311,   313,   315,   317,   323,
     331,   338,   347,   349,   352,   358,   359,   362,   369,   380,
     389,   390,   392,   396,   397,   399,   403,   407,   417,   422,
     430,   434,   438,   444,   445,   450,   453,   459,   460,   462,
     465,   467,   469,   471,   474,   477,   481,   483,   485,   487,
     489
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      69,     0,    -1,    70,    76,    -1,    69,    76,    -1,    -1,
      71,    74,    -1,    71,    -1,    74,    -1,    72,    -1,    71,
      72,    -1,    61,    73,    24,    -1,     7,    -1,    73,    44,
       7,    -1,    75,    -1,    74,    75,    -1,    62,    73,     7,
      24,    -1,    79,    -1,   119,    -1,   101,    -1,    56,    -1,
      57,    -1,    58,    -1,    59,    -1,    77,    -1,    78,    22,
      23,    -1,    78,     7,    18,    80,    19,   117,    -1,    78,
       7,    18,    19,   117,    -1,    78,     7,    18,    80,    19,
      24,    -1,    78,     7,    18,    19,    24,    -1,    78,     7,
      -1,    80,    26,    78,     7,    -1,    84,    -1,    81,    26,
      84,    -1,   101,    -1,    82,   101,    -1,    84,    -1,    83,
      26,    84,    -1,    86,    -1,    93,    85,    84,    -1,    27,
      -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,    49,
      -1,    87,    -1,    86,    29,    87,    -1,    88,    -1,    87,
      28,    88,    -1,    89,    -1,    88,    30,    89,    -1,    88,
      31,    89,    -1,    90,    -1,    89,    32,    90,    -1,    89,
      33,    90,    -1,    89,    34,    90,    -1,    89,    35,    90,
      -1,    91,    -1,    90,    36,    91,    -1,    90,    37,    91,
      -1,    92,    -1,    91,    38,    92,    -1,    91,    39,    92,
      -1,    91,    40,    92,    -1,    93,    -1,    37,    92,    -1,
      43,    92,    -1,    94,    -1,    96,    -1,    94,    22,    83,
      23,    -1,    93,    44,     7,    -1,    93,    18,    81,    19,
      -1,    93,    18,    19,    -1,    93,    50,    -1,    93,    51,
      -1,    18,    83,    19,    -1,     7,    -1,     3,    -1,     4,
      -1,     5,    -1,     6,    -1,    41,    -1,    42,    -1,    17,
      -1,    95,    -1,    20,   100,    21,    -1,    20,   100,    26,
      21,    -1,    60,    77,    97,    -1,    60,    77,    97,    99,
      -1,    98,    -1,    97,    98,    -1,    22,    83,    23,    -1,
      22,    23,    -1,    99,    22,    23,    -1,    -1,    84,    -1,
     100,    26,    84,    -1,    83,    24,    -1,   102,    -1,   106,
      -1,   107,    -1,   108,    -1,   110,    -1,   112,    -1,   113,
      -1,   114,    -1,   115,    -1,   116,    -1,     8,    18,    83,
      19,   117,    -1,     8,    18,    83,    19,   117,     9,   117,
      -1,     8,    18,    83,    19,   117,   103,    -1,     8,    18,
      83,    19,   117,   103,     9,   117,    -1,   104,    -1,   103,
     104,    -1,    10,    18,    83,    19,   117,    -1,    -1,     7,
      25,    -1,   105,    11,    18,    83,    19,   117,    -1,   105,
      12,    18,   109,    24,   109,    24,   109,    19,   117,    -1,
     105,    13,    18,     7,    25,    83,    19,   117,    -1,    -1,
      83,    -1,    14,   109,    24,    -1,    -1,     7,    -1,    15,
     111,    24,    -1,    16,   111,    24,    -1,    52,   117,    53,
      18,     7,    19,   117,    54,   117,    -1,    52,   117,    54,
     117,    -1,    52,   117,    53,    18,     7,    19,   117,    -1,
      55,    83,    24,    -1,    78,     7,    24,    -1,    78,     7,
      27,    83,    24,    -1,    -1,    20,   118,    82,    21,    -1,
      20,    21,    -1,   123,    63,    20,   120,    21,    -1,    -1,
     121,    -1,   120,   121,    -1,   122,    -1,   125,    -1,    79,
      -1,   123,    79,    -1,   124,    79,    -1,   123,   124,    79,
      -1,    64,    -1,    65,    -1,    66,    -1,    67,    -1,   123,
     116,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    63,    63,    64,    66,    68,    72,    76,    82,    83,
      89,    95,    99,   105,   106,   112,   118,   119,   120,   129,
     133,   137,   141,   147,   151,   157,   161,   165,   169,   175,
     179,   185,   189,   195,   199,   205,   206,   212,   213,   219,
     223,   227,   231,   235,   239,   245,   246,   252,   253,   259,
     260,   264,   270,   271,   275,   279,   283,   289,   290,   294,
     300,   301,   305,   309,   315,   316,   320,   326,   327,   330,
     334,   338,   342,   346,   350,   354,   358,   362,   363,   364,
     365,   366,   370,   374,   378,   381,   385,   391,   395,   401,
     402,   408,   414,   418,   426,   429,   433,   439,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   455,   459,
     463,   467,   473,   474,   480,   487,   490,   496,   502,   509,
     516,   519,   522,   529,   532,   535,   541,   547,   551,   555,
     560,   565,   569,   576,   575,   583,   590,   592,   594,   595,
     598,   599,   602,   603,   604,   605,   608,   609,   612,   613,
     616
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT_LITERAL", "DOUBLE_LITERAL",
  "STRING_LITERAL", "REGEXP_LITERAL", "IDENTIFIER", "IF", "ELSE", "ELSIF",
  "WHILE", "FOR", "FOREACH", "RETURN_T", "BREAK", "CONTINUE", "NULL_T",
  "LP", "RP", "LC", "RC", "LB", "RB", "SEMICOLON", "COLON", "COMMA",
  "ASSIGN_T", "LOGICAL_AND", "LOGICAL_OR", "EQ", "NE", "GT", "GE", "LT",
  "LE", "ADD", "SUB", "MUL", "DIV", "MOD", "TRUE_T", "FALSE_T",
  "EXCLAMATION", "DOT", "ADD_ASSIGN_T", "SUB_ASSIGN_T", "MUL_ASSIGN_T",
  "DIV_ASSIGN_T", "MOD_ASSIGN_T", "INCREMENT", "DECREMENT", "TRY", "CATCH",
  "FINALLY", "THROW", "BOOLEAN_T", "INT_T", "DOUBLE_T", "STRING_T", "NEW",
  "REQUIRE", "RENAME", "CLASS_T", "PUBLIC_T", "PRIVATE_T", "VIRTUAL_T",
  "OVERRIDE_T", "$accept", "translation_unit", "initial_declaration",
  "require_list", "require_declaration", "package_name", "rename_list",
  "rename_declaration", "definition_or_statement", "basic_type_specifier",
  "type_specifier", "function_definition", "parameter_list",
  "argument_list", "statement_list", "expression", "assignment_expression",
  "assignment_operator", "logical_or_expression", "logical_and_expression",
  "equality_expression", "relational_expression", "additive_expression",
  "multiplicative_expression", "unary_expression", "primary_expression",
  "primary_no_new_array", "array_literal", "array_creation",
  "dimension_expression_list", "dimension_expression", "dimension_list",
  "expression_list", "statement", "if_statement", "elsif_list", "elsif",
  "label_opt", "while_statement", "for_statement", "foreach_statement",
  "expression_opt", "return_statement", "identifier_opt",
  "break_statement", "continue_statement", "try_statement",
  "throw_statement", "declaration_statement", "block", "@1",
  "class_declaration", "member_declaration_list", "member_declaration",
  "method_member", "access_modifier", "override_modifier", "field_member", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    68,    69,    69,    70,    70,    70,    70,    71,    71,
      72,    73,    73,    74,    74,    75,    76,    76,    76,    77,
      77,    77,    77,    78,    78,    79,    79,    79,    79,    80,
      80,    81,    81,    82,    82,    83,    83,    84,    84,    85,
      85,    85,    85,    85,    85,    86,    86,    87,    87,    88,
      88,    88,    89,    89,    89,    89,    89,    90,    90,    90,
      91,    91,    91,    91,    92,    92,    92,    93,    93,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    95,    95,    96,    96,    97,
      97,    98,    99,    99,   100,   100,   100,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
     102,   102,   103,   103,   104,   105,   105,   106,   107,   108,
     109,   109,   110,   111,   111,   112,   113,   114,   114,   114,
     115,   116,   116,   118,   117,   117,   119,   120,   120,   120,
     121,   121,   122,   122,   122,   122,   123,   123,   124,   124,
     125
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     1,     1,     2,
       3,     1,     3,     1,     2,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     6,     5,     6,     5,     2,
       4,     1,     3,     1,     2,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     1,     2,     2,     1,     1,     4,
       3,     4,     3,     2,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     4,     3,     4,     1,
       2,     3,     2,     3,     0,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     5,     7,
       6,     8,     1,     2,     5,     0,     2,     6,    10,     8,
       0,     1,     3,     0,     1,     3,     3,     9,     4,     7,
       3,     3,     5,     0,     4,     2,     5,     0,     1,     2,
       1,     1,     1,     2,     2,     3,     1,     1,     1,     1,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,   115,   115,     6,     8,     7,    13,    11,
       0,     0,     1,    77,    78,    79,    80,    76,     0,   120,
     123,   123,    83,     0,    94,     0,    81,    82,     0,     0,
       0,    19,    20,    21,    22,     0,   146,   147,     3,    23,
       0,    16,     0,    35,    37,    45,    47,    49,    52,    57,
      60,    64,    67,    84,    68,    18,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    17,     0,     2,
       9,     5,    14,    10,     0,     0,   116,     0,    76,   121,
       0,   124,     0,     0,     0,    95,     0,    65,    64,    66,
     133,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,     0,    40,    41,    42,    43,    44,    73,
      74,     0,     0,     0,     0,     0,     0,    12,    15,     0,
     122,   125,   126,    75,    85,     0,   135,   115,     0,     0,
     130,     0,    87,    89,     0,   131,     0,    24,    36,    46,
      48,    50,    51,    53,    54,    55,    56,    58,    59,    61,
      62,    63,    72,     0,    31,    70,    38,     0,     0,   120,
       0,   137,     0,    86,    96,     0,   115,    33,     0,   128,
       0,     0,    90,    88,     0,     0,     0,     0,    71,     0,
      69,     0,     0,     0,   148,   149,     0,   142,     0,   138,
     140,     0,     0,   141,   108,     0,   134,    34,     0,    91,
      92,     0,    28,    26,    29,     0,     0,   132,    32,     0,
     120,     0,     0,   136,   139,   143,   150,     0,   144,     0,
       0,   110,   112,     0,    93,    27,    25,     0,   117,     0,
       0,   145,   109,     0,     0,   113,   129,    30,   120,     0,
       0,   111,     0,     0,   119,     0,   127,     0,   114,   118
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    10,     7,     8,    38,    39,
     196,    41,   186,   163,   176,    42,    43,   121,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,   142,
     143,   183,    86,    55,    56,   231,   232,    57,    58,    59,
      60,    80,    61,    82,    62,    63,    64,    65,    66,    91,
     137,    67,   198,   199,   200,    68,   202,   203
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -162
static const yytype_int16 yypact[] =
{
      46,    30,    30,    10,   189,    46,  -162,    -7,  -162,  -162,
      -4,    12,  -162,  -162,  -162,  -162,  -162,    71,   102,   426,
     136,   136,  -162,   426,   426,   426,  -162,  -162,   426,   128,
     426,  -162,  -162,  -162,  -162,   108,  -162,  -162,  -162,  -162,
      27,  -162,   174,  -162,    49,   145,   130,   137,    92,   177,
    -162,   107,   186,  -162,  -162,  -162,  -162,   226,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,   160,  -162,
    -162,    -7,  -162,  -162,   236,   228,  -162,   426,  -162,   245,
     238,  -162,   253,   254,    53,  -162,    15,  -162,    62,  -162,
     255,   181,   216,   257,     5,   258,  -162,   426,   426,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   426,
     426,   337,  -162,   273,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,   426,   426,   264,   265,   266,   267,  -162,  -162,    58,
    -162,  -162,  -162,  -162,  -162,   378,  -162,   310,   268,   128,
    -162,   426,   263,  -162,   -13,  -162,   426,  -162,  -162,   145,
     130,   137,   137,    92,    92,    92,    92,   177,   177,  -162,
    -162,  -162,  -162,    72,  -162,  -162,  -162,    60,   426,   426,
     281,   154,   128,  -162,  -162,    32,   252,  -162,   283,  -162,
     199,   385,  -162,   269,    11,    35,    85,   237,  -162,   426,
    -162,    90,   272,   274,  -162,  -162,    66,  -162,    80,  -162,
    -162,    74,   108,  -162,   218,   209,  -162,  -162,   278,  -162,
    -162,   275,  -162,  -162,  -162,   139,   108,  -162,  -162,   128,
     426,   426,   282,  -162,  -162,  -162,  -162,   108,  -162,   128,
     284,   241,  -162,   128,  -162,  -162,  -162,    75,  -162,   277,
      98,  -162,  -162,   426,   128,  -162,   249,  -162,   426,   128,
     100,  -162,   128,   286,  -162,   128,  -162,   128,  -162,  -162
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -162,  -162,  -162,  -162,   287,   304,   314,     0,   316,   288,
      -2,  -138,  -162,  -162,  -162,   -19,   -21,  -162,  -162,   223,
     230,   164,    86,   168,   -16,    77,  -162,  -162,  -162,  -162,
     180,  -162,  -162,  -115,  -162,  -162,   101,  -162,  -162,  -162,
    -162,  -161,  -162,   312,  -162,  -162,  -162,  -162,   133,  -134,
    -162,  -162,  -162,   138,  -162,  -150,   134,  -162
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      79,    40,    40,    85,    84,   179,   184,    72,   192,    87,
      12,    92,    89,    13,    14,    15,    16,    17,    18,    75,
      73,   201,   177,   144,    19,    20,    21,    22,    23,   145,
      24,    90,   146,   197,    94,   212,   134,     9,   204,   205,
      74,   135,   214,    31,    32,    33,    34,    25,   201,    95,
     213,    26,    27,    28,    95,     2,    74,    95,   129,   239,
     197,   207,    29,   225,   228,    30,    31,    32,    33,    34,
      35,    72,   133,   222,    36,    37,   148,   172,    98,    97,
     111,   236,   247,   190,    97,   238,    97,   253,    95,   241,
     164,   188,   159,   160,   161,   242,    76,    95,   189,   246,
     166,   223,    88,   167,   215,    88,   113,     1,     2,   219,
     251,   216,   119,   120,   174,   254,    97,   249,   256,   255,
      77,   258,   180,   259,    97,   111,    97,   187,   106,   107,
      31,    32,    33,    34,   112,   175,    31,    32,    33,    34,
     194,   195,   185,    81,    36,    37,   194,   195,    90,   191,
      79,   113,   114,   115,   116,   117,   118,   119,   120,    90,
     100,   101,   180,   235,    31,    32,    33,    34,   218,   102,
     103,   104,   105,    99,   175,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,   153,   154,
     155,   156,    13,    14,    15,    16,    17,    18,    96,    40,
      97,    79,   240,    19,    20,    21,    22,    23,   122,    24,
      31,    32,    33,    34,   237,   108,   109,   110,    36,    37,
     194,   195,   209,   126,   250,    97,    25,   229,   230,    79,
      26,    27,    28,   145,   138,   139,   146,   123,   124,   125,
     140,    29,    97,   127,    30,    31,    32,    33,    34,    35,
     244,   230,   128,    36,    37,    13,    14,    15,    16,    17,
      18,   217,   130,    97,   151,   152,    19,    20,    21,    22,
      23,    97,    24,   206,   157,   158,   136,   131,   132,   141,
     165,   147,   168,   169,   170,   181,   178,   171,   193,    25,
     208,   211,    70,    26,    27,    28,   220,   233,   234,   221,
     144,   248,   243,   252,    29,   257,    11,    30,    31,    32,
      33,    34,    35,    13,    14,    15,    16,    17,    18,    71,
      69,   149,   182,    93,    19,    20,    21,    22,    23,   150,
      24,     0,   245,    83,   226,   227,   224,     0,     0,     0,
      13,    14,    15,    16,    78,     0,     0,    25,     0,     0,
       0,    26,    27,    28,    22,    23,   162,    24,     0,     0,
       0,     0,    29,     0,     0,    30,    31,    32,    33,    34,
      35,     0,     0,     0,    25,     0,     0,     0,    26,    27,
      28,    13,    14,    15,    16,    78,     0,     0,    13,    14,
      15,    16,    78,     0,     0,    22,    23,    35,    24,   173,
       0,     0,    22,    23,     0,    24,     0,     0,   210,     0,
       0,     0,     0,     0,     0,    25,     0,     0,     0,    26,
      27,    28,    25,     0,     0,     0,    26,    27,    28,    13,
      14,    15,    16,    78,     0,     0,     0,     0,    35,     0,
       0,     0,     0,    22,    23,    35,    24,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,     0,     0,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    35
};

static const yytype_int16 yycheck[] =
{
      19,     3,     4,    24,    23,   139,    19,     7,   169,    25,
       0,    30,    28,     3,     4,     5,     6,     7,     8,     7,
      24,   171,   137,    18,    14,    15,    16,    17,    18,    24,
      20,    20,    27,   171,     7,    24,    21,     7,   172,     7,
      44,    26,     7,    56,    57,    58,    59,    37,   198,    22,
     184,    41,    42,    43,    22,    62,    44,    22,    77,   220,
     198,   176,    52,   201,   202,    55,    56,    57,    58,    59,
      60,    71,    19,     7,    64,    65,    97,    19,    29,    26,
      18,   215,     7,    23,    26,   219,    26,   248,    22,   227,
     111,    19,   108,   109,   110,   229,    25,    22,    26,   233,
     121,    21,    25,   122,    19,    28,    44,    61,    62,    19,
     244,    26,    50,    51,   135,   249,    26,    19,   252,    19,
      18,   255,   141,   257,    26,    18,    26,   146,    36,    37,
      56,    57,    58,    59,    27,   137,    56,    57,    58,    59,
      66,    67,   144,     7,    64,    65,    66,    67,    20,   168,
     169,    44,    45,    46,    47,    48,    49,    50,    51,    20,
      30,    31,   181,    24,    56,    57,    58,    59,   189,    32,
      33,    34,    35,    28,   176,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   102,   103,
     104,   105,     3,     4,     5,     6,     7,     8,    24,   201,
      26,   220,   221,    14,    15,    16,    17,    18,    22,    20,
      56,    57,    58,    59,   216,    38,    39,    40,    64,    65,
      66,    67,    23,    63,   243,    26,    37,     9,    10,   248,
      41,    42,    43,    24,    53,    54,    27,    11,    12,    13,
      24,    52,    26,     7,    55,    56,    57,    58,    59,    60,
       9,    10,    24,    64,    65,     3,     4,     5,     6,     7,
       8,    24,    24,    26,   100,   101,    14,    15,    16,    17,
      18,    26,    20,    21,   106,   107,    21,    24,    24,    22,
       7,    23,    18,    18,    18,    22,    18,    20,     7,    37,
       7,    22,     5,    41,    42,    43,    24,    19,    23,    25,
      18,    24,    18,    54,    52,    19,     2,    55,    56,    57,
      58,    59,    60,     3,     4,     5,     6,     7,     8,     5,
       4,    98,   142,    35,    14,    15,    16,    17,    18,    99,
      20,    -1,   231,    21,   201,   201,   198,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    37,    -1,    -1,
      -1,    41,    42,    43,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    55,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    37,    -1,    -1,    -1,    41,    42,
      43,     3,     4,     5,     6,     7,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    17,    18,    60,    20,    21,
      -1,    -1,    17,    18,    -1,    20,    -1,    -1,    23,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,    41,
      42,    43,    37,    -1,    -1,    -1,    41,    42,    43,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    60,    -1,
      -1,    -1,    -1,    17,    18,    60,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    61,    62,    69,    70,    71,    72,    74,    75,     7,
      73,    73,     0,     3,     4,     5,     6,     7,     8,    14,
      15,    16,    17,    18,    20,    37,    41,    42,    43,    52,
      55,    56,    57,    58,    59,    60,    64,    65,    76,    77,
      78,    79,    83,    84,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,   101,   102,   105,   106,   107,
     108,   110,   112,   113,   114,   115,   116,   119,   123,    76,
      72,    74,    75,    24,    44,     7,    25,    18,     7,    83,
     109,     7,   111,   111,    83,    84,   100,    92,    93,    92,
      20,   117,    83,    77,     7,    22,    24,    26,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    18,    27,    44,    45,    46,    47,    48,    49,    50,
      51,    85,    22,    11,    12,    13,    63,     7,    24,    83,
      24,    24,    24,    19,    21,    26,    21,   118,    53,    54,
      24,    22,    97,    98,    18,    24,    27,    23,    84,    87,
      88,    89,    89,    90,    90,    90,    90,    91,    91,    92,
      92,    92,    19,    81,    84,     7,    84,    83,    18,    18,
      18,    20,    19,    21,    84,    78,    82,   101,    18,   117,
      83,    22,    98,    99,    19,    78,    80,    83,    19,    26,
      23,    83,   109,     7,    66,    67,    78,    79,   120,   121,
     122,   123,   124,   125,   117,     7,    21,   101,     7,    23,
      23,    22,    24,   117,     7,    19,    26,    24,    84,    19,
      24,    25,     7,    21,   121,    79,   116,   124,    79,     9,
      10,   103,   104,    19,    23,    24,   117,    78,   117,   109,
      83,    79,   117,    18,     9,   104,   117,     7,    24,    19,
      83,   117,    54,   109,   117,    19,   117,    19,   117,   117
};

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL          goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      yytoken = YYTRANSLATE (yychar);                           \
      YYPOPSTACK (1);                                           \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (YYID (0))


#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                    \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)                  \
     fprintf (File, "%d.%d-%d.%d",                      \
              (Loc).first_line, (Loc).first_column,     \
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
         constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
                    + sizeof yyexpecting - 1
                    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
                       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
          {
            if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
              {
                yycount = 1;
                yysize = yysize0;
                yyformat[sizeof yyunexpected - 1] = '\0';
                break;
              }
            yyarg[yycount++] = yytname[yyx];
            yysize1 = yysize + yytnamerr (0, yytname[yyx]);
            yysize_overflow |= (yysize1 < yysize);
            yysize = yysize1;
            yyfmt = yystpcpy (yyfmt, yyprefix);
            yyprefix = yyor;
          }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
        return YYSIZE_MAXIMUM;

      if (yyresult)
        {
          /* Avoid sprintf, as that infringes on the user's name space.
             Don't have undefined behavior even if the translation
             produced a string with the wrong number of "%s"s.  */
          char *yyp = yyresult;
          int yyi = 0;
          while ((*yyp = *yyf) != '\0')
            {
              if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
                {
                  yyp += yytnamerr (yyp, yyarg[yyi++]);
                  yyf += 2;
                }
              else
                {
                  yyp++;
                  yyf++;
                }
            }
        }
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;             /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;


        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),

                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss);
        YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 69 "diksam.y"
    {
            dkc_set_require_and_rename_list((yyvsp[(1) - (2)].require_list), (yyvsp[(2) - (2)].rename_list));
        ;}
    break;

  case 6:
#line 73 "diksam.y"
    {
            dkc_set_require_and_rename_list((yyvsp[(1) - (1)].require_list), NULL);
        ;}
    break;

  case 7:
#line 77 "diksam.y"
    {
            dkc_set_require_and_rename_list(NULL, (yyvsp[(1) - (1)].rename_list));
        ;}
    break;

  case 9:
#line 84 "diksam.y"
    {
            (yyval.require_list) = dkc_chain_require_list((yyvsp[(1) - (2)].require_list), (yyvsp[(2) - (2)].require_list));
        ;}
    break;

  case 10:
#line 90 "diksam.y"
    {
            (yyval.require_list) = dkc_create_require_list((yyvsp[(2) - (3)].package_name));
        ;}
    break;

  case 11:
#line 96 "diksam.y"
    {
            (yyval.package_name) = dkc_create_package_name((yyvsp[(1) - (1)].identifier));
        ;}
    break;

  case 12:
#line 100 "diksam.y"
    {
            (yyval.package_name) = dkc_chain_package_name((yyvsp[(1) - (3)].package_name), (yyvsp[(3) - (3)].identifier));
        ;}
    break;

  case 14:
#line 107 "diksam.y"
    {
            (yyval.rename_list) = dkc_chain_rename_list((yyvsp[(1) - (2)].rename_list), (yyvsp[(2) - (2)].rename_list));
        ;}
    break;

  case 15:
#line 113 "diksam.y"
    {
            (yyval.rename_list) = dkc_create_rename_list((yyvsp[(2) - (4)].package_name), (yyvsp[(3) - (4)].identifier));
        ;}
    break;

  case 18:
#line 121 "diksam.y"
    {
            DKC_Compiler *compiler = dkc_get_current_compiler();

            compiler->statement_list
                = dkc_chain_statement_list(compiler->statement_list, (yyvsp[(1) - (1)].statement));
        ;}
    break;

  case 19:
#line 130 "diksam.y"
    {
            (yyval.basic_type_specifier) = DVM_BOOLEAN_TYPE;
        ;}
    break;

  case 20:
#line 134 "diksam.y"
    {
            (yyval.basic_type_specifier) = DVM_INT_TYPE;
        ;}
    break;

  case 21:
#line 138 "diksam.y"
    {
            (yyval.basic_type_specifier) = DVM_DOUBLE_TYPE;
        ;}
    break;

  case 22:
#line 142 "diksam.y"
    {
            (yyval.basic_type_specifier) = DVM_STRING_TYPE;
        ;}
    break;

  case 23:
#line 148 "diksam.y"
    {
            (yyval.type_specifier) = dkc_create_type_specifier((yyvsp[(1) - (1)].basic_type_specifier));
        ;}
    break;

  case 24:
#line 152 "diksam.y"
    {
            (yyval.type_specifier) = dkc_create_array_type_specifier((yyvsp[(1) - (3)].type_specifier));
        ;}
    break;

  case 25:
#line 158 "diksam.y"
    {
            dkc_function_define((yyvsp[(1) - (6)].type_specifier), (yyvsp[(2) - (6)].identifier), (yyvsp[(4) - (6)].parameter_list), (yyvsp[(6) - (6)].block));
        ;}
    break;

  case 26:
#line 162 "diksam.y"
    {
            dkc_function_define((yyvsp[(1) - (5)].type_specifier), (yyvsp[(2) - (5)].identifier), NULL, (yyvsp[(5) - (5)].block));
        ;}
    break;

  case 27:
#line 166 "diksam.y"
    {
            dkc_function_define((yyvsp[(1) - (6)].type_specifier), (yyvsp[(2) - (6)].identifier), (yyvsp[(4) - (6)].parameter_list), NULL);
        ;}
    break;

  case 28:
#line 170 "diksam.y"
    {
            dkc_function_define((yyvsp[(1) - (5)].type_specifier), (yyvsp[(2) - (5)].identifier), NULL, NULL);
        ;}
    break;

  case 29:
#line 176 "diksam.y"
    {
            (yyval.parameter_list) = dkc_create_parameter((yyvsp[(1) - (2)].type_specifier), (yyvsp[(2) - (2)].identifier));
        ;}
    break;

  case 30:
#line 180 "diksam.y"
    {
            (yyval.parameter_list) = dkc_chain_parameter((yyvsp[(1) - (4)].parameter_list), (yyvsp[(3) - (4)].type_specifier), (yyvsp[(4) - (4)].identifier));
        ;}
    break;

  case 31:
#line 186 "diksam.y"
    {
            (yyval.argument_list) = dkc_create_argument_list((yyvsp[(1) - (1)].expression));
        ;}
    break;

  case 32:
#line 190 "diksam.y"
    {
            (yyval.argument_list) = dkc_chain_argument_list((yyvsp[(1) - (3)].argument_list), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 33:
#line 196 "diksam.y"
    {
            (yyval.statement_list) = dkc_create_statement_list((yyvsp[(1) - (1)].statement));
        ;}
    break;

  case 34:
#line 200 "diksam.y"
    {
            (yyval.statement_list) = dkc_chain_statement_list((yyvsp[(1) - (2)].statement_list), (yyvsp[(2) - (2)].statement));
        ;}
    break;

  case 36:
#line 207 "diksam.y"
    {
            (yyval.expression) = dkc_create_comma_expression((yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 38:
#line 214 "diksam.y"
    {
            (yyval.expression) = dkc_create_assign_expression((yyvsp[(1) - (3)].expression), (yyvsp[(2) - (3)].assignment_operator), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 39:
#line 220 "diksam.y"
    {
            (yyval.assignment_operator) = NORMAL_ASSIGN;
        ;}
    break;

  case 40:
#line 224 "diksam.y"
    {
            (yyval.assignment_operator) = ADD_ASSIGN;
        ;}
    break;

  case 41:
#line 228 "diksam.y"
    {
            (yyval.assignment_operator) = SUB_ASSIGN;
        ;}
    break;

  case 42:
#line 232 "diksam.y"
    {
            (yyval.assignment_operator) = MUL_ASSIGN;
        ;}
    break;

  case 43:
#line 236 "diksam.y"
    {
            (yyval.assignment_operator) = DIV_ASSIGN;
        ;}
    break;

  case 44:
#line 240 "diksam.y"
    {
            (yyval.assignment_operator) = MOD_ASSIGN;
        ;}
    break;

  case 46:
#line 247 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(LOGICAL_OR_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 48:
#line 254 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(LOGICAL_AND_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 50:
#line 261 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(EQ_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 51:
#line 265 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(NE_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 53:
#line 272 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(GT_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 54:
#line 276 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(GE_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 55:
#line 280 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(LT_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 56:
#line 284 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(LE_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 58:
#line 291 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(ADD_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 59:
#line 295 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(SUB_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 61:
#line 302 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(MUL_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 62:
#line 306 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(DIV_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 63:
#line 310 "diksam.y"
    {
            (yyval.expression) = dkc_create_binary_expression(MOD_EXPRESSION, (yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 65:
#line 317 "diksam.y"
    {
            (yyval.expression) = dkc_create_minus_expression((yyvsp[(2) - (2)].expression));
        ;}
    break;

  case 66:
#line 321 "diksam.y"
    {
            (yyval.expression) = dkc_create_logical_not_expression((yyvsp[(2) - (2)].expression));
        ;}
    break;

  case 69:
#line 331 "diksam.y"
    {
            (yyval.expression) = dkc_create_index_expression((yyvsp[(1) - (4)].expression), (yyvsp[(3) - (4)].expression));
        ;}
    break;

  case 70:
#line 335 "diksam.y"
    {
            (yyval.expression) = dkc_create_member_expression((yyvsp[(1) - (3)].expression), (yyvsp[(3) - (3)].identifier));
        ;}
    break;

  case 71:
#line 339 "diksam.y"
    {
            (yyval.expression) = dkc_create_function_call_expression((yyvsp[(1) - (4)].expression), (yyvsp[(3) - (4)].argument_list));
        ;}
    break;

  case 72:
#line 343 "diksam.y"
    {
            (yyval.expression) = dkc_create_function_call_expression((yyvsp[(1) - (3)].expression), NULL);
        ;}
    break;

  case 73:
#line 347 "diksam.y"
    {
            (yyval.expression) = dkc_create_incdec_expression((yyvsp[(1) - (2)].expression), INCREMENT_EXPRESSION);
        ;}
    break;

  case 74:
#line 351 "diksam.y"
    {
            (yyval.expression) = dkc_create_incdec_expression((yyvsp[(1) - (2)].expression), DECREMENT_EXPRESSION);
        ;}
    break;

  case 75:
#line 355 "diksam.y"
    {
            (yyval.expression) = (yyvsp[(2) - (3)].expression);
        ;}
    break;

  case 76:
#line 359 "diksam.y"
    {
            (yyval.expression) = dkc_create_identifier_expression((yyvsp[(1) - (1)].identifier));
        ;}
    break;

  case 81:
#line 367 "diksam.y"
    {
            (yyval.expression) = dkc_create_boolean_expression(DVM_TRUE);
        ;}
    break;

  case 82:
#line 371 "diksam.y"
    {
            (yyval.expression) = dkc_create_boolean_expression(DVM_FALSE);
        ;}
    break;

  case 83:
#line 375 "diksam.y"
    {
            (yyval.expression) = dkc_create_null_expression();
        ;}
    break;

  case 85:
#line 382 "diksam.y"
    {
            (yyval.expression) = dkc_create_array_literal_expression((yyvsp[(2) - (3)].expression_list));
        ;}
    break;

  case 86:
#line 386 "diksam.y"
    {
            (yyval.expression) = dkc_create_array_literal_expression((yyvsp[(2) - (4)].expression_list));
        ;}
    break;

  case 87:
#line 392 "diksam.y"
    {
            (yyval.expression) = dkc_create_array_creation((yyvsp[(2) - (3)].basic_type_specifier), (yyvsp[(3) - (3)].array_dimension), NULL);
        ;}
    break;

  case 88:
#line 396 "diksam.y"
    {
            (yyval.expression) = dkc_create_array_creation((yyvsp[(2) - (4)].basic_type_specifier), (yyvsp[(3) - (4)].array_dimension), (yyvsp[(4) - (4)].array_dimension));
        ;}
    break;

  case 90:
#line 403 "diksam.y"
    {
            (yyval.array_dimension) = dkc_chain_array_dimension((yyvsp[(1) - (2)].array_dimension), (yyvsp[(2) - (2)].array_dimension));
        ;}
    break;

  case 91:
#line 409 "diksam.y"
    {
            (yyval.array_dimension) = dkc_create_array_dimension((yyvsp[(2) - (3)].expression));
        ;}
    break;

  case 92:
#line 415 "diksam.y"
    {
            (yyval.array_dimension) = dkc_create_array_dimension(NULL);
        ;}
    break;

  case 93:
#line 419 "diksam.y"
    {
            (yyval.array_dimension) = dkc_chain_array_dimension((yyvsp[(1) - (3)].array_dimension),
                                           dkc_create_array_dimension(NULL));
        ;}
    break;

  case 94:
#line 426 "diksam.y"
    {
            (yyval.expression_list) = NULL;
        ;}
    break;

  case 95:
#line 430 "diksam.y"
    {
            (yyval.expression_list) = dkc_create_expression_list((yyvsp[(1) - (1)].expression));
        ;}
    break;

  case 96:
#line 434 "diksam.y"
    {
            (yyval.expression_list) = dkc_chain_expression_list((yyvsp[(1) - (3)].expression_list), (yyvsp[(3) - (3)].expression));
        ;}
    break;

  case 97:
#line 440 "diksam.y"
    {
          (yyval.statement) = dkc_create_expression_statement((yyvsp[(1) - (2)].expression));
        ;}
    break;

  case 108:
#line 456 "diksam.y"
    {
            (yyval.statement) = dkc_create_if_statement((yyvsp[(3) - (5)].expression), (yyvsp[(5) - (5)].block), NULL, NULL);
        ;}
    break;

  case 109:
#line 460 "diksam.y"
    {
            (yyval.statement) = dkc_create_if_statement((yyvsp[(3) - (7)].expression), (yyvsp[(5) - (7)].block), NULL, (yyvsp[(7) - (7)].block));
        ;}
    break;

  case 110:
#line 464 "diksam.y"
    {
            (yyval.statement) = dkc_create_if_statement((yyvsp[(3) - (6)].expression), (yyvsp[(5) - (6)].block), (yyvsp[(6) - (6)].elsif), NULL);
        ;}
    break;

  case 111:
#line 468 "diksam.y"
    {
            (yyval.statement) = dkc_create_if_statement((yyvsp[(3) - (8)].expression), (yyvsp[(5) - (8)].block), (yyvsp[(6) - (8)].elsif), (yyvsp[(8) - (8)].block));
        ;}
    break;

  case 113:
#line 475 "diksam.y"
    {
            (yyval.elsif) = dkc_chain_elsif_list((yyvsp[(1) - (2)].elsif), (yyvsp[(2) - (2)].elsif));
        ;}
    break;

  case 114:
#line 481 "diksam.y"
    {
            (yyval.elsif) = dkc_create_elsif((yyvsp[(3) - (5)].expression), (yyvsp[(5) - (5)].block));
        ;}
    break;

  case 115:
#line 487 "diksam.y"
    {
            (yyval.identifier) = NULL;
        ;}
    break;

  case 116:
#line 491 "diksam.y"
    {
            (yyval.identifier) = (yyvsp[(1) - (2)].identifier);
        ;}
    break;

  case 117:
#line 497 "diksam.y"
    {
            (yyval.statement) = dkc_create_while_statement((yyvsp[(1) - (6)].identifier), (yyvsp[(4) - (6)].expression), (yyvsp[(6) - (6)].block));
        ;}
    break;

  case 118:
#line 504 "diksam.y"
    {
            (yyval.statement) = dkc_create_for_statement((yyvsp[(1) - (10)].identifier), (yyvsp[(4) - (10)].expression), (yyvsp[(6) - (10)].expression), (yyvsp[(8) - (10)].expression), (yyvsp[(10) - (10)].block));
        ;}
    break;

  case 119:
#line 510 "diksam.y"
    {
            (yyval.statement) = dkc_create_foreach_statement((yyvsp[(1) - (8)].identifier), (yyvsp[(4) - (8)].identifier), (yyvsp[(6) - (8)].expression), (yyvsp[(8) - (8)].block));
        ;}
    break;

  case 120:
#line 516 "diksam.y"
    {
            (yyval.expression) = NULL;
        ;}
    break;

  case 122:
#line 523 "diksam.y"
    {
            (yyval.statement) = dkc_create_return_statement((yyvsp[(2) - (3)].expression));
        ;}
    break;

  case 123:
#line 529 "diksam.y"
    {
            (yyval.identifier) = NULL;
        ;}
    break;

  case 125:
#line 536 "diksam.y"
    {
            (yyval.statement) = dkc_create_break_statement((yyvsp[(2) - (3)].identifier));
        ;}
    break;

  case 126:
#line 542 "diksam.y"
    {
            (yyval.statement) = dkc_create_continue_statement((yyvsp[(2) - (3)].identifier));
        ;}
    break;

  case 127:
#line 548 "diksam.y"
    {
            (yyval.statement) = dkc_create_try_statement((yyvsp[(2) - (9)].block), (yyvsp[(5) - (9)].identifier), (yyvsp[(7) - (9)].block), (yyvsp[(9) - (9)].block));
        ;}
    break;

  case 128:
#line 552 "diksam.y"
    {
            (yyval.statement) = dkc_create_try_statement((yyvsp[(2) - (4)].block), NULL, NULL, (yyvsp[(4) - (4)].block));
        ;}
    break;

  case 129:
#line 556 "diksam.y"
    {
            (yyval.statement) = dkc_create_try_statement((yyvsp[(2) - (7)].block), (yyvsp[(5) - (7)].identifier), (yyvsp[(7) - (7)].block), NULL);
        ;}
    break;

  case 130:
#line 561 "diksam.y"
    {
            (yyval.statement) = dkc_create_throw_statement((yyvsp[(2) - (3)].expression));
        ;}
    break;

  case 131:
#line 566 "diksam.y"
    {
            (yyval.statement) = dkc_create_declaration_statement((yyvsp[(1) - (3)].type_specifier), (yyvsp[(2) - (3)].identifier), NULL);
        ;}
    break;

  case 132:
#line 570 "diksam.y"
    {
            (yyval.statement) = dkc_create_declaration_statement((yyvsp[(1) - (5)].type_specifier), (yyvsp[(2) - (5)].identifier), (yyvsp[(4) - (5)].expression));
        ;}
    break;

  case 133:
#line 576 "diksam.y"
    {
            (yyval.block) = dkc_open_block();
        ;}
    break;

  case 134:
#line 580 "diksam.y"
    {
            (yyval.block) = dkc_close_block((yyvsp[(2) - (4)].block), (yyvsp[(3) - (4)].statement_list));
        ;}
    break;

  case 135:
#line 584 "diksam.y"
    {
            Block *empty_block = dkc_open_block();
            (yyval.block) = dkc_close_block(empty_block, NULL);
        ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2459 "diksam.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
        YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
        if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
          {
            YYSIZE_T yyalloc = 2 * yysize;
            if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
              yyalloc = YYSTACK_ALLOC_MAXIMUM;
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yyalloc);
            if (yymsg)
              yymsg_alloc = yyalloc;
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
              }
          }

        if (0 < yysize && yysize <= yymsg_alloc)
          {
            (void) yysyntax_error (yymsg, yystate, yychar);
            yyerror (yymsg);
          }
        else
          {
            yyerror (YY_("syntax error"));
            if (yysize != 0)
              goto yyexhaustedlab;
          }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
                 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 618 "diksam.y"


