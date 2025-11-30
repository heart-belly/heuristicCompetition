#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <cerrno>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <random>
using namespace std;


#define Local

constexpr double L = 100000.0;
constexpr int N = 300;         // 总点数
constexpr int M = 10;          // 目标分量数
constexpr int K = 30;          // 目标分量大小
constexpr int T = 1000;
constexpr int BONDS_NEEDED = N - M; // 需290个bond
constexpr double EPS = 1e-9;   // 数值精度控制

// 模拟退火参数（适配2秒时间限制）
constexpr double T0 = 8;       // 初始温度
constexpr double alpha = 0.95; // 温度衰减率
constexpr int L1 = 10;         // 每个温度下的迭代次数
constexpr double TIME_LIMIT = 1700; // 模拟退火时间限制（留0.2秒收尾）

const auto startTime = std::chrono::steady_clock::now();
inline int runtime() {
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

// 计算环面距离的平方并四舍五入（精准计算成本）
inline double calculate_bond_cost(double x1, double y1, double x2, double y2) {
    double dx = fabs(x1 - x2);
    dx = min(dx, L - dx);
    double dy = fabs(y1 - y2);
    dy = min(dy, L - dy);
    return round(dx * dx + dy * dy);
}

// 计算点p在时刻t的实时位置（基于初始速度，不考虑合并影响）
inline void calculate_point_position(int p, int t, const vector<double>& initialX, const vector<double>& initialY,
                                     const vector<double>& initialVx, const vector<double>& initialVy,
                                     double& x, double& y) {
    x = fmod(initialX[p] + initialVx[p] * t + L, L);
    y = fmod(initialY[p] + initialVy[p] * t + L, L);
}

// 并查集（仅用于MST生成和聚类验证）
class UnionFind {
public:
    vector<int> parent;
    vector<int> size;

    UnionFind(int n) : parent(n), size(n, 1) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    inline int find(int x) {
        return parent[x] == x ? x : (parent[x] = find(parent[x]));
    }

    inline bool unite(int i, int j) {
        int ri = find(i);
        int rj = find(j);
        if (ri == rj) return false;
        if (size[ri] < size[rj]) swap(ri, rj);
        parent[rj] = ri;
        size[ri] += size[rj];
        return true;
    }

    inline int get_size(int i) {return size[find(i)];}
};

// -------------------------- 核心函数1：对单个时刻t，生成完整解（重新聚类+MST） --------------------------
bool GT(int t, const vector<double>& initialX, const vector<double>& initialY,
                            const vector<double>& initialVx, const vector<double>& initialVy,
                            vector<tuple<int, int, int>>& bonds, double& total_cost) {
    bonds.clear();
    total_cost = 0.0;

    // 1. 计算时刻t所有点的实时位置
    vector<double> x_t(N), y_t(N);
    for (int p = 0; p < N; ++p) {
        calculate_point_position(p, t, initialX, initialY, initialVx, initialVy, x_t[p], y_t[p]);
    }

    // 2. 基于实时位置的网格聚类（2×5=10个大簇，空间集中）
    const int grid_rows = 2, grid_cols = 5;
    const double x_step = L / grid_cols, y_step = L / grid_rows;
    vector<vector<int>> region_points(grid_rows * grid_cols); // 每个区域的点

    // 2.1 点归类到区域（左上角→右下角顺序）
    for (int p = 0; p < N; ++p) {
        int x_col = min(static_cast<int>(x_t[p] / x_step), grid_cols - 1);
        int y_row = min(static_cast<int>((L - y_t[p]) / y_step), grid_rows - 1); // y从上到下
        int region_idx = y_row * grid_cols + x_col;
        region_points[region_idx].push_back(p);
    }

    // 2.2 均衡分配点到10个大簇（每个簇30点）
    vector<vector<int>> big_clusters(M);
    vector<int> region_ptr(grid_rows * grid_cols, 0);

    // 第一轮：优先分配本区域点
    for (int r = 0; r < grid_rows * grid_cols; ++r) {
        int cluster_idx = r % M; // 区域→大簇映射（保证顺序）
        auto& cluster = big_clusters[cluster_idx];
        auto& points = region_points[r];
        int take = min(30 - (int)cluster.size(), (int)points.size() - region_ptr[r]);
        for (int i = 0; i < take; ++i) {
            cluster.push_back(points[region_ptr[r] + i]);
        }
        region_ptr[r] += take;
    }

    // 第二轮：补充不足的簇（从任意区域借点）
    for (int c = 0; c < M; ++c) {
        auto& cluster = big_clusters[c];
        if (cluster.size() == K) continue;
        int need = K - (int)cluster.size();

        for (int r = 0; r < grid_rows * grid_cols && need > 0; ++r) {
            auto& points = region_points[r];
            int available = (int)points.size() - region_ptr[r];
            if (available == 0) continue;
            int take = min(need, available);
            for (int i = 0; i < take; ++i) {
                cluster.push_back(points[region_ptr[r] + i]);
            }
            region_ptr[r] += take;
            need -= take;
        }

        if (cluster.size() != K) return false; // 聚类失败（理论上不会发生）
    }

    // 3. 每个大簇内生成MST（基于t时刻实时位置）
    for (auto& cluster : big_clusters) {
        // 3.1 构建簇内边（基于t时刻位置）
        vector<pair<double, pair<int, int>>> edges;
        int n = cluster.size();
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                int p1 = cluster[i], p2 = cluster[j];
                double cost = calculate_bond_cost(x_t[p1], y_t[p1], x_t[p2], y_t[p2]);
                edges.emplace_back(cost, make_pair(p1, p2));
            }
        }

        // 3.2 Kruskal生成MST
        sort(edges.begin(), edges.end());
        UnionFind mst_uf(N);
        int edge_count = 0;
        for (auto& [cost, p_pair] : edges) {
            int p1 = p_pair.first, p2 = p_pair.second;
            if (mst_uf.unite(p1, p2)) {
                bonds.emplace_back(t, p1, p2);
                total_cost += cost;
                edge_count++;
                if (edge_count == K - 1) break;
            }
        }

        if (edge_count != K - 1) return false; // MST生成失败
    }

    // 4. 验证解的合法性
    if (bonds.size() != BONDS_NEEDED) return false;
    return true;
}

// -------------------------- 核心函数2：遍历所有时刻，选择全局最优解作为初始解 --------------------------
void solve(const vector<double>& initialX, const vector<double>& initialY,
                                         const vector<double>& initialVx, const vector<double>& initialVy,
                                         vector<tuple<int, int, int>>& initialBonds, double& initialCost) {
    cerr << "===== 开始遍历0~999所有时刻，寻找全局最优解 =====" << endl;
    initialCost = numeric_limits<double>::max();
    initialBonds.clear();

    for (int t = 0; t < T; ++t) {
        vector<tuple<int, int, int>> tempBonds;
        double tempCost;
        if (!GT(t, initialX, initialY, initialVx, initialVy, tempBonds, tempCost)) {
            continue; 
        }
        if (tempCost < initialCost - EPS) {
            initialCost = tempCost;
            initialBonds = tempBonds;
        }
    }
    // 按时间步排序（保证初始解有序）
    sort(initialBonds.begin(), initialBonds.end(), [](const auto& a, const auto& b) {
        return get<0>(a) < get<0>(b);
    });

    cerr << "\n===== 全局最优初始解找到 =====" << endl;
    cerr << "最优时刻：t=" << get<0>(initialBonds[0]) << "（所有bond都在该时刻）" << endl;
    cerr << "初始解bond数：" << initialBonds.size() << "（期望" << BONDS_NEEDED << "）" << endl;
    cerr << "初始解总成本：" << initialCost << endl;
}

// 核心功能：重新计算所有bond的实际成本（基于执行时的实时位置）
double recalculate_total_cost(const vector<tuple<int, int, int>>& bonds,
                              const vector<double>& initialX, const vector<double>& initialY,
                              const vector<double>& initialVx, const vector<double>& initialVy,
                              int T) {
    UnionFind uf(N);
    vector<double> x = initialX;
    vector<double> y = initialY;
    double total_cost = 0.0;
    int last_t = 0;

    for (size_t idx = 0; idx < bonds.size(); ++idx) {
        int t = get<0>(bonds[idx]);
        int i = get<1>(bonds[idx]);
        int j = get<2>(bonds[idx]);

        // 确保t合法
        t = max(0, min(t, T - 1));

        // 更新到时刻t的位置（考虑合并后的质心速度）
        vector<double> cluster_vx(N, 0), cluster_vy(N, 0);
        for (int step = last_t; step < t; ++step) {
            // 先计算每个簇的质心速度
            unordered_map<int, pair<double, double>> centroid_v; // root → (vx_sum, vy_sum)
            unordered_map<int, int> cluster_size;
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                centroid_v[root].first += initialVx[p];
                centroid_v[root].second += initialVy[p];
                cluster_size[root]++;
            }
            // 更新质心速度（平均速度）
            for (auto& [root, v] : centroid_v) {
                cluster_vx[root] = v.first / cluster_size[root];
                cluster_vy[root] = v.second / cluster_size[root];
            }
            // 更新点位置
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                x[p] = fmod(x[p] + cluster_vx[root] + L, L);
                y[p] = fmod(y[p] + cluster_vy[root] + L, L);
            }
        }

        // 计算bond成本
        total_cost += calculate_bond_cost(x[i], y[i], x[j], y[j]);

        // 执行合并
        uf.unite(i, j);

        last_t = t + 1;
    }

    return total_cost;
}

// 验证解的合法性（bond数=290，最终簇合法，t<T）
bool validate_solution(const vector<tuple<int, int, int>>& bonds,
                       const vector<double>& initialVx, const vector<double>& initialVy,
                       int T) {
    if (bonds.size() != BONDS_NEEDED) return false;

    // 检查时间步合法
    for (const auto& bond : bonds) {
        int t = get<0>(bond);
        if (t < 0 || t >= T) return false;
    }

    // 检查簇结构合法
    UnionFind uf(N);
    for (const auto& bond : bonds) {
        int i = get<1>(bond), j = get<2>(bond);
        uf.unite(i, j);
    }

    unordered_set<int> roots;
    for (int i = 0; i < N; ++i) roots.insert(uf.find(i));
    if (roots.size() != M) return false;
    for (int root : roots) {
        if (uf.get_size(root) != K) return false;
    }

    return true;
}

tuple<int, int, int> generate_replacement_bond(const vector<tuple<int, int, int>>& current_bonds,
                                               const vector<double>& initialX, const vector<double>& initialY,
                                               const vector<double>& initialVx, const vector<double>& initialVy,
                                               int T) {
    UnionFind uf(N);
    vector<double> x = initialX;
    vector<double> y = initialY;
    int last_t = 0;

    // 执行除最后一个bond外的所有操作
    for (size_t idx = 0; idx < current_bonds.size() - 1; ++idx) {
        int t = get<0>(current_bonds[idx]);
        int i = get<1>(current_bonds[idx]);
        int j = get<2>(current_bonds[idx]);

        // 更新到时刻t
        vector<double> cluster_vx(N, 0), cluster_vy(N, 0);
        for (int step = last_t; step < t; ++step) {
            unordered_map<int, pair<double, double>> centroid_v;
            unordered_map<int, int> cluster_size;
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                centroid_v[root].first += initialVx[p];
                centroid_v[root].second += initialVy[p];
                cluster_size[root]++;
            }
            for (auto& [root, v] : centroid_v) {
                cluster_vx[root] = v.first / cluster_size[root];
                cluster_vy[root] = v.second / cluster_size[root];
            }
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                x[p] = fmod(x[p] + cluster_vx[root] + L, L);
                y[p] = fmod(y[p] + cluster_vy[root] + L, L);
            }
        }

        uf.unite(i, j);
        last_t = t + 1;
    }

    // 找最小成本替代bond
    double min_cost = numeric_limits<double>::max();
    int best_t = last_t, best_i = -1, best_j = -1;
    int max_t = min(last_t + 2, T - 1);

    for (int t = last_t; t <= max_t; ++t) {
        // 更新到时刻t
        vector<double> cluster_vx(N, 0), cluster_vy(N, 0);
        for (int step = last_t; step < t; ++step) {
            unordered_map<int, pair<double, double>> centroid_v;
            unordered_map<int, int> cluster_size;
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                centroid_v[root].first += initialVx[p];
                centroid_v[root].second += initialVy[p];
                cluster_size[root]++;
            }
            for (auto& [root, v] : centroid_v) {
                cluster_vx[root] = v.first / cluster_size[root];
                cluster_vy[root] = v.second / cluster_size[root];
            }
            for (int p = 0; p < N; ++p) {
                int root = uf.find(p);
                x[p] = fmod(x[p] + cluster_vx[root] + L, L);
                y[p] = fmod(y[p] + cluster_vy[root] + L, L);
            }
        }

        // 查找最小成本点对
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if (uf.find(i) == uf.find(j)) continue;
                if (uf.get_size(i) + uf.get_size(j) > K) continue;

                double cost = calculate_bond_cost(x[i], y[i], x[j], y[j]);
                if (cost < min_cost) {
                    min_cost = cost;
                    best_t = t;
                    best_i = i;
                    best_j = j;
                }
            }
        }

        last_t = t + 1;
    }

    return make_tuple(best_t, best_i, best_j);
}

// 扰动当前解（三种方式随机选择，保证合法性）
vector<tuple<int, int, int>> perturb_solution(const vector<tuple<int, int, int>>& current_bonds,
                                              const vector<double>& initialX, const vector<double>& initialY,
                                              const vector<double>& initialVx, const vector<double>& initialVy,
                                              int T) {
    vector<tuple<int, int, int>> new_bonds = current_bonds;
    int r = rng.next(3);
    switch (r) {
        case 0: { // 调整单个bond的时间步（±1~±2）
            int idx = rng.next(BONDS_NEEDED);
            int t = get<0>(new_bonds[idx]);
            int delta = rng.next(500) - rng.next(500);
            int new_t = max(0, min(t + delta, T - 1));
            get<0>(new_bonds[idx]) = new_t;
            break;
        }
        case 1: { // 交换两个bond的时间步
            int idx1 = rng.next(BONDS_NEEDED), idx2 = rng.next(BONDS_NEEDED);
            if (idx1 == idx2) break;
            swap(get<0>(new_bonds[idx1]), get<0>(new_bonds[idx2]));
            break;
        }
        case 2: { // 替换一个bond
            new_bonds.pop_back();
            auto replacement = generate_replacement_bond(current_bonds, initialX, initialY, initialVx, initialVy, T);
            new_bonds.push_back(replacement);
            break;
        }
    }

    // 重新排序并验证合法性
    sort(new_bonds.begin(), new_bonds.end(), [](const auto& a, const auto& b) {
        return get<0>(a) < get<0>(b);
    });

    return validate_solution(new_bonds, initialVx, initialVy, T) ? new_bonds : current_bonds;
}

// 模拟退火核心算法
vector<tuple<int, int, int>> simulated_annealing(const vector<tuple<int, int, int>>& initialBonds,
                                                 const vector<double>& initialX, const vector<double>& initialY,
                                                 const vector<double>& initialVx, const vector<double>& initialVy,
                                                 int T, double& bestCost) {
    vector<tuple<int, int, int>> current_bonds = initialBonds;
    vector<tuple<int, int, int>> best_bonds = initialBonds;
    bestCost = recalculate_total_cost(current_bonds, initialX, initialY, initialVx, initialVy, T);
    double current_cost = bestCost;

    double temp = T0;
    srand(time(nullptr));

    while (runtime() < TIME_LIMIT) {
        for (int i = 0; i < L1; ++i) {
            vector<tuple<int, int, int>> new_bonds = perturb_solution(current_bonds, initialX, initialY, initialVx, initialVy, T);
            if (new_bonds == current_bonds) continue;

            double new_cost = recalculate_total_cost(new_bonds, initialX, initialY, initialVx, initialVy, T);
            double delta_E = new_cost - current_cost;

            if (delta_E < 0) {
                current_bonds = new_bonds;
                current_cost = new_cost;
                if (new_cost < bestCost) {
                    best_bonds = new_bonds;
                    bestCost = new_cost;
                    cerr << "模拟退火：找到更优解，成本=" << bestCost << "，温度=" << temp << endl;
                }
            } else if (exp(-delta_E / temp) > (double)rand() / RAND_MAX) {
                current_bonds = new_bonds;
                current_cost = new_cost;
            }
        }

        temp *= alpha;
    }

    return best_bonds;
}


int main() {

    #ifdef Local
        if (fopen("./data/0000.txt", "r") != nullptr) {
            freopen("./data/0000.txt", "r", stdin);
            freopen("./output.txt", "w", stdout);
        } else {
            cerr << "使用标准输入输出" << endl;
        }
    #endif 

    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N, T, M, K;
    double L;
    cin >> N >> T >> M >> K >> L;

    vector<double> initialX(N), initialY(N);
    vector<double> initialVx(N), initialVy(N);
    for (int i = 0; i < N; ++i)  cin >> initialX[i] >> initialY[i] >> initialVx[i] >> initialVy[i];

    vector<tuple<int, int, int>> initialBonds;
    double initialCost;

    solve(initialX, initialY, initialVx, initialVy, initialBonds, initialCost);

    cerr << "初始解生成耗时：" << runtime() << "ms" << endl;

    cerr << "\n===== 开始模拟退火优化 =====" << endl;
    double bestCost;
    vector<tuple<int, int, int>> best_bonds = simulated_annealing(initialBonds, initialX, initialY, initialVx, initialVy, T, bestCost);

    for (const auto& bond : best_bonds)  cout << get<0>(bond) << " " << get<1>(bond) << " " << get<2>(bond) << "\n";
    
    cerr << "\n===== 最终优化结果 =====" << endl;
    cerr << "最优解总成本：" << bestCost << "（初始：" << initialCost << "，优化幅度：" << (initialCost - bestCost) / initialCost * 100 << "%）" << endl;

    return 0;
}