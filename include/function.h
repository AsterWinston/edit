#pragma once
#include<iostream>
//byte定义冲突
using std::cout, std::cin, std::endl;
#include<vector>
using std::vector;
#include<string>
using std::string;
#include"myclass.h"
#include<fstream>
using std::fstream;
#include<filesystem>
#include<math.h>
#include<Windows.h>
#include<conio.h>
#include<thread>
using std::thread;
#include<mutex>
#include<queue>
#include<atomic>
using std::queue;


enum class Mode{command, insert};

int initEdit(vector<string>& v, Text& text, string& file);
void stringReplace(string& source, const string str1, const string str2);
int editFile(Text& text, const string file);

int getIndexCount(Text& text);
MyWindow getMyWindow(HANDLE& hConsole);
int getIndexOfNumber(int number);
int getLineNumberInText(int lineNumberInConsole, vector<int>& vectorLineNumber);
Position fromMouseToConsole(Mouse& mouse, MyWindow myWin, int indexCount, int startLineNumberInConsole, vector<int>& vectorLineNumber);

void initLineInfo(Text& text ,vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, int indexCount, int startLineNumberInText);

//非引用传参导致内存重复释放，浅拷贝
void showUI(HANDLE& hConsole, Text& text, Mouse& mouse, MyWindow& myWin, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, string bottomContent, int startLineNumberInConsole, int indexCount);
void showCursor(HANDLE& hConsole, Mouse& mouse, MyWindow& myWin, vector<int>& vectorLineNumber, Mode& mode, string bottomContent, int startLineNumberInConsole, int indexCount);

void setCursorPosition(HANDLE& hConsole, int x, int y);
void resetCursor(HANDLE& hConsole);

int moveUp(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole);
int moveDown(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole);
int moveLeft(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInCOnsole);
int moveRight(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole);

void save(Text& text, const string file);
void captureInput(bool& status, queue<int>& queueInput);
void processWindowChange(bool& status, HANDLE& hConsole, Text& text, MyWindow& myWin, vector<int>* vectorLineNumber, vector<int>* vectorLineCount, Mouse& mouse, Mode& mode, string& bottomContent, int& startLineNumberInConsole, int& indexCount);