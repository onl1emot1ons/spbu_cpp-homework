//
// Created by konst on 25.11.2026.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <array>
#include <exception>
#include <mutex>
#include <system_error>
#include <thread>
#include "site_path.h"


void handle_client(SOCKET client_socket, const std::filesystem::path& root) {
    namespace fs = std::filesystem;
    std::error_code ec;

    char buffer[4096];
    std::string request;
    int server_received = recv(client_socket, buffer, 4096, 0);
    while (server_received > 0) {
        request.append(buffer, server_received);
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
        server_received = recv(client_socket, buffer, 4096, 0);
    }
    if (server_received == 0) {
        std::cerr << "border in recv not found" << '\n';

        return;
    }
    if (server_received == SOCKET_ERROR) {
        std::cerr << "recv failed: " << WSAGetLastError() << '\n';

        return;
    }
    std::cout << "Client request: " << request << '\n';


    const auto line_end = request.find("\r\n");
    const std::string request_line = request.substr(0, line_end);
    std::istringstream line_stream(request_line);
    std::string method, target, version;
    std::string response;

    if (!(line_stream >> method >> target >> version)) {
        std::cerr << "Invalid request line\n";
    }
    else if (method != "GET") {
        std::cerr << "Unsupported method: " << method << '\n';
    }
    else {
        const auto query_start = target.find('?');
        const std::string path = target.substr(0, query_start);

        std::cout << "Requested path: " << path << '\n';

        if (path.empty() || path.front() != '/') {
            std::cerr << "Invalid path: " << path << '\n';
        }
        else {
            const fs::path relative_path = path.substr(1);

            if (relative_path.has_root_path()) {
                std::cerr << "Expected a relative path\n";
            }
            else {
                const fs::path candidate = fs::weakly_canonical(root / relative_path, ec);

                if (ec) {
                    std::cerr << "Cannot resolve path: " << ec.message() << '\n';
                }
                else {
                    const bool inside_root = is_inside_root(root, candidate);

                    if (!inside_root) {
                        std::cerr << "Path outside site root\n";
                    }
                    else if (fs::is_regular_file(candidate, ec)) {
                        std::cout << "File found: " << candidate << '\n';

                        std::ifstream file(candidate, std::ios::binary);

                        if (!file) {
                            std::cerr << "Cannot open file\n";
                        }
                        else {
                            std::string body;
                            char file_buffer[4096];

                            while (file) {
                                file.read(file_buffer, sizeof(file_buffer));
                                const auto count = file.gcount();

                                if (count > 0) {
                                    body.append(file_buffer, static_cast<std::size_t>(count));
                                }
                            }

                            if (file.bad() || !file.eof()) {
                                std::cerr << "Cannot read file\n";
                            }
                            else {
                                std::cout << "File size: " << body.size() << " bytes\n";
                                std::string headers = "HTTP/1.1 200 OK\r\n";
                                headers += "Content-Type: text/html; charset=utf-8\r\n";
                                headers += "Content-Length: " + std::to_string(body.size()) + "\r\n";
                                headers += "Connection: close\r\n\r\n";

                                response = headers + body;
                            }

                        }

                    }
                    else
                    {
                        std::cerr << "File unavailable: " << candidate << '\n';
                        response = "HTTP/1.1 404 Not Found\r\n" "Content-Length: 0\r\n"
                        "Connection: close\r\n" "\r\n";
                    }
                }

            }
        }
    }

    std::size_t sent_total = 0;

    while (sent_total < response.size()) {
        const auto remaining = response.size() - sent_total;
        const int chunk_size = static_cast<int>(
            std::min(remaining, std::size_t{4096})
        );

        const int sent = send(
            client_socket, response.data() + sent_total, chunk_size, 0
        );

        if (sent == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            std::cerr << "send failed: " << error << '\n';
            break;
        }
        if (sent == 0) {
            std::cerr << "send made no progress\n";
            break;
        }

        sent_total += static_cast<std::size_t>(sent);
    }
}

int main(int argc, char* argv[]) {

    namespace fs = std::filesystem;

    if (argc < 2) {
        std::cerr << "Specify the site directory\n";
        return 1;
    }

    std::error_code ec;
    const fs::path root = fs::canonical(argv[1], ec);
    if (ec || !fs::is_directory(root, ec)) {
        std::cerr << "Invalid site directory\n";
        return 1;
    }


    WSADATA data{};
    const int result = WSAStartup(MAKEWORD(2, 2), &data);

    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << '\n';
        return 1;
    }
    std::cout << "WinSock initialized\n";

    const SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        const int error = WSAGetLastError();
        std::cerr << "socket failed: " << error << '\n';

        WSACleanup();
        return 1;
    }
    std::cout << "Socket created\n";

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(80);
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int server_bind = bind(server_socket,
        reinterpret_cast<const sockaddr*>(&server_address),
        static_cast<int>(sizeof(server_address))
    );

    if (server_bind == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << '\n';

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }


    int server_listen = listen(server_socket, SOMAXCONN);
    if (server_listen == 0) {
        std::cout << "listening on: " << inet_ntoa(server_address.sin_addr)
          << ':' << ntohs(server_address.sin_port) << '\n';
    }
    else if (server_listen == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << '\n';

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::mutex accept_mutex;
    bool accepting = true;

    const auto serve_clients = [&] {
        while (true) {
            SOCKET client_socket;
            {
                std::lock_guard<std::mutex> lock(accept_mutex);
                if (!accepting) {
                    return;
                }

                client_socket = accept(server_socket, nullptr, nullptr);
                if (client_socket == INVALID_SOCKET) {
                    const int error = WSAGetLastError();
                    if (error == WSAECONNRESET) {
                        continue;
                    }

                    std::cerr << "accept failed: " << error << '\n';
                    accepting = false;
                    return;
                }
            }

            std::cout << "Client connected\n";
            try {
                handle_client(client_socket, root);
            }
            catch (const std::exception& error) {
                std::cerr << "Client handling failed: " << error.what() << '\n';
            }
            closesocket(client_socket);
        }
    };

    std::array<std::thread, 3> workers;
    for (auto& worker : workers) {
        try {
            worker = std::thread(serve_clients);
        }
        catch (const std::system_error& error) {
            std::cerr << "Cannot start worker: " << error.what() << '\n';
            break;
        }
    }

    serve_clients();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 1;
}
