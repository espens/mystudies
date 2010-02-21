#ifndef L3_NET_H
#define L3_NET_H

/* see comments in the c file */

void l3_init( int self );
void l3_linkup( const char* other_hostname, int other_port, int other_mac_address );

int  l3_send( int host_address, const char* buf, int length );
int  l3_recv( int mac_address, const char* buf, int length );

#endif /* L3_NET_H */

