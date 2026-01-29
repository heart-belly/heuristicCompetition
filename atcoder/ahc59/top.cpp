#include <bits/stdc++.h>
using namespace std;
#define PII pair<int,int>

constexpr int LIMIT = 1900;

struct Pair {
    int id, r1, c1, r2, c2;
    Pair() : id(-1), r1(-1), c1(-1), r2(-1), c2(-1) {}
    PII first(int flag) const { return flag == 0 ? make_pair(r1, c1) : make_pair(r2, c2); }
    PII second(int flag) const { return flag == 0 ? make_pair(r2, c2) : make_pair(r1, c1); }
};
Pair Pairs[200];

struct Node {
    Node* parent;
    vector<Node*> childs;
    int id, flag;

    Node(Node* p, int i, int f) : parent(p), id(i), flag(f) {}

    Node* insert(int newId, int newFlag, int start, int end) {
        Node *newNode = new Node(this, newId, newFlag);
        for (int i = start; i < end; ++ i) {
            childs[i]->parent = newNode;
            newNode->childs.push_back(childs[i]);
        }
        childs.erase(childs.begin() + start, childs.begin() + end);
        childs.insert(childs.begin() + start, newNode);
        return newNode;
    }
};


inline int dist(int r1, int c1, int r2, int c2) {
    return abs(r1 - r2) + abs(c1 - c2);
}

inline int firstEp(Node* node) { return node->flag; }

inline int secondEp(Node* node) { return 1 - node->flag; }

PII getFirst(Node* node) {
    if (node->id == -1) return {0, 0};
    return Pairs[node->id].first(node->flag);
}

PII getSecond(Node* node) {
    if (node->id == -1) return {0, 0};
    return Pairs[node->id].second(node->flag);
}

struct Strategy {
    int N, M; 
    int A[20][20];
    Node* Top;
    Node* Nodes[200];  
    string BestOutput;    
    int BestMoves;
    int PairDist[200][2][200][2]; 
    int TopDist[200][2]; 

    void input() {
        cin >> N;
        M = N * N / 2;
        for (int i = 0; i < M; ++ i) Pairs[i].id = i;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cin >> A[i][j];
                int id = A[i][j];
                if (Pairs[id].r1 == -1) { Pairs[id].r1 = i; Pairs[id].c1 = j; }
                else { Pairs[id].r2 = i; Pairs[id].c2 = j; }
            }
        }
    }
    void initPairDist() {
        for (int i = 0; i < M; i++) {
            int r1i = Pairs[i].r1, c1i = Pairs[i].c1;
            int r2i = Pairs[i].r2, c2i = Pairs[i].c2;
            TopDist[i][0] = abs(r1i) + abs(c1i);
            TopDist[i][1] = abs(r2i) + abs(c2i);
            for (int j = 0; j < M; j++) {
                int r1j = Pairs[j].r1, c1j = Pairs[j].c1;
                int r2j = Pairs[j].r2, c2j = Pairs[j].c2;
                PairDist[i][0][j][0] = abs(r1i - r1j) + abs(c1i - c1j);
                PairDist[i][0][j][1] = abs(r1i - r2j) + abs(c1i - c2j);
                PairDist[i][1][j][0] = abs(r2i - r1j) + abs(c2i - c1j);
                PairDist[i][1][j][1] = abs(r2i - r2j) + abs(c2i - c2j);
            }
        }
    }
    void init() {
        initPairDist();
        Top = new Node(nullptr, -1, 0);
    }

    tuple<int, int, int> findBestInsertForNode(Node* parent, int pairId, int flag) {
        int fEp = flag, sEp = 1 - flag;
        int pId = parent->id, pFEp = firstEp(parent), pSEp = secondEp(parent);
        int childCount = parent->childs.size();
        int bestCost = INT_MAX, bestStart = 0, bestEnd = 0;
        int distFirstSecond = distPair(pairId, fEp, pairId, sEp);
        // 找位置插入
        for (int pos = 0; pos <= childCount; ++ pos) {
            int prevId, prevEp, nextId, nextEp;
            if (pos == 0) {prevId = pId, prevEp = pFEp;}
            else {
                auto *c = parent->childs[pos - 1]; prevId = c->id, prevEp = secondEp(c);
            }
            if (pos == childCount) {nextId = pId, nextEp = pSEp;}
            else { auto* c = parent->childs[pos]; nextId = c->id; nextEp = firstEp(c); }
            int cost = distPair(prevId, prevEp, pairId, fEp) + distFirstSecond
                    + distPair(pairId, sEp, nextId, nextEp) - distPair(prevId, prevEp, nextId, nextEp);
            if (cost < bestCost) {
                bestCost = cost; bestStart = pos; bestEnd = pos;
            }
        }
        // 替换最优的子树
        if (childCount >= 1) {
            int minStartCost = INT_MAX, minStartIdx = -1;
            for (int e = 1; e <= childCount; ++ e) {
                int s = e - 1;
                int prevId, prevEp;
                if (s == 0) {prevId = pId, prevEp = pFEp; }
                else { auto* c = parent->childs[s-1]; prevId = c->id; prevEp = secondEp(c); }
                auto* childS = parent->childs[s];
                int startCost = distPair(prevId, prevEp, pairId, fEp)
                + distPair(pairId, fEp, childS->id, firstEp(childS))
                - distPair(prevId, prevEp, childS->id, firstEp(childS));
                if (startCost < minStartCost) { minStartCost = startCost; minStartIdx = s; }
                int nextId, nextEp;
                if (e == childCount) { nextId = pId; nextEp = pSEp; }
                else { auto* c = parent->childs[e]; nextId = c->id; nextEp = firstEp(c); }
                auto* childE = parent->childs[e-1];
                int endCost = distPair(childE->id, secondEp(childE), pairId, sEp)
                            + distPair(pairId, sEp, nextId, nextEp)
                            - distPair(childE->id, secondEp(childE), nextId, nextEp);
                
                int totalCost = minStartCost + endCost;
                if (totalCost < bestCost) {
                    bestCost = totalCost; bestStart = minStartIdx; bestEnd = e;
                }
            }
        }
        return {bestCost, bestStart, bestEnd};
    }
    void findBestInsert(Node* node, int pairId, int& bestCost, Node*& bestParent, int& bestFlag, int& bestStart, int& bestEnd) {
        for (int flag = 0; flag <= 1; flag++) {
            auto [cost, start, end] = findBestInsertForNode(node, pairId, flag);
            if (cost < bestCost) {
                bestCost = cost; bestParent = node; bestFlag = flag; bestStart = start; bestEnd = end;
            }
        }
        for (auto* child : node->childs) {
            findBestInsert(child, pairId, bestCost, bestParent, bestFlag, bestStart, bestEnd);
        }
    }
    void greedy(int startR, int startC) {
        for (int t = 0; t < M; ++ t) {
            int bestPairId = -1, bestCost = INT_MAX, bestFlag = 0, bestStart = 0, bestEnd = 0;
            Node *bestParent = nullptr;
            if (t == 0)  {
                bestPairId = A[startR][startC];
                findBestInsert(Top, bestPairId, bestCost, bestParent, bestFlag, bestStart, bestEnd);
            } else {
                for (int pairId = 0; pairId < M; ++ pairId) {
                    if (Nodes[pairId]) continue;
                    int cost = INT_MAX, flag = 0, start = 0, end = 0;
                    Node *parent = nullptr;
                    findBestInsert(Top, pairId, cost, parent, flag, start, end);
                    if (cost < bestCost) {
                        bestCost = cost; bestPairId = pairId; bestParent = parent;
                        bestFlag = flag; bestStart = start; bestEnd = end;
                    }

                }
            }
            if (bestPairId != -1) {
                Nodes[bestPairId] = bestParent->insert(bestPairId, bestFlag, bestStart, bestEnd);

            }
        }
    }
    void mainLoop() {
        init();
        int startR = 0, startC = 0;
        greedy(startR, startC);
        BestMoves = calcTotalMoves();
        BestOutput = makeOutput();
        output(BestOutput);
        cerr << "Greedy(" << startR << "," << startC << "): " << BestMoves << " moves, " << endl;
    }
    int calcTotalMoves() {
        int moves = 0, curR = 0, curC = 0;
        function<void(Node*)> dfs = [&](Node *node) {
            if (node->id == -1) {
                for (auto *child : node->childs) dfs(child);
            } else {
                auto [fr, fc] = getFirst(node);
                auto [sr, sc] = getSecond(node);
                moves += dist(curR, curC, fr, fc);
                curR = fr; curC = fc;
                for (auto *child : node->childs) dfs(child);
                moves += dist(curR, curC, sr, sc);
                curR = sr; curC = sc;



            }
        };
        dfs(Top);
        return moves;
    }
    string makeOutput() {
        string result;
        int curR = 0, curC = 0;
        auto moveTo = [&](int destR, int destC) {
            while (curR < destR) { result += "D\n"; curR++; }
            while (curR > destR) { result += "U\n"; curR--; }
            while (curC < destC) { result += "R\n"; curC++; }
            while (curC > destC) { result += "L\n"; curC--; }
        };
        function<void(Node*)> visit = [&](Node* node) {
            if (node->id == -1) {
                for (auto* child : node->childs) visit(child);
            } else {
                auto [fr, fc] = getFirst(node);
                auto [sr, sc] = getSecond(node);
                moveTo(fr, fc); result += "Z\n";
                for (auto* child : node->childs) visit(child);
                moveTo(sr, sc); result += "Z\n";
            }
        };
        visit(Top);
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }
    void output(string res) {
        cout << res;
    }
    int distPair(int idA, int epA, int idB, int epB) {
        if (idA == -1 && idB == -1) return 0;
        if (idA == -1) return TopDist[idB][epB];
        if (idB == -1) return 0;
        return PairDist[idA][epA][idB][epB];
    }
};
int main() {
    if (fopen("../data/0000.txt", "r") != nullptr) {
        freopen("../data/0001.txt", "r", stdin);
        freopen("../output.txt", "w", stdout);
    } else {
        cerr << "not found" << endl;
    }
    
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    static Strategy strategy;
    strategy.input();
    strategy.mainLoop();

}
