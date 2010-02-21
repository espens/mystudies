#ifndef IRQ_H
#define IRQ_H

void handle_events( );

int register_timeout_cb( struct timeval tv, void (*cb)(), void* param );
int remove_timeout( int timer_id );

#endif /* IRQ_H */
