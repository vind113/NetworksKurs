﻿// NetworksKurs.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// -------------------------------------------------------- Введення ----------------------------------------------------------
// A - повідомлення вводиться з клавіатури.
string readConsole() {
    std::cout << "Enter text(ending with new line):\n";
    std::string input;
    std::getline(std::cin, input);
    return input + "\n";
}

// B - повідомлення зчитується з файлу.
string readFile(string path) {
    ifstream readFileStream(path);
    string line;
    string result;
    while (getline(readFileStream, line)) {
        result += line;
    }

    return result;
}

// C - перша частина повідомлення вводиться з клавіатури, а друга частина зчитується з файлу.
string readConsoleAndFile(string path) {
    return readConsole() + readFile(path);
}

// D - частина повідомлення зчитується з одного файлу, а частина з іншого.
string readTwoFiles(string path1, string path2) {
    return readFile(path1) + readFile(path2);
}

// E - перша частина повідомлення зчитується з файлу, а друга частина вводиться з клавіатури.
string readFileAndConsole(string path) {
    return readFile(path) + readConsole();
}

// ------------------------------------------------------- Виведення ---------------------------------------------------------
// A - повідомлення виводиться на екран і у файл.
void consoleAndFileWrite(string text, string path) {
    consoleWrite(text);
    fileWrite(text, path);
}

// B - повідомлення виводиться у файл.
void fileWrite(std::string text, std::string path) {
    std::ofstream outputFileStream;
    outputFileStream.open(path, std::ios_base::app);
    outputFileStream << text;
    outputFileStream.close();
}

// C - повідомлення виводиться на екран.
// D - повідомлення виводиться на екран.
// E - повідомлення виводиться на екран.
void consoleWrite(string text) {
    std::cout << text;
}

int main()
{
    std::cout << "Hello world!";
}
