#include"function.h"
int initEdit(vector<string>& v, Text& text){
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
            cout<<"fail to open file!"<<endl;
            //极大可能是文件和目录同名，导致无法创建
            return -1;
        }
        else {
            text.v->push_back(vector<char> {});//文件创建成功，插入一行空串
            ofs.close();
            return 0;
        }
    }
    else {
        string line;
        while(getline(ifs, line)){
            vector<char>line_v(line.begin(), line.end());
            text.v->push_back(line_v);
        }
        ifs.close();
        if(!text.v->size())text.v->push_back(vector<char>{});//文件为空，插入一行空串
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
void initLineInfo(HANDLE& hConsole, Text& text, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, int indexCount, MyWindow& myWin){
    vectorLineCount.resize(0);
    vectorLineNumber.resize(0);
    vectorLineNumber.push_back(1);
    for(auto i: *text.v){
        vectorLineCount.push_back(i.size()>0?(i.size() + myWin.wide - indexCount - 2)/(myWin.wide - indexCount - 1):1);
        vectorLineNumber.push_back(vectorLineNumber[vectorLineNumber.size() - 1] + vectorLineCount[vectorLineCount.size() - 1]);
    }
    vectorLineNumber.pop_back();
}
void showUI(HANDLE& hConsole, Text& text, Mouse& mouse, int startLineNumber, MyWindow& myWin, int indexCount, string bottomContent, vector<int>& vectorLineNumber, vector<int>& vectorLineCount){
    if(startLineNumber < 1 || startLineNumber > vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1){
        system("cls");
        resetCursor(hConsole);
        setCursorPosition(hConsole, 0, 0);
        cout<<"illegal startLineNumber!"<<endl;
        exit(0);
    }
    int bottomLine = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()) + getIndexOfNumber(mouse.getY()) + myWin.wide)/myWin.wide:1;
    int lineNumber = 0;
    for(int i = startLineNumber; i < myWin.height + startLineNumber - bottomLine && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(hConsole, 0, i - startLineNumber);
        if((lineNumber = getLineNumber(i, vectorLineNumber))){
            for(int j=0;j<indexCount - getIndexOfNumber(lineNumber);j++)cout<<' ';
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout<<lineNumber;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout<<' ';
        }
        else {
            for(int j=0;j<indexCount + 1;j++){
                cout<<' '; 
            }
        }
    }
    setCursorPosition(hConsole, 0, myWin.height - 1 - bottomLine);
    cout<<bottomContent;
    setCursorPosition(hConsole, myWin.wide - (getIndexOfNumber(mouse.getX()) + getIndexOfNumber(mouse.getY()) + 1), myWin.height - 1);
    cout<<mouse.getY()<<','<<mouse.getX();
}
void showContent(HANDLE& hConsole, Text& text, MyWindow& myWin, int startLineNumber, vector<int>& vectorLineNumber, vector<int>& vectorLineCount, int indexCount, Mouse& mouse, string bottomContent){
    if(startLineNumber < 1 || startLineNumber > vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1){
        system("cls");
        resetCursor(hConsole);
        setCursorPosition(hConsole, 0, 0);
        cout<<"illegal startLineNumber!"<<endl;
        exit(0);
    }
    int bottomLine = bottomContent.size()>0?(bottomContent.size() + getIndexOfNumber(mouse.getX()) + getIndexOfNumber(mouse.getY()) + myWin.wide)/myWin.wide:1;
    int belongLineNumber, temp = startLineNumber, count = 0;
    while(!(belongLineNumber = getLineNumber(temp --, vectorLineNumber)))count++;
    for(int i = startLineNumber; i < myWin.height + startLineNumber - bottomLine && i <= vectorLineNumber[vectorLineNumber.size()-1] + vectorLineCount[vectorLineCount.size()-1] - 1; i++){
        setCursorPosition(hConsole, indexCount + 1, i - startLineNumber);
        if(count < vectorLineCount[belongLineNumber - 1]){
            for(int j=count*(myWin.wide - indexCount - 1); j<(count+1)*(myWin.wide - indexCount - 1) && j<(*text.v)[belongLineNumber - 1].size(); j++){
                cout<<(*text.v)[belongLineNumber - 1][j];
            }
            count++;
        }
        else {
            count = 1;
            belongLineNumber = getLineNumber(i, vectorLineNumber);
            for(int j = 0; j<myWin.wide - indexCount - 1 && j<(*text.v)[belongLineNumber - 1].size(); j++){
                cout<<(*text.v)[belongLineNumber - 1][j];
            }
        }
    }
}
void setCursorPosition(HANDLE& hConsole, int x, int y){
    COORD cursorPosition;
    cursorPosition.X = x;
    cursorPosition.Y = y;
    SetConsoleCursorPosition(hConsole, cursorPosition);
}
int getLineNumber(int lineNumber, vector<int>& vectorLineNumber){
    int low = 0, high = vectorLineNumber.size() - 1, mid;
    //没有=就炸了
    while(low <= high){
        mid = (low+high)/2;
        if(vectorLineNumber[mid] == lineNumber)return mid + 1;
        else if(lineNumber > vectorLineNumber[mid])low = mid + 1;
        else high = mid - 1;
    }
    return 0;
}
int getIndexOfNumber(int number){
    int temp = 1;
    while(number = number/10)temp++;
    return temp;
}

void showCursor(HANDLE& hConsole, Mouse& mouse, Mode& mode, int indexCount, vector<int>& vectorLineNumber,vector<int>& vectorLineCount, MyWindow& myWin, int startLineNumber){
    if(mode == Mode::command){
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        //向右x为正，向下y为正
        Position mousePositionInConsole = fromMouseToConsole(mouse, myWin, indexCount, startLineNumber, vectorLineNumber);
        COORD position = {static_cast<short>(mousePositionInConsole.getX()), static_cast<short>(mousePositionInConsole.getY())};
        CHAR_INFO charInfo;
        SMALL_RECT readRegion = {static_cast<short>(mousePositionInConsole.getX()), static_cast<short>(mousePositionInConsole.getY()), static_cast<short>(mousePositionInConsole.getX()), static_cast<short>(mousePositionInConsole.getY())};
        COORD bufferSize = {1, 1};
        COORD bufferCoord = {0, 0};
        WORD backgroundColor = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
        ReadConsoleOutput(hConsole, &charInfo, bufferSize, bufferCoord, &readRegion);
        charInfo.Attributes = (charInfo.Attributes & 0x0F) | backgroundColor;
        WriteConsoleOutput(hConsole, &charInfo, bufferSize, bufferCoord, &readRegion);
    }
    else{
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
}
Position fromMouseToConsole(Mouse& mouse, MyWindow myWin, int indexCount, int startLineNumber, vector<int>& vectorLineNumber){
    return {mouse.getX()/(myWin.wide - indexCount - 1) + indexCount + 1, vectorLineNumber[mouse.getY()] - startLineNumber + mouse.getX()/(myWin.wide - indexCount - 1)};
}
void resetCursor(HANDLE& hConsole){
        setCursorPosition(hConsole, 0, 0);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
}


void moveUp(Text& text, Mouse& mouse, int& startLineNumber, vector<int>& vectorLineNumber, vector<int>& vectorLineCount){

}
void editFile(Text& text){
    system("cls");
    int startLineNumber = 1;
    Mouse mouse;//鼠标在文本中的位置
    OperationStack opstk;//操作栈
    Mode mode=Mode::command;//模式
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);//窗口输出句柄
    vector<int>* vectorLineNumber = new vector<int>, * vectorLineCount = new vector<int>;//记录文本在控制台窗口下行号和行数信息
    string bottomContent = "";//底部显示的信息

    MyWindow myWin = getMyWindow(hConsole);//记录窗口大小信息
    int indexCount = getIndexCount(text);//记录文本最大行数的位数
    initLineInfo(hConsole, text, *vectorLineNumber, *vectorLineCount, indexCount, myWin);
    showUI(hConsole, text, mouse, startLineNumber, myWin, indexCount, "", *vectorLineNumber, *vectorLineCount);
    showContent(hConsole, text, myWin, startLineNumber, *vectorLineNumber, *vectorLineCount, indexCount, mouse, "");
    showCursor(hConsole, mouse, mode, indexCount, *vectorLineNumber, *vectorLineCount, myWin, startLineNumber);
    //关回显
    HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);//窗口输入句柄
	DWORD consoleMode;
	GetConsoleMode(stdIn, &consoleMode);
	SetConsoleMode(stdIn, consoleMode & ~ENABLE_ECHO_INPUT);

    int status = 1;//运行状态
    string commandInput;//记录命令
    string destination;//记录查找串
    vector<Position>* result = new vector<Position>;//记录查找结果
    int temp;//记录输入
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    while(status){
        if(GetConsoleScreenBufferInfo(hConsole, &csbi)){
            if(myWin.wide != csbi.srWindow.Right - csbi.srWindow.Left + 1 || myWin.height != csbi.srWindow.Bottom - csbi.srWindow.Top + 1){
                system("cls");
                myWin = getMyWindow(hConsole);
                initLineInfo(hConsole, text, *vectorLineNumber, *vectorLineCount, indexCount, myWin);
                showUI(hConsole, text, mouse, startLineNumber, myWin, indexCount, bottomContent, *vectorLineNumber, *vectorLineCount);
                showContent(hConsole, text, myWin, startLineNumber, *vectorLineNumber, *vectorLineCount, indexCount, mouse, bottomContent);
                showCursor(hConsole, mouse, mode, indexCount, *vectorLineNumber, *vectorLineCount, myWin, startLineNumber);
            }
        }
        switch (mode){
            case Mode::command:
                if(_kbhit()){
                    temp = _getch();
                    //特殊按键
                    if(temp == 0|| temp == 224){
                        temp = _getch();
                        switch(temp){
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
                            default:break;//其他特殊按键不理会
                        }
                    }
                    //普通可显示字符
                    else if(temp >= '\32' && temp <= '\126'){
                        if(commandInput.size() == 0){
                            switch(temp){
                                case ':':
                                    commandInput+=temp;
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
                                switch(temp){
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
                                switch(temp){
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
                                switch(temp){
                                    case '\n':
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        else {
                            if(temp == '\n'){

                            }
                            else{

                            }
                        }
                    }
                    //esc
                    else if(temp == 27){
                        commandInput = "";
                        break;
                    }
                    //其他字符不理会
                    else break;
                }
            case Mode::insert:
                if(_kbhit()){
                    temp = _getch();
                    //特殊按键
                    if(temp == 0|| temp == 224){
                        temp = _getch();
                        switch(temp){
                            case '\73'://pgup
                            break;
                            case '\81'://padown
                            break;
                            case '71'://home
                            break;
                            case '\224'://end
                            break;
                            case '\83'://delete
                            break;
                            case '\72'://上
                            break;
                            case '\80'://下
                            break;
                            case '75'://左
                            break;
                            case '77'://右
                            break;
                            default:break;
                        }
                    }
                    //普通可显示字符
                    else if(temp >= 32 && temp <= 126){
                        
                    }
                    //esc
                    else if(temp == 27){
                        mode=Mode::command;
                        showUI(hConsole, text, mouse, startLineNumber, myWin, indexCount, "", *vectorLineNumber, *vectorLineCount);
                        showCursor(hConsole, mouse, mode, indexCount, *vectorLineNumber, *vectorLineCount, myWin, startLineNumber);
                        break;
                    }
                    //backspace
                    else if(temp == 8){
                    }
                    //enter
                    else if(temp == 10){
                    }
                    //其他字符不理会
                    else break;
                }
            }
    }

    resetCursor(hConsole);
    delete vectorLineNumber;
    delete vectorLineCount;
    system("cls");
}





