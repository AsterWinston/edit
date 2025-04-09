#include"function.h"
//
#include<mutex>
#include<string>
#include<atomic>

std::mutex mtx;

extern Text text;
extern string file;
//
HANDLE hConsole;//窗口输出句柄
//
int startLineNumberInConsole;//开始输出的行数
//
Mouse mouse;//鼠标在文本中的位置
//
OperationStack opstk;//操作栈
//
Mode mode;//模式
//
vector<int> vectorLineNumber , vectorLineCount;//记录文本在控制台窗口下行号和行数信息
//
string bottomContent="";//底部显示的信息
//子线程自动更新
MyWindow myWin;//记录窗口大小信息
//
int indexCount;//记录文本最大行数的位数
//
string commandInput;//记录命令
//子线程自动更新
queue<int> queueInput;//记录输入
//
//vector<Position>* searchResult;//记录查找结果
std::unique_ptr<vector<Position>> searchResult;
//
// bool LastCharIsspecial = 0;//记录是遇到0或者224但是queueInput为空，即特殊键输入还没有完全入队
std::atomic<bool> getInputStatus = true, handleWinVarStatus = true;
bool runStatus = true;
//
int returnFlag;//记录函数返回值
//
int input;//输入
//
bool contentChangeFlag=0;
//
string pasteContent = ""; //复制的内容
//
string destination = ""; 
//
int lineNumber;

int initEdit(vector<string>& v){
    stringReplace(v[0], "\\", "\\\\");
    stringReplace(v[1], "\\", "\\\\");
    stringReplace(v[1], "//", "\\\\");
    stringReplace(v[1], "/", "\\\\");
    string myFile;
    int temp = v[1].find(':');
    if(temp >= 0 && temp < v[1].length())myFile = v[1];
    else myFile=v[0] + "\\\\" + v[1];
    string directory(myFile.begin(), myFile.begin() + myFile.rfind("\\\\"));
    std::filesystem::path dirpath = directory;
    if(!std::filesystem::exists(dirpath)){
        std::filesystem::create_directories(dirpath);
    }
    std::ifstream ifs;
    ifs.open(myFile, std::ios::in);
    if(!ifs.is_open()){
        std::ofstream ofs;
        ofs.open(myFile, std::ios::out);
        if(!ofs.is_open()){
            //极大可能是文件和目录同名，导致无法创建
            cout<<"fail to open file!"<<endl;
            return -1;
        }
        else {
            //创建文件成功
            file = myFile;
            text.v->push_back(vector<char> {});
            ofs.close();
            return 0;
        }
    }
    else {
        //打开文件成功
        file = myFile;
        string line;
        while(getline(ifs, line)){
            vector<char>line_v(line.begin(), line.end());
            text.v->push_back(line_v);
        }
        ifs.close();
        //文件没有内容
        if(!text.v->size())text.v->push_back(vector<char>{});
        return 0;
    }
}
void stringReplace(string& source, const string str1, const string str2){
    int pos=0;
    while(1){
        pos = source.find(str1, pos);
        if(pos < 0 || pos >= source.length())break;
        source.replace(pos, str1.length() ,str2);
        pos+=str2.length();
    }
}
//
int getIndexCount(){
    int line_count = text.v->size();
    int index_count = 1;
    while(line_count = line_count/10)index_count++;
    return index_count;
}
MyWindow getMyWindow(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return {csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
}
//从0开始的坐标
Position fromMouseToConsole(){
    return {mouse.getX()%(myWin.wide - indexCount - 1) + indexCount + 1, vectorLineNumber[mouse.getY()] - startLineNumberInConsole + mouse.getX()/(myWin.wide - indexCount - 1)};
}
int getLineNumberInText(const int lineNumberInConsole){
    int low = 0, high = vectorLineNumber.size() - 1, mid;
    //没有=就炸了
    while(low <= high){
        mid = (low+high)/2;
        if(vectorLineNumber[mid] == lineNumberInConsole)return mid + 1;
        else if(lineNumberInConsole > vectorLineNumber[mid])low = mid + 1;
        else high = mid - 1;
    }
    return 0;
}
int getIndexOfNumber(int number){
    int temp = 1;
    while(number = number/10)temp++;
    return temp;
}
//
void initLineInfo(int startLineNumberInText){
    vectorLineCount.resize((*text.v).size());
    vectorLineNumber.resize((*text.v).size()+1);
    vectorLineNumber[0]=1;
    for(int i=startLineNumberInText;i<(*text.v).size();i++){
        vectorLineCount[i] = (*text.v)[i].size()>0?((*text.v)[i].size() + myWin.wide - indexCount - 2)/(myWin.wide - indexCount - 1):1;
        vectorLineNumber[i+1] = vectorLineNumber[i] + vectorLineCount[i];
    }
    vectorLineNumber.pop_back();
}
//
void showUI(){
    //隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    //信息栏
    int lineNumberInText = 0;
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);//蓝色字体
    for(int i = startLineNumberInConsole; i < myWin.height + startLineNumberInConsole - bottomLineCount && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(0, i - startLineNumberInConsole);
        if((lineNumberInText = getLineNumberInText(i))){
            for(int j=0;j<indexCount - getIndexOfNumber(lineNumberInText);j++)cout<<' ';
            cout<<lineNumberInText;
            cout<<' ';
        }
        else {
            for(int j=0;j<indexCount + 1;j++){
                cout<<' '; 
            }
        }
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    //内容
    int tempLineNumber = startLineNumberInConsole, count = 0;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--)))count++;
    for(int i = startLineNumberInConsole; i < myWin.height + startLineNumberInConsole - bottomLineCount && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(indexCount + 1, i - startLineNumberInConsole);
        if(count < vectorLineCount[lineNumberInText-1]){
            for(int j=count*(myWin.wide - indexCount - 1); j<(count+1)*(myWin.wide - indexCount - 1) && j<(*text.v)[lineNumberInText-1].size(); j++){
                cout<<(*text.v)[lineNumberInText-1][j];
            }
            count++;
        }
        else {
            count = 1;
            lineNumberInText = getLineNumberInText(i);
            for(int j = 0; j<myWin.wide - indexCount - 1 && j<(*text.v)[lineNumberInText-1].size(); j++){
                cout<<(*text.v)[lineNumberInText-1][j];
            }
        }
    }
}
void showCursor(){
    //光标
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    Position mousePositionInConsole = fromMouseToConsole();
    if(mousePositionInConsole.getY()>myWin.height-1-bottomLineCount){
        return;
    }
    else{
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        setCursorPosition(mousePositionInConsole.getX(), mousePositionInConsole.getY());
        if(mode == Mode::command){
            cout << "\x1b[4 q";
        }
        else{ 
	        cout << "\x1b[0 q";
        }
    }
}
void showBottomInfo(){
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    //清空底部
    setCursorPosition(0, myWin.height - bottomLineCount);
    for(int i=0;i<myWin.wide*bottomLineCount;i++)cout<<' ';
    //输出底部信息
    setCursorPosition(0, myWin.height - bottomLineCount);
    cout<<bottomContent;
    setCursorPosition(myWin.wide - (getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + 1), myWin.height - 1);
    cout<<mouse.getY()+1<<','<<mouse.getX()+1;
}
//
void setCursorPosition(const int x, const int y){
    COORD cursorPosition;
    cursorPosition.X = x;
    cursorPosition.Y = y;
    SetConsoleCursorPosition(hConsole, cursorPosition);
}
void resetCursor(){
        setCursorPosition(0, 0);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        cout << "\x1b[0 q";
}
//
void captureInput(){
    int temp;
    while(getInputStatus){
        Sleep(10);
        if(_kbhit()){
            std::lock_guard<std::mutex> lock(mtx);
            temp=_getch();
            queueInput.push(temp);
            if(temp==0||temp==224){
                temp=_getch();
                queueInput.push(temp);
            }
            //记录特殊键的输入
        }
    }
}
void processWindowChange(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;//获取窗口大小信息
    while(handleWinVarStatus){
        Sleep(10);
        if(GetConsoleScreenBufferInfo(hConsole, &csbi)){
            if(myWin.wide != csbi.srWindow.Right - csbi.srWindow.Left + 1 || myWin.height != csbi.srWindow.Bottom - csbi.srWindow.Top + 1){
                myWin = getMyWindow();
                initLineInfo(0);
                clear();
                showUI();
                showBottomInfo();
                showCursor();
            }
        }
    }
}
//
int moveUp(){
    if(mouse.getY() == 0){
        return 0;
    }
    mouse.renewY(mouse.getY()-1);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    switch(mode){
        case Mode::command:{
            if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
            break;
        }
        case Mode::insert:{
            if(mouse.getX()>(*text.v)[mouse.getY()].size()||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size());
            break;
        }
    }
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int moveDown(){
    if(mouse.getY()==(*text.v).size()-1){
        return 0;
    }
    mouse.renewY(mouse.getY()+1);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    switch(mode){
        case Mode::command:{
            if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
            break;
        }
        case Mode::insert:{
            if(mouse.getX()>(*text.v)[mouse.getY()].size()||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size());
            break;
        }
    }
    if(posInConsole.getY()>myWin.height-1-bottomLineCount-gapSize){
        if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount)return 2;
        else {
            startLineNumberInConsole=startLineNumberInConsole+(posInConsole.getY()-(myWin.height-bottomLineCount-1-gapSize))<vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount)?startLineNumberInConsole+(posInConsole.getY()-(myWin.height-bottomLineCount-1-gapSize)):vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
        }
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int moveLeft(){
    int startLineNumberRecord = startLineNumberInConsole;
    if(mouse.getX()==0){
        return 0;
    }
    mouse.renewX(mouse.getX()-1);
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-1>1?startLineNumberInConsole-1:1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int moveRight(){
    int startLineNumberRecord = startLineNumberInConsole;
    switch(mode){
        case Mode::command:{
            if((*text.v)[mouse.getY()].size()==0||mouse.getX()==(*text.v)[mouse.getY()].size()-1){
                return 0;
            }
            break;
        }
        case Mode::insert:{
            if((*text.v)[mouse.getY()].size()==0||mouse.getX()==(*text.v)[mouse.getY()].size()){
                return 0;
            }
            break;
        }
    }
    mouse.renewX(mouse.getX()+1);
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(posInConsole.getY()>myWin.height-1-bottomLineCount-gapSize){
        if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount)return 2;
        else{
            startLineNumberInConsole=startLineNumberInConsole+1<vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount)?startLineNumberInConsole+1:vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
        }
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int pageUp(){
    if(startLineNumberInConsole == 1){
        return 0;
    }
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    startLineNumberInConsole = startLineNumberInConsole-(myWin.height-bottomLineCount)>0?startLineNumberInConsole-(myWin.height-bottomLineCount):1;
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int pageDown(){
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount){
        return 0;
    }
    startLineNumberInConsole = startLineNumberInConsole+myWin.height-bottomLineCount<vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]?startLineNumberInConsole+myWin.height-bottomLineCount:vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int home(){
    if(startLineNumberInConsole == 1){
        return 0;
    }
    else{
        startLineNumberInConsole = 1;
        mouse.renewPos(0, 0);
        return 1;
    }
}
int end(){
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount){
        return 0;
    }
    startLineNumberInConsole = vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int moveToLHeadOfLine(){
    if(mouse.getX()==0){
        return 0;
    }
    mouse.renewX(0);
    if(vectorLineNumber[mouse.getY()]<startLineNumberInConsole){
        startLineNumberInConsole = vectorLineNumber[mouse.getY()];
    }
    return 1;
}
int moveToEndOfLine(){
    if(mouse.getX()==(*text.v)[mouse.getY()].size()-1){
        return 0;
    }
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    mouse.renewX((*text.v)[mouse.getY()].size() - 1);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--)))count++;
    if(vectorLineNumber[lineNumberInText-1]+count>startLineNumberInConsole+myWin.height-bottomLineCount){
        startLineNumberInConsole = vectorLineNumber[lineNumberInText-1]+count - myWin.height + bottomLineCount;
    }
    return 1;
}

int deleteCharHere(){
    //在空空白行或者在结尾
    if((*text.v)[mouse.getY()].size()==0||mouse.getX()==(*text.v)[mouse.getY()].size()){
        return 0;
    }
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size()-1;i++){
        (*text.v)[mouse.getY()][i]=(*text.v)[mouse.getY()][i+1];
    }
    if(mode==Mode::command){
        //更新startLineNumber，!优先级大于%
        if(!(mouse.getX()%(myWin.wide-indexCount-1))&&mouse.getX()!=0)startLineNumberInConsole=startLineNumberInConsole>1?startLineNumberInConsole-1:1;
        mouse.renewX(mouse.getX()-1>=0?mouse.getX()-1:0);
    }
    (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()-1);
    return 1;
}
int deleteCharBefore(){
    //光标在开头
    if(mouse.getX()==0){
        if(mouse.getY()==0){
            return 0;
        }
        else{
            //合并行
            mouse.renewPos((*text.v)[mouse.getY()-1].size(), mouse.getY()-1);
            for(int i=0;i<(*text.v)[mouse.getY()+1].size();i++)(*text.v)[mouse.getY()].push_back((*text.v)[mouse.getY()+1][i]);
            for(int i=mouse.getY()+1;i<(*text.v).size()-1;i++){
                (*text.v)[i] = (*text.v)[i+1];
            }
            (*text.v).resize((*text.v).size()-1);
            indexCount = getIndexOfNumber((*text.v).size());
            startLineNumberInConsole = startLineNumberInConsole>1?startLineNumberInConsole-1:1;
            return 2;
        }
    }
    int indexTemp = mouse.getY();
    for(int i=mouse.getX()-1;i<(*text.v)[indexTemp].size()-1;i++){
        (*text.v)[indexTemp][i]=(*text.v)[indexTemp][i+1];
    }
    (*text.v)[indexTemp].resize((*text.v)[indexTemp].size()-1);
    if(!(mouse.getX()%(myWin.wide-indexCount-1)))startLineNumberInConsole=startLineNumberInConsole>1?startLineNumberInConsole-1:1;
    mouse.renewX(mouse.getX()-1);
    return 1;
}
int pasteAfter(){
    if(pasteContent==""){
        return 0;
    }
    if((*text.v)[mouse.getY()].size()!=0){
        vector<char> temp;
        for(int i=0;i<=mouse.getX();i++)temp.push_back((*text.v)[mouse.getY()][i]);
        for(int i=0;i<pasteContent.size();i++)temp.push_back(pasteContent[i]);
        for(int i=mouse.getX()+1;i<(*text.v)[mouse.getY()].size();i++)temp.push_back((*text.v)[mouse.getY()][i]);
        (*text.v)[mouse.getY()]=temp;
        return 1;
    }
    for(int i=0;i<pasteContent.size();i++){
        (*text.v)[mouse.getY()].push_back(pasteContent[i]);
    }
    return 1;
}
int pasteBefore(){
    if(pasteContent==""){
        return 0;
    }
    if((*text.v)[mouse.getY()].size()!=0){
        vector<char> temp;
        for(int i=0;i<mouse.getX();i++)temp.push_back((*text.v)[mouse.getY()][i]);
        for(int i=0;i<pasteContent.size();i++)temp.push_back(pasteContent[i]);
        for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++)temp.push_back((*text.v)[mouse.getY()][i]);
        (*text.v)[mouse.getY()]=temp;
        //更新startLineNumber
        if(pasteContent.size()>myWin.wide-indexCount-2-mouse.getX()%(myWin.wide-indexCount-1)){
            startLineNumberInConsole = startLineNumberInConsole+(pasteContent.size()-(myWin.wide-indexCount-2-mouse.getX()%(myWin.wide-indexCount-1))+myWin.wide-indexCount-2)%(myWin.wide-indexCount-1);
        }
        mouse.renewX(mouse.getX()+pasteContent.size());
        return 1;
    }
    //getX()==0
    for(int i=0;i<pasteContent.size();i++){
        (*text.v)[mouse.getY()].push_back(pasteContent[i]);
    }
    mouse.renewX(pasteContent.size()-1);
    //更新startLineNumber
    startLineNumberInConsole=startLineNumberInConsole+(pasteContent.size()+myWin.wide-indexCount-2)/(myWin.wide-indexCount-1)-1;
    return 1;
}
int charReplace(char ch){
    if((*text.v)[mouse.getY()].size()==0){
        return 0;
    }
    else {
        (*text.v)[mouse.getY()][mouse.getX()]=ch;
        return 1;
    }
}
int deleteLineHere(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    (*text.v)[mouse.getY()].resize(0);
    mouse.renewX(0);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int deleteLineAfter(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    (*text.v)[mouse.getY()].resize(mouse.getX());
    if(!((mouse.getX()+myWin.wide+indexCount-1)%(myWin.wide-indexCount-1)))startLineNumberInConsole=startLineNumberInConsole>1?startLineNumberInConsole-1:1;
    mouse.renewX(mouse.getX()>0?mouse.getX()-1:0);
    return 1;
}
int deleteLineBefore(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size()-1;i++){
        (*text.v)[mouse.getY()][i-mouse.getX()]=(*text.v)[mouse.getY()][i+1];        
    }
    (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()-mouse.getX()-1);
    mouse.renewX(0);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int insertChar(char ch){
    if(ch==9){
        if((*text.v)[mouse.getY()].size()==0){
            for(int i=0;i<4;i++)(*text.v)[mouse.getY()].push_back(' ');
            mouse.renewX(mouse.getX()+4);
            return 0;
        }
        if((mouse.getX()+1)%(myWin.wide-indexCount-1)>(myWin.wide-indexCount-5)||!((mouse.getX()+1)%(myWin.wide-indexCount-1)))startLineNumberInConsole+=1;
        (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()+4);
        for(int i=(*text.v)[mouse.getY()].size()-1;i>mouse.getX()+3;i--){
            (*text.v)[mouse.getY()][i]=(*text.v)[mouse.getY()][i-4];
        }
        for(int i=0;i<4;i++)(*text.v)[mouse.getY()][mouse.getX()+i]=' ';
        mouse.renewX(mouse.getX()+4);
        return 1;
    }
    else{
        if((*text.v)[mouse.getY()].size()==0){
            (*text.v)[mouse.getY()].push_back(ch);
            mouse.renewX(mouse.getX()+1);
            return 0;
        }
        if(!((mouse.getX()+1)%(myWin.wide-indexCount-1)))startLineNumberInConsole+=1;
        (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()+1);
        for(int i=(*text.v)[mouse.getY()].size()-1;i>mouse.getX();i--){
            (*text.v)[mouse.getY()][i]=(*text.v)[mouse.getY()][i-1];
        }
        (*text.v)[mouse.getY()][mouse.getX()]=ch;
        mouse.renewX(mouse.getX()+1);
        return 1;
    }
}
void enterKey(){
    vector<char> temp;
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++){
        temp.push_back((*text.v)[mouse.getY()][i]);
    }
    (*text.v)[mouse.getY()].resize(mouse.getX());
    (*text.v).resize((*text.v).size()+1);
    indexCount = getIndexOfNumber((*text.v).size());
    for(int i=(*text.v).size()-1;i>mouse.getY()+1;i--){
        (*text.v)[i]=(*text.v)[i-1];
    }
    (*text.v)[mouse.getY()+1]=temp;
    mouse.renewPos(0, mouse.getY()+1);
    startLineNumberInConsole+=1;
}

int undoOneStep(){
    if(opstk.empty()){
        return 0;
    }
    oneStepOperation oso=opstk.top();
    opstk.popBack();
    switch(oso.opr){
        case operate::add:
        {
            vector<char> temp;
            for(int i=0;i<oso.pos.getX();i++)temp.push_back((*text.v)[oso.pos.getY()][i]);
            for(int i=0;i<oso.str.size();i++)temp.push_back(oso.str[i]);
            for(int i=oso.pos.getX();i<(*text.v)[oso.pos.getY()].size();i++)temp.push_back((*text.v)[oso.pos.getY()][i]);
            (*text.v)[oso.pos.getY()]=temp;
            mouse.renewPos(oso.pos.getX(), oso.pos.getY());
            break;
        }      
        case operate::dele:
        {
            for(int i=oso.pos.getX();i<(*text.v)[oso.pos.getY()].size()-1;i++)(*text.v)[oso.pos.getY()][i]=(*text.v)[oso.pos.getY()][i+1];
            (*text.v)[oso.pos.getY()].resize((*text.v)[oso.pos.getY()].size()-oso.str.size());
            mouse.renewPos(oso.pos.getX()>0?oso.pos.getX()-1:0, oso.pos.getY());
            break;
        }
        case operate::repl:
            for(int i=0;i<oso.str.size();i++)(*text.v)[oso.pos.getY()][oso.pos.getX()+i]=oso.str[i];
            mouse.renewPos(mouse.getX(), mouse.getY());
            break;
    }
    return 1;
}
int undoAllStep(){
    if(opstk.empty()){
        return 0;
    }
    oneStepOperation oso;
    while(!opstk.empty()){
        oso=opstk.top();
        opstk.popBack();
        switch(oso.opr){
            case operate::add:
            {
                vector<char> temp;
                for(int i=0;i<oso.pos.getX();i++)temp.push_back((*text.v)[oso.pos.getY()][i]);
                for(int i=0;i<oso.str.size();i++)temp.push_back(oso.str[i]);
                for(int i=oso.pos.getX();i<(*text.v)[oso.pos.getY()].size();i++)temp.push_back((*text.v)[oso.pos.getY()][i]);
                (*text.v)[oso.pos.getY()]=temp;
                mouse.renewPos(oso.pos.getX(), oso.pos.getY());
                break;
            }      
            case operate::dele:
            {
                for(int i=oso.pos.getX();i<(*text.v)[oso.pos.getY()].size()-1;i++)(*text.v)[oso.pos.getY()][i]=(*text.v)[oso.pos.getY()][i+1];
                (*text.v)[oso.pos.getY()].resize((*text.v)[oso.pos.getY()].size()-oso.str.size());
                mouse.renewPos(oso.pos.getX()>0?oso.pos.getX()-1:0, oso.pos.getY());
                break;
            }
            case operate::repl:
                for(int i=0;i<oso.str.size();i++)(*text.v)[oso.pos.getY()][oso.pos.getX()+i]=oso.str[i];
                mouse.renewPos(mouse.getX(), mouse.getY());
                break;
        }
    }    
    return 1;
}

int searchDestination(){
    (*searchResult).resize(0);
    int xLast, xNew;
    for(int i=0;i<(*text.v).size();i++){
        xLast=-1;
        vector<char> temp = (*text.v)[i];
        if(!temp.size())continue;
        while(temp.size()>=destination.size() && (xNew=KMPSearch(temp, destination))!=temp.size()){
            ++xLast += xNew;
            (*searchResult).push_back({xLast, i});
            temp.resize(0);
            for(int j=xLast+1;j<(*text.v)[i].size();j++){
                temp.push_back((*text.v)[i][j]);
            }
        }
    }
    if((*searchResult).size())return 1;
    return 0;
}
int searchNext(){
    Position tempPos;
    int flag=0;
    for(auto i:*searchResult){
        if(i.getY()==mouse.getY()&&i.getX()>mouse.getX()||i.getY()>mouse.getY()){
            tempPos=i;
            flag=1;
            break;
        }
    }
    if(!flag)return 0;
    mouse.renewPos(tempPos.getX(), tempPos.getY());

    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()>myWin.height-1-bottomLineCount-gapSize){
        if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount)return 2;
        else {
            startLineNumberInConsole=startLineNumberInConsole+(posInConsole.getY()-(myWin.height-bottomLineCount-1-gapSize))<vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount)?startLineNumberInConsole+(posInConsole.getY()-(myWin.height-bottomLineCount-1-gapSize)):vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
        }
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
    
}
int searchLast(){
    Position tempPos;
    int flag=0;
    for(int i=(*searchResult).size()-1;i>=0;i--){
        if((*searchResult)[i].getY()==mouse.getY()&&(*searchResult)[i].getX()<mouse.getX()||(*searchResult)[i].getY()<mouse.getY()){
            tempPos=(*searchResult)[i];
            flag=1;
            break;
        }
    }
    if(!flag){
        return 0;
    }
    mouse.renewPos(tempPos.getX(),tempPos.getY());
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole();
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int* buildNext(string dest) {
    int len=dest.size();
	int* arr = new int[len];
	arr[0] = 0;//数组的第一个值永远是0
	int prefix_len = 0;//用于记录从字符串开始到当前位置上的最长相同前后缀长度
	int i = 1;//用于遍历字符串，表示遍历到的字符下标，永不回退
	while (i < len) {
		if (dest[prefix_len] == dest[i]) {
			prefix_len++;//如果比较值相等，那么前缀值加一
			arr[i++] = prefix_len;//记录从字符串开始位置到当前位置的最长相同前后缀长度

		}
		else {
			if (prefix_len == 0) {
				arr[i++] = 0;//如果前缀长度为0，且不匹配，则从字符串开头到当前位置的最长前后缀长度一定为0
			}
			else {
				prefix_len = arr[prefix_len - 1];
			}
		}
	}
	return arr;
}
int KMPSearch(vector<char> src, string dest) {
    int len1=src.size(), len2=dest.size();
	int* next = buildNext(dest);
	int i = 0, j = 0;
	while (i < len1) {//i永不回退
		if (src[i] == dest[j]) {
			i++;
			j++;
		}//相同就让两个指针都往后移
		else if (j > 0) {
			j = next[j - 1];
		}
		else {
			i++;
		}//如果srt[i]!=dest[j]，而且j==0，那么代表指向src字符的指针需要后移
		if (j == len2) {//指向dest的指针后移到了dest的结尾
			return i - j;//返回目标dest字符串在源字符串src中的起始位置
		}
	}
	//cout << "can not find string" << endl;
	return len1;//src中不存在dest，返回src的长度作为起始地址代表没有找到
}

void changeMode(char select){
    switch(select){
        case 'i':
            mouse.renewX((*text.v)[mouse.getY()].size()>0?mouse.getX()+1:0);
            break;
        case 'a':
            break;
        case 'I':
            mouse.renewX((*text.v)[mouse.getY()].size());
            break;
        case 'A':
            mouse.renewX(0);
            break;
    }
}
void save(){
    std::ofstream ofs;
    ofs.open(file, std::ios::out);
    if(!ofs.is_open()){
        clear();
        cout<<"fail to save file"<<endl;
        exit(-1);
    }
    for(auto i:(*text.v)){
        for(auto j:i){
            ofs<<j;
        }
        ofs<<'\n';
    }
    ofs.close();
}

int duplicateLineHere(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(auto i:(*text.v)[mouse.getY()]){
        pasteContent.push_back(i);
    }
    return 1;
}
int duplicateLineAfter(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++){
        pasteContent.push_back((*text.v)[mouse.getY()][i]);
    }
    return 1;
}
int duplicateLineBefore(){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(int i=0;i<=mouse.getX();i++){
        pasteContent.push_back((*text.v)[mouse.getY()][i]);
    }
    return 1;
}

int editFile(){
    //关回显
    echoOff();

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    startLineNumberInConsole = 1;
    mode=Mode::command;
    myWin = getMyWindow();
    indexCount = getIndexCount();
    searchResult = std::make_unique<vector<Position>>();
    initLineInfo(0);
    clear();
    showUI();
    showBottomInfo();
    showCursor();
    thread capInput(captureInput);
    thread handleWinVar(processWindowChange);
    while(runStatus){
        switch (mode){
            case Mode::command:
                while(runStatus){
                    Sleep(10);//减少CPU资源浪费
                    // std::lock_guard<std::mutex> lock(mtx);
                    mtx.lock();
                    if(!queueInput.empty()){
                        input = queueInput.front();
                        queueInput.pop();
                        mtx.unlock();
                        //特殊按键
                        if(input == 0 || input == 224){
                            mtx.lock();
                            input = queueInput.front();
                            queueInput.pop();
                            mtx.unlock();
                            commandSpecialChar();
                        }
                        //普通可显示字符
                        else if(input >= 32 && input <= 126){
                            commandVisibleChar();
                        }
                        //esc
                        else if(input == 27){
                            //清空命令
                            commandCharESC();
                        }
                        //'\n'
                        else if(input==13){
                            commandCharEndLine();
                        }
                        //其他字符
                        else{
                            commandOtherChar();
                        }
                    }
                    else{
                        mtx.unlock();
                    }
                    if(mode==Mode::insert)break;
                }
                break;
            case Mode::insert:
                bottomContent="--insert--";
                while(runStatus){
                    Sleep(10);//如果没有，会导致占用大量的CPU资源
                    // std::lock_guard<std::mutex> lock(mtx);
                    mtx.lock();
                    if(!queueInput.empty()){
                        input = queueInput.front();
                        queueInput.pop();
                        mtx.unlock();
                        //if(LastCharIsspecial)goto labelSpecialChar02;
                        //特殊按键
                        if(input == 0|| input == 224){
                            mtx.lock();
                            input = queueInput.front();
                            queueInput.pop();
                            mtx.unlock();
                            insertSpecialChar();
                            
                        }
                        //普通可显示字符
                        else if(input >= 32 && input <= 126 || input == 9){      
                            insertVisibleChar();
                        }
                        //esc
                        else if(input == 27){
                            insertCharESC();
                        }
                        //backspace
                        else if(input == 8){
                            insertCharBackspace();
                        }
                        //'\n'
                        else if(input == 13){
                            insertCharEndLine();
                        }
                        //其他字符
                        else bell();
                    }
                    else{
                        mtx.unlock();
                    }
                    if(mode==Mode::command)break;
                }
                break;
        }
    }
    getInputStatus = false;
    handleWinVarStatus = false;
    //如果不等待结束子线程后释放资源，导致共享的资源释放出问题
    capInput.join();
    handleWinVar.join();
    resetCursor();
    clear();
    return 1;
}

void bell(){
    cout<<'\7';
}

void clear(){
    system("cls");
}

void echoOff(){
    HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(stdIn, &consoleMode);
	SetConsoleMode(stdIn, consoleMode & ~ENABLE_ECHO_INPUT);
}

void commandSpecialChar(){
    switch(input){
        case 73://pgup
            if(pageUp()){
                bottomContent = "pgup";
                clear();
                showUI();
            }
            else{
                bell();
                bottomContent = "arrived top!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 81://padown
            if(pageDown()){
                bottomContent = "pgdown";
                clear();
                showUI();
            }
            else{
                bell();
                bottomContent = "arrived bottom!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 71://home
            if(home()){
                bottomContent = "home";
                clear();
                showUI();
            }
            else{
                bell();
                bottomContent = "arrived top!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 79://end
            if(end()){
                bottomContent = "end";
                clear();
                showUI();
            }
            else{
                bell();
                bottomContent = "arrived bottom!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 72://上
            if(returnFlag = moveUp()){
                bottomContent = "move up";
                if(returnFlag==1){
                    clear();
                    showUI();
                }
                else{
                    bottomContent="";
                }
            }
            else {
                bell();
                bottomContent = "arrived top!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 80://下
            if(returnFlag = moveDown()){
                bottomContent = "move down";
                if(returnFlag==1){
                    clear();
                    showUI();
                }
                else{
                    bottomContent="";
                }
            }
            else {
                bell();
                bottomContent = "arrived bottom!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 75://左
            if(returnFlag = moveLeft()){
                bottomContent = "move left";
                if(returnFlag==1){
                    clear();
                    showUI();
                }
                else{
                    bottomContent="";
                }
            }
            else {
                bell();
                bottomContent = "arrived left!";
            }
            showBottomInfo();
            showCursor();
            break;
        case 77://右
            if(returnFlag = moveRight()){
                bottomContent = "move right";
                if(returnFlag==1){
                    clear();
                    showUI();
                }
                else{
                    bottomContent="";
                }
            }
            else {
                bell();
                bottomContent = "arrived right!";
            }
            showBottomInfo();
            showCursor();
            break;
        default:
            bell();
            bottomContent="illegal char!";
            showBottomInfo();
            showCursor();
            break;//其他特殊按键不理会
    }
}

void commandVisibleChar(){
    if(commandInput.size() == 0){
        switch(input){
            case ':':
                commandInput+=input;
                bottomContent = commandInput;
                showBottomInfo();
                showCursor();
                break;
            case 'h':
                if(returnFlag = moveLeft()){
                    bottomContent = "moved left";
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent="";
                    }
                }
                else {
                    bell();
                    bottomContent = "arrived left!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'j':
                if(returnFlag = moveDown()){
                    bottomContent = "moved down";
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent="";
                    }
                }
                else {
                    bell();
                    bottomContent = "arrived bottom!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'k':
                if(returnFlag = moveUp()){
                    bottomContent = "moved up";
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent="";
                    }
                }
                else {
                    bell();
                    bottomContent = "arrived top!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'l':
                if(returnFlag = moveRight()){
                    bottomContent = "move right";
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent="";
                    }
                }
                else {
                    bell();
                    bottomContent = "arrived right!";
                }
                showBottomInfo();
                showCursor();
                break;
            case '^':
                if(moveToLHeadOfLine()){
                    bottomContent = "went to head";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "arrived head of line!";
                }
                showBottomInfo();
                showCursor();
                break;
            case '$':
                if(moveToEndOfLine()){
                    bottomContent = "went to end";
                    clear();
                    showUI();
                }
                else {
                    bell();
                    bottomContent = "arrived end of line!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'd':
                commandInput+=input;
                bottomContent = commandInput;
                showBottomInfo();
                showCursor();
                break;
            case 'y':
                commandInput+=input;
                bottomContent = commandInput;
                showBottomInfo();
                showCursor();
                break;
            case 'p':
                if(pasteContent!=""){
                    opstk.pushBack(pasteContent, operate::dele, {mouse.getX()==0?0:mouse.getX()+1, mouse.getY()});
                    contentChangeFlag=1;
                    searchResult->resize(0);
                }
                if(pasteAfter()){
                    initLineInfo(mouse.getY());
                    bottomContent = "pasted after";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "no content to paste!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'P':
                if(pasteContent!=""){
                    opstk.pushBack(pasteContent, operate::dele, {mouse.getX(), mouse.getY()});
                    contentChangeFlag=1;
                    searchResult->resize(0);
                }
                if(pasteBefore()){
                    initLineInfo(0);
                    bottomContent = "pasted before";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "no content to paste!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'r':
                commandInput+=input;
                bottomContent = commandInput;
                showBottomInfo();
                showCursor();
                break;
            case 'x':
                if((*text.v)[mouse.getY()].size()){
                    opstk.pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::add, {mouse.getX(), mouse.getY()});
                    contentChangeFlag=1;
                    searchResult->resize(0);
                }
                if(deleteCharHere()){
                    initLineInfo(mouse.getY());
                    //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                    bottomContent = "";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "no char to delete!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'u':
                if(!opstk.empty())lineNumber=opstk.top().pos.getY();
                if(undoOneStep()){
                    contentChangeFlag=1;
                    searchResult->resize(0);
                    initLineInfo(lineNumber);
                    //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                    bottomContent = "undid one step";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "nothing to undo!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'U':
                if(!opstk.empty())lineNumber=opstk.top().pos.getY();
                if(undoAllStep()){
                    contentChangeFlag=1;
                    searchResult->resize(0);
                    initLineInfo(lineNumber);
                    //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                    bottomContent = "undid all step";
                    clear();
                    showUI();
                }
                else{
                    bell();
                    bottomContent = "nothing to undo!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'n':
                if(returnFlag = searchNext()){
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent='/'+destination;
                    }
                }
                else{
                    bell();
                    bottomContent="no next one!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'N':
                if(returnFlag= searchLast()){
                    if(returnFlag==1){
                        clear();
                        showUI();
                    }
                    else{
                        bottomContent='/'+destination;
                    }
                }
                else{
                    bell();
                    bottomContent = "no last one!";
                }
                showBottomInfo();
                showCursor();
                break;
            case 'i':
                mode=Mode::insert;
                changeMode('i');
                bottomContent="--insert";
                showBottomInfo();
                showCursor();
                break;
            case 'I':
                mode=Mode::insert;
                changeMode('I');
                bottomContent="--insert";
                showBottomInfo();
                showCursor();
                break;
            case 'a':
                mode=Mode::insert;
                changeMode('a');
                bottomContent="--insert";
                showBottomInfo();
                showCursor();
                break;
            case 'A':
                mode=Mode::insert;
                changeMode('A');
                bottomContent="--insert";
                showBottomInfo();
                showCursor();
                break;
            case '/':
                commandInput += input;
                bottomContent = commandInput;
                showBottomInfo();
                showCursor();
                break;
            default://其他字符不理会
                bell();
                bottomContent ="illegal char!";
                showBottomInfo();
                showCursor();
                break;
        }
    }
    else if(commandInput.size() == 1){
        //r+<char>替换
        if(commandInput[0] == 'r'){
            commandInput="";
            if((*text.v)[mouse.getY()].size()){
                opstk.pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::repl, {mouse.getX(), mouse.getY()});
                contentChangeFlag=1;
                searchResult->resize(0);
            }
            if(charReplace(input)){
                clear();
                bottomContent="";
                showUI();
            }
            else{
                bell();
                bottomContent="no char to replace!";
            }
            showBottomInfo();
            showCursor();  
        }
        else if(commandInput[0] == '/'){
            commandInput+=input;
            bottomContent=commandInput;
            showBottomInfo();
            showCursor();  
        }
        else {
            switch(input){
                case 'q':
                case 'w':
                    if(commandInput==":"){
                        commandInput+=input;
                        bottomContent=commandInput;
                    }
                    else{
                        bell();
                        commandInput="";
                        bottomContent="no such command!";
                    }
                    showBottomInfo();
                    showCursor();  
                    break;
                case 'd':
                    if(commandInput=="d"){
                        if((*text.v)[mouse.getY()].size()){
                            string temp;
                            for(auto i:(*text.v)[mouse.getY()]){
                                temp.push_back(i);
                            }
                            opstk.pushBack(temp, operate::add, {0, mouse.getY()});
                            contentChangeFlag=1;
                            searchResult->resize(0);
                        }
                        if(deleteLineHere()){
                            initLineInfo(mouse.getY());
                            bottomContent = "deleted this line";
                            clear();
                            showUI();
                        }
                        else{
                            bell();
                            bottomContent = "no char to delete!";
                        }
                    }
                    else{
                        bell();
                        bottomContent ="no such command!";
                    }
                    commandInput = "";
                    showBottomInfo();
                    showCursor(); 
                    break;
                case 'y':
                    if(commandInput=="y"){
                        if(duplicateLineHere()){
                            bottomContent="duplicated this line";
                        }
                        else{
                            bell();
                            bottomContent="no char to dupicate!";
                        }
                    }
                    else{
                        bell();
                        bottomContent="no such command!";
                    }
                    commandInput="";
                    showBottomInfo();
                    showCursor(); 
                    break;
                case '$':
                    if(commandInput=="d"){
                        if((*text.v)[mouse.getY()].size()){
                            string temp;
                            for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++)temp.push_back((*text.v)[mouse.getY()][i]);
                            opstk.pushBack(temp, operate::add, {mouse.getX(), mouse.getY()});
                            contentChangeFlag=1;
                            searchResult->resize(0);
                        }
                        if(deleteLineAfter()){
                            initLineInfo(mouse.getY());
                            bottomContent = "deleted this line after";
                            clear();
                            showUI();
                        }
                        else{
                            bell();
                            bottomContent="no char to delete!";
                        }
                    }
                    else if(commandInput=="y"){
                        if(duplicateLineAfter()){
                            bottomContent="duplicated this line aster";
                        }
                        else{
                            bell();
                            bottomContent="no char to dupicate!";
                        }
                    }
                    else{
                        bell();
                        bottomContent = "no such command!";
                    }
                    commandInput = "";
                    showBottomInfo();
                    showCursor(); 
                    break;
                case '^':
                    if(commandInput=="d"){
                        if((*text.v)[mouse.getY()].size()){
                            string temp;
                            for(int i=0;i<=mouse.getX();i++){
                                temp.push_back((*text.v)[mouse.getY()][i]);
                            }
                            opstk.pushBack(temp, operate::add, {0, mouse.getY()});
                            contentChangeFlag=1;
                            searchResult->resize(0);
                        }
                        if(deleteLineBefore()){
                            initLineInfo(mouse.getY());
                            bottomContent = "deleted this line after";
                            clear();
                            showUI();
                        }
                        else{
                            bell();
                            bottomContent = "no char to delete!";
                        }
                    }
                    else if(commandInput=="y"){
                        if(duplicateLineBefore()){
                            bottomContent="duplicated this line before";
                        }
                        else{
                            bell();
                            bottomContent="no char to duplicate!";
                        }
                    }
                    else{
                        bell();
                        bottomContent = "no such command!";
                    }
                    commandInput = "";
                    showBottomInfo();
                    showCursor();
                    break;
                default://不符合命令，清空命令
                    commandInput = "";
                    bell();
                    bottomContent = "no such command!";
                    showBottomInfo();
                    showCursor();
                    break;
            }
        }
    }
    else if(commandInput.size() == 2){
        if(commandInput[0] == '/'){
            commandInput+=input;
            bottomContent=commandInput;
            showBottomInfo();
            showCursor();
        }
        else {
            switch(input){
                case 'q':
                    if(commandInput==":w"){
                        commandInput+=input;
                        bottomContent=commandInput;
                    }
                    else{
                        commandInput="";
                        bell();
                        bottomContent="no such command!";
                    }
                    showBottomInfo();
                    showCursor();
                    break;
                case 'w':
                    if(commandInput==":q"){
                        commandInput+=input;
                        bottomContent=commandInput;
                    }
                    else{
                        commandInput="";
                        bell();
                        bottomContent="no such command!";
                    }
                    showBottomInfo();
                    showCursor();
                    break;
                case '!':
                    if(commandInput==":q"){
                        commandInput+=input;
                        bottomContent=commandInput;
                    }
                    else{
                        commandInput="";
                        bell();
                        bottomContent="no such command!";
                    }
                    showBottomInfo();
                    showCursor();
                    break;
                default://不符合命令，清空命令
                    commandInput = "";
                    bell();
                    bottomContent="no such command!";
                    showBottomInfo();
                    showCursor();
                    break;
            }
        }
    }
    else if(commandInput.size() >= 3){
        if(commandInput[0] == '/'){
            commandInput+=input;
            bottomContent=commandInput;
            showBottomInfo();
            showCursor();
        }
        else{
            commandInput = "";
            bell();
            bottomContent="no such command!";
            showBottomInfo();
            showCursor();
        }
    }
}

void commandCharESC(){
    commandInput = "";
    bottomContent = "";
    showBottomInfo();
    showCursor();
}

void commandCharEndLine(){
    if(commandInput.size()==0){
        commandInput="";
        bell();
        bottomContent="illegal char!";
        showBottomInfo();
        showCursor();

    }
    else if(commandInput.size()>1&&commandInput[0]=='/'){
        destination="";
        for(int i=1;i<commandInput.size();i++)destination.push_back(commandInput[i]);
        commandInput="";
        if(searchDestination()){
            string item = searchResult->size()>1?" items":" item";
            bottomContent="search finished, found " + std::to_string(searchResult->size()) + item;
        }
        else{
            bell();
            bottomContent="can not find!";
        }
        showBottomInfo();
        showCursor();
    }
    else if(commandInput==":q"){
        commandInput="";
        if(contentChangeFlag){
            bell();
            bottomContent="change did not saved!";
        }
        else{
            runStatus=0;

        }
        showBottomInfo();
        showCursor();
    }
    else if(commandInput==":w"){
        commandInput="";
        if(contentChangeFlag){
            save();
            opstk.clear();
            bottomContent="file saved";
            contentChangeFlag=0;
        }
        else{
            bell();
            bottomContent="do not need to save!";
        }
        showBottomInfo();
        showCursor();
    }
    else if(commandInput==":q!"){
        commandInput="";
        runStatus=0;
    }
    else if(commandInput==":wq"||commandInput==":qw"){
        commandInput="";
        save();
        runStatus=0;
    }
    else{
        commandInput="";
        bell();
        bottomContent="no such command!";
        showBottomInfo();
        showCursor();
    }
}

void commandOtherChar(){
    commandInput="";
    bell();
    bottomContent="illegal char!";
    showBottomInfo();
    showCursor();
}


void insertSpecialChar(){
    switch(input){
        case 73://pgup
            if(pageUp()){
                clear();
                showUI();
            }
            showBottomInfo();
            showCursor();
            break;
        case 81://padown
            if(pageDown()){
                clear();
                showUI();
            }
            showBottomInfo();
            showCursor();
            break;
        case 71://home
            if(home()){
                clear();
                showUI();
            }
            showBottomInfo();
            showCursor();
            break;
        case 79://end
            if(end()){
                clear();
                showUI();
            }
            showBottomInfo();
            showCursor();
            break;
        case 83://delete
            if((*text.v)[mouse.getY()].size() && mouse.getX() != (*text.v)[mouse.getY()].size()){
                opstk.pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::add, {mouse.getX(), mouse.getY()});
                contentChangeFlag=1;
            }
            if(deleteCharHere()){
                initLineInfo(mouse.getY());
                clear();
                showUI();
            }
            showBottomInfo();
            showCursor();
            break;
        case 72://上
            if(returnFlag = moveUp()){
                if(returnFlag==1){
                    clear();
                    showUI();
                }
            }
            showBottomInfo();
            showCursor();
            break;
        case 80://下
            if(returnFlag = moveDown()){
                if(returnFlag==1){
                    clear();
                    showUI();
                }
            }
            showBottomInfo();
            showCursor();
            break;
        case 75://左
            if(returnFlag = moveLeft()){
                if(returnFlag==1){
                    clear();
                    showUI();
                }
            }
            showBottomInfo();
            showCursor();
            break;
        case 77://右
            if(returnFlag = moveRight()){
                if(returnFlag==1){
                    clear();
                    showUI();
                }
            }
            showBottomInfo();
            showCursor();
            break;
        default://其他特殊字符警告
            bell();
            break;
    }
}

void insertVisibleChar(){
    opstk.pushBack(input==9?string(4, ' '):string(1, input), operate::dele, {mouse.getX(), mouse.getY()});
    contentChangeFlag=1;
    searchResult->resize(0);
    if(insertChar(input)){
        initLineInfo(mouse.getY());
        //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
    }
    clear();
    showUI();
    showBottomInfo();
    showCursor();
}

void insertCharESC(){
    mode=Mode::command;
    if(mouse.getX()==(*text.v)[mouse.getY()].size())mouse.renewX(mouse.getX()>0?mouse.getX()-1:0);
    bottomContent = "--command--";
    showBottomInfo();
    showCursor();
}

void insertCharBackspace(){
    if(mouse.getX() == 0 && mouse.getY() != 0){
        //合并行
        opstk.clear();
        contentChangeFlag=1;
    }
    else if(mouse.getX()!=0){
        opstk.pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()-1]), operate::add, {mouse.getX()-1, mouse.getY()});
        contentChangeFlag=1;
        searchResult->resize(0);
    }
    if(deleteCharBefore()){
        initLineInfo(mouse.getY()-1);
        //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
        contentChangeFlag=1;
        clear();
        showUI();
    }
    showBottomInfo();
    showCursor();
}

void insertCharEndLine(){
    enterKey();
    //重开行
    opstk.clear();
    contentChangeFlag=1;
    searchResult->resize(0);
    initLineInfo(mouse.getY()-1);
    //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
    clear();
    showUI();
    showBottomInfo();
    showCursor();
}