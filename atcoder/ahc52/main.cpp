
#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <cmath>
#include <climits>
#include <chrono>
using namespace std;

#define endl '\n'
#define LIMIT 1800
const auto startTime = std::chrono::steady_clock::now();

int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template<> struct hash<Point> {
        size_t operator()(const Point& p) const {
            return p.x * 100 + p.y;
        }
    };
}

// 计算两点间的曼哈顿距离（用于快速筛选最近点）
int manhattanDistance(const Point& a, const Point& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

void pressButton(int button, vector<Point>& robots, vector<vector<bool>>& covered,
                const vector<string>& v_walls, const vector<string>& h_walls, int N) {
    for (int i = 0; i < robots.size(); i++) {
        Point& p = robots[i];
        int new_x = p.x, new_y = p.y;
        if (button == 0) {
            if (p.y < N-1 && v_walls[p.x][p.y] == '0') {
                new_y = p.y + 1;
            }
        } else if (button == 1) {
            if (p.y > 0 && v_walls[p.x][p.y-1] == '0') {
                new_y = p.y - 1;
            }
        } else if (button == 2) {
            if (p.x > 0 && h_walls[p.x-1][p.y] == '0') {
                new_x = p.x - 1;
            }
        } else if (button == 3) {
            if (p.x < N-1 && h_walls[p.x][p.y] == '0') {
                new_x = p.x + 1;
            }
        } else if (button == 4) {
            if (i == 0 && p.y < N-1 && v_walls[p.x][p.y] == '0') {
                new_y = p.y + 1;
            }
        } else if (button == 5) {
            if (i == 0 && p.y > 0 && v_walls[p.x][p.y-1] == '0') {
                new_y = p.y - 1;
            }
        } else if (button == 6) {
            if (i == 0 && p.x > 0 && h_walls[p.x-1][p.y] == '0') {
                new_x = p.x - 1;
            }
        } else if (button == 7) {
            if (i == 0 && p.x < N-1 && h_walls[p.x][p.y] == '0') {
                new_x = p.x + 1;
            }
        } else if (button == 8) {
            if (i == 1 && p.y < N-1 && v_walls[p.x][p.y] == '0') {
                new_y = p.y + 1;
            }
        } else if (button == 9) {
            if (i == 1 && p.y > 0 && v_walls[p.x][p.y-1] == '0') {
                new_y = p.y - 1;
            }
        }
        p.x = new_x;
        p.y = new_y;
        if (!covered[new_x][new_y]) {
            covered[new_x][new_y] = true;
        }
    }
}

vector<int> findPath(Point start, Point end, const vector<string>& v_walls, const vector<string>& h_walls, int N) {
    vector<vector<Point>> parent(N, vector<Point>(N, Point(-1, -1)));
    vector<vector<int>> buttonUsed(N, vector<int>(N, -1));
    queue<Point> q;
    q.push(start);
    parent[start.x][start.y] = start;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    int buttons[] = {4, 5, 7, 6};

    while (!q.empty()) {
        Point current = q.front();
        q.pop();
        if (current.x == end.x && current.y == end.y) {
            break;
        }
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            if (nx < 0 || nx >= N || ny < 0 || ny >= N) continue;
            if (i == 0 && v_walls[current.x][current.y] == '1') continue;
            if (i == 1 && current.y > 0 && v_walls[current.x][current.y-1] == '1') continue;
            if (i == 2 && h_walls[current.x][current.y] == '1') continue;
            if (i == 3 && current.x > 0 && h_walls[current.x-1][current.y] == '1') continue;
            if (parent[nx][ny].x == -1) {
                parent[nx][ny] = current;
                buttonUsed[nx][ny] = buttons[i];
                q.push(Point(nx, ny));
            }
        }
    }

    vector<int> path;
    Point current = end;
    while (!(current.x == start.x && current.y == start.y)) {
        int button = buttonUsed[current.x][current.y];
        if (button == -1) break;
        path.push_back(button);
        current = parent[current.x][current.y];
    }
    reverse(path.begin(), path.end());
    return path;
}
// 辅助：修改findPath函数以支持指定机器人（使用对应机器人的按钮）
vector<int> findPath(int robot_idx, Point start, Point end, 
                    const vector<string>& v_walls, const vector<string>& h_walls, int N) {
    vector<vector<Point>> parent(N, vector<Point>(N, Point(-1, -1)));
    vector<vector<int>> buttonUsed(N, vector<int>(N, -1));
    queue<Point> q;
    q.push(start);
    parent[start.x][start.y] = start;

    int dx[] = {0, 0, 1, -1}; // 右、左、下、上
    int dy[] = {1, -1, 0, 0};
    int buttons[3][4] = {
        {4, 5, 7, 6},  // 机器人0的按钮：右4、左5、下7、上6
        {8, 9, 3, 2},  // 机器人1的按钮：右8、左9、下3、上2（下/上无单独按钮，用群体按钮）
        {0, 1, 3, 2}   // 其他机器人：用群体按钮
    };
    // 选择当前机器人对应的按钮组
    int* btn_map = buttons[min(robot_idx, 2)];

    while (!q.empty()) {
        Point current = q.front();
        q.pop();
        if (current.x == end.x && current.y == end.y) {
            break;
        }
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            if (nx < 0 || nx >= N || ny < 0 || ny >= N) continue;
            // 墙判断（与原逻辑一致）
            if (i == 0 && v_walls[current.x][current.y] == '1') continue;
            if (i == 1 && current.y > 0 && v_walls[current.x][current.y-1] == '1') continue;
            if (i == 2 && h_walls[current.x][current.y] == '1') continue;
            if (i == 3 && current.x > 0 && h_walls[current.x-1][current.y] == '1') continue;
            if (parent[nx][ny].x == -1) {
                parent[nx][ny] = current;
                buttonUsed[nx][ny] = btn_map[i];
                q.push(Point(nx, ny));
            }
        }
    }

    vector<int> path;
    Point current = end;
    while (!(current.x == start.x && current.y == start.y)) {
        int button = buttonUsed[current.x][current.y];
        if (button == -1) break;
        path.push_back(button);
        current = parent[current.x][current.y];
    }
    reverse(path.begin(), path.end());
    return path;
}
inline int getStep(const vector<int>& sequence) {
    return sequence.size();
}
inline int getScore(const vector<int>& sequence, const vector<vector<bool>> st) {
    int cnt = 0;
    for (int i = 0; i < st.size(); ++ i) {
        for (int j = 0; j < st[0].size(); ++ j) {
            if (st[i][j]) ++ cnt;
        }
    }
    if (cnt == 900) return 2700 - sequence.size();

    return cnt;
}

struct Strategy {
    int N, M, K;
    
    vector<Point> robot_init_pos;
    vector<string> v_walls;
    vector<string> h_walls;
    vector<string> button_config;
    vector<int> bestResult;
    int bestScore;
    void init() {
        cin >> N >> M >> K;
        for (int i = 0; i < M; i++) {
            int x, y;
            cin >> x >> y;
            robot_init_pos.emplace_back(x, y);
        }
        v_walls.resize(N);
        for (int i = 0; i < N; i++) {
            cin >> v_walls[i];
        }
        h_walls.resize(N - 1);
        for (int i = 0; i < N-1; i++) {
            cin >> h_walls[i];
        }
        button_config.resize(K);

        for (int r = 0; r < M; r++) {
            button_config[0] += "R";
            if (r != M-1) button_config[0] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[1] += "L";
            if (r != M-1) button_config[1] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[2] += "U";
            if (r != M-1) button_config[2] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[3] += "D";
            if (r != M-1) button_config[3] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[4] += (r == 0) ? "R" : "S";
            if (r != M-1) button_config[4] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[5] += (r == 0) ? "L" : "S";
            if (r != M-1) button_config[5] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[6] += (r == 0) ? "U" : "S";
            if (r != M-1) button_config[6] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[7] += (r == 0) ? "D" : "S";
            if (r != M-1) button_config[7] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[8] += (r == 1) ? "R" : "S";
            if (r != M-1) button_config[8] += " ";
        }
        for (int r = 0; r < M; r++) {
            button_config[9] += (r == 1) ? "L" : "S";
            if (r != M-1) button_config[9] += " ";
        }

        for (const string& btn : button_config) {
            cout << btn << endl;
        }
    }

    void plan(vector<Point>& robots, vector<int>& resultSequence, vector<vector<bool>>& covered, int cycleQ) {

        vector<Point> backupRobots = robots;
        vector<int> backupSequence = resultSequence;
        vector<vector<bool>> backupCovered = covered;

        vector<Point> bestRobots;
        vector<int> bestSequence;
        vector<vector<bool>> bestCovered;
        // 全局随机寻找
        int bestScore = 0;
        // 4个方向
        // 从上到下，从下到上，从左到右，从右到左
        int d[4][3] = {{0, 1, 3}, {0, 1, 2}, {3, 2, 1}, {3, 2, 0}};
        for (int i = 0; i < 4; ++ i) {
            for (int cycle = 0; cycle < cycleQ; ++ cycle) {
                // 选择需要滑动次数最多的最为下届
                int maxPos = 0;
                int finalStep = 0;
                for (const auto& robot : robots) {
                    // 向d[i][0]滑行
                    if (d[i][0] == 0) {
                        // 向右滑行
                        for (maxPos = robot.y; maxPos < N && v_walls[robot.x][maxPos] == '0'; ++ maxPos) {}
                        finalStep = max(finalStep, maxPos - robot.y);
                    } else if (d[i][0] == 3) {
                        // 向下滑行
                        for (maxPos = robot.x; maxPos < N - 1 && h_walls[maxPos][robot.y] == '0'; ++ maxPos) {
                        }
                        finalStep = max(finalStep, maxPos - robot.x);
                    }
                }
                for (int step = 0; step < finalStep; ++ step) {
                    resultSequence.emplace_back(d[i][0]);
                    pressButton(d[i][0], robots, covered, v_walls, h_walls, N);
                }
                resultSequence.emplace_back(d[i][2]);
                pressButton(d[i][2], robots, covered, v_walls, h_walls, N);
                finalStep = 0;
                for (const auto& robot : robots) {
                    // 向d[i][0]滑行
                    if (d[i][1] == 1) {
                        // 向右滑行
                        for (maxPos = robot.y; maxPos > 0 && v_walls[robot.x][maxPos - 1] == '0'; -- maxPos) {
                            
                        }
                        finalStep = max(finalStep, robot.y - maxPos);
                    } else if (d[i][1] == 2) {
                        // 向上滑行
                        for (maxPos = robot.x; maxPos > 0 ; -- maxPos) {}
                        finalStep = max(finalStep, robot.x - maxPos);
                    }
                }
                for (int step = 0; step < finalStep; ++ step) {
                    resultSequence.emplace_back(d[i][1]);
                    pressButton(d[i][1], robots, covered, v_walls, h_walls, N);
                }
                resultSequence.emplace_back(d[i][2]);
                pressButton(d[i][2], robots, covered, v_walls, h_walls, N);

            }

            //得到当前分数
            int score = getScore(resultSequence, covered);
            if (score > bestScore) {
                bestScore = score;
                // 接受这个解
                bestRobots = move(robots);
                bestSequence = move(resultSequence);
                bestCovered = move(covered);
            }

            robots = backupRobots;
            resultSequence = backupSequence;
            covered = backupCovered;

        }

        // 接受最优解
        resultSequence = move(bestSequence);
        robots = move(bestRobots);
        covered = move(bestCovered);






    }
    void plan2(vector<Point>& robots, vector<int>& resultSequence, vector<vector<bool>>& covered) {

        vector<Point> backupRobots = robots;
        vector<int> backupSequence = resultSequence;
        vector<vector<bool>> backupCovered = covered;

        vector<Point> bestRobots;
        vector<int> bestSequence;
        vector<vector<bool>> bestCovered;
        // 全局随机寻找
        int bestScore = 0;
        // 4个方向
        // 从上到下，从下到上，从左到右，从右到左
        int d[4][3] = {{0, 1, 3}, {0, 1, 2}, {3, 2, 1}, {3, 2, 0}};
        for (int i = 0; i < 4; ++ i) {
            
            for (int cycle = 0; cycle < 4; ++ cycle) {
                // 选择需要滑动次数最多的最为下届
                int maxPos = 0;
                int finalStep = 0;
                for (const auto& robot : robots) {
                    // 向d[i][0]滑行
                    if (d[i][0] == 0) {
                        // 向右滑行
                        for (maxPos = robot.y; maxPos < N && v_walls[robot.x][maxPos] == '0'; ++ maxPos) {}
                        finalStep = max(finalStep, maxPos - robot.y);
                    } else if (d[i][0] == 3) {
                        // 向下滑行
                        for (maxPos = robot.x; maxPos < N - 1 && h_walls[maxPos][robot.y] == '0'; ++ maxPos) {
                        }
                        finalStep = max(finalStep, maxPos - robot.x);
                    }
                }
                for (int step = 0; step < finalStep; ++ step) {
                    resultSequence.emplace_back(d[i][0]);
                    pressButton(d[i][0], robots, covered, v_walls, h_walls, N);
                }
                resultSequence.emplace_back(d[i][2]);
                pressButton(d[i][2], robots, covered, v_walls, h_walls, N);
                finalStep = 0;
                for (const auto& robot : robots) {
                    // 向d[i][0]滑行
                    if (d[i][1] == 1) {
                        // 向右滑行
                        for (maxPos = robot.y; maxPos > 0 && v_walls[robot.x][maxPos - 1] == '0'; -- maxPos) {
                            
                        }
                        finalStep = max(finalStep, robot.y - maxPos);
                    } else if (d[i][1] == 2) {
                        // 向上滑行
                        for (maxPos = robot.x; maxPos > 0 ; -- maxPos) {}
                        finalStep = max(finalStep, robot.x - maxPos);
                    }
                }
                for (int step = 0; step < finalStep; ++ step) {
                    resultSequence.emplace_back(d[i][1]);
                    pressButton(d[i][1], robots, covered, v_walls, h_walls, N);
                }
                resultSequence.emplace_back(d[i][2]);
                pressButton(d[i][2], robots, covered, v_walls, h_walls, N);

            }

            //得到当前分数
            int score = getScore(resultSequence, covered);
            if (score > bestScore) {
                bestScore = score;
                // 接受这个解
                cerr << "接受" << endl;
                bestRobots = move(robots);
                bestSequence = move(resultSequence);
                bestCovered = move(covered);
            }

            robots = backupRobots;
            resultSequence = backupSequence;
            covered = backupCovered;

            cerr << "全局遍历得到的分数: " << score << endl; 
        }

        // 接受最优解
        resultSequence = move(bestSequence);
        robots = move(bestRobots);
        covered = move(bestCovered);






    }
    void output() {
        for (int op : bestResult) cout << op << endl;
    }
    void mainLoop() {
        for (int i = 0; i < 10; ++ i) {
            if (runtime() > LIMIT) break;
            cerr << "全部扫描: " << i << " ************" << endl;
            solve(i);
        }
        cerr << "bestScore: " << bestScore << endl;
    }

    void solve(int cycle) {

        vector<Point> robots = robot_init_pos;
        vector<vector<bool>> covered(N, vector<bool>(N, false));
        for (auto& p : robots) {
            covered[p.x][p.y] = true;
        }

        vector<int> global_sequence;
        plan(robots, global_sequence, covered, cycle);
        // plan2(robots, global_sequence, covered);
        // 收集未覆盖的格子
        vector<Point> uncovered;
        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                if (!covered[x][y]) {
                    uncovered.emplace_back(x, y);
                }
            }
        }
        cerr << "未覆盖的个数: " << uncovered.size() << endl;

        // 补漏阶段：采用就近原则，每次选择最近的未覆盖格子
        vector<int> extra_sequence;
        Point current_pos = robots[0]; // 机器人当前位置
        const int MAX_STEPS = 1800;
        int remaining_steps = MAX_STEPS - global_sequence.size(); // 剩余可用步骤
        // // 当还有未覆盖格子且有剩余步骤时，继续补漏
        // 当还有未覆盖格子且有剩余步骤时，继续补漏
        while (!uncovered.empty() && remaining_steps > 0) {
            // 1. 过滤已被覆盖的点（可能在全局阶段已覆盖）
            vector<Point> valid_uncovered;
            for (const auto& p : uncovered) {
                if (!covered[p.x][p.y]) {
                    valid_uncovered.push_back(p);
                }
            }
            uncovered.swap(valid_uncovered);
            if (uncovered.empty()) break;

            // 2. 对每个未覆盖点，找到最近的机器人（计算所有机器人到该点的距离）
            int best_point_idx = -1;  // 最优目标点索引
            int best_robot_idx = -1;  // 处理该点的最优机器人索引
            int min_total_dist = INT_MAX;

            for (size_t i = 0; i < uncovered.size(); i++) {
                const Point& target = uncovered[i];
                // 计算每个机器人到当前目标点的距离
                for (size_t r = 0; r < robots.size(); r++) {
                    const Point& robot_pos = robots[r];
                    int dist = manhattanDistance(robot_pos, target);
                    // 找到全局最近的（机器人-目标点）组合
                    if (dist < min_total_dist) {
                        min_total_dist = dist;
                        best_point_idx = i;
                        best_robot_idx = r;
                    }
                }
            }

            if (best_point_idx == -1 || best_robot_idx == -1) break;

            // 3. 获取最优目标点和对应的机器人
            Point target = uncovered[best_point_idx];
            int robot_idx = best_robot_idx;
            Point robot_start = robots[robot_idx];

            // 4. 生成该机器人到目标点的路径（需适配不同机器人的按钮）
            vector<int> path = findPath(robot_idx, robot_start, target, v_walls, h_walls, N);

            // 5. 裁剪路径（确保不超过剩余步骤）
            if (path.size() > (size_t)remaining_steps) {
                path.resize(remaining_steps);
            }
            if (path.empty()) {
                // 路径为空（已在目标点或无法到达），移除该点
                uncovered.erase(uncovered.begin() + best_point_idx);
                continue;
            }

            // 6. 添加路径到补漏序列
            extra_sequence.insert(extra_sequence.end(), path.begin(), path.end());
            remaining_steps -= path.size();

            // 7. 执行路径，更新机器人位置和覆盖状态
            for (int btn : path) {
                pressButton(btn, robots, covered, v_walls, h_walls, N);
            }

            // 8. 从列表中移除已处理的目标点
            uncovered.erase(uncovered.begin() + best_point_idx);
        }
        // 合并序列并输出（确保总步骤≤1800）
        vector<int> operation_sequence = global_sequence;
        operation_sequence.insert(operation_sequence.end(), extra_sequence.begin(), extra_sequence.end());
        // 修改步骤数量
        if (operation_sequence.size() > 1800) operation_sequence.resize(1800);

        cerr << "final Step : " << getStep(operation_sequence) << endl;
        cerr << "final Score : " << getScore(operation_sequence, covered) << endl;

        if (getScore(operation_sequence, covered) > bestScore) {
            bestScore = getScore(operation_sequence, covered);
            bestResult = operation_sequence;
            cerr << "提高后: " << bestScore << endl;
        }


    }
};


int main() {
    if (fopen("./data/0005.txt", "r") != nullptr) {
        freopen("./data/0000.txt", "r", stdin);
        freopen("./output.txt", "w", stdout);
    } else {
        cerr << "open file incorrectly" << endl;
    }
    std::ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);

    static Strategy strategy;
    strategy.init();
    strategy.mainLoop();
    strategy.output();


    return 0;
}
    
