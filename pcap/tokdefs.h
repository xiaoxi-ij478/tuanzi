/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 283 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE pcap_lval;


int pcap_parse (void);


#endif /* !YY_PCAP_Y_TAB_H_INCLUDED  */
