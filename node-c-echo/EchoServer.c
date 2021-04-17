#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int server_fd;
    int client_fd;

    struct sockaddr_in server;
    struct sockaddr_in client;

    int len;
    int port = 6807;
    char buffer[1024];

    if (argc == 2) {
        port = atoi(argv[1]);
    }

    // create the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Something went wrong when creating the socket");
        exit(1);
    }

    // configure the server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    len = sizeof(server);

    // bind the server socket to the port
    if(bind(server_fd, (struct sockaddr *)&server, len) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // and listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("Error when listening for a connection.");
        exit(1);
    }

    printf("Listening on port %d...\n", ntohs(server.sin_port));

    for (;;) {
        socklen_t len = sizeof(client);
        puts("Waiting for clients...");

        int client_fd = accept(server_fd, (struct sockaddr *)&client, &len);
        if(client_fd < 0) {
            perror("Error accepting new client");
            close(client_fd);
            close(server_fd);
            exit(1);
        }

        char *client_ip = inet_ntoa(client.sin_addr);
        printf("Accepted a new connection from client %s:%d\n", client_ip, ntohs(client.sin_port));
        memset(buffer, 0, sizeof(buffer));
        int size = read(client_fd, buffer, sizeof(buffer));

        if ( size < 0 ) {
            perror("read error");
            exit(1);
        }
        printf("received %s from client\n", buffer);
        if (write(client_fd, buffer, size) < 0) {
            perror("write error");
            exit(1);
        }
        close(client_fd);
    }

    close(server_fd);
} 