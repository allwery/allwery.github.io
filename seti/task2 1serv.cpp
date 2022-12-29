#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")// подключение библиотеки сокетов
#pragma warning(disable: 4996)// отключение ошибки
using namespace std;
#define PORT 666 // установка порта сервера
#define sHELLO "Hello, STUDENT\n"
int main(int argc, char* argv[]) {
char buff[1024];
cout << "UDP SERVER\n";
//шаг 1 - подключение библиотеки
if (WSAStartup(0x202, (WSADATA*)&buff[0])) //указываем какую версию библиотеки будем использовать
{
// если произошла ошибка то WSAGetLastError вернет последнюю ошибку и мы выведем ее на экран
cout << "WSAStartup error: " << WSAGetLastError();
return -1;
}
//шаг 2 - создание сокета
SOCKET Lsock;
Lsock = socket(AF_INET, SOCK_DGRAM, 0); //SOCK_DGRAM - означает что мы будем пользоваться протоколами на основе
// ДЕЙТАГРАММ то есть UDP
if (Lsock == INVALID_SOCKET) {// если не получилось создать сокет выводим ошибку
cout << "SOCKET() ERROR: " << WSAGetLastError();
WSACleanup();
return -1;
}
//шаг 3 - связывание сокета с локальным адресом
sockaddr_in Laddr; // структура сокета для передачи данных от клиента к серверу
Laddr.sin_family = AF_INET; //формат адреса IPv4
Laddr.sin_addr.s_addr = INADDR_ANY; // или 0 (любой IP адрес) //содержит адрес (номер) узла сети.
Laddr.sin_port = htons(PORT); //содержит порт узла сети.
if (bind(Lsock, (sockaddr*)&Laddr, sizeof(Laddr))) { // связываем сокет со структурой
// если не вышло выводим ошибку связи
cout << "BIND ERROR:" << WSAGetLastError();
closesocket(Lsock);
WSACleanup();
return -1;
}
//шаг 4 обработка пакетов, присланных клиентами
while (1) {
// пока правда принимаем дейтаграммы от клиентов
// в отличие от TCP который ждет пока сервер самостоятельно не закончит сеанс с некоторым клиентом
// UDP разрешает пользователю отправлять сообщения размером 512КБ то есть если удалось установить соединение
// то клиент не занимает весь процесс севера, а только передает сообщение тут же полуает ответ и закрывает сессию
// то есть можно сразу многог клиентов обрабатывать (однако UDP уступает TCP по безопасности и ограниченным размером сообщений)
sockaddr_in Caddr; // структура сокета клиента
int Caddr_size = sizeof(Caddr); //указываем ее размер
int bsize = recvfrom(Lsock, &buff[0], sizeof(buff) - 1, 0, (sockaddr*)&Caddr, &Caddr_size);// полуаем данные от клиента
if (bsize == SOCKET_ERROR)
cout << "RECVFROM() ERROR:" << WSAGetLastError();
//Определяем IP-адрес клиента и прочие атрибуты
HOSTENT* hst; // структура которая содержит данные о хосте (его имя, адрес и т.д)
hst = gethostbyaddr((char*)&Caddr.sin_addr, 4, AF_INET); // получение этой структуры через функцию gethostbyaddr (имя узла, тип, длина адреса)
cout << "NEW DATAGRAM!\n" <<
((hst) ? hst->h_name : "Unknown host") << "/n" << //вывод имени хоста
inet_ntoa(Caddr.sin_addr) << "/n"//приводим адрес хоста в IPv4 вид и выводим его
<< ntohs(Caddr.sin_port) << '\n'; //тоже самое с портом
buff[bsize] = '\0'; // добавление завершающего нуля
cout << "C=>S:" << buff << '\n'; // Вывод на экран
//посылка датаграммы клиенту
sendto(Lsock, &buff[0], bsize, 0, (sockaddr*)&Caddr, sizeof(Caddr));
} return 0;
}