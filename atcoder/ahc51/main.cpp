
#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <numeric>
#include <queue>
#include <random>
#include <climits>
#include <map>
#define LOCAL

using namespace std;
typedef pair<int, int> PII;
typedef pair<double, int> PDI;
#define x first
#define y second
constexpr int N = 30, M = 1010, NM = 1040;
constexpr int limit = 1900;
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
    int next(int a, int b) {
        return a + (rand() % (b - a));  
    }
    double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 

RNG rng;

struct Point {
    int x;
    int y;
    
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};
bool operator!=(const Point& lhs, const Point& rhs) {
    return !(lhs == rhs);
}
struct Line {
    Point p1;
    Point p2;
    Line(Point p1, Point p2) : p1(p1), p2(p2){}
};

int sign(int x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

int orientation(const Point& a, const Point& b, const Point& c) {
    long long cross = (long long)(b.x - a.x) * (c.y - a.y) - (long long)(b.y - a.y) * (c.x - a.x);
    return sign(cross);
}

// 检查线段p1p2和线段q1q2是否相交
bool segmentsIntersect(const Point& p1, const Point& p2, const Point& q1, const Point& q2) {
    // 边界盒检测：如果两个线段的边界盒不相交，则线段一定不相交
    if (std::max(p1.x, p2.x) < std::min(q1.x, q2.x) ||
        std::max(q1.x, q2.x) < std::min(p1.x, p2.x) ||
        std::max(p1.y, p2.y) < std::min(q1.y, q2.y) ||
        std::max(q1.y, q2.y) < std::min(p1.y, p2.y)) {
        return false;
    }
    
    // 计算四个方向值
    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);
    
    // 一般情况：线段相交的条件
    // 四个点必须是不同点
    bool different = (p1 != q1 && p1 != q2 && p2 != q1 && p2 != q2);
    return (o1 * o2 <= 0) && (o3 * o4 <= 0) && different;
}

struct Node{
    vector<int> leafs{};
    PII sons{-1, -1};
    int type{-1}; // isLeaf == true : processorType : sorterType
    bool isLeaf{true};

    Node(){}
    Node(PII _sons, bool _isLeaf, int _type) : sons(_sons), isLeaf(_isLeaf), type(_type){}
};

struct Strategy {
    int n, m, k;
    int entranceX{0}, entranceY{5000};
    vector<PII> proPosition;
    vector<PII> sortPosition;
    vector<vector<double>> probability;

    vector<int> saveType;
    vector<int> saveBestType;
    vector<vector<int>> saveExit;
    vector<vector<int>> saveBestExit;
    vector<int> typeToPos;
    int entranceLinkToSorter{0};
    vector<vector<PII>> disGroups;
    double score{0.0};
    // Node nodes[NM];
    vector<Node> nodes;
    vector<Node> bestNodes;
    int bestIdx = 0;
    int idx = 0;
    vector<double> arrivePros;

    // 新增数据结构用于构建真实树
    vector<int> virtualToReal;  // 虚拟节点ID到真实位置ID的映射
    vector<bool> usedProcessors; // 标记处理器是否被使用
    vector<bool> usedSorters;    // 标记分类器是否被使用
    vector<Line> currentLines;   // 当前已添加的传送带线段
    map<int, int> processorTypeMap; // 处理器类型到位置的映射

    void input() {
        scanf("%d %d %d", &n, &m, &k);
        proPosition.resize(n);
        sortPosition.resize(m);
        probability.resize(k);

        for (int i = 0; i < n; ++ i) scanf("%d %d", &proPosition[i].x, &proPosition[i].y);
        for (int i = 0; i < m; ++ i) scanf("%d %d", &sortPosition[i].x, &sortPosition[i].y);
        for (int i = 0; i < k; ++ i) {
            for (int j = 0; j < n; ++ j) {
                double rate;
                scanf("%lf", &rate);
                probability[i].emplace_back(rate);
            }
        }
    }
        void planning() {
        saveType.resize(n);
        iota(saveType.begin(), saveType.end(), 0);
        // 0 : 2; 1 : 1
        // 映射type to processor
        typeToPos.resize(n);
        for (int i = 0; i < n; ++ i) typeToPos[saveType[i]] = i;

        // 第一个位置
        int pos = 0;
        entranceLinkToSorter = pos + n;
        // 记录选择的分类器，exit1连接的处理器，exit2连接的处理器
        int targetSorter = -1, exit1 = -1, exit2 = -1;
        double tempScore = 0.0;
        // 在该位置遍历所有分类器类型
        for (int i = 0; i < k; ++ i) {
            // 计算exit1与exit2的最大值
            for (int j1 = 0; j1 < n; ++ j1) {
                for (int j2 = 0; j2 < n; ++ j2) {
                    if (j1 != j2) {
                        // exit1 : j1; exit2 : j2;
                        if (probability[i][j1] + (1-probability[i][j2]) > tempScore) {
                            tempScore = probability[i][j1] + (1-probability[i][j2]);
                            targetSorter = i, exit1 = typeToPos[j1], exit2 = typeToPos[j2];
                        }
                    }
                }
            }
        }
        saveExit.resize(m);
        // 保存结果
        for (int i = 0; i < m; ++ i) {
            if (i != pos) {
                saveExit[i] = {-1};
                continue;
            }
            saveExit[i]= {targetSorter, exit1, exit2};
        }

        tempScore = getTotalScore(saveExit, saveType);
        if (tempScore > score) {
            score = tempScore;
            saveBestExit = move(saveExit);
            saveBestType = saveType;
            cerr << score << endl;
        }

    }
    
    bool planning2() {
        // 规划图
        vector<PII> sortForProcessor(n);
        for (int i = 0; i < n; ++ i)  sortForProcessor[i] = make_pair(proPosition[i].x, i);
        sort(sortForProcessor.begin(),  sortForProcessor.end());
        vector<PII> sortForSorter(m);
        for (int i = 0; i < m; ++ i) sortForSorter[i] = make_pair(sortPosition[i].x, i);
        sort(sortForSorter.begin(), sortForSorter.end());
        vector<Line> lines;
        vector<vector<int>> edges(n + m + 1);
        vector<bool> st(n + m + 1, 0);
        vector<int> oldLines;
        Point curPoint{entranceX, entranceY};
        int curPointIndex = n + m;
        int curLine = 0;
        int cnt = 0;
        while (cnt < n) {
            st[curPointIndex] = true;
            int tempLine = curLine;
            int curPid = -1;
            for (const auto& [x, pid] : sortForProcessor) {
                if (st[pid]) continue;
                curPid = pid;
                tempLine = x;
                break;
            }
            Point pid{proPosition[curPid].x, proPosition[curPid].y};
            // // 在rightLine ~ tempLine之间寻找
            int left = upper_bound(sortForSorter.begin(), sortForSorter.end(), make_pair(curLine, 0)) - sortForSorter.begin();
            int right = upper_bound(sortForSorter.begin(), sortForSorter.end(), make_pair(tempLine, 0)) - sortForSorter.begin() - 1;
            // left ~ right
            if (left <= right) {
                int heightDiff = INT_MAX;
                int sorterIndex = -1, poolLevel = (tempLine - curLine) / 2;
                for (int j = right; j >= left; -- j) {
                    int sid = sortForSorter[j].y;
                    int sidX = sortPosition[sid].x, sidY = sortPosition[sid].y;
                    if (!st[sid + n] && (sorterIndex == -1 || ((tempLine - sidX) <= poolLevel && heightDiff > (abs(pid.y - sidY))))) {
                        sorterIndex = sid;
                        heightDiff = abs(pid.y - sidY);
                    }
                }
                // 点之间连接线
                edges[curPointIndex].emplace_back(sorterIndex + n);
                edges[sorterIndex + n].emplace_back(curPid);
                st[curPid] = true;
                Point tempPoint{sortPosition[sorterIndex].x, sortPosition[sorterIndex].y};
                curPointIndex = sorterIndex + n;
                oldLines.emplace_back(curLine);
                curLine = tempLine;
                lines.emplace_back(Line{curPoint, tempPoint});
                lines.emplace_back(Line{tempPoint,pid});
                curPoint = move(tempPoint);

            } else {
                // return false;
                // 回退
                if (!oldLines.empty()) {
                    // preLine ~ curLine
                    curLine = oldLines.back();
                    oldLines.pop_back();
                    continue;
                } else {
                    // 这个点找不到不找了
                    st[curPid] = 1;
                }
            }

            ++ cnt;
        }

        // 留出最后两个点单独处理(n >= 5)
        edges[curPointIndex].clear();
        int pid1 = sortForProcessor[n - 2].second, pid2 = sortForProcessor[n - 1].second;
        Point tempPoint1{proPosition[pid1].x, proPosition[pid1].y};
        Point tempPoint2{proPosition[pid2].x, proPosition[pid2].y};
        edges[curPointIndex].emplace_back(pid1);
        edges[curPointIndex].emplace_back(pid2);
        lines.emplace_back(Line{curPoint, tempPoint1});
        lines.emplace_back(Line{curPoint, tempPoint2});
        cnt += 1;




        int sz = lines.size();
        for (int i = 0; i < sz; ++ i) {
            for (int j = i + 1; j < sz; ++ j) {
                if (segmentsIntersect(lines[i].p1, lines[i].p2, lines[j].p1, lines[j].p2)) {
                    return false;
                }
            }
        }

        // 说明找到部署的图了
        saveType.resize(n);
        iota(saveType.begin(), saveType.end(), 0);
        random_device rd;
        // 使用 std::random_device 生成随机种子
        std::mt19937 g(rd()); // Mersenne Twister 伪随机数生成器
    
        while (runtime() < limit) {
            // 打乱 vector
            std::shuffle(saveType.begin(), saveType.end(), g);
            typeToPos.resize(n);
            for (int i = 0; i < n; ++ i) typeToPos[saveType[i]] = i;
            saveExit.clear();
            saveExit.resize(m);  
            // 构造图
            int root = n + m;
            queue<PII> q;
            q.emplace(root, 0);
            vector<vector<double>> proGroups(m + 1, vector<double>(n, 0.0));
            for (int i = 0; i < n; ++ i) proGroups[m][i] = 1.0; // 映射根节点
            for (int i = 0; i < n; ++ i) proGroups[entranceLinkToSorter - n][i] = 1.0; // 映射入口节点
            while (!q.empty()) {
                auto [u, deepth] = q.front();
                q.pop();
                if (u == root) {
                    for (const int& son : edges[u]) {
                        q.emplace(son, deepth + 1);
                        entranceLinkToSorter = son;
                    }
                    continue;
                } else {
                    int index = u - n;
                    saveExit[index].assign(3, -1);
                    int idx = 1;
                    for (const int& son : edges[u]) {
                        // u - son
                        if (son < n && saveExit[index][0] == -1) {

                            int sonType = saveType[son];
                            bool isLeft = true;
                            int sorterType = -1;
                            // 选择对该类型最大的
                            double tempPro = 0.0;
                            for (int i = 0; i < k; ++ i) {

                                if (probability[i][sonType] > 0.5 && probability[i][sonType] > tempPro) {
                                    tempPro = probability[i][sonType];
                                    isLeft = true;
                                    sorterType = i;
                                } else if (probability[i][sonType] <= 0.5 && (1 - probability[i][sonType]) > tempPro) {
                                    tempPro = 1 - probability[i][sonType];
                                    isLeft = false;
                                    sorterType = i;
                                }
                            }


     



                            saveExit[index][0] = sorterType;
                            if (isLeft) saveExit[index][1] = son;
                            else saveExit[index][2] = son;

                            continue;
                        }
                        
                        int op = 1;
                        if (saveExit[index][op] != -1) op = 2;
                        saveExit[index][op] = son;
                        if (son >= n) {
                            q.emplace(son, deepth + 1);
                        }
                    }
                }
            }
            for (int i = 0; i < m; ++ i) {
                if (saveExit[i].size() > 0) {
                    // cerr << i + n << " " << saveExit[i][0] << " " << saveExit[i][1] << " " << saveExit[i][2] << endl;
                    continue;
                } else {
                    saveExit[i] = {-1};
                }
            }

            double tempScore = getTotalScore(saveExit, saveType);
            if (tempScore > score) {
                score = tempScore;
                saveBestExit = move(saveExit);
                saveBestType = (saveType);
                cerr << score << endl;
            }


        }



        return true;


        

    }    
    
    void dealData() {
        disGroups = vector<vector<PII>> (n + m, vector<PII>(n + m));
        for (int i = 0; i < n + m; ++ i) {
            for (int j = 0; j < n + m; ++ j) {
                if (i == j) {
                    disGroups[i][j] = make_pair(0, j);
                } else {
                    int x1, y1, x2, y2;
                    if (i < n ) {
                        x1 = proPosition[i].x, y1 = proPosition[i].y;
                    } else {
                        x1 = sortPosition[i - n].x, y1 =  sortPosition[i - n].y;
                    }
                    if (j < n ) {
                        x2 = proPosition[j].x, y2 = proPosition[j].y;
                    } else {
                        x2 = sortPosition[j - n].x, y2 =  sortPosition[j - n].y;
                    }

                    int dis = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
                    disGroups[i][j] = make_pair(dis, j);
                }
            }
        }

        for (int i = 0; i < n + m; ++ i ) sort(disGroups[i].begin(), disGroups[i].end());
    }

    double calculate(int id1, int id2, int sorterType, bool& isReverse) {
        double pro = 0.0;
        double sl = 0.0, sr = 0.0, reverseSl = 0.0, reverseSr = 0.0;
        for (const int& leafId : nodes[id1].leafs) {
            int leafType = nodes[leafId].type;
            sl += probability[sorterType][leafType] * arrivePros[leafType];
            reverseSl += (1 - probability[sorterType][leafType]) * arrivePros[leafType];
        }
        for (const int& leafId : nodes[id2].leafs) {
            int leafType = nodes[leafId].type;
            sr += (1 -  probability[sorterType][leafType]) * arrivePros[leafType];
            reverseSr += (probability[sorterType][leafType]) * arrivePros[leafType];
        }
        if (sl + sr >= reverseSl + reverseSr) {
            pro = sl + sr;
            isReverse = false;
            return pro;
        }
        pro = reverseSl + reverseSr;
        isReverse = true;
        return pro;
    }

    void update(int id, int sorterType, bool isLeft) {
        if (isLeft) {
            for (const int& leafId : nodes[id].leafs) {
                int type = nodes[leafId].type;
                arrivePros[type] *= probability[sorterType][type];
            }
        } else {
            for (const int& leafId : nodes[id].leafs) {
                int type = nodes[leafId].type;
                arrivePros[type] *= ( 1- probability[sorterType][type]);
            }            
        }
    }

    void buildVirtualTree(int op, int startTime1, int runTime) {
        // 回复数据
        nodes.clear();
        idx = 0;
        nodes.resize(NM);
        arrivePros.assign(n, 1);
        vector<int> types(n);
        iota(types.begin(), types.end(), 0);
        vector<bool> isInType(n, false);
        int start = -1;
        for (int i = 0; i < min(n, op); ++ i) {
            int type = i;
            nodes[idx] = {make_pair(-1, -1), true, type};
            nodes[idx].leafs.emplace_back(idx);
            isInType[type] = true;
            ++ idx;
        }
        start = idx;
        
        vector<int> anthology(start);
        for (int i = 0; i < start; ++ i) anthology.emplace_back(i);
        random_device rd;
        std::mt19937 g(rd());
        double iterScore = 0.0;

        while ((runtime() - startTime1) < runTime * 0.5 ) {  // 减少虚拟树时间，给真实树留时间
            anthology.resize(start);
            for (int i = 0; i < start; ++ i) anthology[i] = i;
            // // 重新打乱类型
            shuffle(types.begin(), types.end(), g);
            isInType.assign(n, 0);
            idx = 0;
            for (int i = 0; i < start; ++ i) {
                nodes[idx ++ ].type = types[i];
                isInType[types[i]] = true;

            }
            idx = start;
            arrivePros.assign(n, 1.0);

            while (anthology.size() > 1) {
                shuffle(anthology.begin(), anthology.end(), g);
                int sz = anthology.size();
                vector<PII> pairings;
                int zh = rng.next(1, sz / 2 + 1);
                for (int i = 0; i < sz ; i += 2) {
                    if (i + 1 < sz && zh) {
                        pairings.emplace_back(anthology[i], anthology[i + 1]);
                        -- zh;
                        continue;
                    } 
                    pairings.emplace_back(anthology[i], -1);
                    if (i + 1 < sz) {
                        pairings.emplace_back(anthology[i + 1], -1);
                    }
                }

                anthology.clear();
                
                for (const auto& [id1, id2] : pairings) {
                    if (id1 == -1 || id2 == -1) {
                        int targetId = (id1 != -1 ? id1 : id2);
                        anthology.emplace_back(targetId);
                    } else {
                        double pro = 0.0;
                        int sorterType = -1;
                        PII result{};
                        for (int i = 0; i < k; ++ i) {
                            bool isReverse = false;
                            double curPro = calculate(id1, id2, i, isReverse);
                            if (curPro > pro) {
                                pro = curPro;
                                sorterType = i;
                                if (isReverse) {
                                    result = make_pair(id2, id1);
                                } else {
                                    result = make_pair(id1, id2);
                                }
                            }
                        }

                        nodes[idx] = {result, false, sorterType};
                        nodes[idx].leafs.clear();
                        nodes[idx].leafs.reserve(nodes[id1].leafs.size() + nodes[id2].leafs.size());
                        nodes[idx].leafs.insert(nodes[idx].leafs.end(), nodes[id1].leafs.begin(), nodes[id1].leafs.end());
                        nodes[idx].leafs.insert(nodes[idx].leafs.end(), nodes[id2].leafs.begin(), nodes[id2].leafs.end());
                        anthology.emplace_back(idx);
                        ++ idx;
                        update(result.first, sorterType, true);
                        update(result.second, sorterType, false);
                    }
                }
            }
            
            double s = 0.0;
            for (int i = 0; i < n; ++ i ) {
                if (isInType[i])
                    s += arrivePros[i];
            }
            if (s > iterScore) {
                iterScore = s;
                cerr << "虚拟树最优得分: " << iterScore << endl;
                bestNodes = nodes;
                bestIdx = idx;

            }
        }

        nodes = move(bestNodes);
        idx = bestIdx;
    }

    int getDistance(const Point& p1, const Point& p2) {
        return int(sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
    }

    // 检查线段是否与现有线段交叉
    bool isLineValid(const Line& newLine) {
        for (const auto& line : currentLines) {
            if (segmentsIntersect(newLine.p1, newLine.p2, line.p1, line.p2)) {
                return false;
            }
        }
        return true;
    }

    // 为虚拟节点找到最合适的真实分类器位置
    int findBestSorter(int parentRealId, bool isLeft) {
        Point parentPoint;
        if (parentRealId == -1) {
            // 父节点是入口
            parentPoint = Point(entranceX, entranceY);
        } else if (parentRealId < n) {
            // 不应该发生，父节点不能是处理器
            return -1;
        } else {
            // 父节点是分类器
            parentPoint = Point(sortPosition[parentRealId - n].x, sortPosition[parentRealId - n].y);
        }

        // 优先选择x坐标递增的位置，减少交叉概率
        int bestSorter = -1;
        int minDistance = INT_MAX;
        
        for (int i = 0; i < m; ++ i) {
            if (usedSorters[i]) continue;
            
            Point sorterPoint(sortPosition[i].x, sortPosition[i].y);
            // 左子树优先选择y较小的位置，右子树优先选择y较大的位置，减少交叉
            if ((isLeft && sorterPoint.y >= parentPoint.y) || 
                (!isLeft && sorterPoint.y <= parentPoint.y)) {
                continue;
            }
            
            // // 尽量选择x坐标大于父节点的位置，形成从左到右的布局
            if (sorterPoint.x < parentPoint.x) continue;
            
            Line testLine(parentPoint, sorterPoint);

            if (!isLineValid(testLine)) continue;
            
            int distance = getDistance(parentPoint, sorterPoint) + rng.next(0, 2000);
            if (distance < minDistance) {
                minDistance = distance;
                bestSorter = i;
            }
        }
        
        // 如果没找到合适的，放宽x坐标限制
        if (bestSorter == -1) {
            for (int i = 0; i < m; ++ i) {
                if (usedSorters[i]) continue;
                
                Point sorterPoint(sortPosition[i].x, sortPosition[i].y);
                Line testLine(parentPoint, sorterPoint);
                if (!isLineValid(testLine)) continue;

                bool add = (rng.next(0, 2) ? 1 : 0);
                double cost = (add ? rng.next(0, 2000) : 0);
                int distance = getDistance(parentPoint, sorterPoint) + cost ;
                if (distance < minDistance) {
                    minDistance = distance;
                    bestSorter = i;
                }
            }
        }
        
        return bestSorter;
    }

    // 为虚拟叶子节点找到最合适的真实处理器位置
    int findBestProcessor(int virtualLeafId, int parentRealId) {
        Point parentPoint;
        if (parentRealId < n) {
            // 不应该发生
            return -1;
        } else if (parentRealId == n + m) {
            // 父节点是入口
            parentPoint = Point(entranceX, entranceY);
        } else {
            // 父节点是分类器
            parentPoint = Point(sortPosition[parentRealId - n].x, sortPosition[parentRealId - n].y);
        }

        int leafType = nodes[virtualLeafId].type;
        int bestProcessor = -1;
        int minDistance = INT_MAX;
        
        for (int i = 0; i < n; ++ i) {
            if (usedProcessors[i]) continue;
            
            Point processorPoint(proPosition[i].x, proPosition[i].y);
            Line testLine(parentPoint, processorPoint);
            if (!isLineValid(testLine)) continue;
            
            int distance = getDistance(parentPoint, processorPoint);
            if (distance <= minDistance) {
                minDistance = distance;
                bestProcessor = i;
            }
        }
        
        return bestProcessor;
    }

    // 递归构建真实树
    bool recursiveBuild(int virtualNodeId, int parentRealId, bool isLeftChild) {
        // 不需要
        // if ( (runtime() - startTime1) >=  runTime) {
        //     return false; // 时间快到了，提前返回
        // }
        
        if (nodes[virtualNodeId].isLeaf) {
            // 叶子节点，分配处理器
            int processorId = findBestProcessor(virtualNodeId, parentRealId);
            if (processorId == -1) return false;
            
            virtualToReal[virtualNodeId] = processorId;
            usedProcessors[processorId] = true;
            
            // 记录处理器类型映射
            processorTypeMap[processorId] = nodes[virtualNodeId].type;
            
            // 添加传送带线段
            Point parentPoint;
            if (parentRealId == n + m) {
                parentPoint = Point(entranceX, entranceY);
            } else {
                parentPoint = Point(sortPosition[parentRealId - n].x, sortPosition[parentRealId - n].y);
            }
            Point processorPoint(proPosition[processorId].x, proPosition[processorId].y);
            currentLines.emplace_back(parentPoint, processorPoint);
            
            return true;
        } else {
            // 内部节点，分配分类器
            int sorterId = findBestSorter(parentRealId, isLeftChild);
            if (sorterId == -1) return false;
            
            int realNodeId = n + sorterId; // 分类器的真实ID是n + 位置索引
            virtualToReal[virtualNodeId] = realNodeId;
            usedSorters[sorterId] = true;
            
            // 记录分类器类型和出口
            saveExit[sorterId].resize(3);
            saveExit[sorterId][0] = nodes[virtualNodeId].type; // 分类器类型
            
            // 添加传送带线段
            Point parentPoint;
            if (parentRealId == n + m) {
                parentPoint = Point(entranceX, entranceY);
            } else if (parentRealId < n) {
                parentPoint = Point(proPosition[parentRealId].x, proPosition[parentRealId].y);
            } else {
                parentPoint = Point(sortPosition[parentRealId - n].x, sortPosition[parentRealId - n].y);
            }
            Point sorterPoint(sortPosition[sorterId].x, sortPosition[sorterId].y);
            currentLines.emplace_back(parentPoint, sorterPoint);
            
            // 递归处理左子树
            int leftChild = nodes[virtualNodeId].sons.first;
            if (leftChild != -1) {
                if (!recursiveBuild(leftChild, realNodeId, true)) {
                    return false;
                }
                int leftRealId = virtualToReal[leftChild];
                saveExit[sorterId][1] = (leftRealId < n) ? leftRealId : (leftRealId);
            } else {
                saveExit[sorterId][1] = -1;
            }
            
            // 递归处理右子树
            int rightChild = nodes[virtualNodeId].sons.second;
            if (rightChild != -1) {
                if (!recursiveBuild(rightChild, realNodeId, false)) {
                    return false;
                }
                int rightRealId = virtualToReal[rightChild];
                saveExit[sorterId][2] = (rightRealId < n) ? rightRealId : (rightRealId);
            } else {
                saveExit[sorterId][2] = -1;
            }
            
            return true;
        }
    }

    // 评估当前构建的真实树得分
    double evaluateCurrentTree() {
        vector<int> currentType(n);
        for (int i = 0; i < n; ++ i) {
            currentType[i] = processorTypeMap[i];
        }
        return getTotalScore(saveExit, currentType);
    }

    bool buildRealTree(int startTime1, int runTime) {
        // 初始化数据结构
        int virtualRoot = idx - 1; // 虚拟树的根节点
        double bestCurrentScore = -1;
        
        // 多次尝试构建，选择最佳结果
        while ((runtime() - startTime1) < runTime) {
            virtualToReal.assign(NM, -1);
            usedProcessors.assign(n, false);
            usedSorters.assign(m, false);
            currentLines.clear();
            saveExit.assign(m, {-1});
            processorTypeMap.clear();
            
            // 从根节点开始构建
            bool success = recursiveBuild(virtualRoot, n + m, false); // n + m 表示入口
            if (success) {
                // 确保所有处理器都被使用
                bool allUsed = true;
                // for (bool used : usedProcessors) {
                //     if (!used) {
                //         allUsed = false;
                //         break;
                //     }
                // }
                if (allUsed) {
                    // 计算当前树的得分
                    cerr << "尝试 " <<  "成功" << endl;
                    bestCurrentScore = 1;
                    // 更新最佳结果
                    saveBestExit = saveExit;
                    // 保存处理器类型分配
                    saveBestType.assign(n, -1);
                    vector<int> remainType;
                    vector<bool> useType(n, 0);
                    for (int i = 0; i < n; ++ i) {
                        if (processorTypeMap.find(i) != processorTypeMap.end()) {
                            saveBestType[i] = processorTypeMap[i];
                            useType[processorTypeMap[i]] = true;
                        } 
                    }
                    for (int i = 0; i < n; ++ i) {
                        if (!useType[i]) remainType.emplace_back(i);
                    }
                    int index = 0;
                    for (int& type : saveBestType) {
                        if (type == -1) {
                            type = remainType[index ++ ];
                        }
                    }
                        
                    // 保存入口连接
                    entranceLinkToSorter = virtualToReal[virtualRoot];
                    break;

                }
            }
        }
        
        if (bestCurrentScore >= 0) return true;
        return false;
        // 如果没有找到有效配置，使用简单配置
    }

    double getTotalScore(const vector<vector<int>>& saveExit, const vector<int>& saveType) {
        int root = n + m;
        vector<vector<double>> proGroups(m + 1, vector<double>(n, 0.0));
        vector<PII> edges(n + m + 1);
        edges[n + m] = make_pair(entranceLinkToSorter, -1);
        for (int i = 0; i < n; ++ i) proGroups[m][i] = 1.0;
        if (entranceLinkToSorter >= n) {
            for (int i = 0; i < n; ++ i) proGroups[entranceLinkToSorter - n][i] = 1.0;
        }
        
        for (int i = 0; i < m; ++ i) {
            int node = i + n;
            const auto& vec = saveExit[i];
            if (vec.size() > 1) {
                edges[node] = make_pair(vec[1], vec[2]);
            }
        }
        
        double resultScore = 0.0;
        queue<int> q;
        q.emplace(entranceLinkToSorter);
        
        while (!q.empty()) {
            int fa = q.front();
            q.pop();
            if (fa < 0 || fa >= n + m) continue;
            
            const auto& son = edges[fa];
            int leftExit = son.x, rightExit = son.y;
            int faSorterType = (fa >= n && fa - n < m && saveExit[fa - n].size() > 0) ? saveExit[fa - n][0] : -1;
            
            if (faSorterType == -1) continue;
            
            if (leftExit != -1) {
                if (leftExit < n) {
                    int type = saveType[leftExit];
                    if (type >= 0 && type < n && fa - n >= 0 && fa - n < m) {
                        resultScore += proGroups[fa - n][type] * probability[faSorterType][type];
                    }
                } else if (leftExit < n + m) {
                    for (int i = 0; i < n; ++ i) {
                        if (fa - n >= 0 && fa - n < m) {
                            proGroups[leftExit - n][i] = proGroups[fa - n][i] * probability[faSorterType][i];
                        }
                    }
                    q.emplace(leftExit);
                }
            }
            
            if (rightExit != -1) {
                if (rightExit < n) {
                    int type = saveType[rightExit];
                    if (type >= 0 && type < n && fa - n >= 0 && fa - n < m) {
                        resultScore += proGroups[fa - n][type] * (1 - probability[faSorterType][type]);
                    }
                } else if (rightExit < n + m) {
                    for (int i = 0; i < n; ++ i) {
                        if (fa - n >= 0 && fa - n < m) {
                            proGroups[rightExit - n][i] = proGroups[fa - n][i] * (1 - probability[faSorterType][i]);
                        }
                    }
                    q.emplace(rightExit);
                }
            }
        }
        
        return resultScore;
    }

    void output() {

        // 处理为放置的类型
        for (const int& type : saveBestType) printf("%d ", type);
        printf("\n");
        

        printf("%d\n", entranceLinkToSorter);
 
 
        
        for (const auto& vec : saveBestExit) {
            if (vec.size() == 1) {
                printf("%d\n", vec[0]);
            } else {
                printf("%d %d %d\n", vec[0], vec[1], vec[2]);
            }
        }
    }

    void mainLoop() {
        input();
        dealData();
        bool isOk = true;
        vector<int> ops;
        for (int i = n; i >= min(n, 7); -- i) ops.emplace_back(i);
        int allowTime = limit - 50;
        int sz = ops.size();
        for (int i = 0; i < sz;  ++ i) {
            int remainTime = (allowTime - runtime()) / (sz - i);
            int startTime1 = runtime();
            cerr << "第 " << i << "次构建" << "限制上线为 : " << ops[i] <<  "自己原本个数: " << n <<  endl;
            buildVirtualTree(ops[i], startTime1, remainTime);
            isOk = buildRealTree(startTime1, remainTime);
            if (isOk) break;
        }
        if (!isOk) {
            cerr << "使用备用配置" << endl;
            planning();
            planning2();
        }

        output();
        cerr << "结束时间: " << runtime() << endl;
    }
};

int main() {
    if(fopen("./data/0005.txt", "r") != nullptr) {
        freopen(("./data/0001.txt"), "r", stdin);
        freopen(("./output.txt"), "w", stdout);
    } else {
        cerr << "open file uncorrectly" << endl;
    }
    
    static Strategy strategy;
    strategy.mainLoop();
    return 0;
}
