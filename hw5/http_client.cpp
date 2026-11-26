//
// Created by konst on 24.11.2026.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

int main() {
    WSADATA data{};
    const int result = WSAStartup(MAKEWORD(2, 2), &data);

    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << '\n';
        return 1;
    }
    std::cout << "WinSock initialized\n";

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* addresses = nullptr;

    const int lookupResult = getaddrinfo("example.com", "80", &hints, &addresses);

    if (lookupResult != 0) {
        std::cerr << "getaddrinfo failed: " << lookupResult << '\n';
        WSACleanup();
        return 1;
    }
    std::cout << "getaddrinfo succeeded\n";
    // std::cout << addresses;

    const SOCKET client_socket = socket(
        addresses->ai_family,
        addresses->ai_socktype,
        addresses->ai_protocol
    );

    if (client_socket == INVALID_SOCKET) {
        const int error = WSAGetLastError();
        std::cerr << "socket failed: " << error << '\n';

        freeaddrinfo(addresses);
        WSACleanup();
        return 1;
    }
    std::cout << "Socket created\n";

    const int connect_result = connect(
        client_socket,
        addresses->ai_addr,
        static_cast<int>(addresses->ai_addrlen)
    );

    if (connect_result == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        std::cerr << "connect failed: " << error << '\n';

        closesocket(client_socket);
        freeaddrinfo(addresses);
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to example.com\n";

    std::string request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    int send_total = 0;
    while (send_total < static_cast<int>(request.size())) {
        const int client_send = send(
            client_socket,
            request.data() + send_total,
            static_cast<int>(request.size()) - send_total,
            0
        );

        if (client_send == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            std::cerr << "send failed: " << error << '\n';

            closesocket(client_socket);
            freeaddrinfo(addresses);
            WSACleanup();
            return 1;
        }
        if (client_send == 0) {
            std::cerr << "send made no progress\n";

            closesocket(client_socket);
            freeaddrinfo(addresses);
            WSACleanup();
            return 2;
        }

        send_total += client_send;
    }
    std::cout << "Number of bytes sent: " << send_total << '\n';


    char buffer[4096];
    int client_recv = recv(client_socket, buffer, 4096, 0);
    while (client_recv > 0) {
        std::cout.write(buffer, client_recv);
        client_recv = recv(client_socket, buffer, 4096, 0);
    }
    if (client_recv == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        std::cerr << "recv failed: " << error << '\n';

        closesocket(client_socket);
        freeaddrinfo(addresses);
        WSACleanup();
        return 1;
    }


    closesocket(client_socket);
    freeaddrinfo(addresses);
    WSACleanup();

    return 0;
}