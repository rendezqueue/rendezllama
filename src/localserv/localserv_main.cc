#include <csignal>
#include <cstdlib>
#include <iostream>

extern "C" {
#include <fildesh/fildesh_compat_errno.h>
}

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock.h>
#endif

#ifndef _MSC_VER
typedef int fildesh_compat_socket_t;
#define FILDESH_COMPAT_SOCKET_NULL -1
static bool fildesh_compat_socket_ok(fildesh_compat_socket_t sockfd) {
  return sockfd > 0;
}
#else
typedef SOCKET fildesh_compat_socket_t;
#define FILDESH_COMPAT_SOCKET_NULL INVALID_SOCKET
static bool fildesh_compat_socket_ok(fildesh_compat_socket_t sockfd) {
  return sockfd != INVALID_SOCKET;
}
#endif


static inline void fildesh_compat_socket_close(fildesh_compat_socket_t sockfd) {
  if (!fildesh_compat_socket_ok(sockfd)) {return;}
#ifndef _MSC_VER
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
#else
  shutdown(sockfd, SD_BOTH);
  closesocket(sockfd);
#endif
}


static
  fildesh_compat_socket_t
setup_socket(const char* hostname, int port)
{
  // Create a socket.
  fildesh_compat_socket_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (!fildesh_compat_socket_ok(sockfd)) {
    fildesh_compat_errno_trace();
    return FILDESH_COMPAT_SOCKET_NULL;
  }

  int yes = 1;
  if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes))) {
    fildesh_compat_errno_trace();
  }

  // Bind the socket to a port.
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (0 != bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_NULL;
  }

  // Listen for connections.
  if (0 != listen(sockfd, 5)) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_NULL;
  }
  return sockfd;
}

static fildesh_compat_socket_t listening_socket_fd = FILDESH_COMPAT_SOCKET_NULL;

static
  void
exit_signal_fn(int sig)
{
  fildesh_compat_socket_t sockfd = listening_socket_fd;
  listening_socket_fd = -1;
  fildesh_compat_socket_close(sockfd);
}


int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (0 != WSAStartup(MAKEWORD(2,2), &wsa_data)) {
    fildesh_compat_errno_trace();
    return 1;
  }
#endif
  (void) argc;
  (void) argv;
  listening_socket_fd = setup_socket("localhost", 8080);
  signal(SIGINT, exit_signal_fn);

  // Accept connections and handle requests.
  while (fildesh_compat_socket_ok(listening_socket_fd)) {
    fildesh_compat_socket_t fd = accept(listening_socket_fd, NULL, NULL);
    if (!fildesh_compat_socket_ok(fd)) {
      break;
    }

    // Read the request from the client.
    char buf[1024];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    std::cerr << "got " << n << " bytes." << std::endl;
    if (n < 0) {
      fildesh_compat_errno_trace();
      fildesh_compat_socket_close(fd);
      continue;
    }

    n = send(fd, buf, n, 0);
    std::cerr << "sent " << n << " bytes." << std::endl;
    if (n < 0) {
      fildesh_compat_errno_trace();
    }
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(fd);
  }

  fildesh_compat_socket_close(listening_socket_fd);
#ifdef _MSC_VER
  WSACleanup();
#endif
  return 0;
}

