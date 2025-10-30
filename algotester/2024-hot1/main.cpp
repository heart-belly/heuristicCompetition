#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <queue>
#include <vector>
#include <algorithm>
using namespace std;

const int BILLION = 1000000000;
const int MAX_RUNNING_TIME = 1000;//留50ms阈值
typedef pair<double, int> PDI;


#ifndef LOCAL
#define LOCAL
#endif 

#ifndef SCORE
#define SCORE
#endif 
// 计算程序开始时间
const auto startTime = std::chrono::steady_clock::now();

 int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}
// 随机数生成器
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
    //  int next(int x) {return rand() % x;}
     int next(int a, int b) {return a + (rand() % (b - a));}
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 
static RNG rng;

typedef pair<int, int> PII;
struct ComputeUnit
{
    long long u, f, d, c, e;

    string toString() {
        return "u: " + to_string(u) + " f: " + to_string(f / BILLION) + " d: " + to_string(d / BILLION) + " c: " + to_string(c/BILLION)
        + " e: " + to_string(e / BILLION);
    }
};

struct QueryParams
{
    int N;
    double tau;
    vector<long long> I;
    vector<long long > O;
};

struct Answer{
    int p, t, b;
    string toString() {
        return "p: " + to_string(p) + " t: " + to_string(t) + " b: " + to_string(b);
    }
};

/*
* 链表操作
*/
struct Node {
    int id;
    Node* prev = nullptr;
    Node* next = nullptr;;
};
typedef Node* DoubleLinkList;

// 创建链表
inline DoubleLinkList ListInit()
{
	DoubleLinkList list = (DoubleLinkList)malloc(sizeof(Node));
	list->prev = NULL;
	list->next = NULL;
	list->id = -1;
	return list;
}
// 在n位置插入值
inline  int ListInsert(DoubleLinkList list, int id, int n)// 将node插入到第n位,n从1开始
{
	if(list==nullptr || n < 1) // 判断参数有效性
		return -1;
		
	Node* cur = list;	// cur指向当前结点，初始化指向头结点
	int cur_i=0;				// cur_i表示当前结点的序号,0-头结点
	while(cur && cur_i<(n-1))// 当前结点有效，且不是插入位置的前一个结点，就后移一个
	{
		cur = cur->next;
		cur_i++;
	}
	if(!cur)			// 当前结点无效，说明已经移动到最后
	{
		printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
		return -1;	// 链表没有 n 那么长
	}
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->id = id;
	newNode->prev = cur;
	newNode->next = cur->next;
	if(cur->next)		// 在最后一个结点插入时，cur->next==NULL
		cur->next->prev = newNode;
    else if (cur->next == nullptr || cur->next == list){
        newNode->next = list;
        list->prev = newNode;
    }
	cur->next = newNode;
	
	return 0;
}

// 删除第n个结点，且将删除的值通过data传出
inline int ListDelete(DoubleLinkList list, int *id, int n)
{
	if(list==NULL || id==NULL || n<1)
		return -1;
	Node* cur = list;	// cur指向当前结点，初始化指向头结点
	int cur_i=0;				// cur_i表示当前结点的序号,0-头结点
	while(cur->next && cur_i<(n-1))
	{// 下个结点有效，且当前位置不是删除位置的前一个，就后移一个
		cur = cur->next;
		cur_i++;
	}
	if(!cur->next)		// 下个结点无效，说明已经移动到最后
	{
		printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
		return -1;		// 链表没有 n 那么长
	}
	Node *deleteNode = cur->next;
	deleteNode->prev->next = deleteNode->next;
	deleteNode->next->prev = deleteNode->prev;
	free(deleteNode);
	return 0;
}











/*
* l : laywer
* h : dimension of hidden
* F : parameter number
*/
int l, h;
long long F;
double alpha, beta_own, gamma_own;
int n, m;

int totalQuery;
vector<QueryParams> queries;
vector<ComputeUnit> units;
vector<Answer> answer;
vector<PII> answerQ;

/*
* 计算分数
*/
vector<vector<int> > g, w;
vector<vector<int> > A;
vector<int> inst;
vector<int> p, t, b;
int P;

/*
* 保存最好解
*/
vector<Answer> bestAnswer;
vector<PII> bestAnswerQ;
long long bestScore;

/*
* *******************解法一（只考虑total_pre_fill)*********************
*/
void solveAboutPreFill();
/*
* *******************解法二（只考虑total__decode)*********************
*/
void solveAboutDecodeFill();
/*
*******************解法三（只考虑total)*********************
*/
void solveAboutTotal();
/*
*******************解法四（综合考虑所有情况)*********************
*/
void solveAboutAll();
/*******************启发式搜索算法*/
void hillClimb();
/*
* 直接给出划分
*/
void getptb(int op, int &p, int &t, int &b) {
    int batch[] = {1, 1, 1, 1, 1};
    int com_batch = 1;
    switch(op) {
        case 131000: p = 8, t = 1, b = com_batch; break;
        case 140000: p = 8, t = 1, b = com_batch; break;
        case 156500: p = 8, t = 1, b = com_batch; break;
        case 333500: p = 8, t = 1, b = com_batch; break;
        case 1000000: p = 8, t = 1, b = com_batch; break;
        default:
        p = 1, t = 8, b = 1;
    }
    
}


long long getMaxIo();
bool check(long long d, int t, int b);
int H(int a, int b);
double min_io();
double min_i();
double min_o();
double min_nj();
double max_ui();
double max_fi();
double L_opt();
double L_opt_prefill();
double L_opt_decode();
double V_accu(int s, int j);
double V_last_batch(int s, int j);
double L_comm(int s, int j);
double L_decode_mem_comm(int s, int j);
double L_decode_comp(int s, int j);
double L_prefill(int s, int j);
double L(int s, int j);
double L_decode(int s);
double L_prefill(int s);
double L(int s);
double L_total();
double L_total_prefill();
double L_total_decode();
double L_first_token();
double L_incremental_token();
uint32_t get_score();
uint32_t get_score(double l_total_prefill, double l_total_decode, double l_total);
/*
记录L_total_prefill, L_total_decode, L_total
*/
double l_total_prefill, l_total_decode, l_total;
void read_res();
void initSolve();
void printRes();
void printMidScore();
void initRandomSolve(int seed);
void totalInpAndInMachine();
void setPtb();
int main(int argc, char* argv[])
{
    #ifdef LOCAL
    if (fopen("./check_score/input.txt", "r") != nullptr) {

        freopen(("./check_score/input.txt"), "r", stdin);
        freopen(("./check_score/output.txt"), "w", stdout);

        
    } else {
        cerr << "open file uncorrectly" << endl;
    }
    #endif

	std::ios::sync_with_stdio(false); cin.tie(0);cout.tie(0);
	cin >> l >> h >> F;

	cin >> alpha >> beta_own >> gamma_own;

	cin >> n >> m;

    units = vector<ComputeUnit>(n);
    int a, b, c, d, e;
	for (int i = 0; i < n; i ++)
	{

		cin >> units[i].u >> units[i].f >> units[i].d >> units[i].c >> units[i].e;

		units[i].f *= BILLION;
		units[i].d *= BILLION;
		units[i].c *= BILLION;
		units[i].e *= BILLION;
	}
    queries = vector<QueryParams>(m);

    for (int j = 0; j < m; j++)
    {
        // scanf("%d%lf", &queries[j].N, &queries[j].tau);

        cin >> queries[j].N >> queries[j].tau;
        totalQuery += queries[j].N;

        queries[j].I.resize(queries[j].N);
        queries[j].O.resize(queries[j].N);

        int in;
        for (int r = 0; r < queries[j].N; r ++)
        {   
            // scanf("%d", &in);
            // queries[j].I[r] = in;
            cin >> queries[j].I[r];
        }
        
        for (int r = 0; r < queries[j].N; r ++)
        {
            // scanf("%d", &in);
            // queries[j].O[r] = in;
            cin >> queries[j].O[r];
        }
    }
    // 初始解
    // initSolve();
    // setPtb();
    // solveAboutPreFill();

    setPtb();


    // solveAboutDecodeFill();
    // read_res();
    // uint32_t decodeScore = get_score();
    // #ifdef SCORE
    // cerr << "decodeScore: " << decodeScore << endl;
    // #endif SCORE
    // bestScore = decodeScore;

    if (alpha > 0.3) {
        solveAboutTotal();
        read_res();
        uint32_t totalScore = get_score(l_total_prefill, l_total_decode, l_total);
        #ifdef SCORE
        cerr << "totalScore: " << totalScore << endl;
        #endif SCORE


        if (totalScore > bestScore) {
            bestAnswer = answer;
            bestAnswerQ = answerQ;
            bestScore = totalScore;
        }
    }


    // solveAboutAll();
    // read_res();
    // uint32_t allScore = get_score();
    // #ifdef SCORE
    // cerr << "allScore: " << allScore << endl;
    // #endif SCORE


    // if (allScore > bestScore) {
    //     bestAnswer = answer;
    //     bestAnswerQ = answerQ;
    //     bestScore = allScore;
    // }
     // 爬山法优化


    solveAboutPreFill();
    read_res();
    // cerr << runtime() << endl;
    uint32_t fillScore = get_score(l_total_prefill, l_total_decode, l_total);
    // cerr << runtime() << endl;
    #ifdef SCORE
    cerr << "fillScore: " << fillScore << endl;
    #endif SCORE
    if (fillScore > bestScore) {
        bestAnswer = answer;
        bestAnswerQ = answerQ;
        bestScore = fillScore;
    }

    answer = move(bestAnswer);
    answerQ = move(bestAnswerQ);
    // cerr << runtime() << endl;
    #ifdef SCORE
    read_res();
    uint32_t terminalScore = get_score();
    cerr << "terminalScore: " << terminalScore << endl;
    totalInpAndInMachine();
    printMidScore();
    #endif SCORE

    printRes();

    // cerr << runtime() << endl;




    // 输出最总解
    // printRes();
    // read_res();
    // cerr << "initScore:" << initScore <<  "bestScore : " << get_score() << endl;
    return 0;
}


/*
* check the memory of range
*/
inline bool check(long long d, int t, int b) {
    long long left = d * t;
    long long model = 2ll * F;
    long long vcache = 4ll * b * l * h * getMaxIo();
    long long right = model + vcache;

    // #ifdef LOCAL
    // cerr << d << " " << t << endl;
    // cerr << left << " " << right << endl;
    // #endif
    return left >= right;
}
inline long long getMaxIo()
{
    long long res = 0;
    for (int j = 0; j < m; j ++ ) {
        for (int r = 0; r < queries[j].N; r ++ )
        {
            res = max(res, queries[j].I[r] + queries[j].O[r]);
        }        
    }
    return res;
}

/*
* calculate score
*/
inline int H(int a, int b)
{
    return a == b;
}

inline double min_io()
{
    double res = 1e47;
    for (int j = 0; j < m; j++)
    {
        for (int r = 0; r < queries[j].N; r++)
        {
            res = min<double>(res, queries[j].I[r] + queries[j].O[r]);
        }
    }

    return res;
}

inline double min_i()
{
    double res = 1e47;
    for (int j = 0; j < m; j++)
    {
        for (int r = 0; r < queries[j].N; r++)
        {
            res = min<double>(res, queries[j].I[r]);
        }
    }

    return res;
}

inline double min_o()
{
    double res = 1e47;
    for (int j = 0; j < m; j++)
    {
        for (int r = 0; r < queries[j].N; r++)
        {
            res = min<double>(res, queries[j].O[r]);
        }
    }

    return res;
}

inline double min_nj()
{
    double res = 1e47;
    for (int j = 0; j < m; j++)
    {
        res = min<double>(res, queries[j].N);
    }
    return res;
}

inline double max_ui()
{
    double res = 0;
    for (int i = 0; i < n; i++)
    {
        res = max<double>(res, units[i].u);
    }
    return res;
}

inline double max_fi()
{
    double res = 0;
    for (int i = 0; i < n; i++)
    {
        res = max<double>(res, units[i].f);
    }
    return res;
}

inline double L_opt()
{
    double res = 2 * F / max_ui() * m * min_nj() * min_io() / (max_ui() * max_fi());
    return res;
}

inline double L_opt_prefill()
{
    double res = 2 * F / max_ui() * m * min_nj() * min_i() / (max_ui() * max_fi());
    return res;
}

inline double L_opt_decode()
{
    double res = 2 * F / max_ui() * m * min_nj() * min_o() / (max_ui() * max_fi());
    return res;
}

inline double V_accu(int s, int j)
{
    if (A[j][s] <= 1) return 0;

    double res = 0;

    for (int x = 0; x < A[j][s] - 1; x++)
    {
        double max_val = 0;
        for (int r = 0; r < queries[j].N; r++)
        {
            double o_jr = queries[j].O[r];
            double i_jr = queries[j].I[r];
            double cur = o_jr * (i_jr + 0.5 * (o_jr - 1)) * H(w[j][r], x) * H(g[j][r], s);
            max_val = max(max_val, cur);
        }
        res += max_val;
    }
    res *= b[inst[s]];
    return res;
}

inline double V_last_batch(int s, int j)
{
    double res = 0;
    for (int r = 0; r < queries[j].N; r++)
    {
        res += H(w[j][r], A[j][s] - 1) * H(g[j][r], s);
    }
    
    double max_val = 0;
    for (int r = 0; r < queries[j].N; r++)
    {
        double o_jr = queries[j].O[r];
        double i_jr = queries[j].I[r];
        double cur = o_jr * (i_jr + 0.5 * (o_jr - 1)) * H(w[j][r], A[j][s] - 1) * H(g[j][r], s);
        max_val = max(max_val, cur);
    }
    res *= max_val;
    return res;
}

inline double L_comm(int s, int j)
{
    double res =  8 * l * h * (V_accu(s, j) + V_last_batch(s, j));
    res /= units[inst[s]].e;

    res *= (t[inst[s]] - 1) / (double)t[inst[s]];

    return res;
}

inline double L_decode_mem_comm(int s, int j)
{
    double res = 2 * F + 8 * l * h * (V_accu(s, j) + V_last_batch(s, j));

    res /= t[inst[s]] * units[inst[s]].c;
    return res;
}

inline double L_decode_comp(int s, int j)
{
    double res = 2 * F / (double)t[inst[s]];
    double sum = 0;
    for (int r = 0; r < queries[j].N; r++)
    {
        sum += queries[j].O[r] * H(g[j][r], s);
    }
    res *= sum;
    res /= units[inst[s]].f;
    return res;
}

inline double L_prefill(int s, int j)
{
    double res = 2 * F / (double)t[inst[s]];
    double sum = 0;
    for (int r = 0; r < queries[j].N; r++)
    {
        sum += queries[j].I[r] * H(g[j][r], s);
    }
    res *= sum;
    res /= units[inst[s]].f;
    return res;
}

inline double L(int s, int j)
{
    double ls = L_prefill(s, j) + L_decode_comp(s, j) + L_decode_mem_comm(s, j) + L_comm(s, j);

    return max(ls, queries[j].tau);
}

inline double L_decode(int s)
{
    double res = 0;
    for (int j = 0; j < m; j++)
    {
        res += L_decode_comp(s, j) + L_decode_mem_comm(s, j);
    }
    return res;
}

inline double L_prefill(int s)
{
    double res = 0;
    for (int j = 0; j < m; j++)
    {
        res += L_prefill(s, j);
    }
    return res;
}

inline double L(int s)
{
    double res = 0;
    for (int j = 0; j < m; j++)
    {
        res += L(s, j);
    }
    return res;
}

inline double L_total()
{
    double res = 0;
    for (int s = 0; s < P; s++)
    {
        res = max(res, L(s));
    }
    return res;
}

inline double L_total_prefill()
{
    double res = 0;
    for (int s = 0; s < P; s++)
    {
        res = max(res, L_prefill(s));
    }
    return res;
}

inline double L_total_decode()
{
    double res = 0;
    for (int s = 0; s < P; s++)
    {
        res = max(res, L_decode(s));
    }
    return res;
}

inline double L_first_token()
{
    double res = L_total_prefill() * P;
    double sum = 0;
    for (int j = 0; j < m; j++)
    {
        sum += queries[j].N;
    }

    res /= sum;
    return res;
}

inline double L_incremental_token()
{
    double res = L_total_prefill() * P;
    double sum = 0;
    for (int j = 0; j < m; j++)
    {
        for (int r = 0 ; r < queries[j].N; r++)
        {
            sum += queries[j].O[r];
        }
    }

    res /= sum;
    return res;
}

inline uint32_t get_score()
{
    uint32_t a = L_opt() * 1e7 / L_total();
    uint32_t b = L_opt_prefill() * 1e7 / L_total_prefill();
    uint32_t c = L_opt_decode() * 1e7/ L_total_decode();

    uint32_t score = a * alpha + b * beta_own + c * gamma_own;

    double l_first_token_threshold = 1;
    double pen_first = min(l_first_token_threshold / L_first_token(), 1.0);

    double l_incremental_token_threshold = 0.05;
    double pen_incremental = min(l_incremental_token_threshold / L_incremental_token(), 1.0);

    score *= pen_first * pen_incremental;
    // #ifdef SCORE
    // printMidScore();
    // #endif
    return score;
}
inline uint32_t get_score(double l_total_prefill, double l_total_decode, double l_total) {
    uint32_t a = L_opt() * 1e7 / l_total;
    uint32_t b = L_opt_prefill() * 1e7 / l_total_prefill;
    uint32_t c = L_opt_decode() * 1e7/ l_total_decode;

    uint32_t score = a * alpha + b * beta_own + c * gamma_own;

    double l_first_token_threshold = 1;
    double pen_first = min(l_first_token_threshold / L_first_token(), 1.0);

    double l_incremental_token_threshold = 0.05;
    double pen_incremental = min(l_incremental_token_threshold / L_incremental_token(), 1.0);

    score *= pen_first * pen_incremental;
    // #ifdef SCORE
    // printMidScore();
    // #endif
    return score;
}
inline void printMidScore() {
    long long a = L_opt() * 1e7 / L_total();
    long long b = L_opt_prefill() * 1e7 / L_total_prefill();
    long long c = L_opt_decode() * 1e7 / L_total_decode();
        cerr << "------------------------------------------------------------" << endl;
        cerr << "L_opt : " << L_opt() * 1e7 << " L_total: " << L_total() << endl;
        cerr << "L_opt_prefill: " << L_opt_prefill() * 1e7<< " L_total_prefill: " << L_total_prefill() << endl;
        cerr << "L_opt_decode: " << L_opt_decode() * 1e7<< " L_total_decode: " << L_total_decode() << endl;
        cerr << "Lopt/Ltotal: " <<  a << " Loptprefill/ltotalprefill: " << b << " loptDcode/LtotalDecode: " << c << endl;
        cerr << "score: " << a * alpha + b * beta_own + c * gamma_own << endl;
        double l_first_token_threshold = 1;
        double pen_first = min(l_first_token_threshold / L_first_token(), 1.0);

        double l_incremental_token_threshold = 0.05;
        double pen_incremental = min(l_incremental_token_threshold / L_incremental_token(), 1.0);
        cerr << "pen_first: " << pen_first << " " << "pen_incremental: " << pen_incremental << endl;
        // 计算解码第二部分的值
        for (int i = 0; i < P; ++ i) {
            double second1 = 0;
            double second2 = 0;
            double second3 = 0;
            double second4 = 0;
            for (int j = 0; j < m; ++ j) {
                second1 += L_prefill(i, j);
                second3 += L_decode_comp(i, j);
                second2 += L_decode_mem_comm(i, j);
                second4 += L_comm(i, j);
            }
            cerr << units[inst[i]].toString() << endl;
            cerr << answer[inst[i]].toString() << endl;
            cerr << "L_prefill: " <<  " " <<  second1 <<  endl;
            cerr << "L_decode_comp: " <<  " " <<  second2 <<  endl;
            cerr << "L_decode_mem_comm: "   << " " <<  second3 <<  endl;
            cerr << "L_decode: "   << " " <<  second3 + second2<<  endl;
            cerr << "L_comm: "   << " " <<  second4 <<  endl;
            cerr << "L_s: " << max(second1 + second2 + second3 + second4, 0.0) << endl;
            cerr << "real l_decode: " << L(i) << endl;
            cerr << endl;
        }
}

inline void read_res()
{
    p.resize(n);
    t.resize(n);
    b.resize(n);
    P = 0;

    for (int i = 0; i < n; i++)
    {
        p[i] = answer[i].p;
        t[i] = answer[i].t;
        b[i] = answer[i].b;

        P += p[i];
    }
    inst.clear();
    for (int i = 0; i < n; i++)
    {
        for (int x = 0; x < p[i]; x++)
        {
            inst.push_back(i);
        }
    }

    g.resize(m);
    w.resize(m);
    A.resize(m);
    
    int index = 0;
    for (int j = 0; j < m; j++)
    {
        g[j].resize(queries[j].N);
        w[j].resize(queries[j].N);
        A[j].resize(P);
        for (int r = 0; r < queries[j].N; r++)
        {
            g[j][r] = answerQ[index].first;
            w[j][r] = answerQ[index].second;
            index ++;
            g[j][r] --;
            w[j][r] --;

            A[j][g[j][r]]++;
        }

        for (int s = 0; s < P; s++)
        {
            A[j][s] = (A[j][s] + b[inst[s]] - 1) / b[inst[s]];
        }
    }
}
inline void initSolve() {

    answer.resize(n);

    // the select of p 
    vector<int> pSelect{1, 2, 4, 8};


    for (int i = 0; i < n; ++ i) {
        int p = 1, t = units[i].u / p, b = 1;

        getptb(units[i].f / BILLION, p, t, b);
        while (!check(units[i].d, t, b)) {
            t = t * 2, p = p / 2, b = 1;
            if (t > 8) {
                p = 1, t = 8, b = 1;
                break;
            }
        }
        answer[i] = {p, t, b};

    }
    
 
    int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;


    answerQ.resize(totalQuery);
    int index = 0;
    for (int j = 0; j < m; j++)
    {
        vector<int> A(P), cnt(P, 0);
        int pipeline_index = 0;
        for (int r = 0; r < queries[j].N; r++)
        {
            answerQ[index ++] = make_pair(pipeline_index + 1, A[pipeline_index] + 1);
            // cout << pipeline_index + 1 << ' ' << A[pipeline_index] + 1 << endl;
            // contor the batch
            cnt[pipeline_index] ++;
            if ((cnt[pipeline_index] ^ p_batch[pipeline_index]) == 0) cnt[pipeline_index] = 0, A[pipeline_index]++;

            // A[pipeline_index]++;
            pipeline_index = (pipeline_index + 1) % P;
        }
    }
    int preIndex = 0, pIndex = answerQ[0].first;
    int nt = 0;
    for (int i = 1; i < index; i ++ ) {
        if (answerQ[i].first == pIndex) {
            nt = i;
            break;
        }
    }
}

inline void initRandomSolve(int seed) {

    answer.resize(n);

    // the select of p 
    vector<int> pSelect{1, 2, 4, 8};


    for (int i = 0; i < n; ++ i) {
        int p = 1, t = units[i].u / p, b = 1;
        getptb(units[i].f / BILLION, p, t, b);
        while (!check(units[i].d, t, b)) {
            t = t * 2, p = p / 2, b = 1;
            if (t > 8) {
                p = 1, t = 8, b = 1;
                break;
            }
        }
        answer[i] = {p, t, b};
    }
    
 
    int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;


    answerQ.resize(totalQuery);
    int index = 0;

    // for (int j = 0; j < m; j++)
    // {
    //     vector<int> A(P), cnt(P, 0);
    //     int pipeline_index = 0;
    //     for (int r = 0; r < queries[j].N; r++)
    //     {
    //         answerQ[index ++] = make_pair(pipeline_index + 1, A[pipeline_index] + 1);
    //         // cout << pipeline_index + 1 << ' ' << A[pipeline_index] + 1 << endl;
    //         // contor the batch
    //         cnt[pipeline_index] ++;
    //         if ((cnt[pipeline_index] ^ p_batch[pipeline_index]) == 0) cnt[pipeline_index] = 0, A[pipeline_index]++;

    //         // A[pipeline_index]++;
    //         pipeline_index = (pipeline_index + 1) % P;
    //     }
    // }
    // 构造随机集找到尽可能的所有情况
    std::mt19937 gen(seed); 
    std::uniform_int_distribution<> dis(0,  P - 1);  
    for (int j = 0; j < m; j++)
    {
        vector<int> A(P), cnt(P, 0);
        int pipeline_index = 0;
        for (int r = 0; r < queries[j].N; r++)
        {
            pipeline_index = dis(gen);

            // while (pipeline_index == 2 && A[pipeline_index] * p_batch[pipeline_index] >= 20) {
            //     pipeline_index = dis(gen);
            // }

            answerQ[index ++] = make_pair(pipeline_index + 1, A[pipeline_index] + 1);
            
            // contor the batch
            cnt[pipeline_index] ++;
            if ((cnt[pipeline_index] ^ p_batch[pipeline_index]) == 0) cnt[pipeline_index] = 0, A[pipeline_index]++;

            // A[pipeline_index]++;
            // pipeline_index = (pipeline_index + 1) % P;
        }
    }   
}



inline void printRes() {
    // 输出p,t,b
    for (auto &ans : answer) {
        printf("%d %d %d\n", ans.p, ans.t, ans.b);
        // cout << ans.p << " " << ans.t << " " << ans.b << endl;
    }
    // 输出g,w
    for (auto &gw : answerQ) {
        printf("%d %d\n", gw.first, gw.second);
        // cout << gw.first << " " << gw.second << endl;
    }
}
inline void totalInpAndInMachine() {
    vector<int> p_machine;
    int Pt = 0;
    for (int i = 0; i < n; i++) {   
        // cout << answer[i].p << " " << answer[i].t << " " << answer[i].b << endl;
        Pt += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(Pt, 0);
    for (int i = 0; i < Pt; ++ i) p_batch[i] = answer[p_machine[i]].b;    

    // 输出每个管道对应的上查询的数量
    vector<int> cnt(Pt);
    for (int i = 0; i < answerQ.size(); ++ i) {
        cnt[answerQ[i].first - 1] ++;
    }
    cerr << "--------------------------------------------------" << endl;
    for (int i = 0; i < n; i ++ ) {
        cerr << answer[i].toString() << endl;
    }
    vector<int>cntInMachine(answer.size());
    for (int i = 0; i < Pt; ++ i) {
        cerr << "管道：" << i << " 查询的数量: " << cnt[i] << endl;
        cntInMachine[p_machine[i]] += cnt[i];
    }
    for (int i = 0; i < n; ++ i) {
        cerr << "机器：" << i << " 指标: " << units[i].toString() << endl;
        cerr << "机器：" << i << " 查询数量：" << cntInMachine[i] << endl;
    }

}


/***************************************解法一(只考虑total_pre_fill)******************************************************/

inline void setPtb() {
    answer.resize(n);

    // the select of p 
    vector<int> pSelect{1, 2, 4, 8};


    for (int i = 0; i < n; ++ i) {
        int p = 1, t = units[i].u / p, b = 1;

        getptb(units[i].f / BILLION, p, t, b);
        while (!check(units[i].d, t, b)) {
            t = t * 2, p = p / 2, b = 1;
            if (t > 8) {
                p = 1, t = 8, b = 1;
                break;
            }
        }
        answer[i] = {p, t, b};

    }
}


inline void solveAboutPreFill() {
    int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;

    // 预处理每个decode_comp和decide_mem_comm的固定系数
    vector<double> coef_comp(P, 0);
    vector<double> coef_mem_comn(P, 0);
    vector<double> coef_mem_comm(P, 0);
    // 保存comp和mem_comn的两部分结果
    vector<double> partIncomp(P, 0), partInMen(P, 0), partInFill(P, 0), partInComm(P, 0);
    vector<double> coef_b(P, 0);
    // 预处理每个pre_fill的固定系数
    // 利用优先队列进行选择
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap;
    for (int i = 0; i < P; ++ i) {
        coef_comp[i] = 2.0 * F / (answer[p_machine[i]].t * units[p_machine[i]].f);
        coef_mem_comn[i] = 8.0 * l * h / (answer[p_machine[i]].t * units[p_machine[i]].c);
        coef_b[i] = 2 * m * F  / (answer[p_machine[i]].t * units[p_machine[i]].c);
        coef_mem_comm[i] = 8.0 * l * h * (answer[p_machine[i]].t - 1) / (answer[p_machine[i]].t * units[p_machine[i]].e);
        heap.push({1.0 * coef_comp[i], i});
    }
    // 保存每个管道下面的查询
    vector<vector<int>> queryInPipine(P);
    vector<int> q_burst(totalQuery);
    vector<int> q_input(totalQuery);
    vector<int> q_output(totalQuery);
    // 遍历查询

    vector<PII> sortQuery(totalQuery);
    for (int j = 0, qid = 0; j < m; ++ j) {
        for (int r = 0; r < queries[j].N; ++ r, ++ qid) {
            q_burst[qid] = j;
            sortQuery[qid] = make_pair(queries[j].I[r], qid);
            q_output[qid] = queries[j].O[r];
        }
    }




    for (int j = 0, qid = 0; j < m; ++ j ) {
        for (int r = 0; r < queries[j].N; ++ r, ++ qid) {
            
            long long i = queries[j].I[r], o = queries[j].O[r];
            // 得到目前值最小的
            auto [lPre, pid] = heap.top();
            heap.pop();
            // cerr << lPre << " " << pid << endl;
            lPre = lPre + 1.0 * coef_comp[pid] * queries[j].I[r];

            heap.push({lPre, pid});
            // 同时保存分配结果(将对应查询的编号保存到通道中)
            queryInPipine[pid].emplace_back(qid);
            // 将查询编号与bursts对应
            q_burst[qid] = j;
            q_input[qid] = i;


            partInFill[pid] += coef_comp[pid] * i;
            partIncomp[pid] += coef_comp[pid] * o;
            partInMen[pid] += coef_mem_comn[pid] * (o * (i + 0.5 * (o - 1)));
            partInComm[pid] += coef_mem_comm[pid] * (o * (i + 0.5 * (o - 1)));
        }
    }




    answerQ.resize(totalQuery);
    // 重新调整answerQ
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);
        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }


    
    // 迭代最大值
    l_total_prefill = l_total_decode = l_total = 0.0;
    for (int i = 0; i < P; i ++ ) {
        l_total_prefill = max(l_total_prefill, partInFill[i]);
        l_total_decode = max(l_total_decode, partIncomp[i] + partInMen[i] + coef_b[i]);
        l_total = max(l_total, partInFill[i] + partIncomp[i] + partInMen[i] + coef_b[i] + partInComm[i]);
    }
    read_res();

    double max_total_prefill = l_total_prefill;

    // 保证pre_fill满足波动浮动不大的条件下，让decode_fill更加均匀分配
    vector<double> l_decode(P, 0.0);
    vector<double> l_pre_fill(P, 0.0);
    vector<double> l(P, 0.0);
    for (int j = 0; j < P; ++ j) {
        // l_decode[j] = L_decode(j);
        // l_pre_fill[j] = L_prefill(j);
        l_decode[j] =  partIncomp[j] + partInMen[j] + coef_b[j];
        l_pre_fill[j] = partInFill[j];
        l[j] = partInFill[j] + partIncomp[j] + partInMen[j] + coef_b[j] + partInComm[j];

    }
    // 根据l_s对管道编号进行排序
    vector<int> sortByP(P);
    iota(sortByP.begin(), sortByP.end(), 0);
    sort(sortByP.begin(), sortByP.end(), [&](const int& id1, const int& id2){
        return l_decode[id1] < l_decode[id2];
    });











    //寻找可以交换条件的解
    // 调试输出之前的结果
    int cnt = 0;
    while (true) {
        if (runtime() > MAX_RUNNING_TIME) break;
        cnt ++;
        // front:最小;back最大
        int front = sortByP[0], back = sortByP[P - 1]; 




 


        // 寻找管道内的任务
        if (queryInPipine[front].size() == 0 || queryInPipine[back].size() == 0) break;


        int index1 = rng.next(0, queryInPipine[front].size());
        int index2 = rng.next(0, queryInPipine[back].size());
        int q1 = queryInPipine[front][index1];
        int q2 = queryInPipine[back][index2];

        const int &input_1 = q_input[q1], &input_2 = q_input[q2];
        const int &output_1 = q_output[q1], &output_2 = q_output[q2];
        // 计算增加的指标
        double add_pre_fill[2] =  {coef_comp[front] * input_2, coef_comp[back] * input_1};

        double add_decode[2] =  {coef_comp[front] *output_2, coef_comp[back] * output_1};
        double add_decode_mem[2] = {coef_mem_comn[front] * (output_2 * (input_2 + 1.0 / 2 *  (output_2 - 1))), 
        coef_mem_comn[back] * (output_1 * ( input_1 + 1.0 / 2 *  (output_1 - 1)))};
        double add_comm[2] =  {coef_mem_comm[front] * (output_2 * (input_2 + 1.0 / 2 *  (output_2 - 1))), coef_mem_comn[back] * (output_1 * ( input_1 + 1.0 / 2 *  (output_1 - 1)))};

        double sub_pre_fill[2] =  {coef_comp[front] * input_1, coef_comp[back] * input_2};

        double sub_decode[2] =  {coef_comp[front] * output_1, coef_comp[back] * output_2};
        double sub_decode_mem[2] = {coef_mem_comn[front] * (output_1 * (input_1 + 1.0 / 2 *  (output_1 - 1))), 
        coef_mem_comn[back] * (output_2 * ( input_2 + 1.0 / 2 *  (output_2 - 1)))};
        double sub_comm[2] =  {coef_mem_comm[front] * (output_1 * (input_1 + 1.0 / 2 *  (output_1 - 1))), coef_mem_comn[back] * (output_2 * ( input_2 + 1.0 / 2 *  (output_2 - 1)))};


        cnt ++;
        // 条件不满足
        if (add_decode[1] + add_decode_mem[1] > sub_decode[1] + sub_decode_mem[1]) continue;

        // cerr << "back: " << back << " " << "add_decode[1] + add_decode_mem[1]: " << add_decode[1] + add_decode_mem[1] << " sub_decode[1] + sub_decode_mem[1]: " << sub_decode[1] + sub_decode_mem[1] << endl; 
                        // 交换解集
        
        double front_prefill = l_pre_fill[front];
        double back_prefill = l_pre_fill[back];


        double front_prefill_change = l_pre_fill[front] + add_pre_fill[0] - sub_pre_fill[0];
        double back_prefill_change = l_pre_fill[back] + add_pre_fill[1] - sub_pre_fill[1];
        


        if (back_prefill_change > max_total_prefill || front_prefill_change > max_total_prefill) {
            continue;
        }
        // 修改l_s
        l_pre_fill[front] = front_prefill_change;
        l_pre_fill[back] = back_prefill_change;
        // 修改对应的l_decode值
        l_decode[back] += (add_decode[1] + add_decode_mem[1] - sub_decode[1] - sub_decode_mem[1]);
        l_decode[front] += (add_decode[0] + add_decode_mem[0] - sub_decode[0] - sub_decode_mem[0]);
        // 修改l
        l[front] = l_pre_fill[front] + l_decode[front] + (add_comm[0] - sub_comm[0]);
        l[back] = l_pre_fill[back] + l_decode[back] + (add_comm[1] - sub_comm[1]);

        if (l_decode[front] > l_decode[back]) break;

        swap(queryInPipine[front][index1], queryInPipine[back][index2]);

        // 调整管道编号的顺序
        // 先将min往后移动
        int frontId = sortByP[0];
        bool flag = false;
        for (int j = 1; j < P - 1; j ++ ) {
            if (l_decode[sortByP[j]] < l_decode[frontId]) {
                sortByP[j - 1] = sortByP[j];
            } else {
                sortByP[j - 1] = frontId;
                flag = true;
                break;
            }
        }
        if (!flag) sortByP[P - 2] = frontId;

        // 将max往前移动
        int backId = sortByP[P - 1];
        flag = false;
        for (int j = P - 2; j >= 0; j -- ) {
            if (l_decode[sortByP[j]] > l_decode[backId]) {
                sortByP[j + 1] = sortByP[j];
            } else {
                sortByP[j + 1] = backId;
                flag = true;
                break;
            }
        }
        if (!flag) sortByP[0] = backId;





    }
    // 修改三个指标
    l_total_decode = l_decode[sortByP[P - 1]];
    l_total_prefill = 0, l_total = 0;
    for (int i = 0; i < P; ++ i) {
        l_total_prefill = max(l_pre_fill[i], l_total_prefill);
        l_total = max(l_total, l[i]);
    }
    
    // cerr << "ssssssssssssssssssssssssss: cnt :" << cnt << endl;

    // 重新调整answerQ
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);
        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }


}

inline void solveAboutDecodeFill() {
    int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;

    // 预处理每个decode_comp和decide_mem_comm的固定系数
    vector<double> coef_comp(P, 0);
    vector<double> coef_mem_comn(P, 0);
    // 保存comp和mem_comn的两部分结果
    vector<double> partIncomp(P, 0), partInMen(P, 0);
    vector<double> coef_b(P, 0);
    // 利用优先队列进行选择
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap;
    for (int i = 0; i < P; ++ i) {
        coef_comp[i] = 2.0 * F / (answer[p_machine[i]].t * units[p_machine[i]].f);
        coef_mem_comn[i] = 8.0 * l * h / (answer[p_machine[i]].t * units[p_machine[i]].c);
        coef_b[i] = 2 * m * F  / (answer[p_machine[i]].t * units[p_machine[i]].c);
        heap.push({1.0 * m * 2 * F  / (answer[p_machine[i]].t * units[p_machine[i]].f), i});
    }
    // 保存每个管道下面的查询
    vector<vector<int>> queryInPipine(P);
    vector<int> q_burst;
    // 遍历查询
    for (int j = 0, qid = 0; j < m; ++ j ) {
        for (int r = 0; r < queries[j].N; ++ r, ++ qid) {

            // 得到目前值最小的
            auto [lPre, pid] = heap.top();
            heap.pop();
            // cerr << lPre << " " << pid << endl;
            long long i = queries[j].I[r], o = queries[j].O[r];
            partIncomp[pid] += coef_comp[pid] * o;
            partInMen[pid] += coef_mem_comn[pid] * (o * (i + 1.0 / 2 * (o - 1)));
            lPre = partIncomp[pid] + partInMen[pid] + coef_b[pid];

            heap.push({lPre, pid});
            // 同时保存分配结果(将对应查询的编号保存到通道中)
            queryInPipine[pid].emplace_back(qid);
            // 将查询编号与bursts对应
            q_burst.emplace_back(j);
        }
    }
    answerQ.resize(totalQuery);
    // 已经分配好管道
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);

        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }
    // for (int i = 0; i < totalQuery; ++ i ) {
    //     cerr << answerQ[i].first << " " << answerQ[i].second << endl;
    // }

    // 放置完毕    
}
inline void solveAboutTotal() {
   int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;

    // 预处理每个decode_comp和decide_mem_comm的固定系数
    vector<double> coef_comp(P, 0);
    vector<double> coef_mem_comn(P, 0);
    vector<double> coef_mem_comm(P, 0);
    // 保存comp和mem_comn的两部分结果
    vector<double> partIncomp(P, 0), partInMen(P, 0), partInFill(P, 0), partInComm(P, 0);
    vector<double> coef_b(P, 0);
    // 利用优先队列进行选择
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap;
    for (int i = 0; i < P; ++ i) {
        coef_comp[i] = 2.0 * F / (answer[p_machine[i]].t * units[p_machine[i]].f);
        coef_mem_comn[i] = 8.0 * l * h / (answer[p_machine[i]].t * units[p_machine[i]].c);
        coef_b[i] = 2.0  * F  / (answer[p_machine[i]].t * units[p_machine[i]].c);
        heap.push({coef_b[i], i});
        coef_mem_comm[i] = 8.0 * l * h * (answer[p_machine[i]].t - 1) / (answer[p_machine[i]].t * units[p_machine[i]].e);
    }

    // 保存每个管道下面的查询
    vector<vector<int>> queryInPipine(P);
    vector<int> q_burst;
    vector<int> q_input(totalQuery);
    vector<int> q_output(totalQuery);
    // 遍历查询
    for (int j = 0, qid = 0; j < m; ++ j ) {
        for (int r = 0; r < queries[j].N; ++ r, ++ qid) {

            // 得到目前值最小的
            auto [lPre, pid] = heap.top();
            heap.pop();
            // cerr << lPre << " " << pid << endl;
            long long i = queries[j].I[r], o = queries[j].O[r];
            partInFill[pid] += coef_comp[pid] * i;
            partIncomp[pid] += coef_comp[pid] * o;
            partInMen[pid] += coef_mem_comn[pid] * (o * (i + 1.0 / 2 * (o - 1)));
            partInComm[pid] += coef_mem_comm[pid] * (o * (i + 1.0 / 2 * (o - 1)));
            lPre = partInFill[pid] + partIncomp[pid] + partInMen[pid] + coef_b[pid] + partInComm[pid];
            heap.push({lPre, pid});
            // 同时保存分配结果(将对应查询的编号保存到通道中)
            queryInPipine[pid].emplace_back(qid);
            // 将查询编号与bursts对应
            q_burst.emplace_back(j);
            q_input[qid] = i;
            q_output[qid] = o;
        }
    }
    // 迭代最大值
    l_total_prefill = l_total_decode = l_total = 0.0;
    for (int i = 0; i < P; i ++ ) {
        l_total_prefill = max(l_total_prefill, partInFill[i]);
        l_total_decode = max(l_total_decode, partIncomp[i] + partInMen[i] + coef_b[i]);
        l_total = max(l_total, partInFill[i] + partIncomp[i] + partInMen[i] + coef_b[i] + partInComm[i]);
    }

    answerQ.resize(totalQuery);
    // 已经分配好管道
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);

        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }
 

    // 迭代最大值
    l_total_prefill = l_total_decode = l_total = 0.0;
    for (int i = 0; i < P; i ++ ) {
        l_total_prefill = max(l_total_prefill, partInFill[i]);
        l_total_decode = max(l_total_decode, partIncomp[i] + partInMen[i] + coef_b[i]);
        l_total = max(l_total, partInFill[i] + partIncomp[i] + partInMen[i] + coef_b[i] + partInComm[i]);
    }
    read_res();

    double max_l_total =  l_total;
    // 保证max_l_total满足波动浮动不大的条件下，让pre_fill更加均匀分配
    vector<double> l_decode(P, 0.0);
    vector<double> l_pre_fill(P, 0.0);
    vector<double> l(P, 0.0);
    for (int j = 0; j < P; ++ j) {
        // l_decode[j] = L_decode(j);
        // l_pre_fill[j] = L_prefill(j);
        l_decode[j] =  partIncomp[j] + partInMen[j] + coef_b[j];
        l_pre_fill[j] = partInFill[j];
        l[j] = partInFill[j] + partIncomp[j] + partInMen[j] + coef_b[j] + partInComm[j];

    }
    // 根据l_s对管道编号进行排序
    vector<int> sortByP(P);
    iota(sortByP.begin(), sortByP.end(), 0);
    sort(sortByP.begin(), sortByP.end(), [&](const int& id1, const int& id2){
        return l_pre_fill[id1] < l_pre_fill[id2];
    });




    //寻找可以交换条件的解
    // 调试输出之前的结果
    int cnt = 0;
    while (true) {
        if (runtime() > MAX_RUNNING_TIME) break;
        cnt ++;
        // front:最小;back最大
        int front = sortByP[0], back = sortByP[P - 1]; 


 


        // 寻找管道内的任务
        if (queryInPipine[front].size() == 0 || queryInPipine[back].size() == 0) break;


        int index1 = rng.next(0, queryInPipine[front].size());
        int index2 = rng.next(0, queryInPipine[back].size());
        int q1 = queryInPipine[front][index1];
        int q2 = queryInPipine[back][index2];

        const int &input_1 = q_input[q1], &input_2 = q_input[q2];
        const int &output_1 = q_output[q1], &output_2 = q_output[q2];
        // 计算增加的指标
        double add_pre_fill[2] =  {coef_comp[front] * input_2, coef_comp[back] * input_1};

        double add_decode[2] =  {coef_comp[front] *output_2, coef_comp[back] * output_1};
        double add_decode_mem[2] = {coef_mem_comn[front] * (output_2 * (input_2 + 1.0 / 2 *  (output_2 - 1))), 
        coef_mem_comn[back] * (output_1 * ( input_1 + 1.0 / 2 *  (output_1 - 1)))};
        double add_comm[2] =  {coef_mem_comm[front] * (output_2 * (input_2 + 1.0 / 2 *  (output_2 - 1))), coef_mem_comn[back] * (output_1 * ( input_1 + 1.0 / 2 *  (output_1 - 1)))};

        double sub_pre_fill[2] =  {coef_comp[front] * input_1, coef_comp[back] * input_2};

        double sub_decode[2] =  {coef_comp[front] * output_1, coef_comp[back] * output_2};
        double sub_decode_mem[2] = {coef_mem_comn[front] * (output_1 * (input_1 + 1.0 / 2 *  (output_1 - 1))), 
        coef_mem_comn[back] * (output_2 * ( input_2 + 1.0 / 2 *  (output_2 - 1)))};
        double sub_comm[2] =  {coef_mem_comm[front] * (output_1 * (input_1 + 1.0 / 2 *  (output_1 - 1))), coef_mem_comn[back] * (output_2 * ( input_2 + 1.0 / 2 *  (output_2 - 1)))};


        cnt ++;
        // 条件不满足
        if (add_pre_fill[1] > sub_pre_fill[1]) continue;

        // cerr << "back: " << back << " " << "add_decode[1] + add_decode_mem[1]: " << add_decode[1] + add_decode_mem[1] << " sub_decode[1] + sub_decode_mem[1]: " << sub_decode[1] + sub_decode_mem[1] << endl; 
                        // 交换解集
        
        double front_s_total = l[front];
        double back_s_total = l[back];


        double front_s_total_change = l[front] + add_pre_fill[0] + add_decode[0] + add_comm[0] - sub_pre_fill[0] - sub_decode[0] - sub_comm[0];
        double back_s_total_change = l[back] + add_pre_fill[1] + add_decode[1] + add_comm[1] - sub_pre_fill[1] - sub_decode[1] - sub_comm[1];
        


        if (back_s_total_change > max_l_total || front_s_total_change > max_l_total) {
            continue;
        }
        // 修改l_s
        l_pre_fill[front] += (add_pre_fill[0] - sub_pre_fill[0]);
        l_pre_fill[back] += (add_pre_fill[1] - sub_pre_fill[1]);
        // 修改对应的l_decode值
        l_decode[back] += (add_decode[1] + add_decode_mem[1] - sub_decode[1] - sub_decode_mem[1]);
        l_decode[front] += (add_decode[0] + add_decode_mem[0] - sub_decode[0] - sub_decode_mem[0]);
        // 修改l
        l[front] += (add_pre_fill[0] - sub_pre_fill[0]) + (add_decode[0] + add_decode_mem[0] - sub_decode[0] - sub_decode_mem[0]) + (add_comm[0] - sub_comm[0]);
        l[back] += (add_pre_fill[1] - sub_pre_fill[1]) + (add_decode[1] + add_decode_mem[1] - sub_decode[1] - sub_decode_mem[1]) + (add_comm[1] - sub_comm[1]);

        if (l_pre_fill[front] > l_pre_fill[back]) break;

        swap(queryInPipine[front][index1], queryInPipine[back][index2]);

        // 调整管道编号的顺序
        // 先将min往后移动
        int frontId = sortByP[0];
        bool flag = false;
        for (int j = 1; j < P - 1; j ++ ) {
            if (l_decode[sortByP[j]] < l_decode[frontId]) {
                sortByP[j - 1] = sortByP[j];
            } else {
                sortByP[j - 1] = frontId;
                flag = true;
                break;
            }
        }
        if (!flag) sortByP[P - 2] = frontId;

        // 将max往前移动
        int backId = sortByP[P - 1];
        flag = false;
        for (int j = P - 2; j >= 0; j -- ) {
            if (l_decode[sortByP[j]] > l_decode[backId]) {
                sortByP[j + 1] = sortByP[j];
            } else {
                sortByP[j + 1] = backId;
                flag = true;
                break;
            }
        }
        if (!flag) sortByP[0] = backId;





    }
    // 修改三个指标
    l_total_decode = l_decode[sortByP[P - 1]];
    l_total_prefill = 0, l_total = 0;
    for (int i = 0; i < P; ++ i) {
        l_total_prefill = max(l_pre_fill[i], l_total_prefill);
        l_total = max(l_total, l[i]);
    }
    
    // cerr << "ssssssssssssssssssssssssss: cnt :" << cnt << endl;

    // 重新调整answerQ
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);
        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }
     
}

inline void solveAboutAll() {
    // alpha, beta_own, gamma_own;
    double change[3] = {L_opt() * alpha, L_opt_prefill() * beta_own, L_opt_decode() * gamma_own};
    int P = 0;
    vector<int> p_machine;
    for (int i = 0; i < n; i++) {   
    
        P += answer[i].p;

        for (int j = 0; j < answer[i].p; ++ j) p_machine.emplace_back(i);
    }
    // remember thd p to batch
    vector<int> p_batch(P, 0);
    for (int i = 0; i < P; ++ i) p_batch[i] = answer[p_machine[i]].b;

    // 预处理每个decode_comp和decide_mem_comm的固定系数
    vector<double> coef_comp(P, 0);
    vector<double> coef_mem_comn(P, 0);
    vector<double> coef_mem_comm(P, 0);
    // 保存comp和mem_comn的两部分结果
    vector<double> partIncomp(P, 0), partInMen(P, 0), partInFill(P, 0), partInComm(P, 0);
    vector<double> coef_b(P, 0);
    // 利用优先队列进行选择
    priority_queue<PDI, vector<PDI>, greater<PDI>> heap;
    for (int i = 0; i < P; ++ i) {
        coef_comp[i] = 2.0 * F / (answer[p_machine[i]].t * units[p_machine[i]].f);
        coef_mem_comn[i] = 8.0 * l * h / (answer[p_machine[i]].t * units[p_machine[i]].c);
        coef_b[i] = 2 * m * F  / (answer[p_machine[i]].t * units[p_machine[i]].c);

        // 得到所有指标

        heap.push({1.0 * m * 2 * F  / (answer[p_machine[i]].t * units[p_machine[i]].f) * gamma_own, i});

        coef_mem_comm[i] = 8.0 * l * h * (answer[p_machine[i]].t - 1) / (answer[p_machine[i]].t * units[p_machine[i]].e);
    }

    // 保存每个管道下面的查询
    vector<vector<int>> queryInPipine(P);
    vector<int> q_burst;
    // 遍历查询
    for (int j = 0, qid = 0; j < m; ++ j ) {
        for (int r = 0; r < queries[j].N; ++ r, ++ qid) {

            // 得到目前值最小的
            auto [lPre, pid] = heap.top();
            heap.pop();
            // cerr << lPre << " " << pid << endl;
            long long i = queries[j].I[r], o = queries[j].O[r];

            partInFill[pid] += coef_comp[pid] * i;
            partIncomp[pid] += coef_comp[pid] * o;
            partInMen[pid] += coef_mem_comn[pid] * (o * (i + 1.0 / 2 * (o - 1)));
            partInComm[pid] += coef_mem_comm[pid] * (o * (i + 1.0 / 2 * (o - 1)));

            lPre = partInFill[pid] * change[1] + (partInComm[pid] + partInMen[pid]) * change[2]
             + (partInFill[pid] +  partIncomp[pid]+ partInMen[pid] + partInComm[pid]) * change[0];
    
            heap.push({lPre, pid});
            // 同时保存分配结果(将对应查询的编号保存到通道中)
            queryInPipine[pid].emplace_back(qid);
            // 将查询编号与bursts对应
            q_burst.emplace_back(j);
        }
    }

    answerQ.resize(totalQuery);
    // 已经分配好管道
    for (int i = 0; i < P; ++ i) {
        int batch = p_batch[i];
        vector<int> bursts(m), cnt(m, 0);

        for (int qid : queryInPipine[i]) {
            int burstId = q_burst[qid];
            
            answerQ[qid] = {i + 1, bursts[burstId] + 1};
            cnt[burstId] ++;
            // 当一个bacth放满后
            if ((cnt[burstId] ^ batch) == 0) {
                ++ bursts[burstId];
                cnt[burstId] = 0;
            }
        }
    }
    // for (int i = 0; i < totalQuery; ++ i ) {
    //     cerr << answerQ[i].first << " " << answerQ[i].second << endl;
    // }

    // 放置完毕       
}

inline void hillClimb() {

    for (int i = 0; i < P; ++ i) {
        double second1 = 0;
        double second2 = 0;
        double second3 = 0;
        double second4 = 0;
        for (int j = 0; j < m; ++ j) {
            second1 += L_prefill(i, j);
            second3 += L_decode_comp(i, j);
            second2 += L_decode_mem_comm(i, j);
            second4 += L_comm(i, j);
        }
        cerr << units[inst[i]].toString() << endl;
        cerr << answer[inst[i]].toString() << endl;
        cerr << "L_prefill: " <<  " " <<  second1 <<  endl;
        cerr << "L_decode_comp: " <<  " " <<  second2 <<  endl;
        cerr << "L_decode_mem_comm: "   << " " <<  second3 <<  endl;
        cerr << "L_comm: "   << " " <<  second4 <<  endl;
        cerr << "L_s: " << max(second1 + second2 + second3 + second4, 0.0) << endl;
        cerr << "real l_decode: " << L(i) << endl;
        cerr << endl;
    }
}






