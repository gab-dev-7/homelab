#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 4096
#define PORT 8080
#define BUFFER_SIZE 4096

int set_nonblocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1)
    return -1;
  return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
  int server_fd, new_socket, epoll_fd, nfds;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  struct epoll_event ev, events[MAX_EVENTS];
  char buffer[BUFFER_SIZE];

  // HTTP Response with Keep-Alive headers
  char *response = "HTTP/1.1 200 OK\r\n"
                   "Content-Length: 12\r\n"
                   "Connection: keep-alive\r\n"
                   "\r\n"
                   "Hello World!";
  int response_len = strlen(response);

  // Setup Server
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  listen(server_fd, 20000);
  set_nonblocking(server_fd);

  // Setup Epoll
  epoll_fd = epoll_create1(0);
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

  printf("C Server (Keep-Alive) running on %d\n", PORT);

  while (1) {
    nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == server_fd) {
        // Accept as many as possible
        while (1) {
          new_socket = accept(server_fd, (struct sockaddr *)&address,
                              (socklen_t *)&addrlen);
          if (new_socket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            else
              break;
          }
          set_nonblocking(new_socket);
          ev.events = EPOLLIN | EPOLLET;
          ev.data.fd = new_socket;
          epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev);
        }
      } else {
        int fd = events[n].data.fd;
        int bytes_read = read(fd, buffer, BUFFER_SIZE);

        if (bytes_read > 0) {
          // Send response, but don't close
          write(fd, response, response_len);
        } else {
          // Client closed connection -> we close
          close(fd);
        }
      }
    }
  }
  return 0;
}
