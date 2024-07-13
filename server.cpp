#include <cstdio>
#include <sys/socket.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <cassert>

const size_t k_max_msg = 4096;

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv < 0) {
            return -1;
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv < 0) {
            return -1;
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }
    return 0;
}

static int32_t one_request(int connFd) {
    char rBuff[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(connFd, rBuff, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rBuff, 4);
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    err = read_full(connFd, &rBuff[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    rBuff[4 + len] = '\0';
    printf("client says: %s\n", &rBuff[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char wBuf[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(wBuf, &len, 4);
    memcpy(&wBuf[4], reply, len);
    return write_all(connFd, wBuf, 4 + len);


}

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    abort();
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

        while (true) {
            int32_t err = one_request(connFd);
            if (err) {
                break;
            }
        }
        close(connFd);
    }


    return 0;
}
