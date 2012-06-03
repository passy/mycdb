#ifndef __SOCKET_HELPER_H__
#define __SOCKET_HELPER_H__

size_t socket_readline(int fd, void *vptr, size_t maxlen);
size_t socket_writeline(int fc, const void *vptr, size_t maxlen);

#endif
