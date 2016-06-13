#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<stack>
#include<set>
#include<cstring>
#include<queue>

using namespace std;

static const int MOVE = -1;
static const int SPEC = -2;
static const int ACC = -3;


map<char, int> chNum;
// 存储字符
char findChars[100];
// 存储产生式
vector<string> expSet;
// 预测分析表
int table[30][30];
// 类别存储表
int typeTable[30][30];
// 类别index
int num = 0;
int numvt = 0;
// 某终结符在左边的产生式集合
string leftExpSet[100];
// 每个符号的first集
string first[100];
// 是否已经完成FIRST集合
bool isFirst[100];
// 链路状态
int linkType[100][3];
int head[100];
// 链式前向星项目集族图
int linkIndex = 0;
string word;


struct expression {
    // 第几条产生式 点的位置
    int expNum, pointIndex;
    string point;
};

// 项目集族
vector<vector<expression>> LRList;

// 重设表
void resetTable() {
    for (int i = 0; i < 100; i++)
        head[i] = MOVE;
    for (int i = 0; i < 30; i++)
        for (int j = 0; j < 30; j++)
            typeTable[i][j] = table[i][j] = MOVE;
    linkIndex = 0;
}


void getStringFromKeyBoard() {
    memset(table, MOVE, sizeof(table));
    chNum['#'] = 0;
    findChars[0] = '#';
    cout << "请输入终结符集：" << endl;
    char x;
    do {
        cin >> x;
        chNum[x] = ++num;
        findChars[num] = x;
    } while (cin.peek() != '\n');
    numvt = ++num;
    chNum['@'] = numvt;        //kong zi
    findChars[num] = ('@');
    cout << "请输入非终结符集：" << endl;
    do {
        cin >> x;
        chNum[x] = ++num;
        findChars[num] = x;
    } while (cin.peek() != '\n');
    cout << "输入所有产生式（空字用‘@’表示）,以‘end’结束:" << endl;
    string pro;
    while (cin >> pro && pro != "end") {
        string ss;
        ss += pro[0];
        for (int i = 3; i < pro.size(); i++) {
            if (pro[i] == '|') {
                expSet.push_back(ss);
                ss.clear();
                ss += pro[0];
            }
            else {
                ss += pro[i];
            }
        }
        expSet.push_back(ss);
    }
}

void scanfExpSet() {
    for (int i = 0; i < expSet.size(); i++) {
        int temp = chNum[expSet[i][0]];
        leftExpSet[temp] += char('0' + i);
    }
}

// 当前的符号，和对应产生式编号
void getFirst(int ch, int expNum) {
    int temp = chNum[expSet[expNum][1]];  //产生式推出来的首符
    isFirst[expNum] = 1;               //标记
    if (temp <= numvt)first[ch] += char('0' + temp);  //是终结符
    else {
        for (int i = 0; i < leftExpSet[temp].size(); i++)    //所有temp可以推出来的符号对应的产生式
        {
            if (expSet[expNum][0] == expSet[expNum][1])continue; //左递归的产生式不用不影响求fisrt集
            getFirst(temp, leftExpSet[temp][i] - '0');
        }

        first[ch] += first[temp];                  //回溯时候沿途保存
    }
}

void First() {
    // 终结符
    for (int i = 1; i <= numvt; i++) {
        first[i] = char('0' + i);
    }
    for (int i = 0; i < expSet.size(); i++) {
        // 处理左递归
        if (expSet[i][0] == expSet[i][1])continue;
        if (isFirst[i])continue;
        int temp = chNum[expSet[i][0]];
        getFirst(temp, i);
    }
}

// 添加一条链接线
void addLink(int from, int to, int w) {
    linkType[linkIndex][0] = to;
    linkType[linkIndex][1] = head[from];
    head[from] = linkIndex;
    // 这是指通过符号
    linkType[linkIndex++][2] = w;
}

inline bool xmeq(expression a, expression b) {
    if (a.point == b.point && a.pointIndex == b.pointIndex && a.expNum == b.expNum)return 1;
    return 0;
}

// 表达式在项目集中么?
bool isExpIn(expression a, vector<expression> b) {
    for (int i = 0; i < b.size(); i++) {
        if (xmeq(a, b[i]))return 1;
    }
    return 0;
}

// 合并项目集
vector<expression> mergePro(vector<expression> a, vector<expression> b) {
    for (int i = 0; i < b.size(); i++) {
        if (isExpIn(b[i], a))
            continue;
        else
            a.push_back(b[i]);
    }
    return a;
}

// 判断是否相等
bool isProEqual(vector<expression> a, vector<expression> b) {
    if (a.size() != b.size())return 0;
    for (int i = 0; i < a.size(); i++) {
        if (!isExpIn(a[i], b))return 0;
    }
    return 1;
}

// 查找项目编号
int getTheProNum(vector<expression> a,
                 vector<vector<expression> > b) {
    for (int i = 0; i < b.size(); i++) {
        if (isProEqual(a, b[i]))return i;
    }
    return MOVE;
}

vector<expression> get_close(expression t)           //对项目 T作闭包
{
    vector<expression> temp;
    temp.push_back(t);
    queue<expression> q;                         //bfs完成闭包
    q.push(t);
    while (!q.empty()) {
        expression cur = q.front();
        q.pop();
        if (cur.pointIndex == expSet[cur.expNum].size())          //归约项舍去
            continue;
        int tt = chNum[expSet[cur.expNum][cur.pointIndex]];       //tt is thm num of '.'zhihoudefuhao
        if (tt <= numvt) continue;                  //若是终结符，则不必找了
        cout << "leftExpSet size" << leftExpSet[tt].size() << endl;
        cout << expSet[cur.expNum] << " fuck" << endl;
        for (int i = 0; i < leftExpSet[tt].size(); i++)         //对应产生式的编号
        {
            expression c;
            c.pointIndex = 1;                               //
            c.expNum = leftExpSet[tt][i] - '0';             //

            if (expSet[cur.expNum].size() - cur.pointIndex == 1)   // the last : A->BC.D,a/b
                c.point += cur.point;// 参见我的笔记
            else                           // not the last  ：A->B.CFb,a/b
            {
                int tttnum = chNum[expSet[cur.expNum][cur.pointIndex + 1]];

                c.point += first[tttnum];
            }
            if (!isExpIn(c, temp))           // 排重，新的项目就加入。
            {
                q.push(c);
                temp.push_back(c);
            }
        }
    }
    return temp;
}

void print_close(vector<expression> item) {
    for (int i = 0; i < item.size(); i++) {
        cout << i << " : " << endl;
        cout << item[i].pointIndex << " : " << item[i].point << endl;
    }
}

void getExpProSet() {
    vector<expression> temp;
    expression t;
    t.expNum = 0;
    t.pointIndex = 1;
    t.point += '0';    //初始的项目集：0
    temp = get_close(t);
    print_close(temp);
    queue<vector<expression> > q;        //bfs法获得
    q.push(temp);
    LRList.push_back(temp);             //第一个入
    while (!q.empty()) {
        vector<expression> cur = q.front();
        q.pop();
        for (int i = 1; i <= num; i++)     //所有符号
        {
            if (i == numvt)continue;      //'#'
            vector<expression> temp;
            for (int j = 0; j < cur.size(); j++)     //该项目集中的所有项目
            {
                if (cur[j].pointIndex == expSet[cur[j].expNum].size())continue;  //是规约项目，无法再读入了
                int tt = chNum[expSet[cur[j].expNum][cur[j].pointIndex]];
                if (tt == i)                                          //can read in 符号i
                {
                    expression tempt;
                    tempt.point = cur[j].point;
                    tempt.pointIndex = cur[j].pointIndex + 1;
                    tempt.expNum = cur[j].expNum;
                    temp = mergePro(temp, get_close(tempt));
                }
            }
            if (temp.size() == 0)continue;             //该符号无法读入。
            int curNum = getTheProNum(cur, LRList);   //当前节点标号
            int nextNum = getTheProNum(temp, LRList);  //新目标标号
            // 新的项目集
            if (nextNum == MOVE) {
                LRList.push_back(temp);
                q.push(temp);
                addLink(curNum, (int) (LRList.size() - 1), i);   //添加边，权为读入的符号
            }
            else                             //老的项目集
            {
                addLink(curNum, nextNum, i);
            }
        }
    }
}

// 打印项目集族
void printExpPro() {
    for (int i = 0; i < LRList.size(); i++) {
        cout << "项目集" << i << ":" << endl;
        for (int j = 0; j < LRList[i].size(); j++) {
            cout << expSet[LRList[i][j].expNum] << " " << LRList[i][j].pointIndex << " " << LRList[i][j].point << endl;
        }
        cout << endl;
    }
    for (int i = 0; i < LRList.size(); i++) {
        for (int j = head[i]; j != MOVE; j = linkType[j][1]) {
            cout << "  " << findChars[linkType[j][2]] << endl;
            cout << i << "--->" << linkType[j][0] << endl;
        }
    }
}

// 获得分析表 table[i][j]=w:状态i-->j,读入符号W。
bool getTable() {
    // 求移进项目
    for (int i = 0; i < LRList.size(); i++) {
        for (int j = head[i]; j != MOVE; j = linkType[j][1]) {
            if (table[i][linkType[j][2]] != MOVE)
                return 0;
            table[i][linkType[j][2]] = linkType[j][0];
            typeTable[i][linkType[j][2]] = MOVE;
        }
    }
    // 求规约项目
    for (int i = 0; i < LRList.size(); i++) {
        for (int j = 0; j < LRList[i].size(); j++) {
            if (LRList[i][j].pointIndex == expSet[LRList[i][j].expNum].size())  //归约项
            {
                for (int k = 0; k < LRList[i][j].point.size(); k++) {
                    if (table[i][(LRList[i][j].point)[k] - '0'] != MOVE)return 0;           //多重入口，报错.
                    if ((LRList[i][j].point)[k] == '0' && LRList[i][j].expNum == 0)
                        table[i][(LRList[i][j].point)[k] - '0'] = ACC;           // 接受态。
                    else {
                        table[i][(LRList[i][j].point)[k] - '0'] = LRList[i][j].expNum;
                        typeTable[i][(LRList[i][j].point)[k] - '0'] = SPEC;            //归约态
                    }
                }
            }
        }
    }
    return 1;
}

void printTable() {
    cout << "LR(1)分析表：" << endl;
    cout << "状态   " << "         action     " << endl;
    for (int j = 0; j <= num; j++) {
        if (j == numvt)continue;
        cout << "    " << findChars[j];
    }
    cout << endl;
    for (int i = 0; i < LRList.size(); i++) {
        cout << i << "   ";
        for (int j = 0; j <= num; j++) {
            if (j == numvt)continue;
            if (table[i][j] == ACC) cout << "acc" << "  ";       //接受
            else if (table[i][j] == MOVE)cout << "     ";        //空
            else if (typeTable[i][j] == MOVE)cout << "s" << table[i][j] << "   ";  //移近
            else if (typeTable[i][j] == SPEC)cout << "r" << table[i][j] << "   ";  //归约
        }
        cout << endl;
    }
}


void printStateStack(int count, stack<int> state, stack<int> wd, int i) {
    cout << count << '\t' << '\t';
    stack<int> temp;
    while (!state.empty()) {
        temp.push(state.top());
        state.pop();
    }
    while (!temp.empty()) {
        cout << temp.top();
        temp.pop();
    }
    cout << '\t' << '\t';
    while (!wd.empty()) {
        temp.push(wd.top());
        wd.pop();
    }
    while (!temp.empty()) {
        cout << findChars[temp.top()];
        temp.pop();
    }
    cout << '\t' << '\t';
    for (int j = i; j < word.size(); j++)
        cout << word[j];
    cout << '\t' << '\t';
}

bool analyze() {
    cout << "       " << word << "的分析过程：" << endl;
    cout << "步骤\t\t" << "状态栈\t\t" << "符号栈\t\t" << "输入串\t\t" << "动作说明" << endl;
    stack<int> state;
    stack<int> symbol;
    int count = 0;
    state.push(0);     //初始化
    symbol.push(0);        //'#'
    // index 文本指针
    for (int index = 0; ;) {
        int cur = state.top();
        if (table[cur][chNum[word[index]]] == MOVE)    // 空白，报错误
            return 0;
        // acc
        if (table[cur][chNum[word[index]]] == ACC) {
            printStateStack(count++, state, symbol, index);
            cout << " acc! " << endl;
            return 1;
        }
        // 移进项
        if (typeTable[cur][chNum[word[index]]] == MOVE) {
            printStateStack(count++, state, symbol, index);
            int newState = table[cur][chNum[word[index]]];
            cout << "action[" << cur << "," << chNum[word[index]] << "]=" << newState;
            cout << "，状态" << newState << "入栈" << endl;
            symbol.push(chNum[word[index]]);
            state.push(newState);
            index++;
            // 规约
        } else if (typeTable[cur][chNum[word[index]]] == SPEC) {
            printStateStack(count++, state, symbol, index);

            int IndexExp = table[cur][chNum[word[index]]];   //用该产生式归约
            int len = (int) (expSet[IndexExp].size() - 1);
            for (int ii = 0; ii < len; ii++)                 //弹栈
            {
                symbol.pop();
                state.pop();
            }
            symbol.push(chNum[expSet[IndexExp][0]]);    //新入
            cur = state.top();
            cout << "用" << expSet[IndexExp][0] << "->";
            for (int ii = 1; ii <= len; ii++)
                cout << expSet[IndexExp][ii];
            cout << "进行归约," << "goto[" << cur << "," << chNum[word[index]] << "]=" <<
            table[cur][chNum[expSet[IndexExp][0]]];
            cout << "入栈" << endl;
            state.push(table[cur][chNum[expSet[IndexExp][0]]]);
        }
    }
}

int main() {
    // reset
    resetTable();
    // 接受输入
    getStringFromKeyBoard();
    // 扫描生成
    scanfExpSet();
    // 对所有符号求first集
    First();
    getExpProSet();
    if (!getTable()) {
        cout << " 非LR(1)文法！" << endl;
        return 0;
    }

    printExpPro();
    printTable();


    while (true) {
        cout << "请输入表达式：" << endl;
        cin >> word;

        if (word == "exit") {
            break;
        }

        word += '#';
        if (!analyze())
            cout << "error!" << endl;
    }

    return 0;
}
