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
Position fromMouseToConsole(Mouse& mouse, MyWindow myWin, int indexCount, int startLineNumberInConsole, vector<int>& vectorLineNumber){
    return {mouse.getX()%(myWin.wide - indexCount - 1) + indexCount + 1, vectorLineNumber[mouse.getY()] - startLineNumberInConsole + mouse.getX()/(myWin.wide - indexCount - 1)};
}
int getLineNumberInText(int lineNumberInConsole, vector<int>& vectorLineNumber){
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

void initLineInfo(Text& text, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, int indexCount, int startLineNumberInText){
    vectorLineCount.resize((*text.v).size());
    vectorLineNumber.resize((*text.v).size()+1);
    vectorLineNumber[0]=1;
    for(int i=startLineNumberInText;i<(*text.v).size();i++){
        vectorLineCount[i] = (*text.v)[i].size()>0?((*text.v)[i].size() + myWin.wide - indexCount - 2)/(myWin.wide - indexCount - 1):1;
        vectorLineNumber[i+1] = vectorLineNumber[i] + vectorLineCount[i];
    }
    vectorLineNumber.pop_back();
}

void showUI(HANDLE& hConsole, Text& text, Mouse& mouse, MyWindow& myWin, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, string bottomContent, int startLineNumberInConsole, int indexCount){
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
void showCursor(HANDLE& hConsole, Mouse& mouse, MyWindow& myWin, vector<int>& vectorLineNumber, Mode& mode, string bottomContent, int startLineNumberInConsole, int indexCount){
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

void setCursorPosition(HANDLE& hConsole, int x, int y){
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

int moveUp(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    int startLineNumberRecord = startLineNumberInConsole;
    if(mouse.getY() == 0){
        cout<<'\7';
        return 0;
    }
    else {
        mouse.renewy(mouse.getY()-1);
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
}
int moveDown(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    int startLineNumberRecord = startLineNumberInConsole;
    if(mouse.getY()==(*text.v).size()-1){
        cout<<'\7';
        return 0;
    }
    else {
        mouse.renewy(mouse.getY()+1);
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
}
int moveLeft(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    int startLineNumberRecord = startLineNumberInConsole;
    if(mouse.getX()==0){
        cout<<'\7';
        return 0;
    }
    else {
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
}
int moveRight(Text& text, Mouse& mouse, vector<int>& vectorLineNumber, vector<int>& vectorLineCount,MyWindow& myWin, string bottomContent, int indexCount, int& startLineNumberInConsole){
    int startLineNumberRecord = startLineNumberInConsole;
    if((*text.v)[mouse.getY()].size()==0||mouse.getX()==(*text.v)[mouse.getY()].size()-1){
        cout<<'\7';
        return 0;
    }
    else {
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
                showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
            }
        }
    }
}

int editFile(Text& text, const string file){
    //关回显
    HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(stdIn, &consoleMode);
	SetConsoleMode(stdIn, consoleMode & ~ENABLE_ECHO_INPUT);
    //清空已有内容
    system("cls");
    //鼠标移动或者翻页就更新
    int startLineNumberInConsole = 1;//开始输出的行数
    //鼠标移动就更新
    Mouse mouse;//鼠标在文本中的位置
    //有文本操作就更新
    OperationStack opstk;//操作栈
    Mode mode=Mode::command;//模式
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);//窗口输出句柄
    //text或者myWin变化就更新
    vector<int>* vectorLineNumber = new vector<int>, * vectorLineCount = new vector<int>;//记录文本在控制台窗口下行号和行数信息
    string bottomContent = "";//底部显示的信息
    //hConsole变化就更新
    MyWindow myWin = getMyWindow(hConsole);//记录窗口大小信息
    //text行数变化就更新
    int indexCount = getIndexCount(text);//记录文本最大行数的位数
    bool runStatus = true, getInputsStatus = true, handleWinVarStatus = true;
    string commandInput;//记录命令
    queue<int> queueInput;//记录输入
    string destination;//记录查找串
    //destination变化就更新
    vector<Position>* result = new vector<Position>;//记录查找结果
    //有内容输入就更新
    int tempInput;//输入
    bool LastCharIsspecial = 0;//记录是遇到0或者224但是queueInput为空，即特殊键输入还没有完全入队
    int flag;

    initLineInfo(text, *vectorLineNumber, *vectorLineCount, myWin, indexCount, 0);
    showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
    showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
    thread capInput(captureInput, std::ref(getInputsStatus), std::ref(queueInput));
    thread handleWinVar(processWindowChange, std::ref(handleWinVarStatus), std::ref(hConsole), std::ref(text), std::ref(myWin), vectorLineNumber, vectorLineCount, std::ref(mouse), std::ref(mode), std::ref(bottomContent), std::ref(startLineNumberInConsole), std::ref(indexCount));
    while(runStatus){
        switch (mode){
            LastCharIsspecial = 0;
            case Mode::command:
                while(1){
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
                            }
                            specialChar01:
                            if(LastCharIsspecial)LastCharIsspecial=0;
                            switch(tempInput){
                                case 73://pgup
                                break;
                                case 81://padown
                                break;
                                case 71://home
                                break;
                                case 224://end
                                break;
                                case 83://delete
                                break;
                                case 72://上
                                    if(flag = moveUp(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(flag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                break;
                                case 80://下
                                    if(flag = moveDown(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(flag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                break;
                                case 75://左
                                    if(flag = moveLeft(text, mouse, *vectorLineNumber, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(flag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                break;
                                case 77://右
                                    if(flag = moveRight(text, mouse, *vectorLineNumber, *vectorLineCount, myWin, bottomContent, indexCount, startLineNumberInConsole)){
                                        if(flag==1){
                                            system("cls");
                                            showUI(hConsole, text, mouse, myWin, *vectorLineNumber, *vectorLineCount, bottomContent, startLineNumberInConsole, indexCount);
                                        }
                                        showBottomInfo(hConsole, mouse, myWin, bottomContent);
                                        showCursor(hConsole, mouse, myWin, *vectorLineNumber, mode, bottomContent, startLineNumberInConsole, indexCount);
                                    }
                                break;
                                default:break;//其他特殊按键不理会
                            }
                        }
                        //普通可显示字符
                        else if(tempInput >= 32 && tempInput <= 126){
                            if(commandInput.size() == 0){
                                switch(tempInput){
                                    case ':':
                                        commandInput+=tempInput;
                                        break;
                                    case 'h':
                                        break;
                                    case 'j':
                                        break;
                                    case 'k':
                                        break;
                                    case 'l':
                                        break;
                                    case '^':
                                        break;
                                    case '$':
                                        break;
                                    case 'd':
                                        break;
                                    case 'y':
                                        break;
                                    case 'p':
                                        break;
                                    case 'P':
                                        break;
                                    case 'r':
                                        break;
                                    case 'x':
                                        break;
                                    case 'u':
                                        break;
                                    case 'U':
                                        break;
                                    case 'n':
                                        break;
                                    case 'N':
                                        break;
                                    case 'i':
                                        break;
                                    case 'I':
                                        break;
                                    case 'a':
                                        break;
                                    case 'A':
                                        break;
                                    case 8://backspace
                                        break;
                                    case '/':
                                        break;
                                    default://其他字符不理会
                                        break;
                                }
                            }
                            else if(commandInput.size() == 1){
                                //r+<char>替换
                                if(commandInput[0] == 'r'){
                                    
                                }
                                ///+<string>查找
                                else if(commandInput[0] == '/'){

                                }
                                //其他字符
                                else {
                                    switch(tempInput){
                                        case 'q':
                                            break;
                                        case 'w':
                                            break;
                                        case 'd':
                                            break;
                                        case '$':
                                            break;
                                        case '^':
                                            break;
                                        case 'y':
                                            break;
                                        default://不符合命令，清空命令
                                            commandInput = "";
                                            break;
                                    }
                                }
                            }
                            else if(commandInput.size() == 2){
                                ///+<string>查找
                                if(commandInput[0] == '/'){
                                    
                                }

                                else {
                                    switch(tempInput){
                                        case '\n':
                                            break;
                                        case 'q':
                                            break;
                                        case '!':
                                            break;
                                        default://不符合命令，清空命令
                                            commandInput = "";
                                            break;
                                    }
                                }
                            }
                            else if(commandInput.size() == 3){
                                ///+<string>查找
                                if(commandInput[0] == '/'){
                                    
                                }
                                else{
                                    switch(tempInput){
                                        case '\n':
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                            //开头为'/'，用于查找
                            else {
                                if(tempInput == '\n'){

                                }
                                else{

                                }
                            }
                        }
                        //esc
                        else if(tempInput == 27){
                            //清空
                            commandInput = "";
                            break;
                        }
                        //其他字符不理会
                        else break;
                    }
                    if(mode==Mode::insert)break;
                }
            case Mode::insert:
                LastCharIsspecial = 0;
                while(1){
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
                            }
                            labelSpecialChar02:
                            if(LastCharIsspecial)LastCharIsspecial=0;
                            switch(tempInput){
                                case 73://pgup
                                break;
                                case 81://padown
                                break;
                                case 71://home
                                break;
                                case 224://end
                                break;
                                case 83://delete
                                break;
                                case 72://上
                                break;
                                case 80://下
                                break;
                                case 75://左
                                break;
                                case 77://右
                                break;
                                default:break;
                            }
                        }
                        //普通可显示字符
                        else if(tempInput >= 32 && tempInput <= 126){
                            
                        }
                        //esc
                        else if(tempInput == 27){
                            mode=Mode::command;
                            bottomContent = "";
                            showBottomInfo(hConsole, mouse, myWin, bottomContent);
                            break;
                        }
                        //backspace
                        else if(tempInput == 8){
                        }
                        //enter
                        else if(tempInput == 10){
                        }
                        //其他字符不理会
                        else break;
                    }
                    if(mode==Mode::command)break;
                }
            }
    }

    getInputsStatus = false;
    resetCursor(hConsole);
    delete vectorLineNumber;
    delete vectorLineCount;
    system("cls");
    return 1;
}
  
void save(Text& text, const string file){

}



