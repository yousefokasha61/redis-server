#include <cstdio>
#include <sys/socket.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int erro = errno;
    fprintf(stderr, "%s\n", msg);
    abort();
}

static void do_something(int connFd) {
    char rBuf[64];
    ssize_t n = read(connFd, rBuf, sizeof(rBuf) - 1);

    if (n < 0) {
        msg("read() error");
        return;
    }

    printf("client says: %s\n", rBuf);

    char wBuf[] = "world";
    write(connFd, wBuf, strlen(wBuf));
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        die("socket() error");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const struct sockaddr *) &addr, sizeof(addr));
    if (rv) {
        die("bind() error");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen() error");
    }

    while (true) {
        struct sockaddr_in clientAddr = {};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int connFd = accept(fd, (struct sockaddr *) &clientAddr, &clientAddrLen);

        if (connFd < 0) {
            continue;
        }

        do_something(connFd);

        close(connFd);
    }


    return 0;
}
