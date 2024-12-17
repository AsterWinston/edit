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
void Mouse::renewy(int y){
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

void OperationStack::insert(string str, operate opr, Position pos){
    s1->push(new string(str));
    s2->push(new operate(opr));
    s3->push(new Position(pos));
}
void OperationStack::undo(Text& text){

}
void OperationStack::clear(){
    if(s1){
        while(!s1->empty()){
            delete s1->top();
            s1->pop();
        }
    }
    if(s2){
        while(!s2->empty()){
            delete s2->top();
            s2->pop();
        }
    }
    if(s3){
        while(!s3->empty()){
            delete s3->top();
            s3->pop();
        }
    }

}
OperationStack::~OperationStack(){
    //cout<<"destructing OperationStack"<<endl;
    if(s1){
        while(!s1->empty()){
            delete s1->top();
            s1->pop();
        }
        delete s1;
        s1=nullptr;
    }
    if(s2){
        while(!s2->empty()){
            delete s2->top();
            s2->pop();
        }
        delete s2;
        s2=nullptr;
    }
    if(s3){
        while(!s3->empty()){
            delete s3->top();
            s3->pop();
        }
        delete s3;
        s3=nullptr;
    }
    //cout<<"finish destrction of Opstk"<<endl;
}
Text::~Text(){
    //cout<<"destructing Text"<<endl;
    delete this->v;
    this->v=nullptr;
}

MyWindow::~MyWindow(){

}