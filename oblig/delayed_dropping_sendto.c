#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "delayed_dropping_sendto.h"
#include "irq.h"

/* Delay all messages for this time. */
static struct timeval msg_delay = { 3, 0 };  /* 10 seconds delay */

/* see below */
static void send_dropping_delayed( );

/*
 * Every data packet that we want to send with delay is stored in
 * elements of this list type.
 */
struct queue_entry
{
    struct timeval   sendtime;
    int              my_sock;
    void*            data;
    size_t           data_len;
    struct sockaddr* to;
    socklen_t        to_len;
    struct queue_entry* next;
};

typedef struct queue_entry qe_t;

static qe_t* head = NULL;
static qe_t* tail = NULL;

/*
 * For the caller, this function behaves exactly like the function sendto(),
 * except that it always returns success, although it drops packets randomly
 * and adds delay before actually sending the data.
 */

/*
 * Use this function as described in 'man sendto'.
 * The difference is that all necessary information is stored in a queue
 * and the actual send operation is delayed for a time that is specified
 * in a variable named msg_delay that you find in delayed_sendto.c
 *
 * This means also that delayed_sendto will always return without error.
 */
ssize_t delayed_dropping_sendto(int s, const void *msg, size_t len, int flags,
                                const struct sockaddr *to, socklen_t tolen)
{
    int            error;
    struct timeval tnow;
    struct queue_entry* ptr;

    /* just swallow the packet and insist that all is OK */
    if( lrand48() % 20 == 0 )
    {
        return len;
    }

    ptr = (struct queue_entry*)malloc(sizeof (struct queue_entry));
    assert(ptr);

    error = gettimeofday( &tnow, NULL );
    assert( error >= 0 );
    timeradd( &tnow, &msg_delay, &ptr->sendtime );

    ptr->my_sock = s;

    ptr->data = malloc(len);
    memcpy( ptr->data, msg, len );
    ptr->data_len = len;

    ptr->to = (struct sockaddr*)malloc(tolen);
    memcpy( ptr->to, to, tolen );
    ptr->to_len = tolen;

    ptr->next = NULL;

    if( tail )
    {
        tail->next = ptr;
        tail = ptr;
    }
    else
    {
        register_timeout_cb( ptr->sendtime, &send_dropping_delayed, NULL );
        head = tail = ptr;
    }
    return len;
}

/*
 * This function is called whenever a timeout for the oldest data that
 * is to be sent delayed expires. It does the actual sending and starts
 * a new timeout if there is more data left.
 */
void send_dropping_delayed( )
{
    struct timeval tnow;
    qe_t*          ptr;
    int            error;
    error = gettimeofday( &tnow, NULL );
    if( error < 0 )
    {
        perror( "error getting time" );
        return;
    }
    while( head && timercmp( &head->sendtime, &tnow, <= ) )
    {
        error = sendto( head->my_sock,
                        head->data, head->data_len,
                        0,
                        head->to, head->to_len );
        if( error < 0 )
        {
            perror( "Error sending delayed data" );
        }
        ptr = head;
        head = ptr->next;
        if( head == NULL ) tail = NULL;
        free( ptr->data );
        free( ptr->to );
        free( ptr );
    }

    if( head )
    {
        register_timeout_cb( head->sendtime, &send_dropping_delayed, NULL );
    }
}

