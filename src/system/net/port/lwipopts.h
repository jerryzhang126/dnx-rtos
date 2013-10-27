/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "config.h"
#include "system/dnx.h"

/*
   -----------------------------------
   ---------- Miscellaneous ----------
   -----------------------------------
*/

#define LWIP_ERROR(message, expression, handler)
#define LWIP_PLATFORM_ASSERT(x)

/*
   -----------------------------------------------
   ---------- Platform specific locking ----------
   -----------------------------------------------
*/
/*
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    1

/*
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  0

/*
 * LWIP_NOASSERT=1: Disable asserts
 */
#define LWIP_NOASSERT           1

/*
 * NO_SYS_NO_TIMERS==1: Drop support for sys_timeout when NO_SYS==1
 * Mainly for compatibility to old versions.
 */
#define NO_SYS_NO_TIMERS        1

/*
 * LWIP_NETIF_HOSTNAME==1: use DHCP_OPTION_HOSTNAME with netif's hostname
 * field.
 */
#define LWIP_NETIF_HOSTNAME     1


/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/*
 * MEM_ALIGNMENT: should be set to the alignment of the CPU for which
 * lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
 * byte alignment -> define MEM_ALIGNMENT to 2.
 */
#define MEM_ALIGNMENT           CONFIG_HEAP_ALIGN

/*
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#define MEM_SIZE                (4*1024)

/*
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC         1

/*
 * MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
 * Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
 * speed and usage from interrupts!
 */
#define MEMP_MEM_MALLOC         1

/*
 * Memory allocation functions
 */
#define mem_free                sysm_tskfree
#define mem_malloc              sysm_tskmalloc
#define mem_calloc              sysm_tskcalloc

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/*
 * MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
 * sends a lot of data out of ROM (or other static memory), this
 * should be set high.
 */
#define MEMP_NUM_PBUF           10

/*
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 */
#define MEMP_NUM_UDP_PCB        4

/*
 * MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
 * connections.
 */
#define MEMP_NUM_TCP_PCB        10

/*
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
 * connections.
 */
#define MEMP_NUM_TCP_PCB_LISTEN 6

/*
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
 * segments.
 */
#define MEMP_NUM_TCP_SEG        12

/*
 * MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
 * timeouts.
 */
#define MEMP_NUM_SYS_TIMEOUT    5


/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/
/*
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE          10

/*
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool.
 */
#define PBUF_POOL_BUFSIZE       1500


/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
/*
 * LWIP_TCP==1: Turn on TCP.
 */
#define LWIP_TCP                1

/*
 * TCP_TTL: Default Time-To-Live value.
 */
#define TCP_TTL                 255

/*
 * Controls if TCP should queue segments that arrive out of
 * order. Define to 0 if your device is low on memory.
 */
#define TCP_QUEUE_OOSEQ         0

/*
 * TCP Maximum segment size.
 */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/*
 * TCP sender buffer space (bytes).
 */
#define TCP_SND_BUF             (2*TCP_MSS)

/*
 * TCP sender buffer space (pbufs).
 * This must be at least = 2 * TCP_SND_BUF/TCP_MSS for things to work.
 */
#define TCP_SND_QUEUELEN        (6 * TCP_SND_BUF)/TCP_MSS

/*
 * TCP receive window.
 */
#define TCP_WND                 (2*TCP_MSS)


/*
   ----------------------------------
   ---------- ICMP options ----------
   ----------------------------------
*/
/*
 * LWIP_ICMP==1: Enable ICMP module inside the IP stack.
 * Be careful, disable that make your product non-compliant to RFC1122
 */
#define LWIP_ICMP               1


/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
/*
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP               1


/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/
/*
 * LWIP_UDP==1: Turn on UDP.
 */
#define LWIP_UDP                1

/*
 * UDP_TTL: Default Time-To-Live value.
 */
#define UDP_TTL                 255


/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/
/*
 * LWIP_STATS==1: Enable statistics collection in lwip_stats.
 */
#define LWIP_STATS 0

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
/*
The STM32F107 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
#endif


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/*
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/*
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     0

#endif /* __LWIPOPTS_H__ */

