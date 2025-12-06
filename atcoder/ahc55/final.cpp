#pragma GCC optimize "Ofast,omit-frame-pointer,inline,fast-math,unroll-all-loops,tree-loop-vectorize,tree-slp-vectorize"

#include <chrono>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <cassert>
#include <queue>
#include <unordered_set>
using namespace std;

#define mem(a,b) memset(a,b,sizeof a)
#define PII pair<int,int>
#define ll long long
#define ull unsigned long long
#define IOS ios::sync_with_stdio(0),cin.tie(0),cout.tie(0)
#define gcd(a,b) __gcd(a,b)
#define ft first
#define sd second
#define endl '\n'
#define PI acos(-1.0)
#define lcm(a,b) a/gcd(a,b)*b
#define INF_INT 0x3f3f3f3f
#define INF_LONG 4557430888798830399
#define FOR(i,a,b) for(int i=(a);i<(b);++i)
#define REP(i,a) FOR(i,0,a)
#define pre(i,x,y) for(int i=x; i>=y; --i) 
#define x1 x111111
#define y1 y111111
#define x0 x00000
#define y0 y00000
#define lp p<<1
#define rp p<<1|1
#define PB push_back
#define EB emplace_back
#define ZERO(m)     memset(m,0,sizeof(m))
#define MINUS(m)    memset(m,-1,sizeof(m))
#define ALL(x)      x.begin(),x.end()
#define S           size()
#define VC vector
#define VI VC<int>
#define VVI VC<VI>
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}
#define INLINE   inline __attribute__ ((always_inline))
#define NOINLINE __attribute__ ((noinline))

constexpr int N = 200;
constexpr int LIMIT = 1900;
constexpr int MAX = 1e5;
const auto startTime = std::chrono::steady_clock::now();

 int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

template<typename T>
inline bool chmax(T &a, T b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}
template<typename T>
inline bool chmin(T &a, T b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
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
    //  int next(int x) {return ((long long)rand() * x) >> 32;}
     int next(int x) {return rand() % x;}
     int next(int a, int b) {return a + (rand() % (b - a));}
    //  int next(int a, int b) {return a + ((long long) rand() * (b - a)) >> 32;}
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 

static RNG rng;

struct Strategy {
    VI H, C;
    VVI A;
    VI maxAttack;
    int bestAttack{};
    void init() {
        scanf("%*d");
        H.resize(N);
        REP(i, N) scanf("%d", &H[i]);
        C.resize(N);
        REP(i, N) scanf("%d", &C[i]);
        A.resize(N, VI(N));
        REP(i, N) {
            REP(j, N) {
                scanf("%d", &A[i][j]);
            }
        }
        maxAttack.resize(N);

        // 预计算每个武器的最大攻击力
        REP(i, N)
            REP(j, N)
                maxAttack[i] = max(maxAttack[i], A[i][j]);
        
        cerr << "初始化ok" << endl;
    }
    bool isTopological(const VVI &graph) {
        int n = graph.size();
        VI in(n, 0);
        REP(i, n) {
            REP(j, C[i]) {
                int idx = graph[i][j];
                if (idx != -1) in[idx] ++;
            }
        }
        static int q[MAX];
        int head = 0, tail = -1;
        REP(i, n) {
            if (in[i] == 0) {
                q[++ tail] = i;
            }
        }
        int cnt = 0;
        while (head <= tail) {
            int x = q[head ++];
            ++ cnt;
            REP(j, C[x]) {
                int idx = graph[x][j];
                if (idx == -1) continue;
                if (-- in[idx] == 0) {
                    q[++ tail] = idx;
                }
            }

        }
        return cnt == n;
    }
    VI getTopological(const VVI &graph) {
        VI res;
        int n = graph.size();
        VI in(n, 0);
        REP(i, n) {
            REP(j, C[i]) {
                int idx = graph[i][j];
                if (idx != -1) in[idx] ++;
            }
        }
        VI q(n + 10);
        int head = 0, tail = -1;
        REP(i, n) {
            if (in[i] == 0) q[++tail] = i;
        }
        int cnt = 0;
        while (head <= tail) {
            int x = q[head ++];
            res.EB(x);
            REP(j, C[x]) {
                int idx = graph[x][j];
                if (idx == -1) continue;
                if (-- in[idx] == 0) {
                    q[++ tail] = idx;
                }
            }

        }
        if (res.size() != n) return {};
        return res;
    }
    VVI getGraph() {
        VI order(N);
        REP(i, N) order[i] = i;
        sort(ALL(order), [&](const int idx1, const int idx2){
            return H[idx1] / max(1,maxAttack[idx1]) > H[idx2] / max(1, maxAttack[idx2]);
        });
        VVI graph(N, VI(6, -1));
        REP(i, N - 1) {
            REP(j, C[order[i]]) {
                graph[order[i]][j] = order[i + 1];
            }
        }
        REP(j, C[order.back()]) graph[order.back()][j] = -1;
        int score = 0;
        vector<int> hTemp = H;
        REP(i, N) {
            REP(j, C[i]) {
                int nt = graph[i][j];
                if (nt == -1) continue;
                hTemp[nt] -= A[i][nt];
            }
        }
    
        REP(i, N) score += max(0, hTemp[i]);
        VVI candidateAttack(N);
        REP(i, N) {
            REP(j, N) {
                candidateAttack[i].EB(j);
            }
            sort(ALL(candidateAttack[i]), [&](int a, int b){
                return A[i][a] > A[i][b];
            });

            candidateAttack[i].resize(20);
        }
        int bestScore = score;
        VVI bestGraph = graph;
        // 定义退火条件
        constexpr double temp0 = 200;
        constexpr double temp1 = 0.001;
        double temp = temp0;
        int loop = 0, updateLoop = 0;
        const double saStart = runtime(); 
        while (true) {
            if (runtime() > LIMIT) break;
            ++ loop;
            if (loop & 255) {
                double timePassed = (runtime() - saStart) / LIMIT;
                if (timePassed >= 1) break;
                temp = temp0 * pow(temp1 / temp0, timePassed);
            }
            int a = rng.next(N);
            int b = rng.next(C[a]);
            int nx = candidateAttack[a][rng.next(candidateAttack[a].size())];
            //a的b次攻击nx
            if (graph[a][b] == nx) continue;
            int newScore = score;
            int px = graph[a][b];
            if (px != -1) {
                newScore -= max(0, hTemp[px]);
                newScore += max(0, hTemp[px] + A[a][px]);
            }
            newScore -= max(0, hTemp[nx]);
            newScore += max(0, hTemp[nx] - A[a][nx]);

            double diff = score - newScore;
            if (diff >= 0 || exp(diff / temp) > rng.next_double()) {
                graph[a][b] = nx;

                if (!isTopological(graph)) {
                    graph[a][b] = px;
                    continue;
                }
                score = newScore;
                hTemp[px] += A[a][px];
                hTemp[nx] -= A[a][nx];
                if (chmin(bestScore, score)){
                    updateLoop ++;
                    bestGraph = graph;
                    DATA(score);
                }
            }
        
        }
        DATA(updateLoop);
        DATA(loop);
        DATA(bestScore);


        cerr << "构造拓扑图ok" << endl;
        return bestGraph;







    }
    void mainLoop() {
        init();
        auto graph = getGraph();
        VI order = getTopological(graph);
        vector<PII> ans;
        VVI weapons(N);
        
        for (int idx : order) {
            int x = H[idx];
            sort(ALL(weapons[idx]), [&](const int a, const int b){
                return A[a][idx] < A[b][idx];
            });

            while (x > 0 && weapons[idx].size()) {
                int tail = weapons[idx].back();
                weapons[idx].pop_back();
                x -= A[tail][idx];
                ans.EB(tail, idx);

            }

            while (x > 0) {
                -- x;
                ans.EB(-1, idx);
            }

            REP(i, C[idx]) {
                int j = graph[idx][i];
                if (j == -1) continue;
                weapons[j].EB(idx); 
            }

        }

        for (auto &[w, b] : ans) printf("%d %d\n", w, b);
        cerr << "总共攻击次数: " << ans.S << endl;


    }
};

int main() {
    if (fopen("../data/0001.txt", "r") != nullptr) {
        freopen("../data/0001.txt", "r", stdin);
        freopen("../output.txt", "w", stdout);
    } else {
        cerr << "not found file" << endl;
    }
    static Strategy strategy;
    strategy.mainLoop();
    return 0;
}
