#pragma GCC optimize "Ofast,omit-frame-pointer,inline,unroll-all-loops"
#include <bits/stdc++.h>
using namespace std;

#define mem(a,b) memset(a,b,sizeof a)
#define PII pair<int,int>
#define LL long long
#define ULL unsigned long long
#define IOS ios::sync_with_stdio(0),cin.tie(0),cout.tie(0)
#define FOR(i,a,b) for (int i = (a); i < (b); ++ i)
#define REP(i,a) FOR(i,0,a)
#define ALL(a) (a).begin(),(a).end()
#define VC vector
#define VB          VC<byte>
#define VVB         VC<VB>
#define VI          VC<int>
#define VVI         VC<VI>
#define VVVI        VC<VVI>
#define VPII        VC<PII>
#define VVPII       VC<VPII>
#define VD          VC<double>
#define VVD         VC<VD>
#define VVVD        VC<VVD>
#define VVVVD       VC<VVVD>
#define VF          VC<float>
#define VVF         VC<VF>
#define VVVF        VC<VVF>
#define VS          VC<string>
#define VVS         VC<VS>
#define endl '\n'
#define DATA(x) {cerr << "[DATA] " << #x << " = " << (x) << endl;}

// 高斯分布
constexpr int GaussianSize = 1000000;
VD _Gaussian(GaussianSize);
constexpr int INF = 0x3f3f3f3f;
constexpr LL INFL = 4000000000000000000LL;
// 预处理倒数和开平方
VD Dinv, Sqrt;
const auto startTime = std::chrono::steady_clock::now();
inline int runtime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return int(duration.count());
}

template<class T>
inline bool chmin(T &a, T b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}
template<class T>
inline bool chmax(T &a, T b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

// 模拟高斯分布
int xor128() {
	static int x = 123456789, y = 362436069, z = 521288629, w = 88675123;
	int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}
// 生成0~1之间的随机数
double getRandom() {
    double _rand = (double)(xor128()) / (double)INT32_MAX;
    return _rand;
} 
// 生成高斯分布
double genGaussian() {
    double X =getRandom(), Y = getRandom(), Z = sqrt(-2.0 * log(X)) * cos(2.0 * acos(-1.0) * Y);
    return Z; 
}
// 计算两点之间的距离（平方距离）
int distXY(int xFrom, int yFrom, int xTo, int yTo) {
    int xdiff = min(10000 - abs(xFrom - xTo), abs(xFrom - xTo));
    int ydiff = min(10000 - abs(yFrom - yTo), abs(yFrom - yTo));
    return xdiff * xdiff + ydiff * ydiff;
}
// 轮转算法
struct WeightedRandomIndex {
    VD prefix;
    double total = 0.0;
    // 重置权值
    void reset(VD &weights) {
        int n = weights.size();
        prefix.resize(n + 1);
        prefix[0] = 0.0;
        REP(i, n) {
            double  w = weights[i];
            prefix[i + 1] = prefix[i] + w; 
        }
        total = prefix.back();
    }
    // 按照权值随机抽样
    int sample() {
        double r = getRandom() * total;
        auto it = upper_bound(prefix.begin() + 1, prefix.end(), r);
        int idx = (it - prefix.begin() - 1);
        return idx;
    }

};

WeightedRandomIndex weightedRandomIndex;

// 输入部分
struct Input {
    VVD X, Y;
    VD Vx, Vy;
    void readProblem(int &N, int &T, int &M, int &K, int &L) {
		cin >> N >> T >> M >> K >> L;
        X.resize(N, VD(T + 10));
        Y.resize(N, VD(T + 10));
        Vx.resize(N);
        Vy.resize(N);
        // 预处理所有点在未合并时在所有时刻的坐标情况
        REP(i, N) {
            cin >> X[i][0] >> Y[i][0] >> Vx[i] >> Vy[i];
            REP(t, T + 9) {
                X[i][t + 1] = X[i][t] + Vx[i];
                if (X[i][t + 1] < 0.0) X[i][t + 1] += L;
                if (X[i][t + 1] > L) X[i][t + 1] -= L;
                Y[i][t + 1] = Y[i][t] + Vy[i];
                if (Y[i][t + 1] < 0.0) Y[i][t + 1] += L;
                if (Y[i][t + 1] > L) Y[i][t + 1] -= L;
            }
        }
        return ;

    }
};
// 输出部分
struct Output {
    Input input;
    int N, T, M, K, L;
    VI NtoM; // 点 -> 簇编号
    VI NtoK; // 点 -> 簇内编号
    VI NtoNfrom; // 点 -> 来自哪个点
    VI NtoNfromNew;
    VD NtoScore; // 点-> 分数
    VD NtoScoreNew;
    VVPII MKtoTN; // (簇编号, 簇内编号) -> （合并时间, 点）
    VVPII MKtoTNold;
    VD scOld; // 簇分数
    VD scNew;
    VI xdiff, ydiff; // 距离簇内中心点的距离差
    void resizeALL(int _N, int _T, int _M, int _K, int _L, Input &_input) {
        N = _N, T = _T, M = _M, K = _K, L = _L;
        input = _input;
        NtoM.resize(N);
        NtoK.resize(N);
        NtoScore.resize(N);
        NtoScoreNew.resize(N);
        NtoNfrom.resize(N, -1);
        NtoNfromNew.resize(N, -1); 
        MKtoTN.resize(M, VPII(K));
        MKtoTNold.resize(M, VPII(K));
        xdiff.resize(K);
        ydiff.resize(K);
        scOld.resize(M);
        scNew.resize(M);
        REP(m, M) {
            REP(k, K) {
                int id = m * K + k;
                NtoM[id] = m;
                NtoK[id] = k;
                if (k > 0) NtoNfrom[id] = MKtoTN[m][k - 1].second; // 记录该点的前一个点
                MKtoTN[m][k] = {k * 30, id};
            }
        }

    }
    void writeSolution() {
        REP(id, N) {
            if (NtoNfrom[id] == -1) continue;
            int m = NtoM[id];
            int k = NtoK[id];
            int t = MKtoTN[m][k].first;
            int nfrom = NtoNfrom[id];
            cout << t << " " << id << " " << nfrom << endl;
        }
        return;
    }
    void backup(int m) {
        REP(k, K) {
            MKtoTNold[m][k] = MKtoTN[m][k];
        }
    } 
    void rollback(int m) {
        REP(k, K) {
            MKtoTN[m][k] = MKtoTNold[m][k];
        }
    }
    void sortM(int m) {
        sort(ALL(MKtoTN[m]));
    }
    void accept(int m) {
        REP(k, K) {
            int id = MKtoTN[m][k].second;
            NtoNfrom[id] = NtoNfromNew[id];
            NtoScore[id] = NtoScoreNew[id];
            NtoM[id] = m;
            NtoK[id] = k;
        }
        scOld[m] = scNew[m];
    }

    // 评估某个状态下用最优方式连接簇的成本
    double evalTmp(int m, int kd, double scth = 1e20) {
        double sc = 0.0;
        double Xc = 0.0, Yc = 0.0;
        double Vx = 0.0, Vy = 0.0;
        REP(k, K) {
            int t = MKtoTN[m][k].first;
            int id = MKtoTN[m][k].second;
            int xNext = input.X[id][t];
            int yNext = input.Y[id][t];
            int vxNext = input.Vx[id];
            int vyNext = input.Vy[id];
            if (k == 0) {
                Xc = xNext;
                Yc = yNext;
                Vx = vxNext;
                Vy = vyNext;
                xdiff[0] = 0;
                ydiff[0] = 0;
                NtoNfromNew[id] = -1;
                NtoScoreNew[id] = 0;
            } else {
                int tDiff = t - MKtoTN[m][k - 1].first;
                Xc += Vx * tDiff;
                Yc += Vy * tDiff;
                int distMin = INF;
                double kMin = -1;
                int xNextDiff = xNext - (int)(Xc + 0.5);
                int yNextDiff = yNext - (int)(Yc + 0.5);
                xNextDiff /= 10;
                yNextDiff /= 10;
                REP(kk, k) {
                    int distTmp = distXY(xdiff[kk], ydiff[kk], xNextDiff, yNextDiff);
                    if (chmin(distMin, distTmp)) {
                        kMin = kk;
                    }
                }
                double scTmp = Sqrt[distMin / 100] * 100.0;
                sc += scTmp;
                NtoScoreNew[id] = (double) distMin * Sqrt[k];
                int krem = max(kd + K - 1 - k - 10, 0);
                if (sc + 1000.0 * krem > scth) return INFL;
                xdiff[k] = xNextDiff;
                ydiff[k] = yNextDiff;
                Vx = ((double)k * Vx + vxNext) * Dinv[k + 1];
                Vy = ((double)k * Vy + vyNext) * Dinv[k + 1];

                NtoNfromNew[id] = MKtoTN[m][kMin].second;
            }
        }
        scNew[m] = sc;
        return sc;
    }

    double evalAll() {
        double sc = 0.0;
        REP(m, M) {
            sc += evalTmp(m, 0);
            accept(m);
        }
        return sc;
    }
};

// 核心策略部分
struct Strategy {
    int N, T, M, K, L;
    Input input;
    Output output;
    void init() {
        input.readProblem(N, T, M, K, L);
        output.resizeALL(N, T, M, K, L, input);
        // 生成高斯分布
        REP(i, GaussianSize) {
            _Gaussian[i] = genGaussian();
        }
        // 处理倒数
        Dinv.resize(100);
        REP(i, 99) {
            Dinv[i + 1] = 1.0 / (double)(i + 1);
        }
        Sqrt.resize(GaussianSize);
        REP(i, GaussianSize) {
            Sqrt[i] = sqrt(i);
        }

        return;

    }

    void mainLoop() {
        double sc = output.evalAll();
        double initCost = sc;
        DATA(initCost);
        weightedRandomIndex.reset(output.NtoScore);
        
        auto SA = [&]() -> void {
            double startTemp = 6000.0;
            double endTemp = 1.0;
            double TIME_INIT = runtime();
            double TIME_LIMIT = 1985.0 - TIME_INIT;
            LL loopCnt = 0;
            LL improveCnt = 0;
            double timeNow = 0.0;
            double temp = 0.0;

            int mfrom = -1, mto = -1;
            int kfrom = -1, kto = -1;
            int nfrom = -1, nto = -1;
            int tfrom = -1, tto = -1;
            double progress = 0.0;
            double gw = 0.0; // 扰动权值
            while (1) {
                //break;
                if (loopCnt % 1000 == 0) {
                    timeNow = runtime() - TIME_INIT;
                    if (timeNow> TIME_LIMIT)
                        break;
                    progress = pow(timeNow / TIME_LIMIT, 0.7); // 采用了非线性收敛（0.7这里就可以得到早期收敛慢，后期收敛快）
                    temp = startTemp + (endTemp - startTemp) * progress;
                    gw = 220.0 - progress * 100.0; // 早期扰动大，探索阶段;后期扰动小，收敛阶段
                }

                loopCnt ++;
                double scdiff = 0;

                double scth = -temp * log(getRandom()); // 退火能够接受增强成本的阈值
                int seed = xor128() % 2;
                int type = 0;
                if (seed < 1)
                    type = 0;
                else if (seed < INF)
                    type = 1;
                // 微调某个点在其所属分子内的连接时间
                if (type == 0) {
                    nfrom = weightedRandomIndex.sample();
                    mfrom = output.NtoM[nfrom]; // 簇编号
                    kfrom = output.NtoK[nfrom]; // 簇内编号
                    // 重新随机找某个簇的中心点（第一个点）
                    if (xor128() % K == 0) {
                        mfrom = xor128() % M;
                        kfrom = 0;
                        nfrom = output.MKtoTN[mfrom][kfrom].second;
                    }

                    tfrom = output.MKtoTN[mfrom][kfrom].first; // 合并到大簇中的时间
                    tfrom += (int)(_Gaussian[xor128() % GaussianSize] * gw + 0.5); // 对连接时间进行调整
                    if (tfrom < 0) continue;
                    if (tfrom >= T) continue;
                    scdiff -= output.scOld[mfrom]; // 直接减去mfrom大簇的成本
                    output.backup(mfrom);
                    output.MKtoTN[mfrom][kfrom].first = tfrom;
                    output.sortM(mfrom); // 由于修改了时间，那么根据（时间+节点）重新排序
                    scdiff += output.evalTmp(mfrom, 0, scth - scdiff); // 再加上重新调整时间后的这个大簇的成本
                }
                //将属于不同分子的两个点进行交换，改变分子的组成
                else if (type == 1) {
                    nfrom = weightedRandomIndex.sample();
                    mfrom = output.NtoM[nfrom];
                    kfrom = output.NtoK[nfrom];
                    if (xor128() % K == 0) {
                        mfrom = xor128() % M;
                        kfrom = 0;
                        nfrom = output.MKtoTN[mfrom][kfrom].second;
                    }
                    tfrom = output.MKtoTN[mfrom][kfrom].first;

                    nto = weightedRandomIndex.sample();
                    mto = output.NtoM[nto];
                    kto = output.NtoK[nto];
                    if (xor128() % K == 0) {
                        mto = xor128() % M;
                        kto = 0;
                        nto = output.MKtoTN[mto][kto].second;
                    }
                    tto = output.MKtoTN[mto][kto].first;

                    if (mfrom == mto) continue;
                    // 进行交换
                    scdiff -= output.scOld[mfrom];
                    scdiff -= output.scOld[mto];
                    output.backup(mfrom);
                    output.backup(mto);
                    output.MKtoTN[mfrom][kfrom].second = nto;
                    output.MKtoTN[mto][kto].second = nfrom;
                    output.sortM(mfrom);
                    output.sortM(mto);
                    scdiff += output.evalTmp(mfrom, K, scth - scdiff);
                    scdiff += output.evalTmp(mto, 0, scth - scdiff);
                }
                // 接受该解
                if (scdiff < scth) { 
                    improveCnt ++;
                    sc += scdiff;
                    weightedRandomIndex.reset(output.NtoScore);
                    if (type == 0) {
                        output.accept(mfrom);
                    }
                    else if (type == 1) {
                        output.accept(mfrom);
                        output.accept(mto);
                    }
                }
                else {
                    if (type == 0) {
                        output.rollback(mfrom);

                    }
                    else if (type == 1) {
                        output.rollback(mfrom);
                        output.rollback(mto);
                    }
                }
            }
            DATA(improveCnt);
            return; 
        };
        SA();
        double improveCost = sc;
        DATA(improveCost);
        output.writeSolution();

    }

};
int main() {
    if (fopen("../data/0000.txt", "r") != nullptr) {
        if (!freopen("../data/0000.txt", "r", stdin)) {
            cerr << "错误：无法打开输入文件" << endl;
            return 1;
        }
        if (!freopen("./output.txt", "w", stdout)) {
            cerr << "错误：无法打开输出文件" << endl;
            return 1;
        }
        cerr << "成功读取本地文件" << endl;
    } else {
        cerr << "使用标准输入输出" << endl;
    }
    IOS;
    static Strategy strategy;
    strategy.init();
    strategy.mainLoop();
}
