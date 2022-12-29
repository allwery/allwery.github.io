#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#include <ctime>
#include <regex>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

const short serverPort = 1234; // Порт сервера

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
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = 1234 + rand() % 100;
    address.sin_addr.s_addr = 0; // Не заполняет IP адрес
    int errorCode = bind(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error during binding!" << endl;
        system("pause");
        exit(1);
    }
}

void connectSocket(SOCKET sock, string host, string port) {
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* addr = nullptr;
    int errorCode = getaddrinfo(host.c_str(), port.c_str(), &hints, &addr);


    cout << inet_ntoa(((sockaddr_in*)addr->ai_addr)->sin_addr) << endl;
    cout << ((sockaddr_in*)addr->ai_addr)->sin_port << endl;

    if (errorCode != 0) {
        cout << "Error during getting address!" << endl;
        system("pause");
        exit(1);
    }

    errorCode = connect(sock, addr->ai_addr, addr->ai_addrlen);
    if (errorCode != 0) {
        cout << "Error during connection attempt!" << endl;
        system("pause");
        exit(1);
    }
}

string requestText(string host, string filename) {
    return "GET " + filename + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n\r\n";
}

char buffer[32768];
pair<bool, string> readString(SOCKET sock) {
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
    int bytes = send(sock, data.c_str(), data.length() + 1, 0);
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

int main()
{
    setlocale(LC_ALL, 0);
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(time(NULL));

    initializeLibrary();
    while (true) {
        SOCKET sock = createSocket();
        bindSocket(sock);

        cout << "Address: ";
        string hostName;
        getline(cin, hostName);
        if (hostName == "") break;
        regex r("([0-9a-zA-Z.-]+)(:([0-9]+))?([0-9a-zA-Z/.-]*)");

        smatch parts;
        if (!regex_match(hostName, parts, r)) {
            cout << "Uncorrectable address" << endl;
            continue;
        }
        string host = parts[1];
        string port = parts[3];
        string filename = parts[4];
        if (port == "") port = "80";
        if (filename == "") filename = "/";
        string request = requestText(host, filename);
        cout << request << endl;

        connectSocket(sock, host, port);
        if (writeString(sock, request)) {
            cout << "ERROR" << endl;
            closesocket(sock);
            continue;
        }
        pair<bool, string> response = readString(sock);
        if (response.first) {
            cout << "ERROR" << endl;
            closesocket(sock);
            continue;
        }
        cout << response.second << endl;
        closesocket(sock);
    }
}