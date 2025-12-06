

// 优化
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

using namespace std;

/***
 * 双链表实现O（1）下完成插入和删除某个淘汰页
 * 哈希表：记录当前缓冲区有无存放页，如有，则存放哪个租户的哪一页
***/

typedef pair<int, int> PII;
typedef pair<double, int> PDI;
#define x first
#define y second
const int N = 11, M = 1e5 + 10, dieTime = 1, number = 2, CACHEEXTRA = 20;
const double Y = 0.25, U = 3;

/***
 * 功能: 双向链表中的结点
 * 参数定义:
 * tenantId : 租户编号
***/
struct Node {
    int size; // 表示当前租户已分配缓冲区的个数(空闲或已占有) 
    int tenantId; // 租户编号
    int page; // 页号
    int cacheId; // 缓冲区编号
    int time;
    
    // 双向链表必要 (租户对应的缓冲区)
    Node *prev, *next;

    // 类的构造函数
    Node (int tenantId = 0, int page = 0, int cacheId = 0, int size = 0, int time = 0) : 
    tenantId(tenantId), page(page), cacheId(cacheId), size(size), time(time) {}


};

int n, capacity, k; // n : 租户数量; capacity : 总缓冲区大小; k : 单位时间总共可以分配的缓冲区数量
unordered_map<int, PII> cacheTotenant; // 参数含义： key : 缓冲区下标; pair.first : 租户编号; pair.second : 租户所要访问的页号
Node* tenantTocache[N][M]; // 参数含义： pair.first : 租户编号; pair.second : 租户访问的页号; value : 结点信息
unordered_map<int, int> tenantPriority; // 表示租户的优先级 （后面可能考虑用map替换）
unordered_map<int, int> tenantMinCache; // 租户能接受的最小缓冲区大小
unordered_map<int, int> tenantExpCache; // 租户所期待的缓冲区大小
PII sortPriority[N];
unordered_map<int, int> tenantCueCache; // 租户当前占有缓冲区大小

unordered_map<int, Node *> tenantPreCache; // 租户下预分配缓冲区
unordered_map<int, pair<Node *, Node *>> tenantCurCache; // 租户下冷热缓冲区(头结点)
bool pageInHot[N][M]; // 0 : 为冷区; 1 : 为热区
int tenantHotMaxSize[N]; // 租户热缓冲区的最大数量
int tenantPageHit[N][M];
vector<bool> cacheSt; // 判断当前缓存区有没有被使用

int lastTenantNeed[N];


Node *dummy; // 头节点（Lru)

vector<int> systemCache; // 系统缓冲区

// 基线内容
unordered_map<int, Node*> tenantBaselineCache;
Node *baselineToCache[N][M];
PII baselineTwoCacheSize[N]; // 保存租户在基线下的缓冲区实际使用大小和基线大小
// 统计租户在基线和实际的未命中情况
double tenantInBaseline[N], tenantInActual[N], tenantInRealActual[N];
// 租户的成本
PDI cost[N];


int lastT = 0;


void input();
void init();
void removeNode (Node *x);
void pushFront(Node *dummy, Node *x);
Node* getNode(int tenantId, int page, int curTime);
int get(int tenantId, int page);
void solve();
void dealExpCache(int lastT);
bool cmp(PII &p1, PII &p2);
bool cmp1(PDI &p1, PDI &p2);
// 基线函数
void initBaseline();
void baselineOperation(int tenantId, int page);
Node* getNodeInBaseline(int tenantId, int page);
void getInBaseline(int tenantId, int page);

// 计算租户的成本
void calculateCost();
Node* removeMin(int tenantId);

// 主函数
int main() {

    input();
    init();
    initBaseline();
    solve();

    return 0;
}


Node* getNode(int tenantId, int page, int curTime) {
    tenantPageHit[tenantId][page] ++;
    // 如不存在，则表示缺页
    if (!tenantTocache[tenantId][page]) {
        return nullptr;
    }

    auto node = tenantTocache[tenantId][page];
    // 保持时间的一致性
    node->time = curTime;
    
    // 得到租户冷热缓冲区
    auto [coldHead, hotHead] = tenantCurCache[tenantId];
    // 先判断有无在热区
    if (pageInHot[tenantId][page]) {
        // 从热区删除，并插入热区头部
        removeNode(node);
        pushFront(hotHead, node);

    } else {
        //从冷区中删除，插入到热区头部
        if (hotHead->size >= tenantHotMaxSize[tenantId]) {
            // 需要将最后一个元素移动到冷区头部
            auto backNode = hotHead->prev;
            auto prevNode = backNode->prev;
            vector<PII> mayBeDieHotCache;
            // 根据当前时间进行选择lru或者lfu
            // 得到热区元素按照访问次数进行排序
            while (backNode != hotHead) {
                if (lastT - backNode->time > 40) {
                    mayBeDieHotCache.push_back({tenantPageHit[backNode->tenantId][backNode->page], backNode->page});
                    backNode = prevNode;
                    prevNode = backNode->prev;
                } else {
                    break;
                }
            }

            sort(mayBeDieHotCache.begin(), mayBeDieHotCache.end(), [&](const PII &p1, const PII &p2) {
                return p1.first < p2.first;
            });
            if (mayBeDieHotCache.size()) {
                backNode = tenantTocache[tenantId][mayBeDieHotCache[0].second];
            } else {
                backNode = hotHead->prev;
            }
            // 根据频率寻找
            removeNode(backNode);
            pushFront(coldHead, backNode);
            pageInHot[backNode->tenantId][backNode->page] = false;
            hotHead->size --;
            coldHead->size ++;
            // 将当前结点移动到热区头部
            removeNode(node);
            pushFront(hotHead, node);
            coldHead->size --;
            hotHead->size ++;
            // 当前页在热区
            pageInHot[tenantId][page] = true;

        } else {
            // 热区数量未满，直接加入即可
            removeNode(node);
            pushFront(hotHead, node);
            coldHead->size --;
            hotHead->size ++;
            pageInHot[tenantId][page] = true;
        }
    }


    return node;
}

int get(int tenantId, int page, int curTime) {
    
    auto node = getNode(tenantId, page, curTime);
    if (node != nullptr) {
        return node->cacheId;
    }

    lastTenantNeed[tenantId] ++;
    // 选择淘汰页面
    Node *dieNode = removeMin(tenantId);
    // 实际情况下未命中
    tenantInActual[tenantId] = tenantInActual[tenantId] + 1.0;

    // 得到预分配缓冲区
    auto curPre = tenantPreCache[tenantId];
    // 得到冷热缓冲区
    auto [curCold, curHot] = tenantCurCache[tenantId];

    // 预分配的缓冲区有无空缺
    if (curPre->size > 0) {
        // 得到尾结点
        auto backNode = curPre->prev;
        tenantInRealActual[tenantId] = tenantInRealActual[tenantId] + 1.0 + Y * max(0.0, U - (lastT - backNode->time));
        // 从空闲缓冲区中移除
        removeNode(backNode);
        curPre->size -- ;
        // 完善结点内容, 加入到已占有缓冲区中
        backNode->tenantId = tenantId;
        backNode->page = page;
        backNode->time = curTime;

        pushFront(curCold, backNode);


        //改变已占有缓存区的大小
        curCold->size ++;
        // 改变对应的一些状态
        cacheTotenant[backNode->cacheId] = {tenantId, page};
        tenantTocache[tenantId][page] = backNode;

        // 返回缓冲区编号
        return backNode->cacheId;

    }
    tenantInRealActual[tenantId] = tenantInRealActual[tenantId] + 1.75;


    // 如果当前用户已分配的缓冲区没有空闲, 则向系统剩余缓冲区索要
    if (systemCache.size()) {
        // 向系统申请新缓冲区
        int idx = systemCache.back();
        systemCache.pop_back();

        node = new Node(tenantId, page, idx, 0, curTime);
        pushFront(curCold, node);
        // 改变占有缓冲区大小
        curCold->size ++;
        // 改变一些状态
        cacheTotenant[node->cacheId] = {tenantId, page};
        tenantTocache[tenantId][page] = node;

        cacheSt[node->cacheId] = true;

        return node->cacheId;
    }

    // 根据LRU淘汰已经选择页面

    // 删除cacheTotenant中的信息
    cacheTotenant.erase(dieNode->cacheId);
    // 删除tenantTocache中的信息
    tenantTocache[dieNode->tenantId][dieNode->page] = nullptr;
    // 维护信息的一致性
    cacheTotenant[dieNode->cacheId] = {tenantId, page};
    tenantTocache[tenantId][page] = node = new Node(tenantId, page, dieNode->cacheId, dieNode->size, curTime);

    // 移除旧结点
    removeNode(dieNode);


    // 将新结点插入链表头部
    pushFront(curCold, node);
    // 保持一致性很重要
    curCold->size ++;

    // 得到新结点两种缓冲区头部
    auto [swapCold, swapHot] = tenantCurCache[dieNode->tenantId];

    if (pageInHot[dieNode->tenantId][dieNode->page]) {
        // 在热区
        pageInHot[dieNode->tenantId][dieNode->page] = false;
        swapHot->size --;
    } else {
        // 在冷区
        swapCold->size --;
    }


    delete dieNode;
    return node->cacheId;

}
void input() {
    scanf("%d%d%d", &n, &capacity, &k);
    // 输入租户优先级
    for (int i = 1; i <= n; i ++ ) {
        int x;
        scanf("%d", &x);
        tenantPriority[i] = x;
        sortPriority[i] = {x, i};
    }
    // 对输入完成后的优先级数组进行排序
    sort(sortPriority + 1, sortPriority + 1 + n, cmp);
    // 输入租户最小分配缓冲区和期待分配缓冲区大小
    for (int i = 1; i <= n; i ++ ) {
        int minC, expC;
        scanf("%d%d", &minC, &expC);
        tenantMinCache[i] = minC;
        tenantExpCache[i] = expC;
        tenantHotMaxSize[i] =  1.0 * (expC + minC) / (1.66);

    }
}


void init() {
    // 初始化为命中数量
    memset(tenantInActual, 0, sizeof tenantInActual);
    // 初始化租户需求等级列表
    memset(lastTenantNeed, 0, sizeof lastTenantNeed);
    // 初始化租户冷热区映射标记
    memset(pageInHot, 0, sizeof pageInHot);

    cacheSt = vector<bool> (capacity + 1, 0);
    // 初始化系统缓冲区
    dummy = new Node(0, 0, 0, 0, 0);
    dummy->prev = dummy;
    dummy->next = dummy;

    // 初始化tenantTocache
    for (int i = 1; i <= capacity; i ++ ) {
        systemCache.push_back(i);
    }
    // 初始化租户预分配缓冲区
    for (int i = 1; i <= n; i ++ ) {
        Node* tFree = new Node(0, 0, 0, 0, 0); // 初始化分配时以每个租户最小需求缓冲区分配
        tFree->prev = tFree;
        tFree->next = tFree;
        tenantPreCache[i] = tFree;

    }
    // 初始化租户冷热缓冲区大小
    for (int i = 1; i <= n; i ++ ) {
        Node* cold = new Node(0, 0, 0, 0, 0); // 初始化分配时以每个租户最小需求缓冲区分配
        cold->prev = cold;
        cold->next = cold;

        Node* hot = new Node(0, 0, 0, 0, 0);
        hot->prev = hot;
        hot->next = hot;

        tenantCurCache[i] = {cold, hot};
    }


}
void solve() {
    // 由于在初始化的时候已经按照顺序给出预分配缓冲区 （这里未考虑到单位时间内分配缓冲区的数量限制）

    // 根据每个用户最小需求量分配
    int minS = 0;
    for (int i = 1; i <= n; i ++ ) {
        minS += tenantMinCache[i];
    }
    printf("%d ", capacity);
    for (int i = 1; i <= n; i ++ ) {
        auto curPre = tenantPreCache[i];
        for (int j = 1; j <= tenantMinCache[i]; j ++ ) {
            int cache = systemCache.back();
            systemCache.pop_back();

            Node *node = new Node(i, 0, cache, 0, -5);
            pushFront(curPre, node);
            curPre->size ++;

            printf("%d %d ", cache, i);
        }
    }
     // 剩余未预分配的缓冲区
    int cnt = capacity - minS;
    if (cnt) {
        // 分配个数
        int d1 = cnt / n, d2 = cnt % n;
        // 平均分配
        for (int i = 1; i <= n; i ++ ) {
            auto curPre = tenantPreCache[i];
            for (int j = 1; j <= d1; j ++) {
                int cache = systemCache.back();
                systemCache.pop_back();
                Node *node = new Node(i, 0, cache, 0, -5);
                // 缓冲区标识为true
                cacheSt[cache] = true;
                pushFront(curPre, node);
                curPre->size ++;
                printf("%d %d ", cache, i);
            }
        }
        // 把余数部分依次分配
        for (int i = 1, j = 1; i <= n && j <= d2; i ++, j ++ ) {
            auto curPre = tenantPreCache[i];
            int cache = systemCache.back();
            systemCache.pop_back();
            Node *node = new Node(i, 0, cache, 0, -5);
            // 缓冲区标识为true
            cacheSt[cache] = true;
            pushFront(curPre, node);
            curPre->size ++;
            printf("%d %d ", cache, i);
        }
    }
    printf("\n");
    fflush(stdout);


    int t;

    while (scanf("%d", &t), t != -1) {
        if (t) {
            int tenantId, page;
            scanf("%d%d", &tenantId, &page);
            printf("%d\n", get(tenantId, page, t));
            lastT = t;
            // 维护基线操作
            baselineOperation(tenantId, page);
            // 根据当前时间段统计上一段时间的访问次数
        } else {
            dealExpCache(lastT);
            memset(lastTenantNeed, 0, sizeof lastTenantNeed);
        }
        fflush(stdout);
        
    }
}
// 关键函数
void dealExpCache(int lastT) {
    // 对长时间未访问页面进行替换
    priority_queue<PII, vector<PII>> heap; // 根据上次租户访问请求来淘汰页面
    int totalNeed = 0;
    for (int i = 1; i <= n; i ++ ) {

        auto curPre = tenantPreCache[i];

        heap.push({tenantPriority[i], i});
        totalNeed += max(0, lastTenantNeed[i] - curPre->size);
    }
    vector<PII> saveInfo;
    int cnt = 0;
    pair<double,int> accordingToHit[N];
    for (int i = 1; i <= n; i ++ ) {
        double cost = max(0.0, 1.0 * tenantInActual[i] / tenantInBaseline[i] - 1);
        if (cost > 1) cost = cost * cost;
        accordingToHit[i] = {cost * tenantPriority[i],  i};
    }
    // 成本从小到大排序
    sort(accordingToHit + 1, accordingToHit + 1 + n);
    //判断系统缓冲区有无

    if (systemCache.size()) {
        for (int i = n; i >= 1; i -- ) {
            auto [priority, id] = accordingToHit[i];

            auto account = lastTenantNeed[id];
            auto curPre = tenantPreCache[id];
            auto [curCold, curHot] = tenantCurCache[id];
            if (curPre->size + curCold->size + curHot->size >= tenantExpCache[id] + CACHEEXTRA) continue;
            if (curPre->size >= 2 * account) continue;
            if (account == 0) continue;

            while (cnt < k && systemCache.size() && curPre->size < lastTenantNeed[id]
            && curPre->size + curCold->size + curHot->size < tenantExpCache[id] + CACHEEXTRA) {
                int cacheId = systemCache.back();
                systemCache.pop_back();
                Node *node = new Node(id, 0, cacheId, 0, lastT);
                // 插入结点
                pushFront(curPre, node);
                // 保存预分配信息
                saveInfo.push_back({node->cacheId, id});
                cnt ++;
                curPre->size ++;
            }
        }
    }
    vector<int> mostCanDieCache;
    // 判断遍历成本为0的租户淘汰他们的页面
    for (int i = 1; i <= n; i ++ ) {
        auto [priority, id] = accordingToHit[i];
        auto curPre = tenantPreCache[id];
        auto [curCold, curHot] = tenantCurCache[id];
        auto backNode = curCold->prev;
        auto prevNode = backNode->prev;
        if (curCold->size > tenantExpCache[id] * 0.3) {
        // 移除冷区元素
            while (backNode != curCold && curCold->size + curHot->size  >= tenantMinCache[id]
            && cnt < k  && mostCanDieCache.size() < totalNeed ) {
                if (lastT - backNode->time > 5) {
                    // 删除cacheTotenant中的信息
                    cacheTotenant.erase(backNode->cacheId);
                    // 删除tenantTocache中的信息
                    tenantTocache[backNode->tenantId][backNode->page] = nullptr;
                    // 移除结点
                    removeNode(backNode);
                    // 更新结点信息
                    backNode->page = 0;
                    backNode->time = lastT;

                    mostCanDieCache.push_back(backNode->cacheId);
                    cnt ++;
                    curCold->size --;
                }
                backNode = prevNode;
                prevNode = backNode->prev;
            }
        } else {
            break;
        }
    }
    // 分配给每个租户
    for (int i = n; i >= 1; i -- ) {
        auto [priority, id] = accordingToHit[i];

        auto account = lastTenantNeed[id];
        auto curPre = tenantPreCache[id];
        auto [curCold, curHot] = tenantCurCache[id];
        if (curPre->size >= account) continue;

        // 移除冷区元素
        while ( mostCanDieCache.size() > 0 && curPre->size < lastTenantNeed[id]) {
                int cacheId = mostCanDieCache.back();
                mostCanDieCache.pop_back();
                Node *node = new Node(id, 0, cacheId, 0, lastT);
                // 插入结点
                pushFront(curPre, node);
                // 保存预分配信息
                saveInfo.push_back({node->cacheId, id});
                curPre->size ++;
        }
    }    
    // 仍然可以优化，对每个租户依次遍历淘汰
    for (int i = n; i >= 1; i -- ) {
        auto [priority, id] = accordingToHit[i];

        auto account = lastTenantNeed[id];
        auto curPre = tenantPreCache[id];
        auto [curCold, curHot] = tenantCurCache[id];
        if (curPre->size >= account) continue;
        if (account == 0) continue;

        auto backNode = curCold->prev;
        auto prevNode = backNode->prev;
        // 移除冷区元素
        while (backNode != curCold && curCold->size + curHot->size  >= tenantMinCache[id] 
         && cnt < k && account > 0 && curPre->size < lastTenantNeed[id] * 3 ) {
            if (lastT - backNode->time > 0) {
                // 删除cacheTotenant中的信息
                cacheTotenant.erase(backNode->cacheId);
                // 删除tenantTocache中的信息
                tenantTocache[backNode->tenantId][backNode->page] = nullptr;
                // 移除结点
                removeNode(backNode);
                // 更新结点信息
                backNode->page = 0;
                backNode->time = lastT;
                // 插入结点
                pushFront(curPre, backNode);
                // 保存预分配信息
                saveInfo.push_back({backNode->cacheId, id});
                cnt ++;
                curCold->size --;
                curPre->size ++;
                account -- ;
            }
            backNode = prevNode;
            prevNode = backNode->prev;
        }
    }
    // for (int i = 1; i <= n; i ++ ) {
    //     heap.push({tenantPriority[i], i});
    // }
    
    // 释放热区元素
    for (int i = n; i >= 1; i -- ) {
        auto [priority, id] = accordingToHit[i];
        
        auto account = lastTenantNeed[id];
        auto curPre = tenantPreCache[id];
        auto [curCold, curHot] = tenantCurCache[id];
        if (curPre->size >= account) continue;
        if (account == 0) continue;

        auto backNode = curHot->prev;
        auto prevNode = backNode->prev;
        vector<PII> mayBeDieHotCache;
        // 根据当前时间进行选择lru或者lfu
        // 得到热区元素按照访问次数进行排序
        while (backNode != curHot) {
            if (lastT - backNode->time > 1) {
                mayBeDieHotCache.push_back({tenantPageHit[backNode->tenantId][backNode->page], backNode->page});
                backNode = prevNode;
                prevNode = backNode->prev;
            } else {
                break;
            }
        }
        // 进行排序
        // 根据当前时间进行选择lru或者lfu
        if (lastT > 10) {
            sort(mayBeDieHotCache.begin(), mayBeDieHotCache.end(), [&](const PII &p1, const PII &p2) {
                return p1.first < p2.first;
            });
        } else {
            sort(mayBeDieHotCache.begin(), mayBeDieHotCache.end(), [&](const PII &p1, const PII &p2) {
                return tenantTocache[id][p1.y]->time < tenantTocache[id][p2.y]->time;
            });
        }
 

        for (int j = 0; j < mayBeDieHotCache.size() && cnt < k && curPre->size < lastTenantNeed[id] * 3 && curCold->size + curHot->size  >= tenantMinCache[id] ; j ++ ) {
            auto node = tenantTocache[id][mayBeDieHotCache[j].second];
            // 删除cacheTotenant中的信息
            cacheTotenant.erase( node->cacheId);
            // 删除tenantTocache中的信息
            tenantTocache[ node->tenantId][ node->page] = nullptr;
            pageInHot[ node->tenantId][ node->page] = 0;

            // 移除结点
            removeNode( node);
             // 更新结点信息
            node->page = 0;
            node->time = lastT;
            // 插入结点
            pushFront(curPre,  node);
            // 保存预分配信息
            saveInfo.push_back({ node->cacheId, id});
            cnt ++;
            curHot->size --;
            curPre->size ++;

        }


    }
   

    if (saveInfo.size()) {
        printf("%d ", saveInfo.size());
        for (int i = 0; i < saveInfo.size(); i ++ ) {
            printf("%d %d ", saveInfo[i].x, saveInfo[i].y);
        }
        printf("\n");
    } else {
        puts("0");
    }
}
/***
 * 基线部分
***/

void initBaseline() {
    // 初始化为命中数量
    memset(tenantInBaseline, 0, sizeof tenantInBaseline);
    // 初始化基线缓存区数量
    for (int i = 1; i <= n; i ++ ) {
        baselineTwoCacheSize[i] = {0, tenantExpCache[i]};
    }
    // 初始化 tenantBaselineCache（每个头结点进行初始化）
    for (int i = 1; i <= n; i ++ ) {
        Node* head = new Node(0, 0, 0, 0, 0); // 初始化分配时以每个租户最小需求缓冲区分配
        head->prev = head;
        head->next = head;

        tenantBaselineCache[i] = head;
    }
}

void baselineOperation(int tenantId, int page) {
    Node *head = tenantBaselineCache[tenantId];

    getInBaseline(tenantId, page);

}

Node* getNodeInBaseline(int tenantId, int page) {
    // 如不存在，则表示缺页
    if (!baselineToCache[tenantId][page]) {
        return nullptr;
    }

    auto node = baselineToCache[tenantId][page];


    // 在租户基线缓冲区里的操作
    removeNode(node);
    pushFront(tenantBaselineCache[tenantId], node);


    return node;
}

void getInBaseline(int tenantId, int page) {
    auto node = getNodeInBaseline(tenantId, page);
    if (node != nullptr) {
        return;
    }
    // 基线下未命中
    tenantInBaseline[tenantId] += 1.0;
    // 得到基线租户的缓冲区头部
    Node *head = tenantBaselineCache[tenantId];

    int &curNum = baselineTwoCacheSize[tenantId].x, &curMax = baselineTwoCacheSize[tenantId].y;
    // 如果当前缓冲区未满
    if (curNum < curMax) {
        // 申请缓冲区空间加入
        node = new Node(tenantId, page, 0, 0, 0);

        pushFront(head, node);
        baselineToCache[tenantId][page] = node;
        curNum ++;

        return;
    }
    // 如果当前缓冲区已满
    // 得到尾部数据进行淘汰
    Node *backNode = head->prev;
    // 保持数据一致性
    baselineToCache[backNode->tenantId][backNode->page] = nullptr;
    baselineToCache[tenantId][page] = node = new Node(tenantId, page, 0, 0, 0);

    // 删除尾部数据
    removeNode(backNode);

    // 插入头部数据
    pushFront(head, node);

    // 删除移除数据
    delete backNode;
}
// 得到淘汰结点（不移除）
Node* removeMin(int curTenant) {

    
    auto[curC, curH] = tenantCurCache[curTenant];
    if (curC->size > tenantExpCache[curTenant] * 0.3) {
        return curC->prev;
    }
    if (curH->size == tenantHotMaxSize[curTenant]) {
        return curH->prev;
    }
    if (curC->size + curH->size > tenantExpCache[curTenant] + 26) {
        if (curC->size > 0) return curC->prev;
        return curH->prev;
    }
    calculateCost();

    sort(cost + 1, cost + 1 + n);

    int tenantId = 0;
    int flag = 0;

    // 先寻找热区数据（成本为0）的租户
    for (int i = 1; i <= n; i ++ ) {
        tenantId = cost[i].second;
        // 得到当前租户的预分配缓冲区
        auto curPre = tenantPreCache[tenantId];
        // 得到冷热缓冲区
        auto [curCold, curHot] = tenantCurCache[tenantId];
        // 得到当前租户缓冲区的总数量
        int s = curCold->size + curHot->size;

        if (s > tenantMinCache[tenantId] && tenantInRealActual[tenantId] < tenantInBaseline[tenantId] 
        && curHot->size == tenantHotMaxSize[tenantId]) {
            return curHot->prev;
        }
    }
    // 先寻找冷区数据(成本为0)的租户
    for (int i = 1; i <= n; i ++ ) {
        tenantId = cost[i].second;
        // 得到当前租户的预分配缓冲区
        auto curPre = tenantPreCache[tenantId];
        // 得到冷热缓冲区
        auto [curCold, curHot] = tenantCurCache[tenantId];
        // 得到当前租户缓冲区的总数量
        int s = curCold->size + curHot->size;
        if (tenantInRealActual[tenantId] + 500 < tenantInBaseline[tenantId]) {
            if (s + curPre->size > tenantMinCache[tenantId] && curCold->size > 0) {
                return curCold->prev;
            }
        }
    }
    // 先寻找冷区数据
    for (int i = 1; i <= n; i ++ ) {
        tenantId = cost[i].second;
        // 得到当前租户的预分配缓冲区
        auto curPre = tenantPreCache[tenantId];
        // 得到冷热缓冲区
        auto [curCold, curHot] = tenantCurCache[tenantId];
        // 得到当前租户缓冲区的总数量
        int s = curCold->size + curHot->size;
        if (tenantInRealActual[tenantId] + 500 < tenantInBaseline[tenantId]) {
            if (s + curPre->size > tenantMinCache[tenantId] && curCold->size > 0) {
                return curCold->prev;
            }
        }
        if ((s > tenantMinCache[tenantId] || tenantId == curTenant) && curCold->size > 0) {
            flag = 1;
            break;

        }
    }
    if (!flag) {
        for(int i = 1; i <= n; i ++){
            tenantId = cost[i].second;
            // 得到当前租户的预分配缓冲区
            auto curPre = tenantPreCache[tenantId];
            // 得到冷热缓冲区
            auto [curCold, curHot] = tenantCurCache[tenantId];
            // 得到当前租户缓冲区的总数量
            int s = curCold->size + curHot->size;
            if((s > tenantMinCache[tenantId] || tenantId == curTenant) && curHot->size > 0) {
                flag = 2;
                break;
            }
        }
    }

    if (!flag) {
        tenantId = curTenant;
        auto [curCold, curHot] = tenantCurCache[tenantId];

        if (curCold->size > 0) return curCold->prev;
        else
            return curHot->prev;
    } 



    auto [curCold, curHot] = tenantCurCache[tenantId];
    // 得到淘汰结点
    if (flag == 1) return curCold->prev;
    return curHot->prev;


}
void calculateCost() {
    for (int i = 1; i <= n; i ++ ) {
        cost[i] = {0.0, i};
    }
    for(int i = 1;i <= n; i++){
        double cost_mid = (double) max(0.0, 1.0 *  tenantInActual[i] / (double)tenantInBaseline[i] - 1.0);
        if (cost_mid < 1) {
            cost[i].x = cost_mid * (1.0  * tenantPriority[i]);
        } else {
            cost[i].x = cost_mid * cost_mid * cost_mid *  (1.0  * tenantPriority[i]);
        }

    }
  
}


// 排序规律
bool cmp(PII &p1, PII &p2) {
    return p1.x < p2.x;
}
bool cmp1(PDI &p1, PDI &p2) {
    if (p1.x != p2.x) {
        return p1.x < p2.x;
    }
    return tenantPriority[p1.y] < tenantPriority[p2.y];
}
// 移除某个节点
void removeNode (Node *x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
}

// 在头部插入某个节点
void pushFront(Node *dummy, Node *x) {
    x->prev = dummy;
    x->next = x->prev->next;
    x->prev->next = x;
    x->next->prev = x;
}

