#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)
#pragma GCC optimize "Ofast,omit-frame-pointer,inline,fast-math,unroll-all-loops,tree-loop-vectorize,tree-slp-vectorize"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <vector>

using namespace std;


#define LOCAL

typedef long long LL;
typedef pair<int, int> PII;
#define INF 100000000


const int N = 5010;
const int CENTER_LEFT_X = 20000, CENTER_RIGHT_X = 80000;
const int CENTER_LOW_Y = 20000, CENTER_UP_Y = 80000; 
// 随机数
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
     int next(int x) {return rand() % x;}
     int next(int a, int b) {return a + (rand() % (b - a));}
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 

static RNG rng;

PII a[N], b[N];  // max(0, a - b + 1);

int n;

//记录时间
const auto startTime = std::chrono::steady_clock::now();

inline int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

//节点类
struct node
{

    int x;
    int y;
    int centroid = 0;

     node(){}
     node(int _x, int _y, int _centroid) : x(_x), y(_y), centroid(_centroid){}
    


};


// 保存所有点
node nodeA[N], nodeB[N];
// 保存可能的中心点
node centerNode[30];

// 记录距离的函数（可以复用，求解初始聚类点)
int dist[N];

// 簇类相关数据结构
int clusterS_num[30];
unsigned int clusterS_sumX[30];
unsigned int clusterS_sumY [30];

int width, height;
//随机生成二维数据点
inline void inputNode()
{
    scanf("%d", &n);
    for (int i = 1; i <= n; ++ i) scanf("%d%d", &nodeA[i].x, &nodeA[i].y);
    for (int i = 1; i <= n; ++ i) scanf("%d%d", &nodeB[i].x, &nodeB[i].y);
}


inline bool check(node& point) {
    if (point.x >= CENTER_LEFT_X && point.x <= CENTER_RIGHT_X
    && point.y >= CENTER_LOW_Y && point.y <= CENTER_UP_Y) return true;

    return false;
}

void createCentroid(node points[], int centroidNum = 15)
{
    //随机选择一个初始点
    int nowNum = 0;
    // 从1-5000中随便选一个下标
    int firstNode = rng.next(1, 5001);

    while (!check(points[firstNode])) {
        firstNode = rng.next(1, 5001);
    }

    centerNode[0] = node(points[firstNode].x, points[firstNode].y, nowNum);
    ++ nowNum;
    while (nowNum < centroidNum)
    {
        memset(dist, 0, sizeof dist);
        for (int i = 1; i <= n; ++i)
        { //初始化距离
            dist[i] = INF;
        }
        //总距离
        double sumD = 0;
        for (int i = 1; i <= n; ++ i)
        {
            for (int c = 0; c < nowNum; ++ c)
            {
                double dis = sqrt(pow(1.0 * points[i].x - centerNode[c].x, 2) + pow(1.0 * points[i].y - centerNode[c].y, 2));
                if (dis < dist[i])
                    dist[i] = dis;
            }

            sumD += dist[i];
        }
        //选点距离
        // double r = (double)rand() / 2147483647 * sumD;
        // int i = 1;
        // for(;r > 0; ++ i){
        //     r -= dist[i];
        // }
        int index = -1, d = -1;
        // 选取dist最大值
        for (int i = 1; i <= 5000; ++ i) {
            if (dist[i] > d) {
                d = dist[i];
                index = i;
            }
        }
        centerNode[nowNum] = points[index];
        ++ nowNum;
    }

    // 创建好了中心点
}
//输出所有数据点
void printAll_node(node points[])
{
    for (int i = 1; i <= n; ++i)
    {
        printf("%d, (%d %d)\n", points[i].centroid, points[i].x, points[i].y);
    }
}
void printCenterNode(int len) {
    for (int i = 0; i < len; ++ i) {
        cerr << "i: " << " x:" << centerNode[i].x << " y:" << centerNode[i].y << endl;
    }
}
void kMeans_plus(int centroidNum, int len , node points[], int itr)
{

    for (int t = 1; t <= itr; ++ t)
    {
        double sum = 0;
        //对每个数据点进行分类
        for (int i = 1; i <= len; ++ i)
        {
            int dis = INF;
            int newDis;
            for (int x = 0; x < centroidNum; ++ x)
            {
                //遍历所有簇中心取最近的簇
                newDis = (int)sqrt(pow((1.0 * points[i].x - centerNode[x].x), 2) + pow((1.0 * points[i].y - centerNode[x].y), 2));
                if (newDis < dis)
                {
                    dis = newDis;
                    points[i].centroid = x;
                }
            }
        }

        //重新计算簇中心
        //每个簇的计数器
        for (int i = 0; i < centroidNum; ++i)
        {
            //初始化计数器
            clusterS_num[i] = clusterS_sumX[i] = clusterS_sumY[i] = 0;
        }

        for (int i = 1; i <= len; i ++ )
        {
            clusterS_num[points[i].centroid]++;
            clusterS_sumX[points[i].centroid] += points[i].x;
            clusterS_sumY[points[i].centroid] += points[i].y;
        }
        //重新计算簇中心
        for (int i = 0; i < centroidNum; ++i)
        {
            centerNode[i].x = clusterS_sumX[i] / clusterS_num[i];
            centerNode[i].y = clusterS_sumY[i] / clusterS_num[i];
        }
        for (int i = 1; i <= len; ++ i)
        {
            sum = sum + sqrt(pow((1.0 * points[i].x - centerNode[points[i].centroid].x), 2) + pow((1.0 * points[i].y - centerNode[points[i].centroid].y), 2));
        }
    }
}

int main()
{
    #ifdef LOCAL
    if (fopen("./input.txt", "r") != nullptr) {

        freopen(("./input.txt"), "r", stdin);
        freopen(("./output.txt"), "w", stdout);

        
    } else {
        cerr << "open file uncorrectly" << endl;
    }
    #endif
    inputNode();
    createCentroid(nodeA, 15);
    // printAll_node(nodeA);
    #ifdef LOCAL
    cerr << "before" << endl;
    printCenterNode(15);
    #endif
    // 输出中心点
    // node *allNode = randomNode(500, 1000);

    cerr << runtime() << endl;
    kMeans_plus(15, n, nodeA, 1000);

    #ifdef LOCAL
    cerr << "after" << endl;
    printCenterNode(15);
    cerr << runtime() << endl;
    #endif
    // printAll_node(500, allNode);

    width = height = 6000;
    

    auto generatePolyon = [&](auto&& generatePolyon, int len, vector<vector<PII>> &answer) {
        // 遍历所有中心点
        for (int i = 0; i < len; ++ i) {

            int centerX = centerNode[i].x, centerY = centerNode[i].y;
            vector<PII> tmp = {};
            int flagX = 1, flagY = 1;
            for (int x = 0; x < 2; x ++ ) {
                flagX = - flagX;
                flagY = - flagY;
                for (int y = 0; y < 2; ++ y) {
                    tmp.push_back(make_pair(centerX + flagX * width, centerY + flagY * height));
                    flagY = -flagY;
                }
            }

            answer.push_back(tmp);

        }

        
    };
    
    vector<vector<PII>> answer;
    generatePolyon(generatePolyon, 15, answer);


    // 输出答案

    int sz = 4;
    printf("%d\n", sz);
    for (int i = 5; i < 6; ++ i) {
        for (PII point : answer[i]) {
            printf("%d %d\n", point.first, point.second);
        }
    }

}
