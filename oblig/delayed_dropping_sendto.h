#ifndef DELAYED_DROPPING_SENDTO_H
#define DELAYED_DROPPING_SENDTO_H

#include <netdb.h>
#include <sys/types.h>

/* for comments see .c file */

extern ssize_t delayed_dropping_sendto(int s, const void *msg, size_t len,
                                       int flags,
                                       const struct sockaddr *to, socklen_t tolen);

#endif /* DELAYED_SENDTO_H */
