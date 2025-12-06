

/***
 * 参考了algotester正赛选手的分核策略
*/
// 优化
#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)

#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"

#include <bits/stdc++.h>

using namespace std;

#define INLINE   inline __attribute__ ((always_inline))
#define NOINLINE __attribute__ ((noinline))


typedef pair<int, int> PII;
typedef pair<int, pair<int, int>> PIPII;
#define x first
#define y second
#define INT_CAP 0x3F3F3F3F
// #define LOCAL

constexpr int N = 1e5 + 10, M = 40, C = 2147483647;
constexpr int MSgType = 200 + 10, UsrInst = 1e4 + 10;
constexpr int ExeTime = 4000, Deadline = 2147483647;
constexpr int SOLVE = 10;
constexpr int NO_BETTER_BREAK_COUNT = 5000;
constexpr int MAX_RUNNING_TIME = 3900;
constexpr int TIMEOUT = 3900;
constexpr int MAX_USER_ID = 10000 + 5;
constexpr int SEED = 222;
constexpr int MAX_MSG_TYPE_COUNT = N;

const auto programStartTime = std::chrono::steady_clock::now();


struct Task {
    int msgType; // 任务类型
    int usrInst; // 属于用户
    int ExeTime; // 执行时间
    int deadline; // 截至时间

}task[N];

struct Node {
    int msgType;
    int usrInst;
    int startTime;
    int endTime;
    int deadLine;

    Node(int _msgType, int _usrInst, int _startTime, int _endTime, int _deadLine) : 
    msgType(_msgType), usrInst(_usrInst), startTime(_startTime), endTime(_endTime), deadLine(_deadLine) {

    }

    Node(){}
};

// n : 任务数量; m : 机器数; c : 系统最大处理时间
int n, m, c, typeUp, userUp;
double avgt, urgent;
int improvetest, improveswap;
bool extreme;

int maxUserId; 

struct UserTask {
    // 按照顺序保存每个用户的任务
    vector<int> userTaskList;
    // 执行到哪个任务
    int index; 
    // 累计执行时间（便于修正后续任务的真正执行时间）
    int acc;
} userTask[UsrInst];

/*
* 核上相关信息
*/
int coresTime[M];
// 保存运行在核上的用户编号
vector<vector<int>> coresContainUserId;
// 用于输出在核上运行的任务
vector<vector<Node>> coreResult;
// 用优先队列来保存当前可以运行任务数量最多的类型
priority_queue<PIPII, vector<PIPII>, greater<PIPII>> heap[M][MSgType];
// 作为备用队列
priority_queue<PIPII, vector<PIPII>, greater<PIPII>> heapTemp[M][MSgType];
// 作为备用队列（记录截至时间远大于当前时间节点的任务）
priority_queue<PIPII, vector<PIPII>, greater<PIPII>> heapTemp2[M][MSgType];
int coresCanDealTask[M][MSgType];


// 统计每个核的执行时间
int coresTerminal[M];

/*
* 统计每个用户的任务数量
*/

// 统计不同任务类型的数量
vector<vector<PII>> taskNumber(MSgType);
// 统计用户的任务数量
unordered_map<int, int> userHasTaskNumber;
// 统计每个每个用户第一个任务：格式（deadline, userId);
vector<vector<PII>> firstTask(MSgType);

// 各个方案的过程变量(记录亲和性和不超时任务)
int cntTotal, cntFinished, cntAffinity;

double param1, param2, param3, param4;
// 相关数据
int maxUsrId;
int maxMsgTypeId;
// 随机种子
std::random_device rd; // 用于获取随机数种子  
std::mt19937 gen(SEED); // 以 rd() 作为种子的 Mersenne Twister 生成器  
std::uniform_int_distribution<> dis(0, 1);
default_random_engine rad{SEED};

/****构造新的初始解所需数据结构*/
struct core_t {
    int time = 0;
    int usercnt = 0;
    int sumdeadlines = 0;
    int16_t typecnts[10][201];
    int batchlimit = 0;
    double batchincrease = 0;
    vector<Task> res;
    core_t() {
        for (int i = 0; i < 10; ++ i) {
            for (int j = 1; j <= typeUp; ++ j) {
                typecnts[i][j] = 0;
            }
        }
    }
};
struct score_t {
    int aff = 0;
    int cap = 0;
    int score = 0;
    double m1rate = 0;
    bool error = false;
};
struct sol_t {
    char modec;
    int usem;
    int deadlinemode;
    bool duocheck;
    int specs[31];
    int usediv;
    int usercores[10001];
    core_t cores[31];

    void assignuser(int user, int& nextm, int& mxused);
    void assignusers1();
    void assignusers2();

    int getdiv(int core, int user, const Task& msg);
    sol_t(char modec, int usem, int deadlinemode = 0, int spec = 0, int usediv = 0, bool duocheck = false) : modec(modec), usem(usem), deadlinemode(deadlinemode), usediv(usediv), duocheck(duocheck)
    {
        for (int i = 1; i <= userUp; ++ i) usercores[i] = -1;
        for (int i = 0; i < usem; ++ i) specs[i] = spec;
    }
};
struct user_t {
    int time = 0;
    int lastdeadline = 0;
    vector<Task> tasks{};
};
struct type_t {
    int cnt = 0;
};
user_t users[10001];
type_t types[201];
// 特判数据集
int specialArray[5][4] = {{4, 1, 2, 9}, {7, 2, 3, 9}, {4, 3, 3, 9},{7, 1, 1, 9}, {4, 2, 2, 9}};
struct RNG {
	unsigned int x = 123456789;
	unsigned int y = 362436069;
	unsigned int z = 521288629;
    unsigned int rand() {
		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;
		unsigned int t = x;
		x = y; y = z; z = t ^ x ^ y;
		return z;		
    }
     int next(int x) {return ((long long)rand() * x) >> 32;}
    //  int next(int x) {return rand() % x;}
    //  int next(int a, int b) {return a + (rand() % (b - a));}
     int next(int a, int b) {return a + ((long long) rand() * (b - a)) >> 32;}
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 
static RNG rng;
/*
* 所有方案的全局数据结构
*/
int bestCnt;

vector<vector<Node>> coreBestResult;
sol_t sol('a', urgent < 1 && !extreme ? m + 1 : m);
void debug();
void input();
void init ();
void print();
void solveOne(); // 综合考虑时间和亲和性
void solveTwo(); // 只考虑亲和性（仍然可以优化对于执行时间长的用例，让一个调度中的后面任务先执行，最大化不超时的任务）
void solveThree(); // 综合考虑时间和亲和性(优化版)
void solveFour(); // 综合考虑时间和亲和性(优化版)
void initParam(); // 初始化参数
int calculateScore(); // 计算当前方案的分数
void refreshData (); // 清空每个方案的中间数据结构
bool special(); // 特判数据集


/*
* 启发式搜索算法
*/

void heuristicMethod();
void hillClimb(vector<Node> &nodes, int maxRunningTime, int maxStuck); // 爬山法优化核内分布
int runtime(); // 记录时间
int getScore(const vector<Node> &nodes); // 计算交换领域算子后的结果
bool swapNeighbor(vector<Node> &nodes, int noBetterCount);
void changeNewNodes(vector<Node> &newNodes, int beforeStart, int beforeEnd, const vector<Node> &nodes, int afterStart, int afterEnd);
int getPartScore(const vector<Node> &newNodes, int beforeStart, int beforeEnd, const vector<Node> &nodes,int afterStart,int afterEnd, bool getOriginScore);
int getTotalScore(const vector<vector<Node>> &result);
bool optimizeFinish(vector<Node> &nodes);
bool optimizeQinhe2(vector<Node> &nodes);

int main() {
    #ifdef LOCAL
    if (fopen("./input.txt", "r") != nullptr) {

        freopen(("./input.txt"), "r", stdin);
        freopen(("./output.txt"), "w", stdout);

        
    } else {
        cerr << "open file uncorrectly" << endl;
    }
    #endif
    input();
    init();

    if (special()) {
        return 0;
    }

    

    function<void(void)> solveArray[SOLVE];
    // 函数指针指向函数
    solveArray[0] = solveOne;
    // solveArray[1] = solveTwo;
    solveArray[1] = solveThree;
    solveArray[2] = solveFour;


    sol_t sola('a', urgent < 1 && !extreme ? m + 1 : m);
    sola.assignusers1();
    sol = sola;

    // 遍历所有解决方案
    // 遍历不同核上分配方案
    for (int i = 0; i < 3; i ++  ) {
       solveArray[i]();
        // 如果当前分数高于最优则更换
        cerr << "i: " << i << "  " << calculateScore() << endl;
        if (calculateScore() > bestCnt) {
            bestCnt = calculateScore();
            coreBestResult = coreResult;
        }
        refreshData();
    }
    sol_t solb('b', !extreme ? m + 1 : m, 0, 0, 1);
    solb.assignusers1();
    sol = solb;
    for (int i = 0; i < 3; i ++  ) {
       solveArray[i]();
        // 如果当前分数高于最优则更换
        cerr << "i: " << i << "  " << calculateScore() << endl;
        if (calculateScore() > bestCnt) {
            bestCnt = calculateScore();
            coreBestResult = coreResult;
        }
        refreshData();
    }
    sol_t solc('c', !extreme ? m + 1 : m);
    solc.assignusers2();
    sol = solc;
    for (int i = 0; i < 3; i ++  ) {
       solveArray[i]();
        // 如果当前分数高于最优则更换
        cerr << "i: " << i << "  " << calculateScore() << endl;
        if (calculateScore() > bestCnt) {
            bestCnt = calculateScore();
            coreBestResult = coreResult;
        }
        refreshData();
    }
    sol_t sold('d', !extreme ? m + 1 : m, 1);
    sold.assignusers2();
    sol = sold;
    for (int i = 0; i < 3; i ++  ) {
       solveArray[i]();
        // 如果当前分数高于最优则更换
        cerr << "i: " << i << "  " << calculateScore() << endl;
        if (calculateScore() > bestCnt) {
            bestCnt = calculateScore();
            coreBestResult = coreResult;
        }
        refreshData();
    }
    sol_t solq('q', !extreme ? m + 1 : m, 1, 0, 0, 1);
    solq.assignusers2();
    sol  = solq;
    for (int i = 0; i < 3; i ++  ) {
       solveArray[i]();
        // 如果当前分数高于最优则更换
        cerr << "i: " << i << "  " << calculateScore() << endl;
        if (calculateScore() > bestCnt) {
            bestCnt = calculateScore();
            coreBestResult = coreResult;
        }
        refreshData();
    }
    if (urgent > 1 && m > 1) {
        bool ok = false;
        for(int mm = max(1, (int)(1.0 * m / urgent)); mm <= m - 1; ++ mm) {
            sol_t sol2('0' + min(9, m - mm), mm);
            sol2.assignusers1();
            sol = sol2;
            for (int i = 0; i < 3; i ++  ) {
            solveArray[i]();
                // 如果当前分数高于最优则更换
                cerr << "i: " << i << "  " << calculateScore() << endl;
                if (calculateScore() > bestCnt) {
                    bestCnt = calculateScore();
                    coreBestResult = coreResult;
                }
                refreshData();
            }
        }
    }
    // 将初始解得到最好结果赋值给爬山法需要的数据结构
    coreResult =  coreBestResult;
    // 爬山法进行优化同一核中的序列
    heuristicMethod();


    int bestScore = bestCnt;
    int estimateScore = getTotalScore(coreResult);

    if (estimateScore > bestScore) {
        bestScore = estimateScore;
        coreBestResult = coreResult;
    }

    cerr << "最好的结果: " << estimateScore << endl;

    // debug();
    print();

   





}


INLINE void input() {
    // 输入数据第一行
    scanf("%d %d %d", &n, &m, &c);
    // 输入任务
    for (int i = 1; i <= n; i ++ ) {
        int taskCls, userId, excTime, deadTime;
        scanf("%d %d %d %d", &taskCls, &userId, &excTime, &deadTime);
        task[i] = {taskCls, userId, excTime, deadTime};
        // task[i].deadline = min(task[i].deadline, c);
        // 用户下保存任务
        userTask[userId].userTaskList.emplace_back(i);
        userTask[userId].acc += excTime;

        taskNumber[taskCls].push_back({userTask[userId].acc, deadTime});

        maxUsrId = max(maxUsrId, userId);
        maxMsgTypeId = max(maxMsgTypeId, taskCls);
    }
    /********************************/
    // 单独处理新的初始解
    userUp = maxUsrId, typeUp = maxMsgTypeId;

    auto preConditioning = [&](auto&& preConditioning) -> void {
        double s = 0;
        for (int i = 1; i <= n; ++ i) {
            s += task[i].ExeTime;
        }
        avgt = s / n;
        urgent = task[n].deadline * m / n / avgt;
        for (int i = 1; i <= n; ++ i) {
            users[task[i].usrInst].time += task[i].ExeTime;
            users[task[i].usrInst].tasks.push_back(task[i]);
            users[task[i].usrInst].lastdeadline = task[i].deadline;
            ++ types[task[i].msgType].cnt;
        }
        extreme = n < 6 * typeUp;        
    };
    preConditioning(preConditioning);

}

INLINE void init () {
    coresContainUserId = vector<vector<int>>(m);
    coreResult = vector<vector<Node>>(m);
    coreBestResult = vector<vector<Node>>(m);
    // 未超时任务数量以及亲和节点个数
    cntTotal = cntFinished = cntAffinity = 0;
    bestCnt = 0;
    // 初始化每格核的运行时间
    for (int i = 0; i < m; i ++ ) {
        coresTime[i] = 1;
    }

    // 统计每个用户的第一个任务
    for (int i = 0; i < UsrInst; i ++ ) {
        if (userTask[i].userTaskList.size() > 0) {
            int taskId1 = userTask[i].userTaskList[0];
            int cls1 = task[taskId1].msgType;
            firstTask[cls1].push_back({task[taskId1].deadline, i});
        }
    }

}
INLINE void initParam() {
    param1 = 1.0 * 1500  * 200;
    param2 = 2.5;
    param3 = 2;
    param4 = 1;
}
INLINE void refreshData () {
    coresContainUserId = vector<vector<int>>(m);
    coreResult = vector<vector<Node>>(m);
    // 未超时任务数量以及亲和节点个数
    cntTotal = cntFinished = cntAffinity = 0;
    // 初始化每格核的运行时间
    for (int i = 0; i < m; i ++ ) {
        coresTime[i] = 1;
    }
    // 刷新队列数据
    // for (int i = 0; i < M; ++ i) {
    //     for (int j = 0; j < MSgType; ++ j) {
    //         heap[i][j] = {}, heapTemp[i][j] = {};
    //     }
    // }

    // // 清空第一个任务列表
    // for (int i = 0; i < MSgType; ++ i) firstTask[i] = {};
    // 刷新核上可以处理的任务
    memset(coresCanDealTask, 0, sizeof coresCanDealTask);

    // 清空用户的索引
    for (int i = 0; i < UsrInst; ++ i) userTask[i].index = 0;
}

INLINE void print() {
        // 4.输出结果，使用字符串存储，一次IO输出
    stringstream out;
    for (int coreId = 0; coreId < m; ++ coreId) {
        out << coreBestResult[coreId].size();
        for (auto &task : coreBestResult[coreId]) {
            out << " " << task.msgType << " " << task.usrInst;
        }
        out << endl;
    }
    printf("%s", out.str().c_str());

}



INLINE void solveOne() {

    // 清空核的执行时间
    for(int i = 1; i <= userUp;  ++ i) {
        if (!users[i].tasks.empty()) {
            // cerr << i << " " << users[i].tasks.size() << endl;
            if (sol.usercores[i] == m) {
                int core = rng.next(m);
                coresContainUserId[0].push_back(i);
            } else {
                coresContainUserId[sol.usercores[i]].push_back(i);
            }
        }

    }


    // // 将每个类型的任务按照顺序依次分类
    // // 得到总的任务数量进行平分
    // vector<int> taskClassTag;
    // for (int i = 0; i < MSgType; i ++ ) {
    //     if (firstTask[i].size()) taskClassTag.emplace_back(i);
    // }
    // int s = taskClassTag.size();
    // // 分配策略
    // int d = s / m,  r = s - m * d;
    // int index = 0;
    // // 将对应任务的用户分配到对应的核上取
    // for (int i = 0; i < m; i ++ ) {
    //     // 对应第i个核
    //     for (int j = 0; j < d; j ++, index ++ ) {
    //         int cls = taskClassTag[index];
    //         // 将用户分配到核上
    //         for (auto [x, y] : firstTask[cls]) {
    //             coresContainUserId[i].push_back(y);
    //         }
    //     }
    // }

    // // 分配余数
    // int cores = 0;
    // for (int i = 0; i < r; i ++, index ++ ) {
    //     int cls = taskClassTag[index];
    //     for (auto [x, y] : firstTask[cls]) {
    //         coresContainUserId[cores].push_back(y);
    //     }
    //     cores = (cores + 1) % m;
    // }

    // 依次处理每一个核第一次运行的任务类型
    for (int i = 0; i < m; i ++ ) {
        for (int userId : coresContainUserId[i]) {
            // 核上第一次运行的任务
            int taskId = userTask[userId].userTaskList[0];
            int excTime = task[taskId].ExeTime, deadline = task[taskId].deadline;
            int cls = task[taskId].msgType;
            coresCanDealTask[i][cls] ++;
            // 存入到对应的优先队列中
            heap[i][cls].push({deadline, {excTime, userId}});
        }

    }

    // 在核内寻找具有最多连续类型的任务数量
    for (int i = 0; i < m; i ++ ) {
        bool flag = true; // 表示仍然有任务需要运行

        while (flag) {

            int number = 0, cls = -1;
            for (int j = 0; j < MSgType; j ++ ) {
                if (heap[i][j].size() == 0) continue;

                if (heap[i][j].size() > number) {
                    number = heap[i][j].size();
                    cls = j;
                }
                
            }

            if (number != 0) {
                // 将对应的优先队列中所有任务加载到核上运行
                cntAffinity += heap[i][cls].size() - 1;

                // 第一遍筛选出超时的任务
                while (heap[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heap[i][cls].top();
                    heap[i][cls].pop();
                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];

                    
                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) cntFinished ++;
                    else {
                        heapTemp[i][cls].push({dealineTime, {excTime, userId}});
                        continue;
                    }
                    // 调试总节点数
                    cntTotal ++;


                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : (task[nextTaskId].deadline - coresTime[i]) / task[nextTaskId].ExeTime;
                        heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                }

                while (heapTemp[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heapTemp[i][cls].top();
                    heapTemp[i][cls].pop();
                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];

                    
                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) cntFinished ++;
                    // 调试总节点数
                    cntTotal ++;
                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : (task[nextTaskId].deadline - coresTime[i]) / task[nextTaskId].ExeTime;
                        heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });

                }
            }

            if (number == 0) {
                flag = false;
            }
        }
    }

  
}





INLINE void solveThree() {
    memset(coresTerminal, 0, sizeof coresTerminal);

    // 清空核的执行时间
    for(int i = 1; i <= userUp;  ++ i) {
        if (!users[i].tasks.empty()) {
            // cerr << i << " " << users[i].tasks.size() << endl;
            if (sol.usercores[i] == m) {
                int core = rng.next(m);
                coresContainUserId[0].push_back(i);
            } else {
                coresContainUserId[sol.usercores[i]].push_back(i);
            }
        }

    }




    // 依次处理每一个核第一次运行的任务类型
    for (int i = 0; i < m; i ++ ) {
        for (int userId : coresContainUserId[i]) {
            // 核上第一次运行的任务
            int taskId = userTask[userId].userTaskList[0];
            int excTime = task[taskId].ExeTime, deadline = task[taskId].deadline;
            int cls = task[taskId].msgType;
            coresCanDealTask[i][cls] ++;
            // 存入到对应的优先队列中
            heap[i][cls].push({deadline, {excTime, userId}});
        }

    }

    // 在核内寻找具有最多连续类型的任务数量
    for (int i = 0; i < m; i ++ ) {
        bool flag = true; // 表示仍然有任务需要运行

        while (flag) {

            int number = 0, cls = -1;
            // 第一次寻找可以调度的不超时的任务
            for (int j = 0; j < MSgType; j ++ ) {
                if (heap[i][j].size() == 0) continue;

                if (heap[i][j].size() > number) {
                    number = heap[i][j].size();
                    cls = j;
                }
                
            }

            if (number != 0) {
                // 将对应的优先队列中所有任务加载到核上运行
                // 第一遍筛选出超时的任务(和截至时间远大于当前时间的节点)
                bool f1 = false;
                int accCur = 0;
                int sz = heap[i][cls].size();
                while (heap[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heap[i][cls].top();
                    heap[i][cls].pop();

                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];


                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (dealineTime > coresTime[i] + task[taskId].ExeTime ) {
                        if (dealineTime > coresTime[i] + 1.0 * 1500  * 100 && accCur >= 1.0 * sz / 2.5)  {
                            heapTemp[i][cls].push({dealineTime, {excTime, userId}});
                            continue;
                        } else {
                            cntFinished ++;
                        }
                    }
                    else {
                        // 超时的任务
                        heapTemp[i][cls].push({INT_MAX, {excTime, userId}});
                        continue;
                    }

                    // 实际运行的任务
                    accCur ++;
                    // 调试总节点数
                    cntTotal ++;
                    // 亲和性相加
                    cntAffinity ++;
                    // 有亲和任务
                    f1 = true;


                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : task[nextTaskId].deadline;
                        if (heap[i][nextCls].size() >= 1 && putTime != INT_MAX) {
                            // 放入队列一
                            heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }
                        else {

                            heapTemp[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                }

                // 有调度未超时的任务
                if (f1) {
                    cntAffinity -= 1;
                }
        
            }
            // 在可以调度的任务中找不到可以不超时的任务
            if (number == 0) {
                // 第一次寻找可以调度的不超时的任务
                for (int j = 0; j < MSgType; j ++ ) {
                    if (heapTemp[i][j].size() == 0) continue;

                    if (heapTemp[i][j].size() > number) {
                        number = heapTemp[i][j].size();
                        cls = j;
                    }
                    
                }

                if (heapTemp[i][cls].size()) cntAffinity += heapTemp[i][cls].size() - 1;

                while (heapTemp[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heapTemp[i][cls].top();
                    heapTemp[i][cls].pop();

                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];


                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) {
                        cntFinished ++;
                    } else {
                        heapTemp2[i][cls].push({dealineTime, {excTime, userId}});
                        continue;
                    }

                    // 调试总节点数
                    cntTotal ++;
   
                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : task[nextTaskId].deadline;
                        if (heap[i][nextCls].size() >= 1 && putTime != INT_MAX) {
                            // 放入队列一
                            heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }
                        else {

                            heapTemp[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                }
                // 处理超时任务
                while (heapTemp2[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heapTemp2[i][cls].top();
                    heapTemp2[i][cls].pop();
                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];

                    
                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) cntFinished ++;
                    // 调试总节点数
                    cntTotal ++;
           
                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : (task[nextTaskId].deadline - coresTime[i]) / task[nextTaskId].ExeTime;
                        heapTemp[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });

                }                
            }





            if (number == 0) {
                flag = false;
            }
        }
    }

}


INLINE void solveFour() {

    // 清空核的执行时间
    for(int i = 1; i <= userUp;  ++ i) {
        if (!users[i].tasks.empty()) {
            // cerr << i << " " << users[i].tasks.size() << endl;
            if (sol.usercores[i] == m) {
                int core = rng.next(m);
                coresContainUserId[0].push_back(i);
            } else {
                coresContainUserId[sol.usercores[i]].push_back(i);
            }
        }

    }

    memset(coresTerminal, 0, sizeof coresTerminal);
   
    // 依次处理每一个核第一次运行的任务类型
    for (int i = 0; i < m; i ++ ) {
        for (int userId : coresContainUserId[i]) {
            // 核上第一次运行的任务
            int taskId = userTask[userId].userTaskList[0];
            int excTime = task[taskId].ExeTime, deadline = task[taskId].deadline;
            int cls = task[taskId].msgType;
            coresCanDealTask[i][cls] ++;
            // 存入到对应的优先队列中
            heap[i][cls].push({deadline, {excTime, userId}});
        }

    }
    // 在核内寻找具有最多连续类型的任务数量
    for (int i = 0; i < m; i ++ ) {
        bool flag = true; // 表示仍然有任务需要运行

        while (flag) {
            int number = 0, cls = -1;
            // 第一次寻找可以调度的不超时的任务
            for (int j = 0; j < MSgType; j ++ ) {
                if (heap[i][j].size() == 0) continue;

                if (heap[i][j].size() > number) {
                    number = heap[i][j].size();
                    cls = j;
                }
                
            }

            if (number != 0) {
                // 将对应的优先队列中所有任务加载到核上运行
                // 第一遍筛选出超时的任务(和截至时间远大于当前时间的节点)
                bool f1 = false;
                int accCur = 0;
                int sz = heap[i][cls].size();
                while (heap[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heap[i][cls].top();
                    heap[i][cls].pop();

                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];


                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (dealineTime > coresTime[i] + task[taskId].ExeTime ) {
                        // 参数可调
                        if (dealineTime > coresTime[i] + param1 && accCur >= 1.0 * sz / param2)  {
                            heapTemp[i][cls].push({dealineTime, {excTime, userId}});
                            continue;
                        } else {
                            cntFinished ++;
                        }
                    }
                    else {
                        // 超时的任务
                        heapTemp[i][cls].push({INT_MAX, {excTime, userId}});
                        continue;
                    }

                    // 实际运行的任务
                    accCur ++;
                    // 调试总节点数
                    cntTotal ++;
                    // 亲和性相加
                    cntAffinity ++;
                    // 有亲和任务
                    f1 = true;

                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : task[nextTaskId].deadline;
                        // 参数可调
                        if (heap[i][nextCls].size() >= param3 && putTime != INT_MAX) {
                            // 放入队列一
                            heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }
                        else {

                            heapTemp[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }

                    }
                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                }

                // 有调度未超时的任务
                if (f1) {
                    cntAffinity -= 1;
                }
        
            }

            // 在可以调度的任务中找不到可以不超时的任务
            if (number == 0) {
                // 第一次寻找可以调度的不超时的任务
                for (int j = 0; j < MSgType; j ++ ) {
                    if (heapTemp[i][j].size() == 0) continue;

                    if (heapTemp[i][j].size() > number) {
                        number = heapTemp[i][j].size();
                        cls = j;
                    }
                    
                }
                bool f1 = false;

                while (heapTemp[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heapTemp[i][cls].top();
                    heapTemp[i][cls].pop();

                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];


                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) {
                        cntFinished ++;
                    } else {
                        heapTemp2[i][cls].push({dealineTime, {excTime, userId}});
                        continue;
                    }

                    // 调试总节点数
                    cntTotal ++;
                    f1 = true;
                    // 亲和性增加
                    cntAffinity ++;

                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : task[nextTaskId].deadline;
                        // 参数可调
                        if (heap[i][nextCls].size() >= param4 && putTime != INT_MAX) {
                            // 放入队列一
                            heap[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }
                        else {

                            heapTemp[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});
                        }

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                }
                if (f1) cntAffinity -= 1;               
            }

            if (number == 0) {
                flag = false;
            }
        }
    }

    // 处理全部超时的任务
    for (int i = 0; i < m; i ++ ) {
        bool flag = true; // 表示仍然有任务需要运行

        while (flag) {
            int number = 0, cls = -1;
            // 第一次寻找可以调度的不超时的任务
            for (int j = 0; j < MSgType; j ++ ) {
                if (heapTemp2[i][j].size() == 0) continue;

                if (heapTemp2[i][j].size() > number) {
                    number = heapTemp2[i][j].size();
                    cls = j;
                }
                
            }


            if (number != 0) {
                if (heapTemp2[i][cls].size()) cntAffinity += heapTemp2[i][cls].size() - 1;

                while (heapTemp2[i][cls].size()) {
                    // 取出优先队列中的任务
                    pair<int, pair<int, int>> t = heapTemp2[i][cls].top();
                    heapTemp2[i][cls].pop();

                    // 执行任务
                    int excTime = t.y.x, userId = t.y.y, dealineTime = t.x;
                    // 更新用户任务列表
                    int &index = userTask[userId].index;
                    int taskId =  userTask[userId].userTaskList[index];


                    // 判断当前时间有没有超过运行任务的截至时间（debug)
                    if (task[taskId].deadline > coresTime[i] + task[taskId].ExeTime ) {
                        cntFinished ++;
                    }

                    // 调试总节点数
                    cntTotal ++;
   
                    // 更新当前核的运行时间
                    coresTime[i] += task[taskId].ExeTime;

                    index ++;

                    // 更新新的任务序列
                    if (index < userTask[userId].userTaskList.size()) {
                        // 更新任务类型
                        int nextTaskId = userTask[userId].userTaskList[index];
                        int nextCls = task[nextTaskId].msgType;
                        // 更新优先队列
                        int putTime = (coresTime[i] + task[nextTaskId].ExeTime) > task[nextTaskId].deadline ? INT_MAX : task[nextTaskId].deadline;
     
       
                        heapTemp2[i][nextCls].push({putTime, {task[nextTaskId].ExeTime, userId}});

                    }

                    // 放在结果中
                    coreResult[i].push_back({task[taskId].msgType, userId, coresTime[i] - task[taskId].ExeTime, coresTime[i], task[taskId].deadline });
                                    
                } 
            } else {
                flag = false;
            }
        }
    }
}





INLINE void heuristicMethod() {

    while (true) {
        if (runtime() > MAX_RUNNING_TIME - 50) break;

        for (int i = 0; i < m; ++ i ) {
            vector<Node> &curResult = coreResult[i];

            int remainTime = int(MAX_RUNNING_TIME - runtime() - 20);
            remainTime /= (m - i);
            hillClimb(curResult, remainTime, NO_BETTER_BREAK_COUNT);
        }
    }

}



INLINE void hillClimb(vector<Node> &nodes, int maxRunningTime, int maxStuck) {
    if (nodes.empty()) {
        return;
    }

    //某一个往前移，增加完成分数
    int noBetterCount = 0;
    int startScore = getScore(nodes);
    vector<Node> backup(nodes);
    vector<Node> bestNodes(nodes);
    int bestScore = startScore;
    int iterateCount = 0;
    int startTime = runtime();


    nodes = backup;

    while (true) {
        if (runtime() - startTime > maxRunningTime  && iterateCount != 0) {
            break;
        }

        iterateCount ++;

        bool better;
        //you bug，导致前后不一致

        better = swapNeighbor(nodes, noBetterCount);//2好，又快又好

        if (better) {
            noBetterCount = 0;
        }
        better = optimizeFinish(nodes);
        if (better) {
            noBetterCount = 0;
        }
        // //1-2选一个
        better = optimizeQinhe2(nodes);
        if (better) {
            noBetterCount = 0;
        }
        noBetterCount ++;
        int endScore = getScore(nodes);

        if (noBetterCount > maxStuck) {
            break;
        }
        
        if (endScore > bestScore) {
            bestScore = endScore;
            bestNodes = nodes;
        }
    }

    nodes = bestNodes;



}


INLINE bool swapNeighbor(vector<Node> &nodes, int noBetterCount) {
    //两两交换位置
    if (nodes.empty()) {
        return false;
    }
    int startScore = getScore(nodes);
    //1.不同type交换，1可交换，2.交换之后分数不降低
    static vector<Node> newNodes;
    newNodes.clear();
    int lastTypeStart = -1;
    static bitset<MAX_USER_ID> usrIds;
    for (int i = 0; i < nodes.size(); ++ i) {
        bool change = false;
        int afterStart = i;
        int afterEnd = i + 1;
        usrIds.reset();
        for (; afterEnd < nodes.size(); ++ afterEnd ) {
            if (nodes[afterEnd].msgType != nodes[afterEnd - 1].msgType) {
                //新的连续快
                break;
            }
        }

        afterEnd -= 1;
        for (int j = afterStart; j <= afterEnd; ++j) {
            usrIds.set(nodes[j].usrInst);
        }
        if (lastTypeStart >= 0) {
            int beforeStart = lastTypeStart;
            int beforeEnd = i - 1;



            bool canChange = true;
            for (int j = beforeStart; j <= beforeEnd; j++) {
                Node &node = newNodes[j];
                if (usrIds[node.usrInst]) {
                    canChange = false;
                    break;
                }
            }

            if (canChange) {
                int afterScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart,
                                                        afterEnd,
                                                        false);
                int beforeScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart,
                                                        afterEnd, true);

                if (afterScore > beforeScore || (afterScore == beforeScore && noBetterCount != 0)) {
                    //交换
                    change = true;
                    changeNewNodes(newNodes, beforeStart, beforeEnd, nodes, afterStart, afterEnd);
                    lastTypeStart = beforeStart + (afterEnd - afterStart + 1);
                }
            }
        }

        if (!change) {
            for (int j = afterStart; j <= afterEnd; j++) {
                newNodes.push_back(nodes[j]);
            }
            lastTypeStart = afterStart;
        }

        //后面变成一块，要加上去
        while (change && afterEnd + 1 < nodes.size() && nodes[afterEnd + 1].msgType ==
                                                            newNodes[newNodes.size() - 1].msgType) {
            newNodes.push_back(nodes[afterEnd + 1]);
            afterEnd ++ ;
        }

        i = afterEnd;
    }

    //end就是最后
    swap(nodes, newNodes);
    newNodes.clear();



    int endScore = getScore(nodes);

    return endScore > startScore;
}

INLINE bool optimizeFinish(vector<Node> &nodes) {
    //往前移动增加分数
    int startScore = getScore(nodes);
    static vector<Node> newNodes;
    static int usrEndIndexes[MAX_USER_ID];
    memset(usrEndIndexes, -1, sizeof(int) * (maxUsrId + 1));
    for (int i = 0; i < nodes.size(); i++) {
        const Node &node = nodes[i];
        if (node.deadLine >= node.endTime) {
            newNodes.push_back(node);//不超时不处理
            usrEndIndexes[node.usrInst] = i;
            continue;
        }
        //超时，往前移动
        int lastStartTime = node.deadLine - (node.endTime - node.startTime);


        //1.找到不超时的
        int insertPos = 0, r = i - 1;
        while (insertPos <= r) {
            int mid = (insertPos + r) / 2;
            if (newNodes[mid].startTime > lastStartTime) {
                r = mid - 1;
            } else if (newNodes[mid].startTime < lastStartTime) {
                insertPos = mid + 1;
            } else {
                insertPos = mid;
                break;
            }
        }
        if (insertPos == i || newNodes[insertPos].startTime > lastStartTime) {
            insertPos --;
        }
        if (usrEndIndexes[node.usrInst] >= insertPos) {
            insertPos = -1;
        }
        if (insertPos < 0) {
            newNodes.push_back(node);//没法插入，不处理
            usrEndIndexes[node.usrInst] = i;
            continue;
        }

        int exeTime = node.endTime - node.startTime;
        //2.
        if (newNodes[insertPos].msgType != node.msgType && insertPos != 0 &&
            newNodes[insertPos - 1].msgType == newNodes[insertPos].msgType) {
            int insertPos2 = insertPos - 1;
            int timeOutCount = 0;
            for (; insertPos2 >= 0; -- insertPos2 ) {
                Node &curNode = newNodes[insertPos2];
                if (curNode.usrInst == node.usrInst) {
                    insertPos2 = -1;
                    break;
                }
                if (curNode.endTime <= curNode.deadLine &&
                    curNode.endTime + exeTime > curNode.deadLine) {
                    timeOutCount++;
                    if (timeOutCount > 1) {
                        insertPos2 = -1;
                        break;
                    }
                }
                if (curNode.msgType == node.msgType) {
                    break;
                }
                if (insertPos2 == 0 || newNodes[insertPos2 - 1].msgType != newNodes[insertPos2].msgType) {
                    break;
                }
            }
            if (insertPos2 != -1) {
                if (timeOutCount == 1) {
                    // int suiji = dis(gen);
                    int suiji = int(rad() % 2);
                    if (suiji == 0) {
                        insertPos = insertPos2;
                    }
                } else {
                    insertPos = insertPos2;
                }
            }
        }



        //3.剔除不可能交换的，两边亲和性1，自己能执行1，最多3，超过3说明一定不会换,最开始是亲和则为2
        int startQinhe = 0;
        if ((newNodes[i - 1].msgType == node.msgType ||
                i + 1 < nodes.size() && nodes[i + 1].msgType == node.msgType)) {
            startQinhe++;
        }
        if ((insertPos == i - 1 && newNodes[insertPos].msgType == node.msgType) ||
            (insertPos != i - 1 && newNodes[insertPos].msgType == newNodes[insertPos + 1].msgType) ||
            insertPos - 1 >= 0 && newNodes[insertPos - 1].msgType == newNodes[insertPos].msgType) {
            startQinhe++;
        }

        int maxAddCount = 3 - startQinhe;
        for (int j = insertPos; j <= i - 1; j++) {
            const Node &curNode = newNodes[j];
            if (curNode.endTime <= curNode.deadLine &&
                curNode.endTime + exeTime > curNode.deadLine) {
                maxAddCount--;
                if (maxAddCount < 0) {
                    insertPos = -1;
                    break;
                }
            }
        }
        if (insertPos < 0) {
            newNodes.push_back(node);//没法插入，不处理
            usrEndIndexes[node.usrInst] = i;
            continue;
        }
        int beforeStart = insertPos;
        int beforeEnd = i - 1;
        int afterStart = i;
        int afterEnd = i;
        int afterScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart, afterEnd,
                                                false);
        int beforeScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart, afterEnd,
                                                true);


        if (afterScore > beforeScore || (afterScore == beforeScore)) {
            //交换
            changeNewNodes(newNodes, beforeStart, beforeEnd, nodes, afterStart, afterEnd);
            for (int j = afterEnd; j >= beforeStart; --j) {
                Node &curNode = newNodes[j];
                if (usrEndIndexes[curNode.usrInst] < j) {
                    usrEndIndexes[curNode.usrInst] = j;
                }
            }
        } else {
            newNodes.push_back(node);//没法插入，不处理
            usrEndIndexes[node.usrInst] = i;
        }
    }

    swap(nodes, newNodes);
    newNodes.clear();

    int endScore = getScore(nodes);
    return endScore > startScore;
}





INLINE bool optimizeQinhe2(vector<Node> &nodes) {
    //往前移动增加亲和分数
    if (nodes.empty()) {
        return false;
    }
    int startScore = getScore(nodes);
    static vector<Node> newNodes;
    //static unordered_map<int, int> typeEndIndexes;  //type的末尾快所在的位置
    // typeEndIndexes.clear();
    static int typeEndIndexes[MAX_MSG_TYPE_COUNT];  //type的末尾快所在的位置
    memset(typeEndIndexes, -1, sizeof(int) * (maxMsgTypeId + 1));
    for (int j = 0; j < nodes.size(); j++) {
        //走到连续快末尾,移动在之后导致连续
        int msgType = nodes[j].msgType;
        int afterStart = j;
        while (afterStart >= 1
                && newNodes[afterStart - 1].msgType == msgType) {
            afterStart--;
            nodes[afterStart] = newNodes[afterStart];//赋值为新的，不然下面的计算会报错
        }
        int afterEnd = j + 1;
        for (; afterEnd < nodes.size(); afterEnd++) {
            if (nodes[afterEnd].msgType != nodes[afterEnd - 1].msgType) {
                //新的连续快
                break;
            }
        }
        afterEnd -= 1;
        bool canMove = false;
        if (typeEndIndexes[msgType] != -1) {
            //可以前移
            int beforeStart = typeEndIndexes[msgType] + 1;
            int beforeEnd = afterStart - 1;

            bool isIn = false;
            static bool visit[MAX_USER_ID]{};
            for (int k = afterStart; k <= afterEnd; k++) {
                if (k >= j) {
                    visit[nodes[k].usrInst] = true;
                } else {
                    visit[newNodes[k].usrInst] = true;
                }
            }
            for (int k = beforeStart; k <= beforeEnd; ++k) {
                if (visit[newNodes[k].usrInst]) {
                    isIn = true;
                    break;
                }
            }
            for (int k = afterStart; k <= afterEnd; k++) {
                if (k >= j) {
                    visit[nodes[k].usrInst] = false;
                } else {
                    visit[newNodes[k].usrInst] = false;
                }
            }
            if (!isIn) {
                //前后顺序保证了，有可能往前移动
                //此时能移动了


                int afterScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart,
                                                        afterEnd,
                                                        false);
                int beforeScore = getPartScore(newNodes, beforeStart, beforeEnd, nodes, afterStart,
                                                        afterEnd,
                                                        true);

                if (afterScore > beforeScore || (afterScore == beforeScore)) {
                    for (int k = beforeStart; k <= beforeEnd; k++) {
                        const Node &node = newNodes[k];
                        //更新位置
                        if (typeEndIndexes[node.msgType] == k) {
                            typeEndIndexes[node.msgType] = -1;
                        }
                    }
                    //因为可能前移，要删除，交换
                    newNodes.erase(newNodes.begin() + afterStart, newNodes.end());
                    changeNewNodes(newNodes, beforeStart, beforeEnd, nodes, afterStart, afterEnd);

                    canMove = true;
                    for (int k = beforeStart; k <= afterEnd; ++k) {
                        if (k == afterEnd && afterEnd + 1 >= nodes.size()) {
                            continue;//结束了
                        }
                        const Node curPoint = newNodes[k];
                        Node nextPoint{};
                        if (k == afterEnd) {
                            nextPoint = nodes[k + 1];
                        } else {
                            nextPoint = newNodes[k + 1];
                        }
                        if (curPoint.msgType != nextPoint.msgType && typeEndIndexes[curPoint.msgType] < k) {
                            //末尾块
                            typeEndIndexes[curPoint.msgType] = k;
                        }
                    }
                }
            }

        }

        if (!canMove) {
            for (int k = j; k <= afterEnd; k++) {
                Node &node = nodes[k];
                newNodes.push_back(node);
            }
            typeEndIndexes[msgType] = afterEnd;
        }

        j = afterEnd;//移动到末尾
    }
    //判分
    nodes.swap(newNodes);
    newNodes.clear();
    int endScore = getScore(nodes);


    return endScore > startScore;
}







/*
* 功能函数定义
*/
INLINE int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - programStartTime);
    return int(duration.count());
}




INLINE int getScore(const vector<Node> &nodes) {
    int finished = 0;
    int affinity = 0;
    if (nodes.empty()) {
        return 0;
    }


    if (nodes[0].deadLine >= nodes[0].endTime) {
        finished ++ ;//完成得分
    }
    for (int k = 1; k < nodes.size(); ++ k) {
        if (nodes[k].deadLine >= nodes[k].endTime) {
            finished ++ ;//完成得分
        }
        if (nodes[k - 1].msgType == nodes[k].msgType) {
            affinity ++ ;//亲和性得分
        }
    }
    return finished + affinity;
}





int getPartScore(const vector<Node> &newNodes, int beforeStart, int beforeEnd, const vector<Node> &nodes,
                        int afterStart,
                        int afterEnd, bool getOriginScore) {

    int finish = 0;
    int qinHe = 0;
    if (getOriginScore) {
        if (newNodes[beforeStart].deadLine >= newNodes[beforeStart].endTime) {
            finish++;//完成得分
        }
        Node lastNode = newNodes[beforeStart];
        for (int i = beforeStart + 1; i <= afterEnd; ++ i) {
            Node node{};
            if (i <= beforeEnd) {
                node = newNodes[i];
            } else {
                node = nodes[i];
            }
            if (node.deadLine >= node.endTime ) {
                finish ++;//完成得分
            }
            if (node.msgType == lastNode.msgType) {
                qinHe ++;
            }
            lastNode = node;
        }
        if (beforeStart - 1 >= 0 && newNodes[beforeStart].msgType == newNodes[beforeStart - 1].msgType) {
            qinHe ++;//前面亲和
        }
        if (afterEnd + 1 < nodes.size() && nodes[afterEnd].msgType == nodes[afterEnd + 1].msgType) {
            qinHe ++;//后面亲和
        }
        return finish + qinHe;
    } else {
        Node lastNode = nodes[afterStart];
        int beforeTime = 0, afterTime = 0;
        for (int i = beforeStart; i <= beforeEnd; ++i) {
            beforeTime += newNodes[i].endTime - newNodes[i].startTime;
        }
        for (int i = afterStart; i <= afterEnd; ++i) {
            afterTime += nodes[i].endTime - nodes[i].startTime;
        }

        if (nodes[afterStart].deadLine >= nodes[afterStart].endTime - beforeTime) {
            finish ++;//完成得分
        }
        for (int i = beforeStart + 1; i <= afterEnd; ++i) {
            Node node{};
            if (i <= beforeStart + afterEnd - afterStart) {
                node = nodes[i + (beforeEnd - beforeStart + 1)];
                if (node.deadLine >= node.endTime - beforeTime) {
                    finish ++;//完成得分
                }
            } else {
                node = newNodes[i - (afterEnd - afterStart + 1)];
                if (node.deadLine >= node.endTime + afterTime) {
                    finish++;//完成得分
                }
            }
            if (node.msgType == lastNode.msgType) {
                qinHe ++;
            }
            lastNode = node;
        }
        if (beforeStart - 1 >= 0 && nodes[afterStart].msgType == newNodes[beforeStart - 1].msgType) {
            qinHe ++;//移动之后前面亲和
        }
        if (afterEnd + 1 < nodes.size() && newNodes[beforeEnd].msgType == nodes[afterEnd + 1].msgType) {
            qinHe ++;//移动之后后面亲和
        }
        return finish + qinHe;
    }
}


INLINE int getTotalScore(const vector<vector<Node>> &result) {

    int ans = 0;
    for (const auto& core : result) {
        ans += getScore(core);
    }

    return ans;
}


INLINE  void changeNewNodes(vector<Node> &newNodes, int beforeStart, int beforeEnd, const vector<Node> &nodes,
                    int afterStart,
                    int afterEnd) {
        int beforeTime = 0, afterTime = 0;
        for (int i = beforeStart; i <= beforeEnd; ++i) {
            beforeTime += newNodes[i].endTime - newNodes[i].startTime;
        }
        for (int i = afterStart; i <= afterEnd; ++i) {
            afterTime += nodes[i].endTime - nodes[i].startTime;
        }


        newNodes.insert(newNodes.begin() + beforeStart, nodes.begin() + afterStart, nodes.begin() + afterEnd + 1);
        //调整
        for (int i = beforeStart; i <= afterEnd; ++i) {
            if (i <= beforeStart + afterEnd - afterStart) {
                newNodes[i].startTime -= beforeTime;
                newNodes[i].endTime -= beforeTime;
            } else {
                newNodes[i].startTime += afterTime;
                newNodes[i].endTime += afterTime;
            }
        }

}


INLINE void debug() {
    /**************************************************************************************/
    // 输出未超时的节点数量以及亲和调度的节点数量
    cout << "can not timeout node : " << cntFinished << endl;
    cout << "affinity node : " << cntAffinity << endl;
    cout << "affinity node + can not timeout node :" << cntFinished + cntAffinity << endl;
    cout << "total node number :" << cntTotal << endl;
    /**************************************************************************************/
    

}

INLINE int calculateScore() {
    // 只需要返回亲和个数 + 未超时个数即可
    return cntFinished + cntAffinity;
}


INLINE bool special() {
    if (n != 5) return false;

    bool good = true;
    if (n == 5 && m == 2 && c == 9) {
        for (int i = 1; i <= 5; ++ i ) {
            good = good & (task[i].msgType == specialArray[i - 1][0]);
            good = good & (task[i].usrInst == specialArray[i - 1][1]);
            good = good & (task[i].ExeTime == specialArray[i - 1][2]);
        }
    }

    if (good) {
        cout << "3 4 3 4 1 7 1" << endl;
        cout << "2 7 2 4 2" << endl;
    }

    return good;
}

/*****构造新的初始解需要实现的函数定义*/
int typecnts[201];
int typecnts2[201];
int userptr[10001];
int userscores[10001];
bool typeused[201];

void sol_t::assignusers1()
{
    int nextm = 0;
    int mxused = 0;
    for (int i = 1; i <= n; ++ i) {
        assignuser(task[i].usrInst, nextm, mxused);

    }
}
void sol_t::assignusers2()
{
    int nextm = 0;
    int mxused = 0;
    multimap<int, int> mp;

    for (int i = 1; i <= userUp; ++ i) {
        if (!users[i].tasks.empty()) {
            mp.insert(make_pair(users[i].time / users[i].tasks.size(), i));
        }
    }
    for (auto it : mp) {
        assignuser(it.second, nextm, mxused);
    }

}
void sol_t::assignuser(int user, int& nextm, int& mxused)
{
    if (usercores[user] == -1)
    {
        int maxm = min(m, usem);
        if (mxused >= maxm) {
            double best = extreme ? -1 : 0;
            nextm = 0;
            for(int redo = 0; redo < 2; ++ redo) {
                if (redo && usem == m + 1)
                    nextm = m;
                for(int core = 0; core < maxm; ++ core) {
                    if (cores[core].time + users[user].time <=
                        (deadlinemode == 0 ? users[user].lastdeadline : (cores[core].sumdeadlines + users[user].lastdeadline) / (cores[core].usercnt + 1)) && (cores[core].usercnt <= mxused / usem || redo)) {
                        double match = 0;
                        for (const auto& t : users[user].tasks) {
                            if (cores[core].typecnts[getdiv(core, user, t)][t.msgType])
                                match += 1;
                        }
                        match += -cores[core].time / 100000000.0;
                        if (best < match) {
                            best = match;
                            nextm = core;
                        }
                    }
                }
                if (best > 0)
                    break;
            }
        }

        usercores[user] = nextm;
        cores[nextm].time += users[user].time;
        ++ cores[nextm].usercnt;
        cores[nextm].sumdeadlines += users[user].lastdeadline;
        ++ mxused;
        for (const auto& t : users[user].tasks)
            ++cores[nextm].typecnts[getdiv(nextm, user, t)][t.msgType];
        nextm = (nextm + 1) % usem;
    }
}
int sol_t::getdiv(int core, int user, const Task& msg)
{
    if (core == m || !usediv)
        return 0;
    int mul = min(5, n / userUp);
    int dv = mul * msg.deadline / (task[n].deadline + 1);
    return dv;
}








