#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "irq.h"
#include "l1_phys.h"
#include "l2_link.h"
#include "l3_net.h"
#include "l4_trans.h"
#include "l5_app.h"

int main( int argc, char* argv[] )
{
    int          udp_socket_port;
    int          local_port;
    int          local_unique_id;
    int          local_mac_address;
    int          local_host_address;
    int          phys_device;

    if( argc != 3 )
    {
        fprintf( stderr, "Usage: %s <port> <id>\n"
                         "       <port> is the UDP port used on this machine\n"
                         "       <id> is the fake MAC address of this machine\n",
                         argv[0] );
        exit( -1 );
    }

    /*
     * Read information from the command line. This is very primitive.
     * Refine as you see fit.
     */
    local_port       = atoi(argv[1]);
    local_unique_id  = atoi(argv[2]);  /* use for MAC and network address */

    /*
     * Fill the structs necessary for initializing all the
     * layers.
     */
    udp_socket_port    = local_port;

    phys_device        = 0;
    local_mac_address  = local_unique_id;
    local_host_address = local_unique_id;

    /*
     * Initialize all layers. This can include setting up all the
     * network connections, but it doesn't have to. It is also OK
     * to send connect-requests and handle the responses later, in
     * the handle_events loop. Your choice.
     */
    l1_init( udp_socket_port );
    l2_init( local_mac_address, phys_device );
    l3_init( local_host_address );
    l4_init( );
    l5_init( /* whatever you need */ );

    /*
     * An endless loop for processing everything that happens on this
     * machine.
     */
    handle_events( );

    return 1;
}
