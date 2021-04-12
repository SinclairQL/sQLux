/* UQLX  */

/* fucntions to translate C68 to/from HOST constants and options */

#ifdef IPDEV

#include "xc68.h"
#include <errno.h>
#include <stdio.h>

int c68err(int err)
{
  switch(err)
    {
      case ENOTSOCK: return C68ERR_ENOTSOCK;
      case EDESTADDRREQ: return C68ERR_EDESTADDRREQ;
      case EMSGSIZE: return C68ERR_EMSGSIZE;
      case EPROTOTYPE: return C68ERR_EPROTOTYPE;
      case ENOPROTOOPT: return C68ERR_ENOPROTOOPT;
      case EPROTONOSUPPORT: return C68ERR_EPROTONOSUPPORT;
      case ESOCKTNOSUPPORT: return C68ERR_ESOCKTNOSUPPORT;
      case EOPNOTSUPP: return C68ERR_EOPNOTSUPP;
      case EPFNOSUPPORT: return C68ERR_EPFNOSUPPORT;
      case EAFNOSUPPORT: return C68ERR_EAFNOSUPPORT;
      case EADDRINUSE: return C68ERR_EADDRINUSE;
      case EADDRNOTAVAIL: return C68ERR_EADDRNOTAVAIL;
      case ENETDOWN: return C68ERR_ENETDOWN;
      case ENETUNREACH: return C68ERR_ENETUNREACH;
      case ENETRESET: return C68ERR_ENETRESET;
      case ECONNABORTED: return C68ERR_ECONNABORTED;
      case ECONNRESET: return C68ERR_ECONNRESET;
      case ENOBUFS: return C68ERR_ENOBUFS;
      case EISCONN: return C68ERR_EISCONN;
      case ENOTCONN: return C68ERR_ENOTCONN;
      case ESHUTDOWN: return C68ERR_ESHUTDOWN;
      case ETOOMANYREFS: return C68ERR_ETOOMANYREFS;
      case ETIMEDOUT: return C68ERR_ETIMEDOUT;
      case ECONNREFUSED: return C68ERR_ECONNREFUSED;
      case EHOSTDOWN: return C68ERR_EHOSTDOWN;
      case EHOSTUNREACH: return C68ERR_EHOSTUNREACH;
      case EALREADY: return C68ERR_EALREADY;
      case EINPROGRESS: return C68ERR_EINPROGRESS;
       default: return -1;
    }
}

char *protoname(int proto)
{
  switch(proto)
    {
    case  C68_IPPROTO_IP   : return " dummy for IP ";
    case  C68_IPPROTO_ICMP : return " control message protocol ";
    case  C68_IPPROTO_GGP  : return " gateway^2 (deprecated) ";
    case  C68_IPPROTO_TCP  : return " tcp ";
    case  C68_IPPROTO_EGP  : return " exterior gateway protocol ";
    case  C68_IPPROTO_PUP  : return " pup ";
    case  C68_IPPROTO_UDP  : return " user datagram protocol ";
    case  C68_IPPROTO_IDP  : return " xns idp ";
    case  C68_IPPROTO_TP   : return " tp-4 w/ class negotiation ";
    case  C68_IPPROTO_EON  : return " ISO cnlp ";
    case  C68_IPPROTO_RAW  : return " raw IP packet ";
  default : return "unknown protocol";
  }

}


/* dummy fns, sometimes it should do all translations*/
void xso_q2x(int level,int optname, void* optval,int len)
{
  char *proto;

  proto=protoname(level);

  printf("xso_q2x: proto %d %s, optname %d\n",level,proto,optname);
}
void xso_x2q(int level,int optname, void* optval,int len)
{
  char *proto;

  proto=protoname(level);

  printf("xso_q2x: proto %d %s, optname %d\n",level,proto,optname);
}

#endif /* IPDEV */
