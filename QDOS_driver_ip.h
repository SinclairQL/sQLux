
#ifndef QL_SOCKET_H
#define QL_SOCKET_H

#include <stdint.h>

#pragma pack(push,1)

struct ql_sockaddr {
    uint16_t   sa_family;          /* address family */
    uint8_t    sa_data[14];
};

struct ql_in_addr {
    uint32_t    ql_s_addr;
};

struct ql_sockaddr_in {
    uint16_t    sin_family;
    uint16_t    sin_port;
    struct ql_in_addr sin_addr;
    uint8_t     sin_zero[8];
};

struct ql_sockproto {
    uint16_t    sp_family;          /* address family */
    uint16_t    sp_protocol;        /* protocol */
};

struct  ql_hostent {
    uint32_t    h_name;             /* official name of host */
    uint32_t    h_aliases;          /* alias list */
    int32_t     h_addrtype;         /* host address type */
    int32_t     h_length;           /* length of address */
    uint32_t    h_addr_list;        /* list of addresses from name server */
#define ql_h_addr  h_addr_list[0]   /* address, for backward compatiblity */
};

#pragma pack(pop)

#endif /* QL_SOCKET_H */
