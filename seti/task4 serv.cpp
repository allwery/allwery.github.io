#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <algorithm>

#pragma warning(disable: 4996) 
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

CRITICAL_SECTION cs;
vector <pair<string, SOCKET>> sockStore(0);

const short serverPort = 1234; // Порт сервера

enum type_of_message {
    CONNECT,
    DISCONNECT,
    PRIVATE,
    ALL
};

struct TEST {
    char nick[1024];
    type_of_message current;
    char private_nick[1024];
    char message[1024];
};

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
    address.sin_port = serverPort;
    address.sin_addr.s_addr = 0; // Не заполняет IP адрес
    int errorCode = bind(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
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
    HOSTENT* hst;
    hst = gethostbyaddr((char*)&clientAddress.sin_addr, 4, AF_INET);
    cout << "New connection: ";
    cout << ((hst) ? hst->h_name : "Unknown host") << "/" << inet_ntoa(clientAddress.sin_addr) << "/" << ntohs(clientAddress.sin_port) << '\n';
    return result;
}

TEST clearTEST(TEST example) {
    example.nick[0] = '\n';
    example.current = DISCONNECT;
    example.private_nick[0] = '\n';
    example.message[0] = '\n';
    return example;
}

TEST readRequest(SOCKET sock) {
    TEST buffer;
    int bytes = recv(sock, (char*)&buffer, sizeof(buffer), 0);
    if (bytes == 0) { // Если клиент отсоединился
        buffer = clearTEST(buffer);
        return buffer;
    }
    else if (bytes < 0) {
        cout << "ERROR" << endl;
        buffer = clearTEST(buffer);
        return buffer;
    }
    else return buffer;
}

bool alreadyExist(string nick) {
    for (int i = 0; i < sockStore.size(); i++) {
        if (sockStore[i].first == nick)
            return true;
    }
    return false;
}

// Вернет true, если клиент отсоединился
bool writeResponse(SOCKET sock, string str) {
    int bytes = send(sock, str.c_str(), str.length() + 1, 0);
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

DWORD WINAPI ThreadWork(LPVOID lpParameter) {
    SOCKET clientSock = *(SOCKET*)lpParameter;
    while (true) {
        TEST request = readRequest(clientSock);
        EnterCriticalSection(&cs);
        if (request.current == CONNECT) {
            if (alreadyExist(string(request.nick))) {
                writeResponse(clientSock, "Nick has already been taken!");
                closesocket(clientSock);
                LeaveCriticalSection(&cs);
                break;
            }
            else {
                string list_of_members = "";
                if (!sockStore.empty()) {
                    list_of_members += "\nList of chat members:\n";
                    for (pair<string, SOCKET> example : sockStore) {
                        list_of_members += example.first;
                        list_of_members += ";\n";
                    }
                }
                writeResponse(clientSock, "You have been connected!" + list_of_members);
                sockStore.push_back(make_pair(string(request.nick), clientSock));
            }
        }
        else if (request.current == DISCONNECT) {
            closesocket(clientSock);
            sockStore.erase(std::find(sockStore.begin(), sockStore.end(), make_pair(string(request.nick), clientSock)));
            for (int i = 0; i < sockStore.size(); i++) {
                writeResponse(sockStore[i].second, string(request.nick) + " has been disconnected");
            }
            LeaveCriticalSection(&cs);
            break;
        }
        else if (request.current == ALL) {
            for (int i = 0; i < sockStore.size(); i++) {
                if (clientSock != sockStore[i].second)
                    writeResponse(sockStore[i].second, string(request.message));
            }
        }
        else if (request.current == PRIVATE) {
            for (int i = 0; i < sockStore.size(); i++) {
                if (sockStore[i].first == string(request.private_nick))
                    writeResponse(sockStore[i].second, "PRIVATE_MESSAGE_FROM " + string(request.message));
            }
        }
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int main() {

    setlocale(LC_ALL, 0);
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    initializeLibrary();
    InitializeCriticalSection(&cs);

    SOCKET listeningSock = createSocket();
    bindSocket(listeningSock);

    listen(listeningSock, SOMAXCONN);
    cout << "Listening..." << endl;
    while (true) {
        SOCKET clientSock = acceptConnection(listeningSock);
        CreateThread(NULL, 0, &ThreadWork, &clientSock, 0, NULL);
    }
}