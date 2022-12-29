#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <conio.h>
#include <string>
#include <ctime>
#include <regex>

#pragma warning(disable: 4996) 
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

string message;
string nickName;

const short serverPort = 1234; // Порт сервера
const short clientPort = 1235; // Порт клиента

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
    address.sin_port = clientPort + rand() % 100;
    address.sin_addr.s_addr = 0; // Не заполняет IP адрес
    int errorCode = bind(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error during binding!" << endl;
        system("pause");
        exit(1);
    }
}

void connectSocket(SOCKET sock) {
    hostent* hostentStruct;
    hostentStruct = gethostbyname("localhost");

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = serverPort;

    memcpy(&address.sin_addr.s_addr,
        hostentStruct->h_addr_list[0],
        sizeof(address.sin_addr.s_addr)); // Заполняет address.sin_addr.s_addr из hostentStruct
    int errorCode = connect(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error during connection attempt!" << endl;
        system("pause");
        exit(1);
    }
}

string readResponse(SOCKET sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes == 0) { // Если клиент отсоединился
        cout << "You have been disconnected!" << endl;
        closesocket(sock);
        system("pause");
        exit(0);
    }
    else if (bytes < 0) {
        cout << "You have been disconnected!" << endl;
        closesocket(sock);
        system("pause");
        exit(0);
    }
    else return string(buffer);
}

// Вернет true, если клиент отсоединился
bool writeMessage(SOCKET sock, TEST forMessage) {
    TEST example = forMessage;
    int bytes = send(sock, (char*)&example, sizeof(example), 0);
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

TEST createMessageStruct(string nick, type_of_message current, string private_nick, string message) {
    TEST result;
    strcpy(result.nick, nick.c_str());
    result.current = current;
    strcpy(result.private_nick, private_nick.c_str());
    strcpy(result.message, message.c_str());
    return result;
}

bool accessByNick(SOCKET sock) {
    TEST forConnect = createMessageStruct(nickName, CONNECT, "", "");
    writeMessage(sock, forConnect);

    string accessResponse = readResponse(sock);

    cout << accessResponse << endl;
    if (accessResponse == "Nick has already been taken!") {
        closesocket(sock);
        return false;
    }
    else return true;
}

CRITICAL_SECTION cs;

DWORD WINAPI ThreadWork(LPVOID lpParameter) {
    SOCKET sock = *(SOCKET*)lpParameter;
    while (true) {
        string response = readResponse(sock);
        cout << '\r' << response;
        if (response.length() < message.length())
            for (int i = 0; i < message.length() - response.size(); i++)
                cout << ' ';
        cout << endl << message;
    }
    return 0;
}

void Chatting(SOCKET sock) {
    string nickFormat = nickName + ": ";
    TEST forAccess = createMessageStruct("", ALL, "", nickName + " has been connected!");
    writeMessage(sock, forAccess);
    while (true) {
        TEST forMessage;
        cout << nickFormat;
        message += nickFormat;
        while (true) {
            char symbol_example = getch();
            if (symbol_example == '\r') {
                cout << endl;
                break;
            }
            if (symbol_example == '\b') {
                cout << '\r';
                for (int i = 0; i < message.length(); i++)
                    cout << ' ';
                if (message.size() > nickFormat.size())
                    message.erase(message.length() - 1);
                cout << '\r' << message;
            }
            else {
                cout << symbol_example;
                message += symbol_example;
            }
        }
        if (message.find("Leave this chat!") != std::string::npos) {
            forMessage = createMessageStruct(nickName, DISCONNECT, "", "");
            writeMessage(sock, forMessage);
        }
        else if (message.find("PRIVATE_MESSAGE_TO->") != std::string::npos) {
            string private_nick = "";
            int from = message.find("PRIVATE_MESSAGE_TO->") + 20;
            int temp = from - 20;
            while (message[from] != ' ') {
                private_nick += message[from];
                from++;
            }
            while (message[temp] != ' ') {
                message.erase(temp, 1);
            }
            forMessage = createMessageStruct(nickName, PRIVATE, private_nick, message);
            writeMessage(sock, forMessage);
            message.clear();
        }
        else {
            forMessage = createMessageStruct(nickName, ALL, "", message);
            writeMessage(sock, forMessage);
            message.clear();
        }
    }
}

int main() {
    setlocale(LC_ALL, 0);
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(time(NULL));

    initializeLibrary();
    InitializeCriticalSection(&cs);

    SOCKET sock;

    while (true) {
        while (true) {
            cout << "Input your nick: ";
            getline(cin, nickName);
            regex r("([a-zA-Z0-9]+)");
            if (!regex_match(nickName, r)) {
                cout << "Try again" << endl;
            }
            else break;
        }
        system("cls");

        sock = createSocket();
        bindSocket(sock);
        cout << "Waiting for connection..." << endl;
        connectSocket(sock);

        if (accessByNick(sock))
            break;
    }

    CreateThread(NULL, 0, &ThreadWork, &sock, 0, NULL);
    Chatting(sock);
}