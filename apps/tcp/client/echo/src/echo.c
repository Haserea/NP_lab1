#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port>\n", exe_name);
}

int start(int argc, char* argv[]) {
    char host[2048] = "";
    int port = 8080;

    if (argc >= 3) {
        char arg_line[4 * 1024] = "";
        for (int i = 1; i < argc; ++i) {
            strcat(arg_line, argv[i]);
            strcat(arg_line, " ");
        }
        int ret = sscanf(arg_line, "-h %s -p %d", host, &port);
        if (ret < 2) {
            usage(argv[0]);
            return -1;
        }
    }
    else {
        printf("Enter server address (-h <host> -p <port>): ");
        int ret = scanf("-h %s -p %d", host, &port);
        if (ret < 2) {
            usage(argv[0]);
            return -2;
        }
    }

    return init_client(host, port);
}

int init_client(const char* host, short port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock\n");
        return -1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Cannot create client socket\n");
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Cannot connect to server %s:%d\n", host, port);
        closesocket(client_socket);
        WSACleanup();
        return -2;
    }

    printf("Successfully connected to server: %s:%d\n", host, port);

    return process_connection(client_socket);
}

int process_connection(SOCKET client_socket) {
    char buffer[1024] = "";

    int ret = recv(client_socket, buffer, sizeof(buffer), 0);
    if (ret <= 0) {
        printf("Receiving data error\n");
        closesocket(client_socket);
        WSACleanup();
        return -1;
    }

    printf("Server time: %s\n", buffer);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}