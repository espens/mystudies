#ifndef L5_APP_H
#define L5_APP_H

/* see comments in the c file */

void l5_init( );
void l5_linkup( int other_address, const char* other_hostname, int other_port );

void l5_handle_keyboard( );
int l5_recv( int dest_pid, int src_address, int src_port, const char* l5buf, int sz );

#endif /* L5_APP_H */

