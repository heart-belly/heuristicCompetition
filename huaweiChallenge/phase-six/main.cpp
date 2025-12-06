/****
 * 说明 : 所有索引都是从1开始
 * 资源池中的服务器数量最多5000个
***/
// 优化
#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)
#include <iostream>
#include <cstring>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <queue>
#include <bits/stdc++.h>


using namespace std;
typedef pair<int, int> PII;
typedef pair<double ,int> PDI;
#define x first
#define y second
const int N = 9, P = 33, R = 9, S = 17, K = 3, M1 = 2e5 + 10, M2 = 2e4 + 10, VMNUMBER = 21;

// 节点级别
struct Node {
    int useCpu, useMemory;
    double rate;
    unordered_set<int> deployedVmsG; // 存储在该节点上部署的虚拟机
};
// 服务器级别
struct Server {
    Node node[K];
    bool antiAffinity;
    bool affinity;
    double rate;
    int vmNumber;
};
// 机架级别
struct Rack {
    Server servers[S];
    bool antiAffinity;
    bool affinity;
    bool hasServerAnti;
    double rate;
    int vmNumber;
};
// pod级别
struct Pod {
    Rack racks[R];
    bool affinity;
    double rate;
};

// 网络级别
struct Network {
    Pod pods[P];
    bool affinity;
    bool hasRackAnti;
    double rate;
}network[N];

// 虚拟机的类型
struct VmClass {
    int nodeNumber;
    int cpu, memory;
} vmCls[VMNUMBER];

// 创建的虚拟机
struct Vm {
    int vmClass; // 虚拟机的类型
    int groupId; // 所在哪一组
    int path[6]; // 表示存放的虚拟机的路径（path[1] : network; path[2] : pod; path[3] : rack;
    //  path[4] : sever; path[5] : {0:两个节点都有; 1 : 第一个节点; 2 : 第二个节点}
    int partitionId; // 若有分区，则不为0;
} vm[M1];
// 虚拟机编号
int vmIndex;

// 存放组的不同的组(first : 组的约束类型; second : 有无分区的约束)
struct Group {
    int resraintClass; // 约束类型
    int partition; // 分区数量
    unordered_set<int> hasVmG;// 组内虚拟机
    vector<unordered_set<int>> partHasRack; // 若有分区，则保存每个区的机架的编号
    vector<unordered_set<int>> partHasVm; // 若有分区，则对应的分区所拥有的虚拟机
    unordered_set<int> allPartHasRack; // 已经被该组划分的分区中的机架

    int networkAffinityId; // 若不存在，则为0
    int rackAffinityId; // 若不存在，则为0
}group[M1];

// 存放组的编号
int groupIndex;
// 创建组的约束类型
struct GroupRestraint {
    int contains[5]; // {0 : 表}
}groupRestraint[10]; // 10种约束类型

// 创建的虚拟机
// n : 网络域的数量, p : pod域的数量, r : 机架域的数量, s : 服务器域的数量, k : 结点域的数量
int n, p, r, s, k;
vector<int> topologicalDomain;
// 服务器中每个节点的cpu和memory资源数量
int nodeCpu, nodeMemory;
// 虚拟机的类型数量
int F;
// 保存每次创建的虚拟机编号
vector<int> curCreatVm;
/*
* 优先队列的排序要求
*/
struct cmp1
{
	bool operator()(const PII& node1, const PII& node2)
	{
        if (node1.x != node2.x) return node1.x > node2.x;
        
        srand((unsigned)time(0));
        int rand1 = rand() % 100;
        int rand2 = rand() % 100;

        return rand1 < rand2;
	}
};

/*
* 函数声明
*/
void init();
void input();
void initParameter();
void interaction();
void updateRate(int rackId);
void deleteVm(int vmId);
bool solve(int groupId);
void updateNodeResource(Server &server, Vm &vm, int rackId, int serverId, int vmId); // 更新节点所占有的资源
/*
* 根据不同的约束条件去寻找放置虚拟机的拓扑域
*/
bool solveConstrain0(int groupId); // 无约束
bool solveConstrain1(int groupId); // 机架亲和性
bool solveConstrain2(int groupId); // 整个pod分配（只有一次创建需求）
bool solveConstrain3(int groupId); // 网络亲和性
bool solveConstrain4(int groupId); // 虚拟机独占服务器
bool solveConstrain5(int groupId); // 虚拟机独占机架
bool solveConstrain6(int groupId); // 机架分区
bool solveConstrain7(int groupId); // 机架亲和性 + 服务器独占（只有一次创建需求）
bool solveConstrain8(int groupId); // 网络亲和性 + 机架独占
bool solveConstrain9(int groupId); // 网络亲和性 + 整个pod分配

/*
* 根据每个域的编号寻找路径
*/
vector<int> findNetworkPath(int networkId); // path[1] : network
vector<int> findPodPath(int podId); // path[1] : network; path[2] : pod
vector<int> findRackPath(int rackId); // path[1] : network; path[2] : pod; path[3] : rack
vector<int> findServerPath(int serverId); // path[1] : network; path[2] : pod; path[3] : rack; path[4] : serverId 
vector<int> findNodePath(int nodeId);// path[1] : network; path[2] : pod; path[3] : rack; path[4] : serverId; path[5] : nodeId


int main() {
    init();
    input();
    initParameter();
    interaction();
}

void init() {
    // 关闭同步输入流
    std::ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);

    // 初始化拓扑域
    topologicalDomain = vector<int>(6, 0);
}

void input() {
    // 第一行输入
    cin >> n >> p >> r >> s >> k;
    // 第二行输入(结点的cpu和memory数量)
    cin >> nodeCpu >> nodeMemory;
    // 第三行输入
    cin >> F;
    // 第四行输入（不同类型的虚拟机）
    for (int i = 1; i <= F; i++) {
        cin >> vmCls[i].nodeNumber >> vmCls[i].cpu >> vmCls[i].memory;
    }

    // 将每个拓扑域的数量以数组形式保存
    topologicalDomain[1] = n, topologicalDomain[2] = p, topologicalDomain[3] = r, topologicalDomain[4] = s;
    topologicalDomain[5] = k;


}
void initParameter() {
    // 初始化各个域
    for (int i = 1; i <= n; i++) {
        // 网络域
        network[i].affinity = false;
        network[i].rate = 0.0;
        for (int j = 1; j <= p; j++) {
            // pod域
            network[i].pods[j].affinity = false;
            network[i].pods[j].rate = 0.0;
            for (int k = 1; k <= r; k++) {
                // 机架域
                network[i].pods[j].racks[k].affinity = false;
                network[i].pods[j].racks[k].antiAffinity = false;
                network[i].pods[j].racks[k].rate = 0.0;
                for (int l = 1; l <= s; l++) {
                    // 服务器域
                    network[i].pods[j].racks[k].servers[l].antiAffinity = false;
                    network[i].pods[j].racks[k].servers[l].affinity = false;
                    network[i].pods[j].racks[k].servers[l].rate = 0.0;
                }
            }
        }

    }
}

void interaction() {
    int op;
    while (cin >> op) {
        if (op == 1) {
            // 创建组
            int j, tj, kj;
            cin >> j >> tj >> kj;
            group[j].resraintClass = tj, group[j].partition = kj;
            // 若为组约束条件6
            if (tj == 6) {
                //初始化分区相关信息
                group[j].partHasVm = vector<unordered_set<int>>(kj + 1);
                group[j].partHasRack = vector<unordered_set<int>>(kj + 1);
                group[j].allPartHasRack = {};
            }
            // 处理相关操作

        }
        else if (op == 2) {
            int l, f, gj;
            cin >> l >> f >> gj;
            // 创建虚拟机
            curCreatVm = vector<int>(l);
            for (int i = 0; i < l; i ++ ) {
                int id;
                cin >> id;
                // 将对应的虚拟机存入
                vm[id].vmClass = f, vm[id].groupId = gj;
                // 将虚拟机的id存入对应的PG中
                group[gj].hasVmG.insert(id);
                // 将虚拟机Id存入当前
                curCreatVm[i] = id;

                // //debug
                // cout << "id: " << id << "vmclassId : " << vm[id].vmClass << endl;
                // cout << "vmCpu: " << vmCls[vm[id].vmClass].cpu << " " << "vmMemory:" << vmCls[vm[id].vmClass].memory << endl;
            }
            // debug
            // 存入完毕后放置新创建的虚拟机
            if (solve(gj)) {
                //成功放置
                for (int i = 0; i < l; i ++ ) {
                    // 输入存储路径
                    int id = curCreatVm[i];
                    // debug
                    // cout << "id : " << id << " " << "partId : "<< " " << vm[id].partitionId << endl;
                    for (int j = 1; j <= 4; j ++ ) {
                        cout << vm[id].path[j] << " ";
                    }

                    // 单独判断节点的输出
                    if (vm[id].path[5] == 0) {
                        cout << 2 << " " << 1 << " ";
                    } else {
                        cout << vm[id].path[5] << " ";
                    }

                    // 判断有无分区需求
                    cout << vm[id].partitionId << endl;
                    fflush(stdout);
                }


            }
            else {
                break;
            }

        }
        else if (op == 3) {

            // 删除虚拟机
            int number;
            cin >> number;
            for (int i = 0; i < number; i ++ ) {
                int id;
                cin >> id;
                deleteVm(id);
            }

        }
        else if (op == 4){
            // 终止交互
            break;
        }
    }

    // terminal
    cout << -1 << endl;
    fflush(stdout);
}

bool solve(int groupId) {


    int constrainCLS = group[groupId].resraintClass;

    bool flag = true;
    switch (constrainCLS) {
    case 0:
        flag = solveConstrain0(groupId);
        break;
    case 1:
        flag = solveConstrain1(groupId);
        break;        
    case 2:
        flag = solveConstrain2(groupId);
        break;
    case 3:
        flag = solveConstrain3(groupId);
        break; 
    case 4:
        flag = solveConstrain4(groupId);
        break;
    case 5:
        flag = solveConstrain5(groupId);
        break;        
    case 6:
        flag = solveConstrain6(groupId);
        break;
    case 7:
        flag = solveConstrain7(groupId);
        break; 
    case 8:
        flag = solveConstrain8(groupId);
        break;
    case 9:
        flag = solveConstrain9(groupId);
        break; 

    }


    return flag;
}


bool solveConstrain0(int groupId) {
  // 创建优先队列保证组内分区
    auto &g = group[groupId];
    int ss = 0;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap1;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap2;
    priority_queue<PDI, vector<PDI>> heap3;
    priority_queue<PDI, vector<PDI>> heap4;

    
    for (int vmId : curCreatVm) {
        // 网络域
        bool flag = false;
            // 统计网络的占有率
        for (int i = 1; i <= n; i ++ ) {
            double regulate = 0.0;
            if (network[i].hasRackAnti) {
                regulate = 0.3;
            }
            heap1.push({network[i].rate + regulate, i});
        }

        while (heap1.size()) {
            auto [_, networkId] = heap1.top();
            heap1.pop();
            auto &networkTemp = network[networkId];
            for (int i = 1; i <= p; i ++ ) {
                heap2.push({networkTemp.pods[i].rate, i});
            }
            // pod域
            while (heap2.size()) {
                auto [_, podId] = heap2.top();
                heap2.pop();
                auto &podTemp = networkTemp.pods[podId];
                for (int i = 1; i <= r; i ++ ) {
                    if (podTemp.racks[i].antiAffinity) continue;
                    heap3.push({podTemp.racks[i].rate, i});
                }
                int podIdReal = (networkId - 1) * p + podId;
                // rack域
                while (heap3.size()) {
                    auto [_, rackId] = heap3.top();
                    heap3.pop();
                    auto &rackTemp = podTemp.racks[rackId];
                    int rackIdReal = (podIdReal - 1) * r + rackId;

                    if (rackTemp.antiAffinity) continue;

                    for (int i = 1; i <= s; i ++ ) {
                        if (rackTemp.servers[i].antiAffinity) continue;
                        // 找到虚拟机
                        auto &vmTemp = vm[vmId];
                        // 计算serverId
                        int serverIdReal = (rackIdReal - 1) * s + i;
                        // 根据虚拟机所需节点的数量进行选择
                        auto &server = rackTemp.servers[i];

                        int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                        int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                        int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                        int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                        bool flag1 = false;
                        if (vmCls[vmTemp.vmClass].nodeNumber == 2) {
                            if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory &&
                            node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) {
                                flag1 = true;
                            }
                        } else if (vmCls[vmTemp.vmClass].nodeNumber == 1) {
                            if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                            if (node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                        }

                        if (flag1) {

                            updateNodeResource(rackTemp.servers[i], vmTemp, rackIdReal, serverIdReal, vmId);
                            // 更新机架和服务器防止vm的数量
                            rackTemp.vmNumber ++;
                            server.vmNumber ++;
                            ss ++;
                            flag = true;
                            break;
                        }


                    }
                    if (flag) break;

                }
                if (flag) break;
            }
            if (flag) break;
        }
    }

    return ss == curCreatVm.size();    
}

bool solveConstrain1(int groupId) {

  // 创建优先队列保证组内分区
    auto &g = group[groupId];
    int ss = 0;
    int vmcls = vm[curCreatVm[0]].vmClass;
    priority_queue<PDI, vector<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap3;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap4;

    // 第一次分配亲和网络
    if (g.rackAffinityId == 0) {
        // 网络域
        bool flag = false;
        int index = 0;
            // 统计网络的占有率
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        while (heap1.size()) {
            auto [_, networkId] = heap1.top();
            heap1.pop();
            auto &networkTemp = network[networkId];
            for (int i = 1; i <= p; i ++ ) {
                heap2.push({networkTemp.pods[i].rate, i});
            }
            // pod域
            while (heap2.size()) {
                auto [_, podId] = heap2.top();
                heap2.pop();
                auto &podTemp = networkTemp.pods[podId];
                for (int i = 1; i <= r; i ++ ) {
                    if (podTemp.racks[i].antiAffinity) continue;
                    heap3.push({podTemp.racks[i].rate, i});
                }
                int podIdReal = (networkId - 1) * p + podId;
                // rack域
                while (heap3.size()) {
                    auto [_, rackId] = heap3.top();
                    heap3.pop();
                    auto &rackTemp = podTemp.racks[rackId];
                    int rackIdReal = (podIdReal - 1) * r + rackId;

                    if (rackTemp.antiAffinity) continue;

                    // 判断机架下面的服务器能不能容纳所有vm

                    int canPutInNumber = 0;
                    for (int i = 1; i <= s; i ++ ) {
                        if (rackTemp.servers[i].antiAffinity) continue;
                        auto &server = rackTemp.servers[i];
                        // 根据虚拟机所需节点的数量进行选择
                        int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                        int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                        int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                        int node2RemainMemory = nodeMemory - server.node[2].useMemory;
                        // 统计所有的个数
                        int needCpu = vmCls[vmcls].cpu, needMemory = vmCls[vmcls].memory;

                        int acc = 0;
                        if (vmCls[vmcls].nodeNumber == 2) {

                            acc += min({node1RemainCpu / needCpu, node1RemainMemory / needMemory, node2RemainCpu / needCpu, node2RemainMemory / needMemory});
                        } else if (vmCls[vmcls].nodeNumber == 1) {
                            acc += min({node1RemainCpu / needCpu, node1RemainMemory / needMemory});
                            acc += min({node2RemainCpu / needCpu, node2RemainMemory / needMemory});
                        }

                        if (acc > 0) {
                            canPutInNumber += acc;
                            heap4.push({server.rate, i});
                        }
                    }


                    if (canPutInNumber >= curCreatVm.size()) {
                        // 放置在该机架下面
                        while (heap4.size()) {
                            auto [_, id] = heap4.top();
                            heap4.pop();
                            int serverId = (rackIdReal - 1) * s + id;
                            // 更新机架和服务器上的vm数量
                            auto &server = rackTemp.servers[id];
                            int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                            int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                            int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                            int node2RemainMemory = nodeMemory - server.node[2].useMemory;
                            bool flag1 = false;
                            if (vmCls[vmcls].nodeNumber == 2) {
                                if (node1RemainCpu >= vmCls[vmcls].cpu && node1RemainMemory >= vmCls[vmcls].memory &&
                                node2RemainCpu >= vmCls[vmcls].cpu && node2RemainMemory >= vmCls[vmcls].memory) {
                                    flag1 = true;
                                }
                            } else if (vmCls[vmcls].nodeNumber == 1) {
                                if (node1RemainCpu >= vmCls[vmcls].cpu && node1RemainMemory >= vmCls[vmcls].memory) flag1 = true;
                                if (node2RemainCpu >= vmCls[vmcls].cpu && node2RemainMemory >= vmCls[vmcls].memory) flag1 = true;
                            }

                            if (flag1) {
                                Vm &vM = vm[curCreatVm[index]];
                                updateNodeResource(server, vM, rackId, serverId, curCreatVm[index]);
                                index ++;
                                ss ++;
                                // 更新机架和服务器上的vm数量
                                network[networkId].pods[podId].racks[rackId].vmNumber ++;
                                server.vmNumber ++;

                                heap4.push({server.rate, id});

                                if (index == curCreatVm.size()) {
                                    flag = true;
                                    break;
                                }
                            }

                        }

                        rackTemp.affinity = true;
                        g.rackAffinityId = rackIdReal;
                        flag = true;
                        break;

                    }

                    heap4 = {};
                    if (flag) break;
                }
                if (flag) break;
            }
            if (flag) break;
        }
    } else {
        // 放置在该机架下面
        vector<int> path = findRackPath(g.rackAffinityId);
        auto &rackTemp = network[path[1]].pods[path[2]].racks[path[3]];


        for (int vmId : curCreatVm) {
            bool flag = false;

            for (int i = 1; i <= s; i ++ ) {
                if (rackTemp.servers[i].antiAffinity) continue;
                auto &server = rackTemp.servers[i];
                auto &vmTemp = vm[vmId];
                int podIdReal = (path[1] - 1) * p + path[2];
                int rackIdReal = (podIdReal - 1) * r + path[3];
                int serverIdReal = (rackIdReal - 1) * s + i;
                // 根据虚拟机所需节点的数量进行选择
                int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                bool flag1 = false;
                if (vmCls[vmTemp.vmClass].nodeNumber == 2) {
                    if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory &&
                    node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) {
                        flag1 = true;
                    }
                } else if (vmCls[vmTemp.vmClass].nodeNumber == 1) {
                    if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                    if (node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                }

                if (flag1) {
                    updateNodeResource(server, vmTemp, rackIdReal, serverIdReal, vmId);
                    // 更新机架和服务器防止vm的数量
                    rackTemp.vmNumber ++;
                    server.vmNumber ++;
                    ss ++;
                    flag = true;
                    break;
                }
                if (flag) break;
            }
        }
    }



    return ss == curCreatVm.size();    
}

bool solveConstrain2(int groupId) {
    // 创建优先队列保证组内分区
    auto &g = group[groupId];
    // 寻找pod的个数(题目一定保正整个pod分配一定是pod所能容纳服务器的倍数)
    int podNum = (int)curCreatVm.size() / (r * s);
    // 根据网络拓扑域占有率寻找pod数量
    priority_queue<PDI, vector<PDI>> heap;


    bool flag = false;
    int index = 0;
    for (int i = 0; i < podNum; i ++ ) {

        for (int i = 1; i <= n; i ++ ) {
            heap.push({network[i].rate, i});
        }

        while (heap.size()) {
            auto [_, networkId] = heap.top();
            heap.pop();

            for (int i1 = 1; i1 <= p; i1 ++ ) {
                if (network[networkId].pods[i1].rate == 0) {
                    for (int j = 1; j <= r; j ++ ) {
                        for (int j1 = 1; j1 <= s; j1 ++ ) {
                            auto &vM = vm[curCreatVm[index]];
                            auto &server = network[networkId].pods[i1].racks[j].servers[j1];
                            auto vmclass = vM.vmClass;
                            int podId  = (networkId - 1) * p + i;
                            int rackId = (podId - 1) * r + j;
                            int serverId = (rackId - 1) * s + j1;

                            updateNodeResource(server, vM, rackId, serverId, curCreatVm[index]);
                            index ++;
                            // 更新机架和服务器上的vm数量
                            network[networkId].pods[i1].racks[j].vmNumber ++;
                            server.vmNumber ++;
                        }
                    }
                    flag = true;
                }
                if (flag) break;
            }
            if (flag) break;

        }
    }

    return index == curCreatVm.size();

}

bool solveConstrain3(int groupId) {
    // 创建优先队列保证组内分区
    auto &g = group[groupId];
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>> heap3;
    priority_queue<PDI, vector<PDI>> heap4;


    // 第一次分配亲和网络
    if (g.networkAffinityId == 0) {
        // 根据占有率选取网络
        int ss = 0;
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        auto [_, networkId] = heap1.top();
        g.networkAffinityId = networkId;
    }

    int ss = 0;

    for (int vmId : curCreatVm) {
        // 网络域
        bool flag = false;

        auto &networkTemp = network[g.networkAffinityId];
        for (int i = 1; i <= p; i ++ ) {
            heap2.push({networkTemp.pods[i].rate, i});
        }
        // pod域
        while (heap2.size()) {
            auto [_, podId] = heap2.top();
            heap2.pop();
            auto &podTemp = networkTemp.pods[podId];
            for (int i = 1; i <= r; i ++ ) {
                if (podTemp.racks[i].antiAffinity) continue;
                heap3.push({podTemp.racks[i].rate, i});
            }
            int podIdReal = (g.networkAffinityId - 1) * p + podId;
            // rack域
            while (heap3.size()) {
                auto [_, rackId] = heap3.top();
                heap3.pop();
                auto &rackTemp = podTemp.racks[rackId];
                int rackIdReal = (podIdReal - 1) * r + rackId;

                if (rackTemp.antiAffinity) continue;
                for (int i = 1; i <= s; i ++ ) {
                    if (rackTemp.servers[i].antiAffinity) continue;

                    // 找到虚拟机
                    auto &vmTemp = vm[vmId];
                    // 计算serverId
                    int serverIdReal = (rackIdReal - 1) * s + i;
                    auto &server = rackTemp.servers[i];
                    int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                    int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                    int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                    int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                    bool flag1 = false;
                    if (vmCls[vmTemp.vmClass].nodeNumber == 2) {
                        if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory &&
                        node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) {
                            flag1 = true;
                        }
                    } else if (vmCls[vmTemp.vmClass].nodeNumber == 1) {
                        if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                        if (node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                    }

                    if (flag1) {
                        updateNodeResource(server, vmTemp, rackIdReal, serverIdReal, vmId);
                        // 更新机架和服务器防止vm的数量
                        rackTemp.vmNumber ++;
                        server.vmNumber ++;
                        ss ++;
                        flag = true;
                        break;
                    }

                }
                if (flag) break;

            }
            if (flag) break;
        }
    }

    return ss == curCreatVm.size();

}
bool solveConstrain4(int groupId) {

    // 创建优先队列保证组内分区
    auto &g = group[groupId];
    int ss = 0;
    priority_queue<PDI, vector<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>> heap3;
    priority_queue<PDI, vector<PDI>> heap4;
    
    for (int vmId : curCreatVm) {
        // 网络域
        bool flag = false;
            // 统计网络的占有率
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        while (heap1.size()) {
            auto [_, networkId] = heap1.top();
            heap1.pop();
            auto &networkTemp = network[networkId];
            for (int i = 1; i <= p; i ++ ) {
                heap2.push({networkTemp.pods[i].rate, i});
            }
            // pod域
            while (heap2.size()) {
                auto [_, podId] = heap2.top();
                heap2.pop();
                auto &podTemp = networkTemp.pods[podId];
                for (int i = 1; i <= r; i ++ ) {
                    if (podTemp.racks[i].antiAffinity) continue;
                    heap3.push({podTemp.racks[i].rate, i});
                }
                int podIdReal = (networkId - 1) * p + podId;
                // rack域
                while (heap3.size()) {
                    auto [_, rackId] = heap3.top();
                    heap3.pop();
                    auto &rackTemp = podTemp.racks[rackId];
                    int rackIdReal = (podIdReal - 1) * r + rackId;

                    if (rackTemp.antiAffinity) continue;
                    for (int i = 1; i <= s; i ++ ) {
                        if (rackTemp.servers[i].antiAffinity || rackTemp.servers[i].rate > 0 || rackTemp.servers[i].vmNumber > 0 ) continue;

                        // 找到对应的服务器防止节点
                        rackTemp.servers[i].antiAffinity = true;
                        // 找到虚拟机
                        auto &vmTemp = vm[vmId];
                        // 计算serverId
                        int serverIdReal = (rackIdReal - 1) * s + i;

                        updateNodeResource(rackTemp.servers[i], vmTemp, rackIdReal, serverIdReal, vmId);
                        // 更新机架和服务器防止vm的数量
                        rackTemp.vmNumber ++;
                        rackTemp.servers[i].vmNumber ++;
                        ss ++;
                        flag = true;
                        break;

                    }
                    if (flag) break;

                }
                if (flag) break;
            }
            if (flag) break;
        }
    }

    return ss == curCreatVm.size();
}


bool solveConstrain5(int groupId) {

 // 创建优先队列保证组内分区
    auto &g = group[groupId];
    int ss = 0;
    priority_queue<PDI, vector<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>> heap3;
    priority_queue<PDI, vector<PDI>> heap4;

    
    for (int vmId : curCreatVm) {
        // 网络域
        bool flag = false;
            // 统计网络的占有率
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        while (heap1.size()) {
            auto [_, networkId] = heap1.top();
            heap1.pop();
            auto &networkTemp = network[networkId];
            for (int i = 1; i <= p; i ++ ) {
                heap2.push({networkTemp.pods[i].rate, i});
            }
            // pod域
            while (heap2.size()) {
                auto [_, podId] = heap2.top();
                heap2.pop();
                auto &podTemp = networkTemp.pods[podId];
                for (int i = 1; i <= r; i ++ ) {
                    if (podTemp.racks[i].antiAffinity || podTemp.racks[i].rate != 0 || podTemp.racks[i].vmNumber > 0) continue;
                    int podIdReal = (networkId - 1) * p + podId;
                    int rackIdReal = (podIdReal - 1) * r + i;
                    auto &rackTemp = podTemp.racks[i];
                    // 找到对应的机架
                    rackTemp.antiAffinity = true;

                    for (int j = 1; j <= s; j ++ ) {
                        if (rackTemp.servers[j].antiAffinity || rackTemp.servers[j].rate != 0 || rackTemp.servers[j].vmNumber > 0) continue;

                        // 找到对应的服务器
                        rackTemp.servers[j].antiAffinity = true;
                        // 找到虚拟机
                        auto &vmTemp = vm[vmId];
                        // 计算serverId
                        int serverIdReal = (rackIdReal - 1) * s + j;

                        updateNodeResource(rackTemp.servers[j], vmTemp, rackIdReal, serverIdReal, vmId);
                        // 更新机架和服务器防止vm的数量
                        rackTemp.vmNumber ++;
                        rackTemp.servers[j].vmNumber ++;
                        ss ++;
                        flag = true;
                        break;

                    }
                    if (flag) break;
                }
                if (flag) break;
            }
            if (flag) break;
        }
        
    }

    return ss == curCreatVm.size();
}

bool solveConstrain6(int groupId) {

    // 创建优先队列保证组内分区
    auto &g = group[groupId];
    priority_queue<PII, vector<PII>, greater<PII>> heap;
    for (int i = g.partition; i >= 1; i -- ) {
        heap.push({g.partHasVm[i].size(), i});


    }


    int ss = 0;
    // 处理当前需要放置发虚拟机
    for (int vmId : curCreatVm) {
        // 得到小顶堆头部元素
        auto [sz, partId] = heap.top();
        heap.pop();
        // 将该虚拟机放入该分区(处理小顶堆相关数据)
        vm[vmId].partitionId = partId;

        g.partHasVm[partId].insert(vmId);
        heap.push({g.partHasVm[partId].size(), partId});
        // debug



        // 寻找该分区的能够容纳的机架

        // 根据每个（已有）机架的占有率来选择哪个机架来容纳
        priority_queue<PDI, vector<PDI>> heap1;
        for (int rackId : g.partHasRack[partId]) {
            vector<int> path = findRackPath(rackId);
            auto &curRack = network[path[1]].pods[path[2]].racks[path[3]];

            // 筛选机架（主要看当前机架能不能放vm)
            // 当前机架有反亲和性
            if (curRack.antiAffinity) continue;

            heap1.push({curRack.rate, rackId});

        }


        bool flag1 = false;

        while (heap1.size()) {
            auto [_, rackId] = heap1.top();
            heap1.pop();
            vector<int> path = findRackPath(rackId);
            auto &curRack = network[path[1]].pods[path[2]].racks[path[3]];
            // 机架内服务器的选择（从编号小到编号大进行选择）
            bool flag2 = false;

            for (int i = 1; i <= s; i ++ ) {
                // 当前服务器具有反亲和性
                if (curRack.servers[i].antiAffinity) continue;

                auto &server = curRack.servers[i];
                auto vmclass = vm[vmId].vmClass;

                // 根据虚拟机所需节点的数量进行选择
                int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                if (vmCls[vmclass].nodeNumber == 2) {
                    if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory
                    && node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                        // 将虚拟机放置在该服务器上(更新)
                        server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                        server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                        // 更新节点的利用率
                        for (int j = 1; j <= 2; j ++ ) {
                            server.node[j].rate = 1.0 / 2 * (server.node[j].useCpu / nodeCpu + server.node[j].useMemory / nodeMemory);
                        }
                        // 更新服务器的利用率
                        server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);
                        // 依次更新服务器以上利用率
                        updateRate(rackId);
                        // 更新虚拟机信息
                        vm[vmId].path[1] = path[1], vm[vmId].path[2] = path[2], vm[vmId].path[3] = path[3], vm[vmId].path[4] = i, vm[vmId].path[5] = 0;
                        flag2 = true;

                        // 更新机架信息
                        // 更新机架和服务器防止vm的数量
                        curRack.vmNumber ++;
                        server.vmNumber ++;

                        server.node[1].deployedVmsG.insert(vmId);
                        server.node[2].deployedVmsG.insert(vmId);
                    }
                } else if (vmCls[vmclass].nodeNumber == 1){
                    // 没有一个节点满足要求
                    if ((node1RemainCpu < vmCls[vmclass].cpu || node1RemainMemory < vmCls[vmclass].memory)
                    && (node2RemainCpu < vmCls[vmclass].cpu || node2RemainMemory < vmCls[vmclass].memory)) continue;

                    // 更新虚拟机信息
                    vm[vmId].path[1] = path[1], vm[vmId].path[2] = path[2], vm[vmId].path[3] = path[3], vm[vmId].path[4] = i;

                    if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory && 
                    node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                        // 计算两个节点的利用率
                        double rate1 = 1.0 / 2 * ((nodeCpu - node1RemainCpu) / nodeCpu + (nodeMemory - node1RemainMemory) / nodeMemory);
                        double rate2 = 1.0 / 2 * ((nodeCpu - node2RemainCpu) / nodeCpu + (nodeMemory - node2RemainMemory) / nodeMemory);
                        if (rate1 >= rate2) {
                            // 将虚拟机放置在该服务器上(更新)
                            server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                            // 更新节点的利用率
                            server.node[1].rate = 1.0 / 2 * (server.node[1].useCpu / nodeCpu + server.node[1].useMemory / nodeMemory);

                            vm[vmId].path[5] = 1;
                            server.node[1].deployedVmsG.insert(vmId);
                        } else {
                            // 将虚拟机放置在该服务器上(更新)
                            server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                            // 更新节点的利用率
                            server.node[2].rate = 1.0 / 2 * (server.node[2].useCpu / nodeCpu + server.node[2].useMemory / nodeMemory);     

                            vm[vmId].path[5] = 2;   
                            server.node[2].deployedVmsG.insert(vmId);                 
                        }
                    } else if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory) {
                            // 将虚拟机放置在该服务器上(更新)
                            server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                            // 更新节点的利用率
                            server.node[1].rate = 1.0 / 2 * (server.node[1].useCpu / nodeCpu + server.node[1].useMemory / nodeMemory);  

                            vm[vmId].path[5] = 1;   
                            server.node[1].deployedVmsG.insert(vmId);                
                    } else if (node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                            // 将虚拟机放置在该服务器上(更新)
                            server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                            // 更新节点的利用率
                            server.node[2].rate = 1.0 / 2 * (server.node[2].useCpu / nodeCpu + server.node[2].useMemory / nodeMemory);

                            vm[vmId].path[5] = 2;   
                            server.node[2].deployedVmsG.insert(vmId);      
                                                                       
                    }

                    // 更新服务器的利用率
                    server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);
                    // 依次更新服务器以上利用率
                    updateRate(rackId);
                    flag2 = true;
                    // 更新机架和服务器防止vm的数量
                    curRack.vmNumber ++;
                    server.vmNumber ++;
                }

                if (flag2) break; 
            }

            if (flag2) flag1 = true;
            if (flag1) break;

        }
        // 当前虚拟机可以放置在已有的分区里
        if (flag1) {
            ss += 1;
            continue;
        }

        // 若该组内分区机架无法容纳当前虚拟机，则寻找新的无分区的机架放入该分区
        // 若属于该组的分区没有机架，则选择pod占有率高，机架占有率低的划分进该组的分区内

        heap1 = {};
        priority_queue<PDI, vector<PDI>, greater<PDI>> heap2;
        // 寻找pod放入heap1中
        for (int i = 1; i <= n; i ++ ) {
            for (int j = 1; j <= p; j ++ ) {
                int podId = (i - 1) * p + j;
                auto &curPod = network[i].pods[j];
                double regulate = 0.0;
                if (network[i].hasRackAnti) {
                    regulate = 0.3;
                }
                heap1.push({curPod.rate + regulate, podId});
                // cout << "netowrk :" << i << " " << "pod : " << j << " " << "rate : " << curPod.rate << endl;
                // cout << "podId : " << podId << endl;

            }
        }

        // 根据占有率从高到低依次取出pod,并计算机架的占有率
        while (heap1.size()) {
            auto [_, podId] = heap1.top();
            heap1.pop();
            vector<int> path = findPodPath(podId);
            auto &curPod = network[path[1]].pods[path[2]];
            // pod内机架的选择（从占有率低到高选择）

            for (int i = 1; i <= r; i ++ ) {
                auto &curRack = curPod.racks[i];
                // 机架编号
                int rackId = (podId - 1) * r + i;

                // 当前机架有反亲和性
                if (curRack.antiAffinity) continue;
                // 并且需要判断该机架是否已经属于这个组的某个分区
                if (group[groupId].allPartHasRack.count(rackId)) continue;

                heap2.push({curRack.rate, rackId});
            }



            while (heap2.size()) {
                auto [_, rackId] = heap2.top();
                heap2.pop();
                vector<int> path = findRackPath(rackId);
                auto &curRack = network[path[1]].pods[path[2]].racks[path[3]];
                // 机架内服务器的选择（从编号小到编号大进行选择）

                int canUseRack = -1;
                for (int i = 1; i <= s; i ++ ) {
                    // 当前服务器具有反亲和性
                    if (curRack.servers[i].antiAffinity) continue;
                    auto &server = curRack.servers[i];
                    auto vmclass = vm[vmId].vmClass;

                    // 根据虚拟机所需节点的数量进行选择
                    int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                    int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                    int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                    int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                    if (vmCls[vmclass].nodeNumber == 2) {
                        if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory
                        && node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                            // 将虚拟机放置在该服务器上(更新)
                            server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                            server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                            // 更新节点的利用率
                            for (int j = 1; j <= 2; j ++ ) {
                                server.node[j].rate = 1.0 / 2 * (server.node[j].useCpu / nodeCpu + server.node[j].useMemory / nodeMemory);
                            }
                            // 更新服务器的利用率
                            server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);
                            // 依次更新服务器以上利用率
                            updateRate(rackId);
                            // 更新虚拟机信息
                            vm[vmId].path[1] = path[1], vm[vmId].path[2] = path[2], vm[vmId].path[3] = path[3], vm[vmId].path[4] = i, vm[vmId].path[5] = 0;
                            
                            server.node[1].deployedVmsG.insert(vmId);
                            server.node[2].deployedVmsG.insert(vmId);
                            // 更新机架信息
                            canUseRack = rackId;
                            // 更新机架和服务器防止vm的数量
                            curRack.vmNumber ++;
                            server.vmNumber ++;
                        }
                    } else if (vmCls[vmclass].nodeNumber == 1){
                        // 没有一个节点满足要求
                        if ((node1RemainCpu < vmCls[vmclass].cpu || node1RemainMemory < vmCls[vmclass].memory)
                        && (node2RemainCpu < vmCls[vmclass].cpu || node2RemainMemory < vmCls[vmclass].memory)) continue;

                        // 更新虚拟机信息
                        vm[vmId].path[1] = path[1], vm[vmId].path[2] = path[2], vm[vmId].path[3] = path[3], vm[vmId].path[4] = i;

                        if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory && 
                        node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                            // 计算两个节点的利用率
                            double rate1 = 1.0 / 2 * ((nodeCpu - node1RemainCpu) / nodeCpu + (nodeMemory - node1RemainMemory) / nodeMemory);
                            double rate2 = 1.0 / 2 * ((nodeCpu - node2RemainCpu) / nodeCpu + (nodeMemory - node2RemainMemory) / nodeMemory);
                            if (rate1 >= rate2) {
                                // 将虚拟机放置在该服务器上(更新)
                                server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                                // 更新节点的利用率
                                server.node[1].rate = 1.0 / 2 * (server.node[1].useCpu / nodeCpu + server.node[1].useMemory / nodeMemory);

                                vm[vmId].path[5] = 1;
                                server.node[1].deployedVmsG.insert(vmId);
                            } else {
                                // 将虚拟机放置在该服务器上(更新)
                                server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                                // 更新节点的利用率
                                server.node[2].rate = 1.0 / 2 * (server.node[2].useCpu / nodeCpu + server.node[2].useMemory / nodeMemory);     

                                vm[vmId].path[5] = 2;  
                                server.node[2].deployedVmsG.insert(vmId);                  
                            }
                        } else if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory) {
                                // 将虚拟机放置在该服务器上(更新)
                                server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                                // 更新节点的利用率
                                server.node[1].rate = 1.0 / 2 * (server.node[1].useCpu / nodeCpu + server.node[1].useMemory / nodeMemory);  

                                vm[vmId].path[5] = 1;  
                                server.node[1].deployedVmsG.insert(vmId);                 
                        } else if (node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                                // 将虚拟机放置在该服务器上(更新)
                                server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                                // 更新节点的利用率
                                server.node[2].rate = 1.0 / 2 * (server.node[2].useCpu / nodeCpu + server.node[2].useMemory / nodeMemory);

                                vm[vmId].path[5] = 2;   
                                server.node[2].deployedVmsG.insert(vmId);      
                                                                        
                        }

                        // 更新服务器的利用率
                        server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);
                        // 依次更新服务器以上利用率
                        updateRate(rackId);

                        canUseRack = rackId;

                        // 更新机架和服务器防止vm的数量
                        curRack.vmNumber ++;
                        server.vmNumber ++;

                    }
                    if (canUseRack != -1) {
                        // 将虚拟机部署到节点上后，该节点对应的机架应该归属于某该组内的分区
                        group[groupId].partHasRack[partId].insert(canUseRack);
                        group[groupId].allPartHasRack.insert(canUseRack);
                        break;
                    }

                
                }

                if (canUseRack != -1) {
                    flag1 = true;
                    break;
                }

            }

            if (flag1) break;

        }

        if (flag1) {
            ss += 1;
            continue;
        }


    }
    return ss == curCreatVm.size();

}

bool solveConstrain7(int groupId) {
// 创建优先队列保证组内分区
    auto &g = group[groupId];
    int ss = 0;
    int vmcls = vm[curCreatVm[0]].vmClass;
    priority_queue<PDI, vector<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap3;
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap4;
    int index = 0;

    // 第一次分配亲和网络
    if (g.rackAffinityId == 0) {
        // 网络域
        bool flag = false;
            // 统计网络的占有率
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        while (heap1.size()) {
            auto [_, networkId] = heap1.top();
            heap1.pop();
            auto &networkTemp = network[networkId];
            for (int i = 1; i <= p; i ++ ) {
                heap2.push({networkTemp.pods[i].rate, i});
            }
            // pod域
            while (heap2.size()) {
                auto [_, podId] = heap2.top();
                heap2.pop();
                auto &podTemp = networkTemp.pods[podId];
                for (int i = 1; i <= r; i ++ ) {
                    if (podTemp.racks[i].antiAffinity) continue;
                    heap3.push({podTemp.racks[i].rate, i});
                }
                int podIdReal = (networkId - 1) * p + podId;
                // rack域
                while (heap3.size()) {
                    auto [_, rackId] = heap3.top();
                    heap3.pop();
                    auto &rackTemp = podTemp.racks[rackId];
                    int rackIdReal = (podIdReal - 1) * r + rackId;

                    if (rackTemp.antiAffinity) continue;

                    // 判断机架下面的服务器能不能容纳所有vm

                    int canPutInNumber = 0;
                    for (int i = 1; i <= s; i ++ ) {
                        if (rackTemp.servers[i].antiAffinity || rackTemp.servers[i].rate > 0 || rackTemp.servers[i].vmNumber > 0) continue;
                        auto &server = rackTemp.servers[i];
                        // 根据虚拟机所需节点的数量进行选择
                        int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                        int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                        int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                        int node2RemainMemory = nodeMemory - server.node[2].useMemory;
                        // 统计所有的个数
                        int needCpu = vmCls[vmcls].cpu, needMemory = vmCls[vmcls].memory;

                        int acc = 0;
                        if (vmCls[vmcls].nodeNumber == 2) {

                            acc += min(1, min({node1RemainCpu / needCpu, node1RemainMemory / needMemory, node2RemainCpu / needCpu, node2RemainMemory / needMemory}));
                        } else if (vmCls[vmcls].nodeNumber == 1) {
                            acc += min(1, min({node1RemainCpu / needCpu, node1RemainMemory / needMemory}));
                        }

                        if (acc > 0) {
                            canPutInNumber += acc;
                        }
                    }



                    if (canPutInNumber >= curCreatVm.size()) {
                        // 放置在该机架下面
                        for (int i = 1; i <= s; i ++ ) {
                            if (rackTemp.servers[i].antiAffinity || rackTemp.servers[i].rate > 0 || rackTemp.servers[i].vmNumber > 0 ) continue;
                            // 找到对应的服务器防止节点
                            rackTemp.servers[i].antiAffinity = true;
                            // 找到虚拟机
                            auto &vmTemp = vm[curCreatVm[index]];
                            // 计算serverId
                            int serverIdReal = (rackIdReal - 1) * s + i;

                            updateNodeResource(rackTemp.servers[i], vmTemp, rackIdReal, serverIdReal, curCreatVm[index]);
                            // 更新机架和服务器防止vm的数量
                            rackTemp.vmNumber ++;
                            rackTemp.servers[i].vmNumber ++;
                            rackTemp.servers[i].antiAffinity = true;
                            ss ++;
                            index ++;
                            if (index == curCreatVm.size()) {
                                flag = true;
                                break;
                            }

                        }

                        rackTemp.affinity = true;
                        rackTemp.hasServerAnti = true;
                        g.rackAffinityId = rackIdReal;
                        flag = true;
                        break;

                    }
                    if (flag) break;
                }
                if (flag) break;
            }
            if (flag) break;
        }
    } else {
        // 放置在该机架下面
        vector<int> path = findRackPath(g.rackAffinityId);
        auto &rackTemp = network[path[1]].pods[path[2]].racks[path[3]];


        for (int vmId : curCreatVm) {
            bool flag = false;

            for (int i = 1; i <= s; i ++ ) {
                if (rackTemp.servers[i].antiAffinity || rackTemp.servers[i].rate > 0 || rackTemp.servers[i].vmNumber > 0 ) continue;
                auto &server = rackTemp.servers[i];
                auto &vmTemp = vm[vmId];
                int podIdReal = (path[1] - 1) * p + path[2];
                int rackIdReal = (podIdReal - 1) * r + path[3];
                int serverIdReal = (rackIdReal - 1) * s + i;
                // 根据虚拟机所需节点的数量进行选择
                int node1RemainCpu = nodeCpu - server.node[1].useCpu;
                int node1RemainMemory = nodeMemory - server.node[1].useMemory;
                int node2RemainCpu = nodeCpu - server.node[2].useCpu;
                int node2RemainMemory = nodeMemory - server.node[2].useMemory;

                bool flag1 = false;
                if (vmCls[vmTemp.vmClass].nodeNumber == 2) {
                    if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory &&
                    node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) {
                        flag1 = true;
                    }
                } else if (vmCls[vmTemp.vmClass].nodeNumber == 1) {
                    if (node1RemainCpu >= vmCls[vmTemp.vmClass].cpu && node1RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                    if (node2RemainCpu >= vmCls[vmTemp.vmClass].cpu && node2RemainMemory >= vmCls[vmTemp.vmClass].memory) flag1 = true;
                }

                if (flag1) {
                    updateNodeResource(server, vmTemp, rackIdReal, serverIdReal, vmId);
                    // 更新机架和服务器防止vm的数量
                    rackTemp.vmNumber ++;
                    server.vmNumber ++;
                    rackTemp.servers[i].antiAffinity = true;
                    ss ++;
                    flag = true;
                    break;
                }
                if (flag) break;
            }
        }
    }


    return ss == curCreatVm.size();    
}
bool solveConstrain8(int groupId) {
   // 创建优先队列保证组内分区
    auto &g = group[groupId];
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap1;
    priority_queue<PDI, vector<PDI>> heap2;
    priority_queue<PDI, vector<PDI>> heap3;
    priority_queue<PDI, vector<PDI>> heap4;


    // 第一次分配亲和网络
    if (g.networkAffinityId == 0) {
        // 根据占有率选取网络
        int ss = 0;
        for (int i = 1; i <= n; i ++ ) {
            heap1.push({network[i].rate, i});
        }

        auto [_, networkId] = heap1.top();
        g.networkAffinityId = networkId;
        network[networkId].hasRackAnti = true;
    }

    int ss = 0;

    for (int vmId : curCreatVm) {

        // 网络域
        bool flag = false;

        auto &networkTemp = network[g.networkAffinityId];
        for (int i = 1; i <= p; i ++ ) {
            heap2.push({networkTemp.pods[i].rate, i});
        }
        // pod域
        while (heap2.size()) {
            auto [_, podId] = heap2.top();
            heap2.pop();
            auto &podTemp = networkTemp.pods[podId];
            for (int i = 1; i <= r; i ++ ) {
                if (podTemp.racks[i].antiAffinity || podTemp.racks[i].rate > 0 || podTemp.racks[i].vmNumber > 0) continue;
                heap3.push({podTemp.racks[i].rate, i});
            }
            int podIdReal = (g.networkAffinityId - 1) * p + podId;
            // rack域
            while (heap3.size()) {
                auto [_, rackId] = heap3.top();

                heap3.pop();
                auto &rackTemp = podTemp.racks[rackId];
                int rackIdReal = (podIdReal - 1) * r + rackId;

                if (rackTemp.antiAffinity) continue;

                rackTemp.antiAffinity = true;

                for (int j = 1; j <= s; j ++ ) {
                    if (rackTemp.servers[j].antiAffinity || rackTemp.servers[j].rate > 0 || rackTemp.servers[j].vmNumber > 0) continue;

                    // 找到对应的服务器
                    rackTemp.servers[j].antiAffinity = true;
                    // 找到虚拟机
                    auto &vmTemp = vm[vmId];
                    // 计算serverId
                    int serverIdReal = (rackIdReal - 1) * s + j;
                    // // debug
                    // cout << "rackTemp.servers[j].antiAffinity: " << rackTemp.servers[j].antiAffinity << endl;
                    // cout << "netId : " << g.networkAffinityId << " " << "podId:" << podId << " " << "rackId:" << rackId << endl;
                    updateNodeResource(rackTemp.servers[j], vmTemp, rackIdReal, serverIdReal, vmId);
                    // 更新机架和服务器防止vm的数量
                    rackTemp.vmNumber ++;
                    rackTemp.servers[j].vmNumber ++;
                    ss ++;
                    flag = true;
                    break;

                }
                if (flag) break;

            }
            if (flag) break;
        }
    }

    return ss == curCreatVm.size();
}

bool solveConstrain9(int groupId) {
    // 创建优先队列保证组内分区
    auto &g = group[groupId];
    // 寻找pod的个数(题目一定保正整个pod分配一定是pod所能容纳服务器的倍数)
    int podNum = (int)curCreatVm.size() / (r * s);
    // 根据网络拓扑域占有率寻找pod数量
    priority_queue<PDI, vector<PDI>> heap;

    for (int i = 1; i <= n; i ++ ) {
        heap.push({network[i].rate, i});
    }
    int canUseNetwork = -1;
    while (heap.size()) {
        auto [_, id] = heap.top();
        heap.pop();

        int canUsePodNumber = 0;
        for (int i = 1; i <= p; i ++ ) {
            if (network[id].pods[i].rate == 0) {
                canUsePodNumber ++;
            }
        }
        if (canUsePodNumber >= podNum) {
            canUseNetwork = id;
            break;
        }
    }

    // 找不到可以分配的
    if (canUseNetwork == -1) return false;

    // 从左到右边寻找占有率为0的rate
    int index = 0;
    for (int i = 1; i <= p; i ++ ) {
        if (network[canUseNetwork].pods[i].rate == 0 && podNum > 0) {
            for (int j = 1; j <= r; j ++ ) {
                for (int j1 = 1; j1 <= s; j1 ++ ) {

                    auto &vM = vm[curCreatVm[index]];
                    auto &server = network[canUseNetwork].pods[i].racks[j].servers[j1];
                    auto vmclass = vM.vmClass;
                    int podId  = (canUseNetwork - 1) * p + i;
                    int rackId = (podId - 1) * r + j;
                    int serverId = (rackId - 1) * s + j1;
    

                    updateNodeResource(server, vM, rackId, serverId, curCreatVm[index]);
                    index ++;
                    // 更新机架和服务器上的vm数量
                    network[canUseNetwork].pods[i].racks[j].vmNumber ++;
                    server.vmNumber ++;
                }
            }

            podNum --;

        }
    }

    return true;




}


vector<int> findNetworkPath(int networkId) {
    vector<int> path = vector<int>(2, 0);
    path[1] = networkId;

    return path;
}
vector<int> findPodPath(int podId) {
    vector<int> path = vector<int>(3, 0);
    int d = podId;
    for (int i = 2; i >= 1; i -- ) {

        int mod = d % topologicalDomain[i];
        d = d / topologicalDomain[i];
        if (mod == 0) {
            path[i] = topologicalDomain[i];
        } else {
            path[i] = mod;
            d += 1;
        }
    }


    return path;
}
vector<int> findRackPath(int rackId) {
    vector<int> path = vector<int>(4, 0);
    int d = rackId;
    for (int i = 3; i >= 1; i -- ) {
        int mod = d % topologicalDomain[i];
        d = d / topologicalDomain[i];

        if (mod == 0) {
            path[i] = topologicalDomain[i];
        } else {
            path[i] = mod;
            d += 1;
        }
    }
    return path;
}
vector<int> findServerPath(int serverId) {
    vector<int> path = vector<int>(5, 0);
    int d = serverId;
    for (int i = 4; i >= 1; i -- ) {
        int mod = d % topologicalDomain[i];
        d = d / topologicalDomain[i];

        if (mod == 0) {
            path[i] = topologicalDomain[i];
        } else {
            path[i] = mod;
            d += 1;
        }
    }
    return path;
}

vector<int> findNodePath(int nodeId) {
    vector<int> path = vector<int> (6, 0);
    int d = nodeId;
    for (int i = 5; i >= 1; i -- ) {
        int mod = d % topologicalDomain[i];
        d = d / topologicalDomain[i];

        if (mod == 0) {
            path[i] = topologicalDomain[i];
        } else {
            path[i] = mod;
            d += 1;
        }
    }
    return path;
}


void updateRate(int rackId) {
    vector<int> path = findRackPath(rackId);
    auto &curRack = network[path[1]].pods[path[2]].racks[path[3]];
    // 更新机架的利用率
    curRack.rate = 0.0;
    for (int i = 1; i <= s; i ++ ) {
        curRack.rate += 1.0 / s * curRack.servers[i].rate;
    }
    // 更新pod的利用率
    auto &curPod = network[path[1]].pods[path[2]];
    curPod.rate = 0.0;
    for (int i = 1; i <= r; i ++ ) {
        curPod.rate += 1.0 / r * curPod.racks[i].rate;
    }
    // 更新网络域的利用率
    auto &curNetwork = network[path[1]];
    curNetwork.rate = 0.0;
    for (int i = 1; i <= p; i ++ ) {
        curNetwork.rate += 1.0 / p * curNetwork.pods[i].rate;
    }
}

void deleteVm(int vmId) {
    auto &vM = vm[vmId];
    // 释放虚拟机在组内占有的资源
    auto &g = group[vM.groupId];
    g.hasVmG.erase(vmId);
    // 说明该组内部有分区
    if (vM.partitionId != 0) {
        g.partHasVm[vM.partitionId].erase(vmId);
    }

    // 释放在每个拓扑域上占有的资源
    // 表示存放的虚拟机的路径（path[1] : network; path[2] : pod; path[3] : rack;
    //  path[4] : sever; path[5] : {0:两个节点都有; 1 : 第一个节点; 2 : 第二个节点}

    // 判断对应的拓扑域是否有反亲和性
    auto &rack = network[vM.path[1]].pods[vM.path[2]].racks[vM.path[3]];
    rack.antiAffinity = false;
    auto &server = network[vM.path[1]].pods[vM.path[2]].racks[vM.path[3]].servers[vM.path[4]];
    server.antiAffinity = false;

    rack.vmNumber --;
    server.vmNumber --;
    
    // 释放在节点上的资源
    for (int i = 1; i <= 2; i ++ ) {
        if (server.node[i].deployedVmsG.count(vmId)) {
            server.node[i].deployedVmsG.erase(vmId);
            server.node[i].useCpu -= vmCls[vM.vmClass].cpu;
            server.node[i].useMemory -= vmCls[vM.vmClass].memory;
            // 重新计算节点的占有率
            server.node[i].rate = 1.0 / 2 * (1.0 * server.node[i].useCpu / nodeCpu + 1.0 * server.node[i].useMemory / nodeMemory);

        }
    }
    // 计算服务器的占有率
    server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);
    // 更新rack以及以上的占有率
    int podId = (vM.path[1]  - 1) * p + vM.path[2];
    int rackId = (podId - 1) * s + vM.path[3];
    updateRate(rackId);


}

void updateNodeResource(Server &server, Vm &vM, int rackId, int serverId, int vmId) {
    // 根据虚拟机所需节点的数量进行选择
    auto vmclass = vM.vmClass;
    // 找到路径
    vector<int> path = findServerPath(serverId);

    int node1RemainCpu = nodeCpu - server.node[1].useCpu;
    int node1RemainMemory = nodeMemory - server.node[1].useMemory;
    int node2RemainCpu = nodeCpu - server.node[2].useCpu;
    int node2RemainMemory = nodeMemory - server.node[2].useMemory;

    if (vmCls[vmclass].nodeNumber == 2) {

            // 将虚拟机放置在该服务器上(更新)
            server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
            server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
            // 更新节点的利用率
            for (int j = 1; j <= 2; j ++ ) {
                server.node[j].rate = 1.0 / 2 * (1.0 * server.node[j].useCpu / nodeCpu + 1.0 * server.node[j].useMemory / nodeMemory);
            }
            // 更新服务器的利用率
            server.rate = 1.0 / 2 * (server.node[1].rate + server.node[2].rate);

            // 更新虚拟机信息
            vm[vmId].path[1] = path[1],  vm[vmId].path[2] = path[2],  vm[vmId].path[3] = path[3],  vm[vmId].path[4] = path[4],  vm[vmId].path[5] = 0;
            
            server.node[1].deployedVmsG.insert(vmId);
            server.node[2].deployedVmsG.insert(vmId);


    } else if (vmCls[vmclass].nodeNumber == 1){

        // 更新虚拟机信息
        vm[vmId].path[1] = path[1],  vm[vmId].path[2] = path[2],  vm[vmId].path[3] = path[3],  vm[vmId].path[4] = path[4];

        if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory && 
        node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
            // 计算两个节点的利用率
            double rate1 = 1.0 / 2 * ((1.0 * nodeCpu - node1RemainCpu) / nodeCpu + (1.0 *nodeMemory - node1RemainMemory) / nodeMemory);
            double rate2 = 1.0 / 2 * ((1.0 * nodeCpu - node2RemainCpu) / nodeCpu + (1.0 * nodeMemory - node2RemainMemory) / nodeMemory);
            if (rate1 >= rate2) {
                // 将虚拟机放置在该服务器上(更新)
                server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                // 更新节点的利用率
                server.node[1].rate = 1.0 / 2 * (1.0 * server.node[1].useCpu / nodeCpu + 1.0 * server.node[1].useMemory / nodeMemory);

                vm[vmId].path[5] = 1;
                server.node[1].deployedVmsG.insert(vmId);
            } else {
                // 将虚拟机放置在该服务器上(更新)
                server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                // 更新节点的利用率
                server.node[2].rate = 1.0 / 2 * (1.0 * server.node[2].useCpu / nodeCpu + 1.0 * server.node[2].useMemory / nodeMemory);     

                vm[vmId].path[5] = 2;  
                server.node[2].deployedVmsG.insert(vmId);                  
            }
        } else if (node1RemainCpu >= vmCls[vmclass].cpu && node1RemainMemory >= vmCls[vmclass].memory) {
                // 将虚拟机放置在该服务器上(更新)
                server.node[1].useCpu += vmCls[vmclass].cpu, server.node[1].useMemory += vmCls[vmclass].memory;
                // 更新节点的利用率
                server.node[1].rate = 1.0 / 2 * (1.0 * server.node[1].useCpu / nodeCpu + 1.0 * server.node[1].useMemory / nodeMemory);  

                vm[vmId].path[5] = 1;  
                server.node[1].deployedVmsG.insert(vmId);                 
        } else if (node2RemainCpu >= vmCls[vmclass].cpu && node2RemainMemory >= vmCls[vmclass].memory) {
                // 将虚拟机放置在该服务器上(更新)
                server.node[2].useCpu += vmCls[vmclass].cpu, server.node[2].useMemory += vmCls[vmclass].memory;
                // 更新节点的利用率
                server.node[2].rate = 1.0 / 2 * (1.0 * server.node[2].useCpu / nodeCpu + 1.0 * server.node[2].useMemory / nodeMemory);

                vm[vmId].path[5] = 2;   
                server.node[2].deployedVmsG.insert(vmId);      
                                                        
        }

        // 更新服务器的利用率
        server.rate = 1.0 / 2 * (1.0 * server.node[1].rate + 1.0 * server.node[2].rate);
        // 依次更新服务器以上利用率

    }

    updateRate(rackId);
}