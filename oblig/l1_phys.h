#ifndef L1_PHYS_H
#define L1_PHYS_H

#include <netinet/in.h>

struct PhysicalConnection
{
    int device;
    char* remote_hostname;
    int   remote_port;
    struct sockaddr_in addr;

    enum {
        UNASSIGNED = 0,
        CONNECTING,
        ESTABLISHED,
        DISCONNECTED,
    } state;
};

typedef struct PhysicalConnection phys_conn_t;

/*
 * This is the one UDP socket that is used for all sending
 * and receiving. The select loop must know it.
 */
extern int my_udp_socket;

/* see more comments in the c file */

void l1_init( int local_port, int local_mac_address );
int  l1_connect( const char* hostname, int port );
void l1_req_physical_connection( const char* hostname, int port );
int  l1_send( int device, const char* buf, int length );
void l1_handle_event( );

#endif /* L1_PHYS_H */

