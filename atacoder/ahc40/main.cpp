#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#define LOCAL

using namespace std;
typedef pair<int, int> PII;
#define x first
#define y second
constexpr int N = 110;
constexpr int T = 4 * N;
constexpr int MAX_VARIANCE = 10010;
constexpr int INF = 0x3f3f3f3f;


const auto startTime = std::chrono::steady_clock::now();

 int runtime() {
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
     int next(int x) {return ((long long)rand() * x) >> 32;}
    //  int next(int x) {return rand() % x;}
     int next(int a, int b, bool flag) {
        if (flag) {
          return a + (rand() % (b - a));  
        } else {
            return a + ((long long) rand() * (b - a)) >> 32;
        }
    }
     double next_double() {return (rand() + 0.5) * (1.0 / 4294967296.0);}
}; 

RNG rng;

struct Rectangle {
    int w, h;
    bool isRotate;
    int posX, posY; // 左上角坐标
    int initW, initH;

    Rectangle(int _w, int _h, int _posX, int _posY, bool _flag = false): w(_w), h(_h),  posX(_posX), posY(_posY), isRotate(_flag) {}
    Rectangle():w(0), h(0), isRotate(false), posX(0), posY(0){}

    inline void reset() {
        w = initW, h = initH, isRotate = 0;
    }
};
struct Section {
    int start = 0, end = 0;
    int length = 0;
};
struct Strategy {
    int n, t, variance;
    Rectangle wh[N];
    vector<vector<int>> temp;  
    vector<bool> st;
    int maxWidth, maxHeight;
    int indexMaxH, indexMaxW;
    // Section maxH[1000], maxW[1000], temp[1000];


    inline void input() {
        #ifdef LOCAL

                if(fopen("./file/input.txt", "r") != nullptr) {

                    freopen(("./file/input.txt"), "r", stdin);
                    freopen(("./file/output.txt"), "w", stdout);

                    
                } else {
                    cerr << "open file uncorrectly" << endl;
                }
        #endif
        scanf("%d%d%d", &n, &t, &variance);
        for (int i = 0; i < n; ++ i) {
            scanf("%d%d", &wh[i].w, &wh[i].h);
            wh[i].initW = wh[i].w, wh[i].initH = wh[i].h;
            // 将最大的度量放在第一位
            if (wh[i].w < wh[i].h) {
                swap(wh[i].w, wh[i].h);
                wh[i].isRotate = true;
            }
        }


        initLayoutInfo();

        
    }


    inline void initLayoutInfo() {
        indexMaxH = indexMaxW = 0;
        maxHeight = maxWidth = 0;
        st = vector<bool>(n, 0);
    }
    inline void backupLayoutInfo() {
        for (int i = 0; i < n; ++ i) {
            // 将最大的度量放在第一位
            if (wh[i].initW < wh[i].initH) {
                wh[i].w = wh[i].initH, wh[i].h = wh[i].initW;
                wh[i].isRotate = true;
            } else {
                wh[i].w = wh[i].initW, wh[i].h = wh[i].initH;
                wh[i].isRotate = false;
            }

            // 重置每个矩阵的左上角坐标位置
            wh[i].posX = 0, wh[i].posY = 0;
        }
        st = vector<bool>(n, 0);
        maxWidth = maxHeight = 0;

    }
    inline bool setPointPos(int index, int op, int aligningPos) {

        auto check = [&](auto&& check, const int& l1, const int& r1, const int& l2, const int& r2)-> bool {
            if (r1 <= l2 || l1 >= r2) return false;
            return true;
        };
        // 记录新的区间不同高度的数量
        int width = wh[index].w, height = wh[index].h;
        // 有两种不同的放入方式(up, left)
        if (op == 1) {
            // 被顶起的区间在[aligningPos, aligningPos + width]
            int layHeight = 0;
            for (int i = 0; i < n; ++ i) {
                if (!st[i]) continue;
                if (check(check, wh[i].posX, wh[i].posX + wh[i].w, aligningPos, aligningPos + width)) {
                    // 有交集
                    layHeight = max(layHeight, wh[i].posY + wh[i].h);
                }

            }
            //找到最大的layHeight
            wh[index].posX = aligningPos, wh[index].posY = layHeight;

        } else if (op == 0) {
            // 被顶起的区间在[aligningPos, aligningPos + height]
            int layWidth = 0;
            for (int i = 0; i < n; ++ i) {
                if (!st[i]) continue;
                if (check(check, wh[i].posY, wh[i].posY + wh[i].h, aligningPos, aligningPos + height)) {
                    layWidth = max(layWidth, wh[i].posX + wh[i].w);
                }
            }
            wh[index].posX = layWidth, wh[index].posY = aligningPos;


        }
        st[index] = true;
        maxHeight = max(maxHeight, wh[index].posY +wh[index].h);
        maxWidth = max(maxWidth, wh[index].posX + wh[index].w);

        return true;


    }

    inline bool cancelPointPos(int index) {
        // 该撤销操作一定要保证从放置矩阵序列的最后开始撤销
        st[index] = false;
        wh[index].posX = wh[index].posY = 0;
        maxHeight = maxWidth = 0;
        for (int i = 0; i < n; ++ i) {
            if (st[i]) {
                maxHeight = max(maxHeight, wh[i].posY + wh[i].h);
                maxWidth = max(maxWidth, wh[i].posX + wh[i].w);
            }
        }

        return true;


    }

    inline double getLayoutArea() {
        double area = 0.0;
        for (int i = 0; i < n; ++ i ) {
            if (st[i]) {
                area += 1.0 * wh[i].h * wh[i].w;
            }
        }
        return area;
    }

    inline double getAreaRate() {
        if (maxHeight == 0 || maxWidth == 0) return 1;

        return 1.0 * getLayoutArea() / (1.0 * maxHeight * maxWidth);
    }

    inline double getSum() {
        return maxHeight + maxWidth;
    }

    inline PII getUpDown(int op) {

        vector<int> arraySort(2 * n);
        for (int i = 0, j = 0; i < n; ++ i) arraySort[j ++ ] = wh[i].w, arraySort[j ++ ] = wh[i].h;
        sort(arraySort.begin(), arraySort.end());
        PII res = make_pair(0, 0);
        int sqrtN = sqrt(1.0 * n) + 1;
        for (int i = 0; i < 2 * n && sqrtN > 0; ++ i, -- sqrtN) {
            res.x += arraySort[2 * n - 1 - i];
            res.y += arraySort[i];
        }

        cerr << "sqrtN: " << sqrtN << "maxLength: " << res.x << "minLength:" << res.y << endl;
        // 对最差上节进行简单惩罚
        switch(op) {
            case 0: res.x =  1.0 * (res.x + res.y) / 2; break;
            case 1: res.x =  1.0 *(res.x + res.y) / 1.9; break;
            case 2: res.x =  1.0 *(res.x + res.y) / 1.8; break;
            case 3: res.x =  1.0 *(res.x + res.y) / 1.7; break;
            case 4: res.x =  1.0 *(res.x + res.y) / 1.6; break;
            case 5: res.x =  1.0 *(res.x + res.y) / 1.5; break;
            case 6: res.x =  1.0 *(res.x + res.y) / 1.4; break;
            default:
                break;
        }

        return res;
        

    }

    inline vector<int> selectAlign(int index) {



        double occupancyU = 0x3f3f3f3f;
        int alginIndexU = -1;
        bool modeU = 0;
        //重置当前矩阵的长宽和转置标记
        wh[index].reset();
        // 长宽不交换
        for (int i = 0; i < 2; ++ i) {
            
            double curOccupancy = 0x3f3f3f3f;
            int curAlginIndex = -1;

            // 选择靠初始边放置
            setPointPos(index, 1, 0);
            curOccupancy = min(curOccupancy,  getSum());
            curAlginIndex = -1;
            cancelPointPos(index);


            for (int j = 0; j < n; ++ j) {
                if (!st[j]) continue;
                // 依靠索引为j的放置
                int align =  wh[j].posX + wh[j].w;
                setPointPos(index, 1, align);
                if (getSum() < curOccupancy) {
                    curOccupancy =  getSum();
                    curAlginIndex = j;
                }
                cancelPointPos(index);
            }

            if (curOccupancy < occupancyU) {
                occupancyU = curOccupancy;
                alginIndexU = curAlginIndex;
                modeU = wh[index].isRotate;

            }

            // 交换长宽
            if (i == 0) {
                swap(wh[index].w, wh[index].h);
                wh[index].isRotate = true;
            }
        }



            
        double occupancyL = 0x3f3f3f3f;
        int alginIndexL = -1;
        bool modeL = 0;
        //重置当前矩阵的长宽和转置标记
        wh[index].reset();
        // 长宽不交换
        for (int i = 0; i < 2; ++ i) {
            
            double curOccupancy = 0x3f3f3f3f;
            int curAlginIndex = -1;

            // 选择靠初始边放置
            setPointPos(index, 0, 0);
            curOccupancy = min(curOccupancy,  getSum());
            curAlginIndex = -1;
            cancelPointPos(index);


            for (int j = 0; j < n; ++ j) {
                if (!st[j]) continue;
                // 依靠索引为j的放置
                int align =  wh[j].posY + wh[j].h;
                setPointPos(index, 0, align);
                if ( getSum() < curOccupancy) {
                    curOccupancy =  getSum();
                    curAlginIndex = j;
                }
                cancelPointPos(index);
            }

            if (curOccupancy < occupancyL) {
                occupancyL = curOccupancy;
                alginIndexL = curAlginIndex;
                modeL = wh[index].isRotate;

            }

            // 交换长宽
            if (i == 0) {
                swap(wh[index].w, wh[index].h);
                wh[index].isRotate = true;
            }
        }
        // cerr << "U: " << occupancyU << " L: " << occupancyL << endl;
        // 比较哪种选择最好
        if (occupancyU < occupancyL) {
            return {1, alginIndexU, modeU};
        }
        return {0, alginIndexL, modeL};

    }

    inline vector<int> selectAlignUp(int index) {



        double occupancyU = 0x3f3f3f3f;
        int alginIndexU = -1;
        bool modeU = 0;
        //重置当前矩阵的长宽和转置标记
        wh[index].reset();
        // 长宽不交换
        for (int i = 0; i < 1; ++ i) {
            
            double curOccupancy = 0x3f3f3f3f;
            int curAlginIndex = -1;

            // 选择靠初始边放置
            setPointPos(index, 1, 0);
            curOccupancy = min(curOccupancy,  getSum());
            curAlginIndex = -1;
            cancelPointPos(index);


            for (int j = 0; j < n; ++ j) {
                if (!st[j] || abs(index - j) > 1) continue;
                // 依靠索引为j的放置
                int align =  wh[j].posX + wh[j].w;
                setPointPos(index, 1, align);
                if (getSum() < curOccupancy) {
                    curOccupancy =  getSum();
                    curAlginIndex = j;
                }
                cancelPointPos(index);
            }

            if (curOccupancy < occupancyU) {
                occupancyU = curOccupancy;
                alginIndexU = curAlginIndex;
                modeU = wh[index].isRotate;

            }

            // 交换长宽
            if (i == 0) {
                swap(wh[index].w, wh[index].h);
                wh[index].isRotate = true;
            }
        }

        return {1, alginIndexU, modeU};
    }
    

    inline void normalSolve() {
        temp.resize(n, vector<int>(4));

        // 按照局部最优进行放置
        for (int i = 0; i < n; ++ i) {
            
            vector<int> searchRes = selectAlign(i);
            if (searchRes[0] == 1) {
                // 布局的宽度大于高度，选择向上放置（U）
                int aligningIndex = searchRes[1], mode = searchRes[2];
                wh[i].reset();
                if (mode) swap(wh[i].h, wh[i].w), wh[i].isRotate = true;

                int align = (aligningIndex == -1 ? 0 : wh[aligningIndex].posX + wh[aligningIndex].w);
                setPointPos(i, 1, align);
                temp[i] = {i, wh[i].isRotate, 1, aligningIndex};
                // cerr << "maxHeight: " << maxHeight << "maxWidth: " << maxWidth << endl;
            } else {
                // 布局的高度大于宽度，选择向左放置 (L)
                int aligningIndex = searchRes[1], mode = searchRes[2];
                wh[i].reset();
                if (mode) swap(wh[i].h, wh[i].w), wh[i].isRotate = true;

                int align = (aligningIndex == -1 ? 0 : wh[aligningIndex].posY + wh[aligningIndex].h);
                setPointPos(i, 0, align);
                temp[i] = {i, wh[i].isRotate, 0, aligningIndex};     
                // cerr << "maxHeight: " << maxHeight << "maxWidth: " << maxWidth << endl;      
            }
        }

        print(temp);
        // cerr << "所有矩形的位置:" << endl;
        // for (int i = 0; i < n; ++ i) {
        //     cerr << "第" << i << "个矩形的位置(左上角（x,y): " << wh[i].posX << " " << wh[i].posY
        //     << "当前矩形的长度和宽度: " << wh[i].w << " " << wh[i].h << endl;
        // }
  
        backupLayoutInfo();
        t = t - 1;
    }



    inline void solve() {
        for (int i = -1; i < 7 && t > 0; ++ i, -- t) {
            auto [maxLength, minLength] = getUpDown(i);
            cerr << maxLength << endl;
            // 构造特殊解
            // 先执行up操作
            int rowLength = 0;
            int index = 0;
            temp.resize(n, vector<int>(4));
            // up (封顶)
            int heightMax = 0;
            while (index < n && 1.0 * rowLength + 0.8 * wh[index].w < maxLength) {

                
                rowLength += wh[index].w;
                temp[index] = {index, wh[index].isRotate, 1, index - 1};
                int align = (index > 0 ? wh[index - 1].posX + wh[index - 1].w : 0);
                setPointPos(index, 1, align);
                ++ index;
            }


            while (index < n) {
                rowLength = 0;
                int pos = 0;
                while (index < n && pos < maxLength) {

                    // 两种模式选择
                    wh[index].reset();
                    int align = ( rowLength != 0 ? wh[index - 1].posX + wh[index - 1].w : 0);
                    setPointPos(index, 1, align);
                    double len1 = getSum();
                    cancelPointPos(index);
                    // // 选择靠初始边放置
                    // setPointPos(index, 1, 0);
                    // curOccupancy = min(curOccupancy,  getSum());
                    // curAlginIndex = -1;
                    // cancelPointPos(index);
                    swap(wh[index].w, wh[index].h), wh[index].isRotate = true;
                    setPointPos(index, 1, align);
                    double len2 = getSum();
                    cancelPointPos(index);

                    wh[index].reset();

                    if (len1 < len2) {
                        setPointPos(index, 1, align);
                    } else {
                        swap(wh[index].w, wh[index].h), wh[index].isRotate = true;
                        setPointPos(index, 1, align);
                    }
                    temp[index] = {index, wh[index].isRotate, 1, rowLength == 0 ? -1 : index - 1};
                    rowLength += wh[index].w;
                    pos = wh[index].posX + wh[index].w;
                    ++ index;
                }            
            }


            print(temp);
            // cerr << "所有矩形的位置:" << endl;
            // for (int i = 0; i < n; ++ i) {
            //     cerr << "第" << i << "个矩形的位置(左上角（x,y): " << wh[i].posX << " " << wh[i].posY
            //     << "当前矩形的长度和宽度: " << wh[i].w << " " << wh[i].h << endl;
            // }

            backupLayoutInfo();
        }
        // 随机解
        if (t > 0) {
            temp.resize(n, vector<int>(4));
            for (int i = 0; i < n; ++ i) {
                int isRotate = rng.next(0, 2, true);
                int isU = 1;
                int dui_qi = -1;

                temp[i] = {i, isRotate, isU, dui_qi};
            }

            print(temp);
            t --;

        }

        for (int i = -1; i < 7 && t > 0; ++ i, -- t) {
            auto [maxLength, minLength] = getUpDown(i);
            cerr << maxLength << endl;
            // 构造特殊解
            // 先执行up操作
            int rowLength = 0;
            int index = 0;
            temp.resize(n, vector<int>(4));
            // up (封顶)
            int heightMax = 0;
            vector<int> preSeries, curSeries;
            while (index < n && 1.0 * rowLength + 0.8 * wh[index].w < maxLength) {

                
                rowLength += wh[index].w;
                temp[index] = {index, wh[index].isRotate, 1, index - 1};
                int align = (index > 0 ? wh[index - 1].posX + wh[index - 1].w : 0);
                setPointPos(index, 1, align);
                curSeries.push_back(index);
                ++ index;
            }


            while (index < n) {
                rowLength = 0;
                int pos = 0;
                preSeries = curSeries;
                curSeries.clear();
                while (index < n && pos < maxLength) {

                    // 两种模式选择
                    wh[index].reset();
                    int align = ( rowLength != 0 ? wh[index - 1].posX + wh[index - 1].w : 0);
                    setPointPos(index, 1, align);
                    double len1 = getSum();
                    cancelPointPos(index);
                    // // 选择靠初始边放置
                    // setPointPos(index, 1, 0);
                    // curOccupancy = min(curOccupancy,  getSum());
                    // curAlginIndex = -1;
                    // cancelPointPos(index);
                    swap(wh[index].w, wh[index].h), wh[index].isRotate = true;
                    setPointPos(index, 1, align);
                    double len2 = getSum();
                    cancelPointPos(index);

                    wh[index].reset();

                    if (len1 < len2) {
                        setPointPos(index, 1, align);
                    } else {
                        swap(wh[index].w, wh[index].h), wh[index].isRotate = true;
                        setPointPos(index, 1, align);
                    }
                    temp[index] = {index, wh[index].isRotate, 1, rowLength == 0 ? -1 : index - 1};
                    rowLength += wh[index].w;
                    pos = wh[index].posX + wh[index].w;
                    curSeries.push_back(index);
                    ++ index;
                }            
            }
            // 调整curSeries
            for (int i = 0; i < curSeries.size(); ++ i ) {
                auto id = curSeries[i];
                cancelPointPos(id);

            }

            for (int i = 0; i < curSeries.size(); ++ i) {
                auto id = curSeries[i];
                double lenBest = 0x3f3f3f3f;
                int use = -1;
                for (int j = 0; j < preSeries.size(); ++ j ) {
                    int align = wh[preSeries[j]].posX + wh[preSeries[j]].w;
                    setPointPos(id, 1, align);
                    double len = getSum();
                    cancelPointPos(id);
                    if (len < lenBest) {
                        lenBest = len;
                        use = preSeries[j];
                    }
                }
                int align = wh[use].posX + wh[use].w;
                setPointPos(id, 1, align);
                temp[id] = {id, wh[id].isRotate, 1, use};
                preSeries.push_back(id);
            }


            print(temp);
            // cerr << "所有矩形的位置:" << endl;
            // for (int i = 0; i < n; ++ i) {
            //     cerr << "第" << i << "个矩形的位置(左上角（x,y): " << wh[i].posX << " " << wh[i].posY
            //     << "当前矩形的长度和宽度: " << wh[i].w << " " << wh[i].h << endl;
            // }


            backupLayoutInfo();
        }


        while (t > 0 && t -- ) {
            int number = n;
            temp.resize(number, vector<int>(4));
            for (int i = 0; i < number; ++ i) {
                int isRotate = (wh[i].isRotate && wh[i].w < wh[i].h) || (!wh[i].isRotate && wh[i].h < wh[i].w ) ? 0 : 1;
                int isU = 1;
                int dui_qi = -1;

                temp[i] = {i, isRotate, isU, dui_qi};
            }

            print(temp);
        }
    }

    inline void print(vector<vector<int>> &result) {
        int len = result.size();
        printf("%d\n", len);
        fflush(stdout);
        for (auto& vec : result) {
            char c = (vec[2] == 1 ? 'U' : 'L');
            printf("%d %d %c %d\n", vec[0], vec[1], c, vec[3]);
            fflush(stdout);
        }
        int w, h;
        scanf("%d%d", &w, &h);



    }




};

int main() {
    static Strategy strategy;
    strategy.input();
    strategy.normalSolve();
    strategy.solve();

    return 0;
}
