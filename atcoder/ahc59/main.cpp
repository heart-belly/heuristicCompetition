
#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <bits/stdc++.h>
using namespace std;

using namespace chrono;


#define PII pair<int, int>
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}

constexpr int N = 20;
constexpr int M = 200;
constexpr double TIME_LIMIT = 1850;

constexpr int dx[4] = {-1, 1, 0, 0};
constexpr int dy[4] = {0, 0, -1, 1};
constexpr char dirC[4] = {'U', 'D', 'L', 'R'};

const auto startTime = std::chrono::steady_clock::now();

inline int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}


// 模拟高斯分布
inline int xor128() {
	static int x = 123456789, y = 362436069, z = 521288629, w = 88675123;
	int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}
// 生成0~1之间的随机数
inline double getRandom() {
    double _rand = (double)(xor128()) / (double)INT32_MAX;
    return _rand;
} 
// 生成高斯分布
inline double genGaussian() {
    double X =getRandom(), Y = getRandom(), Z = sqrt(-2.0 * log(X)) * cos(2.0 * acos(-1.0) * Y);
    return Z; 
}

constexpr int GaussianSize = 1000000;
vector<double> _Gaussian(GaussianSize);
uint64_t s = 0x123456789abcdef0ULL;
inline uint64_t xor64() {
uint64_t x = s;
// 移位操作：增强随机性
x ^= x >> 12;
x ^= x << 25;
x ^= x >> 27;
s = x;
// 乘以大质数混淆结果，进一步降低随机性偏差
return x * 2685821657736338717ULL;
}
// 快速洗牌（随机高效打乱数组）
// 模板函数：快速洗牌（基于xor64()随机数，高效打乱容器元素）
template <class RandomIt>
inline void fast_shuffle(RandomIt first, RandomIt last) {
    // 计算容器中元素的数量
    ptrdiff_t n = last - first;
    // 元素数量<=1时无需洗牌
    if (n <= 1) return;
    // 从后往前洗牌（每轮交换两个元素，减少随机数调用次数）
    for (ptrdiff_t i = n - 1 - xor64() % 2; i > 0; i -= 2) {
        // 生成0~i的随机索引j
        uint32_t j = xor64() % (i + 1);
        // 交换第i个和第j个元素
        swap(first[i], first[(ptrdiff_t)j]);
    }
}
struct Pos { 
    int x, y; 
    bool operator==(const Pos &l) const {return x == l.x && y == l.y;}
    bool operator!=(const Pos &l) const {return x != l.x || y != l.y;}
};
int grid[N][N];
Pos series[M][2];
int dist[N * N][N * N];
int idx[M];


inline bool inGrid(int x, int y) {
    return x >= 0 && x < N && y >= 0 && y < N;
}


vector<char> bfsPath(Pos s, Pos t) {
    if (s == t) return {};
    
    vector<vector<int>> vis(N, vector<int>(N, 0));
    vector<vector<Pos>> pre(N, vector<Pos>(N, {-1, -1}));
    queue<Pos> q;
    q.push(s);
    vis[s.x][s.y] = 1;

    while (!q.empty()) {
        Pos p = q.front(); q.pop();
        if (p == t) break;
        for (int k = 0; k < 4; k++) {
            int nx = p.x + dx[k], ny = p.y + dy[k];
            if (inGrid(nx, ny) && !vis[nx][ny]) {
                vis[nx][ny] = 1;
                pre[nx][ny] = p;
                q.push({nx, ny});
            }
        }
    }

    vector<char> path;
    Pos p = t;
    while (!(p == s)) {
        Pos q = pre[p.x][p.y];
        if (q.x == -1) break;
        for (int k = 0; k < 4; k++) {
            if (q.x + dx[k] == p.x && q.y + dy[k] == p.y) {
                path.push_back(dirC[k]);
                break;
            }
        }
        p = q;
    }
    reverse(path.begin(), path.end());
    return path;
}

struct Solution {
    vector<int> solSeries;
    int moveCount = 0;
    int score = 0;
    Solution(){}
    Solution(vector<int> _solSeries, int _moveCount) : solSeries(_solSeries), moveCount(_moveCount){}
    void calcScore() {
        score = N * N +  2 * N * N * N - moveCount;
        
    }
    void output() {
        vector<char> ans;
        Pos cur{0, 0};
        for (int i = 0; i < 400; ++ i) {
            int v = solSeries[i] / 2, r = solSeries[i] % 2;
            Pos target{series[v][r]};
            vector<char> temp = bfsPath(cur, target);
            ans.insert(ans.end(), temp.begin(), temp.end());
            ans.emplace_back('Z');
            cur = move(target);
            
        }
        for (char c : ans) printf("%c\n", c);
        
    }
};

vector<int> strategyDirectPair(int &totalOp) {
    vector<int> strategySeries;
    
    bool hasCard[N][N];
    memset(hasCard, true, sizeof(hasCard));
    
    stack<int> deck;
    set<int> removed;
    Pos cur = {0, 0};
    int x = 0;
    while ((int)removed.size() < M) {
        // 当前目标卡计算
        Pos target = {-1, -1};
        int targetVal = -1;

        // 若手中有卡，则寻找配对位置
        if (!deck.empty()) {
            int curVal = deck.top();
            auto &vec = series[curVal];
            for (auto &p : vec) {
                if (hasCard[p.x][p.y]) { // 当前配对位置还有卡
                    target = p;
                    targetVal = curVal;
                    break;
                }
            }
        }
        // 否则寻找尚未消除卡号中最近的一张卡
        if (targetVal == -1) {
            int bestDist = 1e9;
            for (int point = 0; point < M; ++ point) {
                if (removed.count(point)) continue;
                for (auto &p : series[point]) {
                    if (!hasCard[p.x][p.y]) continue;
                    int dist = abs(cur.x - p.x) + abs(cur.y - p.y);
                    if (dist < bestDist) {
                        bestDist = dist; target = p; targetVal = point;
                    }
                }
            }
        }
        if (targetVal == -1) break; // 没有目标可达
        int v1 = grid[cur.x][cur.y], v2 = grid[target.x][target.y];
        int hashAIdx = 2 * v1, hashBIdx = 2 * v2;
        if (cur != series[v1][0]) hashAIdx += 1;
        if (target != series[v2][0]) hashBIdx += 1;
        totalOp += dist[hashAIdx][hashBIdx];
        
        cur = target;

        // 操作：拾取卡（Z）
        if (hasCard[cur.x][cur.y]) {
            deck.push(grid[cur.x][cur.y]);
            hasCard[cur.x][cur.y] = false;

            // 匹配卡的顺序
            int v = grid[cur.x][cur.y];
            int hashIdx = v * 2;
            if (cur != series[v][0]) hashIdx += 1; 
            strategySeries.emplace_back(hashIdx);

            // 检查是否匹配消除
            if (deck.size() >= 2) {
                int a = deck.top(); deck.pop();
                int b = deck.top(); deck.pop();
                if (a == b) {
                    removed.insert(a);
                } else {
                    deck.push(b);
                    deck.push(a);
                }
            }
        }

    }
    return strategySeries;
}

int main() {
    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0010.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    }
    
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int v;
            cin >> v;
            grid[i][j] = v;
            series[v][idx[v] ++ ]= Pos{i, j};
        }
    }
    // 求出所以点之间的距离
    for (int i = 0; i < M; ++ i) {
        int idx1 = 2 * i + 0, idx2 = 2 * i + 1;
        dist[idx1][idx2] = dist[idx2][idx1] = abs(series[i][0].x - series[i][1].x) + abs(series[i][0].y - series[i][1].y);
        for (int j = 0; j < 2; ++ j) {
            int temp = 2 * i + j;
            for (int k = 0; k < M; ++ k) {
                if (k == i) continue;
                int idx3 = 2 * k + 0, idx4 = 2 * k + 1;
                dist[temp][idx3] = dist[idx3][temp] = abs(series[i][j].x - series[k][0].x) + abs(series[i][j].y - series[k][0].y);
                dist[temp][idx4] = dist[idx4][temp] = abs(series[i][j].x - series[k][1].x) + abs(series[i][j].y - series[k][1].y);
            }
        }
    }

    int move = 0;
    auto ops = strategyDirectPair(move);
    Solution initSol = Solution({}, move);
    initSol.calcScore();

    // 构建括号结构
    struct ParenthesesMatch{
        vector<PII> matchSeries;
        vector<int> pointToSeries;
        int move;

        ParenthesesMatch(){}
        ParenthesesMatch(const vector<int> &ops, int _move) : matchSeries(N * N), pointToSeries(N * N), move(_move){
            int sz = ops.size();
            vector<int> adds(M, 0);
            for (int i = 0; i < 400; ++ i) {
                int v = ops[i] / 2;
                matchSeries[i] = make_pair(ops[i], adds[v]);
                pointToSeries[ops[i]] = i;
                ++ adds[v];
            }
        }

    };
    auto calcScore = [&](const vector<PII> groups) -> int {
        int sz = groups.size();
        assert(sz == 400);
        int move = 0;
        Pos start{0, 0};
        int v = groups[0].first / 2, r = groups[0].first % 2;
        move += series[v][r].x + series[v][r].y;
        for (int i = 1; i < 400; ++ i) {
            move += dist[groups[i].first][groups[i - 1].first];
        }
        return move;
    };
    ParenthesesMatch initMatch{ops, move};
    move = calcScore(initMatch.matchSeries);
    initMatch.move = move;
    int initMove = move;
    DATA(initMove);

    auto SA = [&]() {
        ParenthesesMatch best = initMatch;
        double startTemp = 10;
        double endTemp = 0.0001;
        double TIME_INIT = runtime();
        double TIME_LIMIT = 1980.0 - TIME_INIT;
        double timeNow = 0.0;
        double temp = 0.0;
        double progress = 0.0;
        double gw = 0.0; // 扰动权值
        int ls = 2;
        int loopCnt = 0;
        int improveCnt = 0;

        vector<ParenthesesMatch> current(ls, initMatch);
        
                
        while (1) {
            if (loopCnt % 1000 == 0) {
                timeNow = runtime() - TIME_INIT;
                if (timeNow > TIME_LIMIT)
                    break;
                progress = pow(timeNow / TIME_LIMIT, 0.7); // 采用了非线性收敛（0.7这里就可以得到早期收敛慢，后期收敛快）
                temp = startTemp + (endTemp - startTemp) * progress;
                gw = (1 - progress) * 400;
            }

            ++ loopCnt;
            for (int g = 0; g < ls; ++ g) {
                // 生成邻居解
                int oldDist = 0, newDist = 0; 
                int type = xor128() % 3;
                if (type == 0) {
                    // 调整同卡片左右括号类型
                    int id = xor128() % M;
                    int hashA = id * 2, hashB = 2 * id + 1;
                    for (int i = 0; i < 2; ++ i) {
                        // 计算交换之前的距离
                        int index0 = current[g].pointToSeries[hashA + i];
                        if (index0 == 0) {
                            oldDist += series[id][i].x + series[id][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index0 - 1].first;
                            oldDist += dist[hashA + i][preHash];
                        }
                        if (index0 != 399) {
                            int nextHash = current[g].matchSeries[index0 + 1].first;
                            oldDist += dist[hashA + i][nextHash];
                        }
                    }
                    if (abs(current[g].pointToSeries[hashA] - current[g].pointToSeries[hashB]) == 1) oldDist -= dist[hashA][hashB];
                    swap(current[g].pointToSeries[hashA],  current[g].pointToSeries[hashB]);
                    current[g].matchSeries[current[g].pointToSeries[hashA]].first = hashA;
                    current[g].matchSeries[current[g].pointToSeries[hashB]].first = hashB;
                    for (int i = 0; i < 2; ++ i) {
                        // 计算交换之后的距离
                        int index0 = current[g].pointToSeries[hashA + i];
                        if (index0 == 0) {
                            newDist += series[id][i].x + series[id][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index0 - 1].first;
                            newDist += dist[hashA + i][preHash];
                        }
                        if (index0 != 399) {
                            int nextHash = current[g].matchSeries[index0 + 1].first;
                            newDist += dist[hashA + i][nextHash];
                        }
                    }
                    if (abs(current[g].pointToSeries[hashA] - current[g].pointToSeries[hashB]) == 1) newDist -= dist[hashA][hashB];
                    int delta = oldDist - newDist;
                    if (delta > 0 || getRandom() < exp(delta / temp)) {
                        // 接受
                        current[g].move -= (delta);
                        if (current[g].move < best.move) {
                            best = current[g];
                            ++ improveCnt;
                            cerr << "t1 " << "impove: " << best.move << endl;
                        }
        
                    } else {
                        // 拒绝
                        swap(current[g].pointToSeries[hashA],  current[g].pointToSeries[hashB]);
                        current[g].matchSeries[current[g].pointToSeries[hashA]].first = hashA;
                        current[g].matchSeries[current[g].pointToSeries[hashB]].first = hashB;
                    }
                } else if (type == 1) {
                    // 随机交换一对卡牌
                    int id1 = xor128() % M, id2 = xor128() % M;
                    if (id1 == id2) continue;
                    int hash1A = 2 * id1, hash1B = hash1A + 1;
                    int hash2A = 2 * id2, hash2B = hash2A + 1;
                    vector<int> gs{current[g].pointToSeries[hash1A], current[g].pointToSeries[hash1B], current[g].pointToSeries[hash2A], current[g].pointToSeries[hash2B]};
                    sort(gs.begin(), gs.end());
                    for (int i = 1; i < 4; ++ i) {
                        if (gs[i] - gs[i - 1] == 1) oldDist -= dist[current[g].matchSeries[gs[i]].first][current[g].matchSeries[gs[i - 1]].first];
                    }
                    // 计算交换之前的距离
                    for (int i = 0; i < 2; ++ i) {
                        // 计算交换之前的距离
                        int index0 = current[g].pointToSeries[hash1A + i];
                        if (index0 == 0) {
                            oldDist += series[id1][i].x + series[id1][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index0 - 1].first;
                            oldDist += dist[hash1A + i][preHash];
                        }
                        if (index0 != 399) {
                            int nextHash = current[g].matchSeries[index0 + 1].first;
                            oldDist += dist[hash1A + i][nextHash];
                        }
                        int index1 = current[g].pointToSeries[hash2A + i];
                        if (index1 == 0) {
                            oldDist += series[id2][i].x + series[id2][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index1 - 1].first;
                            oldDist += dist[hash2A + i][preHash];
                        }
                        if (index1 != 399) {
                            int nextHash = current[g].matchSeries[index1 + 1].first;
                            oldDist += dist[hash2A + i][nextHash];
                        }
                        
                    }
                    // 左括号交换左括号，右括号交换右括号
                    int left1 = hash1A, right1 = hash1A + 1;
                    if (current[g].matchSeries[current[g].pointToSeries[left1]].second != 0) swap(left1, right1);
                    int left2 = hash2A, right2 = hash2A + 1;
                    if (current[g].matchSeries[current[g].pointToSeries[left2]].second != 0) swap(left2, right2);

                    // 开始交换
                    swap(current[g].pointToSeries[left1],  current[g].pointToSeries[left2]);
                    current[g].matchSeries[current[g].pointToSeries[left1]].first = left1;
                    current[g].matchSeries[current[g].pointToSeries[left2]].first = left2;
                    swap(current[g].pointToSeries[right1],  current[g].pointToSeries[right2]);
                    current[g].matchSeries[current[g].pointToSeries[right1]].first = right1;
                    current[g].matchSeries[current[g].pointToSeries[right2]].first = right2;

                    vector<int> gs1{current[g].pointToSeries[hash1A], current[g].pointToSeries[hash1B], current[g].pointToSeries[hash2A], current[g].pointToSeries[hash2B]};
                    sort(gs1.begin(), gs1.end());
                    for (int i = 1; i < 4; ++ i) {
                        if (gs1[i] - gs1[i - 1] == 1) newDist -= dist[current[g].matchSeries[gs1[i]].first][current[g].matchSeries[gs1[i - 1]].first];
                    }
                    
                    for (int i = 0; i < 2; ++ i) {
                        // 计算交换之前的距离
                        int index0 = current[g].pointToSeries[hash1A + i];
                        if (index0 == 0) {
                            newDist += series[id1][i].x + series[id1][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index0 - 1].first;
                            newDist += dist[hash1A + i][preHash];
                        }
                        if (index0 != 399) {
                            int nextHash = current[g].matchSeries[index0 + 1].first;
                            newDist += dist[hash1A + i][nextHash];
                        }
                        int index1 = current[g].pointToSeries[hash2A + i];
                        if (index1 == 0) {
                            newDist += series[id2][i].x + series[id2][i].y;
                        } else {
                            int preHash = current[g].matchSeries[index1 - 1].first;
                            newDist += dist[hash2A + i][preHash];
                        }
                        if (index1 != 399) {
                            int nextHash = current[g].matchSeries[index1 + 1].first;
                            newDist += dist[hash2A + i][nextHash];
                        }
                        
                    }

                    int delta = oldDist - newDist;
                    if (delta > 0  || getRandom() < exp(delta / temp)) {
                        // 接受
                        current[g].move -= (delta);
                        if (current[g].move < best.move) {
                            best = current[g];
                            ++ improveCnt;
                            cerr << "t2 " << loopCnt << " impove: " << best.move << endl;

                        }
        
                    } else {
                        // 拒绝
                        swap(current[g].pointToSeries[left1],  current[g].pointToSeries[left2]);
                        current[g].matchSeries[current[g].pointToSeries[left1]].first = left1;
                        current[g].matchSeries[current[g].pointToSeries[left2]].first = left2;
                        swap(current[g].pointToSeries[right1],  current[g].pointToSeries[right2]);
                        current[g].matchSeries[current[g].pointToSeries[right1]].first = right1;
                        current[g].matchSeries[current[g].pointToSeries[right2]].first = right2;
                    }
                } else if (type == 2) {
                    // 整体移动
                    int id = xor128() % M;
                    int hashA = id * 2, hashB = hashA + 1;
                    int left = hashA, right = hashB;
                    int isR = 0;
                    if (current[g].matchSeries[current[g].pointToSeries[left]].second != 0) {
                        swap(left, right);
                        isR = 1;
                    }
                    
                    int leftPos = current[g].pointToSeries[left], rightPos = current[g].pointToSeries[right];

                    int op = xor128() % 1;
                    if (op == 0 && leftPos == 0) continue;
                    if (op == 1 && rightPos == 399) continue;
                    // 计算交换前的距离
                    if (leftPos == 0) {
                        oldDist += series[id][isR].x + series[id][isR].y;
                    } else {
                        int preHash = current[g].matchSeries[leftPos - 1].first;
                        oldDist += dist[left][preHash];
                    }
                    if (rightPos != 399) {
                        int nextHash = current[g].matchSeries[rightPos + 1].first;
                        oldDist += dist[right][nextHash];   
                    }

                    
                    if (op == 0) {
                        // 向前移动
                        int insertPos = xor128() % leftPos;
                        int ida = current[g].matchSeries[insertPos].first / 2, idb = current[g].matchSeries[insertPos].first % 2;
                        if (insertPos == 0) {
                            oldDist += series[ida][idb].x + series[ida][idb].y;
                        } else {
                            int preHash = current[g].matchSeries[insertPos - 1].first;
                            oldDist += dist[current[g].matchSeries[insertPos].first][preHash];
                        }
                        if (insertPos == 0) {
                            newDist += series[id][isR].x + series[id][isR].y;
                        } else {
                            int preHash = current[g].matchSeries[insertPos - 1].first;
                            newDist += dist[left][preHash];
                        }
                        newDist += dist[right][current[g].matchSeries[insertPos].first];
                        if (rightPos != 399) {
                            int idp = current[g].matchSeries[leftPos - 1].first, idn = current[g].matchSeries[rightPos + 1].first;
                            newDist += dist[idp][idn]; 
                        }

                        int delta = oldDist - newDist;

                        if (delta > 0 || getRandom() < exp(delta / temp)) {
                            // 接受
                            current[g].move -= (delta);
                            if (current[g].move < best.move) {
                                best = current[g];
                                ++ improveCnt;
                                cerr << "t3 " <<  loopCnt << " impove: " << best.move << endl;

                            }
                            vector<PII> backup(rightPos - insertPos + 1);

                            int temp = insertPos;
                            for (int i = insertPos; i <= rightPos; ++ i) backup[i - insertPos] = current[g].matchSeries[i];
                            int len = leftPos - insertPos;
                            for (int i = leftPos, j = 0; i <= rightPos; ++ i, ++ j) {
                                current[g].matchSeries[insertPos ++ ] = backup[j + len];
                            }
                            for (int j = 0; j < len; ++ j) {
                                current[g].matchSeries[insertPos ++ ] = backup[j];
                            }
                            // 更新
                            for (int i = temp; i <= rightPos; ++ i) {
                                current[g].pointToSeries[current[g].matchSeries[i].first] = i;
                            }
                        } 

                    } 
                    
                    
                }
            }

            
        }
        DATA(improveCnt);
        DATA(loopCnt);
        int improveMove = calcScore(best.matchSeries);
        cerr << improveMove << endl;
        return best;
    };

    ParenthesesMatch best = SA();
    // 构造结果
    for (int i = 0; i < 400; ++ i) {
        ops[i] =  best.matchSeries[i].first;
    }
    move = best.move;
    Solution sol = Solution(ops,move);
    sol.calcScore();

    DATA(sol.score);
    // // 输出结果
    sol.output();
    // for (char c : ops) cout << c << "\n";
    

    int endTime = runtime();
    DATA(endTime);
    
    return 0;
}
