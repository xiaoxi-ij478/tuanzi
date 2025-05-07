/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         pcap_parse
#define yylex           pcap_lex
#define yyerror         pcap_error
#define yydebug         pcap_debug
#define yynerrs         pcap_nerrs
#define yylval          pcap_lval
#define yychar          pcap_char

/* First part of user prologue.  */
#line 1 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/grammar.y,v 1.86.2.5 2005/09/05 09:08:06 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <pcap-stdinc.h>
#else /* WIN32 */
#include <sys/types.h>
#include <sys/socket.h>
#endif /* WIN32 */

#include <stdlib.h>

#ifndef WIN32
#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <netinet/in.h>
#endif /* WIN32 */

#include <stdio.h>

#include "pcap-int.h"

#include "gencode.h"
#include "pf.h"
#include <pcap-namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(char *msg)
{
	++n_errors;
	bpf_error("%s", msg);
	/* NOTREACHED */
}

#ifndef YYBISON
int yyparse(void);

int
pcap_parse()
{
	return (yyparse());
}
#endif


#line 167 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_PCAP_Y_TAB_H_INCLUDED
# define YY_PCAP_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int pcap_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    DST = 258,                     /* DST  */
    SRC = 259,                     /* SRC  */
    HOST = 260,                    /* HOST  */
    GATEWAY = 261,                 /* GATEWAY  */
    NET = 262,                     /* NET  */
    NETMASK = 263,                 /* NETMASK  */
    PORT = 264,                    /* PORT  */
    PORTRANGE = 265,               /* PORTRANGE  */
    LESS = 266,                    /* LESS  */
    GREATER = 267,                 /* GREATER  */
    PROTO = 268,                   /* PROTO  */
    PROTOCHAIN = 269,              /* PROTOCHAIN  */
    CBYTE = 270,                   /* CBYTE  */
    ARP = 271,                     /* ARP  */
    RARP = 272,                    /* RARP  */
    IP = 273,                      /* IP  */
    SCTP = 274,                    /* SCTP  */
    TCP = 275,                     /* TCP  */
    UDP = 276,                     /* UDP  */
    ICMP = 277,                    /* ICMP  */
    IGMP = 278,                    /* IGMP  */
    IGRP = 279,                    /* IGRP  */
    PIM = 280,                     /* PIM  */
    VRRP = 281,                    /* VRRP  */
    ATALK = 282,                   /* ATALK  */
    AARP = 283,                    /* AARP  */
    DECNET = 284,                  /* DECNET  */
    LAT = 285,                     /* LAT  */
    SCA = 286,                     /* SCA  */
    MOPRC = 287,                   /* MOPRC  */
    MOPDL = 288,                   /* MOPDL  */
    TK_BROADCAST = 289,            /* TK_BROADCAST  */
    TK_MULTICAST = 290,            /* TK_MULTICAST  */
    NUM = 291,                     /* NUM  */
    INBOUND = 292,                 /* INBOUND  */
    OUTBOUND = 293,                /* OUTBOUND  */
    PF_IFNAME = 294,               /* PF_IFNAME  */
    PF_RSET = 295,                 /* PF_RSET  */
    PF_RNR = 296,                  /* PF_RNR  */
    PF_SRNR = 297,                 /* PF_SRNR  */
    PF_REASON = 298,               /* PF_REASON  */
    PF_ACTION = 299,               /* PF_ACTION  */
    LINK = 300,                    /* LINK  */
    GEQ = 301,                     /* GEQ  */
    LEQ = 302,                     /* LEQ  */
    NEQ = 303,                     /* NEQ  */
    ID = 304,                      /* ID  */
    EID = 305,                     /* EID  */
    HID = 306,                     /* HID  */
    HID6 = 307,                    /* HID6  */
    AID = 308,                     /* AID  */
    LSH = 309,                     /* LSH  */
    RSH = 310,                     /* RSH  */
    LEN = 311,                     /* LEN  */
    IPV6 = 312,                    /* IPV6  */
    ICMPV6 = 313,                  /* ICMPV6  */
    AH = 314,                      /* AH  */
    ESP = 315,                     /* ESP  */
    VLAN = 316,                    /* VLAN  */
    MPLS = 317,                    /* MPLS  */
    PPPOED = 318,                  /* PPPOED  */
    PPPOES = 319,                  /* PPPOES  */
    ISO = 320,                     /* ISO  */
    ESIS = 321,                    /* ESIS  */
    CLNP = 322,                    /* CLNP  */
    ISIS = 323,                    /* ISIS  */
    L1 = 324,                      /* L1  */
    L2 = 325,                      /* L2  */
    IIH = 326,                     /* IIH  */
    LSP = 327,                     /* LSP  */
    SNP = 328,                     /* SNP  */
    CSNP = 329,                    /* CSNP  */
    PSNP = 330,                    /* PSNP  */
    STP = 331,                     /* STP  */
    IPX = 332,                     /* IPX  */
    NETBEUI = 333,                 /* NETBEUI  */
    LANE = 334,                    /* LANE  */
    LLC = 335,                     /* LLC  */
    METAC = 336,                   /* METAC  */
    BCC = 337,                     /* BCC  */
    SC = 338,                      /* SC  */
    ILMIC = 339,                   /* ILMIC  */
    OAMF4EC = 340,                 /* OAMF4EC  */
    OAMF4SC = 341,                 /* OAMF4SC  */
    OAM = 342,                     /* OAM  */
    OAMF4 = 343,                   /* OAMF4  */
    CONNECTMSG = 344,              /* CONNECTMSG  */
    METACONNECT = 345,             /* METACONNECT  */
    VPI = 346,                     /* VPI  */
    VCI = 347,                     /* VCI  */
    RADIO = 348,                   /* RADIO  */
    SIO = 349,                     /* SIO  */
    OPC = 350,                     /* OPC  */
    DPC = 351,                     /* DPC  */
    SLS = 352,                     /* SLS  */
    OR = 353,                      /* OR  */
    AND = 354,                     /* AND  */
    UMINUS = 355                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define DST 258
#define SRC 259
#define HOST 260
#define GATEWAY 261
#define NET 262
#define NETMASK 263
#define PORT 264
#define PORTRANGE 265
#define LESS 266
#define GREATER 267
#define PROTO 268
#define PROTOCHAIN 269
#define CBYTE 270
#define ARP 271
#define RARP 272
#define IP 273
#define SCTP 274
#define TCP 275
#define UDP 276
#define ICMP 277
#define IGMP 278
#define IGRP 279
#define PIM 280
#define VRRP 281
#define ATALK 282
#define AARP 283
#define DECNET 284
#define LAT 285
#define SCA 286
#define MOPRC 287
#define MOPDL 288
#define TK_BROADCAST 289
#define TK_MULTICAST 290
#define NUM 291
#define INBOUND 292
#define OUTBOUND 293
#define PF_IFNAME 294
#define PF_RSET 295
#define PF_RNR 296
#define PF_SRNR 297
#define PF_REASON 298
#define PF_ACTION 299
#define LINK 300
#define GEQ 301
#define LEQ 302
#define NEQ 303
#define ID 304
#define EID 305
#define HID 306
#define HID6 307
#define AID 308
#define LSH 309
#define RSH 310
#define LEN 311
#define IPV6 312
#define ICMPV6 313
#define AH 314
#define ESP 315
#define VLAN 316
#define MPLS 317
#define PPPOED 318
#define PPPOES 319
#define ISO 320
#define ESIS 321
#define CLNP 322
#define ISIS 323
#define L1 324
#define L2 325
#define IIH 326
#define LSP 327
#define SNP 328
#define CSNP 329
#define PSNP 330
#define STP 331
#define IPX 332
#define NETBEUI 333
#define LANE 334
#define LLC 335
#define METAC 336
#define BCC 337
#define SC 338
#define ILMIC 339
#define OAMF4EC 340
#define OAMF4SC 341
#define OAM 342
#define OAMF4 343
#define CONNECTMSG 344
#define METACONNECT 345
#define VPI 346
#define VCI 347
#define RADIO 348
#define SIO 349
#define OPC 350
#define DPC 351
#define SLS 352
#define OR 353
#define AND 354
#define UMINUS 355

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 90 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"

	int i;
	bpf_u_int32 h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		int atmfieldtype;
		int mtp3fieldtype;
		struct block *b;
	} blk;
	struct block *rblk;

#line 436 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE pcap_lval;


int pcap_parse (void);


#endif /* !YY_PCAP_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DST = 3,                        /* DST  */
  YYSYMBOL_SRC = 4,                        /* SRC  */
  YYSYMBOL_HOST = 5,                       /* HOST  */
  YYSYMBOL_GATEWAY = 6,                    /* GATEWAY  */
  YYSYMBOL_NET = 7,                        /* NET  */
  YYSYMBOL_NETMASK = 8,                    /* NETMASK  */
  YYSYMBOL_PORT = 9,                       /* PORT  */
  YYSYMBOL_PORTRANGE = 10,                 /* PORTRANGE  */
  YYSYMBOL_LESS = 11,                      /* LESS  */
  YYSYMBOL_GREATER = 12,                   /* GREATER  */
  YYSYMBOL_PROTO = 13,                     /* PROTO  */
  YYSYMBOL_PROTOCHAIN = 14,                /* PROTOCHAIN  */
  YYSYMBOL_CBYTE = 15,                     /* CBYTE  */
  YYSYMBOL_ARP = 16,                       /* ARP  */
  YYSYMBOL_RARP = 17,                      /* RARP  */
  YYSYMBOL_IP = 18,                        /* IP  */
  YYSYMBOL_SCTP = 19,                      /* SCTP  */
  YYSYMBOL_TCP = 20,                       /* TCP  */
  YYSYMBOL_UDP = 21,                       /* UDP  */
  YYSYMBOL_ICMP = 22,                      /* ICMP  */
  YYSYMBOL_IGMP = 23,                      /* IGMP  */
  YYSYMBOL_IGRP = 24,                      /* IGRP  */
  YYSYMBOL_PIM = 25,                       /* PIM  */
  YYSYMBOL_VRRP = 26,                      /* VRRP  */
  YYSYMBOL_ATALK = 27,                     /* ATALK  */
  YYSYMBOL_AARP = 28,                      /* AARP  */
  YYSYMBOL_DECNET = 29,                    /* DECNET  */
  YYSYMBOL_LAT = 30,                       /* LAT  */
  YYSYMBOL_SCA = 31,                       /* SCA  */
  YYSYMBOL_MOPRC = 32,                     /* MOPRC  */
  YYSYMBOL_MOPDL = 33,                     /* MOPDL  */
  YYSYMBOL_TK_BROADCAST = 34,              /* TK_BROADCAST  */
  YYSYMBOL_TK_MULTICAST = 35,              /* TK_MULTICAST  */
  YYSYMBOL_NUM = 36,                       /* NUM  */
  YYSYMBOL_INBOUND = 37,                   /* INBOUND  */
  YYSYMBOL_OUTBOUND = 38,                  /* OUTBOUND  */
  YYSYMBOL_PF_IFNAME = 39,                 /* PF_IFNAME  */
  YYSYMBOL_PF_RSET = 40,                   /* PF_RSET  */
  YYSYMBOL_PF_RNR = 41,                    /* PF_RNR  */
  YYSYMBOL_PF_SRNR = 42,                   /* PF_SRNR  */
  YYSYMBOL_PF_REASON = 43,                 /* PF_REASON  */
  YYSYMBOL_PF_ACTION = 44,                 /* PF_ACTION  */
  YYSYMBOL_LINK = 45,                      /* LINK  */
  YYSYMBOL_GEQ = 46,                       /* GEQ  */
  YYSYMBOL_LEQ = 47,                       /* LEQ  */
  YYSYMBOL_NEQ = 48,                       /* NEQ  */
  YYSYMBOL_ID = 49,                        /* ID  */
  YYSYMBOL_EID = 50,                       /* EID  */
  YYSYMBOL_HID = 51,                       /* HID  */
  YYSYMBOL_HID6 = 52,                      /* HID6  */
  YYSYMBOL_AID = 53,                       /* AID  */
  YYSYMBOL_LSH = 54,                       /* LSH  */
  YYSYMBOL_RSH = 55,                       /* RSH  */
  YYSYMBOL_LEN = 56,                       /* LEN  */
  YYSYMBOL_IPV6 = 57,                      /* IPV6  */
  YYSYMBOL_ICMPV6 = 58,                    /* ICMPV6  */
  YYSYMBOL_AH = 59,                        /* AH  */
  YYSYMBOL_ESP = 60,                       /* ESP  */
  YYSYMBOL_VLAN = 61,                      /* VLAN  */
  YYSYMBOL_MPLS = 62,                      /* MPLS  */
  YYSYMBOL_PPPOED = 63,                    /* PPPOED  */
  YYSYMBOL_PPPOES = 64,                    /* PPPOES  */
  YYSYMBOL_ISO = 65,                       /* ISO  */
  YYSYMBOL_ESIS = 66,                      /* ESIS  */
  YYSYMBOL_CLNP = 67,                      /* CLNP  */
  YYSYMBOL_ISIS = 68,                      /* ISIS  */
  YYSYMBOL_L1 = 69,                        /* L1  */
  YYSYMBOL_L2 = 70,                        /* L2  */
  YYSYMBOL_IIH = 71,                       /* IIH  */
  YYSYMBOL_LSP = 72,                       /* LSP  */
  YYSYMBOL_SNP = 73,                       /* SNP  */
  YYSYMBOL_CSNP = 74,                      /* CSNP  */
  YYSYMBOL_PSNP = 75,                      /* PSNP  */
  YYSYMBOL_STP = 76,                       /* STP  */
  YYSYMBOL_IPX = 77,                       /* IPX  */
  YYSYMBOL_NETBEUI = 78,                   /* NETBEUI  */
  YYSYMBOL_LANE = 79,                      /* LANE  */
  YYSYMBOL_LLC = 80,                       /* LLC  */
  YYSYMBOL_METAC = 81,                     /* METAC  */
  YYSYMBOL_BCC = 82,                       /* BCC  */
  YYSYMBOL_SC = 83,                        /* SC  */
  YYSYMBOL_ILMIC = 84,                     /* ILMIC  */
  YYSYMBOL_OAMF4EC = 85,                   /* OAMF4EC  */
  YYSYMBOL_OAMF4SC = 86,                   /* OAMF4SC  */
  YYSYMBOL_OAM = 87,                       /* OAM  */
  YYSYMBOL_OAMF4 = 88,                     /* OAMF4  */
  YYSYMBOL_CONNECTMSG = 89,                /* CONNECTMSG  */
  YYSYMBOL_METACONNECT = 90,               /* METACONNECT  */
  YYSYMBOL_VPI = 91,                       /* VPI  */
  YYSYMBOL_VCI = 92,                       /* VCI  */
  YYSYMBOL_RADIO = 93,                     /* RADIO  */
  YYSYMBOL_SIO = 94,                       /* SIO  */
  YYSYMBOL_OPC = 95,                       /* OPC  */
  YYSYMBOL_DPC = 96,                       /* DPC  */
  YYSYMBOL_SLS = 97,                       /* SLS  */
  YYSYMBOL_OR = 98,                        /* OR  */
  YYSYMBOL_AND = 99,                       /* AND  */
  YYSYMBOL_100_ = 100,                     /* '!'  */
  YYSYMBOL_101_ = 101,                     /* '|'  */
  YYSYMBOL_102_ = 102,                     /* '&'  */
  YYSYMBOL_103_ = 103,                     /* '+'  */
  YYSYMBOL_104_ = 104,                     /* '-'  */
  YYSYMBOL_105_ = 105,                     /* '*'  */
  YYSYMBOL_106_ = 106,                     /* '/'  */
  YYSYMBOL_UMINUS = 107,                   /* UMINUS  */
  YYSYMBOL_108_ = 108,                     /* ')'  */
  YYSYMBOL_109_ = 109,                     /* '('  */
  YYSYMBOL_110_ = 110,                     /* '>'  */
  YYSYMBOL_111_ = 111,                     /* '='  */
  YYSYMBOL_112_ = 112,                     /* '<'  */
  YYSYMBOL_113_ = 113,                     /* '['  */
  YYSYMBOL_114_ = 114,                     /* ']'  */
  YYSYMBOL_115_ = 115,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 116,                 /* $accept  */
  YYSYMBOL_prog = 117,                     /* prog  */
  YYSYMBOL_null = 118,                     /* null  */
  YYSYMBOL_expr = 119,                     /* expr  */
  YYSYMBOL_and = 120,                      /* and  */
  YYSYMBOL_or = 121,                       /* or  */
  YYSYMBOL_id = 122,                       /* id  */
  YYSYMBOL_nid = 123,                      /* nid  */
  YYSYMBOL_not = 124,                      /* not  */
  YYSYMBOL_paren = 125,                    /* paren  */
  YYSYMBOL_pid = 126,                      /* pid  */
  YYSYMBOL_qid = 127,                      /* qid  */
  YYSYMBOL_term = 128,                     /* term  */
  YYSYMBOL_head = 129,                     /* head  */
  YYSYMBOL_rterm = 130,                    /* rterm  */
  YYSYMBOL_pqual = 131,                    /* pqual  */
  YYSYMBOL_dqual = 132,                    /* dqual  */
  YYSYMBOL_aqual = 133,                    /* aqual  */
  YYSYMBOL_ndaqual = 134,                  /* ndaqual  */
  YYSYMBOL_pname = 135,                    /* pname  */
  YYSYMBOL_other = 136,                    /* other  */
  YYSYMBOL_pfvar = 137,                    /* pfvar  */
  YYSYMBOL_reason = 138,                   /* reason  */
  YYSYMBOL_action = 139,                   /* action  */
  YYSYMBOL_relop = 140,                    /* relop  */
  YYSYMBOL_irelop = 141,                   /* irelop  */
  YYSYMBOL_arth = 142,                     /* arth  */
  YYSYMBOL_narth = 143,                    /* narth  */
  YYSYMBOL_byteop = 144,                   /* byteop  */
  YYSYMBOL_pnum = 145,                     /* pnum  */
  YYSYMBOL_atmtype = 146,                  /* atmtype  */
  YYSYMBOL_atmmultitype = 147,             /* atmmultitype  */
  YYSYMBOL_atmfield = 148,                 /* atmfield  */
  YYSYMBOL_atmvalue = 149,                 /* atmvalue  */
  YYSYMBOL_atmfieldvalue = 150,            /* atmfieldvalue  */
  YYSYMBOL_atmlistvalue = 151,             /* atmlistvalue  */
  YYSYMBOL_mtp3field = 152,                /* mtp3field  */
  YYSYMBOL_mtp3value = 153,                /* mtp3value  */
  YYSYMBOL_mtp3fieldvalue = 154,           /* mtp3fieldvalue  */
  YYSYMBOL_mtp3listvalue = 155             /* mtp3listvalue  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   591

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  250

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   355


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   100,     2,     2,     2,     2,   102,     2,
     109,   108,   105,   103,     2,   104,     2,   106,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   115,     2,
     112,   111,   110,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   113,     2,   114,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   101,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   107
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   160,   160,   164,   166,   168,   169,   170,   171,   172,
     174,   176,   178,   179,   181,   183,   184,   186,   188,   193,
     202,   211,   220,   229,   231,   233,   235,   236,   237,   239,
     241,   243,   244,   246,   247,   248,   249,   250,   251,   253,
     254,   255,   256,   258,   260,   261,   262,   263,   264,   267,
     268,   271,   272,   273,   274,   275,   276,   279,   280,   281,
     282,   285,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   342,   343,   344,   345,   346,   347,
     350,   351,   364,   375,   376,   377,   379,   380,   381,   383,
     384,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   400,   401,   402,   403,   404,   406,
     407,   409,   410,   411,   412,   413,   414,   415,   416,   418,
     419,   420,   421,   424,   425,   427,   428,   429,   430,   432,
     439,   440,   443,   444,   445,   446,   448,   449,   450,   451,
     453,   462,   463
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "DST", "SRC", "HOST",
  "GATEWAY", "NET", "NETMASK", "PORT", "PORTRANGE", "LESS", "GREATER",
  "PROTO", "PROTOCHAIN", "CBYTE", "ARP", "RARP", "IP", "SCTP", "TCP",
  "UDP", "ICMP", "IGMP", "IGRP", "PIM", "VRRP", "ATALK", "AARP", "DECNET",
  "LAT", "SCA", "MOPRC", "MOPDL", "TK_BROADCAST", "TK_MULTICAST", "NUM",
  "INBOUND", "OUTBOUND", "PF_IFNAME", "PF_RSET", "PF_RNR", "PF_SRNR",
  "PF_REASON", "PF_ACTION", "LINK", "GEQ", "LEQ", "NEQ", "ID", "EID",
  "HID", "HID6", "AID", "LSH", "RSH", "LEN", "IPV6", "ICMPV6", "AH", "ESP",
  "VLAN", "MPLS", "PPPOED", "PPPOES", "ISO", "ESIS", "CLNP", "ISIS", "L1",
  "L2", "IIH", "LSP", "SNP", "CSNP", "PSNP", "STP", "IPX", "NETBEUI",
  "LANE", "LLC", "METAC", "BCC", "SC", "ILMIC", "OAMF4EC", "OAMF4SC",
  "OAM", "OAMF4", "CONNECTMSG", "METACONNECT", "VPI", "VCI", "RADIO",
  "SIO", "OPC", "DPC", "SLS", "OR", "AND", "'!'", "'|'", "'&'", "'+'",
  "'-'", "'*'", "'/'", "UMINUS", "')'", "'('", "'>'", "'='", "'<'", "'['",
  "']'", "':'", "$accept", "prog", "null", "expr", "and", "or", "id",
  "nid", "not", "paren", "pid", "qid", "term", "head", "rterm", "pqual",
  "dqual", "aqual", "ndaqual", "pname", "other", "pfvar", "reason",
  "action", "relop", "irelop", "arth", "narth", "byteop", "pnum",
  "atmtype", "atmmultitype", "atmfield", "atmvalue", "atmfieldvalue",
  "atmlistvalue", "mtp3field", "mtp3value", "mtp3fieldvalue",
  "mtp3listvalue", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-177)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-42)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -177,    15,   196,  -177,    -4,     0,    13,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,    -9,    10,
      17,    21,   -24,    14,  -177,  -177,  -177,  -177,  -177,  -177,
     -14,   -14,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,   480,  -177,
     -69,   386,   386,  -177,   -33,  -177,   556,     4,  -177,  -177,
     475,  -177,  -177,  -177,  -177,   133,   139,  -177,  -177,   -56,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,   -14,
    -177,  -177,   480,   -39,  -177,  -177,  -177,   291,   291,  -177,
     -71,   -31,   -27,  -177,  -177,     5,   -15,  -177,  -177,  -177,
     -33,   -33,  -177,   -30,   -26,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,    55,  -177,  -177,   480,  -177,  -177,
    -177,   480,   480,   480,   480,   480,   480,   480,   480,  -177,
    -177,  -177,   480,   480,  -177,    50,    63,    70,  -177,  -177,
    -177,    72,    94,   105,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,   112,   -27,    99,  -177,   291,   291,  -177,     2,  -177,
    -177,  -177,  -177,  -177,   106,   122,   123,  -177,  -177,    58,
     -69,   -27,   163,   164,   167,   168,  -177,    59,    41,    41,
     335,    16,    -1,    -1,  -177,  -177,    99,    99,  -177,   -77,
    -177,  -177,  -177,   -75,  -177,  -177,  -177,   -47,  -177,  -177,
    -177,  -177,   -33,   -33,  -177,  -177,  -177,  -177,  -177,   136,
    -177,    50,  -177,    72,  -177,  -177,    62,  -177,  -177,  -177
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,    50,     1,     0,     0,     0,    64,    65,    63,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    80,    79,   149,   105,   106,     0,     0,
       0,     0,     0,     0,    62,   143,    81,    82,    83,    84,
     108,   110,   111,   112,    85,    86,    95,    87,    88,    89,
      90,    91,    92,    94,    93,    96,    97,    98,   151,   152,
     153,   154,   157,   158,   155,   156,   159,   160,   161,   162,
     163,   164,    99,   172,   173,   174,   175,    24,     0,    25,
       2,    50,    50,     5,     0,    31,     0,    49,    44,   113,
       0,   130,   129,    45,    46,     0,     0,   102,   103,     0,
     114,   115,   116,   117,   120,   121,   118,   122,   119,     0,
     107,   109,     0,     0,   141,    11,    10,    50,    50,    32,
       0,   130,   129,    15,    21,    18,    20,    22,    39,    12,
       0,     0,    13,    52,    51,    57,    61,    58,    59,    60,
      36,    37,   100,   101,    34,    35,    38,     0,   124,   126,
     128,     0,     0,     0,     0,     0,     0,     0,     0,   123,
     125,   127,     0,     0,   169,     0,     0,     0,    47,   165,
     180,     0,     0,     0,    48,   176,   145,   144,   147,   148,
     146,     0,     0,     0,     7,    50,    50,     6,   129,     9,
       8,    40,   142,   150,     0,     0,     0,    23,    26,    30,
       0,    29,     0,     0,     0,     0,    33,     0,   139,   140,
     138,   137,   133,   134,   135,   136,    42,    43,   170,     0,
     166,   167,   181,     0,   177,   178,   104,   129,    17,    16,
      19,    14,     0,     0,    54,    56,    53,    55,   131,     0,
     168,     0,   179,     0,    27,    28,     0,   171,   182,   132
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -177,  -177,  -177,   175,   -22,  -176,   -83,  -120,     6,    -2,
    -177,  -177,   -76,  -177,  -177,  -177,  -177,    38,  -177,     8,
    -177,  -177,  -177,  -177,    44,    60,   -20,   -68,  -177,   -34,
    -177,  -177,  -177,  -177,  -156,  -177,  -177,  -177,  -145,  -177
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,   120,   117,   118,   197,   129,   130,   112,
     199,   200,    83,    84,    85,    86,   144,   145,   146,   113,
      88,    89,   106,   108,   162,   163,    90,    91,   181,    92,
      93,    94,    95,   168,   169,   219,    96,   174,   175,   223
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      82,   128,   -13,    25,   -41,   119,   110,   111,    81,   218,
      87,   198,   104,   194,   121,     3,   123,   124,   125,   126,
     127,   115,    25,   115,   233,   105,   222,   115,   116,   115,
     116,   240,    97,   242,   184,   189,    98,   191,   109,   109,
     100,   187,   190,   241,   121,   176,   177,   243,   122,    99,
     132,   -29,   -29,   102,   178,   179,   180,   103,   114,   101,
     135,   193,   137,   107,   138,   139,   198,    77,   202,   203,
     151,   152,   204,   205,   147,   182,    79,   192,   122,    82,
      82,   193,   131,   188,   188,   247,   164,    81,    81,    87,
      87,   196,   183,   165,   171,    79,   132,   201,   248,   220,
     -13,   -13,   -41,   -41,   157,   158,   221,   109,   170,   119,
     -13,   195,   -41,   151,   152,   186,   186,   147,   121,   155,
     156,   157,   158,   185,   185,    87,    87,   207,   131,   109,
     224,   208,   209,   210,   211,   212,   213,   214,   215,   166,
     172,   225,   216,   217,   155,   156,   157,   158,   226,   244,
     245,   188,   227,   151,   152,   167,   173,   228,   229,   230,
     153,   154,   155,   156,   157,   158,   231,   234,   235,   164,
     236,   237,   246,   238,   239,   170,   249,    80,   232,   148,
     149,   150,   206,   186,    82,   148,   149,   150,     0,     0,
       0,   185,   185,    87,    87,     0,    -3,     0,   132,   132,
     153,   154,   155,   156,   157,   158,     0,     4,     5,     0,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
     131,   131,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    79,   159,   160,   161,     0,     0,    79,   159,
     160,   161,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,     0,    77,     0,     0,     0,
      78,     0,     4,     5,     0,    79,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,     0,     0,     0,
     123,   124,   125,   126,   127,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,   151,
     152,    77,     0,     0,     0,    78,     0,     4,     5,     0,
      79,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,     0,     0,     0,     0,   154,   155,   156,
     157,   158,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,     0,    77,     0,     0,     0,
      78,     0,     0,     0,     0,    79,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,     0,    25,     0,     0,     0,
       0,   148,   149,   150,     0,    34,     0,     0,     0,   151,
     152,     0,     0,     0,     0,     0,    35,    36,    37,    38,
      39,     0,     0,     0,     0,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,   133,
     134,   135,   136,   137,     0,   138,   139,     0,     0,   140,
     141,     0,     0,    72,     0,     0,   153,   154,   155,   156,
     157,   158,     0,     0,    78,   159,   160,   161,     0,    79,
     142,   143
};

static const yytype_int16 yycheck[] =
{
       2,    84,     0,    36,     0,    81,    40,    41,     2,   165,
       2,   131,    36,     8,    82,     0,    49,    50,    51,    52,
      53,    98,    36,    98,   200,    49,   171,    98,    99,    98,
      99,   108,    36,   108,   117,   118,    36,   108,    40,    41,
      49,   117,   118,   219,   112,   101,   102,   223,    82,    36,
      84,    98,    99,    36,   110,   111,   112,    36,    78,    49,
       5,   108,     7,    49,     9,    10,   186,   100,    98,    99,
      54,    55,    98,    99,   113,   109,   109,   108,   112,    81,
      82,   108,    84,   117,   118,   241,    36,    81,    82,    81,
      82,   106,   112,    95,    96,   109,   130,   131,   243,    36,
      98,    99,    98,    99,   105,   106,    36,   109,    36,   185,
     108,   106,   108,    54,    55,   117,   118,   113,   186,   103,
     104,   105,   106,   117,   118,   117,   118,   147,   130,   131,
      36,   151,   152,   153,   154,   155,   156,   157,   158,    95,
      96,    36,   162,   163,   103,   104,   105,   106,    36,   232,
     233,   185,   186,    54,    55,    95,    96,    51,    36,    36,
     101,   102,   103,   104,   105,   106,   108,     4,     4,    36,
       3,     3,    36,   114,   115,    36,   114,     2,   200,    46,
      47,    48,   144,   185,   186,    46,    47,    48,    -1,    -1,
      -1,   185,   186,   185,   186,    -1,     0,    -1,   232,   233,
     101,   102,   103,   104,   105,   106,    -1,    11,    12,    -1,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     232,   233,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,   109,   110,   111,   112,    -1,    -1,   109,   110,
     111,   112,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,    -1,    -1,    -1,
     104,    -1,    11,    12,    -1,   109,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    54,
      55,   100,    -1,    -1,    -1,   104,    -1,    11,    12,    -1,
     109,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,
     105,   106,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,   109,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    46,    47,    48,    -1,    45,    -1,    -1,    -1,    54,
      55,    -1,    -1,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     3,
       4,     5,     6,     7,    -1,     9,    10,    -1,    -1,    13,
      14,    -1,    -1,    93,    -1,    -1,   101,   102,   103,   104,
     105,   106,    -1,    -1,   104,   110,   111,   112,    -1,   109,
      34,    35
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   117,   118,     0,    11,    12,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,   100,   104,   109,
     119,   124,   125,   128,   129,   130,   131,   135,   136,   137,
     142,   143,   145,   146,   147,   148,   152,    36,    36,    36,
      49,    49,    36,    36,    36,    49,   138,    49,   139,   125,
     145,   145,   125,   135,   142,    98,    99,   120,   121,   128,
     119,   143,   145,    49,    50,    51,    52,    53,   122,   123,
     124,   125,   145,     3,     4,     5,     6,     7,     9,    10,
      13,    14,    34,    35,   132,   133,   134,   113,    46,    47,
      48,    54,    55,   101,   102,   103,   104,   105,   106,   110,
     111,   112,   140,   141,    36,   125,   140,   141,   149,   150,
      36,   125,   140,   141,   153,   154,   101,   102,   110,   111,
     112,   144,   145,   142,   122,   124,   125,   128,   145,   122,
     128,   108,   108,   108,     8,   106,   106,   122,   123,   126,
     127,   145,    98,    99,    98,    99,   133,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   150,   151,
      36,    36,   154,   155,    36,    36,    36,   145,    51,    36,
      36,   108,   120,   121,     4,     4,     3,     3,   114,   115,
     108,   121,   108,   121,   122,   122,    36,   150,   154,   114
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   116,   117,   117,   118,   119,   119,   119,   119,   119,
     120,   121,   122,   122,   122,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   124,   125,   126,   126,   126,   127,
     127,   128,   128,   129,   129,   129,   129,   129,   129,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   131,
     131,   132,   132,   132,   132,   132,   132,   133,   133,   133,
     133,   134,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   137,   137,   137,   137,   137,   137,
     138,   138,   139,   140,   140,   140,   141,   141,   141,   142,
     142,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   144,   144,   144,   145,
     145,   146,   146,   146,   146,   146,   146,   146,   146,   147,
     147,   147,   147,   148,   148,   149,   149,   149,   149,   150,
     151,   151,   152,   152,   152,   152,   153,   153,   153,   153,
     154,   155,   155
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     0,     1,     3,     3,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     2,     1,     1,     1,     3,     3,     1,
       1,     1,     2,     3,     2,     2,     2,     2,     2,     2,
       3,     1,     3,     3,     1,     1,     1,     2,     2,     1,
       0,     1,     1,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     4,     1,     1,     2,     1,     2,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     6,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     1,
       1,     3,     1,     1,     1,     1,     1,     2,     2,     3,
       1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* prog: null expr  */
#line 161 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
{
	finish_parse((yyvsp[0].blk).b);
}
#line 1837 "y.tab.c"
    break;

  case 4: /* null: %empty  */
#line 166 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).q = qerr; }
#line 1843 "y.tab.c"
    break;

  case 6: /* expr: expr and term  */
#line 169 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_and((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 1849 "y.tab.c"
    break;

  case 7: /* expr: expr and id  */
#line 170 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_and((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 1855 "y.tab.c"
    break;

  case 8: /* expr: expr or term  */
#line 171 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_or((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 1861 "y.tab.c"
    break;

  case 9: /* expr: expr or id  */
#line 172 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_or((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 1867 "y.tab.c"
    break;

  case 10: /* and: AND  */
#line 174 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[-1].blk); }
#line 1873 "y.tab.c"
    break;

  case 11: /* or: OR  */
#line 176 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[-1].blk); }
#line 1879 "y.tab.c"
    break;

  case 13: /* id: pnum  */
#line 179 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_ncode(NULL, (bpf_u_int32)(yyvsp[0].i),
						   (yyval.blk).q = (yyvsp[-1].blk).q); }
#line 1886 "y.tab.c"
    break;

  case 14: /* id: paren pid ')'  */
#line 181 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[-1].blk); }
#line 1892 "y.tab.c"
    break;

  case 15: /* nid: ID  */
#line 183 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_scode((yyvsp[0].s), (yyval.blk).q = (yyvsp[-1].blk).q); }
#line 1898 "y.tab.c"
    break;

  case 16: /* nid: HID '/' NUM  */
#line 184 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_mcode((yyvsp[-2].s), NULL, (yyvsp[0].i),
				    (yyval.blk).q = (yyvsp[-3].blk).q); }
#line 1905 "y.tab.c"
    break;

  case 17: /* nid: HID NETMASK HID  */
#line 186 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_mcode((yyvsp[-2].s), (yyvsp[0].s), 0,
				    (yyval.blk).q = (yyvsp[-3].blk).q); }
#line 1912 "y.tab.c"
    break;

  case 18: /* nid: HID  */
#line 188 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                {
				  /* Decide how to parse HID based on proto */
				  (yyval.blk).q = (yyvsp[-1].blk).q;
				  (yyval.blk).b = gen_ncode((yyvsp[0].s), 0, (yyval.blk).q);
				}
#line 1922 "y.tab.c"
    break;

  case 19: /* nid: HID6 '/' NUM  */
#line 193 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                {
#ifdef INET6
				  (yyval.blk).b = gen_mcode6((yyvsp[-2].s), NULL, (yyvsp[0].i),
				    (yyval.blk).q = (yyvsp[-3].blk).q);
#else
				  bpf_error("'ip6addr/prefixlen' not supported "
					"in this configuration");
#endif /*INET6*/
				}
#line 1936 "y.tab.c"
    break;

  case 20: /* nid: HID6  */
#line 202 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                {
#ifdef INET6
				  (yyval.blk).b = gen_mcode6((yyvsp[0].s), 0, 128,
				    (yyval.blk).q = (yyvsp[-1].blk).q);
#else
				  bpf_error("'ip6addr' not supported "
					"in this configuration");
#endif /*INET6*/
				}
#line 1950 "y.tab.c"
    break;

  case 21: /* nid: EID  */
#line 211 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { 
				  (yyval.blk).b = gen_ecode((yyvsp[0].e), (yyval.blk).q = (yyvsp[-1].blk).q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free((yyvsp[0].e));
				}
#line 1964 "y.tab.c"
    break;

  case 22: /* nid: AID  */
#line 220 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                {
				  (yyval.blk).b = gen_acode((yyvsp[0].e), (yyval.blk).q = (yyvsp[-1].blk).q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free((yyvsp[0].e));
				}
#line 1978 "y.tab.c"
    break;

  case 23: /* nid: not id  */
#line 229 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_not((yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 1984 "y.tab.c"
    break;

  case 24: /* not: '!'  */
#line 231 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[-1].blk); }
#line 1990 "y.tab.c"
    break;

  case 25: /* paren: '('  */
#line 233 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[-1].blk); }
#line 1996 "y.tab.c"
    break;

  case 27: /* pid: qid and id  */
#line 236 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_and((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 2002 "y.tab.c"
    break;

  case 28: /* pid: qid or id  */
#line 237 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_or((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 2008 "y.tab.c"
    break;

  case 29: /* qid: pnum  */
#line 239 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_ncode(NULL, (bpf_u_int32)(yyvsp[0].i),
						   (yyval.blk).q = (yyvsp[-1].blk).q); }
#line 2015 "y.tab.c"
    break;

  case 32: /* term: not term  */
#line 244 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { gen_not((yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 2021 "y.tab.c"
    break;

  case 33: /* head: pqual dqual aqual  */
#line 246 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-2].i), (yyvsp[-1].i), (yyvsp[0].i)); }
#line 2027 "y.tab.c"
    break;

  case 34: /* head: pqual dqual  */
#line 247 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-1].i), (yyvsp[0].i), Q_DEFAULT); }
#line 2033 "y.tab.c"
    break;

  case 35: /* head: pqual aqual  */
#line 248 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-1].i), Q_DEFAULT, (yyvsp[0].i)); }
#line 2039 "y.tab.c"
    break;

  case 36: /* head: pqual PROTO  */
#line 249 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-1].i), Q_DEFAULT, Q_PROTO); }
#line 2045 "y.tab.c"
    break;

  case 37: /* head: pqual PROTOCHAIN  */
#line 250 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-1].i), Q_DEFAULT, Q_PROTOCHAIN); }
#line 2051 "y.tab.c"
    break;

  case 38: /* head: pqual ndaqual  */
#line 251 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { QSET((yyval.blk).q, (yyvsp[-1].i), Q_DEFAULT, (yyvsp[0].i)); }
#line 2057 "y.tab.c"
    break;

  case 39: /* rterm: head id  */
#line 253 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk) = (yyvsp[0].blk); }
#line 2063 "y.tab.c"
    break;

  case 40: /* rterm: paren expr ')'  */
#line 254 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = (yyvsp[-1].blk).b; (yyval.blk).q = (yyvsp[-2].blk).q; }
#line 2069 "y.tab.c"
    break;

  case 41: /* rterm: pname  */
#line 255 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_proto_abbrev((yyvsp[0].i)); (yyval.blk).q = qerr; }
#line 2075 "y.tab.c"
    break;

  case 42: /* rterm: arth relop arth  */
#line 256 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_relation((yyvsp[-1].i), (yyvsp[-2].a), (yyvsp[0].a), 0);
				  (yyval.blk).q = qerr; }
#line 2082 "y.tab.c"
    break;

  case 43: /* rterm: arth irelop arth  */
#line 258 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_relation((yyvsp[-1].i), (yyvsp[-2].a), (yyvsp[0].a), 1);
				  (yyval.blk).q = qerr; }
#line 2089 "y.tab.c"
    break;

  case 44: /* rterm: other  */
#line 260 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = (yyvsp[0].rblk); (yyval.blk).q = qerr; }
#line 2095 "y.tab.c"
    break;

  case 45: /* rterm: atmtype  */
#line 261 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_atmtype_abbrev((yyvsp[0].i)); (yyval.blk).q = qerr; }
#line 2101 "y.tab.c"
    break;

  case 46: /* rterm: atmmultitype  */
#line 262 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_atmmulti_abbrev((yyvsp[0].i)); (yyval.blk).q = qerr; }
#line 2107 "y.tab.c"
    break;

  case 47: /* rterm: atmfield atmvalue  */
#line 263 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = (yyvsp[0].blk).b; (yyval.blk).q = qerr; }
#line 2113 "y.tab.c"
    break;

  case 48: /* rterm: mtp3field mtp3value  */
#line 264 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = (yyvsp[0].blk).b; (yyval.blk).q = qerr; }
#line 2119 "y.tab.c"
    break;

  case 50: /* pqual: %empty  */
#line 268 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_DEFAULT; }
#line 2125 "y.tab.c"
    break;

  case 51: /* dqual: SRC  */
#line 271 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_SRC; }
#line 2131 "y.tab.c"
    break;

  case 52: /* dqual: DST  */
#line 272 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_DST; }
#line 2137 "y.tab.c"
    break;

  case 53: /* dqual: SRC OR DST  */
#line 273 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_OR; }
#line 2143 "y.tab.c"
    break;

  case 54: /* dqual: DST OR SRC  */
#line 274 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_OR; }
#line 2149 "y.tab.c"
    break;

  case 55: /* dqual: SRC AND DST  */
#line 275 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_AND; }
#line 2155 "y.tab.c"
    break;

  case 56: /* dqual: DST AND SRC  */
#line 276 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_AND; }
#line 2161 "y.tab.c"
    break;

  case 57: /* aqual: HOST  */
#line 279 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_HOST; }
#line 2167 "y.tab.c"
    break;

  case 58: /* aqual: NET  */
#line 280 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_NET; }
#line 2173 "y.tab.c"
    break;

  case 59: /* aqual: PORT  */
#line 281 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_PORT; }
#line 2179 "y.tab.c"
    break;

  case 60: /* aqual: PORTRANGE  */
#line 282 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_PORTRANGE; }
#line 2185 "y.tab.c"
    break;

  case 61: /* ndaqual: GATEWAY  */
#line 285 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_GATEWAY; }
#line 2191 "y.tab.c"
    break;

  case 62: /* pname: LINK  */
#line 287 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_LINK; }
#line 2197 "y.tab.c"
    break;

  case 63: /* pname: IP  */
#line 288 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_IP; }
#line 2203 "y.tab.c"
    break;

  case 64: /* pname: ARP  */
#line 289 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ARP; }
#line 2209 "y.tab.c"
    break;

  case 65: /* pname: RARP  */
#line 290 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_RARP; }
#line 2215 "y.tab.c"
    break;

  case 66: /* pname: SCTP  */
#line 291 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_SCTP; }
#line 2221 "y.tab.c"
    break;

  case 67: /* pname: TCP  */
#line 292 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_TCP; }
#line 2227 "y.tab.c"
    break;

  case 68: /* pname: UDP  */
#line 293 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_UDP; }
#line 2233 "y.tab.c"
    break;

  case 69: /* pname: ICMP  */
#line 294 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ICMP; }
#line 2239 "y.tab.c"
    break;

  case 70: /* pname: IGMP  */
#line 295 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_IGMP; }
#line 2245 "y.tab.c"
    break;

  case 71: /* pname: IGRP  */
#line 296 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_IGRP; }
#line 2251 "y.tab.c"
    break;

  case 72: /* pname: PIM  */
#line 297 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_PIM; }
#line 2257 "y.tab.c"
    break;

  case 73: /* pname: VRRP  */
#line 298 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_VRRP; }
#line 2263 "y.tab.c"
    break;

  case 74: /* pname: ATALK  */
#line 299 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ATALK; }
#line 2269 "y.tab.c"
    break;

  case 75: /* pname: AARP  */
#line 300 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_AARP; }
#line 2275 "y.tab.c"
    break;

  case 76: /* pname: DECNET  */
#line 301 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_DECNET; }
#line 2281 "y.tab.c"
    break;

  case 77: /* pname: LAT  */
#line 302 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_LAT; }
#line 2287 "y.tab.c"
    break;

  case 78: /* pname: SCA  */
#line 303 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_SCA; }
#line 2293 "y.tab.c"
    break;

  case 79: /* pname: MOPDL  */
#line 304 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_MOPDL; }
#line 2299 "y.tab.c"
    break;

  case 80: /* pname: MOPRC  */
#line 305 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_MOPRC; }
#line 2305 "y.tab.c"
    break;

  case 81: /* pname: IPV6  */
#line 306 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_IPV6; }
#line 2311 "y.tab.c"
    break;

  case 82: /* pname: ICMPV6  */
#line 307 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ICMPV6; }
#line 2317 "y.tab.c"
    break;

  case 83: /* pname: AH  */
#line 308 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_AH; }
#line 2323 "y.tab.c"
    break;

  case 84: /* pname: ESP  */
#line 309 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ESP; }
#line 2329 "y.tab.c"
    break;

  case 85: /* pname: ISO  */
#line 310 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISO; }
#line 2335 "y.tab.c"
    break;

  case 86: /* pname: ESIS  */
#line 311 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ESIS; }
#line 2341 "y.tab.c"
    break;

  case 87: /* pname: ISIS  */
#line 312 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS; }
#line 2347 "y.tab.c"
    break;

  case 88: /* pname: L1  */
#line 313 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_L1; }
#line 2353 "y.tab.c"
    break;

  case 89: /* pname: L2  */
#line 314 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_L2; }
#line 2359 "y.tab.c"
    break;

  case 90: /* pname: IIH  */
#line 315 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_IIH; }
#line 2365 "y.tab.c"
    break;

  case 91: /* pname: LSP  */
#line 316 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_LSP; }
#line 2371 "y.tab.c"
    break;

  case 92: /* pname: SNP  */
#line 317 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_SNP; }
#line 2377 "y.tab.c"
    break;

  case 93: /* pname: PSNP  */
#line 318 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_PSNP; }
#line 2383 "y.tab.c"
    break;

  case 94: /* pname: CSNP  */
#line 319 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_ISIS_CSNP; }
#line 2389 "y.tab.c"
    break;

  case 95: /* pname: CLNP  */
#line 320 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_CLNP; }
#line 2395 "y.tab.c"
    break;

  case 96: /* pname: STP  */
#line 321 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_STP; }
#line 2401 "y.tab.c"
    break;

  case 97: /* pname: IPX  */
#line 322 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_IPX; }
#line 2407 "y.tab.c"
    break;

  case 98: /* pname: NETBEUI  */
#line 323 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_NETBEUI; }
#line 2413 "y.tab.c"
    break;

  case 99: /* pname: RADIO  */
#line 324 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = Q_RADIO; }
#line 2419 "y.tab.c"
    break;

  case 100: /* other: pqual TK_BROADCAST  */
#line 326 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_broadcast((yyvsp[-1].i)); }
#line 2425 "y.tab.c"
    break;

  case 101: /* other: pqual TK_MULTICAST  */
#line 327 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_multicast((yyvsp[-1].i)); }
#line 2431 "y.tab.c"
    break;

  case 102: /* other: LESS NUM  */
#line 328 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_less((yyvsp[0].i)); }
#line 2437 "y.tab.c"
    break;

  case 103: /* other: GREATER NUM  */
#line 329 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_greater((yyvsp[0].i)); }
#line 2443 "y.tab.c"
    break;

  case 104: /* other: CBYTE NUM byteop NUM  */
#line 330 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_byteop((yyvsp[-1].i), (yyvsp[-2].i), (yyvsp[0].i)); }
#line 2449 "y.tab.c"
    break;

  case 105: /* other: INBOUND  */
#line 331 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_inbound(0); }
#line 2455 "y.tab.c"
    break;

  case 106: /* other: OUTBOUND  */
#line 332 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_inbound(1); }
#line 2461 "y.tab.c"
    break;

  case 107: /* other: VLAN pnum  */
#line 333 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_vlan((yyvsp[0].i)); }
#line 2467 "y.tab.c"
    break;

  case 108: /* other: VLAN  */
#line 334 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_vlan(-1); }
#line 2473 "y.tab.c"
    break;

  case 109: /* other: MPLS pnum  */
#line 335 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_mpls((yyvsp[0].i)); }
#line 2479 "y.tab.c"
    break;

  case 110: /* other: MPLS  */
#line 336 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_mpls(-1); }
#line 2485 "y.tab.c"
    break;

  case 111: /* other: PPPOED  */
#line 337 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pppoed(); }
#line 2491 "y.tab.c"
    break;

  case 112: /* other: PPPOES  */
#line 338 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pppoes(); }
#line 2497 "y.tab.c"
    break;

  case 113: /* other: pfvar  */
#line 339 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = (yyvsp[0].rblk); }
#line 2503 "y.tab.c"
    break;

  case 114: /* pfvar: PF_IFNAME ID  */
#line 342 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_ifname((yyvsp[0].s)); }
#line 2509 "y.tab.c"
    break;

  case 115: /* pfvar: PF_RSET ID  */
#line 343 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_ruleset((yyvsp[0].s)); }
#line 2515 "y.tab.c"
    break;

  case 116: /* pfvar: PF_RNR NUM  */
#line 344 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_rnr((yyvsp[0].i)); }
#line 2521 "y.tab.c"
    break;

  case 117: /* pfvar: PF_SRNR NUM  */
#line 345 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_srnr((yyvsp[0].i)); }
#line 2527 "y.tab.c"
    break;

  case 118: /* pfvar: PF_REASON reason  */
#line 346 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_reason((yyvsp[0].i)); }
#line 2533 "y.tab.c"
    break;

  case 119: /* pfvar: PF_ACTION action  */
#line 347 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.rblk) = gen_pf_action((yyvsp[0].i)); }
#line 2539 "y.tab.c"
    break;

  case 120: /* reason: NUM  */
#line 350 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = (yyvsp[0].i); }
#line 2545 "y.tab.c"
    break;

  case 121: /* reason: ID  */
#line 351 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { const char *reasons[] = PFRES_NAMES;
				  int i;
				  for (i = 0; reasons[i]; i++) {
					  if (pcap_strcasecmp((yyvsp[0].s), reasons[i]) == 0) {
						  (yyval.i) = i;
						  break;
					  }
				  }
				  if (reasons[i] == NULL)
					  bpf_error("unknown PF reason");
				}
#line 2561 "y.tab.c"
    break;

  case 122: /* action: ID  */
#line 364 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { if (pcap_strcasecmp((yyvsp[0].s), "pass") == 0 ||
				      pcap_strcasecmp((yyvsp[0].s), "accept") == 0)
					(yyval.i) = PF_PASS;
				  else if (pcap_strcasecmp((yyvsp[0].s), "drop") == 0 ||
				      pcap_strcasecmp((yyvsp[0].s), "block") == 0)
					(yyval.i) = PF_DROP;
				  else
					  bpf_error("unknown PF action");
				}
#line 2575 "y.tab.c"
    break;

  case 123: /* relop: '>'  */
#line 375 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JGT; }
#line 2581 "y.tab.c"
    break;

  case 124: /* relop: GEQ  */
#line 376 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JGE; }
#line 2587 "y.tab.c"
    break;

  case 125: /* relop: '='  */
#line 377 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JEQ; }
#line 2593 "y.tab.c"
    break;

  case 126: /* irelop: LEQ  */
#line 379 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JGT; }
#line 2599 "y.tab.c"
    break;

  case 127: /* irelop: '<'  */
#line 380 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JGE; }
#line 2605 "y.tab.c"
    break;

  case 128: /* irelop: NEQ  */
#line 381 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = BPF_JEQ; }
#line 2611 "y.tab.c"
    break;

  case 129: /* arth: pnum  */
#line 383 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.a) = gen_loadi((yyvsp[0].i)); }
#line 2617 "y.tab.c"
    break;

  case 131: /* narth: pname '[' arth ']'  */
#line 386 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_load((yyvsp[-3].i), (yyvsp[-1].a), 1); }
#line 2623 "y.tab.c"
    break;

  case 132: /* narth: pname '[' arth ':' NUM ']'  */
#line 387 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_load((yyvsp[-5].i), (yyvsp[-3].a), (yyvsp[-1].i)); }
#line 2629 "y.tab.c"
    break;

  case 133: /* narth: arth '+' arth  */
#line 388 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_ADD, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2635 "y.tab.c"
    break;

  case 134: /* narth: arth '-' arth  */
#line 389 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_SUB, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2641 "y.tab.c"
    break;

  case 135: /* narth: arth '*' arth  */
#line 390 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_MUL, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2647 "y.tab.c"
    break;

  case 136: /* narth: arth '/' arth  */
#line 391 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_DIV, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2653 "y.tab.c"
    break;

  case 137: /* narth: arth '&' arth  */
#line 392 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_AND, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2659 "y.tab.c"
    break;

  case 138: /* narth: arth '|' arth  */
#line 393 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_OR, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2665 "y.tab.c"
    break;

  case 139: /* narth: arth LSH arth  */
#line 394 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_LSH, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2671 "y.tab.c"
    break;

  case 140: /* narth: arth RSH arth  */
#line 395 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_arth(BPF_RSH, (yyvsp[-2].a), (yyvsp[0].a)); }
#line 2677 "y.tab.c"
    break;

  case 141: /* narth: '-' arth  */
#line 396 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_neg((yyvsp[0].a)); }
#line 2683 "y.tab.c"
    break;

  case 142: /* narth: paren narth ')'  */
#line 397 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = (yyvsp[-1].a); }
#line 2689 "y.tab.c"
    break;

  case 143: /* narth: LEN  */
#line 398 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { (yyval.a) = gen_loadlen(); }
#line 2695 "y.tab.c"
    break;

  case 144: /* byteop: '&'  */
#line 400 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = '&'; }
#line 2701 "y.tab.c"
    break;

  case 145: /* byteop: '|'  */
#line 401 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = '|'; }
#line 2707 "y.tab.c"
    break;

  case 146: /* byteop: '<'  */
#line 402 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = '<'; }
#line 2713 "y.tab.c"
    break;

  case 147: /* byteop: '>'  */
#line 403 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = '>'; }
#line 2719 "y.tab.c"
    break;

  case 148: /* byteop: '='  */
#line 404 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = '='; }
#line 2725 "y.tab.c"
    break;

  case 150: /* pnum: paren pnum ')'  */
#line 407 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = (yyvsp[-1].i); }
#line 2731 "y.tab.c"
    break;

  case 151: /* atmtype: LANE  */
#line 409 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_LANE; }
#line 2737 "y.tab.c"
    break;

  case 152: /* atmtype: LLC  */
#line 410 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_LLC; }
#line 2743 "y.tab.c"
    break;

  case 153: /* atmtype: METAC  */
#line 411 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_METAC;	}
#line 2749 "y.tab.c"
    break;

  case 154: /* atmtype: BCC  */
#line 412 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_BCC; }
#line 2755 "y.tab.c"
    break;

  case 155: /* atmtype: OAMF4EC  */
#line 413 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_OAMF4EC; }
#line 2761 "y.tab.c"
    break;

  case 156: /* atmtype: OAMF4SC  */
#line 414 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_OAMF4SC; }
#line 2767 "y.tab.c"
    break;

  case 157: /* atmtype: SC  */
#line 415 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_SC; }
#line 2773 "y.tab.c"
    break;

  case 158: /* atmtype: ILMIC  */
#line 416 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_ILMIC; }
#line 2779 "y.tab.c"
    break;

  case 159: /* atmmultitype: OAM  */
#line 418 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_OAM; }
#line 2785 "y.tab.c"
    break;

  case 160: /* atmmultitype: OAMF4  */
#line 419 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_OAMF4; }
#line 2791 "y.tab.c"
    break;

  case 161: /* atmmultitype: CONNECTMSG  */
#line 420 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_CONNECTMSG; }
#line 2797 "y.tab.c"
    break;

  case 162: /* atmmultitype: METACONNECT  */
#line 421 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.i) = A_METACONNECT; }
#line 2803 "y.tab.c"
    break;

  case 163: /* atmfield: VPI  */
#line 424 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).atmfieldtype = A_VPI; }
#line 2809 "y.tab.c"
    break;

  case 164: /* atmfield: VCI  */
#line 425 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).atmfieldtype = A_VCI; }
#line 2815 "y.tab.c"
    break;

  case 166: /* atmvalue: relop NUM  */
#line 428 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_atmfield_code((yyvsp[-2].blk).atmfieldtype, (bpf_int32)(yyvsp[0].i), (bpf_u_int32)(yyvsp[-1].i), 0); }
#line 2821 "y.tab.c"
    break;

  case 167: /* atmvalue: irelop NUM  */
#line 429 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_atmfield_code((yyvsp[-2].blk).atmfieldtype, (bpf_int32)(yyvsp[0].i), (bpf_u_int32)(yyvsp[-1].i), 1); }
#line 2827 "y.tab.c"
    break;

  case 168: /* atmvalue: paren atmlistvalue ')'  */
#line 430 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                 { (yyval.blk).b = (yyvsp[-1].blk).b; (yyval.blk).q = qerr; }
#line 2833 "y.tab.c"
    break;

  case 169: /* atmfieldvalue: NUM  */
#line 432 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                   {
	(yyval.blk).atmfieldtype = (yyvsp[-1].blk).atmfieldtype;
	if ((yyval.blk).atmfieldtype == A_VPI ||
	    (yyval.blk).atmfieldtype == A_VCI)
		(yyval.blk).b = gen_atmfield_code((yyval.blk).atmfieldtype, (bpf_int32) (yyvsp[0].i), BPF_JEQ, 0);
	}
#line 2844 "y.tab.c"
    break;

  case 171: /* atmlistvalue: atmlistvalue or atmfieldvalue  */
#line 440 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                        { gen_or((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 2850 "y.tab.c"
    break;

  case 172: /* mtp3field: SIO  */
#line 443 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).mtp3fieldtype = M_SIO; }
#line 2856 "y.tab.c"
    break;

  case 173: /* mtp3field: OPC  */
#line 444 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).mtp3fieldtype = M_OPC; }
#line 2862 "y.tab.c"
    break;

  case 174: /* mtp3field: DPC  */
#line 445 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).mtp3fieldtype = M_DPC; }
#line 2868 "y.tab.c"
    break;

  case 175: /* mtp3field: SLS  */
#line 446 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).mtp3fieldtype = M_SLS; }
#line 2874 "y.tab.c"
    break;

  case 177: /* mtp3value: relop NUM  */
#line 449 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_mtp3field_code((yyvsp[-2].blk).mtp3fieldtype, (u_int)(yyvsp[0].i), (u_int)(yyvsp[-1].i), 0); }
#line 2880 "y.tab.c"
    break;

  case 178: /* mtp3value: irelop NUM  */
#line 450 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                { (yyval.blk).b = gen_mtp3field_code((yyvsp[-2].blk).mtp3fieldtype, (u_int)(yyvsp[0].i), (u_int)(yyvsp[-1].i), 1); }
#line 2886 "y.tab.c"
    break;

  case 179: /* mtp3value: paren mtp3listvalue ')'  */
#line 451 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                  { (yyval.blk).b = (yyvsp[-1].blk).b; (yyval.blk).q = qerr; }
#line 2892 "y.tab.c"
    break;

  case 180: /* mtp3fieldvalue: NUM  */
#line 453 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                    {
	(yyval.blk).mtp3fieldtype = (yyvsp[-1].blk).mtp3fieldtype;
	if ((yyval.blk).mtp3fieldtype == M_SIO ||
	    (yyval.blk).mtp3fieldtype == M_OPC ||
	    (yyval.blk).mtp3fieldtype == M_DPC ||
	    (yyval.blk).mtp3fieldtype == M_SLS )
		(yyval.blk).b = gen_mtp3field_code((yyval.blk).mtp3fieldtype, (u_int) (yyvsp[0].i), BPF_JEQ, 0);
	}
#line 2905 "y.tab.c"
    break;

  case 182: /* mtp3listvalue: mtp3listvalue or mtp3fieldvalue  */
#line 463 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"
                                          { gen_or((yyvsp[-2].blk).b, (yyvsp[0].blk).b); (yyval.blk) = (yyvsp[0].blk); }
#line 2911 "y.tab.c"
    break;


#line 2915 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 465 "/home/xiaoxi/rjsupplicant_research/pcap/grammar.y"

