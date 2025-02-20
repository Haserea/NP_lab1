#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

SOCKET server_socket = -1;

void free_socket() {
    if (server_socket > 0) {
        closesocket(server_socket);
    }
}

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s -p <port> -q <que_size>\n", exe_name);
}

int start(int argc, char* argv[]) {
    int port = 8080; // DEFAULT_PORT
    int queue_size = 5; // DEFAULT_QUEUE
    if (argc >= 3) {
        char arg_line[128];
        memset(arg_line, 0, sizeof(arg_line));
        for (int i = 1; i < argc; ++i) {
            strcat(arg_line, argv[i]);
            strcat(arg_line, " ");
        }
        int ret = sscanf(arg_line, "-p %d -q %d", &port, &queue_size);
        if (ret < 1) {
            usage(argv[0]);
            return -1;
        }
    }
    return init_server(port, queue_size);
}

int init_server(short port, int queue_size) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock\n");
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Cannot create socket\n");
        WSACleanup();
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Cannot bind socket to port %d\n", port);
        closesocket(server_socket);
        WSACleanup();
        return -2;
    }

    if (listen(server_socket, queue_size) == SOCKET_ERROR) {
        printf("Cannot listen socket on port %d\n", port);
        closesocket(server_socket);
        WSACleanup();
        return -3;
    }

    printf("Server running on port %d\n", port);
    return process_connection();
}

int process_connection() {
    SOCKET client_socket = -1;
    while (1) {
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len);
        if (client_socket == INVALID_SOCKET) {
            printf("Error accepting incoming connection\n");
            continue;
        }

        printf("Connection established from: %s\n", inet_ntoa(client_addr.sin_addr));

        time_t raw_time;
        struct tm* time_info;
        char buffer[1024];

        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

        int ret = send(client_socket, buffer, strlen(buffer), 0);
        if (ret <= 0) {
            printf("Sending data error\n");
        }
        else {
            printf("Sent time: %s\n", buffer);
        }

        closesocket(client_socket);
    }

    WSACleanup();
    return 0;
}