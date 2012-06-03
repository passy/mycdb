/*
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net

  Implementation of sockets helper functions.

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "socket_helper.h"


//  Read a line from a socket
size_t socket_readline(int sockd, void *vptr, size_t maxlen) {
    size_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {

    if ( (rc = read(sockd, &c, 1)) == 1 ) {
        // NL or CR
        if ( c == '\n' || c == '\r' )
        break;
        *buffer++ = c;
    }
    else if ( rc == 0 ) {
        if ( n == 1 )
        return 0;
        else
        break;
    }
    else {
        if ( errno == EINTR )
        continue;
        return -1;
    }
    }

    *buffer = 0;
    return n;
}


//  Write a line to a socket
size_t socket_writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    size_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
    if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
        if ( errno == EINTR )
        nwritten = 0;
        else
        return -1;
    }
    nleft  -= nwritten;
    buffer += nwritten;
    }

    return n;
}

