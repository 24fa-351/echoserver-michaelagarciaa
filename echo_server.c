#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

void *handle_client(void *arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char message[1024];
    int str_len;
    printf("Connected to client\n");


    while ((str_len = read(client_sock, message, sizeof(message) - 1)) != 0) {
        message[str_len] = '\0';
        write(client_sock, message, str_len);
    }

    printf("Disconnected from client\n");
    close(client_sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(1);
    }

    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    int port = atoi(argv[2]);


    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        error_handling("socket() error");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);


    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("bind() error");
    }


    if (listen(server_sock, 5) == -1) {
        error_handling("listen() error");
    }

    printf("Multi-threaded echo server listening on port %d\n", port);

    while (1) {
        client_addr_size = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("malloc() error");
            continue;
        }


        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (*client_sock == -1) {
            perror("accept() error");
            free(client_sock);
            continue;
        }


        pthread_t t_id;
        if (pthread_create(&t_id, NULL, handle_client, client_sock) != 0) {
            perror("pthread_create() error");
            close(*client_sock);
            free(client_sock);
        }


        pthread_detach(t_id);
    }

    close(server_sock);
    return 0;
}
