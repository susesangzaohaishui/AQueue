

#include "ip.hpp"
#include "err.hpp"
#include "platform.hpp"

#if defined AQUEUE_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#if defined AQUEUE_HAVE_OPENVMS
#include <ioctl.h>
#endif

aqueue::fd_t aqueue::open_socket (int domain_, int type_, int protocol_)
{
    //  Setting this option result in sane behaviour when exec() functions
    //  are used. Old sockets are closed and don't block TCP ports etc.
#if defined AQUEUE_HAVE_SOCK_CLOEXEC
    type_ |= SOCK_CLOEXEC;
#endif

    fd_t s = socket (domain_, type_, protocol_);
#ifdef AQUEUE_HAVE_WINDOWS
    if (s == INVALID_SOCKET)
        return INVALID_SOCKET;
#else
    if (s == -1)
        return -1;
#endif

    //  If there's no SOCK_CLOEXEC, let's try the second best option. Note that
    //  race condition can cause socket not to be closed (if fork happens
    //  between socket creation and this point).
#if !defined AQUEUE_HAVE_SOCK_CLOEXEC && defined FD_CLOEXEC
    int rc = fcntl (s, F_SETFD, FD_CLOEXEC);
    errno_assert (rc != -1);
#endif

    //  On Windows, preventing sockets to be inherited by child processes.
#if defined AQUEUE_HAVE_WINDOWS && defined HANDLE_FLAG_INHERIT
    BOOL brc = SetHandleInformation ((HANDLE) s, HANDLE_FLAG_INHERIT, 0);
    win_assert (brc);
#endif

    return s;
}

void aqueue::unblock_socket (fd_t s_)
{
#ifdef AQUEUE_HAVE_WINDOWS
    u_long nonblock = 1;
    int rc = ioctlsocket (s_, FIONBIO, &nonblock);
    wsa_assert (rc != SOCKET_ERROR);
#elif AQUEUE_HAVE_OPENVMS
    int nonblock = 1;
    int rc = ioctl (s_, FIONBIO, &nonblock);
    errno_assert (rc != -1);
#else
    int flags = fcntl (s_, F_GETFL, 0);
    if (flags == -1)
        flags = 0;
    int rc = fcntl (s_, F_SETFL, flags | O_NONBLOCK);
    errno_assert (rc != -1);
#endif
}

void aqueue::enable_ipv4_mapping (fd_t s_)
{
#ifdef IPV6_V6ONLY
#ifdef AQUEUE_HAVE_WINDOWS
    DWORD flag = 0;
#else
    int flag = 0;
#endif
    int rc = setsockopt (s_, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) &flag,
        sizeof (flag));
#ifdef AQUEUE_HAVE_WINDOWS
    wsa_assert (rc != SOCKET_ERROR);
#else
    errno_assert (rc == 0);
#endif
#endif
}
