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
#include<queue>
using std::queue;

enum class Mode{command, insert};

int initEdit(vector<string>& v, Text& text, string& file);
void stringReplace(string& source, const string str1, const string str2);

int getIndexCount(Text& text);
MyWindow getMyWindow(HANDLE& hConsole);

Position fromMouseToConsole(const Mouse& mouse, const MyWindow& myWin, const int& indexCount, const int& startLineNumberInConsole, const vector<int>& vectorLineNumber);
int getLineNumberInText(const int& lineNumberInConsole, const vector<int>& vectorLineNumber);
int getIndexOfNumber(int number);

void initLineInfo(const Text& text, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, const MyWindow& myWin, const int& indexCount, int startLineNumberInText);

void showUI(HANDLE& hConsole, const Text& text, const Mouse& mouse, const MyWindow& myWin, const vector<int>& vectorLineNumber, const vector<int>& vectorLineCount, const string bottomContent, const int& startLineNumberInConsole, const int& indexCount);
void showCursor(HANDLE& hConsole, const Mouse& mouse, const MyWindow& myWin, const vector<int>& vectorLineNumber, const Mode& mode, const string bottomContent, const int& startLineNumberInConsole, const int& indexCount);
void showBottomInfo(HANDLE& hConsole, Mouse& mouse, MyWindow& myWin, string bottomContent);

void setCursorPosition(HANDLE& hConsole, const int x, const int y);
void resetCursor(HANDLE& hConsole);

void captureInput(bool& status, queue<int>& queueInput);
void processWindowChange(bool& status, HANDLE& hConsole, Text& text, MyWindow& myWin, vector<int>* vectorLineNumber, vector<int>* vectorLineCount, Mouse& mouse, Mode& mode, string& bottomContent, int& startLineNumberInConsole, int& indexCount);

int moveUp(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int moveDown(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int moveLeft(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole);
int moveRight(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount,MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int pageUp(Mouse& mouse, vector<int>& vectorLineNumber,MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int pageDown(Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount,MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int home(Mouse& mouse, int& startLineNumberInConsole);
int end(Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int moveToLHeadOfLine(Mouse& mouse, vector<int>& vectorLineNumber, Mode& mode, int& startLineNumberInConsole);
int moveToEndOfLine(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, Mode& mode, string bottomContent, int& startLineNumberInConsole);

int deleteCharHere(Text& text, Mouse& mouse, MyWindow& myWin, Mode& mode, int& indexCount, int& startLineNumberInConsole);
int deleteCharBefore(Text& text, Mouse& mouse, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole);
int pasteAfter(Text& text, Mouse& mouse, string pasteContent);
int pasteBefore(Text& text, Mouse& mouse, string pasteContent, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole);
int charReplace(Text& text, Mouse& mouse, char ch);
int deleteLineHere(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int deleteLineAfter(Text& text, Mouse& mouse, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole);
int deleteLineBefore(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int insertChar(Text& text, Mouse& mouse, char ch, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole);
void enterKey(Text& text, Mouse& mouse, int& startLineNumberInConsole, int& indexCount);

int undoOneStep(Text& text, Mouse& mouse, OperationStack& opstk);
int undoAllStep(Text& text, Mouse& mouse, OperationStack& opstk);

int searchDestination(Text& text, string destination, vector<Position>& searchResult);
int searchNext(vector<Position>& result, Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole);
int searchLast(vector<Position>& result, Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole);
int* buildNext(string dest);
int KMPSearch(vector<char> src, string dest);

void changeMode(char select, Text& text, Mouse& mouse);
void save(Text& text, const string file);

int duplicateLineHere(Text& text, Mouse& mouse, string& pasteContent);
int duplicateLineAfter(Text& text, Mouse& mouse, string& pasteContent);
int duplicateLineBefore(Text& text, Mouse& mouse, string& pasteContent);

int editFile(Text& text, const string file);