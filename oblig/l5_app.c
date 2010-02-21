#include <string.h>
#include <stdio.h>

#include "slow_receiver.h"
#include "l5_app.h"
#include "l1_phys.h"

/*
 * Initialize however you want.
 */
void l5_init( )
{

}

/*
 * Report success for a physical link establishment.
 */
void l5_linkup( int other_address, const char* other_hostname, int other_port )
{
    fprintf( stderr, "Successfully established a physical link (plugged in a cable)\n"
                     "with host:port %s:%d.\n"
                     "We can use the address >>%d<< for that machine.\n"
                     "\n",
                     other_hostname,
                     other_port,
                     other_address );
}

void l5_handle_keyboard( )
{
    char  buffer[1024];
    char *retval;


    retval = fgets( buffer, sizeof(buffer), stdin );
    if( retval != 0 )
    {
        buffer[strlen(buffer)-1] = 0;

        /* debug */
        fprintf( stderr, "The buffer contains: >>%s<<\n", buffer );

        if( strstr( buffer, "CONNECT" ) != NULL )
        {
            char hostname[1024];
            int  port;
            int  device;

            /*
             * sscanf tries to find the pattern in the buffer, and extracts
             * the parts into the given variables.
             */
            int ret = sscanf( buffer, "CONNECT %s %d", hostname, &port );
            if( ret == 2 )
            {
                /* two matches, we got our hostname and port */
                device = l1_connect( hostname, port );

                fprintf( stderr,
                         "Physical connection to host:port %s:%d has device number %d\n",
                         hostname, port, device );

            }
        }

        /* Your keyboard processing here */

        /* ... */
    }
}

int l5_recv( int dest_pid, int src_address, int src_port, const char* l5buf, int sz )
{
    return slow_receiver( l5buf, sz );
}
