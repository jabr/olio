#include <sys/types.h>
#include <sys/socket.h>

#include "socket.h"

int olio_socket_create(olio_socket * sock)
{
  sock->socket = socket(PF_INET, SOCK_STREAM|SOCK_DGRAM, 0);
  if (sock->socket < 0)
    return 1;
  if (fcntl(sock->socket, F_SETFL, O_NONBLOCK) != 0)
    return 1;

  SO_KEEPALIVE
SO_REUSEADDR

}

rcv

MSG_NOSIGNAL

MSG_PEEK
