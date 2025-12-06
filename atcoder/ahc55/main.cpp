#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <vector>
#include <climits>
#include <chrono>
#include <algorithm>
#include <functional>
#include <queue>
#include <cmath>
using namespace std;

#define endl '\n'
#define INF 0x3f3f3f3f
#define PII pair<int, int>

constexpr int N = 200;
constexpr int M = 1e5 + 10;
constexpr int LIMIT = 1800;
const auto startTime = std::chrono::steady_clock::now();

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
int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

int main() {
    if (fopen("./data/0000.txt", "r") != nullptr) {
        freopen("./data/0021.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    } else {
        cerr << "open file incorrectly" << endl;
    }

    vector<int> H(N), C(N);
    vector<vector<int>> A(N, vector<int>(N));
    scanf("%*d");
    // 读取输入
    for (int i = 0; i < N; ++ i) scanf("%d", &H[i]);
    for (int i = 0; i < N; ++ i) scanf("%d", &C[i]);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            scanf("%d", &A[i][j]);
        }
    }
    /*
    * 启发搜索
    */
    vector<PII> bestResult;
    int bestAttack = INF;


    // 状态初始化
    vector<bool> opened(N, false);
    vector<int> rem_hard(N);
    vector<int> rem_dur(N, 0);
    int opened_cnt = 0;
    int attack = 0;
    int total_rem_hard = 0;
    vector<int> attacks(N, 0);
    for (int i = 0; i < N; ++i) {
        total_rem_hard += H[i];
        rem_hard[i] = H[i];
    }

    // 预处理：计算每个武器的最大攻击力
    vector<int> max_attack(N, 0);
    vector<int> max_vaild(N, 0);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j) {
                max_attack[i] = max(max_attack[i], int(A[i][j]));
                max_vaild[j] = max(max_vaild[j], A[i][j]);
            }
        }
    }
    for (int i = 0; i < N; ++ i) {
        attacks[i] = max_attack[i];
    }
    sort(attacks.begin(), attacks.end());
    int averageAttack = attacks[attacks.size() / 2];


    auto refresh = [&]()-> void {
        opened.assign(N, false);
        for (int i = 0; i < N; ++i) {
            total_rem_hard += H[i];
            rem_hard[i] = H[i];
        }
        rem_dur.assign(N, 0);
        opened_cnt = 0;
        attack = 0;

    };

    // 改进的贪心策略：优先开启能提供高价值武器的宝箱[6,7](@ref)
    auto get_box_priority = [&](int box) {
        // 综合考虑硬度、武器耐久度和最大攻击力
        if (rem_hard[box] <= 0) return -1.0;
        return (C[box] * max_attack[box] * 1.0) / rem_hard[box];
    };

    auto find_best_weapon_attack = [&](int pA, double pB) -> pair<int, int> {
        int best_w = -1, best_b = -1;
        double best_efficiency = -1;

        // 遍历所有可用武器
        for (int w = 0; w < N; ++ w) {
            if (!opened[w] || rem_dur[w] <= 0) continue;
            
            // 遍历所有未开启的宝箱
            for (int b = 0; b < N; ++ b) {
                if (opened[b]) continue;
                
                int damage = min(A[w][b], rem_hard[b]);
                if (damage <= 1) continue; // 只有伤害大于1才考虑使用武器
                
                // 计算攻击效率：避免伤害溢出[6](@ref)
                double efficiency = damage * 1.0;
                if (damage >= rem_hard[b]) {
                    // 能直接打开宝箱，给予额外奖励
                    efficiency += pA;
                }
                int up = max(1, int( 0.8 *  total_rem_hard / (200 - opened_cnt)));
                if (A[w][b] == max_vaild[b]) efficiency += min(averageAttack, up);
                if (A[w][b] == max_attack[w]) efficiency += min(averageAttack, up);
                // 考虑武器耐久度的价值
                // efficiency *= (rem_dur[w] * 1.0 / C[w]);
                efficiency *= 1.0 * rem_dur[w] / pow(1.0 * C[w], pB);
                
                if (efficiency > best_efficiency) {
                    best_efficiency = efficiency;
                    best_w = w;
                    best_b = b;
                }
            }
        }
        
        return {best_w, best_b};
    };

    auto find_best_barehand_target = [&]() -> int {
        int best_b = -1;
        double best_priority = -1;
        
        for (int b = 0; b < N; ++b) {
            if (opened[b]) continue;
            
            double priority = get_box_priority(b);
            // 剩余硬度少的宝箱有更高优先级
            priority += (500.0 - rem_hard[b]) / 500.0;
            
            if (priority > best_priority) {
                best_priority = priority;
                best_b = b;
            }
        }
        
        return best_b;
    };
    int select1[M];
    double select2[M];
    int end1 = 0, end2 = 0;
    for (int i = 0, factor = 100;; ++ i) {
        select1[i] = i * factor;
        if (select1[i] >= 1000) {
            end1 = i + 1;
            break;
        }
    }
    double alpha = 1;
    for (int i = 0;; ++ i) {
        select2[i] = 1.0 * i * alpha;
        if (select2[i] >= 2) {
            end2 = i + 1;
            break;
        }
    } 
    // 构造多个参数组合
    static int parameterA[M];
    static double parameterB[M];

    int end3 = 0;
    for (int i = 0; i < end1; ++ i) {
        for (int j = 0; j < end2; ++ j) {
            parameterA[i * end2 + j] = select1[i];
            parameterB[i * end2 + j] = select2[j];
        }

    }
    end3 = (end1) * (end2);

    vector<pair<double, double>> gs;
    for (int i = 0; i < N; ++ i) gs.emplace_back(1.0 *  H[i] / ( C[i]), i);
    sort(gs.begin(), gs.end());
    int index = 0;
    while (runtime() < LIMIT && index < end3) {
        // 主循环
        int cnt = 3;
        for (int c1 = 0; c1 < cnt; ++ c1) {
            vector<PII> curResult;
            for (int i = 0; i < c1; ++ i) {
                int x = gs[i].second;
                for (int j = 0; j < rem_hard[x]; ++ j ) curResult.emplace_back(-1, x);
                opened[x] = true;
                opened_cnt++;
                total_rem_hard -= rem_hard[x];
                rem_dur[x] = C[x];
            }
            while (opened_cnt < N) {
                // 尝试寻找最优的武器攻击
                auto [weapon, target] = find_best_weapon_attack(parameterA[index], parameterB[index]);
                
                if (weapon != -1) {
                    // 使用武器攻击
                    curResult.emplace_back(weapon, target);
                    rem_hard[target] -= A[weapon][target];
                    total_rem_hard -= A[weapon][target];
                    rem_dur[weapon]--;
                    
                    // 检查宝箱是否被打开
                    if (rem_hard[target] <= 0 && !opened[target]) {
                        opened[target] = true;
                        opened_cnt++;
                        rem_dur[target] = C[target];
                    }
                } else {
                    // 使用空手攻击
                    int target = find_best_barehand_target();
                    if (target == -1) break;
                    curResult.emplace_back(-1, target);
                    total_rem_hard -= 1;
                    rem_hard[target] --;
                    
                    if (rem_hard[target] <= 0 && !opened[target]) {
                        opened[target] = true;
                        opened_cnt ++;
                        rem_dur[target] = C[target];
                    }
                }
                

            }
            attack = curResult.size();
            if (attack < bestAttack) {
                bestAttack = attack;
                bestResult = move(curResult);
                cerr << "improved: " << bestAttack << endl;
            }

            refresh();
        }
        ++ index;
      
    }

    for (auto [x, y] : bestResult) {
        printf("%d %d\n", x, y);
    }

    cerr << "the number of attack : " << bestResult.size() << endl;

    return 0;
}