#include<iostream>
using std::cout, std::cin, std::endl;
#include"function.h"
#include"myclass.h"
#include<string>
using std::string;
#define MAX_DIR_SIZE 1024
#include<direct.h>//获取调用程序时所在目录
Text text;
string file;
int main(int argc, char* argv[]){

    if(argc==1){
        cout<<"parameter too less!"<<endl;
        return -1;
    }
    else if(argc==2){
        char path[MAX_DIR_SIZE];
        _getcwd(path, MAX_DIR_SIZE);//get current directory
        vector<string> v;
        v.push_back(string(path));
        v.push_back(string(argv[1]));
        if(initEdit(v))return -2;
    }
    else{
        cout<<"parameters too much!"<<endl;
        return -1;
    }
    editFile();
    return 0;
}

