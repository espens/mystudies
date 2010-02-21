#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "l2_link.h"
#include "l3_net.h"
#include "l4_trans.h"

#define MAX_ADDRESSES 1024

/*
 * This header is included in every network layer packet.
 */
struct L3Header
{
    int src_address;
    int dst_address;
};

/*
 * This static variable contains the host name of this machine.
 * It must be initialized at startup time.
 */
static int own_host_address = -1;

/*
 * The network layer needs to maintain private information about
 * the link that leads to a host address. Filling this map is
 * part of the routing process.
 * The map is static because it is inappropriate for other layers
 * to see or change it. Network layer functions should be written
 * for all necessary changes to the map.
 */
static int host_to_mac_map[MAX_ADDRESSES];

/*
 * Call at the start of the program. Initialize data structures
 * like an operating system would do at boot time. Set the own
 * host address.
 *
 * ONLY FOR ASSIGNMENT 1:
 * Host address and mac address are simply the same, allowing
 * a one-to-one mapping. Routing doesn't happen yet.
 */
void l3_init( int self )
{
    int i;

    own_host_address = self;

    for( i=0; i<MAX_ADDRESSES; i++ )
    {
        host_to_mac_map[i] = i;
    }
}

/*
 * We have received a MAC address and should lookup or create a host
 * address for it (we fake the address assignment, we don't want to
 * implement ARP (address resolution protocol) in this course!).
 *
 * We have chosen MAC address = host address
 */
void l3_linkup( const char* other_hostname, int other_port, int other_mac_address )
{
    int other_host_address = other_mac_address;
    l4_linkup( other_host_address, other_hostname, other_port );
}

/*
 * Called by layer 4, transport, when it wants to send data to the
 * host identified by host_address.
 * A positive return value means the number of bytes that have been
 * sent.
 * A negative return value means that an error has occured.
 */
int l3_send( int dest_address, const char* buf, int length )
{
    int              mac_address;
    char*            l2buf;
    struct L3Header* hdr_pointer;
    int              retval;

    mac_address = host_to_mac_map[dest_address];

    l2buf = (char*)malloc( length+sizeof(struct L3Header) );
    if( l2buf == 0 )
    {
        fprintf( stderr, "Not enough memory in l3_send\n" );
        return -1;
    }

    memcpy( &l2buf[sizeof(struct L3Header)], buf, length );

    hdr_pointer = (struct L3Header*)l2buf;
    hdr_pointer->dst_address = htonl(dest_address);
    hdr_pointer->src_address = htonl(own_host_address);

    retval = l2_send( mac_address, l2buf, length+sizeof(struct L3Header) );
    free(l2buf);
    if( retval < 0 )
    {
        return -1;
    }
    else
    {
        return retval-sizeof(struct L3Header);
    }
}

/*
 * Called by layer 2, link, when it has received data and wants to
 * deliver it.
 * A positive return value means that all data has been delivered.
 * A zero return value means that the receiver can not receive the
 * data right now.
 * A negative return value means that an error has occured and
 * receiving failed.
 *
 * NOTE:
 * Packet forwarding should be added here when it becomes relevant.
 * Routing is usually not included in layer 3 code, but sits beside
 * it. But routing can call functions of layer 3 to update
 * forwarding information.
 */
int l3_recv( int mac_address, const char* buf, int length )
{
    const struct L3Header* hdr_pointer;
    const char*            l4buf;
    int                    dest_address;
    int                    src_address;

    hdr_pointer   = (const struct L3Header*)buf;
    src_address   = ntohl(hdr_pointer->src_address);
    dest_address  = ntohl(hdr_pointer->dst_address);
    if( dest_address != own_host_address )
    {
        /* no packet forwarding yet */
        return -1;
    }

    l4buf = buf + sizeof(struct L3Header);
    return l4_recv( src_address, l4buf, length-sizeof(struct L3Header) );
}
