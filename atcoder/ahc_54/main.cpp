#pragma GCC optimize "Ofast,omit-frame-pointer,inline,fast-math,unroll-all-loops,tree-loop-vectorize,tree-slp-vectorize"
#pragma GCC option("arch=native","tune=native","no-zero-upper")
#pragma GCC optimize(3)
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <climits>
#include <chrono>
using namespace std;

#define endl '\n'
#define INF 0x3f3f3f3f
typedef pair<int, int> PII;
typedef pair<int, pair<int, int>> PIPII;
constexpr int LIMIT = 1800;
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
    //  int next(int x) {return ((long long)rand() * x) >> 32;}
     int next(int x) {return rand() % x;}
     int next(int a, int b) {return a + (rand() % (b - a));}
    //  int next(int a, int b) {return a + ((long long) rand() * (b - a)) >> 32;}
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 
static RNG rng;
const auto startTime = std::chrono::steady_clock::now();

 int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}
// 全局状态管理（包含本地测试所需的q列表）
struct ForestState {
    int N, ti, tj, si, sj;
    vector<vector<char>> originalMap;  // 原始地图（.或T）
    vector<vector<bool>> isTree;       // 当前树的状态（初始树+Treant）
    vector<vector<bool>> isRevealed;   // 已揭示区域
    vector<pair<int, int>> qList;      // 目的地候选列表（本地测试特有）
    vector<PII> allowPutList;
    int qStr;                          // 当前q列表的遍历位置
    const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}}; // 上→下→左→右
    const int reverseDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};


};

ForestState state;

// 曼哈顿距离
inline int manhattan(int x1, int y1, int x2, int y2) {
    return abs(x1-x2) + abs(y1-y2);
}

// BFS计算从起点到所有点的距离（基于临时地图）
vector<vector<int>> bfsDist(int sx, int sy, const vector<vector<bool>>& tempMap, bool isRevealed = false) {
    int N = state.N;
    vector<vector<int>> dist(N, vector<int>(N, -1));

    queue<pair<int, int>> q;
    if (tempMap[sx][sy]) {
        dist[sx][sy] = 0;
        q.emplace(sx, sy);
    }
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        auto& dir = ( isRevealed ? state.reverseDirs : state.dirs);
        for (const auto& d : dir) {
            int nx = x + d[0], ny = y + d[1];
            if (nx >= 0 && nx < N && ny >= 0 && ny < N && tempMap[nx][ny] && dist[nx][ny] == -1) {
                dist[nx][ny] = dist[x][y] + 1;
                q.emplace(nx, ny);
            }
        }
    }
    return dist;
}

// aStar计算从起点到所有点的距离（基于临时地图）
int aStar(int sx, int sy, int ex, int ey, const vector<vector<bool>>& tempMap, vector<PII>& path) {
    int N = state.N;
    vector<vector<int>> dist(N, vector<int>(N, -1));
    vector<vector<bool>> st(N, vector<bool>(N, 0));
    vector<vector<PII>> pres(N, vector<PII>(N, {-1, -1}));
    priority_queue<PIPII, vector<PIPII>, greater<PIPII>> heap;

    if (tempMap[sx][sy]) {
        dist[sx][sy] = 0;
        heap.push({0, {sx, sy}});
    }

    while (!heap.empty()) {
        auto [dis, point] = heap.top(); heap.pop();
        int a = point.first, b = point.second;
        if (st[a][b]) continue;
        st[a][b] = true;

        if (a == ex && b == ey) {
            int tempx = ex, tempy = ey;
            path.clear();
            while (tempx != -1 ) {
                path.emplace_back(tempx, tempy);

                int curX = pres[tempx][tempy].first, curY = pres[tempx][tempy].second;
                tempx = curX, tempy = curY;
                

            }
            reverse(path.begin(), path.end());
            return path.size();
        }

        for (const auto& d : state.dirs) {
            int nx = a + d[0], ny = b + d[1];
            if (nx >= 0 && nx < N && ny >= 0 && ny < N && tempMap[nx][ny] && dist[nx][ny] == -1) {
                if (st[nx][ny]) continue;
                if (dist[nx][ny] == -1 || dist[nx][ny] < dis + 1) {
                    dist[nx][ny] = dis + 1;
                    pres[nx][ny] = make_pair(a, b);
                    heap.push({dist[nx][ny] + manhattan(nx, ny, ex, ey), {nx, ny}});
                }
            }
        }
    }
    return -1;
}
// 检查入口到AA花的路径是否存在
bool check_path() {
    int N = state.N;
    vector<vector<bool>> vis(N, vector<bool>(N, false));
    queue<pair<int, int>> q;
    if (state.isTree[state.si][state.sj]) return false;
    q.emplace(state.si, state.sj);
    vis[state.si][state.sj] = true;
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        if (x == state.ti && y == state.tj) return true;
        for (const auto& d : state.dirs) {
            int nx = x + d[0], ny = y + d[1];
            if (nx>=0 && nx<N && ny>=0 && ny<N && !state.isTree[nx][ny] && !vis[nx][ny]) {
                vis[nx][ny] = true;
                q.emplace(nx, ny);
            }
        }
    }
    return false;
}

// 模拟冒险者探索（更新已揭示区域）
int explore(int pi, int pj) {
    int N = state.N;
    vector<pair<int, int>> newRevealed;
    // 向四个方向探索
    for (const auto& d : state.dirs) {
        int x = pi, y = pj;
        while (true) {
            x += d[0];
            y += d[1];
            // 超出网格或遇到树，停止探索
            if (x < 0 || x >= N || y < 0 || y >= N) break;
            // 未揭示的单元格，标记为已揭示
            if (!state.isRevealed[x][y]) {
                // auto iter = remove(state.allowPutList.begin(), state.allowPutList.end(), PII{x, y});
                // state.allowPutList.erase(iter);
                state.isRevealed[x][y] = true;
                newRevealed.emplace_back(x, y);
            }
            if (state.isTree[x][y]) {
                break;
            }
        }
    }
    return newRevealed.size(); // 返回新增揭示的数量
}

// 本地测试：从q列表选择目的地（按顺序找第一个符合条件的）
pair<int, int> opDestination(int pi, int pj, bool aaRevealed) {
    int N = state.N;
    // 临时地图：未揭示视为空地
    vector<vector<bool>> tempMap(N, vector<bool>(N));
    for (int i=0; i<N; ++i) {
        for (int j=0; j<N; ++j) {
            tempMap[i][j] = !state.isTree[i][j] || !state.isRevealed[i][j];
        }
    }
        
    // 计算当前位置可达区域
    auto dist = bfsDist(pi, pj, tempMap);
   
    // 若AA花已揭示，优先选AA花
    if (aaRevealed && dist[state.ti][state.tj] != -1) {
        return {state.ti, state.tj};
    }
    // 从q列表找第一个符合条件的（未揭示、可达、初始为空）
    for (int k = 0; k < state.qList.size(); ++ k) {
        auto [x, y] = state.qList[k];
        if (state.isRevealed[x][y]) continue; 
        if (dist[x][y] == -1) continue; 
        return {x, y};
    }
    return {-1, -1};
}

// 模拟冒险者移动（按最短路径+方向优先级）
pair<int, int> moveing(int pi, int pj, int dest_x, int dest_y) {
    if (pi == dest_x && pj == dest_y) return {pi, pj};
    
    // 临时地图
    vector<vector<bool>> tempMap(state.N, vector<bool>(state.N));
    for (int i=0; i<state.N; ++i) {
        for (int j=0; j<state.N; ++j) {
            tempMap[i][j] = !state.isTree[i][j] || !state.isRevealed[i][j];
        }
    }
    // 计算到目的地的最短距离
    auto dist = bfsDist(dest_x, dest_y, tempMap, true);
    if (dist[pi][pj] == -1) return {pi, pj}; 
    // 按方向优先级找下一步
    for (const auto& d : state.dirs) {
        int nx = pi + d[0], ny = pj + d[1];
        if (nx < 0 || nx >= state.N || ny < 0 || ny >= state.N) continue;
        if (tempMap[nx][ny] && dist[nx][ny] == dist[pi][pj] - 1) {
            return {nx, ny};
        }
    }
    return {pi, pj}; 
}

// 生成Treant放置位置
vector<pair<int, int>> generate_treants(int curr_x, int curr_y) {
    int N = state.N;
    vector<pair<int, int>> candidates;
    
    // 收集候选位置
    for (int i = 0; i < N; ++ i) {
        for (int j = 0; j< N; ++ j) {
            if (!state.isRevealed[i][j] && state.originalMap[i][j] == '.' && !state.isTree[i][j]) {
                candidates.emplace_back(i, j);
            }
        }
    }
    vector<vector<bool>> tempMap(state.N, vector<bool>(state.N));
    for (int i=0; i<state.N; ++i) {
        for (int j=0; j<state.N; ++j) {
            tempMap[i][j] = !state.isTree[i][j] || !state.isRevealed[i][j];
        }
    }
    // 触发Cutoff条件
    int revealedCnt = 0;
    for (int i = 0; i < N; ++ i) 
        for (int j = 0; j < N; ++ j) 
            if (state.isRevealed[i][j]) revealedCnt ++;

    
    int op = rng.next(0, 3);
    if (op == 0) {
        // 评分排序（优先封锁近路）
        sort(candidates.begin(), candidates.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
            int score_a = (50 - manhattan(a.first, a.second, state.ti, state.tj));
            int score_b = (50 - manhattan(b.first, b.second, state.ti, state.tj));     
            return score_a > score_b;
        });
    } else if (op == 1){
        sort(candidates.begin(), candidates.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
            int score_a = (100 - manhattan(a.first, a.second, state.ti, state.tj))
                        + (100 - manhattan(a.first, a.second, curr_x, curr_y));
            int score_b = (100 - manhattan(b.first, b.second, state.ti, state.tj))
                        + (100 - manhattan(b.first, b.second, curr_x, curr_y));
            return score_a > score_b;
        });
    } else {
        sort(candidates.begin(), candidates.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
            int score_a = (50 - manhattan(a.first, a.second, curr_x, curr_y));
            int score_b = (50 - manhattan(b.first, b.second, curr_x, curr_y));
            return score_a > score_b;
        });
    }

    // 放置1个Treant（确保路径存在）
    vector<pair<int, int>> selected;
    if (candidates.empty()) return selected;
    // 选择数量
    // 根据距离选择可能的下届和上界
    // int dis1 = manhattan(curr_x, curr_y, state.ti, state.tj);
    vector<PII> path;
    int dis1 = aStar(curr_x, curr_y, state.ti, state.tj, tempMap, path);
    int down = (dis1 <= 20 ? 2 : 0);
    int up = (dis1 > 20 ? 1 : 4);
    down = min(down, int(candidates.size())), up = min(up, int(candidates.size()));
    if (down >= up) return {};
    int cnt = rng.next(down, up);
    int vaild = 0;
    for (int i = 0, j = 0; i < cnt && j < candidates.size(); ++ j) {
        auto [x, y] = candidates[j];
        if (state.isTree[x][y]) continue;
        state.isTree[x][y] = true;
        if (check_path()) {
            selected.emplace_back(x, y);
            ++ i;
        } else {
            state.isTree[x][y] = false;
        }
    }
    return selected;
}

int main() {

    if (fopen("./data/0000.txt", "r") != nullptr) {

        freopen(("./data/0002.txt"), "r", stdin);
        freopen(("./output.txt"), "w", stdout);

        
    } else {
        cerr << "open file uncorrectly" << endl;
    }

    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    cin >> state.N;
    cin >> state.ti >> state.tj;
    int N = state.N;
    state.originalMap.resize(N, vector<char>(N));

    for (int i = 0; i <N; ++ i) {
        for (int j = 0; j < N; ++ j) {
            cin >> state.originalMap[i][j];
        }
    }
    
    // 初始化状态
    state.si = 0;
    state.sj = N / 2; // 入口坐标
    state.isTree.assign(N, vector<bool>(N, false));
    state.isRevealed.assign(N, vector<bool>(N, false));
    // 初始树状态（T）
    for (int i=0; i<N; ++i) {
        for (int j=0; j<N; ++j) {
            if (state.originalMap[i][j] == 'T') {
                state.isTree[i][j] = true;
            }
        }
    }

    // 入口初始已揭示
    state.isRevealed[state.si][state.sj] = true;
    int pi = state.si, pj = state.sj; 
    bool aaRevealed = false;         
    int steps = 0;          
    // 初始化可以放树的位置
    for (int i = 0; i < N; ++ i) {
        for (int j = 0; j < N; ++ j) {
            if (i == state.ti && j == state.tj) continue;
            if (i == state.si && j == state.sj) continue;
            if (state.isTree[i][j]) continue;
            state.allowPutList.emplace_back(i, j);
        }
    }

    
    while (true) {

   
        cin >> pi >> pj;
        int n;
        cin >> n;
        for (int k = 0; k < n; ++k) {
            int x, y;
            cin >> x >> y;
            state.isRevealed[x][y] = true;
        }

        // 2.2 终止判断：冒险者已到达AA花
        if (pi == state.ti && pj == state.tj) {
            cerr << "Adventurer reached AA flower in " << steps << " steps\n";
            return 0;
        }

        if (runtime() > LIMIT) {
            cout << "-1\n";
            cout.flush();
            return 0;
        }
        // cerr << "轮数: " << steps << " " << pi << " " << pj << endl;
        // 步骤1：程序放置Treant
        vector<pair<int, int>> treants = generate_treants(pi, pj);
        // 处理Cutoff
        if (!treants.empty() && treants[0].first == -1) {
            cout << "-1\n";
            cout.flush();
            return 0;
        }
                    
        // 输出Treant放置
        cout << treants.size();
        for (auto& [x, y] : treants) {
            cout << " " << x << " " << y;
        }
        cout << "\n";
        cout.flush();

        // 2.2 检查AA花是否被揭示
        aaRevealed = state.isRevealed[state.ti][state.tj];

        steps ++;

    }
    
    return 0;
}
