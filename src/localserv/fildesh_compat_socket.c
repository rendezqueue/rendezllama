#include "src/localserv/fildesh_compat_socket.h"

#include <stdio.h>

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#else
/* winsock2.h included via header */
typedef int socklen_t;
#endif

#include <fildesh/fildesh_compat_errno.h>

int fildesh_compat_socket_init(void) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (0 != WSAStartup(MAKEWORD(2,2), &wsa_data)) {
    fildesh_compat_errno_trace();
    return -1;
  }
#endif
  return 0;
}

void fildesh_compat_socket_cleanup(void) {
#ifdef _MSC_VER
  WSACleanup();
#endif
}

int fildesh_compat_socket_ok(FildeshCompat_socket fd) {
#ifndef _MSC_VER
  return fd > 0;
#else
  return fd != INVALID_SOCKET;
#endif
}

void fildesh_compat_socket_close(FildeshCompat_socket fd) {
  if (!fildesh_compat_socket_ok(fd)) {return;}
#ifndef _MSC_VER
  shutdown(fd, SHUT_RDWR);
  close(fd);
#else
  shutdown(fd, SD_BOTH);
  closesocket(fd);
#endif
}

void fildesh_compat_socket_set_timeout(FildeshCompat_socket fd, int seconds) {
#ifdef _MSC_VER
  DWORD timeout = seconds * 1000;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
  struct timeval tv;
  tv.tv_sec = seconds;
  tv.tv_usec = 0;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
}

FildeshCompat_socket
fildesh_compat_socket_setup_server(const char* hostname, int* port)
{
  /* Create a socket. */
  FildeshCompat_socket sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int yes = 1;
  struct sockaddr_in addr;

  (void)hostname;

  if (!fildesh_compat_socket_ok(sockfd)) {
    fildesh_compat_errno_trace();
    return FILDESH_COMPAT_SOCKET_INVALID;
  }

  if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes))) {
    fildesh_compat_errno_trace();
  }

  /* Bind the socket to a port. */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(*port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (0 != bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_INVALID;
  }

  /* Listen for connections. */
  if (0 != listen(sockfd, 5)) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_INVALID;
  }

  if (*port == 0) {
    socklen_t len = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr *)&addr, &len) == -1) {
      fildesh_compat_errno_trace();
      fildesh_compat_socket_close(sockfd);
      return FILDESH_COMPAT_SOCKET_INVALID;
    }
    *port = ntohs(addr.sin_port);
  }

  return sockfd;
}

FildeshCompat_socket fildesh_compat_socket_accept(FildeshCompat_socket fd) {
  return accept(fd, NULL, NULL);
}

int fildesh_compat_socket_recv(FildeshCompat_socket fd, void* buf, size_t len) {
  return (int)recv(fd, (char*)buf, (int)len, 0);
}

int fildesh_compat_socket_send(FildeshCompat_socket fd, const void* buf, size_t len) {
  return (int)send(fd, (const char*)buf, (int)len, 0);
}

int fildesh_compat_socket_recv_error_is_benign(void) {
#ifndef _MSC_VER
  if (errno == EAGAIN || errno == EWOULDBLOCK) { return 1; }
#else
  int err = WSAGetLastError();
  if (err == WSAEWOULDBLOCK || err == WSAETIMEDOUT) { return 1; }
#endif
  return 0;
}
