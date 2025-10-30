#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <limits>
#include "immintrin.h"

using namespace std;

// 计算两个向量之间的欧氏距离
float euclidean_distance(const float* v1, const float* v2, int dim) {
    __m128 sum = _mm_setzero_ps(); // 初始化累加和为0

    for (int i = 0; i < dim; i += 4) {
        __m128 vec1 = _mm_loadu_ps(v1 + i); // 加载v1中的4个元素
        __m128 vec2 = _mm_loadu_ps(v2 + i); // 加载v2中的4个元素

        __m128 diff = _mm_sub_ps(vec1, vec2); // 计算差值
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff)); // 累加平方差值
    }

    float result[4];
    _mm_store_ps(result, sum); // 将结果存储到result中

    float final_result = 0.0f;
    for (int i = 0; i < 4; ++i) {
        final_result += result[i];
    }

    return std::sqrt(final_result);
}


std::vector<int> find_topk_nearest_neighbors(const std::vector<std::vector<float>>& data, const std::vector<float>& query, int k) {
    std::priority_queue<std::pair<float, int>> min_heap;

    for (int i = 0; i < data.size(); ++i) {
        float distance = euclidean_distance(data[i].data(), query.data(), query.size());

        if (min_heap.size() < k) {
            min_heap.push({ distance, i });
        }
        else {
            if (distance < min_heap.top().first) {
                min_heap.pop();
                min_heap.push({ distance, i });
            }
        }
    }

    std::vector<int> topk_indices;
    while (!min_heap.empty()) {
        topk_indices.push_back(min_heap.top().second);
        min_heap.pop();
    }

    return topk_indices;
}

int main() {
    int num_vectors, dim, k;
    std::ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    cin >> num_vectors >> dim;
    std::vector<std::vector<float>> data(num_vectors, std::vector<float>(dim));
    std::vector<float> query(dim);

    // 初始化数据和查询向量
    for (int i = 0; i < num_vectors; ++i) {
        for (int j = 0; j < dim; ++j) {
            cin >> data[i][j];
        }
    }

    cin >> k;
    cout << "ok" << endl;
    fflush(stdout);
    string op;
    while (cin >> op, op != "end") {
        query[0] = (stof(op));
        for (int i = 1; i < dim; i++) {
            float x;
            cin >> x;
            query[i] = x;
        }
        std::vector<int> topk_indices = find_topk_nearest_neighbors(data, query, k);
        for (int i : topk_indices) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        fflush(stdout);
    }


    return 0;
}
