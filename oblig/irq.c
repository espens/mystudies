#include <assert.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * add includes as you need them
 */
#include "delayed_sendto.h"
#include "slow_receiver.h"
#include "l1_phys.h"
#include "l5_app.h"

/*
 * You will be interested to try things repeatedly after a little
 * bit of waiting time. This will happens at several layers, and
 * it will happen for several layers at the same time. These functions
 * help you to register a function when a particular timer has
 * expired. You can use a void pointer to give the function parameters
 */
typedef void (*TimeoutCallFunc)(void *p);

struct TimeoutCallback
{
    struct timeval  calltime;
    TimeoutCallFunc callback;
    void* parameter;
    int timerId;
    struct TimeoutCallback* next;
};
typedef struct TimeoutCallback timeout_cb_t;

static int timerId_next = 0;

/* two local functions, defined below */
static struct timeval* set_timeout_time( struct timeval* tv );
static void check_timeout_expired( );

/*
 * The list of pending timeouts. Initially the list is empty.
 */
static timeout_cb_t* timeouts = 0;

/*
 * This is the main loop of this program.
 * It is meant to emulate a very basic interrupt dispatcher and
 * scheduler of an operating system.
 */
void handle_events( )
{
    int max_fd = my_udp_socket + 1;

    while( 1 )
    {
        fd_set          read_set;
        struct timeval  tv;
        struct timeval* tv_ptr = NULL;
        int             retval;

        /* Allow the timeout mechanisms to set a time when select
         * must wake up at the latest.
         */
        tv_ptr = set_timeout_time( &tv );

        /* The fd_set must be cleared and refilled every time before
         * you call select.
         */
        FD_ZERO( &read_set );
        FD_SET( STDIN_FILENO, &read_set );             /* add keyboard input */
        FD_SET( my_udp_socket, &read_set ); /* add UDP socket */

        /* Now wait until something happens.
         */
        retval = select( max_fd, &read_set, 0, 0, tv_ptr );

        switch( retval )
        {
        case -1 :
            perror( "Error in select" );
            break;

        case 0 :
            /* Nothing happened on the socket or the keyboard. But the
             * timeout has expired.
             */
            check_timeout_expired();
            break;

        default :
            /* Something happened on the socket or keyboard. But the
             * timeout may have expired as well. Check that first.
             */
            check_timeout_expired();

            /* note: when you check several sockets and file descriptors
             *       in an fd_set, always check all of them. If you do an
             *       if-else, one of them may starve.
             */

            /* If file descriptor 0 is set, something has happened on
             * the keyboard. That's most likely user input. Call
             * the application layer directly.
             */
            if( FD_ISSET( STDIN_FILENO, &read_set ) ) l5_handle_keyboard( );

            /* If this file descriptor is set, something has happened on
             * the UDP socket. Probably data has arrived. Call the event
             * handler of the physical layer.
             */
            if( FD_ISSET( my_udp_socket, &read_set ) ) l1_handle_event( );
            break;
        }
    }
}

/*
 * First, we check whether timers have already expired. For all those,
 * we call all callback functions.
 * Second, we check whether there are still timeouts left. If not, we
 * can stop processing. We return 0, and select can wait infinitely
 * or until something else happens.
 * Third, we compute the timeout time for the waiting timeout structure
 * with the shortest waiting time. We substract the current time from
 * it and fill the timeval structure for select with the rest. We
 * return a pointer to that struct.
 */
struct timeval* set_timeout_time( struct timeval* tv )
{
    check_timeout_expired();

    if( timeouts == 0 )
    {
        /* nothing to do */
        return 0;
    }

    struct timeval now;
    gettimeofday( &now, 0 );

    timersub( &timeouts->calltime, &now, tv );

    /* make sure we don't end up calling select() with a negative timeout time */
    if(tv->tv_usec < 0 || tv->tv_sec < 0)
        tv->tv_usec = tv->tv_sec = 0;

    return tv;
}

/*
 * Check whether any timeout callbacks should be triggered because
 * enough time has passed.
 * If yes, call the callback function, than delete the structure.
 */
static void check_timeout_expired( )
{
    if( timeouts == 0 )
    {
        /* nothing to do */
        return;
    }

    struct timeval now;
    gettimeofday( &now, 0 );

    while( timeouts && timercmp( &timeouts->calltime, &now, < ) )
    {
        timeout_cb_t* temp = timeouts;
        timeouts = temp->next;
        temp->next = 0;
        (*temp->callback)(temp->parameter);
        free(temp);
    }
}

/*
 * Add a timeout to the timeout list.
 * The timeout time is an absolute time, as in "today, 16:34".
 * It is NOT a time relative from now, as in "in two minutes".
 * The first timeout that will expire is first in the queue.
 *
 * The return value is a unique id, which can be used 
 * with remove_timeout() to remove the timer.
 */
int register_timeout_cb( struct timeval tv, TimeoutCallFunc cb, void* param )
{
    timeout_cb_t* t;
    t = (timeout_cb_t*)malloc(sizeof(timeout_cb_t));
    t->timerId          = timerId_next++;
    t->calltime.tv_sec  = tv.tv_sec;
    t->calltime.tv_usec = tv.tv_usec;
    t->callback         = cb;
    t->parameter        = param;
    t->next             = 0;

    if( timeouts == 0 )
    {
        timeouts = t;
    }
    else if( timercmp( &t->calltime, &timeouts->calltime, < ) )
    {
        t->next = timeouts;
        timeouts = t;
    }
    else
    {
        timeout_cb_t* finder = timeouts;

        while( finder->next )
        {
            if( timercmp( &t->calltime, &finder->next->calltime, < ) )
            {
                t->next = finder->next;
                finder->next = t;
                return t->timerId;
            }

            finder = finder->next;
        }

        finder->next = t;
    }

    return t->timerId;
}


/* Remove a timeout from the timeout list.
 *
 * The parameter is a timer id, as returned from register_timeout_cb().
 * Returns 0 on success, -1 if not found.
 */
int remove_timeout( int timerId ) 
{
    timeout_cb_t *t = timeouts;
    timeout_cb_t *prev = NULL;

    while(t != NULL) 
    {
        if(t->timerId == timerId)
            break;

        prev = t;
        t = t->next;
    }

    if(t == NULL) /* not found */
        return -1;

    if(prev == NULL) /* remove head */
    {
        timeouts = timeouts->next;
    }
    else 
    {
        prev->next = t->next;
    }

    free(t);
    return 0;
}


