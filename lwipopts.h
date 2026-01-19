#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Grundläggande inställningar för Pico W
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            16
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_STATS                  0
#define LWIP_CHECKSUM_CTRL_PER_PBUF 0
#define TCP_LISTEN_BACKLOG          1
#define LWIP_DNS                    1
#define LWIP_HTTPD_CGI              1
#define LWIP_HTTPD_SSI              1
#define LWIP_NETIF_LIST_GENERATOR 1 // Denna rad är ofta det som fixar 'netif_list' felet
#define LWIP_HAVE_LOOPIF            1
#define LWIP_SINGLE_NETIF           0
#define LWIP_NETIF_LOOPBACK         1
#endif