#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "l3_net.h"
#include "l4_trans.h"
#include "l5_app.h"

#define MAX_PORTS 1024

/*
 * The transport layer header that is include in every datagram
 * or segment.
 */
struct L4Header
{
    int dest_port;
    int src_port;
};

/*
 * The transport layer needs to maintain private information about
 * the ports that an application uses to communicate. In real
 * operating systems, ports do not have to be globally unique.
 * That is a simplification, and you are welcome to fix it.
 */
static int port_to_process_map[MAX_PORTS];

/*
 * Call at the start of the program. Initialize data structures
 * like an operating system would do at boot time. Initialize all
 * the data structures that you want to use for error correction
 * and end-to-end flow control here.
 *
 * NOTE:
 * You have to add many structures if you want a transport layer
 * that supports connection-oriented communication.
 */
void l4_init( )
{
    int i;
    for( i=0; i<MAX_PORTS; i++ )
    {
        port_to_process_map[i] = -1;
    }
}

/*
 * Nothing to do here.
 */
void l4_linkup( int other_address, const char* other_hostname, int other_port )
{
    l5_linkup( other_address, other_hostname, other_port );
}

/*
 * Allows the application layer to reserve one of the ports for
 * a "process". We don't actually create any processes in the
 * assignments of this course, but we can have multiple fake
 * processes if we want to.
 * The first parameter identifies the "process". The second
 * parameter identifies the desired port. If it is -1, any
 * port is acceptable.
 * If the port allocation fails, the function returns -1, otherwise
 * the allocated port.
 */
int l4_getport( int pid, int desired_port )
{
    int i;
    if( desired_port >= 0 )
    {
        if( port_to_process_map[desired_port] == -1 )
        {
            port_to_process_map[desired_port] = pid;
            return desired_port;
        }
        return -1;
    }
    else
    {
        for( i=MAX_PORTS-1; i>=0; i-- )
        {
            if( port_to_process_map[i] == -1 )
            {
                port_to_process_map[i] = pid;
                return i;
            }
        }
        return -1;
    }
}

/*
 * Release a port that has been allocated using l4_getport
 */
void l4_putport( int port )
{
    port_to_process_map[port] = -1;
}

/*
 * Called by the application layer when it wants to send data to the
 * host identified by (dest_address,dest_port). The src_port is
 * added by the calling "process" to allow the receive to figure
 * out who the sender is.
 * A positive return value means the number of bytes that have been
 * sent.
 * A negative return value means that an error has occured.
 */
int l4_send( int dest_address, int dest_port, int src_port, const char* buf, int length )
{
    char*            l3buf;
    struct L4Header* hdr_pointer;
    int              retval;

    l3buf = (char*)malloc( length+sizeof(struct L4Header) );
    if( l3buf == 0 )
    {
        fprintf( stderr, "Not enough memory in l4_send\n" );
        return -1;
    }

    memcpy( &l3buf[sizeof(struct L4Header)], buf, length );

    hdr_pointer = (struct L4Header*)l3buf;
    hdr_pointer->src_port  = htonl(src_port);
    hdr_pointer->dest_port = htonl(dest_port);

    retval = l3_send( dest_address, l3buf, length+sizeof(struct L4Header) );
    free(l3buf);
    if( retval < 0 )
    {
        return -1;
    }
    else
    {
        return retval-sizeof(struct L4Header);
    }
}

/*
 * Called by layer 3, network, when it has received data for the
 * local host and wants to deliver it.
 * A positive return value means that all data has been delivered.
 * A zero return value means that the receiver can not receive the
 * data right now.
 * A negative return value means that an error has occured and
 * receiving failed.
 */
int l4_recv( int src_address, const char* buf, int length )
{
    const struct L4Header* hdr_pointer;
    const char*            l5buf;
    int                    src_port;
    int                    dest_port;
    int                    dest_pid;

    hdr_pointer = (const struct L4Header*)buf;
    src_port    = ntohl(hdr_pointer->src_port);
    dest_port   = ntohl(hdr_pointer->dest_port);
    dest_pid    = port_to_process_map[dest_port];

    l5buf = buf + sizeof(struct L4Header);
    return l5_recv( dest_pid, src_address, src_port, l5buf, length-sizeof(struct L4Header) );
}
