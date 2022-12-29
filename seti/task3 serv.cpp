// HTTPServer.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

const char* index_pro = "HTTP-SERVER/index_pro.html";
const char* index = "HTTP-SERVER/index.html";
const char* index_low = "HTTP-SERVER/index_low.html";
const char* not_found_pro = "HTTP-SERVER/not_found_pro.html";
const char* not_found = "HTTP-SERVER/not_found.html";
const char* css = "HTTP-SERVER/style.css";

void initializeLibrary() {
    WSADATA wsaData;
    int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData); // MAKEWORD(2, 2) == 0x0202 - версия 2.2
    if (errorCode != 0) {
        cout << "Error during initialization!" << endl;
        system("pause");
        exit(1);
    }
}

SOCKET createSocket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Создает TCP сокет
    if (sock == INVALID_SOCKET) {
        cout << "Error during socket creation!" << endl;
        system("pause");
        exit(1);
    }
    return sock;
}

void bindSocket(SOCKET sock) {
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* addr = nullptr;
    int errorCode = getaddrinfo(0, "1234", &hints, &addr);
    if (errorCode != 0) {
        cout << "Error during getting address!" << endl;
        system("pause");
        exit(1);
    }

    errorCode = bind(sock, addr->ai_addr, addr->ai_addrlen);
    if (errorCode != 0) {
        freeaddrinfo(addr);
        cout << "Error during binding!" << endl;
        system("pause");
        exit(1);
    }
}

SOCKET acceptConnection(SOCKET listeningSock) {

    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET result = accept(listeningSock, (sockaddr*)&clientAddress, &clientAddressSize);
    // Делаем что то с адресом клиента clientAddress
    // Если не нужно, можно просто сделать accept(listeningSock, NULL, NULL)
    return result;
}

pair <bool, string> readString(SOCKET sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes == 0) return make_pair(true, ""); // Если клиент отсоединился
    else if (bytes < 0) {
        cout << "ERROR" << endl;
        return make_pair(true, "");
    }
    else {
        buffer[bytes] = '\0';
        return make_pair(false, string(buffer));
    }
}

bool writeString(SOCKET sock, string data) {
    int bytes = send(sock, data.c_str(), data.length(), 0);
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

string responseText(string requset) {
    string text = "";
    string response_status = "HTTP/1.1 200 OK\r\n";
    string content_type = "Content-Type: text/html; charset=UTF-8\r\n";
    string fragment;
    ifstream source;

    if (requset.find("GET /") != string::npos) {
        if (requset.find("GET /style.css ") != string::npos) {
            content_type = "Content-Type: text/css;\r\n";
            source.open(css);
        }
        else if (requset.find("GET / ") != string::npos) {
            if (requset.find("User-Agent:") != string::npos)
                source.open(index_pro);
            else
                source.open(index);
        }
        else {
            if (requset.find("User-Agent:") != string::npos)
                source.open(not_found_pro);
            else
                source.open(not_found);
            response_status = "HTTP/1.1 404 Not Found\r\n";
        }
    }
    else { //telnet
        source.open(index_low);
    }

    while (getline(source, fragment)) {
        text += fragment;
        text += '\n';
    }
    source.close();

    return response_status +
        "Host: mysite.com\r\n"
        + content_type +
        "Connection: close\r\n"
        "Content-Length: " + to_string(text.length()) + "\r\n"
        "\r\n" + text;
}


int main()
{
    initializeLibrary();
    SOCKET listeningSock = createSocket();
    bindSocket(listeningSock);

    listen(listeningSock, SOMAXCONN);
    cout << "Listening..." << endl;
    while (true) {
        SOCKET clientSock = acceptConnection(listeningSock);
        cout << "New connection!" << endl;

        while (true) {
            pair<bool, string> request = readString(clientSock);
            if (request.first) break;
            cout << request.second << endl;
            string response = responseText(request.second);
            bool disconnected = writeString(clientSock, response);
            if (disconnected) break;
        }
        closesocket(clientSock);
        cout << "Connection closed" << endl;
    }
}