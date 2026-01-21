#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <tuple>
#include <queue>
#include <map>
using namespace std;
#define endl '\n'
#define LL long long
#define LD long double
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}
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
// 机器候选体：存储层级、ID、单次成本、累计收益、收益比
struct Candidate {
    int i, j;
    LL single_cost;  // 单次强化成本 (P+1)*C[i][j]
    LL max_k;        // 最大可强化次数
    LD total_benefit_k;  // 批量强化k次的总收益
    LD ratio;        // 单次收益比 = 单次收益 / 单次成本
};

struct State {
    vector<vector<LL>> P;
    vector<vector<LL>> B;
    LL apples;
    vector<pair<int, int>> actions;  // 记录所有动作
    
    State() : apples(1) {
        P.resize(4, vector<LL>(10, 0));
        B.resize(4, vector<LL>(10, 1));
    }
    
    State(const State& other) {
        P = other.P;
        B = other.B;
        apples = other.apples;
        actions = other.actions;
    }
    
    State& operator=(const State& other) {
        if (this != &other) {
            P = other.P;
            B = other.B;
            apples = other.apples;
            actions = other.actions;
        }
        return *this;
    }
    
    // 比较函数，用于优先队列
    bool operator<(const State& other) const {
        return apples < other.apples;
    }
};

// 预计算各阶累加和（剩余回合R）
inline LL sum1(LL R) { return R * (R + 1) / 2; }       // 一阶和 (i=1)
inline LL sum2(LL R) { return R * (R + 1) * (R + 2) / 6; }  // 二阶和 (i=2)
inline LL sum3(LL R) { return R * (R + 1) * (R + 2) * (R + 3) / 24; } // 三阶和 (i=3)

// 计算单次强化机器(i,j)的收益（剩余回合R）
inline LD calc_single_benefit(int i, int j, int R, const vector<int>& A, const vector<vector<LL>>& B) {
    if (i == 0) {
        return (LD)A[j] * B[0][j] * R;
    } else if (i == 1) {
        return (LD)A[j] * B[1][j] * sum1(R);
    } else if (i == 2) {
        return (LD)A[j] * B[2][j] * sum2(R);
    } else if (i == 3) {
        return (LD)A[j] * B[3][j] * sum3(R);
    }
    return 0.0;
}

class IterativeOptimizer {
private:
    vector<int> A;
    vector<vector<LL>> C;
    
public:
    IterativeOptimizer(const vector<int>& A_, const vector<vector<LL>>& C_)
        : A(A_), C(C_){
    }
    
    // 使用优质贪心算法生成初始解
    State greedySol(int turns) {
        State state;
        int current_turn = 0;
        while (current_turn < turns) {
            int R = turns - current_turn;  // 剩余有效回合数（强化后生效）
            Candidate best = {-1, -1, 0, 0, 0.0, 0.0};
            int best_k = 0;            // 最优批量强化次数
            
            // 步骤1：从j=9到j=0遍历，优先高A[j]
            for (int j = 9; j >= 0; -- j) {
                // 步骤2：遍历层级（0→3，低层级优先，复利效应更优）
                for (int i = 0; i <= 3; ++i) {
                    // 计算单次强化成本（当前P[i][j]+1次）
                    LL base_cost = C[i][j];
                    LL current_p = state.P[i][j];
                    LL single_cost = base_cost * (current_p + 1);
                    if (single_cost == 0 || single_cost > state.apples) continue;
                    
                    // 计算单次收益
                    LD single_benefit = calc_single_benefit(i, j, R, A, state.B);
                    if (single_benefit < single_cost * 1.1) continue;  // 过滤微利
                    
                    // 步骤3：计算最大可强化次数k_max
                    LL k_max = 0;
                    LL total_cost = 0;
                    while (true) {
                        LL k = k_max + 1;
                        LL cost_k = base_cost * (current_p + k);  // 第k次强化的成本
                        if (total_cost + cost_k > state.apples || k > R) break;  // 苹果不足/超过剩余回合
                        total_cost += cost_k;
                        k_max++;
                    }
                    if (k_max == 0) continue;
                    
                    // 步骤4：计算批量强化k_max次的总收益
                    LD total_benefit = 0.0;
                    for (int k = 1; k <= k_max; ++k) {
                        // 每次强化后，剩余回合数递减（因为占用1回合）
                        total_benefit += calc_single_benefit(i, j, R - (k-1), A, state.B);
                    }
                    
                    // 步骤5：计算批量收益比（总收益/总成本）
                    LD ratio = total_benefit / total_cost;
                    if (ratio > best.ratio || (ratio == best.ratio && total_cost < best.single_cost)) {
                        best = {i, j, total_cost, k_max, total_benefit, ratio};
                        best_k = k_max;
                    }
                }
                // 找到j层的最优解，直接跳出j循环（j=9优先）
                if (best.i != -1) break;
            }
            
            // 步骤6：执行强化/积累苹果
            if (best.i != -1 && best_k > 0 && current_turn + best_k <= turns) {
                // 批量强化best_k次
                for (int k = 1; k <= best_k; ++k) {
                    // 记录强化指令
                    state.actions.emplace_back(best.i, best.j);
                    // 扣除单次成本（逐次扣，避免溢出）
                    LL single_cost_k = C[best.i][best.j] * (state.P[best.i][best.j] + 1);
                    state.apples -= single_cost_k;
                    state.P[best.i][best.j] += 1;
                    current_turn++;
                    
                    // 执行本回合的产出阶段
                    simulate_production(state);
                }
            } else {
                // 无可用机器，不行动
                state.actions.emplace_back(-1, -1);
                current_turn++;
                
                // 执行本回合的产出阶段
                simulate_production(state);
            }
        }
        
        return state;
    }
    // 模拟退火算法
    State SA(const State& initial, int turns) {
        State current = initial;
        State best = initial;

        double startTemp = 1000.0;
        double endTemp = 1.0;
        double TIME_INIT = runtime();
        double TIME_LIMIT = 1980.0 - TIME_INIT;
        LL loopCnt = 0;
        LL improveCnt = 0;
        double timeNow = 0.0;
        double temp = 0.0;
        double progress = 0.0;
        double gw = 0.0; // 扰动权值
        while (1) {
            if (loopCnt % 1000 == 0) {
                timeNow = runtime() - TIME_INIT;
                if (timeNow > TIME_LIMIT)
                    break;
                progress = pow(timeNow / TIME_LIMIT, 0.7); // 采用了非线性收敛（0.7这里就可以得到早期收敛慢，后期收敛快）
                temp = startTemp + (endTemp - startTemp) * progress;
                gw = max(1.0, 4 - progress * 4); // 早期扰动大，探索阶段;后期扰动小，收敛阶段
            }

            ++ loopCnt;
            int numChanges = (int)(gw + 0.5);
            if (numChanges <= 0 || numChanges >= 500) continue;
            // 生成邻居解
            State neighbor = current;
            int type = xor128() % 5;
            for (int c = 0; c < numChanges; c ++) {
                int turn = xor128() % turns;
                if (type <= 3) {
                    int i = xor128() % 4;
                    int j = xor128() % 10;
                    neighbor.actions[turn] = {i, j};
                } else {
                    neighbor.actions[turn] = {-1, -1};
                }

            }
            // 评估邻居解
            neighbor = evalSol(neighbor, turns);
            // 计算能量差
            LL delta = neighbor.apples - current.apples;
            // 接受准则
            if (delta > 0 || getRandom() < exp(delta / temp)) {
                current = neighbor;
                
                if (current.apples > best.apples) {
                    best = current;
                    ++ improveCnt;
                    // cerr << "SA Iteration " << loopCnt << ": Best = " 
                    //      << scientific << setprecision(6) << (LD)best.apples 
                    //      << ", Temp = " << fixed << setprecision(3) << temp << endl;
                }
            }
            
        }
        DATA(improveCnt);
        DATA(loopCnt);
        
        return best;
    }
    

    
    // 迭代优化主函数
    State solve(int turns) {
        State greedSol = greedySol(turns);
        cerr << "Greedy: " << scientific << setprecision(6) 
             << (LD)greedSol.apples << " apples" << endl;
        State saSol = SA(greedSol, turns);
        cerr << "Simulated annealing: " << scientific << setprecision(6) 
                << (LD)saSol.apples << " apples" << endl;
        cerr << "Time used: " << fixed << setprecision(0) 
             << runtime() << "ms" << endl;
        
        return saSol;
    }
    
private:
    // 模拟生产阶段
    void simulate_production(State& state) {
        // Level 0: 生产苹果
        for (int j = 0; j < 10; ++j) {
            state.apples += A[j] * state.B[0][j] * state.P[0][j];
        }
        
        // Level 1-3: 生产低级机器
        for (int i = 1; i < 4; ++i) {
            for (int j = 0; j < 10; ++j) {
                state.B[i-1][j] += state.B[i][j] * state.P[i][j];
            }
        }
    }
    
    // 评估完整解
    State evalSol(const State& solution, int turns) {
        State result;
        result.actions = solution.actions;
        
        // 重新执行所有动作
        for (int turn = 0; turn < turns; turn++) {
            auto action = result.actions[turn];
            if (action.first != -1) {
                int i = action.first, j = action.second;
                LL cost = C[i][j] * (result.P[i][j] + 1);
                if (cost <= result.apples) {
                    result.apples -= cost;
                    result.P[i][j]++;
                } else {
                    // 如果买不起，改为不行动
                    result.actions[turn] = {-1, -1};
                }
            }
            
            simulate_production(result);
        }
        
        return result;
    }

};

int main() {
    // 设置文件重定向
    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0001.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    }
    
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    
    // 读取输入
    int N, L, T;
    LL K;
    cin >> N >> L >> T >> K;
    
    vector<int> A(N);
    vector<vector<LL>> C(L, vector<LL>(N));
    
    for (int j = 0; j < N; ++j) cin >> A[j];
    for (int i = 0; i < L; ++i)
        for (int j = 0; j < N; ++j)
            cin >> C[i][j];
    

    // 创建迭代优化器
    IterativeOptimizer optimizer(A, C);
    
    // 执行迭代优化
    State bestSol = optimizer.solve(T);
    
    // 输出动作序列
    for (const auto& action : bestSol.actions) {
        if (action.first == -1) {
            cout << "-1\n";
        } else {
            cout << action.first << " " << action.second << "\n";
        }
    }
    
    return 0;
}