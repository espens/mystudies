#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "l1_phys.h"
#include "l2_link.h"
#include "l3_net.h"

#define MAX_ADDRESSES 1024

/*
 * The MAC header. It is included in every frame.
 * You must extend it for your needs.
 */
struct L2Header
{
    int src_mac_address;
    int dst_mac_address;
};

/*
 * The network layer needs to maintain private information about
 * the MAC address at the other end of every link.
 * The map is static because it is inappropriate for other layers
 * to see or change it.
 */
static link_entry_t mac_to_device_map[MAX_ADDRESSES];

/*
 * Call at the start of the program. Initialize data structures
 * like an operating system would do at boot time.
 *
 * In particular initialize all the data structures that you
 * need for link-layer error correction and flow control.
 */
void l2_init( int local_mac_address, int device )
{
    int mac;

    for( mac=0; mac<MAX_ADDRESSES; mac++ )
    {
        mac_to_device_map[mac].remote_mac_address = -1;
        mac_to_device_map[mac].phys_device        = -1;
    }

    mac_to_device_map[local_mac_address].remote_mac_address = -1;
    mac_to_device_map[local_mac_address].phys_device        = device;
}

/*
 * We have gotten an UP packet for a particular device from the remote host.
 * We have to remember that in our table that maps MAC addresses to devices.
 */
void l2_linkup( int device, const char* other_hostname, int other_port, int other_mac_address )
{
    int mac;
    for( mac=0; mac<MAX_ADDRESSES; mac++ )
    {
        if( mac_to_device_map[mac].phys_device == device )
        {
            mac_to_device_map[mac].remote_mac_address = other_mac_address;
            l3_linkup( other_hostname, other_port, other_mac_address );
            return;
        }
    }
    fprintf( stderr, "Programming error in establishing a physical link\n" );
    exit( -1 );
}

/*
 * Called by layer 3, network, when it wants to send data to a
 * direct neighbour identified by the MAC address.
 * A positive return value means the number of bytes that have been
 * sent.
 * A negative return value means that an error has occured.
 *
 * NOTE:
 * You will need to split this function into many small helper
 * functions. In particular, you will need something that allows
 * you to perform retransmissions after a timeout.
 */
int l2_send( int dest_mac_addr, const char* buf, int length )
{
    int   device = -1;
    int   src_mac_addr = -1;
    char* l1buf;
    struct L2Header*  hdr_pointer;
    int   retval;
    int   i;

    for( i=0; i<MAX_ADDRESSES; i++ )
    {
        if( mac_to_device_map[i].remote_mac_address == dest_mac_addr )
        {
            device       = mac_to_device_map[i].phys_device;
            src_mac_addr = i;
            break;
        }
    }
    if( i==MAX_ADDRESSES )
    {
        fprintf( stderr, "MAC address not found in l2_send" );
        return -1;
    }

    l1buf = (char*)malloc( length+sizeof(struct L2Header) );
    if( l1buf == 0 )
    {
        fprintf( stderr, "Not enough memory in l2_send\n" );
        return -1;
    }

    memcpy( &l1buf[sizeof(struct L2Header)], buf, length );

    hdr_pointer = (struct L2Header*)l1buf;
    hdr_pointer->src_mac_address = htonl(src_mac_addr);
    hdr_pointer->dst_mac_address = htonl(dest_mac_addr);

    retval = l1_send( device, l1buf, length+sizeof(struct L2Header) );
    free(l1buf);
    if( retval < 0 )
    {
        return -1;
    }
    else
    {
        return retval-sizeof(struct L2Header);
    }
}

/*
 * Called by layer 1, physical, when a frame has arrived.
 *
 * This function has no return value. It must handle all
 * problems itself because the physical layer isn't able
 * to handle errors.
 *
 * NOTE:
 * Link layer error correction and flow control must be considered
 * here. You will certainly need several helper functions because
 * you will need to perform retransmissions after a timeout.
 */
void l2_recv( int device, const char* buf, int length )
{
    const struct L2Header* hdr_pointer;
    const char*            l3buf;
    int                    src_mac_address;
    int                    dst_mac_address;
    int                    err;

    hdr_pointer = (const struct L2Header*)buf;
    src_mac_address  = ntohl(hdr_pointer->src_mac_address);
    dst_mac_address  = ntohl(hdr_pointer->dst_mac_address);

    /*
     * This is not enough.
     * Add link layer protocol processing!
     */

    l3buf = buf + sizeof(struct L2Header);
    err = l3_recv( dst_mac_address, l3buf, length-sizeof(struct L2Header) );

    /*
     * Add error handling.
     * The most common error will be that the receiver is too slow.
     */
}
