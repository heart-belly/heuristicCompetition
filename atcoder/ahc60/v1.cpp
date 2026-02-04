#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <bits/stdc++.h>
using namespace std;

static const int NONE_PREV = 1000000007;
const double TIME_LIMIT = 1.8; 
int ways[110][110];
vector<int> targets{0,1,2,3,4,5,6,7,8,9};
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
// 评分函数：严格模拟赛题执行过程，计算真实得分∑|Sᵥ|
int calculate_score(const vector<string>& actions, int N, int K, const vector<vector<int>>& g) {
    int cur = 0, prev = -1;
    string cone; 
    vector<char> color(N, 'W'); 
    unordered_set<string> shop_inv[10]; 

    for (const string& act : actions) {
        if (act == "-1") {

            if (cur >= K && color[cur] == 'W') {
                color[cur] = 'R';
            }
        } else {

            int to = stoi(act);
            prev = cur;
            cur = to;
            if (cur >= K) {

                cone += color[cur];
            } else {

                shop_inv[cur].insert(cone);
                cone.clear();
            }
        }
    }


    int total = 0;
    for (int i = 0; i < K; i++) {
        total += shop_inv[i].size();
    }
    return total;
}


vector<string> generate_schedule(int N, int M, int K, int T, const vector<vector<int>>& g) {
    vector<string> actions(T);
    int idx = 0;
    vector<char> color(N, 'W');

    // 随机数生成器：每次生成新调度用不同种子，保证随机性
    std::mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
    auto rand_int = [&](int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    };
    auto rand_prob = [&]() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    };

    auto bfs_path_no_backtrack = [&](int start, int prev, int goal) -> vector<int> {
        int P0 = (prev < 0 ? N : prev);
        const int INF = 1e9;
        vector<vector<int>> dist(N, vector<int>(N + 1, INF));
        vector<vector<pair<int,int>>> par(N, vector<pair<int,int>>(N + 1, {-1, -1}));
        deque<pair<int,int>> q;
        dist[start][P0] = 0;
        q.push_back({start, P0});
        pair<int,int> goal_state = {-1, -1};

        while (!q.empty()) {
            auto [v, p] = q.front();
            q.pop_front();
            if (v == goal) {
                goal_state = {v, p};
                break;
            }
            for (int to : g[v]) {
                if (to == p) continue;
                int np = v;
                int w = (color[to] == 'W'? 1 : 3);
                if (dist[to][np] > dist[v][p] + w ) {
                    dist[to][np] = dist[v][p] + w;
                    par[to][np] = {v, p};
                    q.push_back({to, np});
                }
            }
        }
        if (goal_state.first == -1) return vector<int>{start};
        vector<int> path;
        auto [cv, cp] = goal_state;
        while (!(cv == start && cp == P0)) {
            path.push_back(cv);
            auto pr = par[cv][cp];
            cv = pr.first;
            cp = pr.second;
        }
        path.push_back(start);
        reverse(path.begin(), path.end());
        return path;
    };


    int cur = 0;
    int prev = -1;
    int cone_len = 0;
    int next_shop = 1 % K;
    int cnt = 0;
    int L = rand_int(5, 25);
    int flip_count = 0;
    const int FLIP_LIMIT = 60;
    const double FLIP_PROB = 0.022;
    int actions_cnt = 0;
    fast_shuffle(targets.begin(), targets.end());
    while (actions_cnt < T) {
        if (cnt % 10 == 0) {
            fast_shuffle(targets.begin(), targets.end());
        }
        if (cur >= K && color[cur] == 'W' && flip_count < FLIP_LIMIT) {
            if (rand_prob() < FLIP_PROB) {
                actions[idx ++] = "-1";
                color[cur] = 'R';
                flip_count++;
                actions_cnt++;
                continue;
            }
        }

        bool go_shop = false;
        if (cone_len >= L ) go_shop = true;
        if (T - actions_cnt <= 50) go_shop = true;

        if (go_shop) {
            int target = targets[next_shop];
            next_shop = (next_shop + 1) % K;
            ++ cnt;
            auto path = bfs_path_no_backtrack(cur, prev, target);
            for (int i = 1; i < (int)path.size() && actions_cnt < T; i++) {
                int to = path[i];
                actions[idx ++] = to_string(to);
                actions_cnt++;
                prev = cur;
                cur = to;
                if (cur < K) {
                    cone_len = 0;
                    
                    L = rand_int(5, 15);
                    

                } else {
                    cone_len ++;
                }
            }
            continue;
        }

        vector<int> cand;
        for (int to : g[cur]) {
            if (to == prev) continue;
            cand.push_back(to);
        }
        if (cand.empty()) cand = g[cur];

        int chosen = -1;
        if (cone_len == 0) {
            vector<int> trees;
            for (int to : cand) if (to >= K) trees.push_back(to);
            if (!trees.empty()) {
                chosen = trees[rand_int(0, (int)trees.size() - 1)];
            }
        }
        if (chosen == -1) {
            chosen = cand[rand_int(0, (int)cand.size() - 1)];
        }
        actions[idx++] = to_string(chosen);
        actions_cnt++;
        prev = cur;
        cur = chosen;
        if (cur < K) {
            cone_len = 0;
            L = rand_int(5, 25);
        } else {
            cone_len++;
        }
    }

    return actions;
}

int main() {

    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0001.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    }
    ios::sync_with_stdio(false);
    cin.tie(nullptr);


    int N, M, K, T;
    cin >> N >> M >> K >> T;
    vector<vector<int>> g(N);
    for (int i = 0; i < M; i++) {
        int A, B;
        cin >> A >> B;
        g[A].push_back(B);
        g[B].push_back(A);
    }

    for (int i = 0; i < N; i++) {
        int X, Y;
        cin >> X >> Y;
    }
    memset(ways, 1, sizeof ways);

    int best_score = -1;
    vector<string> best_actions;
    auto start_time = chrono::high_resolution_clock::now();
    int iter_count = 0; 


    while (true) {

        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();
        if (elapsed >= TIME_LIMIT) {
            break;
        }


        vector<string> cur_actions = generate_schedule(N, M, K, T, g);

        int cur_score = calculate_score(cur_actions, N, K, g);
        iter_count++;


        if (cur_score > best_score) {
            best_score = cur_score;
            best_actions = cur_actions;
            cerr << "第" << iter_count << "轮 | 分数提升至：" << best_score << " | 耗时：" << fixed << setprecision(2) << elapsed << "s\n";
        }
    }

    // 输出最终结果
    cerr << "\n[寻优结束] 总迭代轮数：" << iter_count << " | 最优分数：" << best_score << "\n";
    cerr << "[输出最优行动序列] 总步数：" << best_actions.size() << "\n";
    for (const string& act : best_actions) {
        cout << act << "\n";
    }

    return 0;
}