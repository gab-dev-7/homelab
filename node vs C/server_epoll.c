#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define PORT 8080

// Helper to set a socket to non-blocking mode
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

  // HTTP Response
  char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
  int response_len = strlen(response);

  // Create Server Socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    exit(1);
  }

  listen(server_fd, 10000);
  printf("Epoll C Server listening on %d\n", PORT);

  // Set Non-Blocking
  set_nonblocking(server_fd);

  // Create Epoll Instance
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(1);
  }

  // Add Server Socket to Epoll
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
    perror("epoll_ctl: listen_sock");
    exit(1);
  }

  // Event Loop
  while (1) {
    nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == server_fd) {
        new_socket = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen);
        if (new_socket == -1)
          continue;

        set_nonblocking(new_socket);

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = new_socket;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev);

      } else {
        // In a real server, we'd read() the HTTP request first.
        // For this benchmark, itlll just blast the response back immediately.

        // dummy read to clear the buffer
        char buffer[1024];
        read(events[n].data.fd, buffer, 1024);

        write(events[n].data.fd, response, response_len);
        close(events[n].data.fd);
      }
    }
  }
  return 0;
}
