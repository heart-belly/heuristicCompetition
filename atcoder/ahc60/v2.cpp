#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <chrono>
using namespace std;
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}
#define endl '\n'
constexpr int LIMIT = 1650;
// bfs
constexpr int MAX_DEPTH = 20;
constexpr int MAX_STATUE = 50000;
// hash
constexpr uint64_t HASH_BASE = 3ULL;
constexpr uint64_t  HASH_EMPTY = 0ULL;
// adj
constexpr int N = 110, M = 1e4 + 10;

struct Point {
    int v, fa;
};
struct BFSState {
    int v, pv, parentIdx, depth;
    uint64_t hash;
};
const auto startTime = std::chrono::steady_clock::now();

inline int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

int xor128() {
    static int x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    int t = (x ^ (x << 11));
    x = y; y = z; z = w;
    return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}

double getRandom() {
    double rand_ = (double)(xor128()) / (double)INT32_MAX;
    return rand_;
}

double genGaussian() {
    double X = getRandom();
    double Y = getRandom();
    double Z = sqrt(-2.0 * log(X)) * cos(2.0 * acos(-1.0) * Y);
    return Z;
    }
struct Strategy {
    vector<char> simFlavor;
    vector<unordered_set<uint64_t>>   simInventory;
    vector<int> simActions;
    vector<BFSState> simStates;
    vector<vector<int>> adj;
    int n, m, K, T;

    void input() {
        cin >> n >> m >> K >> T;
        adj.resize(n);
        for (int i = 0; i < m; ++ i) {
            int a, b;
            cin >> a >> b;
            adj[a].push_back(b); adj[b].push_back(a);
        }
        for (int i = 0; i < n; ++ i) {
            int x, y;
            cin >> x >> y;
        }


    }
    void init() {
        simFlavor.resize(N);
        simInventory.resize(K);
        simActions.reserve(T);
        simStates.reserve(MAX_STATUE + 1000);
    }
    vector<int> getPath(int idx, int shop) {
        vector<int> path;
        path.push_back(shop);
        for (int p = idx; p > 0; p = simStates[p].parentIdx) {
            path.push_back(simStates[p].v);
        }
        reverse(path.begin(), path.end());
        return path;
    }
    void greedy(const vector<int>& convertAt) {
        fill(simFlavor.begin(), simFlavor.end(), 'W');
        for (int i = 0; i < K; ++ i) simInventory[i].clear();
        simActions.clear();

        Point cur{0, -1};
        uint64_t coneHash = HASH_EMPTY;
        int steps = 0;

        while (steps < T) {
            simStates.clear();
            simStates.push_back({cur.v, cur.fa, -1, 0, coneHash});
            vector<int> chosenPath;
            int fState = -1, fShop = -1;
            bool found = false;
            for (int idx = 0; idx < simStates.size() && !found && simStates.size() < MAX_STATUE; idx ++) {
                const auto &st = simStates[idx];
                if (st.depth >= MAX_DEPTH) continue;
                for (int j : adj[st.v]) {
                    if (j == st.pv) continue;
                    if (j < K) {
                        if (fState == -1) fState = idx, fShop = j;
                        if (simInventory[j].find(st.hash) == simInventory[j].end()) {
                            chosenPath = getPath(idx, j);
                            found = true;
                            break;
                        }
                    } else if (simStates.size() < MAX_STATUE) {
                        uint64_t ch = (simFlavor[j] == 'W') ? 1ULL : 2ULL;
                        simStates.push_back({j, st.v, idx, st.depth + 1, st.hash * HASH_BASE + ch});
                    }
                }

            }
            if (chosenPath.empty()) {
                if (fState == -1) break;
                chosenPath = getPath(fState, fShop);
            }

            for (int node : chosenPath) {
                if (steps >= T) break;
                simActions.push_back(node);
                ++ steps;
                cur.fa = cur.v;
                cur.v = node;

                if (node < K) {
                    simInventory[node].insert(coneHash);
                    coneHash = HASH_EMPTY;
                } else {
                    uint64_t ch = (simFlavor[node] == 'W') ? 1ULL : 2ULL;
                    coneHash = coneHash * HASH_BASE + ch;
                    if (simFlavor[node] == 'W' && steps < T && steps >= convertAt[node]) {
                        simFlavor[node] = 'R';
                        simActions.push_back(-1);
                        ++ steps;

                    }
                }
            }
        }

        

    }
    int getScore() {
        int ans = 0;
        for (int i = 0; i < K; ++ i) {
            ans += simInventory[i].size();
        }
        return ans;
    }
    void mainLoop() {
        input();
        init();

        vector<int> convertAt(N, T + 1);
        for (int i = K; i < N; ++ i) {
            convertAt[i] = (int)xor128() %  T;
        }
        vector<int> bestActions;
        int bestScore = 0;
        while (runtime() < LIMIT) {
            greedy(convertAt);
            int score = getScore();
            if (score > bestScore) {
                bestScore = score;
                bestActions = move(simActions);
                DATA(score);
            }
            for (int i = K; i < N; ++ i) {
                convertAt[i] = (int)xor128() %  T;
            }
        }

        DATA(bestScore);

        auto output = [&](){
        for (int a : bestActions)
            cout << a << '\n';
        };
        output();
    }

};
int main() {
    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0001.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    }
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    static Strategy strategy;
    strategy.mainLoop();

    int endTime = runtime();
    DATA(endTime);
}