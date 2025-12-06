#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <tuple>
#include <chrono>
#include <map>
#include <cassert>
#include <numeric>
using namespace std;
#define PII pair<int, int>
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}
constexpr int MAXN = 20;
constexpr int INF = 0x3f3f3f3f;
constexpr int LIMIT = 1800;
// 方向向量：上、下、左、右（对应U、D、L、R）
constexpr int dx[] = {-1, 1, 0, 0};
constexpr int dy[] = {0, 0, -1, 1};
// int dx[] = {0, -1, 0, 1};
// int dy[] = {-1, 0, 1, 0};
// constexpr char dir[] = {'U', 'D', 'L', 'R'};
const auto startTime = std::chrono::steady_clock::now();

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

template<class T> inline bool chmin(T &a,T b){
    if(a>b){
        a=b;
        return true;
    }
    return false;
}
template<class T> inline bool chmax(T &a,T b){
    if(a<b){
        a=b;
        return true;
    }
    return false;
}
inline int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

// BFS找最短路径，返回从start到end的路径（包含起点和终点）
vector<pair<int, int>> find_shortest_path(
    int N,
    const vector<vector<char>>& vWalls,  // 垂直墙：v[i][j]表示(i,j)与(i,j+1)间的墙
    const vector<vector<char>>& hWalls,  // 水平墙：h[i][j]表示(i,j)与(i+1,j)间的墙
    pair<int, int> start,
    pair<int, int> end
) {
    vector<vector<bool>> visited(N, vector<bool>(N, false));
    vector<vector<pair<int, int>>> parent(N, vector<pair<int, int>>(N, {-1, -1}));
    queue<pair<int, int>> q;

    int sx = start.first, sy = start.second;
    int ex = end.first, ey = end.second;

    q.push({sx, sy});
    visited[sx][sy] = true;
    vector<int> g1(4, 0);
    iota(g1.begin(), g1.end(), 0);
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        // 找到终点，结束BFS
        if (x == ex && y == ey) break;

        // // 尝试四个方向
        // // 随机打乱选择的方向
        
        fast_shuffle(g1.begin(), g1.end());
        for (int d : g1) {
            int nx = x + dx[d];
            int ny = y + dy[d];

            // 边界检查
            if (nx < 0 || nx >= N || ny < 0 || ny >= N) continue;
            if (visited[nx][ny]) continue;

            // 墙检查：根据移动方向判断是否有墙
            bool has_wall = false;

            int nd = -1;
            if (nx == x - 1) nd = 0;
            else if (nx == x + 1) nd = 1;
            else if (ny == y - 1) nd = 2;
            else if (ny == y + 1) nd = 3;;

            if (nd == 0) {  // 上移：检查水平墙h[x-1][y]（当前点x,y上方是x-1行）
                if (x == 0) has_wall = true;
                else if (hWalls[x-1][y] == '1') has_wall = true;
            } else if (nd == 1) {  // 下移：检查水平墙h[x][y]
                if (x == N-1) has_wall = true;
                else if (hWalls[x][y] == '1') has_wall = true;
            } else if (nd == 2) {  // 左移：检查垂直墙v[x][y-1]
                if (y == 0) has_wall = true;
                else if (vWalls[x][y-1] == '1') has_wall = true;
            } else if (nd == 3) {  // 右移：检查垂直墙v[x][y]
                if (y == N-1) has_wall = true;
                else if (vWalls[x][y] == '1') has_wall = true;
            }

            if (!has_wall) {
                visited[nx][ny] = true;
                parent[nx][ny] = {x, y};
                q.push({nx, ny});
            }
        }
    }

    // 回溯重建路径
    vector<pair<int, int>> path;
    pair<int, int> curr = {ex, ey};
    while (curr.first != -1 && curr.second != -1) {
        path.push_back(curr);
        curr = parent[curr.first][curr.second];
    }
    reverse(path.begin(), path.end());  // 反转后得到从start到end的路径
    return path;
}

struct Solution {
    int C{}, Q{}, M{};
    vector<tuple<int, int, int, int, char>> transitions;
    vector<vector<int>> initColors;
    int bestScore{INF};
    
    void output() {
        cout <<  C << " " << Q << " " << M << "\n";
        // 输出初始颜色网格
        int N = initColors.size();
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cout << initColors[i][j] << " ";
            }
            cout << "\n";
        }
        // 输出转移规则
        for (auto& t : transitions) {
            int c = get<0>(t);
            int q = get<1>(t);
            int A = get<2>(t);
            int S = get<3>(t);
            char D = get<4>(t);
            cout << c << " " << q << " " << A << " " << S << " " << D << "\n";
        }

    }

};

int main() {
    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0001.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    } else {
        cerr << "open file incorrectly" << endl;
    }
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1. 读取输入
    int N, K, T;
    cin >> N >> K >> T;

    // 读取垂直墙（N行，每行N-1个字符）
    vector<vector<char>> vWalls(N, vector<char>(N - 1));
    for (int i = 0; i < N; ++i) {
        string s;
        cin >> s;
        for (int j = 0; j < N-1; ++j) {
            vWalls[i][j] = s[j];
        }
    }

    // 读取水平墙（N-1行，每行N个字符）
    vector<vector<char>> hWalls(N-1, vector<char>(N));
    for (int i = 0; i < N-1; ++i) {
        string s;
        cin >> s;
        for (int j = 0; j < N; ++j) {
            hWalls[i][j] = s[j];
        }
    }

    // 读取K个目标点
    vector<pair<int, int>> destinations(K);
    for (int i = 0; i < K; ++i) {
        int x, y;
        cin >> x >> y;
        destinations[i] = {x, y};
    }

    // 2. 计算所有连续目标点之间的最短路径
    vector<pair<pair<int, int>, pair<int, int>>> steps;  // 存储每一步：(当前点, 下一个点)
    for (int i = 0; i < K-1; ++i) {
        auto start = destinations[i];
        auto end = destinations[i+1];
        auto path = find_shortest_path(N, vWalls, hWalls, start, end);
        // 提取路径中的每一步（当前点→下一个点）
        for (int j = 0; j < path.size()-1; ++j) {
            steps.emplace_back(path[j], path[j+1]);
        }
        
    }
    steps.emplace_back(steps.back().second, steps.back().first);
    
    int X = steps.size();  // 总步数（所有最短路径的总长度）
    Solution solution;
    double paramColor = 0.1;
    int iter = 1;
    int mod = 0;
    while (runtime() < LIMIT && paramColor <= 4) {

        vector<vector<int>> initColors(N, vector<int>(N, -1));  //代表未放置颜色
        vector<vector<int>> curColors(N, vector<int>(N, -1));  //代表未放置颜色
        vector<vector<int>> gridCnts(N, vector<int>(N, -1)); // 每个格子走过的次数
        vector<vector<tuple<int, int, char>>> query(MAXN * MAXN, vector<tuple<int, int, char>> (N * N * N, {-1, -1, 'x'})); // 记录（color,state) 对应的状态是否存在
        vector<int> colorCnts(MAXN * MAXN, -1); // 按照顺序使用的最大状态
        vector<vector<bool>> useStates(MAXN *MAXN, vector<bool>(N * N * N, 0));
        vector<vector<PII>> lastRules(N + 1, vector<PII>(N + 1));
        vector<vector<int>> makeInitColor(N + 1, vector<int>(N + 1, -1));
        vector<vector<int>> appear(MAXN * MAXN, vector<int>(N * N * N, -1));
        vector<int> appearColors(MAXN * MAXN, 0);
        for (int i = 0; i < N; ++ i) 
            for (int j = 0; j < N; ++ j) lastRules[i][j] = {-1, -1};
        int C = 0;             // 颜色数：仅用1种颜色（0）
        int Q = 0;             // 状态数：等于总步数（每个状态对应一步）
        int M = 0;             // 转移规则数：每个状态对应一个规则
        // 转移规则：(颜色c, 状态q) → (重涂颜色A, 新状态S, 方向D)
        vector<tuple<int, int, int, int, char>> transitions;
        int nextState = -1, prevState = -1;
        int maxState = -1, maxColor = -1;
        char lastStep = 'q';
        int lastState = 0;
        for (int s = 0; s < X; ++ s) {
            auto [currX, currY] = steps[s].first;
            auto [nextX, nextY] = steps[s].second;

            // 确定颜色
            int curColor = curColors[currX][currY];

            if (lastRules[currX][currY].first != -1 & curColor == -1) {
                // 选择一个颜色
                auto &hashRule = lastRules[currX][currY];
                int minCnt = INF;
                int targetColor = -1;
                for (int c = 0; c < max(maxColor, int(paramColor * N)); ++ c) {
                    for (int xs = 0;; ++ xs) {
                        if (!useStates[c][xs]) {
                            if (chmin(minCnt, xs)) {
                                targetColor = c;
                            } 
                            break;
                        }
                    }
                }
                get<0>(query[hashRule.first][hashRule.second]) = targetColor; // 为该规则已经找到了对应的转移颜色
                curColor = curColors[currX][currY] = targetColor;
            } 
            // 刷新一下当前格子和下一个格子
            if (lastRules[currX][currY].first != -1 && get<0>(query[lastRules[currX][currY].first][lastRules[currX][currY].second]) != -1) 
                curColors[currX][currY] = get<0>(query[lastRules[currX][currY].first][lastRules[currX][currY].second]);
            if (lastRules[nextX][nextY].first != -1 && get<0>(query[lastRules[nextX][nextY].first][lastRules[nextX][nextY].second]) != -1) 
                curColors[nextX][nextY] = get<0>(query[lastRules[nextX][nextY].first][lastRules[nextX][nextY].second]);  
            // 确定移动方向
            char D = 'S';  // 默认停留（不会触发）
            if (nextX == currX - 1) D = 'U';
            else if (nextX == currX + 1) D = 'D';
            else if (nextY == currY - 1) D = 'L';
            else if (nextY == currY + 1) D = 'R';
            int curState = -1;
            // 确定下一个状态（最后一步保持状态，避免超出范围）
            // 选择颜色
            if (initColors[currX][currY] == -1) {
                int targetColor = -1;

                // 搜可以转移状态最多的
                int maxTransState = 0;
                for (int c = 0; c < max(maxColor, int(paramColor * N)); ++ c) {
                    int transState = 0;
                    for (int state = 0; state <= maxState; ++ state) {
                        if (useStates[c][state]) {
                            if (get<2>(query[c][state]) == D && get<1>(query[c][state]) != -1) {
                                ++ transState;
                            }
                        }
                    }
                    if (chmax(maxTransState, transState)) targetColor = c;
                } 

                if (targetColor == -1 || prevState != -1) {
                    int minCnt = INF;
                    for (int c = 0; c < max(maxColor, int(paramColor * N)); ++ c) {
                        for (int xs = 0;; ++ xs) {
                            if (!useStates[c][xs]) {
                                if (chmin(minCnt, xs)) {
                                    targetColor = c;
                                } 
                                break;
                            }
                        }
                    }
                }
            
                assert(targetColor != -1);
                initColors[currX][currY] = curColors[currX][currY] = targetColor;
                // 在这一步确定颜色
                makeInitColor[currX][currY] = s;

            } 
            if (prevState == -1){
                int useState = -1, max1State = -1;
                // 我的目标就是保证下一个格子有个新的状态
                vector<PII> candidate;
                for (int state = 0; state <= maxState; ++ state) {
                    if (get<2>(query[curColors[currX][currY]][state]) == D && get<1>(query[curColors[currX][currY]][state]) != -1) {
                        candidate.emplace_back(state, get<1>(query[curColors[currX][currY]][state]));
                    }
                }
                reverse(candidate.begin(), candidate.end());
                if (!candidate.empty() && curColors[nextX][nextY] == -1) {
                    for (const auto& [cState, nState] : candidate) {
                        curState = cState, nextState =  nState;
                        // 看看nextState的最小颜色
                        bool isOk = false;
                        for (int c = 0; c <= max(maxColor, maxState); ++ c) {
                            if (!useStates[c][nextState]) {
                                isOk = true;
                                break;
                            }
                        }
                        if (!isOk) {
                            curState = nextState = -1;
                        } else {
                            break;
                        }                       
                    }

                }
                else if (!candidate.empty()) {
                    sort(candidate.begin(), candidate.end(), [&](const PII &p1, const PII &p2){ return p1.second < p2.second;});
                    for (const auto& [k, v] : candidate) {
                        if (!useStates[curColors[nextX][nextY]][v]) {
                            curState = k, nextState = v;
                            break;
                        }
                    }
                }
            }

            curColor  = curColors[currX][currY];
            bool need = true;
            if (curState != -1 && nextState != -1  && lastStep != 'q') {
                // 这里说明上一步是preStata == -1 走过来的
                // 计算上一步的格子
                int prevX = -1, prevY = -1;
                switch(lastStep) {
                    case 'U': prevX = currX + 1, prevY = currY; break;
                    case 'D': prevX = currX - 1, prevY = currY; break;
                    case 'L': prevY = currY + 1, prevX = currX; break;
                    case 'R': prevY = currY - 1, prevX = currX; break;
                };
                mod ++;
                for (int c = 0; c <= initColors[prevX][prevY] && mod % 3 == 0; ++ c) {
                    // cerr << prevX << " " << prevY << endl;
                    if (makeInitColor[prevX][prevY] == s - 1 && get<1>(query[c][lastState]) == curState && get<2>(query[c][lastState]) == lastStep) {
                        //  如果有颜色对应固定的转移状态并且方向与指定方向一致，那么这里可以复用
                            int tempColor = initColors[prevX][prevY];
                            appear[tempColor][lastState] = -1;
                            // 我修改上一个格子的颜色，有两种情况，可能是初始化状况，可能是被走过的
                            // 在什么时候情况下，我可以修改呢？ 初始化状态 and 在该格子的时候确定刷新规则的时候
                            // 这边我先强制看一下上一步的格子是第一次初始化，说明一定放置了新规则；
                            // 修改初始化颜色，并且删除上一步加的规则
                            appearColors[tempColor] --;
                            // 重置max
                            int tempMaxColor = 0;
                            for (int c1 = maxColor; c1 >= 0; -- c1) {
                                if (appearColors[c1] > 0) {
                                    tempMaxColor = c1;
                                    break;
                                } 

                            } 
                            maxColor = tempMaxColor;

                            initColors[prevX][prevY] = c;

                            curColors[prevX][prevY] = get<0>(query[c][lastState]);
                            lastRules[prevX][prevY] = {c, lastState};
                            // cerr << get<0>(transitions.back()) << " " << get<1>(transitions.back()) <<  " " << c << " " << lastState << " " << maxState << endl;
                            transitions.pop_back();
                            // 修改下
                            // cerr << get<0>(transitions.back()) << " " << get<1>(transitions.back()) << " " << c << " " << lastState << " " << maxState << endl;
                            // cerr << endl;
                            query[tempColor][lastState] = {-1, -1, 'x'};
                            need = false;

                            break;


                    }
                }
            }
            // 确定状态
            if (prevState == -1) {
            // 确定当前格子转移的状态（上一步转移状态未确定）
                if (curState == -1) {
                    // 这边需要遍历了
                    for (int i = 0;; ++ i) {
                        if (!useStates[curColor][i]) {
                            useStates[curColor][i] = true;
                            curState = i;
                            break;
                        }
                    }
                }
            } else {

                // 上一步转移状态确定
                if (useStates[curColor][nextState]) {
                    // 来到这一步有两种可能，1：未初始化 2 : 初始化了，但是转移规则颜色未确定
                    // 选择颜色
                    int targetColor = -1;
                    for (int i = 0; i <= max(maxColor, maxState); ++ i) {
                        if (!useStates[i][nextState]) {
                            targetColor = i;
                            break;
                        }
                    }
     
                    if (lastRules[currX][currY].first == -1) {
                        initColors[currX][currY] = curColors[currX][currY] = targetColor;
                    } else {
                        auto &hashRule = lastRules[currX][currY];
                        get<0>(query[hashRule.first][hashRule.second]) = targetColor;
                        curColors[currX][currY] = targetColor;
                    }
                    curColor = curColors[currX][currY];
                    
                    
     
                }
                useStates[curColor][nextState] = true;
                curState = nextState;
                nextState = -1;
            }
            if (!transitions.empty() && prevState == -1 && need) {
                get<3>(transitions.back()) = curState;
                int lastGridColor =  get<0>(transitions.back());
                int lastState =  get<1>(transitions.back());
                get<1>(query[lastGridColor][lastState]) = curState;
  
            }
            if (nextState == -1) {
                maxState = max(maxState, curState);
                maxColor = max(maxColor, curColor);
                appearColors[curColor] ++;
                transitions.emplace_back(curColor, curState, -1, -1, D);
                query[curColor][curState] = {-1, -1, D};
                curColors[currX][currY] = -1;
                prevState = -1;
                appear[curColor][curState] = s;
                
            } else {

                // 已经确认下一步了，所以当前转移策略明了（curColor, curState, nextColor, nextState, D)
                curColors[currX][currY] = get<0>(query[curColor][curState]);
                prevState = curState;
            }
            // 当前步的转移规则
            lastRules[currX][currY] = {curColor, curState};
            lastStep = D, lastState = curState;

        }
        get<3>(transitions.back()) = 0;
        // cerr << get<0>(query[8][4]) << " " << get<1>(query[8][4]) << " " << get<2>(query[8][4]) << endl;
        // 统计结果
        for (auto& transition : transitions) {
            int color = get<0>(transition), state = get<1>(transition), newColor = get<2>(transition);
            C = max(C, color);
            Q = max(Q, state);

            if (newColor == -1) {
 
                get<2>(transition) = (get<0>(query[color][state]) == -1 ? 0 : get<0>(query[color][state]));
            }
        }

        C += 1; Q += 1;
        M = transitions.size();
        // 重新初始化-1网格
        for (int i = 0; i < N; ++ i) {
            for (int j = 0; j < N; ++ j) {
                if (initColors[i][j] == -1) initColors[i][j] = 0;
            }
        }

        if (chmin(solution.bestScore, C + Q)) {
            solution.C = C;
            solution.Q = Q;
            solution.M = M;
            solution.initColors = move(initColors);
            solution.transitions = move(transitions);
            int score = C + Q;
            DATA(paramColor);
            DATA(score);
            
        }
        paramColor += 0.04;
        

    }
    solution.output();

    cerr << "endTime: " << runtime() << endl;

    return 0;
}