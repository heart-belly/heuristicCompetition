

#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3)

#include "bits/stdc++.h"




using namespace std;


#define FOR(i,a,b) for (int i = (a); i < (b); i++)
#define RFOR(i,b,a) for (int i = (b) - 1; i >= (a); i--)
#define ITER(it,a) for (__typeof(a.begin()) it = a.begin(); it != a.end(); it++)
#define FILL(a,value) memset(a, value, sizeof(a))

#define SZ(a) (int)a.size()
#define ALL(a) a.begin(), a.end()
#define PB push_back
#define MP make_pair

typedef long long LL;
typedef vector<int> VI;
typedef pair<LL, LL> PII;

const double PI = acos(-1.0);
const int INF = 1000 * 1000 * 1000 + 7;
const LL LINF = INF * (LL) INF;
const int MAX_NUMBER_CONTAINER = 12000 + 10;
const int MAX_NUMBER_VM = 12000 + 10;
const int MAX_CLASS = 50;

struct VMType
{
    int cpu;
    int mem;
    int price;

    VMType(int cpu, int mem, int price)
    {
        this->cpu = cpu;
        this->mem = mem;
        this->price = price;
    }
};
struct Fraction {
    double numberator, denominator;
    double result() const {
        return numberator / max(1.0, denominator);
    }

    Fraction &operator+=(const Fraction &rhs) {
        numberator += rhs.numberator;
        denominator += rhs.denominator;

        return *this;
    }
};

// 平均运行时间
Fraction avergeRunTime{200.0, 1};
vector<VMType> vm_types;
int m, d;
unordered_set<int> never_select_vm;
unordered_set<int> container_wait_list;
vector<unordered_set<int>> cur_already_all_vms(MAX_CLASS);
vector<unordered_set<int>> cur_un_all_vms(MAX_CLASS);

// 记录每个最低价位可用的虚拟机单价
unordered_map<int, unordered_set<int>> p_to_p;
// 保存已创建类型的虚拟机数量（只要创建就记录）
vector<int> create_vm_number(MAX_CLASS);

/*保存最近一次创建请求的cpu和memory*/
int last_create_time;
int last_create_cpu, last_create_mem;

// 保存预备创建的虚拟机id
vector<int> pre_create_vms;



inline bool is_in_qj(int x) {
    if (x >=0 && x <= 10) return true;
    if (x >= 200 && x <= 350) return true;

    // if (x >= 5000 && x <= 5050) return true;
    return false;
}
inline int find_vm_type(int cpu, int mem)
{
    int best_price = INF;
    int ind = -1;
    FOR (i, 0, SZ(vm_types))
    {
        if (never_select_vm.count(i)) continue;
        if (vm_types[i].cpu >= cpu && vm_types[i].mem >= mem && vm_types[i].price < best_price)
        {
            best_price = vm_types[i].price;
            ind = i;
        }
    }

    return ind;
}

inline int find_vm_type_2(int cpu, int mem, int cur_time)
{
    int best_price = -1;
    int ind = -1;
    vector<PII> can_use_vm;
    FOR (i, 0, SZ(vm_types))
    {
        // if (never_select_vm.count(i)) continue;
        if (vm_types[i].cpu >= cpu && vm_types[i].mem >= mem)
        {
            can_use_vm.push_back(make_pair(vm_types[i].price, i));
        }
    }

    sort(can_use_vm.begin(), can_use_vm.end(), [&](const PII& p1, const PII& p2){
        return p1.first < p2.first;
    });

    int len = can_use_vm.size();

    int correct_price = vm_types[find_vm_type(cpu, mem)].price;
    int sub = 0;
    unordered_map<int, int> select;
    select[1] = 1;
    select[2] = 2;
    select[3] = 3;
    select[4] = 3;
    select[5] = 3;
    select[6] = 3;
    select[7] = 4;
    select[8] = 4;
    select[9] = 4;
    sub = select[len];

    // if (cur_time - last_create_time > 1000) {
    //     sub = 2;
    // } else if (cur_time - last_create_time > 700) {
    //     sub = 3;
    // }
    // sub = (len > 6 ? 4 : 3);
    int can_index = max(0, len - sub);
    return can_use_vm[can_index].second;
}

inline int find_vm_type_3(int cpu, int mem)
{
    int best_price = -1;
    int ind = -1;
    vector<PII> can_use_vm;
    FOR (i, 0, SZ(vm_types))
    {
        // if (never_select_vm.count(i)) continue;
        if (vm_types[i].cpu >= cpu && vm_types[i].mem >= mem)
        {
            can_use_vm.push_back(make_pair(vm_types[i].price, i));
        }
    }

    sort(can_use_vm.begin(), can_use_vm.end(), [&](const PII& p1, const PII& p2){
        return p1.first < p2.first;
    });

    int len = can_use_vm.size();

    int correct_price = vm_types[find_vm_type(cpu, mem)].price;
    unordered_map<int, int> select;
    for (int i = 1; i <= m; ++ i) {
        select[i] = i;
    };

    // 调整下选择
    // select[42] = 32;
    // select[41] = 32;
    // select[40] = 32;
    // select[39] = 32;
    // select[38] = 32;
    // select[37] = 32;
    // select[36] = 32;
    // select[35] = 30;
    // select[34] = 30;
    // select[33] = 30;
    // select[32] = 30;
    // select[31] = 30;
    int sub = select[len];
    // sub = (len > 6 ? 4 : 3);
    int can_index = max(0, len - sub);
    if (len >= 31) can_index = len - 31;
    return can_use_vm[can_index].second;
}


// 得分函数
double best_price = 1e47;
double dc, rc, tc;
struct VM
{
    int start_time;
    int end_time;
    int type;
    int cpu, memory;
    int use_cpu, use_memory;
    bool need; // 主要是来调整第一次创建后有无使用

    int hidden_use_cpu, hidden_use_memory;
    // 这里的隐藏使用的意思是（当前实际使用的cpu加上预定使用的cpu)

    int last_use; // 最近一次使用
    int free_time; // 记录最近一次的空闲时间点（当该虚拟机被预定或者正在被使用该值为-1，否则该值为认定为空闲的时刻）

    int has_container; // 当前在此虚拟机上容器的数量
    set<int> containerIds;
    VM()
    {   
        start_time = -1;
        end_time = -1;
        type = -1;
    }

    VM(int _start_time, int _end_time, int _type, int _cpu, int _memory) : start_time(_start_time), end_time(_end_time), type(_type), 
    cpu(_cpu), memory(_memory), use_cpu(0), use_memory(0), hidden_use_cpu(0), hidden_use_memory(0), last_use(0), free_time(-1), has_container(0) {

    }
};
vector<VM> vms;
int vm_index;

struct Container
{
    int start_time;
    int allocation_time;

    int shutdown_time = -1;
    int cpu, memory;

    int use_vm_id;

    Container() {
        start_time = -1;
        allocation_time = -1;
        shutdown_time = -1;
        cpu = -1;
        use_vm_id = -1;
        memory = -1;
    }

    Container(int _start_time, int _allocation_time, int _shutdown_time, int _cpu, int _memory, int _use_vm_id): start_time(_start_time), allocation_time(_allocation_time), shutdown_time(_shutdown_time),
    cpu(_cpu), memory(_memory), use_vm_id(_use_vm_id){}

};
vector<Container> containers;
inline void getBp()
{

 
    FOR (i, 0, m)
    {
        best_price = min(best_price, 1.0 * vm_types[i].price / 1e4 / vm_types[i].cpu );
    }

    //初始化容器数组
    containers = vector<Container>(MAX_NUMBER_CONTAINER);
    vms = vector<VM>(2 * MAX_NUMBER_CONTAINER);
    vm_index = 0;

    // 初始化单价
    p_to_p[143] = {229, 314, 486, 457, 629, 971, 943, 1300};
    p_to_p[229] = {314, 486, 457, 629, 971, 943, 1300};
    p_to_p[314] = {486, 457, 629, 971, 943, 1300};
    p_to_p[486] = {629, 971, 943, 1300};
    p_to_p[457] = {486, 629, 971, 943, 1300};
    p_to_p[629] = {971, 943, 1300};
    p_to_p[943] = {971, 1300};
    p_to_p[971] = {1300};
    p_to_p[1300] = {1871, 2029};
    p_to_p[1871] = {2029, 2600};
    p_to_p[2029] = {2600};
    p_to_p[4558] = {5157};
    p_to_p[6362] = {6836, 6876};
    p_to_p[6836] = {6876};
    p_to_p[8429] = {9115};
    p_to_p[9115] = {10314};
    p_to_p[12657] = {13673, 13753};
    p_to_p[13673] = {13753};
    p_to_p[16857] = {18230};
    p_to_p[18230] = {20471, 20629};
    p_to_p[20471] = {20629};
    p_to_p[27344] = {27505, 28900};
    p_to_p[27505] = {28900};

    p_to_p[40943] = {41258, 43082};
    p_to_p[54689] = {55011};

    
}


inline LL getScore() {

    return (LL)(tc * best_price / (rc + dc * 10) * 1e7);
}

/*
* @function:计算当前虚拟机在一段时间内的开销
* @return: void
*/

double getCost(int dur_time, int vm_type) {
    int price = vm_types[vm_type].price;
    return 1.0 * dur_time * price * 1e-4;
}

/*
* @function:在未准备以及准备好的虚拟机中查看指定类型的虚拟机数量
* @return: void
*/


int get_number_of_no_vm(int vm_type) {
    int number = 0;
    for (auto vm_id : cur_un_all_vms[vm_type]) {
        if (vms[vm_id].use_cpu == 0 && vms[vm_id].use_memory == 0 
        && vms[vm_id].hidden_use_cpu == 0 && vms[vm_id].hidden_use_memory == 0) number ++;
    }

    return number;
}
int get_number_of__vm(int vm_type) {
    int number = 0;
    for (auto vm_id : cur_already_all_vms[vm_type]) {
        if (vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu + last_create_cpu <= vms[vm_id].cpu && vms[vm_id].use_memory + 
        vms[vm_id].hidden_use_memory  + last_create_mem <= vms[vm_id].memory) number ++;
    }

    return number;
}

int get_total_vm() {
    int number = 0;
    for (int i = 0; i < m; ++ i) {
        number += cur_already_all_vms[i].size();
        number +=  cur_un_all_vms[i].size();
    }

    return number;
}
/*
* @function:在已经创建的虚拟机列表中寻找满足当前容器要求的虚拟机
* @return: 若能成功找到满足要求的，返回虚拟机id,若不能则返回-1
*/
// 在已经创建的且准备好虚拟机列表中寻找满足当前容器要求的虚拟机
int find_already_vm(int cpu, int mem) {

    // if (m != 42) return -1;

    // 寻找价格最低的虚拟机（贪心 + 预处理）(目前只赵最低的，并且可以将当前容器放入的即可)
    int vm_type = find_vm_type(cpu, mem);
    if (cur_already_all_vms[vm_type].empty()) {
        return -1;
    } 

    vector<int> can_use_vm;
    for (auto vm_id : cur_already_all_vms[vm_type]) {
        if (vms[vm_id].cpu >= cpu + vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu && 
        vms[vm_id].memory >= mem + vms[vm_id].use_memory + vms[vm_id].hidden_use_memory) {
            can_use_vm.emplace_back(vm_id);
        }
    }

    sort(can_use_vm.begin(), can_use_vm.end(), [&](const int &id1, const int &id2){
        return vms[id1].use_cpu + vms[id1].use_memory + vms[id1].hidden_use_cpu + vms[id1].hidden_use_memory
        > vms[id2].use_cpu + vms[id2].use_memory + vms[id2].hidden_use_memory + vms[id2].hidden_use_memory;
    });
    for (auto vm_id : can_use_vm) {
        if (vms[vm_id].cpu >= cpu + vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu && 
        vms[vm_id].memory >= mem + vms[vm_id].use_memory + vms[vm_id].hidden_use_memory) {
            // 表示找到了
            return vm_id;
        }
    }

    return -1;
}


/*
* @function:更新在当前时刻时，未准备好的虚拟机状态
* @return: void
*/
inline void update_status_un_vm(int cur_time) {
    // 遍历所有未准备好虚拟机列表(事实上只有在需要用的时候调用即可，在无事件的事件段内不需要调用)
    for (int i = 0; i < m; ++ i) {
        if (cur_un_all_vms[i].empty()) continue;
        vector<int> need_erases;
        for (auto vm_id : cur_un_all_vms[i]) {
            if (vms[vm_id].start_time + d >= cur_time) {
                // 可投入使用
                need_erases.emplace_back(vm_id);
                cur_already_all_vms[i].insert(vm_id);
            }
        }


        for (auto vm_id : need_erases) {
            cur_un_all_vms[i].erase(vm_id);
        }
    }
}

/*
* @function:在已经创建的为准备好虚拟机列表中寻找满足当前容器要求的虚拟机
* @return: 若能成功找到满足要求的，返回虚拟机id,若不能则返回-1
*/

int find_un_already_vm(int cpu, int mem, int cur_time, bool isTrue) {
    // 寻找价格最低的虚拟机（贪心 + 预处理）(目前只赵最低的，并且可以将当前容器放入的即可)
    int vm_type = find_vm_type(cpu, mem);
    if (cur_un_all_vms[vm_type].empty()) {
        return -1;
    }

    // 寻找未准备好的时候需要对已经准备的时间进行排序
    int max_time = 40, can_use_id = -1;
    for (auto vm_id : cur_un_all_vms[vm_type]) {
        if (vms[vm_id].cpu >= cpu + vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu && 
        vms[vm_id].memory >= mem + vms[vm_id].use_memory + vms[vm_id].hidden_use_memory) {
            if ((vms[vm_id].start_time + d - cur_time) < max_time) {
                can_use_id = vm_id;
                max_time = (vms[vm_id].start_time + d - cur_time);
            }
        }
    }
    if (isTrue) return max_time < 50 ? can_use_id : -1;

    return can_use_id;
    
}

/*
* @function:在以及创建并且准备好的虚拟机中寻找（其他类型）
* @return: 若能成功找到满足要求的，返回虚拟机id,若不能则返回-1
*/

int find__already_other_vm(int cpu, int mem, int cur_time, bool isTrue) {
    // 寻找价格最低的虚拟机（贪心 + 预处理）(目前只赵最低的，并且可以将当前容器放入的即可)
    int vm_type = find_vm_type(cpu, mem);



    // 先根据价格对虚拟机类型排序
    vector<int> sort_by_prices(m);
    iota(sort_by_prices.begin(), sort_by_prices.end(), 0);
    sort(sort_by_prices.begin(), sort_by_prices.end(), [&](const int& type1, const int& type2){
        return vm_types[type1].price < vm_types[type2].price;
    });

    for (int _vm_type : sort_by_prices) {
        // 对当前类型存在的虚拟机按照时间进行排序
        vector<int> can_use_vm;
        for (auto vm_id : cur_already_all_vms[_vm_type]) {
            can_use_vm.emplace_back(vm_id);
        }
        sort(can_use_vm.begin(), can_use_vm.end(), [&](const int& id1, const int& id2) {
            double rate1 = (1.0 * vms[id1].use_cpu + vms[id1].hidden_use_cpu) / vms[id1].cpu;
            double rate2 = (1.0 * vms[id2].use_cpu + vms[id2].hidden_use_cpu) / vms[id2].cpu;

            // return rate1 > rate2;
            return vms[id1].use_cpu + vms[id1].use_memory + vms[id1].hidden_use_cpu + vms[id1].hidden_use_memory
             > vms[id2].use_cpu + vms[id2].use_memory + vms[id2].hidden_use_cpu + vms[id2].hidden_use_memory;

            //  if (vms[id1].has_container != vms[id2].has_container) return vms[id1].has_container > vms[id2].has_container;
            // return vms[id1].use_cpu + vms[id1].use_memory + vms[id1].hidden_use_cpu + vms[id1].hidden_use_memory
            //  > vms[id2].use_cpu + vms[id2].use_memory + vms[id2].hidden_use_cpu + vms[id2].hidden_use_memory;
        });

        for (auto vm_id : can_use_vm) {
            if (vms[vm_id].cpu >= cpu + vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu &&
            vms[vm_id].memory >= mem + vms[vm_id].use_memory + vms[vm_id].hidden_use_memory) {

                int cur_h_price = vm_types[vm_type].price;
                int cur_use_price = vm_types[_vm_type].price;
                if (false) {
                    bool flag = false;
                    
                    if (m != 42) flag = true;

                    if (flag) {
                        return vm_id;
                    }  

                } else {
                    bool flag = false;
                    static int c = 0;
                    //统计放入当前虚拟机带来的可能结果
                    double ref = 1;
                    for (const auto &id: vms[vm_id].containerIds) {
                        int containerRunTime = cur_time - containers[id].allocation_time;
                        if (containerRunTime > 5000) {
                            ref += 1.5;
                        } else if (containerRunTime > 300 || containerRunTime < 100) {
                            ref += 1.2;
                        } else {
                            ref += 1;
                        }
                        //ref += 1;
                    }
                    double maybe_during_time = avergeRunTime.result();
                    // 如果使用自己创建的最小的
                    double add_dc_1 = (pow(1.1, 40) - 1) *1e5;
                    double add_rc_1 = (maybe_during_time + d) * vm_types[vm_type].price;
                    // 使用当前的虚拟机
                    int curD = cur_time - vms[vm_id].start_time >= d ? 0 : vms[vm_id].start_time + d - cur_time;
                    double add_dc_2 = (pow(1.1, curD) - 1) *1e5;
                    double add_rc_2 = maybe_during_time * vm_types[_vm_type].price / ref;

                    double rate = (1.0 * vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu) / vms[vm_id].cpu;
                    double r1 = (rate > 0.3 ? 1.2 : 1.2);

                    if (add_dc_1 +  r1 * add_rc_1 > add_dc_2 + add_rc_2) flag = true;
                    if (flag) {
                        return vm_id;
                    }
                }

            }
        }
    }

    return -1;
}

/*
* @function:在以及创建并且没有准备好的虚拟机中寻找（其他类型）
* @return: 若能成功找到满足要求的，返回虚拟机id,若不能则返回-1
*/

int find__un_already_other_vm(int cpu, int mem, int cur_time) {
    // 寻找价格最低的虚拟机（贪心 + 预处理）(目前只赵最低的，并且可以将当前容器放入的即可)
    int vm_type = find_vm_type(cpu, mem);

    // 先根据价格对虚拟机类型排序
    vector<int> sort_by_prices(m);
    iota(sort_by_prices.begin(), sort_by_prices.end(), 0);
    sort(sort_by_prices.begin(), sort_by_prices.end(), [&](const int& type1, const int& type2){
        return vm_types[type1].price < vm_types[type2].price;
    });

    for (int _vm_type : sort_by_prices) {
        for (auto vm_id : cur_un_all_vms[_vm_type]) {
            if (vms[vm_id].cpu >= cpu + vms[vm_id].use_cpu + vms[vm_id].hidden_use_cpu &&
            vms[vm_id].memory >= mem + vms[vm_id].use_memory + vms[vm_id].hidden_use_memory) {

                double cur_h_price = vm_types[vm_type].price;
                double cur_use_price = vm_types[_vm_type].price;
                bool flag = false;
                double d = 1.0 * (450 - pow(vms[_vm_type].start_time + d - cur_time, 1.1)) * 1e4 / (vm_types[_vm_type].price - vm_types[vm_type].price);

                if (d > 200) flag = true;
                
                // if (m != 42) flag = abs(vms[vm_id].start_time + d - cur_time) < 35;

                if (m != 42) flag = true;

                if (flag) {
                    return vm_id;
                }
            }
        }
    }

    return -1;
}
/*
* @function:删除不必要的虚拟机（在长时间无事件过程中删除）
* @return: 返回vector(要删除的虚拟机id)
*/
vector<int> need_delete_vm_1(int time_to_next_r, int accumulate_blank, int cur_time) {
    // 不进行删除

    int delete_time = 0;



    // 遍历未准备好的虚拟机
    vector<int> deles = {};
    for (int i = 0; i < m; ++ i) {
        if (cur_un_all_vms[i].empty()) continue;
        vector<int> tmp;
        // 一因为是未准备好的，所以没有进行分配，可以直接删除
        for (auto vm_id : cur_un_all_vms[i]) {
            if (vms[vm_id].hidden_use_cpu != 0 || vms[vm_id].hidden_use_memory != 0) continue;
            int vm_type = vms[vm_id].type;

            tmp.emplace_back(vm_id);
        }
        // 遍历删除
        for (auto vm_id : tmp) {
            cur_un_all_vms[i].erase(vm_id);
        }
        if (tmp.size()) {
            deles.insert(deles.end(), tmp.begin(), tmp.end());
        }
    }


    // 遍历准备好的虚拟机
    for (int i = 0; i < m; ++ i) {
        if (cur_already_all_vms[i].empty()) continue;
        vector<int> tmp;
        // 需要查看当前cpu和memroy的情况
        for (auto vm_id : cur_already_all_vms[i]) {
            if (vms[vm_id].hidden_use_cpu != 0 || vms[vm_id].hidden_use_memory != 0) continue;

            int vm_type = vms[vm_id].type;



            if (vms[vm_id].use_cpu == 0 && vms[vm_id].use_memory == 0) {
                tmp.emplace_back(vm_id);
            }
        }

        for (auto vm_id : tmp) {
            cur_already_all_vms[i].erase(vm_id);
        }

        if (tmp.size()) {
            deles.insert(deles.end(), tmp.begin(), tmp.end());
        }

    }

    return deles;
}


vector<int> need_delete_vm_2(int cur_time, int time_to_next_r, int accumulate_blank) {
    // 不进行删除

    int delete_time = 0;
    // if (time_to_next_r < delete_time) return {};


    // 遍历未准备好的虚拟机
    vector<int> deles = {};
    for (int i = 0; i < m; ++ i) {
        if (cur_un_all_vms[i].empty()) continue;
        vector<int> tmp;
        // 一因为是未准备好的，所以没有进行分配，可以直接删除
        for (auto vm_id : cur_un_all_vms[i]) {
            if (vms[vm_id].hidden_use_cpu != 0 || vms[vm_id].hidden_use_memory != 0) continue;
            int vm_type = vms[vm_id].type;
            double per_price = getCost(d, vm_type);
            int a_number = get_number_of_no_vm(vm_type);
            int b_number = get_number_of__vm(vm_type);
            
            if ( accumulate_blank < 10 ) continue;

            bool flag = (cur_time - vms[vm_id].free_time) < 20 && vm_types[vm_type].price == 2734 && b_number < 1;
            if (flag) continue;
            
            tmp.emplace_back(vm_id);
        }
        // 遍历删除
        for (auto vm_id : tmp) {
            cur_un_all_vms[i].erase(vm_id);
        }
        if (tmp.size()) {
            deles.insert(deles.end(), tmp.begin(), tmp.end());
        }
    }



    for (int i = m - 1; i >= 0; -- i) {
        if (cur_already_all_vms[i].empty()) continue;
        vector<int> tmp;
        // 需要查看当前cpu和memroy的情况
        for (auto vm_id : cur_already_all_vms[i]) {
            if (vms[vm_id].hidden_use_cpu != 0 || vms[vm_id].hidden_use_memory != 0) continue;

            int vm_type = vms[vm_id].type;
            double per_price = getCost(d, vm_type);
            int number = get_total_vm();
            if (number <= 1) continue;
            bool flag = (cur_time - vms[vm_id].free_time) < 10 && vm_types[vm_type].price == 2734 && number < 1;
            if (flag) continue;
            if ( accumulate_blank < 10 && vms[vm_id].free_time != -1 && cur_time - vms[vm_id].free_time < 10 ) continue;
   

            if (vms[vm_id].use_cpu == 0 && vms[vm_id].use_memory == 0) {
                tmp.emplace_back(vm_id);
            }
        }

        for (auto vm_id : tmp) {
            cur_already_all_vms[i].erase(vm_id);
        }

        if (tmp.size()) {
            deles.insert(deles.end(), tmp.begin(), tmp.end());
        }

    }

    return deles;
}

vector<int> need_delete_pre_vm(int cur_time) {
    vector<int> deletes = pre_create_vms;
    pre_create_vms.clear();

    return deletes;
}
PII create_vm_1(int cpu, int mem, int cur_time) {
    int vm_type = find_vm_type_3(cpu, mem);

    vms[++ vm_index] = VM(cur_time, -1, vm_type, vm_types[vm_type].cpu, vm_types[vm_type].mem);

    // 将该虚拟机放入已创建未准备好的虚拟机队列中
    cur_un_all_vms[vm_type].insert(vm_index);


    create_vm_number[vm_type] ++;
    return make_pair(vm_index, vm_type);

}

PII create_vm_2(int cpu, int mem, int cur_time) {
    int vm_type = find_vm_type_2(cpu, mem, cur_time);

    vms[++ vm_index] = VM(cur_time, -1, vm_type, vm_types[vm_type].cpu, vm_types[vm_type].mem);

    // 将该虚拟机放入已创建未准备好的虚拟机队列中
    cur_un_all_vms[vm_type].insert(vm_index);


    create_vm_number[vm_type] ++;
    return make_pair(vm_index, vm_type);

}



int main(int argc, char* argv[])
{
	ios::sync_with_stdio(false); // cin.tie(0);
    cin.tie(0);
    cout.tie(0);

    cin >> m >> d;

    FOR (i, 0, m) 
    {
        int cpu, mem, price;
        cin >> cpu >> mem >> price;

        vm_types.PB(VMType(cpu, mem, price));
    }

    getBp();

    if (m == 42) {
        never_select_vm = {0};
    }


    vector<int> vms_to_shutdown;
    map<int, vector<int>> containers_to_allocate;
    int accumulate_blank = 0;
    int t;
    cin >> t;
    for (int j = 0; j < t; ++ j) {
        int e;
        cin >> e;
        if (e == -1) return 0; // :(

        int cnt = 1;
        if (e == 0) {
            cin >> cnt;
        }

        vector<int> containers_to_shutdown;
        vector<PII> vms_to_create, container_to_vms;


        // 更新一波状态
        update_status_un_vm(j);

        if (e != 0) {
            for (int vm_id : pre_create_vms) {
                int vm_type = vms[vm_id].type;
                if (vms[vm_id].start_time + d <= j) {
                    cur_already_all_vms[vm_type].insert(vm_id);
                } else {
                    cur_un_all_vms[vm_type].insert(vm_id);
                }
            }
            
            pre_create_vms.clear();
        }

        for (int k = 0; k < e; ++ k)
        {
            int type;
            cin >> type;
            if (type == 1)
            {
                accumulate_blank = 0;
                int id, cpu, mem;
                cin >> id >> cpu >> mem;

                last_create_cpu = cpu, last_create_mem = mem;
                // 在已创建的虚拟机中查找(已经准备好的虚拟机中)
                int can_vm_id = find_already_vm(cpu, mem);
                // 在已创建的虚拟机中查找(未准备好的虚拟机中)
                if (can_vm_id == -1) {
                    can_vm_id = find_un_already_vm(cpu, mem, j, true);
                }
                // 在已创建的虚拟机中查找(其他类型中寻找)
                if (can_vm_id == -1) {
                    can_vm_id = find__already_other_vm(cpu, mem, j, true);
                }
   
                if (can_vm_id == -1 ) {
                    can_vm_id = find__already_other_vm(cpu, mem, j, false);
                }
                if (can_vm_id == -1) {
                    can_vm_id = find_un_already_vm(cpu, mem, j, false);
                }
                if (can_vm_id == -1) {
                    can_vm_id = find__un_already_other_vm(cpu, mem, j);
                }
                // 若在已有的虚拟机中不满足查找要求，则在创建一个vm(这个vm可能提前创建 -- 40)
                if (can_vm_id == -1) {
                    if (m == 42) {
                        PII points = create_vm_1(cpu, mem, j);
                        vms_to_create.push_back(points);

                        can_vm_id = points.first;
                    } else {
                        PII points = create_vm_2(cpu, mem, j);
                        vms_to_create.push_back(points);

                        can_vm_id = points.first;
                    }

                    last_create_time = j;
                }

                
                // 在该虚拟机上加上隐藏使用的cpu和mem防止其他容器抢夺(在正式分配的时候以及当前容器转移虚拟机时释放)
                vms[can_vm_id].hidden_use_cpu += cpu, vms[can_vm_id].hidden_use_memory += mem;

                vms[can_vm_id].free_time = -1;
                // 更新该容器的相关参数，该容器放入等待虚拟机列表中（默认给该容器指定一个 虚拟机id, 但是不一定使用)
                // 经过上面的三个筛选一定可以找到待使用的虚拟机id
                container_wait_list.insert(id);
                // 放入容器数组中
                containers[id] = Container(j, -1, -1, cpu, mem, can_vm_id);

            }
            if (type == 2)
            {
                int id;
                cin >> id;

                containers_to_shutdown.push_back(id);

                containers[id].shutdown_time = j;
                vms[containers[id].use_vm_id].has_container --;
                vms[containers[id].use_vm_id].containerIds.erase(id);
                // 统计分数指标
                tc += 1.0 * (containers[id].shutdown_time - containers[id].allocation_time) * containers[id].cpu;

                avergeRunTime += {1.0 * containers[id].shutdown_time - containers[id].start_time, 1.0 };
            }
        }

        j -- ;
        // 分有事件和无事件来执行动作
        if (e != 0) {
            j ++;
            // update_status_un_vm(j);
            // 将可能预创建的虚拟机加入到列表中
            // 遍历删除容器列表
            // 当vm满足删除要求，则进行删除（这里的要求不一定只满足vm为空，要考虑到后面创建的容器可能需要放入， 在一段可能连续的请求中，对于为空的vm不进行删除，可以为容器的创建请求来满足要求）
            vector<int> maybe_use_vm;
            for (auto container_id : containers_to_shutdown) {

                // 更新虚拟机的可用cpu和memory
                int vm_id = containers[container_id].use_vm_id;
                vms[vm_id].use_cpu -= containers[container_id].cpu;
                vms[vm_id].use_memory -= containers[container_id].memory;


                if (vms[vm_id].use_cpu == 0 && vms[vm_id].use_memory == 0 && vms[vm_id].hidden_use_cpu == 0 && vms[vm_id].hidden_use_memory == 0) vms[vm_id].free_time = j;
                maybe_use_vm.emplace_back(vm_id);
            }

            // 遍历待容器等待列表
            // 若当前容器所需vm满足可放入条件，则进行放入（由于我可能有相应的容器删除操作，所以我把现有虚拟机的遍历和是否创建新的虚拟机来满足请求放在这里，注意删除之后不能在同一时刻申请虚拟机）
            // 先为容器预分配虚拟机
            vector<int> deletes;
            // vector<int> 
            for (auto container_id : container_wait_list) {

                // 得到容器预分配的vm
                int vm_id = containers[container_id].use_vm_id;
                if (vms[vm_id].start_time + d <= j) {
                    // 说明该虚拟机是可以使用的(在这个时刻将容器分配到虚拟机上)
                    int cpu = containers[container_id].cpu, mem = containers[container_id].memory;
                    // 更新虚拟机相关数据结构
                    vms[vm_id].use_cpu += cpu, vms[vm_id].use_memory += mem;
                    vms[vm_id].hidden_use_cpu -= cpu, vms[vm_id].hidden_use_memory -= mem;

                    vms[vm_id].has_container ++;
                    vms[vm_id].containerIds.insert(container_id);
                    // 更新容器相关数据结构
                    containers[container_id].allocation_time = j;
                    container_to_vms.push_back(make_pair(container_id, vm_id));

                    // 统计指标
                                    // 统计分数指标
                    dc += pow(1.1, containers[container_id].allocation_time -  containers[container_id].start_time ) - 1;
                    
                    deletes.emplace_back(container_id);

                    // 更新最近一次的访问请求
                    vms[vm_id].last_use = j;
                    vms[vm_id].free_time = -1;
                }
            }

            // 在等待列表中删除
            for (auto container_id : deletes) {
                container_wait_list.erase(container_id);
            }

            // 由于可能存在删除的操作，可能能把正在等待vm的容器分配到释放资源的虚拟机中

            if (!maybe_use_vm.empty() && !container_wait_list.empty()) {

                sort(maybe_use_vm.begin(), maybe_use_vm.end(), [&](const int& vm_id1, const int& vm_id2){
                    int type1 = vms[vm_id1].type;
                    int type2 = vms[vm_id2].type;

                    // return vm_types[type1].price < vm_types[type2].price;

                    return vms[vm_id1].last_use > vms[vm_id2].last_use;
                });

                // 先遍历container_wait_list（处理大请求）
                vector<int> wait_to_tmp(container_wait_list.size());
                int itr = 0;
                for (auto container_id : container_wait_list) {
                    wait_to_tmp[itr ++] = container_id;
                }
                sort(wait_to_tmp.begin(), wait_to_tmp.end(), [&](const int& id1, const int& id2){
                    return containers[id1].cpu + containers[id1].memory > containers[id2].cpu + containers[id2].memory;
                });

                // 先遍历container_wait_list
                for (auto container_id : wait_to_tmp) {
                    // 再遍历可能可以使用的虚拟机
                    for (auto vm_id : maybe_use_vm) {

                        int really_type = vms[vm_id].type;
                        int exp_type = find_vm_type(containers[container_id].cpu, containers[container_id].memory);

                        if (really_type == exp_type) {
                            // 无条件满足要求
                            if (vms[vm_id].use_cpu + containers[container_id].cpu + vms[vm_id].hidden_use_cpu <= vms[vm_id].cpu
                            && vms[vm_id].use_memory + containers[container_id].memory + vms[vm_id].hidden_use_memory <= vms[vm_id].memory) {
                                // 使用
                                // 更新容器相关数据结构(改变容器的预分配虚拟机id)
                                int old_vm_id = containers[container_id].use_vm_id;

                                containers[container_id].use_vm_id = vm_id;

                                // 更新隐藏使用的cpu和memory

                                vms[old_vm_id].hidden_use_cpu -= containers[container_id].cpu;
                                vms[old_vm_id].hidden_use_memory -= containers[container_id].memory;

                                vms[vm_id].hidden_use_cpu += containers[container_id].cpu;
                                vms[vm_id].hidden_use_memory += containers[container_id].memory;
    
                            }
                        } else if (true) {
                            if (vms[vm_id].use_cpu + containers[container_id].cpu + vms[vm_id].hidden_use_cpu <= vms[vm_id].cpu
                            && vms[vm_id].use_memory + containers[container_id].memory + vms[vm_id].hidden_use_memory <= vms[vm_id].memory) {

                                double d = 1.0 * 450 * 1e4 / (vm_types[really_type].price - vm_types[exp_type].price + 1.0);

                                if (d > 3000 || m != 42) {
                                    double cur_h_price = vm_types[exp_type].price;
                                    double cur_use_price = vm_types[really_type].price;


                                    int old_vm_id = containers[container_id].use_vm_id;

                                    containers[container_id].use_vm_id = vm_id;

                                    // 更新隐藏使用的cpu和memory

                                    vms[old_vm_id].hidden_use_cpu -= containers[container_id].cpu;
                                    vms[old_vm_id].hidden_use_memory -= containers[container_id].memory;

                                    vms[vm_id].hidden_use_cpu += containers[container_id].cpu;
                                    vms[vm_id].hidden_use_memory += containers[container_id].memory;                                    
                                }

                            }
                        }
                    }
                }
            }


            // 输出对应的请求
            cout << (int)(vms_to_create.size() + container_to_vms.size()) << "\n";
            for (auto [vm_id, vm_type] : vms_to_create) {
                cout << "1 " << vm_id << " " << vm_type + 1 << "\n";
            }
            for (auto [container_id, vm_id] :  container_to_vms) {
                cout << "3 " << container_id << " " << vm_id << "\n";
            }
 


        } else {
            // 无事件动作
            // 在（0，40）中每次循环都需要遍历待容器等待列表（因为我对于上一次事件中创建的容器，可能需要创建新的vm，所以最快的是用现有的，或者等待创建时间d==40,所以无事件的[1, 40]我都需要遍历待分配列表
            // cnt : 1 - 1000
            // 在第一个无事件的时间点就执行必要的删除操作
            // 需要删除的虚拟机
            /********************************************************************************************/
            accumulate_blank ++;
            j ++;
            vector<int> delets = (m == 42 ? need_delete_vm_1(cnt, accumulate_blank, j) : need_delete_vm_2(j, cnt, accumulate_blank)) ;
            vector<int> deletes_tmp = need_delete_pre_vm(j);
            delets.insert(delets.end(), deletes_tmp.begin(), deletes_tmp.end());

            vector<int> deletes;
            container_to_vms.clear();
            // 遍历待容器等待列表
            for (auto container_id : container_wait_list) {
                // 得到容器预分配的vm
                int vm_id = containers[container_id].use_vm_id;
                if (vms[vm_id].start_time + d <= j) {
                    // 说明该虚拟机是可以使用的(在这个时刻将容器分配到虚拟机上)
                    int cpu = containers[container_id].cpu, mem = containers[container_id].memory;
                    // 更新虚拟机相关数据结构
                    vms[vm_id].use_cpu += cpu, vms[vm_id].use_memory += mem;
                    vms[vm_id].hidden_use_cpu -= cpu, vms[vm_id].hidden_use_memory -= mem;
                    vms[vm_id].has_container ++;
                    vms[vm_id].containerIds.insert(container_id);
                    // 更新容器相关数据结构
                    containers[container_id].allocation_time = j;
                    container_to_vms.push_back(make_pair(container_id, vm_id));

                    // 统计指标
                    // 统计分数指标
                    dc += pow(1.1, containers[container_id].allocation_time -  containers[container_id].start_time ) - 1;
                    deletes.emplace_back(container_id);

                    // 更新虚拟机的使用时间
                    vms[vm_id].last_use = j;
                    vms[vm_id].free_time = -1;
                }  
            }

            for (auto container_id : deletes) {
                container_wait_list.erase(container_id);
            }

            cout << (int) (delets.size() + container_to_vms.size() ) << "\n";
            // 判断有无需要创建请求（慎用）
            // 部署分配请求
            for (auto [container_id, vm_id] : container_to_vms) {
                cout << "3 " << container_id << " " << vm_id << "\n";
            }
            // 执行删除请求
            for (int vm_id : delets) {
                // 统计分数指标
                vms[vm_id].end_time = j;
                rc += 1.0 * (vms[vm_id].end_time - vms[vm_id].start_time) * vm_types[vms[vm_id].type].price * 1e-4;

                cout << "2 " << vm_id << "\n";
            }

            /********************************************************************************************/
            for (int k = 1; k < min(d, cnt); ++ k) {
                accumulate_blank ++;
                j ++;

                vector<int> delets = (m == 42 ? need_delete_vm_1(cnt, accumulate_blank, j) : need_delete_vm_2(j, cnt, accumulate_blank));
                vector<int> deletes;
                container_to_vms.clear();
                // 遍历待容器等待列表
                for (auto container_id : container_wait_list) {
                    // 得到容器预分配的vm
                    int vm_id = containers[container_id].use_vm_id;
                    if (vms[vm_id].start_time + d <= j) {
                        // 说明该虚拟机是可以使用的(在这个时刻将容器分配到虚拟机上)
                        int cpu = containers[container_id].cpu, mem = containers[container_id].memory;
                        // 更新虚拟机相关数据结构
                        vms[vm_id].use_cpu += cpu, vms[vm_id].use_memory += mem;
                        vms[vm_id].hidden_use_cpu -= cpu, vms[vm_id].hidden_use_memory -= mem;
                        vms[vm_id].has_container ++;
                        vms[vm_id].containerIds.insert(container_id);
                        // 更新容器相关数据结构
                        containers[container_id].allocation_time = j;
                        container_to_vms.push_back(make_pair(container_id, vm_id));

                        // 统计分数指标
                        dc += pow(1.1, containers[container_id].allocation_time -  containers[container_id].start_time ) - 1;

                        deletes.emplace_back(container_id);

                        // 更新虚拟机的使用时间
                        vms[vm_id].last_use = j;
                        vms[vm_id].free_time = -1;
                    }  
                }

                for (auto container_id : deletes) {
                    container_wait_list.erase(container_id);
                }

                // 判断有无需要创建请求（慎用）

                vector<int> pre_create_tmp;
                // if (pre_create_vms.empty() && is_in_qj(accumulate_blank) )  {
                    
                //     int number = get_total_vm();

                //     // 得到上一次请求的cpu和memory
                //     int last_vm_type = find_vm_type(last_create_cpu, last_create_mem);

                    
                //     int vm_type = min(last_vm_type + 1, m - 1);
                //     if (number <= 1) {
                //         // 创建类型为5的虚拟机（固定）
                //         vms[++ vm_index] = VM(j, -1, vm_type, vm_types[vm_type].cpu, vm_types[vm_type].mem);

                //         // // 将该虚拟机放入已创建未准备好的虚拟机队列中
                //         // cur_un_all_vms[vm_type].insert(vm_index);


                //         create_vm_number[vm_type] ++;

                //         // 将该虚拟机放入pre_create_vm 中
                //         pre_create_tmp.emplace_back(vm_index);
                //         pre_create_vms.emplace_back(vm_index);

                        
                //     }
                // }

                // 处理请求

                cout << (int) (delets.size() + container_to_vms.size() + pre_create_tmp.size()) << "\n";
                // 部署分配请求
                for (auto [container_id, vm_id] : container_to_vms) {
                    cout << "3 " << container_id << " " << vm_id << "\n";
                }
                // 执行删除请求
                for (int vm_id : delets) {
                    // 统计分数指标
                    vms[vm_id].end_time = j;
                    rc += 1.0 * (vms[vm_id].end_time - vms[vm_id].start_time) * vm_types[vms[vm_id].type].price * 1e-4;

                    cout << "2 " << vm_id << "\n";
                }

                // 执行可能的创建请求
                for (int vm_id : pre_create_tmp) {
                    cout << "1 " << vm_id << " " << vms[vm_id].type + 1 << "\n";
                }
 
            }

            /********************************************************************************************/
            // 中后半段(40, cnt - 40)
            bool flag = true;
            for (int k = min(d, cnt); k < cnt; ++ k) {
                j ++;
                if (cnt - k <= d && flag) {
                    flag = false; // 表示第一次进行后半段区间
                    // 允许提前创建一些虚拟机
                    cout << "0" << "\n";
                } else {

                    cout << "0" << "\n";
                } 
            }
    
        }

        cout.flush();
    }




    int e;
    cin >> e;
    if (e == 0)
    {
        // :)
    }
    else
    {
        // :(
    }


}










