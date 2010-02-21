#ifndef L2_LINK_H
#define L2_LINK_H

/*
 * This struct is meant to keep information about the local
 * physical layer device that must be used to reach a device
 * with a given remote MAC address.
 * The local device does have an own MAC address as well, of
 * course, even though we don't ever check whether a frame
 * has actually been sent to it or to another MAC address.
 */
struct LinkEntry
{
    int remote_mac_address;
    int phys_device;
};
typedef struct LinkEntry link_entry_t;

/* see more comments in the c file */

void l2_init( int local_mac_address, int device );
void l2_linkup( int device, const char* other_hostname, int other_port, int other_mac_address );

int  l2_send( int mac_address, const char* buf, int length );
void l2_recv( int device, const char* buf, int length );

#endif /* L2_LINK_H */
