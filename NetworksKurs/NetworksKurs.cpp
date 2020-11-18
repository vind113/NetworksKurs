// NetworksKurs.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// A - повідомлення вводиться з клавіатури.
string consoleRead() {
    string input;
    getline(cin, input, ';');
    return input;
}

// B - повідомлення виводиться у файл.
void consoleWrite(string text) {
    std::cout << text;
}

// B - повідомлення зчитується з файлу.
string fileRead(string path) {
    ifstream readFileStream(path);
    string line;
    string result;
    while (getline(readFileStream, line)) {
        result += line;
    }

    return result;
}

// B - повідомлення зчитується з файлу.
void fileWrite(string text, string path) {
    ofstream outputFileStream(path);
    outputFileStream << text;
    outputFileStream.close();
}

int main()
{
    string path = "file.txt";
    fileWrite("Hello from file", path);
    consoleWrite(fileRead(path));
}
