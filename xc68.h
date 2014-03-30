/* UQLX */

/* various defintions for constants defined by c68 */
/* prefixed by  'C68_' or C68ERR_ */

/* many of these have to bve translated :-(( */

/* netdb.h */
#define C68ERR_HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
#define C68ERR_TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define C68ERR_NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define C68ERR_NO_DATA         4 /* Valid name, no data record of requested type */
#define C68ERR_NO_ADDRESS      NO_DATA         /* no address, look for MX record */

#define C68ERR_ENOTSOCK        40      /* Socket operation on non-socket */
#define C68ERR_EDESTADDRREQ    41      /* Destination address required */
#define C68ERR_EMSGSIZE        42      /* Message too long */
#define C68ERR_EPROTOTYPE      43      /* Protocol wrong type for socket */
#define C68ERR_ENOPROTOOPT     44      /* Protocol not available */
#define C68ERR_EPROTONOSUPPORT 45      /* Protocol not supported */
#define C68ERR_ESOCKTNOSUPPORT 46      /* Socket type not supported */
#define C68ERR_EOPNOTSUPP      47      /* Operation not supported on transport endpoint */
#define C68ERR_EPFNOSUPPORT    48      /* Protocol family not supported */
#define C68ERR_EAFNOSUPPORT    49      /* Address family not supported by protocol */
#define C68ERR_EADDRINUSE      50      /* Address already in use */
#define C68ERR_EADDRNOTAVAIL   51      /* Cannot assign requested address */
#define C68ERR_ENETDOWN        52      /* Network is down */
#define C68ERR_ENETUNREACH     53      /* Network is unreachable */
#define C68ERR_ENETRESET       54      /* Network dropped connection because of reset */
#define C68ERR_ECONNABORTED    55      /* Software caused connection abort */
#define C68ERR_ECONNRESET      56      /* Connection reset by peer */
#define C68ERR_ENOBUFS         57      /* No buffer space available */
#define C68ERR_EISCONN         58      /* Transport endpoint is already connected */
#define C68ERR_ENOTCONN        59      /* Transport endpoint is not connected */
#define C68ERR_ESHUTDOWN       60      /* Cannot send after transport endpoint shutdown */
#define C68ERR_ETOOMANYREFS    61      /* Too many references: cannot splice */
#define C68ERR_ETIMEDOUT       62      /* Connection timed out */
#define C68ERR_ECONNREFUSED    63      /* Connection refused */
#define C68ERR_EHOSTDOWN       64      /* Host is down */
#define C68ERR_EHOSTUNREACH    65      /* No route to host */
#define C68ERR_EALREADY        66      /* Operation already in progress */
#define C68ERR_EINPROGRESS     67      /* Operation now in progress */

/* netinet definitions */
#define C68_IPPROTO_IP              0               /* dummy for IP */
#define C68_IPPROTO_ICMP            1               /* control message protocol */
#define C68_IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define C68_IPPROTO_TCP             6               /* tcp */
#define C68_IPPROTO_EGP             8               /* exterior gateway protocol */
#define C68_IPPROTO_PUP             12              /* pup */
#define C68_IPPROTO_UDP             17              /* user datagram protocol */
#define C68_IPPROTO_IDP             22              /* xns idp */
#define C68_IPPROTO_TP              29              /* tp-4 w/ class negotiation */
#define C68_IPPROTO_EON             80              /* ISO cnlp */
#define C68_IPPROTO_RAW             255             /* raw IP packet */
#define C68_IPPROTO_MAX             256
#define C68_IPPORT_RESERVED         1024
#define C68_IPPORT_USERRESERVED     5000

#if 0
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128
#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536
#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff
#define IN_CLASSD(i)            (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(i)         IN_CLASSD(i)
#define IN_EXPERIMENTAL(i)      (((long)(i) & 0xe0000000) == 0xe0000000)
#define IN_BADCLASS(i)          (((long)(i) & 0xf0000000) == 0xf0000000)
#define INADDR_ANY              (u_long)0x00000000
#define INADDR_BROADCAST        (u_long)0xffffffff      /* must be masked */
#define INADDR_NONE             0xffffffff              /* -1 return */
#define IN_LOOPBACKNET          127                     /* official! */
#endif

#define C68_IP_OPTIONS      1       /* buf/ip_opts; set/get IP per-packet options */
#define C68_IP_HDRINCL      2       /* int; header is included with data (raw) */
#define C68_IP_TOS          3       /* int; IP type of service and precedence */
#define C68_IP_TTL          4       /* int; IP time to live */
#define C68_IP_RECVOPTS     5       /* bool; receive all IP options w/datagram */
#define C68_IP_RECVRETOPTS  6       /* bool; receive IP options for response */
#define C68_IP_RECVDSTADDR  7       /* bool; receive IP dst addr w/datagram */
#define C68_IP_RETOPTS      8       /* ip_opts; set/get IP per-packet options */


#define C68_IPOPT_EOL               0               /* end of option list */
#define C68_IPOPT_NOP               1               /* no operation */
#define C68_IPOPT_RR                7               /* record packet route */
#define C68_IPOPT_TS                68              /* timestamp */
#define C68_IPOPT_SECURITY          130             /* provide s,c,h,tcc */
#define C68_IPOPT_LSRR              131             /* loose source route */
#define C68_IPOPT_SATID             136             /* satnet id */
#define C68_IPOPT_SSRR              137             /* strict source route */
#define C68_IPOPT_OPTVAL            0               /* option ID */
#define C68_IPOPT_OLEN              1               /* option length */
#define C68_IPOPT_OFFSET            2               /* offset within option */
#define C68_IPOPT_MINOFF            4               /* min value of above */
#define C68_IPOPT_TS_TSONLY         0               /* timestamps only */
#define C68_IPOPT_TS_TSANDADDR      1               /* timestamps and addresses */
#define C68_IPOPT_TS_PRESPEC        3               /* specified modules only */
#define C68_IPOPT_SECUR_UNCLASS     0x0000
#define C68_IPOPT_SECUR_CONFID      0xf135
#define C68_IPOPT_SECUR_EFTO        0x789a
#define C68_IPOPT_SECUR_MMMM        0xbc4d
#define C68_IPOPT_SECUR_RESTR       0xaf13
#define C68_IPOPT_SECUR_SECRET      0xd788
#define C68_IPOPT_SECUR_TOPSECRET   0x6bc5



#define C68_TH_FIN  0x01
#define C68_TH_SYN  0x02
#define C68_TH_RST  0x04
#define C68_TH_PUSH 0x08
#define C68_TH_ACK  0x10
#define C68_TH_URG  0x20



/* sys definitions */



/**/
#define C68_SOCK_STREAM     1               /* stream socket */
#define C68_SOCK_DGRAM      2               /* datagram socket */
#define C68_SOCK_RAW        3               /* raw-protocol interface */
#define C68_SOCK_RDM        4               /* reliably-delivered message */
#define C68_SOCK_SEQPACKET  5               /* sequenced packet stream */

/**/
#define C68_SO_DEBUG      1
#define C68_SO_REUSEADDR  2
#define C68_SO_TYPE               3
#define C68_SO_ERROR      4
#define C68_SO_DONTROUTE  5
#define C68_SO_BROADCAST  6
#define C68_SO_SNDBUF     7
#define C68_SO_RCVBUF     8
#define C68_SO_KEEPALIVE  9
#define C68_SO_OOBINLINE  10
#define C68_SO_NO_CHECK   11
#define C68_SO_PRIORITY   12
#define C68_SO_LINGER     13
#define C68_SO_BSDCOMPAT  14
/* To add :#define SO_REUSEPORT 15 */
#define C68_SO_BINDTODEVICE 25
#define C68_SOL_SOCKET      0x1     /* options for socket level */

#define C68_AF_UNSPEC       0               /* unspecified */
#define C68_AF_UNIX         1               /* local to host (pipes, portals) */
#define C68_AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define C68_AF_IMPLINK      3               /* arpanet imp addresses */
#define C68_AF_PUP          4               /* pup protocols: e.g. BSP */
#define C68_AF_CHAOS        5               /* mit CHAOS protocols */
#define C68_AF_NS           6               /* XEROX NS protocols */
#define C68_AF_ISO          7               /* ISO protocols */
#define C68_AF_OSI          C68_AF_ISO
#define C68_AF_ECMA         8               /* european computer manufacturers */
#define C68_AF_DATAKIT      9               /* datakit protocols */
#define C68_AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define C68_AF_SNA          11              /* IBM SNA */
#define C68_AF_DECnet       12              /* DECnet */
#define C68_AF_DLI          13              /* DEC Direct data link interface */
#define C68_AF_LAT          14              /* LAT */
#define C68_AF_HYLINK       15              /* NSC Hyperchannel */
#define C68_AF_APPLETALK    16              /* Apple Talk */
#define C68_AF_ROUTE        17              /* Internal Routing Protocol */
#define C68_AF_LINK         18              /* Link layer interface */
#define C68_pseudo_AF_XTP   19              /* eXpress Transfer Protocol (no AF) */
#define C68_AF_MAX          20


/*#define C68_SOMAXCONN       5*/
#define C68_MSG_OOB         0x1             /* process out-of-band data */
#define C68_MSG_PEEK        0x2             /* peek at incoming message */
#define C68_MSG_DONTROUTE   0x4             /* send without using routing tables */
#define C68_MSG_EOR         0x8             /* data completes record */
#define C68_MSG_TRUNC       0x10            /* data discarded before delivery */
#define C68_MSG_CTRUNC      0x20            /* control data lost before delivery */
#define C68_MSG_WAITALL     0x40            /* wait for full request or error */


