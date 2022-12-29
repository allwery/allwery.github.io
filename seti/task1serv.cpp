#include<iostream>
#include<fstream>
#include<string>
#include<Windows.h>

using namespace std;
struct Person {
char name[25];
int height;
int weight;
}B;

int answer;
long size_pred;

void main()
{
setlocale(LC_ALL, "rus");
ifstream fR; // чтение
ofstream fA; //запись
string nameR = "REQUEST.bin"; // файл запросов от программы клиента
string nameA = "ANSWER.bin"; // файл ответов от программы сервера
cout << "server is working" << endl;
fR.open(nameR, ios::binary); // открытие файла
fR.seekg(0, ios::end); // встаёт в конец
size_pred = fR.tellg(); // стартовая позиция сервера в файле
fR.close();
while (true)
{
fR.open(nameR, ios::binary);
fR.seekg(0, ios::end);
while (size_pred >= fR.tellg())
{
Sleep(100);
fR.seekg(0, ios::end);
}
fR.seekg(size_pred, ios::beg);
fR.read((char*)&B, sizeof(B)); // считываем длину с конца
size_pred = fR.tellg(); //возвращение текущей позиции в файле
fR.close();
double IMT = B.weight / (0.01 * B.height) / (0.01 * B.height);
if (18.5 <= IMT && IMT < 25) answer = 1;
if (18.5 > IMT) answer = 0;
if (IMT >= 25)answer = 2;
fA.open(nameA, ios::binary | ios::app);
fA.write((char*)&answer, sizeof(answer));
fA.close();
}
}