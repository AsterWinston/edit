#include"function.h"

int initEdit(vector<string>& v, Text& text, string& file){
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

int getIndexCount(Text& text){
    int line_count = text.v->size();
    int index_count = 1;
    while(line_count = line_count/10)index_count++;
    return index_count;
}
MyWindow getMyWindow(HANDLE& hConsole){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return {csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
}
//从0开始的坐标
Position fromMouseToConsole(const Mouse& mouse, const MyWindow& myWin, const int& indexCount, const int& startLineNumberInConsole, const vector<int>& vectorLineNumber){
    return {mouse.getX()%(myWin.wide - indexCount - 1) + indexCount + 1, vectorLineNumber[mouse.getY()] - startLineNumberInConsole + mouse.getX()/(myWin.wide - indexCount - 1)};
}
int getLineNumberInText(const int& lineNumberInConsole, const vector<int>& vectorLineNumber){
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

void initLineInfo(const Text& text, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, const MyWindow& myWin, const int& indexCount, const int& startLineNumberInText){
    vectorLineCount.resize((*text.v).size());
    vectorLineNumber.resize((*text.v).size()+1);
    vectorLineNumber[0]=1;
    for(int i=startLineNumberInText;i<(*text.v).size();i++){
        vectorLineCount[i] = (*text.v)[i].size()>0?((*text.v)[i].size() + myWin.wide - indexCount - 2)/(myWin.wide - indexCount - 1):1;
        vectorLineNumber[i+1] = vectorLineNumber[i] + vectorLineCount[i];
    }
    vectorLineNumber.pop_back();
}

void showUI(HANDLE& hConsole, const Text& text, const Mouse& mouse, const MyWindow& myWin, const vector<int>& vectorLineNumber, const vector<int>& vectorLineCount, const string bottomContent, const int& startLineNumberInConsole, const int& indexCount){
    //隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    //信息栏
    int lineNumberInText = 0;
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    for(int i = startLineNumberInConsole; i < myWin.height + startLineNumberInConsole - bottomLineCount && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(hConsole, 0, i - startLineNumberInConsole);
        if((lineNumberInText = getLineNumberInText(i, vectorLineNumber))){
            for(int j=0;j<indexCount - getIndexOfNumber(lineNumberInText);j++)cout<<' ';
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout<<lineNumberInText;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout<<' ';
        }
        else {
            for(int j=0;j<indexCount + 1;j++){
                cout<<' '; 
            }
        }
    }
    //内容
    int tempLineNumber = startLineNumberInConsole, count = 0;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--, vectorLineNumber)))count++;
    for(int i = startLineNumberInConsole; i < myWin.height + startLineNumberInConsole - bottomLineCount && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(hConsole, indexCount + 1, i - startLineNumberInConsole);
        if(count < vectorLineCount[lineNumberInText-1]){
            for(int j=count*(myWin.wide - indexCount - 1); j<(count+1)*(myWin.wide - indexCount - 1) && j<(*text.v)[lineNumberInText-1].size(); j++){
                cout<<(*text.v)[lineNumberInText-1][j];
            }
            count++;
        }
        else {
            count = 1;
            lineNumberInText = getLineNumberInText(i, vectorLineNumber);
            for(int j = 0; j<myWin.wide - indexCount - 1 && j<(*text.v)[lineNumberInText-1].size(); j++){
                cout<<(*text.v)[lineNumberInText-1][j];
            }
        }
    }
}
void showCursor(HANDLE& hConsole, const Mouse& mouse, const MyWindow& myWin, const vector<int>& vectorLineNumber, const Mode& mode, const string bottomContent, const int& startLineNumberInConsole, const int& indexCount){
    //光标
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    Position mousePositionInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
    if(mousePositionInConsole.getY()>myWin.height-1-bottomLineCount){
        return;
    }
    else{
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        setCursorPosition(hConsole ,mousePositionInConsole.getX(), mousePositionInConsole.getY());
        if(mode == Mode::command){
            cout << "\x1b[4 q";
        }
        else{ 
	        cout << "\x1b[0 q";
        }
    }
}
void showBottomInfo(HANDLE& hConsole, Mouse& mouse, MyWindow& myWin, string bottomContent){
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    //清空底部
    setCursorPosition(hConsole, 0, myWin.height - bottomLineCount);
    for(int i=0;i<myWin.wide*bottomLineCount;i++)cout<<' ';
    //输出底部信息
    setCursorPosition(hConsole, 0, myWin.height - bottomLineCount);
    cout<<bottomContent;
    setCursorPosition(hConsole, myWin.wide - (getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + 1), myWin.height - 1);
    cout<<mouse.getY()+1<<','<<mouse.getX()+1;
}

void setCursorPosition(HANDLE& hConsole, const int x, const int y){
    COORD cursorPosition;
    cursorPosition.X = x;
    cursorPosition.Y = y;
    SetConsoleCursorPosition(hConsole, cursorPosition);
}
void resetCursor(HANDLE& hConsole){
        setCursorPosition(hConsole, 0, 0);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void captureInput(bool& status, queue<int>& queueInput){
    int temp;
    while(status){
        Sleep(10);
        if(_kbhit()){
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
void processWindowChange(bool& status, HANDLE& hConsole, Text& text, MyWindow& myWin, vector<int>* vectorLineNumber, vector<int>* vectorLineCount, Mouse& mouse, Mode& mode, string& bottomContent, int& startLineNumberInConsole, int& indexCount){
    CONSOLE_SCREEN_BUFFER_INFO csbi;//获取窗口大小信息
    while(status){
        Sleep(10);
        if(GetConsoleScreenBufferInfo(hConsole, &csbi)){
            if(myWin.wide != csbi.srWindow.Right - csbi.srWindow.Left + 1 || myWin.height != csbi.srWindow.Bottom - csbi.srWindow.Top + 1){
                myWin = getMyWindow(hConsole);
                initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                system("cls");
                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
            }
        }
    }
}

int moveUp(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    if(mouse.getY() == 0){
        return 0;
    }
    mouse.renewY(mouse.getY()-1);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
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
int moveDown(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    if(mouse.getY()==(*text.v).size()-1){
        return 0;
    }
    mouse.renewY(mouse.getY()+1);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
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
int moveLeft(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    int startLineNumberRecord = startLineNumberInConsole;
    if(mouse.getX()==0){
        cout<<'\7';
        return 0;
    }
    mouse.renewX(mouse.getX()-1);
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-1>1?startLineNumberInConsole-1:1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int moveRight(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount,MyWindow& myWin, Mode& mode, string bottomContent, int& indexCount, int& startLineNumberInConsole){
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
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
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
int pageUp(Mouse& mouse, vector<int>& vectorLineNumber,MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    if(startLineNumberInConsole == 1){
        return 0;
    }
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    startLineNumberInConsole = startLineNumberInConsole-(myWin.height-bottomLineCount)>0?startLineNumberInConsole-(myWin.height-bottomLineCount):1;
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--, vectorLineNumber)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int pageDown(Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount,MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount){
        return 0;
    }
    startLineNumberInConsole = startLineNumberInConsole+myWin.height-bottomLineCount<vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]?startLineNumberInConsole+myWin.height-bottomLineCount:vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--, vectorLineNumber)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int home(Mouse& mouse, int& startLineNumberInConsole){
    if(startLineNumberInConsole == 1){
        return 0;
    }
    else{
        startLineNumberInConsole = 1;
        mouse.renewPos(0, 0);
        return 1;
    }
}
int end(Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    if(vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-startLineNumberInConsole<=myWin.height-bottomLineCount){
        return 0;
    }
    startLineNumberInConsole = vectorLineNumber[vectorLineNumber.size()-1]+vectorLineCount[vectorLineCount.size()-1]-(myWin.height-bottomLineCount);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--, vectorLineNumber)))count++;
    mouse.renewPos(count*(myWin.wide-indexCount-1), lineNumberInText-1);
    return 1;
}
int moveToLHeadOfLine(Mouse& mouse, vector<int>& vectorLineNumber, Mode& mode, int& startLineNumberInConsole){
    if(mouse.getX()==0){
        return 0;
    }
    mouse.renewX(0);
    if(vectorLineNumber[mouse.getY()]<startLineNumberInConsole){
        startLineNumberInConsole = vectorLineNumber[mouse.getY()];
    }
    return 1;
}
int moveToEndOfLine(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, Mode& mode, string bottomContent, int& startLineNumberInConsole){
    if(mouse.getX()==(*text.v)[mouse.getY()].size()-1){
        return 0;
    }
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    mouse.renewX((*text.v)[mouse.getY()].size() - 1);
    int tempLineNumber = startLineNumberInConsole, count = 0, lineNumberInText;
    while(!(lineNumberInText = getLineNumberInText(tempLineNumber--, vectorLineNumber)))count++;
    if(vectorLineNumber[lineNumberInText-1]+count>startLineNumberInConsole+myWin.height-bottomLineCount){
        startLineNumberInConsole = vectorLineNumber[lineNumberInText-1]+count - myWin.height + bottomLineCount;
    }
    return 1;
}

int deleteCharHere(Text& text, Mouse& mouse, MyWindow& myWin, Mode& mode, int& indexCount, int& startLineNumberInConsole){
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
int deleteCharBefore(Text& text, Mouse& mouse, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole){
    //光标在开头
    if(mouse.getX()==0){
        if(mouse.getY()==0){
            return 0;
        }
        else{
            //合并行
            mouse.renewPos((*text.v)[mouse.getY()].size(), mouse.getY());
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
    (*text.v).resize((*text.v)[indexTemp].size()-1);
    if(!(mouse.getX()%(myWin.wide-indexCount-1)))startLineNumberInConsole=startLineNumberInConsole>1?startLineNumberInConsole-1:1;
    mouse.renewX(mouse.getX()-1);
    return 1;
}
int pasteAfter(Text& text, Mouse& mouse, string pasteContent){
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
int pasteBefore(Text& text, Mouse& mouse, string pasteContent, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole){
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
            startLineNumberInConsole = startLineNumberInConsole+(pasteContent.size()-(myWin.wide-indexCount-2-mouse.getX()%(myWin.wide-indexCount-1))+myWin.wide-indexCount-1-1)%(myWin.wide-indexCount-1);
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
    startLineNumberInConsole=startLineNumberInConsole+(pasteContent.size()-myWin.wide+indexCount)/(myWin.wide-indexCount-1)-1;
    return 1;
}
int charReplace(Text& text, Mouse& mouse, char ch){
    if((*text.v)[mouse.getY()].size()==0){
        return 0;
    }
    else {
        (*text.v)[mouse.getY()][mouse.getX()]=ch;
        return 1;
    }
}
int deleteLineHere(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    (*text.v)[mouse.getY()].resize(0);
    mouse.renewX(0);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int deleteLineAfter(Text& text, Mouse& mouse, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    (*text.v)[mouse.getY()].resize(mouse.getX());
    if(!((mouse.getX()+myWin.wide+indexCount-1)%(myWin.wide-indexCount-1)))startLineNumberInConsole=startLineNumberInConsole>1?startLineNumberInConsole-1:1;
    mouse.renewX(mouse.getX()-1);
    return 1;
}
int deleteLineBefore(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size()-1;i++){
        (*text.v)[mouse.getY()][i-mouse.getX()]=(*text.v)[mouse.getY()][i];        
    }
    (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()-mouse.getX()-1);
    mouse.renewX(0);
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
    int bottomLineCount = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()+1) + getIndexOfNumber(mouse.getY()+1) + myWin.wide)/myWin.wide:1;
    int gapSize = (myWin.height-1-bottomLineCount)/2>5?5:(myWin.height-1-bottomLineCount)/2;
    if(mouse.getX()>(*text.v)[mouse.getY()].size() - 1||(*text.v)[mouse.getY()].size() == 0)mouse.renewX((*text.v)[mouse.getY()].size() == 0?0:(*text.v)[mouse.getY()].size() - 1);
    if(posInConsole.getY()<gapSize){
        startLineNumberInConsole=startLineNumberInConsole-(gapSize-posInConsole.getY())>1?startLineNumberInConsole-(gapSize-posInConsole.getY()):1;
    }
    if(startLineNumberRecord!=startLineNumberInConsole)return 1;
    return 2;
}
int insertChar(Text& text, Mouse& mouse, char ch, MyWindow& myWin, int& indexCount, int& startLineNumberInConsole){
    if((*text.v)[mouse.getY()].size()==0){
        (*text.v)[mouse.getY()].push_back(ch);
        mouse.renewX(mouse.getX()+1);
        return 0;
    }
    if(!((mouse.getX()+1)%(myWin.wide-indexCount-1)))startLineNumberInConsole+=1;
    (*text.v)[mouse.getY()].resize((*text.v)[mouse.getY()].size()+1);
    for(int i=(*text.v)[mouse.getY()].size();i>mouse.getX();i--){
        (*text.v)[mouse.getY()][i]=(*text.v)[mouse.getY()][i-1];
    }
    (*text.v)[mouse.getY()][mouse.getX()]=ch;
    mouse.renewX(mouse.getX()+1);
    return 1;
}
void enterKey(Text& text, Mouse& mouse, int& startLineNumberInConsole, int& indexCount){
    vector<char> temp;
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++){
        temp.push_back((*text.v)[mouse.getY()][i]);
    }
    (*text.v).resize((*text.v).size()+1);
    indexCount = getIndexCount(text);
    for(int i=(*text.v).size();i>mouse.getY()+1;i--){
        (*text.v)[i]=(*text.v)[i-1];
    }
    (*text.v)[mouse.getY()+1]=temp;
    mouse.renewPos(0, mouse.getY()+1);
    startLineNumberInConsole+=1;
}

int undoOneStep(Text& text, OperationStack& opstk){
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
            break;
        }      
        case operate::dele:
        {
            for(int i=oso.pos.getX();i<oso.str.size()-1;i++)(*text.v)[oso.pos.getY()][i]=(*text.v)[oso.pos.getY()][i+1];
            (*text.v)[oso.pos.getY()].resize((*text.v)[oso.pos.getY()].size()-oso.str.size());
            break;
        }
        case operate::repl:
            for(int i=0;i<oso.str.size();i++)(*text.v)[oso.pos.getY()][oso.pos.getX()+i]=oso.str[i];
            break;
    }
    return 1;
}
int undoAllStep(Text& text, OperationStack& opstk){
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
                break;
            }      
            case operate::dele:
            {
                for(int i=oso.pos.getX();i<oso.str.size()-1;i++)(*text.v)[oso.pos.getY()][i]=(*text.v)[oso.pos.getY()][i+1];
                (*text.v)[oso.pos.getY()].resize((*text.v)[oso.pos.getY()].size()-oso.str.size());
                break;
            }
            case operate::repl:
                for(int i=0;i<oso.str.size();i++)(*text.v)[oso.pos.getY()][oso.pos.getX()+i]=oso.str[i];
                break;
        }
    }    
    return 1;
}

int searchDestination(Text& text, string destination, vector<Position>& searchResult){
    searchResult.resize(0);
    int xLast, xNew ,y;
    for(int i=0;i<(*text.v).size();i++){
        y=i;
        xLast=-1;
        vector<char> temp = (*text.v)[i];
        while((xNew=KMPSearch(temp, destination))!=temp.size()){
            searchResult.push_back({xNew+xLast+1, y});
            xLast = xNew;
            temp.resize(0);
            for(int j=xLast+1;i<(*text.v)[i].size();j++){
                temp.push_back((*text.v)[i][j]);
            }
        }
    }
    if(searchResult.size())return 1;
    return 0;
}
int searchNext(vector<Position>& result, Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int& indexCount, int& startLineNumberInConsole){
    Position tempPos;
    int flag=0;
    for(auto i:result){
        if(i.getY()>=mouse.getY()&&i.getX()>mouse.getX()){
            tempPos=i;
            flag=1;
            break;
        }
    }
    if(!flag){
        return 0;
    }
    mouse.renewPos(tempPos.getX(),tempPos.getY());
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
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
int searchLast(vector<Position>& result, Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    Position tempPos;
    int flag=0;
    for(int i=result.size()-1;i>=0;i--){
        if(result[i].getY()<=mouse.getY()&&result[i].getX()<mouse.getX()){
            tempPos = result[i];
            flag=1;
        }
    }
    if(!flag){
        return 0;
    }
    mouse.renewPos(tempPos.getX(),tempPos.getY());
    int startLineNumberRecord = startLineNumberInConsole;
    Position posInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumberInConsole, vectorLineNumber);
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

void changeMode(char select, Text& text, Mouse& mouse){
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
void save(Text& text, const string file){
    std::ofstream ofs;
    ofs.open(file, std::ios::out);
    if(!ofs.is_open()){
        system("cls");
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

int duplicateLineHere(Text& text, Mouse& mouse, string& pasteContent){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(auto i:(*text.v)[mouse.getY()]){
        pasteContent.push_back(i);
    }
    return 1;
}
int duplicateLineAfter(Text& text, Mouse& mouse, string& pasteContent){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++){
        pasteContent.push_back((*text.v)[mouse.getY()][i]);
    }
    return 1;
}
int duplicateLineBefore(Text& text, Mouse& mouse, string& pasteContent){
    if(!(*text.v)[mouse.getY()].size()){
        return 0;
    }
    pasteContent.resize(0);
    for(int i=0;i<=mouse.getX();i++){
        pasteContent.push_back((*text.v)[mouse.getY()][i]);
    }
    return 1;
}

int editFile(Text& text, const string file){
    //关回显
    HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(stdIn, &consoleMode);
	SetConsoleMode(stdIn, consoleMode & ~ENABLE_ECHO_INPUT);
    //不变量
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);//窗口输出句柄
    //
    int startLineNumberInConsole = 1;//开始输出的行数
    //
    Mouse mouse;//鼠标在文本中的位置
    //
    OperationStack* opstk = new OperationStack;//操作栈
    //
    Mode mode=Mode::command;//模式
    //
    vector<int>* vectorLineNumber = new vector<int>, * vectorLineCount = new vector<int>;//记录文本在控制台窗口下行号和行数信息
    //
    string bottomContent = "";//底部显示的信息
    //子线程自动更新
    MyWindow myWin = getMyWindow(hConsole);//记录窗口大小信息
    //
    int indexCount = getIndexCount(text);//记录文本最大行数的位数
    //
    string commandInput;//记录命令
    //子线程自动更新
    queue<int> queueInput;//记录输入
    //
    vector<Position>* searchResult = new vector<Position>;//记录查找结果
    //
    bool LastCharIsspecial = 0;//记录是遇到0或者224但是queueInput为空，即特殊键输入还没有完全入队
    //
    bool runStatus = true, getInputsStatus = true, handleWinVarStatus = true;
    //
    int returnFlag;
    //
    int tempInput;//输入
    //
    int contentChangeFlag=0;
    //
    string pasteContent = "";//复制的内容
    //
    string destination = "";

    initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
    system("cls");
    showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
    showBottomInfo(hConsole, mouse, myWin, bottomContent);
    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
    thread capInput(captureInput, std::ref(getInputsStatus), std::ref(queueInput));
    thread handleWinVar(processWindowChange, std::ref(handleWinVarStatus), std::ref(hConsole), std::ref(text), std::ref(myWin), vectorLineNumber, vectorLineCount, std::ref(mouse), std::ref(mode), std::ref(bottomContent), std::ref(startLineNumberInConsole), std::ref(indexCount));
    while(runStatus){
        switch (mode){
            case Mode::command:
                while(runStatus){
                    if(!queueInput.empty()){
                        tempInput = queueInput.front();
                        queueInput.pop();
                        if(LastCharIsspecial)goto specialChar01;//上一个是0或者224
                        //特殊按键
                        if(tempInput == 0 || tempInput == 224){
                            if(!queueInput.empty()){
                                tempInput = queueInput.front();
                                queueInput.pop();
                            }
                            else {
                                LastCharIsspecial = 1;
                                break;
                            }
                            specialChar01:
                            if(LastCharIsspecial)LastCharIsspecial=0;
                            switch(tempInput){
                                case 73://pgup
                                    if(pageUp(mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "pgup";
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    else{
                                        cout<<'\7';
                                        bottomContent = "arrived top!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 81://padown
                                    if(pageDown(mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "pgdown";
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    else{
                                        cout<<'\7';
                                        bottomContent = "arrived bottom!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 71://home
                                    if(home(mouse, startLineNumberInConsole)){
                                        bottomContent = "home";
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    else{
                                        cout<<'\7';
                                        bottomContent = "arrived top!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 79://end
                                    if(end(mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "end";
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    else{
                                        cout<<'\7';
                                        bottomContent = "arrived bottom!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 72://上
                                    if(returnFlag = moveUp(text, mouse, *vectorLineNumber, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "move up";
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            bottomContent="";
                                        }
                                    }
                                    else {
                                        cout<<'\7';
                                        bottomContent = "arrived top!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 80://下
                                    if(returnFlag = moveDown(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode,  bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "move down";
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            bottomContent="";
                                        }
                                    }
                                    else {
                                        cout<<'\7';
                                        bottomContent = "arrived bottom!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 75://左
                                    if(returnFlag = moveLeft(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "move left";
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            bottomContent="";
                                        }
                                    }
                                    else {
                                        cout<<'\7';
                                        bottomContent = "arrived left!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 77://右
                                    if(returnFlag = moveRight(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                        bottomContent = "move right";
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            bottomContent="";
                                        }
                                    }
                                    else {
                                        cout<<'\7';
                                        bottomContent = "arrived right!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                default:
                                    cout<<'\7';
                                    bottomContent="illegal char!";
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;//其他特殊按键不理会
                            }
                        }
                        //普通可显示字符
                        else if(tempInput >= 32 && tempInput <= 126){
                            if(commandInput.size() == 0){
                                switch(tempInput){
                                    case ':':
                                        commandInput+=tempInput;
                                        bottomContent = commandInput;
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'h':
                                        if(returnFlag = moveLeft(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                            bottomContent = "moved left";
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent="";
                                            }
                                        }
                                        else {
                                            cout<<'\7';
                                            bottomContent = "arrived left!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'j':
                                        if(returnFlag = moveDown(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                            bottomContent = "moved down";
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent="";
                                            }
                                        }
                                        else {
                                            cout<<'\7';
                                            bottomContent = "arrived bottom!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'k':
                                        if(returnFlag = moveUp(text, mouse, *vectorLineNumber, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                            bottomContent = "moved up";
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent="";
                                            }
                                        }
                                        else {
                                            cout<<'\7';
                                            bottomContent = "arrived top!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'l':
                                        if(returnFlag = moveRight(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                            bottomContent = "move right";
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent="";
                                            }
                                        }
                                        else {
                                            cout<<'\7';
                                            bottomContent = "arrived right!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case '^':
                                        if(moveToLHeadOfLine(mouse, *vectorLineNumber, mode, startLineNumberInConsole)){
                                            bottomContent = "went to head";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "arrived head of line!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case '$':
                                        if(moveToEndOfLine(text, mouse, *vectorLineNumber, myWin, mode, bottomContent, startLineNumberInConsole)){
                                            bottomContent = "went to end";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else {
                                            cout<<'\7';
                                            bottomContent = "arrived end of line!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'd':
                                        commandInput+=tempInput;
                                        bottomContent = commandInput;
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'y':
                                        commandInput+=tempInput;
                                        bottomContent = commandInput;
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'p':
                                        if(pasteContent!=""){
                                            opstk->pushBack(pasteContent, operate::dele, {mouse.getX()==0?0:mouse.getX()+1, mouse.getY()});
                                            contentChangeFlag=1;
                                            searchResult->resize(0);
                                        }
                                        if(pasteAfter(text, mouse, pasteContent)){
                                            contentChangeFlag=1;
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            bottomContent = "pasted after";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "no content to paste!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'P':
                                        if(pasteContent!=""){
                                            opstk->pushBack(pasteContent, operate::dele, {mouse.getX(), mouse.getY()});
                                            contentChangeFlag=1;
                                            searchResult->resize(0);
                                        }
                                        if(pasteBefore(text, mouse, pasteContent, myWin, indexCount, startLineNumberInConsole)){
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            bottomContent = "pasted before";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "no content to paste!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'r':
                                        commandInput+=tempInput;
                                        bottomContent = commandInput;
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'x':
                                        if((*text.v)[mouse.getY()].size()){
                                            opstk->pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::add, {mouse.getX(), mouse.getY()});
                                            contentChangeFlag=1;
                                            searchResult->resize(0);
                                        }
                                        if(deleteCharHere(text, mouse, myWin, mode, indexCount, startLineNumberInConsole)){
                                            //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, mouse.getY());
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            bottomContent = "";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "no char to delete!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'u':
                                        if(undoOneStep(text, *opstk)){
                                            contentChangeFlag=1;
                                            searchResult->resize(0);
                                            //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, opstk.top().pos.getY());
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            bottomContent = "undid one step";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "nothing to undo!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'U':
                                        if(undoAllStep(text, *opstk)){
                                            contentChangeFlag=1;
                                            searchResult->resize(0);
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, opstk->top().pos.getY());
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            bottomContent = "undid all step";
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "nothing to undo!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'n':
                                        if(returnFlag = searchNext(*searchResult, text, mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent='/'+destination;
                                            }
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent="no next one!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'N':
                                        if(returnFlag= searchLast(*searchResult, text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                            if(returnFlag==1){
                                                system("cls");
                                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                            }
                                            else{
                                                bottomContent='/'+destination;
                                            }
                                        }
                                        else{
                                            cout<<'\7';
                                            bottomContent = "no last one!";
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'i':
                                        mode=Mode::insert;
                                        changeMode('i', text, mouse);
                                        bottomContent="--insert";
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'I':
                                        mode=Mode::insert;
                                        changeMode('I', text, mouse);
                                        bottomContent="--insert";
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'a':
                                        mode=Mode::insert;
                                        changeMode('a', text, mouse);
                                        bottomContent="--insert";
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case 'A':
                                        mode=Mode::insert;
                                        changeMode('A', text, mouse);
                                        bottomContent="--insert";
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    case '/':
                                        commandInput += tempInput;
                                        bottomContent = commandInput;
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                    default://其他字符不理会
                                        cout<<'\7';
                                        bottomContent ="illegal char!";
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                        break;
                                }
                            }
                            else if(commandInput.size() == 1){
                                //r+<char>替换
                                if(commandInput[0] == 'r'){
                                    commandInput="";
                                    if((*text.v)[mouse.getY()].size()){
                                        opstk->pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::repl, {mouse.getX(), mouse.getY()});
                                        contentChangeFlag=1;
                                        searchResult->resize(0);
                                    }
                                    if(charReplace(text, mouse, tempInput)){
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    else{
                                        cout<<'\7';
                                        bottomContent="no char to replace!";
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);  
                                }
                                else if(commandInput[0] == '/'){
                                    commandInput+=tempInput;
                                    bottomContent=commandInput;
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);  
                                }
                                else {
                                    switch(tempInput){
                                        case 'q':
                                        case 'w':
                                            if(commandInput==":"){
                                                commandInput+=tempInput;
                                                bottomContent=commandInput;
                                            }
                                            else{
                                                cout<<'\7';
                                                commandInput="";
                                                bottomContent="no such command!";
                                            }
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);  
                                            break;
                                        case 'd':
                                            if(commandInput=="d"){
                                                if((*text.v)[mouse.getY()].size()){
                                                    string temp;
                                                    for(auto i:(*text.v)[mouse.getY()]){
                                                        temp.push_back(i);
                                                    }
                                                    opstk->pushBack(temp, operate::add, {0, mouse.getY()});
                                                    contentChangeFlag=1;
                                                    searchResult->resize(0);
                                                }
                                                if(deleteLineHere(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                                    initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                                    bottomContent = "deleted this line";
                                                    system("cls");
                                                    showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent = "no char to delete!";
                                                }
                                            }
                                            else{
                                                cout<<'\7';
                                                bottomContent ="no such command!";
                                            }
                                            commandInput = "";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount); 
                                            break;
                                        case 'y':
                                            if(commandInput=="y"){
                                                if(duplicateLineHere(text, mouse, pasteContent)){
                                                    bottomContent="duplicated this line";
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent="no char to dupicate!";
                                                }
                                            }
                                            else{
                                                cout<<'\7';
                                                bottomContent="no such command!";
                                            }
                                            commandInput="";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount); 
                                            break;
                                        case '$':
                                            if(commandInput=="d"){
                                                if((*text.v)[mouse.getY()].size()){
                                                    string temp;
                                                    for(int i=mouse.getX();i<(*text.v)[mouse.getY()].size();i++)temp.push_back((*text.v)[mouse.getY()][i]);
                                                    opstk->pushBack(temp, operate::add, {mouse.getX(), mouse.getY()});
                                                    contentChangeFlag=1;
                                                    searchResult->resize(0);
                                                }
                                                if(deleteLineAfter(text, mouse, myWin, indexCount, startLineNumberInConsole)){
                                                    initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                                    bottomContent = "deleted this line after";
                                                    system("cls");
                                                    showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent="no char to delete!";
                                                }
                                            }
                                            else if(commandInput=="y"){
                                                if(duplicateLineAfter(text, mouse, pasteContent)){
                                                    bottomContent="duplicated this line aster";
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent="no char to dupicate!";
                                                }
                                            }
                                            else{
                                                cout<<'\7';
                                                bottomContent = "no such command!";
                                            }
                                            commandInput = "";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount); 
                                            break;
                                        case '^':
                                            if(commandInput=="d"){
                                                if((*text.v)[mouse.getY()].size()){
                                                    string temp;
                                                    for(int i=0;i<=mouse.getX();i++){
                                                        temp.push_back((*text.v)[mouse.getY()][i]);
                                                    }
                                                    opstk->pushBack(temp, operate::add, {0, mouse.getY()});
                                                    contentChangeFlag=1;
                                                    searchResult->resize(0);
                                                }
                                                if(deleteLineBefore(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                                    initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                                    bottomContent = "deleted this line after";
                                                    system("cls");
                                                    showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent = "no char to delete!";
                                                }
                                            }
                                            else if(commandInput=="y"){
                                                if(duplicateLineBefore(text, mouse, pasteContent)){
                                                    bottomContent="duplicated this line before";
                                                }
                                                else{
                                                    cout<<'\7';
                                                    bottomContent="no char to duplicate!";
                                                }
                                            }
                                            else{
                                                cout<<'\7';
                                                bottomContent = "no such command!";
                                            }
                                            commandInput = "";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                        default://不符合命令，清空命令
                                            commandInput = "";
                                            cout<<'\7';
                                            bottomContent = "no such command!";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                    }
                                }
                            }
                            else if(commandInput.size() == 2){
                                if(commandInput[0] == '/'){
                                    commandInput+=tempInput;
                                    bottomContent=commandInput;
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                }
                                else {
                                    switch(tempInput){
                                        case 'q':
                                            if(commandInput==":w"){
                                                commandInput+=tempInput;
                                                bottomContent=commandInput;
                                            }
                                            else{
                                                commandInput="";
                                                cout<<'\7';
                                                bottomContent="no such command!";
                                            }
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                        case 'w':
                                            if(commandInput==":q"){
                                                commandInput+=tempInput;
                                                bottomContent=commandInput;
                                            }
                                            else{
                                                commandInput="";
                                                cout<<'\7';
                                                bottomContent="no such command!";
                                            }
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                        case '!':
                                            if(commandInput==":q"){
                                                commandInput+=tempInput;
                                                bottomContent=commandInput;
                                            }
                                            else{
                                                commandInput="";
                                                cout<<'\7';
                                                bottomContent="no such command!";
                                            }
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                        default://不符合命令，清空命令
                                            commandInput = "";
                                            cout<<'\7';
                                            bottomContent="no such command!";
                                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                            break;
                                    }
                                }
                            }
                            else if(commandInput.size() >= 3){
                                if(commandInput[0] == '/'){
                                    commandInput+=tempInput;
                                    bottomContent=commandInput;
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                }
                                else{
                                    commandInput = "";
                                    cout<<'\7';
                                    bottomContent="no such command!";
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                }
                                break;
                            }
                        }
                        //esc
                        else if(tempInput == 27){
                            //清空命令
                            commandInput = "";
                            bottomContent="";
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            break;
                        }
                        //'\n'
                        else if(tempInput==13){
                            if(commandInput.size()==0){
                                commandInput="";
                                cout<<'\7';
                                bottomContent="illegal char!";
                                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                break;
                            }
                            else if(commandInput.size()>1&&commandInput[0]=='/'){
                                destination="";
                                for(int i=1;i<commandInput.size();i++)destination.push_back(commandInput[i]);
                                commandInput="";
                                if(searchDestination(text, destination, *searchResult)){
                                    bottomContent="search finished";
                                }
                                else{
                                    cout<<'\7';
                                    bottomContent="can not find!";
                                }
                                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            }
                            else if(commandInput==":q"){
                                commandInput="";
                                if(contentChangeFlag){
                                    cout<<'\7';
                                    bottomContent="change did not saved!";
                                }
                                else{
                                    runStatus=0;
                                    break;
                                }
                                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            }
                            else if(commandInput==":w"){
                                commandInput="";
                                if(contentChangeFlag){
                                    save(text, file);
                                    opstk->clear();
                                    bottomContent="file saved";
                                    contentChangeFlag=0;
                                }
                                else{
                                    cout<<'\7';
                                    bottomContent="do not need to save!";
                                }
                                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            }
                            else if(commandInput==":q!"){
                                commandInput="";
                                runStatus=0;
                                break;
                            }
                            else if(commandInput==":wq"||commandInput==":qw"){
                                commandInput="";
                                save(text, file);
                                runStatus=0;
                                break;
                            }
                            else{
                                commandInput="";
                                cout<<'\7';
                                bottomContent="no such command!";
                                showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            }
                            break;
                        }
                        //其他字符
                        else{
                            commandInput="";
                            cout<<'\7';
                            bottomContent="illegal char!";
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                            break;
                        }
                    }
                    if(mode==Mode::insert)break;
                }
                break;
            case Mode::insert:
                bottomContent="--insert--";
                while(runStatus){
                    if(!queueInput.empty()){
                        tempInput = queueInput.front();
                        queueInput.pop();
                        if(LastCharIsspecial)goto labelSpecialChar02;
                        //特殊按键
                        if(tempInput == 0|| tempInput == 224){
                            if(!queueInput.empty()){
                                tempInput = queueInput.front();
                                queueInput.pop();
                            }
                            else {
                                LastCharIsspecial=1;
                                break;
                            }
                            labelSpecialChar02:
                            if(LastCharIsspecial)LastCharIsspecial=0;
                            switch(tempInput){
                                case 73://pgup
                                    if(pageUp(mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 81://padown
                                    if(pageDown(mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 71://home
                                    if(home(mouse, startLineNumberInConsole)){
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 79://end
                                    if(end(mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        system("cls");
                                        showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 83://delete
                                    if((*text.v)[mouse.getY()].size())opstk->pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()]), operate::add, {mouse.getX(), mouse.getY()});
                                        if(deleteCharHere(text, mouse, myWin, mode, indexCount, startLineNumberInConsole)){
                                            contentChangeFlag=1;
                                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 72://上
                                    if(returnFlag = moveUp(text, mouse, *vectorLineNumber, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 80://下
                                    if(returnFlag = moveDown(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode,  bottomContent, indexCount, startLineNumberInConsole)){
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 75://左
                                    if(returnFlag = moveLeft(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                case 77://右
                                    if(returnFlag = moveRight(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, mode, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(returnFlag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                    }
                                    showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    break;
                                default://其他特殊字符警告
                                    cout<<'\7';
                                    break;
                            }
                        }
                        //普通可显示字符
                        else if(tempInput >= 32 && tempInput <= 126){
                            opstk->pushBack(string(1, tempInput), operate::dele, {mouse.getX(), mouse.getY()});
                            contentChangeFlag=1;
                            if(insertChar(text, mouse, tempInput, myWin, indexCount, startLineNumberInConsole)){
                                //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, mouse.getY());
                                initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                            }
                            system("cls");
                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                        }
                        //esc
                        else if(tempInput == 27){
                            mode=Mode::command;
                            if(mouse.getX()==(*text.v)[mouse.getY()].size())mouse.renewX(mouse.getX()>0?mouse.getX()-1:0);
                            bottomContent = "--command--";
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                        }
                        //backspace
                        else if(tempInput == 8){
                            if(mouse.getX() == 0 && mouse.getY() != 0){
                                opstk->clear();
                            }
                            else if(mouse.getX()!=0){
                                opstk->pushBack(string(1, (*text.v)[mouse.getY()][mouse.getX()-1]), operate::add, {mouse.getX()-1, mouse.getY()});
                                contentChangeFlag=1;
                                searchResult->resize(0);
                            }
                            if(deleteCharBefore(text, mouse, myWin, indexCount, startLineNumberInConsole)){
                                //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, mouse.getY()-1);
                                initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                                contentChangeFlag=1;
                                system("cls");
                                showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                            }
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                        }
                        //'\n'
                        else if(tempInput == 13){
                            enterKey(text, mouse, startLineNumberInConsole, indexCount);
                            opstk->clear();
                            contentChangeFlag=1;
                            searchResult->resize(0);
                            //initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, mouse.getY());
                            initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
                            system("cls");
                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                        }
                        //其他字符
                        else cout<<'\7';
                    }
                    if(mode==Mode::command)break;
                }
                break;
        }
    }

    getInputsStatus = false;
    handleWinVarStatus = false;
    resetCursor(hConsole);
    system("cls");
    delete opstk;
    delete vectorLineNumber;
    delete vectorLineCount;
    delete searchResult;
    return 1;
}
  

