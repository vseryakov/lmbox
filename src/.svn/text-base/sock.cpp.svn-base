/*
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 *   
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 */

#include "lmbox.h"

int sock_connect(struct sockaddr_in *server, int timeout)
{
   int fd;

   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
       systemObject->Log(0, "sock_connect: socket: %s", strerror(errno));
       return -1;
   }
   // Connect to server
   sock_nonblocking(fd);
   if (connect(fd, (struct sockaddr *)server, sizeof(struct sockaddr_in)) < 0) {
       if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
           close(fd);
           systemObject->Log(0, "sock_connect: connect: %s: %s", sock_inet_str(server->sin_addr), strerror(errno));
           return -1;
       }
   }
   // Wait for connect completion
   if (sock_select(fd, 1, timeout) != 1) {
       systemObject->Log(0, "sock_connect: select: %s: timeout %d: %s", sock_inet_str(server->sin_addr), timeout, strerror(errno));
       close(fd);
       return -1;
   }
   Tls *tls = tlsGet();
   tls->buf.pos = 0;
   return fd;
}

// mode: 0 - read, 1 - write
int sock_select(int fd, int mode, int timeout)
{
    int n;
    struct pollfd pfd;

    if (timeout < 0) {
        return 0;
    }
    timeout *= 1000;
    pfd.fd = fd;
    switch (mode) {
    case 0:
        pfd.events = POLLIN;
        break;
    case 1:
        pfd.events = POLLOUT;
        break;
    case 2:
        pfd.events = POLLPRI;
        break;
    default:
        return -1;
        break;
    }
    pfd.revents = 0;
    do {
        n = poll(&pfd, 1, timeout);
    } while (n < 0 && errno == EINTR);
    if (n > 0) {
        return 1;
    }
    return 0;
}

int sock_nonblocking(int fd)
{
#ifndef O_NONBLOCK
    int n = 1;
    return ioctl(fd, FIONBIO, &n);
#else
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) < 0) {
        systemObject->Log(0, "sock_nonblocking: get: %d: %s", fd, strerror(errno));
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        systemObject->Log(0, "sock_nonblocking: set: %d: %s", fd, strerror(errno));
        return -1;
    }
    return 0;
#endif
}

int sock_blocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) < 0) {
        systemObject->Log(0, "sock_blocking: get: %d: %s", fd, strerror(errno));
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags & (~O_NONBLOCK)) < 0) {
        systemObject->Log(0, "sock_blocking: set: %d: %s", fd, strerror(errno));
        return -1;
    }
    return 0;
}

void sock_nodelay(int fd)
{
#ifdef TCP_NODELAY
    int n = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&n, sizeof(n)) < 0) {
        systemObject->Log(0, "sock_nodelay: %d: %s", fd, strerror(errno));
    }
#endif
}

char *sock_inet_str(struct in_addr addr)
{
    union {
        unsigned long l;
        unsigned char b[4];
    } u;
    Tls *tls = tlsGet();
 
    u.l = (unsigned long) addr.s_addr;
    sprintf(tls->nabuf, "%u.%u.%u.%u", u.b[0], u.b[1], u.b[2], u.b[3]);
    return tls->nabuf;
}

int sock_is_ipaddr(const char *addr)
{
   int dot_count = 0, digit_count = 0;
    
   if (!addr) {
       return (0);
   }
   while (*addr && *addr != ' ') {
       if (*addr == '.') {
           dot_count++;
           digit_count = 0;
       } else
       if (!isdigit(*addr))
           dot_count = 5;
       else {
           digit_count++;
           if (digit_count > 3) {
               dot_count = 5;
           }
       }
       addr++;
   }
   return (dot_count == 3 ? 1 : 0);
}

u_long sock_inet_addr(const char *str)
{
   char buf[6], *ptr;
   u_long ipaddr = 0;
   int i, count, octet;

   for (i = 0;i < 4;i++) {
       ptr = buf;
       count = 0;
       *ptr = 0;
       while (*str != '.' && *str && count < 4) {
             if (!isdigit(*str)) {
                 // Last octet may be terminated with non-digit
                 if (i == 3) {
                     break;
                 }
                 return 0;
             }
             *ptr++ = *str++;
             count++;
       }
      if (count >= 4 || count == 0) {
          return 0;
      }
      *ptr = 0;
      octet = atoi(buf);
      if (octet < 0 || octet > 255) {
          return 0;
      }
      str++;
      ipaddr = ipaddr << 8 | octet;
   }
   return htonl(ipaddr);
}

void sock_buffer(int fd, int snd, int size)
{
    if (setsockopt(fd, IPPROTO_TCP, snd ? SO_SNDBUF : SO_RCVBUF, (char *)&size, sizeof(size)) < 0) {
        systemObject->Log(0, "sock_buffer: %d: %s: %d: %s", fd, (snd ? "SO_SNDBUF":"SO_RCVBUF"), size, strerror(errno));
    }
}

int sock_listen_udp(int port)
{
    struct sockaddr_in sa;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        systemObject->Log(0, "sock_listen_udp: %d: socket: %s", port, strerror(errno));
        return -1;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    int n = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n));
    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
        systemObject->Log(0, "sock_listen_udp: %d: bind: %s", port, strerror(errno));
        return -1;
    }
    return fd;
}

int sock_listen_tcp(int port)
{
    struct sockaddr_in sa;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        systemObject->Log(0, "sock_listen_tcp: %d: socket: %s", port, strerror(errno));
        return -1;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    int n = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n));
    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
        systemObject->Log(0, "sock_listen_tcp: %d: bind: %s", port, strerror(errno));
        return -1;
    }
    if (listen(fd, 5) != 0) {
        systemObject->Log(0, "sock_listen_tcp: %d: listen: %s", port, strerror(errno));
        return -1;
    }
    return fd;
}

int sock_write(int fd, const char *buf, int len, int timeout)
{
    int r,count = 0;

    while (len > 0) {
        if (sock_select(fd, 1, timeout) != 1 || (r = write(fd, buf, len)) <= 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            systemObject->Log(0, "sock_write: %d: %s", fd, strerror(errno));
            return -1;
        }
        len -= r;
        count += r;
        buf += r;
    }
    return count;
}

int sock_recv(int fd, char *buf, size_t toread, int timeout)
{
    int nread;

    nread = recv(fd, buf, toread, 0);
    if (nread == -1 && errno == EWOULDBLOCK && sock_select(fd, 0, timeout) == 1) {
        nread = recv(fd, buf, toread, 0);
    }
    return nread;
}

int sock_read(int fd, char *buf, int len, int timeout)
{
    int nread = 0, n;
    Tls *tls = tlsGet();

    while (len > 0) {
        if (tls->buf.pos > 0) {
            /* Copy bytes already in read-ahead buffer. */
            if (tls->buf.pos > len) {
                n = len;
            } else {
                n = tls->buf.pos;
            }
            memcpy(buf, tls->buf.ptr, (unsigned int)n);
            tls->buf.ptr += n;
            tls->buf.pos -= n;
            len -= n;
            buf += n;
            nread += n;
        }
        if (len > 0) {
            /* Attempt to fill the read-ahead buffer. */
            tls->buf.ptr = tls->buf.data;
            tls->buf.pos = sock_recv(fd, tls->buf.data, BUFFER_SIZE, timeout);
            switch (tls->buf.pos) {
             case 0:
                return nread;
             case -1:
                return -1;
            }
        }
    }
    return nread;
}

int sock_read_line(int fd, char *data, int size, int timeout)
{
    char buf[2] = { 0, 0 };
    int len = 0, nread;

    do {
      if ((nread = sock_read(fd, buf, 1, timeout)) == 1) {
          data[len++] = buf[0];
          if (buf[0] == '\n') {
              break;
          }
      }
    } while (len <= size && nread == 1);
    data[len] = 0;
    return (nread > 0 ? len : nread);
}

// Read all data from the socket, dynamically expand the buffer, if
// size is specified, then read up to size bytes, otherwise all
// until end of data
int sock_read_data(int fd, char **data, int size, int timeout)
{
    int len = 0, nread, toread;
    char buf[BUFFER_SIZE + 1], *ptr = 0;

    do {
      toread = BUFFER_SIZE;
      if (size > 0) {
          if (len >= size) {
              break;
          }
          if (size < toread) {
              toread = size;
          }
      }
      if ((nread = sock_read(fd, buf, toread, timeout)) > 0) {
          buf[nread] = 0;
          ptr = (char*)realloc(ptr, len + nread + 1);
          memcpy(ptr + len, buf, nread + 1);
          len += nread;
      }
    } while (nread > 0);
    *data = ptr;
    return len;
}

char *sock_inet_ntoa(unsigned long ip, char *buf)
{
    unsigned long ia = ntohl(ip);
    sprintf(buf,"%ld.%ld.%ld.%ld",ia >> 24,(ia & 0x00ff0000) >> 16,(ia & 0x0000ff00) >> 8,(ia & 0x000000ff));
    return(buf);
}

u_long sock_gethostbyname(const char *host, vector <u_long> *alist)
{
    int hp_errno;
    u_long hp_addr;
    char hp_buf[1024];
    struct hostent *hp = 0, hp_a, *hp_b;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (sock_is_ipaddr(host)) {
        return sock_inet_addr(host);
    }
#if HAVE_GETHOSTBYNAME_R
#if defined(linux)
    if (!gethostbyname_r(host, &hp_a, hp_buf, sizeof(hp_buf), &hp_b, &hp_errno)) {
        hp = &hp_a;
    }
#else
    hp = gethostbyname_r(host, &hp_a, hp_buf, sizeof(hp_buf), &hp_errno);
#endif
#else
    pthread_mutex_lock(&lock);
    hp = gethostbyname(host);
    hp_errno = h_errno;
#endif
    if (hp == NULL) {
        systemObject->Log(0, "sock_gethostbyname: %s: %s", host, hstrerror(hp_errno));
#ifndef HAVE_GETHOSTBYNAME_R
        pthread_mutex_unlock(&lock);
#endif
        return 0;
    }
    hp_addr = *((unsigned int *)hp->h_addr);
    if (alist) {
        for(int i = 0, *addr = 0;*(hp->h_addr_list + i); i++) {
          alist->push_back(*((unsigned int *)(hp->h_addr_list + i)));
        }
    }
#ifndef HAVE_GETHOSTBYNAME_R
    pthread_mutex_unlock(&lock);
#endif
    return hp_addr;
}

int sock_geturl(const char *mrl, int timeout, const char *headers, char **outdata, int *outsize, string *outheaders)
{
    string url = "/";
    struct sockaddr_in addr;
    char line[BUFFER_SIZE + 1];
    char *ptr, *host, *data = 0;
    int fd, status = -1, length = 0, port = 80;

    *outdata = 0;
    *outsize = 0;

    host = tlsStr(1, mrl);
    // Parse http url string
    if (!strncmp(host, "http://", 7)) {
        host += 7;
    }
    // Query url
    if ((ptr = strchr(host, '/'))) {
        *ptr++ = 0;
        url += ptr;
    }
    // Optional port
    if ((ptr = strchr(host, ':'))) {
        *ptr++ = 0;
        port = atoi(ptr);
    }
    if (!*host) {
        return status;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short) port);
    addr.sin_addr.s_addr = sock_gethostbyname(host, 0);
    if (!addr.sin_addr.s_addr) {
        return -1;
    }
    fd = sock_connect(&addr, timeout);
    if (fd == -1) {
        systemObject->Log(0, "sock_geturl: cannot connect to: %s", mrl);
        return -1;
    }
    data = lmbox_printf("GET %s HTTP/1.0\r\nHost: %s:%d\r\n%s\r\n", url.c_str(), host, port, (headers ? headers : ""));
    if (sock_write(fd, (const char*)data, strlen(data), timeout) <= 0) {
        free(data);
        close(fd);
        return -1;
    }
    free(data);
    while (1) {
        if (sock_read_line(fd, line, BUFFER_SIZE, timeout) <= 0) {
            close(fd);
            return -1;
        }
        // End of headers
        if (!strcmp(line, "\r\n")) {
            break;
        }
        // Status line
        if (!strncasecmp(line, "HTTP/1.", 7) && status == -1) {
            char *p = strchr(line, ' ');
            while (p && *p == ' ') p++;
            if (p) {
                status = atoi(p);
            }
        }
        if (!strncasecmp(line, "Content-Length:",15)) {
            length = atoi(line + 15);
        }
        // Put headers into provided list
        if (outheaders) {
            char *p = strchr(line, ':');
            // Regular header
            if (p) {
                *p++ = 0;
                strTrim(p, 0);
                *outheaders += strLower(line);
                *outheaders += " ";
                if (strchr(p, ' ')) {
                    *outheaders += "{";
                    *outheaders += p;
                    *outheaders += "}";
                } else {
                    *outheaders += p;
                }
                *outheaders += " ";
            }
        }
    }
    *outsize = sock_read_data(fd, &data, length, timeout);
    close(fd);
    *outdata = data;
    return status;
}
