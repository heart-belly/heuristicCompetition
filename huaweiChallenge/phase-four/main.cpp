
#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)

#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <map>

using namespace std;
typedef long long LL;
typedef pair<int, int> PII;
#define x first
#define y second
const int N = 60, M = 2e6 + 10, NODE_LIMIT = 110;
const LL defaultTime = 1e8;


int cnt = 0;
/***
 * 定义节点类型
*/
struct allNode{
    int cpu;
    int memory;
    double price;
} allNode[N];
/***
 * 定义节点
*/
struct Node{
    int nodeId;
    int curCpu; // 已经使用cpu数量
    int curMemory; // 已经使用存储容量
    int nodeClass; // 结点类型
    int podNumber; //节点上pod个数
    LL createTime; // 结点创建时间
    LL dieTime; // 估计节点的消亡时间
    LL lastUseTime; // 节点最近一次被使用时间
    PII mode; // 标记结点的模式

}node[M];

// 对已有节点编号进行索引
struct LinkNode{
    int nodeId;
    LinkNode *next, *prev;

    LinkNode(int nodeId = 0) : nodeId(nodeId), next(nullptr), prev(nullptr) {
    }
};

void remove(LinkNode* x);
void pushFront(LinkNode *dummy,LinkNode* x);
double calculateScore1(const Node& node);
double calculateScore2(const Node& node);
double calculateScore3(const Node& node);
double calculateScore4(const Node& node);
double calculateScore5(const Node& node);
double calculateScore6(const Node& node);
int curCpu, curMemory; //用于优先队列的比较函数
// 定义优先队列的比较函数
struct cmp
{
	bool operator()(const Node& node1, const Node& node2)
	{
        // 计算得分
        // node1的cpu利用率和内存利用率

        // 1
        // double score1 = calculateScore1(node1);
        // double score2 = calculateScore1(node2);
        // 2
        // double score1 = calculateScore2(node1);
        // double score2 = calculateScore2(node2);
        // 3
        // double score1 = calculateScore3(node1);
        // double score2 = calculateScore3(node2);
        // 4
        // double score1 = calculateScore4(node1);
        // double score2 = calculateScore4(node2);

        // 5
        // double score1 = calculateScore5(node1);
        // double score2 = calculateScore5(node2);

        // 6
        double cost1 = calculateScore6(node1);
        double cost2 = calculateScore6(node2);

        return cost1 < cost2;
	}
};


LinkNode *dummy; // 定义节点的使用情况（使用链表）
int podInNode[M]; // 定义pod在哪个节点上
int podOccupyCpu[M]; // 定义pod占有cpu
int podOccupyMemory[M]; // 定义pod占有内存
int podCreateTime[M];

int F; // 节点类型 
int idx; // 节点标记
int T; // 当前时刻

vector<int> useToNewPodInNode;
vector<int> useToNewNodeCreate;

// 总的消亡时间
// 消亡节点的个数
double dieSumT = 0;
double dieNumber = 0;

// 一次创建的时间以及个数
PII creatPod[M];
int creatNumber = 0;

// 统计每一次创建的节点类型以及个数
map<PII, vector<int>> perCreatePod;
map<PII, int> backup;
// 统计最近一次创建的上一次创建
map<PII, int> lastCreate;

void solve();
void input();
void init();
int deal(int id, int cpu, int memory, int lastTime);
void dealDelete(int podId, int T);

int main() {
    init();
    solve();
}

void init() {
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    // 节点初始化为1
    idx = 1;
    // 初始化节点
    dummy = new LinkNode(0);
    dummy->next = dummy;
    dummy->prev = dummy;
}
void solve() {

    input();
}

void input() {
        // 输入节点类型
    cin >> F;
    for (int i = 1; i <= F; i ++ ) {
        int cpu, memory;
        double price;
        cin >> cpu >> memory >> price;
        allNode[i] ={cpu, memory, price};
    }

    string op; // 要执行操作
    int S; // 数量
    while (cin >> T >> op >> S) {

        if (op == "END" && S == 0) break;

        if (op == "CREATE") {
            creatPod[creatNumber ++] = {T, S};
            perCreatePod.clear();
            useToNewPodInNode = vector<int>(S, 0); // S个结点对应的下标为0---S - 1
            useToNewNodeCreate.clear();
            int curMinId = -1;
            vector<PII> saveList;
            backup.clear();
            vector<vector<int>> podList(S, vector<int>(3));
            // 输入
            for (int i = 1; i <= S; i ++ ) {
                int id, cpu, memory;
                cin >> id >> cpu >> memory;
                
                saveList.push_back({cpu, memory});
                backup[{cpu, memory}] ++;
                // 存入临时数组
                podList[i - 1] = {id, cpu, memory};
            }

            // 处理输入
            for (int i = 0; i < S; i ++ ) {
                int id, cpu, memory;
                id = podList[i][0], cpu = podList[i][1], memory = podList[i][2];
                // 题目按照顺序给出创建的id
                if (curMinId == -1) {
                    curMinId = id;
                }

                podOccupyCpu[id] = cpu;
                podOccupyMemory[id] = memory;
                podCreateTime[id] = T;
                // 得到当前申请的cpu和内存
                curCpu = cpu;
                curMemory = memory;

                int ret = deal(id, cpu, memory, T);
                if (ret == 0) {
                    perCreatePod[{cpu, memory}].push_back(id);
                } else {
                    useToNewPodInNode[id - curMinId] = ret;
                }
            }
            
            // 遍历可以预先处理过的pod
            for (auto [pod, podVec] : perCreatePod) {
                int cpu = pod.x, memory = pod.y;
                // 得到该类pod的数量
                int number = podVec.size();
                int newNumber = 0;
                // 计算新节点类型能够容纳当前节点的数量，按照性价比从小到大排序
                // 预测未来时刻的结点个数
                if (creatNumber > 1) {
                    if (T - creatPod[creatNumber - 2].first < 2 ) {
                        newNumber = lastCreate[{cpu, memory}];
                    }
                }

                vector<PII> mayBeNode;
                for (int i = 1; i <= F; i ++ ) {
                     if (allNode[i].cpu < cpu || allNode[i].memory < memory) continue;
                     // 可以继续修改
                     mayBeNode.push_back({i, min(number, min(allNode[i].cpu / cpu, allNode[i].memory / memory))});
                }
                sort(mayBeNode.begin(), mayBeNode.end(), [&](PII &p1, PII &p2) {
                    double price1 = allNode[p1.x].price;
                    double price2 = allNode[p2.x].price;
                    // 按照性价比排序
                    double p1n = price1 / (1.0 * p1.y) / pow(0.995, p1.y - 1);
                    double p2n = price2 / (1.0 * p2.y) / pow(0.995, p2.y - 1);
                    return p1n < p2n;
                });
                auto firstNode = mayBeNode[0];

                // 满足所有结点数量需求，创建这个节点
                if (firstNode.y >= number) {
                    // 将编号加入到链表中
                    LinkNode *p = new LinkNode(idx);
                    p->next = dummy->next;
                    dummy->next = p;
                    p->prev = dummy;
                    p->next->prev = p;
                    node[idx].nodeId = idx;
                    node[idx].nodeClass = firstNode.x;
                    node[idx].curCpu = cpu * number;
                    node[idx].curMemory = memory * number;
                    node[idx].podNumber += number;
                    node[idx].createTime = T;
                    node[idx].lastUseTime = T;
                    node[idx].mode = {cpu, memory};

                    if (dieNumber == 0) {
                        node[idx].dieTime = (LL)T + defaultTime;
                    } else {
                        node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                    }
                    // 把对应pod放入节点中
                    for (int i = 0; i < podVec.size(); i ++ ) {
                        podInNode[podVec[i]] = idx;
                        useToNewPodInNode[podVec[i] - curMinId] = idx;
                    }

                    useToNewNodeCreate.emplace_back(firstNode.x);
                    idx ++;
                    
                } else {
                    // 节点可以存放的数量小于当前需求的数量
                    // 求出需要的节点数量
                    int d1 = number / firstNode.y;
                    int d2 = (number % firstNode.y != 0 ? 1 : 0);
                    int index = 0;
                    for (int i = 0; i < d1; i ++ ) {
                        // 将编号加入到链表中
                        LinkNode *p = new LinkNode(idx);
                        p->next = dummy->next;
                        dummy->next = p;
                        p->prev = dummy;
                        p->next->prev = p;

                        node[idx].nodeId = idx;
                        node[idx].nodeClass = firstNode.x;
                        node[idx].curCpu = cpu * firstNode.y;
                        node[idx].curMemory = memory * firstNode.y;
                        node[idx].podNumber += firstNode.y;
                        node[idx].createTime = T;
                        node[idx].lastUseTime = T;
                        node[idx].mode = {cpu, memory};
                        if (dieNumber == 0) {
                            node[idx].dieTime = (LL)T + defaultTime;
                        } else {
                            node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                        }

                        for (int j = 0; j < firstNode.y; j ++ ) {
                            podInNode[podVec[index]] = idx;
                            useToNewPodInNode[podVec[index] - curMinId] = idx;
                            index ++;
                        }
                        useToNewNodeCreate.emplace_back(firstNode.x);
                        idx ++;
                    }
                    if (newNumber > 0) {
                        // 可以修改，加入可能的新结点数量（尝试以下不一定对）
                        for (int i = 0; i < d2; i ++ ) {
                            // 将编号加入到链表中
                            LinkNode *p = new LinkNode(idx);
                            p->next = dummy->next;
                            dummy->next = p;
                            p->prev = dummy;
                            p->next->prev = p;

                            node[idx].nodeId = idx;
                            node[idx].nodeClass = firstNode.x;
                            node[idx].curCpu = cpu * (number % firstNode.y);
                            node[idx].curMemory = memory * (number % firstNode.y);
                            node[idx].podNumber += (number % firstNode.y);
                            node[idx].createTime = T;
                            node[idx].lastUseTime = T;
                            node[idx].mode = {cpu, memory};
                            if (dieNumber == 0) {
                                node[idx].dieTime = (LL)T + defaultTime;
                            } else {
                                node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                            }
                            for (int j = 0; j < (number % firstNode.y); j ++ ) {
                                podInNode[podVec[index]] = idx;
                                useToNewPodInNode[podVec[index] - curMinId] = idx;
                                index ++;
                            }
                            useToNewNodeCreate.emplace_back(firstNode.x);
                            idx ++;

                        }
                        continue;
                    }  

                    // 对于剩余的数量重新寻找结点
                    int remainPodOfNumber = number % firstNode.y;
                    if (remainPodOfNumber > 0) {
                        // 重新寻找最小成本结点
                        mayBeNode.clear();
                        for (int i = 1; i <= F; i ++ ) {
                            if (allNode[i].cpu < cpu || allNode[i].memory < memory) continue;
                            // 可以继续修改
                            mayBeNode.push_back({i, min(remainPodOfNumber, min(allNode[i].cpu / cpu, allNode[i].memory / memory))});
                        }
                        sort(mayBeNode.begin(), mayBeNode.end(), [&](PII &p1, PII &p2) {
                            double price1 = allNode[p1.x].price;
                            double price2 = allNode[p2.x].price;
                            // 按照性价比排序
                            double p1n = price1 / (1.0 * p1.y) / pow(0.985, p1.y - 1);
                            double p2n = price2 / (1.0 * p2.y) / pow(0.985, p2.y - 1);
                            return p1n < p2n;
                        });   
                        auto firstNode = mayBeNode[0];
                        // 满足所有结点数量需求，创建这个节点
                        if (firstNode.y >= remainPodOfNumber) {
                            // 将编号加入到链表中
                            LinkNode *p = new LinkNode(idx);
                            p->next = dummy->next;
                            dummy->next = p;
                            p->prev = dummy;
                            p->next->prev = p;
                            node[idx].nodeId = idx;
                            node[idx].nodeClass = firstNode.x;
                            node[idx].curCpu = cpu * remainPodOfNumber;
                            node[idx].curMemory = memory * remainPodOfNumber;
                            node[idx].podNumber += remainPodOfNumber;
                            node[idx].createTime = T;
                            node[idx].lastUseTime = T;
                            node[idx].mode = {cpu, memory};

                            if (dieNumber == 0) {
                                node[idx].dieTime = (LL)T + defaultTime;
                            } else {
                                node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                            }
                            // 把对应pod放入节点中
                            for (int i = 0; i < remainPodOfNumber; i ++ ) {
                                podInNode[podVec[index]] = idx;
                                useToNewPodInNode[podVec[index] - curMinId] = idx;
                                index ++;
                            }

                            useToNewNodeCreate.emplace_back(firstNode.x);
                            idx ++;
                            
                        } else {
                            int sd1 = remainPodOfNumber / firstNode.y;
                            int sd2 = (remainPodOfNumber % firstNode.y != 0 ? 1 : 0);
                            for (int i = 0; i < sd1; i ++ ) {
                                // 将编号加入到链表中
                                LinkNode *p = new LinkNode(idx);
                                p->next = dummy->next;
                                dummy->next = p;
                                p->prev = dummy;
                                p->next->prev = p;

                                node[idx].nodeId = idx;
                                node[idx].nodeClass = firstNode.x;
                                node[idx].curCpu = cpu * firstNode.y;
                                node[idx].curMemory = memory * firstNode.y;
                                node[idx].podNumber += firstNode.y;
                                node[idx].createTime = T;
                                node[idx].lastUseTime = T;
                                node[idx].mode = {cpu, memory};
                                if (dieNumber == 0) {
                                    node[idx].dieTime = (LL)T + defaultTime;
                                } else {
                                    node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                                }

                                for (int j = 0; j < firstNode.y; j ++ ) {
                                    podInNode[podVec[index]] = idx;
                                    useToNewPodInNode[podVec[index] - curMinId] = idx;
                                    index ++;
                                }
                                useToNewNodeCreate.emplace_back(firstNode.x);
                                idx ++;
                            }

                            for (int i = 0; i < sd2; i ++ ) {
                                // 将编号加入到链表中
                                LinkNode *p = new LinkNode(idx);
                                p->next = dummy->next;
                                dummy->next = p;
                                p->prev = dummy;
                                p->next->prev = p;

                                node[idx].nodeId = idx;
                                node[idx].nodeClass = firstNode.x;
                                node[idx].curCpu = cpu * (remainPodOfNumber % firstNode.y);
                                node[idx].curMemory = memory * (remainPodOfNumber % firstNode.y);
                                node[idx].podNumber += (remainPodOfNumber % firstNode.y);
                                node[idx].createTime = T;
                                node[idx].lastUseTime = T;
                                node[idx].mode = {cpu, memory};
                                if (dieNumber == 0) {
                                    node[idx].dieTime = (LL)T + defaultTime;
                                } else {
                                    node[idx].dieTime = (LL)T + dieSumT / dieNumber;
                                }
                                for (int j = 0; j < (remainPodOfNumber % firstNode.y); j ++ ) {
                                    podInNode[podVec[index]] = idx;
                                    useToNewPodInNode[podVec[index] - curMinId] = idx;
                                    index ++;
                                }
                                useToNewNodeCreate.emplace_back(firstNode.x);
                                idx ++;

                            }                            
                        }

                    }
                    


                }

            }


            if (useToNewNodeCreate.empty()) {
                cout << "0" << endl;
            } else {
                cout << useToNewNodeCreate.size() << " ";
                for (int i = 0; i < useToNewNodeCreate.size(); i ++ ) {
                    cout << useToNewNodeCreate[i] << " ";
                }
                cout << endl;
            }
            fflush(stdout);
            
            for (int i = 0; i < useToNewPodInNode.size(); i ++ ) {
                cout << useToNewPodInNode[i] << " ";
            }
            cout << endl;
            fflush(stdout);

            // 处理一次创建后
            lastCreate.clear();
            for (int i = 0; i < saveList.size(); i ++ ) {
                lastCreate[{saveList[i].x, saveList[i].y}] ++;
            }

            
        } else if (op == "DELETE") {
            for (int i = 1; i <= S; i ++ ) {
                int podId;
                cin >> podId;
                dealDelete(podId, T);
            }
        }
    }
}
// 对已有节点进行过滤和打分，得到满足要求的节点
int deal(int id, int cpu, int memory, int lastTime) {
    if (dummy->next == dummy) return 0;

    double durTime = 0;
    if (dieNumber == 0) {
        durTime = defaultTime;
    } else {
        durTime = dieSumT / dieNumber;
    }
    LinkNode *p = dummy->next;
    // 说明上次命令就是创建结点 (先去寻找上一时刻创建的结点,将pod放入)
    while (p != dummy) {
        int nodeId = p->nodeId;
        auto canNode = node[nodeId];
        int cls = canNode.nodeClass;
        if (true) {
            int timet = 2;
            int tt1 = min(allNode[cls].cpu / cpu, allNode[cls].memory / memory);
            int tt = min((allNode[cls].cpu - canNode.curCpu) / cpu, (allNode[cls].memory - canNode.curMemory) / memory);
            if (allNode[cls].cpu >= cpu + canNode.curCpu && allNode[cls].memory >= memory + canNode.curMemory 
            && canNode.podNumber < NODE_LIMIT && canNode.mode == make_pair(cpu, memory)
            && (tt < backup[{cpu, memory}] || abs(tt - backup[{cpu, memory}]) < 2 )) {
                    // 更新数据
                    backup[{cpu, memory}] --;
                    node[nodeId].curCpu += cpu;
                    node[nodeId].curMemory += memory;
                    node[nodeId].podNumber += 1;
                    node[nodeId].dieTime = lastTime + durTime;
                    podInNode[id] = nodeId;
                    node[nodeId].lastUseTime = lastTime;
                    // 将对应的p插入头部
                    remove(p);
                    pushFront(dummy, p);


                    return nodeId;
                }
            // 优先使用

        }
        p = p->next;
    }

    p = dummy->next;
    while (p != dummy) {
        int nodeId = p->nodeId;
        auto canNode = node[nodeId];
        int cls = canNode.nodeClass;
        if (true) {
            int timet = 2;
            int tt1 = min(allNode[cls].cpu / cpu, allNode[cls].memory / memory);
            int tt = min((allNode[cls].cpu - canNode.curCpu) / cpu, (allNode[cls].memory - canNode.curMemory) / memory);
            if (allNode[cls].cpu >= cpu + canNode.curCpu && allNode[cls].memory >= memory + canNode.curMemory 
            && canNode.podNumber < NODE_LIMIT && 1.0 * canNode.mode.first / cpu == 1.0 * canNode.mode.second / memory ) {
                    // 更新数据
                    backup[{cpu, memory}] --;
                    node[nodeId].curCpu += cpu;
                    node[nodeId].curMemory += memory;
                    node[nodeId].podNumber += 1;
                    node[nodeId].dieTime = lastTime + durTime;
                    podInNode[id] = nodeId;
                    node[nodeId].lastUseTime = lastTime;
                    // 将对应的p插入头部
                    remove(p);
                    pushFront(dummy, p);


                    return nodeId;
                }
            // 优先使用

        }
        p = p->next;
    }

    int t = -1, curMinCost = 1e8;

    p = dummy->next;
    LinkNode *temp = p;
    // while (p != dummy) {
    //     int nodeId = p->nodeId;
    //     auto canNode = node[nodeId];
    //     int cls = canNode.nodeClass;
    //     if (allNode[cls].cpu >= cpu + canNode.curCpu && allNode[cls].memory >= memory + canNode.curMemory 
    //     && canNode.podNumber < NODE_LIMIT  && (lastTime - canNode.lastUseTime < 1)) {
    //         double cost2 = 1.0 * durTime * (allNode[node[nodeId].nodeClass].price);
    //         if (t == -1 || curMinCost > cost2) {
    //             t = nodeId;
    //             curMinCost = cost2;
    //             temp = p;
    //         }

    //     }

    //     p = p->next;
    // }
    if (t == -1) return 0;
    // 得到结点编号
    int nodeId = t;
    // 节点的消亡时间在当前时间之前
    if (node[nodeId].dieTime <= lastTime) return 0;
    // 得到能放入的新节点（与已创建节点进行比较）
    t = -1;
    for (int i = 1; i <= F; i ++ ) {
        if (allNode[i].cpu < cpu || allNode[i].memory < memory) continue;

        if (t == -1 || allNode[i].price < allNode[t].price) {
            t = i;
        }
    }

    double cost1 = 1.0 * (node[nodeId].dieTime - lastTime) * allNode[node[nodeId].nodeClass].price + 1.0 * (durTime * allNode[t].price);
    double cost2 = 1.0 * durTime * (allNode[node[nodeId].nodeClass].price);

    if (cost1 < cost2) return 0;
    // 更新数据
    node[nodeId].curCpu += cpu;
    node[nodeId].curMemory += memory;
    node[nodeId].podNumber += 1;
    node[nodeId].dieTime = lastTime + durTime;
    podInNode[id] = nodeId;
    node[nodeId].lastUseTime = lastTime;

    remove(temp);
    pushFront(dummy, temp);


    return nodeId;

}

void dealDelete(int podId, int T) {
    // 消亡时间累加
    dieSumT += (T - podCreateTime[podId]);
    dieNumber += 1.0;

    int nodeId = podInNode[podId];

    node[nodeId].curCpu -= podOccupyCpu[podId];
    node[nodeId].curMemory -= podOccupyMemory[podId];
    node[nodeId].podNumber -= 1;

    if (node[nodeId].curCpu == 0 && node[nodeId].curMemory == 0) {
        // 从链表中删除
        LinkNode *p = dummy;
        LinkNode *q = dummy->next;
        while (q->nodeId != nodeId) {
            p = q;
            q = q->next;
        }
        p->next = q->next;
        q->next->prev = p;
        delete q;
    }
}

void remove(LinkNode* x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
}

void pushFront(LinkNode *dummy,LinkNode* x) {
    x->prev = dummy;
    x->next = x->prev->next;
    x->prev->next = x;
    x->next->prev = x;
}

double calculateScore1(const Node& node) {
    double lc = 1.0 * node.curCpu / allNode[node.nodeClass].cpu, lm = 1.0 * node.curMemory / allNode[node.nodeClass].memory;
    double score = 1.0 * 10 * (1 - (lc + lm) / (allNode[node.nodeClass].cpu + allNode[node.nodeClass].memory));

    return score;
}
//Least Requested Priority
double calculateScore2(const Node& node) {
    double lc = 1.0 * 10 * (allNode[node.nodeClass].cpu - node.curCpu - curCpu) / allNode[node.nodeClass].cpu;
    double lm = 1.0 * 10 * (allNode[node.nodeClass].memory - node.curMemory - curMemory) / allNode[node.nodeClass].memory;

    double score = (lc + lm) / 2;
    return score;
}
//Balanced Resource Allocation
double calculateScore3(const Node& node) {
    double lc = 1.0 * (node.curCpu + curCpu) / allNode[node.nodeClass].cpu;
    double lm = 1.0 * (node.curMemory + curMemory) / allNode[node.nodeClass].memory;

    double score = (1.0 - abs(lc - lm)) * 10;

    return score;
    
}
// 秃鹰搜索算法
double calculateScore4(const Node& node) {
    // 定义节点类型
    double cCpu = 1.0 * curCpu / allNode[node.nodeClass].cpu;
    double cMem = 1.0 * curMemory / allNode[node.nodeClass].memory;
    double k = cCpu / cMem;
    double u1 = 1.0 * k / (k + 1);
    double u2 = 1.0 * 1 / (k + 1);
    // 定义利用率
    double rCpu = 1.0 - (1.0 * curCpu / (allNode[node.nodeClass].cpu - node.curCpu));
    double rMem = 1.0 - (1.0 * curMemory / (allNode[node.nodeClass].memory - node.curMemory));

    double rn = u1 * rCpu + u2 * rMem;

    // 定义负载均衡度
    double tcpu = 1.0, tmem = 1.0;
    double lcpu = 1.0 * (curCpu + node.curCpu) / allNode[node.nodeClass].cpu;
    double lmem = 1.0 * (curMemory + node.curMemory) / allNode[node.nodeClass].memory;

    double ln = u1 * (tcpu - lcpu) + u2 * (tmem - lmem);

    double score = rn;

    return score;
}
// 负载均衡改进
double calculateScore5(const Node& node) {
    // 定义节点类型
    double cCpu = 1.0 * curCpu / allNode[node.nodeClass].cpu;
    double cMem = 1.0 * curMemory / allNode[node.nodeClass].memory;
    double k = cCpu / cMem;
    double u1 = 1.0 * k / (k + 1);
    double u2 = 1.0 * 1 / (k + 1);

    // 定义利用率
    double lcpu = 1.0 * (curCpu + node.curCpu) / allNode[node.nodeClass].cpu;
    double lmem = 1.0 * (curMemory + node.curMemory) / allNode[node.nodeClass].memory;

    double s = abs(u1 * lcpu - u2 * lmem);

    return 10.0 - 10.0 * s;
}

// 负载均衡改进
double calculateScore6(const Node& node) {
    double cost = (node.dieTime - T) * allNode[node.nodeClass].price;
    

    return cost;
}