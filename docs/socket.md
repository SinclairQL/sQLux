QDOS TCP/IP and socket functionality
====================================

 - [Introduction](#1-Introduction)
 - [TCP/IP device driver interface](#2-tcpip-device-driver-interface)
 - [Socket Library](#3-Socket-Library)

---

1 Introduction
--------------

This document implements TCP/IP as implemented in UQLX. The
implementation is due to Jonathan Hudson and is free, the hope is that
native QDOS implementations can be kept compatible with it.

The characteristics of the implementation:

-   TCP/IP interface as device drivers
-   most of TCP functionality useable from SBasic. Full functionality
    with SBasic and some available toolkits.
-   implementation of BSD compatible socket library for c68 available

The general design of the interface is chosen so that features more to
be used from Assembler/Basic follow QDOS interfacing conventions, those
used from C/unix like applications follow conventions that make it
easier to interface for such programs.

---

2 TCP/IP device driver interface
--------------------------------

- [open](#21-open):
- [IO operation](#22-IO-operation):

---

### 2.1 open

Following new devices are available for the `trap#2,open` call:

`SCK_`

generic socket, can be used for accepting connections or netdb access.

`TCP_host:port`\
`UDP_host:port`

TCP or UDP protocol sockets. Both parameters optional

`host` and `port` can be both given either by numerical value or name.
Eg `"129.69.1.59:119"` or `"news.uni-stuttgart.de:nntp"`

Supported `keys` in `D3` are:

`D3=0`

creates a socket of requested type/protocol.

`D3=1`

`TCP` and `UDP`. `host` and `port` must be specified. Opens a
connection `TCP`, or sets peer address for `UDP` sockets

returns without error if connection can't be completed within
1-2/50s, internally the connection buildup continues. Every i/o
operation will be blocked until the connection succeeds or fails.

`D3=2`

bind `TCP` or `UDP` socket to an address. Such sockets can be used
for accepting incoming connections

`D3=channel_id`

`SCK` only. accept connection for socket specified by `channel_id`

returns error if can't complete immediately.

---

### 2.2 IO operation

Many operations typically not regarded as IO were provided by `trap#3`
calls to gain flexibility.

Basic IO operations (`D0=0..7`) are defined for connected TCP sockets.
They may work for UDP sockets when peer address is set, however this use
is strongly discouraged. Trap\#3,\[\$48,\$49\] also work but it is not
clear whether they are meaningful and thus may not be supported.

Generally, TCP/IP aware software should probably use the socket specific
IO functions - send,recv,sendto,recvfrom.

When a trap#3 returns with an error, an additional c68 conforming error
code may be queried by IP_ERRNO, IP_H_ERRNO and IP_H_STRERROR
operations. This code is valid unless -1.

---

#### 2.2.1 Basic IO operations

These are compatible to QDOS. The only questionable issue here is
whether io.fstrg should always fill its buffer before returning as it
does now, or rather mimic the behaviour of recv/recvfrom. Since the
number of received characters will be in `D1` anyway, this should not
disturb any QDOS applications.

---

#### 2.2.2 Constants and Datatypes

Here only the most important constants that are defined, the rest is in
socket library header files.

```
EQU IP_LISTEN           $50
EQU IP_SEND             $51
EQU IP_SENDTO           $52
EQU IP_RECV             $53
EQU IP_RECVFM           $54
EQU IP_GETOPT           $55
EQU IP_SETOPT           $56
EQU IP_SHUTDWN          $57
EQU IP_BIND             $58
EQU IP_CONNECT          $59
EQU IP_FCNTL            $5a

EQU IP_GETHOSTNAME      $5b
EQU IP_GETSOCKNAME      $5c
EQU IP_GETPEERNAME      $5d

EQU IP_GETHOSTBYNAME    $5e
EQU IP_GETHOSTBYADDR    $5f
EQU IP_SETHOSTENT       $60
EQU IP_ENDHOSTENT       $61
EQU IP_H_ERRNO          $62

EQU IP_GETSERVENT       $63
EQU IP_GETSERVBYNAME    $64
EQU IP_GETSERVBYPORT    $65
EQU IP_SETSERVENT       $66
EQU IP_ENDSERVENT       $67

EQU IP_GETNETENT        $68
EQU IP_GETNETBYNAME     $69
EQU IP_GETNETBYADDR     $6a
EQU IP_SETNETENT        $6b
EQU IP_ENDNETENT        $6c

EQU IP_GETPROTOENT      $6d
EQU IP_GETPROTOBYNAME   $6e
EQU IP_GETPROTOBYNUMBER $6f
EQU IP_SETPROTOENT      $70
EQU IP_ENDPROTOENT      $71

EQU IP_INET_ATON        $72
EQU IP_INET_ADDR        $73
EQU IP_INET_NETWORK     $74
EQU IP_INET_NTOA        $75
EQU IP_INET_MAKEADDR    $76
EQU IP_INET_LNAOF       $77
EQU IP_INET_NETOF       $78

EQU IP_IOCTL            $79
EQU IP_GETDOMAIN        $7a
EQU IP_H_STRERROR       $7b
```

Following constants and datatypes are a mix from AmiTCP/IP and Linux
definitions. Not every of them is meaningful or supported on every
implementation.

Next some definitions useful for socket(), bind() and connect() calls
and their trap#2/#3 equivalents.

```
#define SOCK_STREAM 1       /* stream socket */
#define SOCK_DGRAM  2       /* datagram socket */
#define SOCK_RAW    3       /* raw-protocol interface */
#define SOCK_RDM    4       /* reliably-delivered message */
#define SOCK_SEQPACKET  5       /* sequenced packet stream */

#define AF_UNSPEC   0       /* unspecified address family */
#define AF_INET     2       /* internet: UDP, TCP, etc. */
#define PF_UNSPEC   AF_UNSPEC       /* aliases */
#define PF_INET     AF_INET


struct sockaddr {
    u_char  sa_len;         /* total length */
    u_char  sa_family;      /* address family */
    char    sa_data[14];        /* actually longer; address value */
};

struct sockproto {
    u_short sp_family;      /* address family */
    u_short sp_protocol;        /* protocol */
};


/* constants for getsockopt()/setsockopt() :*/

#define SOL_SOCKET  0x1 /* options for socket level */
/* other values conforming IP may be possible */

#define SO_DEBUG      1
#define SO_REUSEADDR  2
#define SO_TYPE       3
#define SO_ERROR      4
#define SO_DONTROUTE  5
#define SO_BROADCAST  6
#define SO_SNDBUF     7
#define SO_RCVBUF     8
#define SO_KEEPALIVE  9
#define SO_OOBINLINE  10
#define SO_NO_CHECK   11
#define SO_PRIORITY   12
#define SO_LINGER     13        /* ignored, doesn't seem practicable in QDOS */
#define SO_BSDCOMPAT  14
```

Next, some netdb definitions. This seems like a nightmare for assembler
programmers..

```

struct  hostent {
    char    *h_name;    /* official name of host */
    char    **h_aliases;    /* alias list */
    int h_addrtype; /* host address type */
    int h_length;   /* length of address */
    char    **h_addr_list;  /* list of addresses from name server */
#define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
};

struct  netent {
    char        *n_name;    /* official name of net */
    char        **n_aliases;    /* alias list */
    int     n_addrtype; /* net address type */
    unsigned long   n_net;      /* network # */
};

struct  servent {
    char    *s_name;    /* official service name */
    char    **s_aliases;    /* alias list */
    int s_port;     /* port number */
    char    *s_proto;   /* protocol to use */
};

struct  protoent {
    char    *p_name;    /* official protocol name */
    char    **p_aliases;    /* alias list */
    int p_proto;    /* protocol # */
};

```

---

#### 2.2.3 Socket control and management

```

IP_LISTEN

Provides listen(2) functionality.

For a socket that has been bound during open or explicitly with
bind() this will set the number of connect requests that are queued
for accept(). Additional requests will not be handled and clients
receive a protocol specific error or retry will be initiated.

    Input

    D0 = IP_LISTEN
    D1 = (int) backlog
    D3 = (short) timeout (should be -1)
    A0 = (chanid_t) channel ID

    Output

    D0 = result (0 if OK)

IP_BIND

Provides bind(2) functionality

    Input

    D0 = IP_BIND
    D1 = (int) namelen
    D3 = (short) timeout (-1)

    A0 = (chanid_t) Channel ID
    A2 = (struct  sockaddr  *) name;

    Output

    D0 = result


IP_CONNECT

Provides connect(2) functionality

  TCP: opens connection to host:prot
  UDP: (re)sets peer address to host:port

    Input

    D0 = IP_CONNECT
    D1 = (int) namelen
    D3 = (short) timeout (-1)

    A0 = (chanid_t) Channel ID
    A2 = (struct  sockaddr  *) name;

    Output

    D0 = result

  TCP: opens connection
  UDP: (re)sets peer address

  regardless of the timeout specified, the socket will remain
  blocked (any IO will timeout or be delayed) until the connection
  buildup succeeded or failed.


IP_FCNTL

Provides fcntl(2) functionality for IPDEV sockets only.

An awful hack for now.. don't use it unless you have to.

    Input

    D0 = IP_FCNTL
    D1 = (int) cmd;
    D2 = (int) arg;
    D3 = (short) timeout -1;

    A0 = (chanid_t) channel ID

    Output

    D0 = result


IP_GETOPT

Provides (some) getsockopt functionality

    Input

    D0 = IP_GETOPT
    D1 = (int) optlen
    D2 = (int) level
    D3 = (short) timeout (-1)

    A0 = (chanid_t) channel ID
    A1 = (void *) optval address
    A2 = (int) optname

    Output

    D0 = result
    D1 = optlen

IP_SETOPT

Provides (some) setsockopt functionality

    Input

    D0 = IP_SETOPT
    D1 = (int) optlen
    D2 = (int) level
    D3 = (short) timeout (-1)

    A0 = (chanid_t) channel ID
    A1 = (void *) optval address
    A2 = (int) optname

    Output

    D0 = result


IP_SHUTDWN

Provides shutdown(2) functionality

    Input

    D0 = IP_SHUTDWN
    D1 = (int) how          # how=0 disable receive ,1 send,
                                #     2 send&receive
    D3 = (short) timeout (-1);

    A0 = (chanid_t) Channel ID

    Output

    D0 = result

```

---

#### 2.2.4 Socket specific IO

send and recv differ from io.sstrg and io.fstrg in that they message
oriented and allow chunks longer than 32k.

recv and recvfrom return immediately when data is available, or after
the first message arrives.

send and recv can be (unlike sendto, recvfrom for UDP) applied only to
sockets that have been connected previously.

```
IP_SEND

Provides send(2) functionality

    Input

    D0 = IP_SEND;
    D1 = (uint) flag;
    D2 = (int) buffer size;
    D3 = (short) timeout (should be -1)

    A0 = (chanid_t) channel ID
    A1 = (void *) buffer address

    Output

    D0 = result
    D1 = (int) bytes written

    A1 = buffer address + bytes written

IP_SENDTO

Provides sendto(2) functionality

    Input

    D0 = IP_SENDTO;
    D1 = (uint) flag;
    D2 = (int) buffer size;
    D3 = (short) timeout (should be -1)

    A0 = (chanid_t) channel ID
    A1 = void *) buffer address
    A2 = parameter block (2 long words)
        params[0] = (struct sockaddr*) to
        params[1] = (int) tolen;


    Output

    D0 = result
        +ve => number of bytes sent
        -ve => error code


IP_RECV

Provides recv(2) functionality

    Input

    D0 = IP_RECV
    D1 = (uint) flag
    D2 = (int) buffer size
    D3 = (short) timeout (should be -1)

    A0 = (chanid_t) channel ID
    A1 = (void *) buffer address

    Output

    D0 = result code
    D1 = bytes written


IP_RECVFM

Provides recvfrom(2) functionality

    D0 = IP_RECVFM
    D1 = (uint) flag
    D2 = (int) buffer size
    D3 = (short) timeout (-1)

    A0 = (chanid_t) channel ID
    A1 = (void *) buffer address
    A2 = parameter block (2 long words)
        params[0] = (struct sockaddr*) from
        params[1] = (int) fromlen;

    Output

    D0 = result
        +ve => number of bytes sent
        -ve => error code
    D1 = size of returned struct sockaddr
```

---

#### 2.2.5 Netdb functions

```
IP_GETHOSTNAME

Provides gethostname(2) functionality

    Input

    D0 = IP_GETHOSTNAME;
    D2 = (int) namebufferlen
    D3 = (short) timeout (-1)

    A0 = (chanid_t) channel ID
    A1 = (char *)namebuffer;

    Output

    D0 = result


IP_GETSOCKNAME

Provides getsockname(2) functionality


    Input

    D0 = IP_GETSOCKNAME
    D2 = (int) namelen
    D3 = (short) timeout (-1);

    A0 = (chanid_t) channel ID
    A1 = (struct sockaddr *) name

    Output

    D0 = result
    D1 = namelen



IP_GETPEERNAME

Provides getpeername(2) functionality

    Input

    D0 = IP_GETPEERNAME
    D2 = (int) addrlen
    D3 = (short) timeout (-1);

    A0 = (chanid_t) channel ID
    A1 = (struct sockaddr *) addr

    Output

    D0 = result
    D1 = addrlen



IP_GETHOSTBYNAME

Provides gethostbyname(2) functionality

    Input

    D0 = IP_GETHOSTBYNAME
    D3 = (short) timeout (-1)

    A0 = (chanid_t *) channel ID
        A1 = (char *) name          // NULL terminated
        A2 = (struct hostent *)hostent buffer   // minimum of 500 bytes

The buffer pointed to by A2 must be at large enough to hold the
largest struct hostent returned.

    D0 = result


IP_GETHOSTBYADDR

Provides gethostbyaddr(2) functionality

    Input

    D0 = IP_GETHOSTBYNAME
        D1 = (int) addrlen;
        D2 = (int) type;
    D3 = (short) timeout (-1)

    A0 = (chanid_t *) channel ID
        A1 = (char *) addr
        A2 = (struct hostent *)hostent buffer   // minimum of 500 bytes

The buffer pointed to by A2 must be at large enough to hold the
largest struct hostent returned.

    D0 = result



IP_SETHOSTENT
IP_SETSERVENT
IP_SETNETENT
IP_SETPROTOENT

Provides set*ent(2) functionality

    Input

    D0 = IP_SET*ENT
        D1 = (int) stayopen;
    D3 = (short) timeout (-1)
    A0 = (chanid_t *) channel ID

    Output

    D0 = result


IP_ENDHOSTENT
IP_ENDSERVENT
IP_ENDNETENT
IP_ENDPROTOENT

Provides end*ent(2) functionality

    Input

    D0 = IP_END*ENT
    D3 = (short) timeout (-1)
    A0 = (chanid_t *) channel ID

    Output

    D0 = result



IP_GETSERVBYNAME
IP_GETSERVBYPORT
IP_GETSERVENT

Provides get*ent(2) functionality

    Input

    D0 = IP_GET*ENT
    D3 = (short) timeout (-1)
    A0 = (chanid_t *) channel ID
    A2 = (void *) buffer        // cast as necessary

    Output

    D0 = result


IP_GETNETBYNAME

Provides getnetbyname(2) functionality

    Input

    D0 = IP_GETNETBYNAME
    D3 = (short) timeout (-1)
        A0 = (chanid_t)channel ID
        A1 = (char *)name
    A2 = (struct netent *)netent buffer

    Output

    D0 = result


IP_GETNETBYADDR

Provides getnetbyname(2) functionality

    Input

        D0 = IP_GETNETBYADDR;
        A0 = (chanid_t) channel ID
        A2 = (struct netent *)netent buffer
        D1 = (uint) net
        D2 = (int) type
        D3 = (short) timeout (-1)

    Output

    D0 = result


IP_GETPROTOBYNAME

Provides getprotobyname(2) functionality

    Input

    D0 = IP_GETPROTOBYNAME;
        D3 = (short) timeout (-1)
        A0 = (chanid_t) channel ID
        A1 = (char *)name;
        A2 = (struct protoent *)protoent buffer

    Output

    D0 = result


IP_GETPROTOBYNUMBER

Provides getprotobynumber(2) functionality

    Input

    D0 = IP_GETPROTOBYNUMBER;
    D1 = (int) proto number
        D3 = (short) timeout (-1)
        A0 = (chanid_t) channel ID
        A2 = (struct protoent *)protoent buffer

    Output

    D0 = result


IP_INET_ATON

Provides inet_aton(2) functionality

    Input

        D0 = IP_INET_ATON;
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (char *) name
        A2 = ( struct in_addr *)inaddr buffer

    Output

    D0 = result



IP_INET_ADDR

Provides inet_addr(2) functionality

    Input

        D0 = IP_INET_ADDR
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (char *) name

    Output

    D0 = result



IP_INET_NETWORK

Provides inet_network(2) functionality

    Input

        D0 = IP_INET_NETWORK
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (char *) name

    Output

    D0 = result



IP_INET_NTOA

Provides (2) functionality

    Input

    D0 = IP_INET_NTOA
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (struct in_addr *) net address buffer
        A2 = (char *) result buffer

    Output

    D0 = result



IP_INET_MAKEADDR

Provides (2) functionality

    Input

        D0 = IP_INET_MAKEADDR
        D1 = (int) network number
        D2 = (int) host address
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A2 = (struct in_addr *) result buffer

    Output

    D0 = result



IP_INET_LNAOF

Provides inet_lnaof (2) functionality

    Input

        D0 = IP_INET_LNAOF;
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (struct in_addr *) net address buffer

    Output

    D0 = result



IP_INET_NETOF

Provides inet_netof(2) functionality

    Input

        D0 = IP_INET_NETOF;
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = (struct in_addr *) net address buffer

    Output

    D0 = result


IP_IOCTL

Provides ioctl(2) functionality

    Input

    D0 = IP_IOCTL
    D1 = request
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
    A1 = (char *) argp

    Output

    D0 = result


IP_GETDOMAIN

Provides getdomainname(2) functionality

    Input

        D0 = IP_GETDOMAIN;
        D2 = len;
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
        A1 = name;

    Output

    D0 = result


IP_H_ERRNO

Provides h_errno (2) functionality

    Input

    D0 = IP_H_ERRNO
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID

    Output

    D0 = result
    D1 = h_errno



IP_H_STRERROR

Provides special functionality to return the text for h_errno

    Input

        D0 = IP_H_STRERROR
        D3 = (short) timeout (-1)
    A0 = (chanid_t) channel ID
    A1 = buffer for text

    Output

    D0 = result


```

---

3 Socket Library
----------------

```

int socket(int domain, int type, int protocol);
int bind(int s, const struct sockaddr *name, int namelen);
int listen(int s, int backlog);
int accept(int s, struct sockaddr *addr, int *addrlen);
int connect(int s, const struct sockaddr *name, int namelen);
int sendto(int s, const void *msg, int len, unsigned int flags,
        struct sockaddr *to, int tolen);
int send(int s, const void *msg, int len, unsigned int flags);
int recvfrom(int s, void *buf, int len, unsigned int flags,
          struct sockaddr *from, int *fromlen);
int recv(int s, void *buf, int len, unsigned int flags);
int shutdown(int s, int how);
int setsockopt(int s, int level, int optname, void *optval, int optlen);
int getsockopt(int s, int level, int optname, void *optval, int *optlen);
int getsockname(int s, struct sockaddr *name, int *namelen);
int getpeername(int s, struct sockaddr *name, int *namelen);

u_long inet_addr(const char *);
u_long inet_network(const char *);

struct hostent  *gethostbyname(const char *name);
struct hostent  *gethostbyaddr(const char *addr, int len, int type);
struct netent   *getnetbyname(const char *name);
struct netent   *getnetbyaddr(int net, int type);
struct servent  *getservbyname(const char *name, const char *proto);
struct servent  *getservbyport(int port, const char *proto);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);

char *inet_ntoa(struct in_addr);
struct in_addr inet_makeaddr(int , int);
unsigned long inet_lnaof(struct in_addr);
unsigned long inet_netof(struct in_addr);
int inet_aton(const char *cp, struct in_addr *inp);

int gethostname(char *name, size_t len);

int ioctl(int,int,void *);
int sock_fcntl(int s, int action, int val);
```
