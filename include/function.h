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
enum class Mode{command, insert};

int initEdit(vector<string>& v, Text& text);
void stringReplace(string& source, const string str1, const string str2);
void editFile(Text& text);

int getIndexCount(Text& text);
MyWindow getMyWindow(HANDLE& hConsole);
void initLineInfo(HANDLE& hConsole, Text& text ,vector<int>& vectorLineNumber, vector<int>& vectorLineCount, int indexCount, MyWindow& myWin);
//非引用传参导致内存重复释放，浅拷贝
void showUI(HANDLE& hConsole, Text& text, Mouse& mouse, int startLineNumber, MyWindow& myWin, int indexCount, string bottomContent, vector<int>& vectorLinNumber, vector<int>& vectorLineCount);
void showContent(HANDLE& hConsole, Text& text, MyWindow& myWin, int startLineNumber, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, int indexCount, Mouse& mouse, string bottomContent);
void setCursorPosition(HANDLE& hConsole, int x, int y);
int getLineNumber(int lineNumber, vector<int>& vectorLineNumber);
int getIndexOfNumber(int number);
void showCursor(HANDLE& hConsole, Mouse& mouse, Mode& mode, int idnexCount, vector<int>& vectorLineNumber,vector<int>& vectorLineCount, MyWindow& myWin, int startLineNumber);
Position fromMouseToConsole(Mouse& mouse, MyWindow myWin, int indexCount, int startLineNumber, vector<int>& vectorLineNumber);
void resetCursor(HANDLE& hConsole);


void moveUp(Text& text, Mouse& mouse, int& startLineNumber, vector<int>& vectorLineNumber, vector<int>& vectorLineCount);



