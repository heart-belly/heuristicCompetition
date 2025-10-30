#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)
#pragma GCC optimize "Ofast,omit-frame-pointer,inline,fast-math,unroll-all-loops,tree-loop-vectorize,tree-slp-vectorize"
#include "bits/stdc++.h"

using namespace std;

const int B_MAX = 4;
const int W_MAX = 207;
const int D_MAX = 960;
const int N_MAX = 10000;
const int ANCHOR_MAX = 32;
const int ALLOW_INTERVAL_SAME = 4;
const int ALLOW_INTERVAL_DIFF = 0;
const int MAX_IO_NUM = 10000 + 10;
typedef pair<double, int> PDI;
typedef pair<int, int> PII;
#define x first
#define y second

const double TOGGLE_WRAP = 2.6667;
const double TOGGLE_BAND = 13.333;
const int MAX_RUN_TIME = 4900;

#ifndef LOCAL
#define LOCAL
#endif

// #define DEBUG

const auto startTime = std::chrono::steady_clock::now();

 int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

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

int n;
/*
* 调试区
*/

vector<vector<double>> wrap;
vector<PDI> anchorInWrap[W_MAX + 5][ANCHOR_MAX + 5];
PII anchorInWrapSE[W_MAX + 5][ANCHOR_MAX + 5]; // 保存以wrap方向最早开始点坐标以及最迟结束点坐标
bool anchorInWrapSt[W_MAX + 5][ANCHOR_MAX + 5];

int bandHasAnchor[B_MAX][2][ANCHOR_MAX + 5]; // 0 : 正向; 1 : 反向

// 以wrap和anchor为锚点作唯一标识
// uint16_t wrapLinkAnchor[W_MAX * D_MAX];
vector<uint16_t> wrapLinkAnchor;
vector<int> result;

// 记录当前最左以及最右侧可走锚点
int forwardAnchor[2] = {0, 31};
int backwardAnchor[2] = {1, 32};


int special[5] = {7, 47, 47, 74, 77};


struct TapePos
{
    int wrap;
    double distance;
    int index;

    TapePos(int wrap, double distance, int index)
    {
        this->wrap = wrap;
        this->distance = distance;
        this->index = index;
    }

    
    TapePos()
    {}


    bool operator <(const TapePos& other) const
    {
        return distance < other.distance;
    }

    string toString() {
        return  "wrap : " + to_string(wrap) + " distance : " + to_string(distance) + "index: " + to_string(index);
    }
};

vector<TapePos> points;

void init() {
    wrap.resize(W_MAX + 10);
}
inline int getBand(int wrapId) {
    if (wrapId <= 51) return 0;
    else if (wrapId <= 103) return 1;
    else if (wrapId <= 155) return 2;

    return 3;

}
inline PII getAnchor(double pos) {
    int left =  (int)(pos / 30); 

    return {left, left + 1};
}

inline double getPointTime(const TapePos &p1, const TapePos &p2) {
    int b1 = getBand(p1.wrap), b2 = getBand(p2.wrap);
    int w1 = p1.wrap, w2 = p2.wrap;

    double costTime = 0.0;
    if (b1 != b2) costTime += 4;
    if (w1 != w2) costTime += 1;

    // 得到wrap方向
    int direct1 = (w1 % 2 == 0 ? 1 : -1);
    int direct2 = (w2 % 2 == 0 ? 1 : -1);

    // 同一wrap且同方向
    if (w1 == w2 && direct1 * (p2.distance - p1.distance) > 0) {
        // 同方向
        return abs(p2.distance - p1.distance) * 0.5;
    }

    // 得到里点b最近的锚点
    PII anchorPII = getAnchor(p2.distance);
    int anchorTo2 = (direct2 == 1 ? anchorPII.x : anchorPII.y);
    double dis2 = anchorTo2 * 30;
    // double p1toAncDis = abs(p1.distance - dis2) * 0.125;
    costTime += abs(p1.distance - dis2) * 0.125;
    // double AntoP2Dis = abs(dis2 - p2.distance) * 0.5;
    costTime += abs(dis2 - p2.distance) * 0.5;
    // 判断有无转向
    if ((p1.distance - dis2) * (p2.distance - dis2) < 0 && direct1 == direct2 && (p2.distance - p1.distance) * direct1 > 0) {
    } else {
        costTime += 2.5;
    }

    return costTime;

}
inline double getPointTime1(const TapePos &p1, const TapePos &p2) {
    int b1 = getBand(p1.wrap), b2 = getBand(p2.wrap);
    int w1 = p1.wrap, w2 = p2.wrap;

    double costTime = 0.0;
    if (b1 != b2) costTime += 3;
    if (w1 != w2) costTime += 1;

    // 得到wrap方向
    int direct1 = (w1 % 2 == 0 ? 1 : -1);
    int direct2 = (w2 % 2 == 0 ? 1 : -1);

    // 同一wrap且同方向
    if (w1 == w2 && direct1 * (p2.distance - p1.distance) > 0) {
        // 同方向
        return abs(p2.distance - p1.distance) * 0.5;
    }

    // 得到里点b最近的锚点
    PII anchorPII = getAnchor(p2.distance);
    int anchorTo2 = (direct2 == 1 ? anchorPII.x : anchorPII.y);
    double dis2 = anchorTo2 * 30;
    // double p1toAncDis = abs(p1.distance - dis2) * 0.125;
    costTime += abs(p1.distance - dis2) * 0.125;
    // double AntoP2Dis = abs(dis2 - p2.distance) * 0.5;
    costTime += abs(dis2 - p2.distance) * 0.5;
    // 判断有无转向
    if ((p1.distance - dis2) * (p2.distance - dis2) < 0 && direct1 == direct2 && (p2.distance - p1.distance) * direct1 > 0) {
    } else {
        costTime += 2;
    }

    return costTime;

}
inline double getTotalTime(const vector<int>& series) {
    // 从（0，0）到第一点
    double costTime = 0.0;
    int len = series.size();
    costTime += getPointTime(TapePos(0, 0, -1), points[series[0]]);
    for (int i = 0; i < len - 1; ++ i) {
        costTime += getPointTime(points[series[i]], points[series[i + 1]]);
    }

    costTime += getPointTime(points[series[len - 1]], TapePos(0, 0, -1));

    return costTime;
}

vector<int> wrapLinkAnchorToIO(const vector<uint16_t> &wrapLinkAnchor) {
    vector<int> res;
    for (int i = 0; i < wrapLinkAnchor.size(); ++ i) {
        int wrap = wrapLinkAnchor[i] >> 6;
        int anchor = wrapLinkAnchor[i] & (0x003f);

        for (const auto &[d, i] : anchorInWrap[wrap][anchor]) res.emplace_back(i);
    }
    return res;

}










inline int cmp2(const void *a, const void *b) {
    return (int) (((*(unsigned long long *) a) >> 32) - ((*(unsigned long long *) b) >> 32));
}

void tanxindiaodu() {

    static TapePos pointInInfo[MAX_IO_NUM + 1];
    static TapePos pointOutInfo[MAX_IO_NUM + 1];
    pointInInfo[0] = TapePos(0, 0.00001, -1);
    pointOutInfo[0] = TapePos(0, 0, -1);

    static int bingcha[MAX_IO_NUM + 1];

    static int nearTable[MAX_IO_NUM + 1][2];
    static int in[MAX_IO_NUM + 1];
    static int out[MAX_IO_NUM + 1];
    int n = (int) (1 + wrapLinkAnchor.size());
    nearTable[0][0] = -1;
    nearTable[0][1] = -1;
    bingcha[0] = 0;

    for (int i = 1; i < n; i++) {
        int wrap = wrapLinkAnchor[i - 1] >> 6;
        int anchor = wrapLinkAnchor[i - 1] & (0x003f);
        bingcha[i] = i;
        pointInInfo[i].wrap = wrap;
        double sub = (wrap % 2 == 0 ? 0.001 : -0.001);
        pointInInfo[i].distance = anchor * 30 + sub;
        pointOutInfo[i].wrap = wrap;
        pointOutInfo[i].distance = points[anchorInWrapSE[wrap][anchor].second].distance;
        nearTable[i][0] = -1;
        nearTable[i][1] = -1;
    }

    // for (int i = 0; i < n; ++ i) {
    //     cerr << "i: " << i << " pointInInfo[i].toString(): " << pointInInfo[i].toString() << endl;
    // }

    // for (int i = 0; i < n; ++ i) {
    //     cerr << "i: " << i << " pointOutInfo[i].toString(): " << pointOutInfo[i].toString() << endl;
    // }

    //todo in,out分别排序，out去二分查找in的位置，求前n个

    int totalTime = 0;
    for (int i = 0; i < n; i++) {
        in[i] = i;
        out[i] = i;
    }
    int originN = n;

    const int k = 500;
    static int cnt[MAX_IO_NUM + 1];
    memset(cnt, 0, sizeof cnt);
    while (n != 1) {
        static unsigned long long inSort[MAX_IO_NUM + 1];
        static unsigned long long outSort[MAX_IO_NUM + 1];
        for (int i = 0; i < n; i++) {
            int index = in[i];
            int left = (int) (pointInInfo[index].distance);
            inSort[i] = ((unsigned long long) left << 32) + i;
        }
        for (int i = 0; i < n; i++) {
            int index = out[i];
            int right = (int)(pointOutInfo[index].distance);
            outSort[i] = ((unsigned long long) right << 32) + i;
        }
        qsort(inSort, n, sizeof(unsigned long long), cmp2);
        qsort(outSort, n, sizeof(unsigned long long), cmp2);

        static int sortDisIn[MAX_IO_NUM + 1][2];
        static int sortDisOut[MAX_IO_NUM + 1][2];
        for (int i = 0; i < n; i++) {
            sortDisIn[i][0] = (int) (inSort[i] >> 32);
            sortDisIn[i][1] = (int) (inSort[i] & 0xFFFFFFFF);
            sortDisOut[i][0] = (int) (outSort[i] >> 32);
            sortDisOut[i][1] = (int) (outSort[i] & 0xFFFFFFFF);
        }

        //对每一个end，去找候选的n个中最近的
        int leftPos = 0, rightPos = min(k, n - 1);//,一般情况下包含同一块不能到0，只能0出发
        static double indexNearest[MAX_IO_NUM + 1][2];
        static unsigned long long indexSortNearest[MAX_IO_NUM + 1];
        for (int i = 0; i < n; i++) {
            int dis = sortDisOut[i][0];
            int index1 = sortDisOut[i][1];
            // cerr << "index1: " << index1 << endl;
            int leftDis = abs(dis - sortDisIn[leftPos][0]);
            while (rightPos + 1 < n && abs(dis - sortDisIn[rightPos + 1][0]) < leftDis) {
                leftPos++;
                rightPos++;
                leftDis = abs(dis - sortDisIn[leftPos][0]);
            }

            indexNearest[index1][0] = 0x3f3f3f3f;

            int realIndex1 = out[index1];
            for (int j = leftPos; j <= rightPos; ++ j) {
                int index2 = sortDisIn[j][1];
                int realIndex2 = in[index2];
                if (index1 == index2 ) {
                    //同一块,不可以往0插入
                    continue;
                }
                int realTime = (int)((getPointTime(pointOutInfo[realIndex1], pointInInfo[realIndex2]) - (realIndex2 == 0 ? 2.5 : 0)) * 10000 );
                if (realTime < indexNearest[index1][0] || (realTime == indexNearest[index1][0]
                 && cnt[(int)indexNearest[index1][1]] < cnt[index2])) {
                    //只取最近的
                    indexNearest[index1][0] = realTime;
                    indexNearest[index1][1] = index2;

                }
            }
            indexSortNearest[i] = ((unsigned long long) indexNearest[index1][0] << 32) + index1;
            cnt[(int)indexNearest[index1][1]] ++;
            // cerr << "index1 : " << index1 << "indexNearest[index1][0] << 32: " << indexNearest[index1][0] << endl;
            // cerr << "index1 的最近块 : " << indexNearest[index1][1] << endl;
        }
        
        qsort(indexSortNearest, n, sizeof(unsigned long long), cmp2);

        static int sortDisTotal[MAX_IO_NUM + 1][2];
        for (int i = 0; i < n; i++) {
            sortDisTotal[i][0] = (int) (indexSortNearest[i] >> 32);
            sortDisTotal[i][1] = (int) (indexSortNearest[i] & 0xFFFFFFFF);
        }

        //合并，近的开始
        int newN = 0;
        static int newIn[MAX_IO_NUM + 1];
        static int newOut[MAX_IO_NUM + 1];
        static int endStart[MAX_IO_NUM + 1];
        static int startEnd[MAX_IO_NUM + 1];
        for (int i = 0; i < n; i++) {
            endStart[out[i]] = in[i];
            startEnd[in[i]] = out[i];
        }
        //
        int acc = 0;
        for (int i = 0; i < n; i++) {
            int dis = sortDisTotal[i][0];
            int index1 = sortDisTotal[i][1];
            int index2 = indexNearest[sortDisTotal[i][1]][1];
            //todo index2没被连接
            int realIndex1 = out[index1];
            int realIndex2 = in[index2];
            int fa1 = bingcha[realIndex1]; 
            int fa2 = bingcha[realIndex2];
            while (bingcha[fa1] != fa1) {
                bingcha[fa1] = bingcha[bingcha[fa1]];
                fa1 = bingcha[fa1];
            }
            while (bingcha[fa2] != fa2) {
                bingcha[fa2] = bingcha[bingcha[fa2]];
                fa2 = bingcha[fa2];
            }
            if (fa1 != fa2 && nearTable[realIndex2][0] == -1 && nearTable[realIndex1][1] == -1 && acc <= 50) {
                //没被连接
                bingcha[fa2] = fa1;
                nearTable[realIndex2][0] = realIndex1;
                nearTable[realIndex1][1] = realIndex2;
                totalTime += dis;

                //两块合并了
                newIn[newN] = endStart[realIndex1];
                newOut[newN] = startEnd[realIndex2];
                endStart[newOut[newN]] = newIn[newN];
                startEnd[newIn[newN]] = newOut[newN];
                acc ++;
            } else {
                newIn[newN] = in[index1];
                newOut[newN] = out[index1];
            }
            newN++;
        }
        n = 0;
        for (int i = 0; i < newN; i++) {
            if (nearTable[newIn[i]][0] == -1 && nearTable[newOut[i]][1] == -1) {
                in[n] = newIn[i];
                out[n] = newOut[i];
                n++;
                // cerr << "i : " << i << " in: " << in[n - 1] << "out: " << out[n - 1] << endl;

            }
        }
        int realN = 0;
        for (int i = 0; i < originN; i++) {
            if (nearTable[i][0] == -1) {
                realN++;
            }
        }

        // printf("n:%d realN:%d\n", n, realN);
        // cerr << "n : " << n << "realN: " << realN << endl;

        // int cur = 0;
        // while (cur != -1) {
        //     cerr << nearTable[cur][0] << endl;
        //     cur = nearTable[cur][0];
        // }
    }

    //合并完进行输出


    //printf("totalTime:%d\n", totalTime);
    //更新了
    //更新in out，重开,
    //按照链表恢复数据
    // vector<uint16_t> backup;
    // int cur = 0;
    // for (int i = 0; i < wrapLinkAnchor.size(); i ++ ) {
    //      backup.emplace_back(wrapLinkAnchor[nearTable[cur][1] - 1]);
    //     cur = nearTable[cur][1];
    // }

    // wrapLinkAnchor = move(backup);

    // 接受0之前的
    vector<int> g1, g2, g3;
    int cur = 0;
    while (nearTable[cur][0] != -1) {
        g1.emplace_back(nearTable[cur][0]);
        cur = nearTable[cur][0];
    }
    cur = 0;
    while (nearTable[cur][1] != -1) {
        g2.emplace_back(nearTable[cur][1]);
        cur = nearTable[cur][1];
    }

    // for (int& e : g1) cerr << e << endl;
    // for (int& e : g2) cerr << e << endl;


    vector<uint16_t> backup;
    for (int i = 0; i < g2.size(); i ++ ) {
        backup.emplace_back(wrapLinkAnchor[g2[i] - 1]);
    }
    for (int i = g1.size() - 1; i >= 0; i -- ) {
        backup.emplace_back(wrapLinkAnchor[g1[i] - 1]);
    }
    cerr << backup.size() << endl;
    wrapLinkAnchor = move(backup);
}







void nearest(int startIndex, int endIndex) {
    // index : 表示从wrapLinkeAnchor那个位置开始调整 [startIndex, endIndex]
    int len = endIndex - startIndex + 1;
    vector<uint16_t> backup(len);
    int index = 0;
    vector<bool> st(len);
    // 重置数据
    // for (int i = startIndex; i <= endIndex; ++ i) {
    //     int wrap = wrapLinkAnchor[i] >> 6;
    //     int anchor = wrapLinkAnchor[i] & (0x003f);
    //     anchorInWrapSt[wrap][anchor] = false;
    // }
    // 当前(wrap, anchor)的最后一个点
    int curPointId = -1;

    while (index < len) {
        if (!index) {
            int wrap = wrapLinkAnchor[startIndex] >> 6;
            int anchor = wrapLinkAnchor[startIndex] & (0x003f);
            backup[index ++] = wrapLinkAnchor[startIndex];
            curPointId = anchorInWrapSE[wrap][anchor].second;
            // anchorInWrapSt[wrap][anchor] = true;
            st[0] = true;
            continue;
        }
        // 寻找最近邻居
        double cost = 10000;
        int target = -1;
        for (int i = startIndex; i <= endIndex; ++ i) {
            if (st[i - startIndex]) continue;

            int wrap = wrapLinkAnchor[i] >> 6;
            int anchor = wrapLinkAnchor[i] & (0x003f);
            int pointId = anchorInWrapSE[wrap][anchor].first;
            double curCost = getPointTime(points[curPointId], points[pointId]) - getPointTime(TapePos(wrap, anchor * 30, -1), points[pointId]);
            if (curCost < cost) {
                cost = curCost;
                target = i;
            }

        }
        if (target != -1) {
            st[target - startIndex] = true;
            backup[index ++] = wrapLinkAnchor[target];


            int wrap = wrapLinkAnchor[target] >> 6;
            int anchor = wrapLinkAnchor[target] & (0x003f);
            int pointId = anchorInWrapSE[wrap][anchor].second;
            curPointId = pointId;


        }
    }
    index = 0;
    for (int i = startIndex; i <= endIndex; ++ i) {
        wrapLinkAnchor[i] = backup[index ++];
    }

}

inline void nearestMixWithInsert() {
    // 预处理任意两个锚点之间的距离
    int len = wrapLinkAnchor.size();
    vector<uint16_t> backup(len + 1, 0);
    vector<bool> st(len, 0);
    // 起始点为(0,0)
    backup[0] = 0;
    double alpha = 0.2;
    int k = 0;
    while (k < len) {
        if (k + 2 < (int)(1.0 * len * (1 - alpha) + 1)) {
            // 采用最近邻
            k = k + 1;
            double dist = 0x3f3f3f3f;
            int index = -1;
            for (int i = 0; i < len; ++ i) {
                if (st[i]) continue;
                // 计算距离
                // 得到上一个点的出点
                if (k - 1 == 0) {
                    // int wrap = wrapLinkAnchor[target] >> 6;
                    // int anchor = wrapLinkAnchor[target] & (0x003f);
                    TapePos beforeOut = TapePos(0, 0, -1);
                    int afterWrap = wrapLinkAnchor[i] >> 6, afterAnchor =  wrapLinkAnchor[i] & (0x003f);
                    double sub = afterWrap % 2 == 0 ? 0.0001 : -0.0001;
                    TapePos afterIn = TapePos(afterWrap, afterAnchor * 30 + sub, -1);
                    double distance = getPointTime(beforeOut, afterIn);
                                // double curCost = getPointTime(points[curPointId], points[pointId]) - getPointTime(TapePos(wrap, anchor * 30, -1), points[pointId]);
                    if (distance < dist) {
                        dist = distance;
                        index = i;
                    }
                } else {
                    int beforeWrap = backup[k - 1] >> 6, beforeAnchor = backup[k - 1] & (0x003f);
                    TapePos beforeOut = points[anchorInWrapSE[beforeWrap][beforeAnchor].second];
                    int afterWrap = wrapLinkAnchor[i] >> 6, afterAnchor =  wrapLinkAnchor[i] & (0x003f);
                    double sub = afterWrap % 2 == 0 ? 0.0001 : -0.0001;
                    TapePos afterIn = TapePos(afterWrap, afterAnchor * 30 + sub, -1);
                    // double distance = getPointTime(beforeOut, afterIn);
                    double distance = getPointTime(beforeOut, afterIn);
                    if (distance < dist) {
                        dist = distance;
                        index = i;
                    }
                }
            }
            if (index != -1) {
                backup[k] = wrapLinkAnchor[index];
                st[index] = true;
            }
        }
        else {
            // 采用最近邻插入
            // 选择为在序列中的io
            int t = -1;
            double cost = 0x3f3f3f3f;
            // for (int i = 0; i < len; ++ i) {
            //     if (st[i]) continue;
            //     double cur =  0x3f3f3f3f;
            //     for (int j = 1; j <= k; ++ j) {
            //         int wrap = backup[j] >> 6, anchor = backup[j] & (0x003f);
            //         int afterWrap = wrapLinkAnchor[i] >> 6, afterAnchor = wrapLinkAnchor[i] & (0x003f);
            //         double sub = (afterWrap % 2 == 0? 0.0001 : -0.0001);
            //         double distance = getPointTime(points[anchorInWrapSE[wrap][anchor].second], TapePos(afterWrap, afterAnchor * 30 + sub, -1));
            //         if (distance < cur) {
            //             cur = distance;
            //         }
            //     }
            //     if (cur < cost) {
            //         cost = cur;
            //         t = i;
            //     }
            // }
            for (int i = len - 1; i >= 0; -- i) {
                if (st[i]) continue;
                t = i;
                break;
            }

            double dist = 0x3f3f3f3f;
            int insert = -1;
            for (int i = 0; i <= k; ++ i) {
                uint16_t a = -1, b = -1;
                if (i == k) {
                    a = backup[k], b = backup[0];
                } else {
                    a = backup[i], b = backup[i + 1];
                }
                int wrapA = a >> 6, anchorA = a & (0x003f);
                int wrapT = wrapLinkAnchor[t] >> 6, anchorT =  wrapLinkAnchor[t] & (0x003f);
                int wrapB = b >> 6, anchorB =  b & (0x003f);
                double sub = wrapT % 2 == 0 ? 0.0001 : -0.0001;
                double dat = getPointTime(points[anchorInWrapSE[wrapA][anchorA].second], TapePos(wrapT, anchorT * 30 + sub, -1));
                sub = wrapB % 2 == 0 ? 0.0001 : -0.0001;
                double dtb = getPointTime(points[anchorInWrapSE[wrapT][anchorT].second], TapePos(wrapB, anchorB * 30 + sub, -1));
                double dab = getPointTime(points[anchorInWrapSE[wrapA][anchorA].second], TapePos(wrapB, anchorB * 30 + sub, -1));

                if (dat + dtb - dab < dist) {
                    dist = dat + dtb - dab;
                    insert = i;
                }
                // 计算T(u, u + 1);
            }

            // 选择完毕
            for (int i = k + 1; i >= insert + 2; -- i) {
                backup[i] = backup[i - 1];
            }
            backup[insert + 1] = wrapLinkAnchor[t];
            st[t] = true;
            k = k + 1;

        }
    }

    for (int i = 0; i < len; ++ i) {
        wrapLinkAnchor[i] = backup[i + 1];
    }





}
// 领域算子3
inline void neighborsInsertPartChange(int *insertPos, int *insertL, int *insertR, int length) {
    // 选择需要移动的部分
    int l = rng.next(length);
    int r = rng.next(length);

    while (l == r || abs(r - l + 1) >= length || abs(l - r + 1) >= length) {
        r = rng.next(length);
    }

    if (l > r) {
        int temp = l;
        l = r;
        r = temp;
    }

    // 选择插入的位置随机选择
    int flag = 0, _insertPos = -1;
    while (_insertPos == -1) {
        int randomPos = -1;
        if (flag == 0 && l >= 0) {
            if (l > 0) {
                randomPos = rng.next(0, l);
            }
            flag = 1;
        } else if (flag == 1 && r < length){
            if (r < length - 1) {
                randomPos = rng.next(r + 1, length);    
            }
            flag = 0;
        }

        if (randomPos != -1) {
            _insertPos = randomPos;
        }
        // printf("%d% d %d\n", insertPos, l, r);
    }

    // 得到插入的位置以及待插入的序列

    (*insertPos) = _insertPos, (*insertL) = l, (*insertR) = r;

}
inline void updateInsertPartChange(vector<uint16_t>& oldSeries, vector<uint16_t>& newSeries, int insertPos, int insertL, int insertR, int length) {
    int l = insertL, r = insertR;
    // l - r 插入到pos位置
    if (insertPos < l) {
        // 向前插入
        for (int i = 0; i < insertPos; ++ i) newSeries[i] = oldSeries[i];
        for (int i = insertPos, len = 0; len < r - l + 1; ++ len, ++ i) {
            newSeries[i] = oldSeries[l + len];
        }
        // insertPos ---- l - 1
        for (int i = insertPos + r - l + 1, len = 0; len < l - 1 - insertPos + 1; ++ len, ++ i) {
            newSeries[i] = oldSeries[insertPos + len];
        }
        // r + 1 - length - 1
        for (int i = r + 1; i < length; ++ i) {
            newSeries[i] = oldSeries[i];
        }
    } else {
        // for (int i = 0; i < length; ++ i) {
        //     newSeries[i] = oldSeries[i];
        // }

        for (int i = 0; i <= l - 1; ++ i) newSeries[i] = oldSeries[i];
        for (int i = l, len = 0; len < insertPos - (r + 1) + 1; ++ len, ++ i) {
            newSeries[i] = oldSeries[r + 1 + len];
        }
        for (int i = l + insertPos - (r + 1) + 1, len = 0; len < r - l + 1; ++ len, ++ i) {
            newSeries[i] = oldSeries[l + len];
        }
        for (int i = insertPos + 1; i < length; ++ i) {
            newSeries[i] = oldSeries[i];
        }
    }

}
inline void hillClimbInsert() {
 /* 定初始化输出参数 */

    vector<uint16_t> oldSeries = wrapLinkAnchor;
    int length = wrapLinkAnchor.size();
    vector<uint16_t> newSeries =  oldSeries;
    vector<uint16_t> bestSeries = oldSeries;

    int cnt = 0;
     result = wrapLinkAnchorToIO( oldSeries);
    double cur =  getTotalTime(result);
    // cerr << "oldSeries.size(): " <<  oldSeries.size() << endl;
    // int wrap = oldSeries[1191] >> 6, anchor = oldSeries[1191] & (0x003f);
    // cerr << points[anchorInWrapSE[wrap][anchor].second].toString() << endl;
    
    while (runtime() < MAX_RUN_TIME) {

        cnt ++;
        /**
        * 使用swapChange算子的时候减少不必要的计算开销
        */
        int insertPos = -1, left = -1, right = -1;
        neighborsInsertPartChange(&insertPos, &left,  &right, length);
        // 计算改变的时间
        double before = 0.0;
        double after = 0.0;
        
        if (insertPos < left) {
            // 得到旧情况未插入前开销
            // 插入点前一个点与插入点之间的开销
            /*****************************计算旧序列的所有改变的开销**********************************************/
                        // 得到两个包的wrap和anchor
            int wrapInsert = oldSeries[insertPos] >> 6, anchorInsert = oldSeries[insertPos] & (0x003f);
            int wrapLeft = oldSeries[left] >> 6, anchorLeft = oldSeries[left] & (0x003f);
            int wrapBeforeLeft = oldSeries[left - 1] >> 6, anchorBeforeLeft = oldSeries[left - 1] & (0x003f);
            int wrapRight = oldSeries[right] >> 6, anchorRight = oldSeries[right] & (0x003f);


            const TapePos &InsertP = points[anchorInWrapSE[wrapInsert][anchorInsert].first];

            const TapePos &beforeLeftP = points[anchorInWrapSE[wrapBeforeLeft][anchorBeforeLeft].second];
            const TapePos &leftP = points[anchorInWrapSE[wrapLeft][anchorLeft].first];

            if (insertPos == 0) {
                // 插入位置为0计算（0，0）到起点的开销
                TapePos zero = TapePos(0, 0, -1);
                before += getPointTime(zero, InsertP);
            } else {

                int wrapInsertBefore = oldSeries[insertPos - 1] >> 6, anchorInsertBefore = oldSeries[insertPos - 1] & (0x003f);
                const TapePos &InsertBeforeP = points[anchorInWrapSE[ wrapInsertBefore][ anchorInsertBefore].second];
                before += getPointTime(InsertBeforeP, InsertP);
            }

            // 待插入序列的l与l-1之间的开销
            before += getPointTime(beforeLeftP, leftP);
            // 待插入序列r与r+1之间的开销
            if (right == length - 1) {
                const TapePos &rightP = points[anchorInWrapSE[wrapRight][anchorRight].second];
                const TapePos zero = TapePos(0, 0.0001, -1);
                before += getPointTime(rightP, zero);
            }
            else {
                int wrapafterRight = oldSeries[right + 1] >> 6, anchorafterRight = oldSeries[right + 1] & (0x003f);
                const TapePos &rightP = points[anchorInWrapSE[wrapRight][anchorRight].second];
                const TapePos &afterRightP = points[anchorInWrapSE[wrapafterRight][ anchorafterRight].first];
                before += getPointTime(rightP, afterRightP);
                // cerr << points[anchorInWrapSE[wrapRight][anchorRight].second].toString() << endl;
                // cerr << points[anchorInWrapSE[wrapafterRight][ anchorafterRight].first].toString() << endl;
                // cerr << "ssssssssss: " << getPointTime(rightP, afterRightP) << endl;
            }


            /*****************************计算新序列的所有改变的开销**********************************************/

            // 插入点之前的点到l的开销
            if (insertPos == 0) {
                // 插入位置为0计算（0，0）到起点的开销
                TapePos zero = TapePos(0, 0, -1);
                after += getPointTime(zero, leftP);

            } else {

                int wrapInsertBefore = oldSeries[insertPos - 1] >> 6, anchorInsertBefore = oldSeries[insertPos - 1] & (0x003f);
                const TapePos &InsertBeforeP = points[anchorInWrapSE[ wrapInsertBefore][ anchorInsertBefore].second];
                after += getPointTime(InsertBeforeP, leftP);           
            }

            // 插入点的右侧到插入点的开销
            const TapePos &rightP = points[anchorInWrapSE[wrapRight][anchorRight].second];
            after += getPointTime(rightP, InsertP );

            // l前一个点到r后一点的开销
            if (right == length - 1) {
                const TapePos zero = TapePos(0, 0.0001, -1);
                 after += getPointTime(beforeLeftP, zero);
            }
            else {
                int wrapafterRight = oldSeries[right + 1] >> 6, anchorafterRight = oldSeries[right + 1] & (0x003f);
                const TapePos &afterRightP = points[anchorInWrapSE[wrapafterRight][anchorafterRight].first];
                 after += getPointTime(beforeLeftP,afterRightP);
            }


        } else {

        }

        /********************************************************************/
        if (after < before) {
            // cerr << after << " " << before << endl;
            updateInsertPartChange(oldSeries, newSeries, insertPos, left, right, length);
            oldSeries = newSeries;
            result = wrapLinkAnchorToIO( oldSeries);
            cur = cur + (after - before);
            // cerr << "caculate: " << cur << endl;
            // cerr << "really: " << getTotalTime(result) << endl;

            #ifdef DEBUG
                printf("bestScore: %lf\n", bestResult );
            #endif
        }

                
                
        
        
        
    }
  
    // cerr << "迭代次数: " << cnt << endl;


    bestSeries = oldSeries;

    result = wrapLinkAnchorToIO( bestSeries);


}

int main(int argc, char* argv[])
{
    init();

    #ifdef LOCAL
    if (fopen("../../input.txt", "r") != nullptr) {

        freopen(("../../input.txt"), "r", stdin);
        // freopen(("../output.txt"), "r", stdout);
        
    } else {
        cerr << "open file uncorrectly" << endl;
    }
    #endif

    std::ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);

    int n;
    cin >> n;


    for (int i = 0; i < n; i++)
    {
        int w;
        double d;
        cin>> w >> d;
        points.push_back(TapePos(w, d, i));
        // wrap[w].emplace_back(d);

        int AnchorId = -1;
        if (w % 2 == 0 ) {
            AnchorId = getAnchor(d).first;
        } else {
            AnchorId = getAnchor(d).second;
        }

        int bandId = getBand(w);

        anchorInWrap[w][AnchorId].push_back({d, i});
    }
  
    for (int i = 0; i <= 207; ++ i) {
        int bandId = getBand(i);
        for (int j = 0; j <= 32; ++ j) {
            if (!anchorInWrap[i][j].empty()) {
                ++ bandHasAnchor[bandId][i % 2][j];
            }
        }
    }





    if (n == 5) {
        bool good = true;
        for (int i = 0; i < n; i ++ ) {
            good = good & (special[i] == points[i].wrap);
        }
        if (good) {
            // 暴力dfs找最优
            vector<int> initSolve = {0, 1, 2, 3, 4};
            vector<int> best = initSolve;
            double bestTime = getTotalTime(initSolve);
            do {
                if (getTotalTime(initSolve) < bestTime) {
                    bestTime = getTotalTime(initSolve);
                    best = initSolve;
                }
            }while(next_permutation(initSolve.begin(), initSolve.end()));


            
            for (int i = 0; i < n; ++ i) {
                printf("%d ", best[i]);
            }

            return 0;
        }


    }



    // 排序
    for (int i = 0; i <= 207; ++ i) {
        if (i % 2 == 0) {
            for (int j = 0; j < 32; j ++ ) {
                if (anchorInWrap[i][j].empty()) continue;
                sort(anchorInWrap[i][j].begin(), anchorInWrap[i][j].end());
                // anchorInWrapSE[i][j] = make_pair(anchorInWrap[i][j][0].second, anchorInWrap[i][j].back().second);
            }
        } else {
            for (int j = 1; j <= 32; j ++ ) {
                if (anchorInWrap[i][j].empty()) continue;
                sort(anchorInWrap[i][j].rbegin(), anchorInWrap[i][j].rend());
            }            
        }
    }

    for (int i = 0; i <= 207; ++ i) {
        for (int j = 0; j <= 32; ++ j) {
            if (anchorInWrap[i][j].empty()) {
                anchorInWrapSE[i][j] = make_pair(0, 0);
                continue;
            }

            anchorInWrapSE[i][j] = make_pair(anchorInWrap[i][j][0].second, anchorInWrap[i][j].back().second);

            // cout << "wrap: " << i << " anchor: " << j << " start: " << anchorInWrap[i][j][0].x << " end: " << anchorInWrap[i][j].back().x << endl;
        }
    }

    for (int i = 0; i <= 207; ++ i) {
        if (i % 2 == 0) {
            for (int j = 0; j <= 32; ++ j) {
                if (!anchorInWrap[i][j].empty()) {
                    uint16_t flag = (i << 6) + (j);
                    wrapLinkAnchor.emplace_back(flag);
                }
            }            
        } else {
            for (int j = 32; j >= 0; -- j) {
                if (!anchorInWrap[i][j].empty()) {
                    uint16_t flag = (i << 6) + (j);
                    wrapLinkAnchor.emplace_back(flag);
                }
            }  
        }
    }
    // nearestMixWithInsert();
    tanxindiaodu();
    // nearest(0, wrapLinkAnchor.size() - 1);
    result = wrapLinkAnchorToIO(wrapLinkAnchor);
    const vector<int> &res = result; 

    #ifdef LOCAL
        cerr << runtime() << endl;
        double costTime = getTotalTime(res);
        cerr << "start total Time: " << costTime << endl;
        cerr << "wrapAndAnchorNumber: " << wrapLinkAnchor.size() << endl;
        cerr << "io number :" << result.size() << endl;
        int score = (int)(1.0 * res.size() / costTime * 1e7);
        cerr << "score: " << score << endl;
    #endif

    // cerr << "优化之前的结果: " << endl;
    // int cnt = 0;
    // int preD = 1;
    // for (int i = 0; i < wrapLinkAnchor.size(); ++ i) {
    //     int wrap = wrapLinkAnchor[i] >> 6;
    //     int anchor = wrapLinkAnchor[i] & (0x003f);

    //     int curD = (wrap % 2 == 0 ? 1 : -1);
    //     if (curD != preD ) {
    //         cerr << "***************************** "  << "cnt: "<< cnt << endl;
    //         preD = curD;
    //         cnt = 1;
    //     } else {
    //         cnt ++;
    //     }


    //     cerr << "index: " << i << " wrap: " << wrap << "anchor: " << anchor << endl;



    // }

    // double noVaildTime = 0.0;
    // cerr << "每个wrap与前一个wrap的时间差" << endl;
    // double add = (wrapLinkAnchor[0] >> 6) % 2 == 0 ? 0.0001 : -0.0001;
    // noVaildTime += getPointTime(TapePos{0, 0, -1}, TapePos{wrapLinkAnchor[0] >> 6, 1.0 * (wrapLinkAnchor[0] & (0x003f)) * 30 + add, -1});
    // cerr << getPointTime(TapePos{0, 0, -1}, TapePos{wrapLinkAnchor[0] >> 6, 1.0 * (wrapLinkAnchor[0] & (0x003f)) * 30 + add, -1}) << endl;

    // for (int i = 0; i < wrapLinkAnchor.size() - 1; ++ i) {
    //     int wrap1 = wrapLinkAnchor[i] >> 6;
    //     int anchor1 = wrapLinkAnchor[i] & (0x003f);

    //     int wrap2 = wrapLinkAnchor[i + 1] >> 6;
    //     int anchor2 = wrapLinkAnchor[i + 1] & (0x003f);

    //     double add = wrap2 % 2 == 0 ? 0.00001 : -0.00001;

    //     TapePos p = points[anchorInWrapSE[wrap1][anchor1].y];
    //     if (getPointTime(p, TapePos{wrap2, 1.0 * anchor2 * 30 + add, -1}) > 10) {
    //         cerr << "index:" + to_string(i) + "[" + to_string(wrap1) + "," + to_string(anchor1) + "]" + "->" 
    //         + "[" + to_string(wrap2) + "," + to_string(anchor2) + "]"
    //         << getPointTime(p, TapePos{wrap2, 1.0 * anchor2 * 30 + add, -1}) << endl;  
    //     }
    //     // cerr << "[" + to_string(wrap1) + "," + to_string(anchor1) + "]" + "->" 
    //     // + "[" + to_string(wrap2) + "," + to_string(anchor2) + "]"
    //     // << getPointTime(p, TapePos{wrap2, 1.0 * anchor2 * 30 + add, -1}) << endl;
    //     noVaildTime += getPointTime(p, TapePos{wrap2, 1.0 * anchor2 * 30 + add, -1});
    // }
    // int wrap1 = wrapLinkAnchor[wrapLinkAnchor.size() - 1] >> 6;
    // int anchor1 = wrapLinkAnchor[wrapLinkAnchor.size() - 1] & (0x003f);
    // TapePos p = points[anchorInWrapSE[wrap1][anchor1].y];
    // noVaildTime += getPointTime(p, TapePos{0, 0.00001, -1});
    // cerr << "noVaildTime : " << noVaildTime << endl;

  
  



    // hillClimbInsert();
    // #ifdef LOCAL
    //     cerr << runtime() << endl;
    //     costTime = getTotalTime(res);
    //     cerr << "end total Time: " << costTime << endl;
    //     cerr << "io number :" << result.size() << endl;
    //     score = (int)(1.0 * res.size() / costTime * 1e7);
    //     cerr << "score: " << score << endl;
    // #endif
    








    //  for (int i = 0; i < (int)res.size(); i++)
    // {
    //     printf("%d ", res[i]);
    // }


    // cerr << runtime() << endl;


}







