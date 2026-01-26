#ifndef FILDESH_COMPAT_SOCKET_H_
#define FILDESH_COMPAT_SOCKET_H_

#include <fildesh/fildesh.h>

#ifdef _MSC_VER
#include <winsock2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER
typedef int FildeshCompat_socket;
#define FILDESH_COMPAT_SOCKET_INVALID -1
#else
typedef SOCKET FildeshCompat_socket;
#define FILDESH_COMPAT_SOCKET_INVALID INVALID_SOCKET
#endif

int fildesh_compat_socket_init(void);
void fildesh_compat_socket_cleanup(void);

int fildesh_compat_socket_ok(FildeshCompat_socket fd);
void fildesh_compat_socket_close(FildeshCompat_socket fd);

FildeshCompat_socket fildesh_compat_socket_setup_server(const char* hostname, int* port);
FildeshCompat_socket fildesh_compat_socket_accept(FildeshCompat_socket fd);

void fildesh_compat_socket_set_timeout(FildeshCompat_socket fd, int seconds);

int fildesh_compat_socket_recv(FildeshCompat_socket fd, void* buf, size_t len);
int fildesh_compat_socket_send(FildeshCompat_socket fd, const void* buf, size_t len);

int fildesh_compat_socket_recv_error_is_benign(void);

#ifdef __cplusplus
}
#endif

#endif
