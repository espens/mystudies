#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "l1_phys.h"
#include "l2_link.h"

#include "delayed_dropping_sendto.h"

#define MAX_CONNS 1024

static phys_conn_t my_conns[MAX_CONNS];

int my_udp_socket = -1;

/* Finds the connection associated with the given sockaddr */
static phys_conn_t *get_phys_conn( struct sockaddr_in *addr ) {
    phys_conn_t *conn = NULL;

    int i;
    for( i=0; i<MAX_CONNS && my_conns[i].remote_hostname != 0; ++i )
    {
        if ( my_conns[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr 
                && my_conns[i].addr.sin_port == addr->sin_port )
        {
            /* Match */
            conn = &my_conns[i];
            break;
        }
    }

    return conn;
}

/* Create an entry in the table of physical connection */
static phys_conn_t *create_phys_conn( const char *hostname, unsigned short port )
{
    phys_conn_t *conn = NULL;

    /* Find an available device id */
    int device;
    for( device=0; device<MAX_CONNS; device++ )
    {
        if( my_conns[device].remote_hostname == 0 )
        {
            conn = &my_conns[device];
            conn->device = device;
            break;
        }
    }

    if( !conn  )
    {
        fprintf( stderr, "Too many physical connections established.\n" );
        exit( -1 );
    }

    conn->remote_hostname = strdup(hostname);
    conn->remote_port = port;
    conn->state = UNASSIGNED;



    /* ... */

    /* HERE: Get the address of the remote host and initialize conn->addr.  */

    
    return conn;
}

/*
 * Call at the start of the program. Initialize data structures
 * like an operating system would do at boot time.
 *
 * local_port is the port that for the local UDP socket that
 *   we will use for all communication.
 *
 * hosts is a pointer to an array of remote hostnames and ports
 *   that we want to establish connections to. Each entry in
 *   hosts represents one network device. The index of the entry
 *   is the device number.
 *
 * numhosts is the number of hosts in the array.
 *
 * Here in particular: initialize all of your socket communication
 * in this function. The socket communication is meant to fake
 * physical cables, so it should be done completely before
 * continuing.
 */
void l1_init( int local_port )
{
    int                err;
    struct sockaddr_in addr;

    my_udp_socket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( my_udp_socket < 0 )
    {
        fprintf( stderr, "Failed to create local UDP socket" );
        exit( -1 );
    }

    memset( &addr, 0, sizeof(struct sockaddr_in) );
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(local_port);

    err = bind( my_udp_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in) );
    if( err < 0 )
    {
        fprintf( stderr, "Failed to bind local UDP socket to port %d", local_port );
        exit( -1 );
    }

    memset( my_conns, 0, sizeof(my_conns) );
}

/*
 * Initiate establishment of a "physical connection" to the given host and
 * port. The return value of the function is the device that has been
 * allocated to this connection.
 */
int l1_connect( const char* hostname, int port )
{
    phys_conn_t *conn = create_phys_conn( hostname, port );

    if (!conn) {
        fprintf( stderr, "Could not create physical connection to %s\n", hostname );
        return -1;
    }


    /* ... */

    /*
     * Extend this function to set up all the necessary information that 
     * makes it possible to communicate between my_udp_socket and the 
     * socket on (hostname,port) as if there was a physical connection.
     *
     * To do that, you can block in this function, or you can make use of
     * the select loop.
     */


    return conn->device;
}

/*
 * Called by layer 2, link, when it wants to send data over the
 * "physical connection" that is represented by device.
 * A positive return value means the number of bytes that have been
 * sent.
 * A negative return value means that an error has occured.
 */
int l1_send( int device, const char* buf, int length )
{    
    /* ... */
    return -1;
    /* ... */

    /*
     * Extend this function so that you send the given data over YOUR
     * "physical connection" using the function sendto(), the
     * function delayed_sendto() and the function delayed_dropping_sendto().
     *
     * It's easiest to start with sendto(), because it doesn't delay
     * and it doesn't drop packets randomly.
     *
     * delayed_sendto() works very similar to the system function
     * sendto() but it delays packet delivery in annoying ways.
     * The same for delayed_dropping_sendto().
     */
}

/*
 * If a packet that has been received is an UP packet that establishes
 * a link instead of carrying data, l1_handle_event should call this
 * function.
 * The function should assign a device that is now connected in the
 * table my_conn and print an UP message onto the screen.
 */
static phys_conn_t *l1_linkup( phys_conn_t *conn, const char* other_hostname, int other_port, int other_address )
{

    if ( !conn) {
        /* If the conn parameter was NULL, we need to assign a new device.  */

        /* ... */
   }

    /* ... */

    return conn;
}

/*
 * In interrupt occurs when data arrives. Our interrupts are simulated
 * by data-arrival events in the select loop.
 * When select notices that data for my_udp_socket has arrived, it calls
 * this function.
 *
 * A positive return value means that all data has been delivered.
 * A zero return value means that the receiver can not receive the
 * data right now.
 * A negative return value means that an error has occured and
 * receiving failed.
 *
 * NOTE:
 * Link layer error correction and flow control must be considered
 * here. You will certainly need several helper functions because
 * you will need to perform retransmissions after a timeout.
 */
void l1_handle_event( )
{

    /* ... */

    /*
     * Write this function so that it can deliver data to layer 2
     * using l2_recv. You must find the right device identifier to
     * pass to l2_recv. A good way of finding it is a combination
     * the system function recvfrom for receiving from the network
     * and finding the right entry in my_conns.
     *
     * This function does not have any return values. The physical
     * layer can not handle errors, and the interrupt handler can't,
     * either. The higher level functions must be able to deal with
     * everything.
     */

     /* Should among other functions call if the data that has
      * arrived was an UP packet.
      *
      * The hostname and port that l1_linkup needs are those of
      * the remote host.
      *    l2_linkup( hostname, port, mac_address );
      */
}

