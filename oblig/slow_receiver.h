#ifndef SLOW_NDEMUX_H
#define SLOW_NDEMUX_H

/* This function writes length bytes from buf to a file in the current
 * directory with a random name.
 * The function returns 1 if the bytes contained in buf are written
 * correctly to disk.
 * It returns 0 if it could not write the bytes to disk. This is not
 * necessarily and error. It can also happen because Ndemux has decided
 * to take a break to emulate a very slow receiver.
 *
 * You should not change the interface of Ndemux because it will be
 * the same in later assignments.
 * You should not change the meaning of the return value.
 * You should not remove the writing delays.
 * But you could for example change the implementation to look into
 * 'buf' for commands such as "open file <name>" and "close file".
 */
int slow_receiver( const char *buf, int length );

#endif /* SLOW_NDEMUX_H */

