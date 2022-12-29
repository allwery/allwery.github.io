#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;
struct Person
{
char name[25];
int height;
int weight;
} A;
int main()
{
setlocale(LC_ALL, "rus");
string nameR = "REQUEST.bin";
string nameA = "ANSWER.bin";
ofstream f_REQ;
ifstream f_ANS;
long pred_size;
int answer;
while (true)
{
cout << "Введите запрос: Фамилия Рост Вес" << endl;
cin >> A.name >> A.height >> A.weight;
f_REQ.open(nameR, ios::app, ios::binary);
f_REQ.write((char*)&A, sizeof(A)); // запись запроса в файл REQ
f_REQ.close();
f_ANS.open(nameA, ios::binary);
f_ANS.seekg(0, ios::end); // указатель на конец
pred_size = f_ANS.tellg(); // стартовая позиция
while (pred_size >= f_ANS.tellg())
{
Sleep(100);
f_ANS.seekg(0, ios::end);
}
f_ANS.seekg(pred_size, ios::beg); // на начало нового ответа
f_ANS.read((char*)&answer, sizeof(answer)); // считывание ответа
f_ANS.close();
switch (answer) { // его проверка
case 0: {cout << " Недостаток веса\n"; break; }
case 1: {cout << " Hopмa Beca\n"; break; }
case 2: {cout << " Избыток вес\n"; break; }
}
}
}