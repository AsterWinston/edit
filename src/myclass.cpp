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
    if(!this->stk->empty()&&pos.getY()!=this->stk->top().pos.getY())this->clear();
    this->stk->push({str, opr, pos});
}
void OperationStack::clear(){
    while(!this->stk->empty())stk->pop();
}
void OperationStack::popBack(){
    this->stk->pop();
}
oneStepOperation OperationStack::top(){
    return this->stk->top();
}
bool OperationStack::empty(){
    return this->stk->empty();
}
OperationStack::~OperationStack(){
    //cout<<"destructing OperationStack"<<endl;
    if(this->stk)delete this->stk;
    this->stk=nullptr;
    //cout<<"finish destrction of Opstk"<<endl;
}

Text::~Text(){
    //cout<<"destructing Text"<<endl;
    if(this->v)delete this->v;
    this->v=nullptr;
}

MyWindow::~MyWindow(){
    //cout<<"destructing mywindow"<<endl;
}