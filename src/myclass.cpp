#include"myclass.h"
Position::~Position(){
    //cout<<"destructing Position"<<endl;
}

void Mouse::renewPos(int x, int y){
    this->p.x=x;
    this->p.y=y;
}
void Mouse::renewX(int x){
    this->p.x=x;
}
void Mouse::renewY(int y){
    this->p.y=y;
}
int Mouse::getX()const noexcept{
    return this->p.x;
}
int Mouse::getY()const noexcept{
    return this->p.y;
}
Mouse::~Mouse(){
    //cout<<"destructing Mouse"<<endl;
}

void OperationStack::pushBack(string str, operate opr, Position pos){
    stk->push({str, opr, pos});
}
void OperationStack::clear(){
    while(!stk->empty())stk->pop();
}
void OperationStack::popBack(){
    stk->pop();
}
oneStepOperation OperationStack::top(){
    return stk->top();
}
bool OperationStack::empty(){
    return stk->empty();
}
OperationStack::~OperationStack(){
    //cout<<"destructing OperationStack"<<endl;
    if(stk)delete stk;
    //cout<<"finish destrction of Opstk"<<endl;
}

Text::~Text(){
    //cout<<"destructing Text"<<endl;
    delete this->v;
    this->v=nullptr;
}

MyWindow::~MyWindow(){

}