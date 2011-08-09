#ifndef _OLIO_SOCKET_H_
#define _OLIO_SOCKET_H_

typedef struct _olio_socket {
  int socket;
  olio_buffer in;
  olio_buffer out;
  int frozen;
  int frozen_readable;
  int listening;
  int connected;
  int write;
} olio_socket;

#endif /* _OLIO_SOCKET_H_ */
