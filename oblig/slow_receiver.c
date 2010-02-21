#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include "slow_receiver.h"

/* Change the following line to a higher number if you get bored
 * watching your file transfer. But your code must work with this
 * line.
 */
#define SPEED 1000 /* kbyte/second */

static int  testfile = 0;
static char filename[25];

int slow_receiver( const char* buf, int length )
{
    static int            writtenbytes = 0;
    static struct timeval starttime;
    struct timeval        now;
    double                sec;
    int                   err;

    if( !testfile )
    {
        strcpy( filename, "./inf3190-test-XXXXXX" );
        testfile = mkstemp( filename );
        if( testfile < 0 )
        {
            perror("Error opening output file");
            exit(-1);
        }

        gettimeofday( &starttime, NULL );
    }
    gettimeofday( &now, NULL );
    timersub( &now, &starttime, &now );
    sec = now.tv_sec + (double)now.tv_usec/1000000.0;

    if( writtenbytes < sec * SPEED )
    {
        err = write( testfile, buf, length );
        if( err < 0 )
        {
            perror( "Error writing to output file" );
            switch( errno )
            {
            case EBADF :
            case EINVAL :
            case EFAULT :
            case EPIPE :
            case ENOSPC :
            case EIO :
                exit( -1 );
                break;
            case EAGAIN :
            case EINTR :
            default :
                return 0;
            }
        }
        writtenbytes += length;
        return 1;
    }
    else
    {
        return 0;
    }
}

