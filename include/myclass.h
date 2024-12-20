#pragma once
#include<iostream>
using std::cout, std::cin, std::endl;
#include<stack>
using std::stack;
#include<vector>
using std::vector;
#include<string>
using std::string;
class Text;

class Position{
    friend class Mouse;
private:
    int x,y;
public:
    Position():x(0),y(0){}
    Position(int x, int y):x(x),y(y){}
    int getX()const noexcept{return this->x;}
    int getY()const noexcept{return this->y;}
    ~Position();
};

class Mouse{
private:
    Position p;//文本中的位置，y为行数，x为字符数
public:
    Mouse(){}
    void renewPos(int x, int y);
    void renewX(int x);
    void renewY(int y);
    int getX()const noexcept;
    int getY()const noexcept;
    ~Mouse();
};

enum class operate{add, dele, repl};
class oneStepOperation{
public:
    string str;
    operate opr;
    Position pos;
};

class OperationStack{
private:
    stack<oneStepOperation>* stk;
public:
    OperationStack():stk(new stack<oneStepOperation>){}
    void pushBack(string str, operate opr, Position pos);
    void popBack();
    oneStepOperation top();
    void clear();
    bool empty();
    ~OperationStack();
};


class Text{
public:
    vector<vector<char>> *v;
    Text():v(new vector<vector<char>>){}
    ~Text();
};

class MyWindow{
public:
    int wide;
    int height;
    MyWindow():wide(0), height(0){}
    MyWindow(int wide, int height):wide(wide), height(height){}
    ~MyWindow();
};
