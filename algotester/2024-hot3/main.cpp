#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> PII;
typedef pair<int, pair<int, int>> PIPII;
#define x first
#define y second
const int NUM_NODES         = 10;
const int NUM_TYPES         = 3;
const int MAX_BATCH_SIZE    = 16;
const int MAX_NUM_PACKETS   = 100000;

struct Packet
{
    int type;
    int arriveTime;
    int t;
    int arriveCurtime;
};

Packet packets[MAX_NUM_PACKETS];
int cost[NUM_NODES + 1][NUM_TYPES + 1][MAX_BATCH_SIZE + 1];
int batchLimit[NUM_NODES + 1][NUM_TYPES + 1];
vector<int> ids[NUM_NODES + 1][NUM_TYPES + 1];
vector<int> waitPackets, otherPackets;
int c4, c6, cr;
int n, curTime = 1, nout = 0, queueTime = 0, inQueueNumber = 0, nin = 0;
int curNode = 0;
bool allowRun = false;
int lastR = -1;
int cntType[4]{};
double avgArriveTime{};
int nextNode1 = -1, nextType1 = -1;
bool isSpecial = false;
bool isOpen = true;
map<pair<int, int>, int> nextNode = 
{
    {{1, 1}, 2},
    {{2, 1}, 3},
    {{3, 1}, 4},
    {{4, 1}, 5},
    {{5, 1}, 6},
    {{6, 1}, 7},
    {{7, 1}, 8},
    {{8, 1}, 9},
    {{9, 1}, 10},
    {{1, 2}, 2},
    {{2, 2}, 3},
    {{3, 2}, 8},
    {{8, 2}, 9},
    {{9, 2}, 10},
    {{1, 3}, 2},
    {{2, 3}, 9},
    {{9, 3}, 10}
};

double savaAverageDelay[4][2]; // 保留每个类型以一定bacth运行的平均延迟
double s = 0.0;
PII costPart[11][MAX_BATCH_SIZE * 2 + 1];

const vector<PIPII> exc16 = {
    {8, {1, 1}},
    {8, {2, 1}},
    {8, {1, 1}},
    {8, {2, 1}},
    {16, {3, 1}},
    {6, {4, 1}},
    {6, {4, 1}},
    {4, {4, 1}},
    {9, {5, 1}},
    {8, {6, 1}},
    {7, {5, 1}},
    {8, {6, 1}},
    {8, {7, 1}},
    {8, {8, 1}},
    {7, {9, 1}},
    {7, {10, 1}},
    {8, {7, 1}},
    {8, {8, 1}},
    {9, {9, 1}},
    {4, {10, 1}},
    {5, {10, 1}}
};
const vector<PIPII> exc24 = {
    {8, {1, 1}},
    {8, {2, 1}},
    {8, {1, 1}},
    {8, {2, 1}},
    {16, {3, 1}},
    {6, {4, 1}},
    {6, {4, 1}},
    {8, {1, 1}},
    {8, {2, 1}},
    {8, {3, 1}},
    {6, {4, 1}},
    {6, {4, 1}},
    {11, {5, 1}},
    {8, {6, 1}},
    {11, {5, 1}},
    {8, {6, 1}},
    {2, {5, 1}},
    {8, {6, 1}},
    {9, {7, 1}},
    {6, {7, 1}},
    {11, {8, 1}},
    {10, {9, 1}},
    {7, {10, 1}},
    {9, {7, 1}},
    {13, {8, 1}},
    {10, {9, 1}},
    {7, {10, 1}},
    {4, {9, 1}},
    {5, {10, 1}},
    {5, {10, 1}}
};

void initNecessaryData() {
    // 处理类型一
    for (int i = 1; i <= NUM_NODES; ++ i) {
        savaAverageDelay[1][0] += cost[i][1][1];
        savaAverageDelay[1][1] += 1.0 * cost[i][1][batchLimit[i][1]] / batchLimit[i][1];
    }
    savaAverageDelay[1][0] += 31 + 107;
    savaAverageDelay[1][1] += 31 + 107;
    // 处理类型二
    for (auto &id : {1, 2, 3, 8, 9, 10}) {
        savaAverageDelay[2][0] += cost[id][2][1];
        savaAverageDelay[2][1] += 1.0 * cost[id][2][batchLimit[id][2]] / batchLimit[id][2];
    }
    // 处理类型三
    for (auto &id : {1, 2, 9, 10}) {
        savaAverageDelay[3][0] += cost[id][3][1];
        savaAverageDelay[3][1] += 1.0 * cost[id][3][batchLimit[id][3]] / batchLimit[id][3];
    }

    cerr << "one: " << "单个耗时: " << savaAverageDelay[1][0] << " 最优耗时: " << savaAverageDelay[1][1] << endl;

    for (int node = 1; node <= 10; ++ node) {
        int maxNode = batchLimit[node][1];
        for (int number = 1; number <= maxNode * 2; ++ number) {
            int a = -1, b = -1;
            int bestCost = 0x3f3f3f3f;
            for (int j = 0; j <= maxNode; ++ j) {
                int c1 = j, c2 = number - j;
                if (c2 >= 0 && c2 <= maxNode) {
                    int cost1 = cost[node][1][c1] + cost[node][1][c2];
                    if (bestCost >= cost1) {
                        bestCost = cost1;
                        a = c1;
                        b = c2;
                    }

                }
            }
            costPart[node][number] = {a, b};
        }
    }

    for (int node = 1; node <= 10; ++ node) {
        int maxNode = batchLimit[node][1];
        for (int number = 1; number <= maxNode * 2; ++ number) {
            cerr << "node: " << node << " " << "number: " << number << "maxPart: " << costPart[node][number].x <<  " " << costPart[node][number].y << endl;
        }

    }

}
int aa = 0;
int bb = 0;
void ReceivePackets(int t)
{
    ++ bb;
    // cerr << lastR << " " << t << " " << t - lastR << endl;
    if (t - lastR == 20) {
        ++ aa;
    }
    lastR = t;
    curTime = t + cr;
    cout << 'R' << " " << t << "\n";
    cout.flush();
    int p, id, type, a;
    cin >> p;
    if (p == -1)
    {
        exit(0);
    }
    nin += p;
    curNode += p;
    // waitPackets.clear();
    for (int i = 0; i < p; ++i) 
    {
        cin >> id >> type >> a;
        packets[id].type = type;
        packets[id].arriveTime = a;
        packets[id].t = curTime;
        packets[id].arriveCurtime = curTime;
        if (type == 1) {
            waitPackets.emplace_back(id);
        } else {
            otherPackets.emplace_back(id);
        }
        // ids[1][type].push_back(id);
        ++ cntType[type];
    }
    unordered_map<int, int> hashmap = {
        {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 8}, {10, 8}, {11, 11},
        {12, 12},{13, 13}, {14, 14}, {15, 15}, {16, 16}, {17, 16}, {18, 16}, {19, 16}, {20, 16},
        {21, 16}, {22, 16}, {23, 16}, {24, 24}, {25, 24}, {26, 24}, {27, 24}, {28, 24}, {29, 24}, {30, 24}, {31, 24}, {32, 32}
    };
    // 改变取出来的数量
    int size = waitPackets.size();
    if (true) {
        int temp = size;
        if (temp <= 24) size =  hashmap[temp];
        else {
            size = 24;
        }
    } 


    for (int i = 0; i < size; ++ i) {
        int id = waitPackets[i];
        int type = packets[id].type;
        ids[1][type].push_back(id);       
    }
    waitPackets.erase(waitPackets.begin(), waitPackets.begin() + size);
    for (int i = 0; i < otherPackets.size(); ++ i) {
        int id = otherPackets[i];
        int type = packets[id].type;
        ids[1][type].push_back(id);       
    }
    otherPackets.clear();
    avgArriveTime = 0.0;
    int number = 0;
    double sum = 0;
    for (int i = nin; i >= max(nin - 50, 2); -- i) {
        int next = i, pre = i - 1;
        sum += (packets[next].arriveTime - packets[pre].arriveTime);
        number ++;
    }

    // cerr << "*******************************" << endl;
    // cerr << "到来的请求数量: " << p << endl;
    // // cerr << nin << endl;
    avgArriveTime = sum / number;
    // cerr << "前50个平均到达时间: " << avgArriveTime << endl;


}
int execute = 0;
void ExecuteTask(int nodeId, vector<int> arr, int t)
{
    int b = arr.size(), type = packets[arr[0]].type;
    cout << 'E' << " " << t << " " << nodeId << " " << b;

    if (++ execute <= 500) {
        cerr << 'E' << " " << t << " " << nodeId << " " << b << endl;
    }

    for (int i = 0; i < b; ++i) 
    {
        cout << " " << arr[i];
    }
    cout << "\n";
    cout.flush();

    int p;
    cin >> p;
    if (p == -1) 
    {
        exit(0);
    }

    int curCost = cost[nodeId][type][b];
    int tempTime = curTime;
    curTime = t + curCost;
    for (int i = 0; i < b; ++i) 
    {
        packets[arr[i]].t = curTime;
    }
    if (nextNode.count({nodeId, type})) 
    {
        int nextNodeId = nextNode[{nodeId, type}];
        for (int i = 0; i < b; ++i) 
        {
            ids[nextNodeId][type].push_back(arr[i]);
        }
    }
    static int a = 0, c = 0;
    if (nodeId == NUM_NODES) 
    {
        nout += b;
        curNode -= b;
        // 输出packet的到达和离开时间
        for (int i = 0; i < b; ++ i) {
            // cerr << "packet: " << arr[i] << " type: " << type  << "arriveCurtime: " << packets[arr[i]].arriveCurtime <<"arrive: " << packets[arr[i]].arriveTime << " depart: " << packets[arr[i]].t << 
            // "time sub :" << packets[arr[i]].t - packets[arr[i]].arriveTime << endl; 
            s += (1.0 * packets[arr[i]].t - packets[arr[i]].arriveTime) / n;
        }
        if (nout == n) 
        {
            cerr << "20叫的次数: " << aa << endl;
            cerr << "总: " << bb << endl;
            cerr << "sum: " << s << endl;
            cerr << "a : " << a << " c: " << c << endl; 
            exit(0);
        }
    }
    // hardware accelerator
    // if (nout <= 2000) {
    //     cerr << "nodeId: " << nodeId << "   queuTime: " << queueTime << "   cpuTime: " << curTime << endl;
    // }
    // if (nout >= 500 && nout <= 550 ){
    //     cerr << nout << " " << s << " " << nout / s << endl;
    // }

    if (nodeId == 4 || nodeId == 6) 
    {
        // 统计在结点4和6的数量
        int cnt4 = ids[4][1].size();
        int cnt6 = ids[6][1].size();
        // bool flag = false;
        // if (queueTime > tempTime) {
        //     cerr << "----------------------------------------" << endl;
        //     c ++;
        //     cerr << "queueTime: " << queueTime << "cpuTime: " << tempTime << "  " << tempTime - queueTime << endl;
        //     flag = true;
        // } else {
        //     a ++;
        // }
        for (int i = 0; i < b; ++i)
        {
            packets[arr[i]].t = max(curTime, queueTime) + (nodeId == 4 ? c4 : c6);
            queueTime = packets[arr[i]].t;
        }
        // if (flag) {
        //     cerr << "queueTime: " << queueTime << "cpuTime: " << curTime << "  " << curTime - queueTime << "   结点4: " << cnt4 << "   结点6: " << cnt6 
        //     << "       处理的类型: " << nodeId 
        //     << "       处理的数量: " << b <<   endl; 
        //     cerr <<endl;
        // } 
    }
}


bool TakeSameTypePacket(int i, int selectType, int number) {

    vector<int> arr, temp;
    arr.reserve(ids[i][selectType].size());
    temp.reserve(ids[i][selectType].size());
    int cnt = 0;
    int maxNode = batchLimit[i][selectType];
    for (const auto& id : ids[i][selectType]) {
        if (cnt < number && packets[id].t <= curTime) {
            arr.push_back(id);
            ++ cnt;
        } else {
            temp.push_back(id);
        }
    }
    ids[i][selectType] = temp;
    ExecuteTask(i, arr, curTime);
    return true;


}


vector <PIPII> dispatch;

void selectMax() {
    dispatch = {};
        
    if (!isSpecial) {
        if (nout >= 500 && (nout / s <= 2) && n >= 7000 ) {
            isSpecial = true;
        }
    }
    int runNode = -1, runType = -1, number = 0;
    vector<int>  type1 = {1, 2, 3, 4, 5, 6, 7, 8, 9 ,10};
    vector<int> type2 = {1, 2, 3, 8, 9, 10};
    vector<int> type3 = {1, 2, 9, 10};
  
    // 调整type1
    if (n <= 1000 ) {
         type1 = {8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    } 

    int cnt = 0;
     
    for (int type = 1; type >= 1; -- type) {
        int cnt1 = 0;
        for (int i = 10; i >= 8; -- i) {
            int cnt2 = 0;
            for (int &id : ids[i][type]) {
                if (packets[id].t <= curTime) ++ cnt1, ++ cnt2;
            } 
            if (cnt2 && cnt1 >= batchLimit[i][type]) {
                if (cnt2 <= batchLimit[i][1] * 2 ) {
                    auto [x, y] = costPart[i][cnt2];
                    // 选择最大的调度
                    cnt2 = max(x, y);
                }
                cnt2 = min(cnt2, batchLimit[i][1]);
                dispatch.push_back({cnt2, {i, type}});
                return;
            }
        }
    }
    vector<int> listType = {3, 2, 1};
    int s = 0;
    for (const auto& node : type1) {
        int cnt = 0;
        for (int &id : ids[node][1]) {
            if (packets[id].t <= curTime) ++ cnt;
        }
        s += ids[node][1].size();
        
    }
    for (int type : listType) {
        vector<int> list;
        switch(type) {
            case 1 : list = move(type1); break;
            case 2 : list = move(type2); break;
            case 3 : list = move(type3); break;
        }
        int pos = -1;
        for (const int& node : list) {
            ++ pos;
            int i = node;
            int cnt = 0;
            for (int &id : ids[i][type]) {
                if (packets[id].t <= curTime) ++ cnt;
            }
            int allCnt = ids[i][type].size();
            bool isCatch = false;
            if (cnt > 0) {
                if (type == 1 && isOpen) {
                    if (i == 1 &&  ids[1][1].size() == 16 && s == 16) {
                        dispatch = exc16;
                        isCatch = true;
                    }
                    else if (i == 1 &&  ids[1][1].size() == 24 && s == 24) {
                        dispatch = exc24;
                        isCatch = true;
                    }
                    else if (i == 1 &&  ids[1][1].size() == 32 && s == 32) {
                        continue;
                        isCatch = true;
                    }
                }
                if (!isCatch) {
                    if (i <= 3) {
                    // 选择当前划分的最好cost
                        if (cnt <= batchLimit[node][type] * 2 && type == 1) {
                            auto [x, y] = costPart[node][cnt];
                            // 选择最大的调度
                            cnt = max(x, y);
                            dispatch.push_back({cnt, {i, type}});
                            isCatch = true;
                        }
                    } else {
                        if (node == 4) {
                            if (queueTime <= curTime) {
                                if (cnt == 11) {
                                    dispatch.push_back({3, {i, type}});
                                    dispatch.push_back({4, {i, type}});
                                    dispatch.push_back({4, {i, type}});
                                    isCatch = true;
                                }
                                else if (cnt == 10) {
                                    dispatch.push_back({4, {i, type}});
                                    dispatch.push_back({3, {i, type}});
                                    dispatch.push_back({3, {i, type}});
                                    isCatch = true;
                                }
                                else if ((cnt > 4 || (cnt > 1 && queueTime <= curTime) )&& cnt <= 8) {
                                    // 拆分
                                    int a = (cnt + 1) / 2;
                                    int b = cnt - a;
                                    dispatch.push_back({a, {i, type}});
                                    dispatch.push_back({b, {i, type}});
                                    isCatch = true;
                                }
                            }

                        }
                        if (node == 5) {
                            if (cnt == 5 && allCnt == 8) {
                                dispatch.push_back({4, {5, type}});
                                dispatch.push_back({4, {6, type}});
                                dispatch.push_back({4, {5, type}});
                                dispatch.push_back({4, {6, type}});
                            } else {
                                cnt = min(cnt, batchLimit[i][type]);
                                dispatch.push_back({cnt, {i, type}});
                                cnt = min(cnt + (int)ids[6][1].size(), batchLimit[6][type]);
                                dispatch.push_back({cnt, {i + 1, type}});
                            }
  
                            isCatch = true;
                        }

                        if (node == 7 && queueTime <= curTime) {
                            cnt = min(cnt, batchLimit[7][type]);
                            dispatch.push_back({cnt, {7, type}});

                            
                            if ((allCnt - cnt <= 2 && allCnt - cnt >= 1) && (cnt >= 4 && cnt <= batchLimit[7][1])) {
                                int nextTime = curTime + cost[7][1][cnt];
                                int cnt1 = 0;
                                for (int i = 0; i < ids[7][1].size(); ++ i) if (packets[ids[7][1][i]].t <= nextTime) ++ cnt1;
                                if (cnt1 == allCnt) {
                                    int sub = allCnt - cnt;
                                    dispatch.push_back({sub, {7, type}});
                                    cnt = cnt + sub;
                                }

        
                            }
                            for (int j = 8; j <= 10; ++ j) {
                                if (j == 10 && cnt  +  (int)ids[10][1].size() == 8) {
                                    dispatch.push_back({4, {10, 1}});
                                    dispatch.push_back({4, {10, 1}});
                                    isCatch = true;
                                    break;
                                } 
                                cnt = min(cnt + (int)ids[j][1].size(), batchLimit[j][type]);

        
                                auto[x, y] = costPart[j][cnt];
                                if (x != 0) {
                                    dispatch.push_back({x, {j, type}});                                     
                                }
                                if (y != 0) {
                                    dispatch.push_back({y, {j, type}}); 
                                }

                        
                            }
            


                            isCatch = true;
                        } 

                        if (node >= 8) {
                            // 选择满的过去
                            for (int i = 10; i >= 8; -- i) {
                                int cnt2 = 0;
                                for (int &id : ids[i][type]) {
                                    if (packets[id].t <= curTime)  ++ cnt2;
                                } 
                                if ( cnt2 && cnt2 >= batchLimit[i][type]) 
                                {
                                    dispatch.push_back({ batchLimit[i][type], {i, type}});
                                    return;
                                }
                            }
                        }
                    }
                }

                if (!isCatch) {
                    if (cnt <= batchLimit[node][type] * 2 && type == 1) {
                        auto [x, y] = costPart[node][cnt];
                        // 选择最大的调度
                        cnt = max(x, y);
                        dispatch.push_back({cnt, {i, type}});
                    } else {
                        cnt = min(cnt, batchLimit[i][type]);
                        dispatch.push_back({cnt, {i, type}});
                    }

                }

                return;
   
            }
        }
    }


    return ;



}
int main()
{
    ios::sync_with_stdio(false); 
    cin.tie(0);

    for (int i1 = 0; i1 < 20; ++i1) 
    {
        int i, j, b;
        cin >> i >> j >> b;
        batchLimit[i][j] = b;
        for (int k = 1; k <= b; ++k) 
        {
            cin >> cost[i][j][k];
        }
    }
    initNecessaryData();

    cin >> c4 >> c6 >> cr;
    cin >> n;

    while (true)
    {
        bool any = false;
        bool flag = false;
         while (true) {
            // 选择当前数量最多的来运行
            selectMax();
            if (!dispatch.empty()) {
                for (int i = 0; i < dispatch.size(); ++ i) {
                    auto [number, all]  = dispatch[i];
                    int node = all.first, selectType = all.second;
                    any = true;
                    bool isFull = TakeSameTypePacket(node, selectType, number);
                }
            } else {
                break;
            }
        }

        if (any)
        {
            continue;
        }


        ReceivePackets(curTime);

        if (cntType[2] || cntType[3]) isOpen = false;

    }

    return 0;
}










