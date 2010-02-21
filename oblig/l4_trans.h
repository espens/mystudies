#ifndef L4_TRANS_H
#define L4_TRANS_H

/* see comments in the c file */

void l4_init( );
void l4_linkup( int other_address, const char* other_hostname, int other_port );

int  l4_getport( int pid, int desired_port );
void l4_putport( int port );

int  l4_send( int dest_address, int dest_port, int src_port, const char* buf, int length );
int  l4_recv( int host_address, const char* buf, int length );

#endif /* L4_TRANS_H */

