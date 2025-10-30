
import random
from math import sqrt

import numpy as np

# 读取数据
# File Path
folder_path = './data_zs'
input_file = folder_path + '/input.txt'
output_file_true = folder_path + '/output.txt'
output_file_est = folder_path + '/output_est.txt'
maybe_deviation_path = './data_zs/pianyi.txt'
test_file = "./data_zs/input.txt"

# 光速
c = 0.299792458
# 目标函数（优先目标）
cost_loss = 0.0
best_loss = 1e8
b0 = 41.792
# 结果数组
be_res = []
ue_res = []

# 读取测试数据
def read_test_data(test_file):
    # 打开文件
    with open(test_file, 'r', encoding='utf-8') as file:
        # 逐行读取
        # 读取ue数量
        ue_number = eval(file.readline().strip())  # 使用 strip() 去除可能的前后空格或换行符

        # 读取已知的两个be坐标
        be_list = []
        for _ in range(2):  # 假设只有两个be坐标
            be_list.append(list(map(eval, file.readline().split(','))))

            # 读取ue到各个be的光传播时间
        ts_list = []
        while True:
            line = file.readline().strip()
            if not line:  # 如果读取到空行，说明已经到达文件末尾
                break
            ts_list.append(list(map(eval, line.split())))  # 假设每行都是一系列数字，用空格分隔

    return ue_number, ts_list, be_list

def read_py_txt(data_path):
    # 打开文件并逐行读取内容
    with open(data_path, 'r', encoding='utf-8') as file:
        ret = []
        for line in file:
            # print(line, end='')  # 使用 end='' 防止每行末尾出现额外的换行符
            ret.append(list(map(eval, line.split(' '))))
        return ret


# ue_number, ts_list, be_list = read_test_data(test_file)
# print(ue_number, ts_list, be_list)

def read_txt():
    data_input = np.genfromtxt(input_file, delimiter=',')
    ue_idx_input = np.int32(data_input[:, 0])
    toa_all = data_input[:, 1:]

    return toa_all
    # print(type(ue_idx_input))
    # print(ue_idx_input)
    # print(toa_all)


def read_true_txt():
    data_input = np.genfromtxt(output_file_true, delimiter=',')
    ue_idx_input = np.int32(data_input[:, 0])
    toa_all = data_input[:50, 1:]

    return toa_all


# TDOA
def tdoa(stations, tds, sub):
    # 结果

    global cost_loss
    position = np.array([])
    # 得到基站的坐标（真实）
    x0, y0 = stations[0]
    x1, y1 = stations[1]
    x2, y2 = stations[2]
    x3, y3 = stations[3]

    # 得到某个ue到be的距离差
    r10 = tds[0]
    r20 = tds[1]
    r30 = tds[2]
    r12 = tds[3]
    r13 = tds[4]
    r23 = tds[5]
    # 基站之间的距离
    x10 = x1 - x0
    x20 = x2 - x0
    y10 = y1 - y0
    y20 = y2 - y0

    k0 = x0 ** 2 + y0 ** 2
    k1 = x1 ** 2 + y1 ** 2
    k2 = x2 ** 2 + y2 ** 2

    A = np.array([[x10, y10], [x20, y20]])
    C = np.array([-r10, -r20])
    D = np.array([(k1 - k0 - r10 ** 2) / 2, (k2 - k0 - r20 ** 2) / 2])

    # 求解Ax = r0 * C + D
    # 使用linalg.solve()求解线性方程组
    a = np.linalg.solve(A, C)
    b = np.linalg.solve(A, D)

    A_ = (a[0] ** 2) + (a[1] ** 2) - 1
    B_ = a[0] * (b[0] - x0) + a[1] * (b[1] - y0)
    C_ = (x0 - b[0]) ** 2 + (y0 - b[1]) ** 2
    if (B_ ** 2) - A_ * C_ < 0:
        position = [100, 100]
        cost_loss = cost_loss + 10000
    else:
        r0_1 = -(B_ + sqrt((B_ ** 2) - A_ * C_)) / A_
        r0_2 = -(B_ - sqrt((B_ ** 2) - A_ * C_)) / A_
        X1 = a * r0_1 + b
        X2 = a * r0_2 + b

        loss = 0.0


        if abs(r30 - (distance(X1[0], X1[1], x3, y3) - distance(X1[0], X1[1], x0, y0))) + abs(r23 - (distance(X1[0], X1[1], x2, y2) - distance(X1[0], X1[1], x3, y3))) < 5:
            loss = abs(r30 - (distance(X1[0], X1[1], x3, y3) - distance(X1[0], X1[1], x0, y0))) ** 2 + abs(
                r23 - (distance(X1[0], X1[1], x2, y2) - distance(X1[0], X1[1], x3, y3))) ** 2 + abs(
                r12 - (distance(X1[0], X1[1], x1, y1) - distance(X1[0], X1[1], x2, y2))) ** 2 + abs(
                r13 - (distance(X1[0], X1[1], x1, y1) - distance(X1[0], X1[1], x3, y3))) ** 2


            # print(loss)
            position = X1

        else:
            loss = abs(r30 - (distance(X2[0], X2[1], x3, y3) - distance(X2[0], X2[1], x0, y0))) ** 2 + abs(
                r23 - (distance(X2[0], X2[1], x2, y2) - distance(X2[0], X2[1], x3, y3))) ** 2 + abs(
                r12 - (distance(X2[0], X2[1], x1, y1) - distance(X2[0], X2[1], x2, y2))) ** 2 + abs(
                r13 - (distance(X2[0], X2[1], x1, y1) - distance(X2[0], X2[1], x3, y3))) ** 2
            # print(loss)

            position = X2
        # print(loss)
        cost_loss = cost_loss + loss

    # print(position)
    return position

# TDOA
def tdoa1(stations, tds):
    # 结果

    global cost_loss
    position = np.array([])
    # 得到基站的坐标（真实）
    x0, y0 = stations[0]
    x1, y1 = stations[1]
    x2, y2 = stations[2]
    x3, y3 = stations[3]

    # 得到某个ue到be的距离差
    r10 = tds[0]
    r20 = tds[1]
    r30 = tds[2]
    r12 = tds[3]
    r13 = tds[4]
    r23 = tds[5]
    # 基站之间的距离
    x10 = x1 - x0
    x20 = x2 - x0
    y10 = y1 - y0
    y20 = y2 - y0

    k0 = x0 ** 2 + y0 ** 2
    k1 = x1 ** 2 + y1 ** 2
    k2 = x2 ** 2 + y2 ** 2

    A = np.array([[x10, y10], [x20, y20]])
    C = np.array([-r10, -r20])
    D = np.array([(k1 - k0 - r10 ** 2) / 2, (k2 - k0 - r20 ** 2) / 2])

    # 求解Ax = r0 * C + D
    # 使用linalg.solve()求解线性方程组
    a = np.linalg.solve(A, C)
    b = np.linalg.solve(A, D)

    A_ = (a[0] ** 2) + (a[1] ** 2) - 1
    B_ = a[0] * (b[0] - x0) + a[1] * (b[1] - y0)
    C_ = (x0 - b[0]) ** 2 + (y0 - b[1]) ** 2
    if (B_ ** 2) - A_ * C_ < 0:
        position = [10, 10]

    else:
        r0_1 = -(B_ + sqrt((B_ ** 2) - A_ * C_)) / A_
        r0_2 = -(B_ - sqrt((B_ ** 2) - A_ * C_)) / A_
        X1 = a * r0_1 + b
        X2 = a * r0_2 + b

        if abs(r12 - (distance(X1[0], X1[1], x1, y1) - distance(X1[0], X1[1], x2, y2))) < 0.1:

            position = X1
        else:

            position = X2


    return position

# 计算欧式距离
def distance(x1, y1, x2, y2):
    return sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)


# 得到光传播的距离
def get_light_dis(light_ts):
    return c * light_ts


# 计算分数
def caculate_score():
    global ue_res, be_res
    pos_true = read_true_txt()
    # 转变为list
    pos_true_list = list(pos_true)

    s = 0.0
    for i in range(len(pos_true_list)):
        s = s + (pos_true_list[i][0] - ue_res[i][0]) ** 2 + (pos_true_list[i][1] - ue_res[i][1]) ** 2

    s = s / len(pos_true_list)
    s = sqrt(s)
    # 计算be
    s_be = 0.0    #     # -3, -4.504, 4.356
    #     # -4, 7.651, 8.632
    be_pos_true = [[-4.504, 4.356], [7.651, 8.632]]

    for i in range(2):
        s_be = s_be + (be_res[i][0] - be_pos_true[i][0]) ** 2 + (be_res[i][1] - be_pos_true[i][1]) ** 2

    s_be = s_be / 2
    s_be = sqrt(s_be)

    print("最终的分数为：", 50 / (1 + s) + 50 / (1 + s_be))


# 解决方案
def solve_brute():
    global ue_res, be_res
    global cost_loss, best_loss
    # 返回每个ue到be的测量时间数据（有误差，但是到每个be的时间差是真实的）
    ts = read_txt()
    pos_true = read_true_txt()
    # 转变为list
    pos_true_list = list(pos_true)
    ts_list = list(ts)
    # 定义步长
    step_1 = 1
    step_2 = 1
    # 枚举(bs2)的x位置和y位置(-10 <= x <= 0; 0 <= y <= 10)
    # 从坐下角开始枚举
    bs2_x_start = -10
    bs2_y_start = 0
    bs3_x_start = 0
    bs3_y_start = 0
    while bs2_x_start < 0:
        bs2_y_start = 0
        while bs2_y_start < 10:
            # 枚举bs3坐标
            bs3_x_start = 0
            while bs3_x_start < 10:
                bs3_y_start = 0
                while bs3_y_start < 10:
                    # 计算损失同时保存当前结果
                    stations = [[-5, -5], [5, -5], [bs2_x_start, bs2_y_start], [bs3_x_start, bs3_y_start]]
                    ue_cur = []
                    be_cur = [[bs2_x_start, bs2_y_start], [bs3_x_start, bs3_y_start]]
                    for i in range(len(ts_list)):
                        ue_0, ue_1, ue_2, ue_3 = ts_list[i]
                        r0_real = get_light_dis(ue_0)
                        r1_real = get_light_dis(ue_1)
                        r2_real = get_light_dis(ue_2)
                        r3_real = get_light_dis(ue_3)
                        tds = [r1_real - r0_real, r2_real - r0_real, r3_real - r0_real]

                        # print(tds)
                        position = tdoa(stations, tds, 0.5)
                        ue_cur.append(position)

                    # print(bs2_x_start,' ', bs2_y_start,' ', bs3_x_start, ' ' ,bs3_y_start)
                    # print(cost_loss)
                    if cost_loss < best_loss:
                        best_loss = cost_loss
                        ue_res = ue_cur
                        be_res = be_cur

                    cost_loss = 0.0

                    bs3_y_start = bs3_y_start + step_2
                bs3_x_start = bs3_x_start + step_2
            bs2_y_start = bs2_y_start + step_1

        bs2_x_start = bs2_x_start + step_1

    print(best_loss)

    print(be_res)
    print(ue_res)




# pso(允许be2和be3可以改变）
def pso1():
    # ts = read_txt()
    # ts_list = list(ts)
    ue_number, ts_list, be_list = read_test_data(test_file)
    # read_may_maybe_deviation
    maybe_deviation = []
    ret = read_py_txt(maybe_deviation_path)
    # print(len(ret))
    deviation_res = []
    # 调整距离差
    for i in range(ue_number):
        for j in range(4):
            ts_list[i][j] = ret[i][j]
    # print(ue_number)
    # print(ts_list)
    # print(be_list)
    global ue_res, be_res
    global cost_loss, best_loss
    # stations = [[-5, -5], [5, -5], [-5.000,-5.000],[7.000, -4.000]]
    stations = [be_list[0], be_list[1], [0, 0], [0, 0]]
    # -2 3 7 9
    # print(stations)
    # 粒子群相关参数
    n = 400 # 粒子个数
    max_iter = 8  # 迭代次数
    vmax = 0.2
    y = [0 for i in range(n)]

    x = [[] for i in range(n)]
    v = [[] for i in range(n)]
    c = [4 , 4]
    p_best = [[] for i in range(n)]
    p_best_position = [[] for i in range(n)]
    g_best = 1e30
    g_best_positon = []

    # 预处理实际的路程（有误差）
    hash_map = dict()
    for i in range(len(ts_list)):
        ue_0, ue_1, ue_2, ue_3 = ts_list[i]
        hash_map[ue_0] = get_light_dis(ue_0)
        hash_map[ue_1] = get_light_dis(ue_1)
        hash_map[ue_2] = get_light_dis(ue_2)
        hash_map[ue_3] = get_light_dis(ue_3)

    # 预处理每个ue到四个be的路程差

    route_diff_map = [[] for i in range(len(ts_list))]

    for i in range(len(ts_list)):
        ue_0, ue_1, ue_2, ue_3 = ts_list[i]
        r0_real = hash_map[ue_0]
        r1_real = hash_map[ue_1]
        r2_real = hash_map[ue_2]
        r3_real = hash_map[ue_3]
        route_diff_map[i] = [r1_real - r0_real, r2_real - r0_real, r3_real - r0_real, r1_real - r2_real, r1_real - r3_real, r2_real - r3_real]




    # 初始化(对应be2_x,be2_y,be3_x,be3_y)
    add = 1



    for i in range(n):
        v[i] = [0.01, 0.01, 0.01, 0.01]

    # -37.8375796
    # 11.3409
    # 3.464544
    # 12.418274
    for i in range(n):
        xx_1 = random.uniform(-38, -37)
        yy_1 = random.uniform(11, 12)
        xx_2 = random.uniform(3, 4)
        yy_2 = random.uniform(12, 13)
        # x[i] = [xx_1, yy_1, xx_2, yy_2]
        x[i] = [xx_1, yy_1, xx_2, yy_2]
        # print(x[i])


    # 假设得到实际的be坐标(根据实际的坐标求解距离
    # 计算适应度或者损失函数
    def fitness():
        global cost_loss
        for i in range(n):
            cost_loss = 0.0
            stations[2] = x[i][0:2]
            stations[3] = x[i][2:]
            for j in range(len(ts_list)):
                # ri_real只是用来计算tds，实际上它会带有时钟误差，而这个误差我们不能直接得到
                tds = route_diff_map[j]
                position = tdoa(stations, tds, 0.5)
            y[i] = cost_loss




    res = [0, 0, 0, 0]
    # 计算适应度
    fitness()
    print("init_ok")
    # 初始化当前个体最优位置
    for i in range(n):
        p_best[i] = [y[i], y[i], y[i], y[i]]
        p_best_position[i] = x[i].copy()

        if y[i] < g_best:
            g_best = y[i]
            g_best_positon = x[i].copy()
            res[0], res[1], res[2], res[3] = x[i]
            # print(x[i])



    # 开始迭代
    w_max = 0.8
    w_min = 0.4
    for i in range(max_iter):
        w = w_max - (w_max - w_min) * ((i) / max_iter)
        c[0] = c[0] - 2 * 1 / max_iter
        # vmax = vmax * 0.8
        for j in range(n):

            x[j][0] += v[j][0]
            x[j][1] += v[j][1]
            x[j][2] += v[j][2]
            x[j][3] += v[j][3]

            v[j][0] = w * v[j][0] + c[0] * random.uniform(0, 1) * (p_best_position[j][0] - x[j][0]) + c[1] * random.uniform(0, 1) * (g_best_positon[0] - x[j][0])
            v[j][1] = w * v[j][1] + c[0] * random.uniform(0, 1) * (p_best_position[j][1] - x[j][1]) + c[1] * random.uniform(0, 1) * (g_best_positon[1] - x[j][1])
            v[j][2] = w * v[j][2] + c[0] * random.uniform(0, 1) * (p_best_position[j][2] - x[j][2]) + c[1] * random.uniform(0, 1) * (g_best_positon[2] - x[j][2])
            v[j][3] = w * v[j][3] + c[0] * random.uniform(0, 1) * (p_best_position[j][3] - x[j][3]) + c[1] * random.uniform(0, 1) * (g_best_positon[3] - x[j][3])

            for k in range(4):
                if v[j][k] > vmax:
                    v[j][k] = vmax
                elif v[j][k] < -vmax:
                    v[j][k] = -vmax



            # if x[j][0] < -10:
            #     x[j][0] = -10
            # if x[j][0] > 0:
            #     x[j][0] = 0
            # for k in range(3):
            #     if x[j][k + 1] < 0:
            #         x[j][k + 1] = 0
            #     elif x[j][k + 1] > 10:
            #         x[j][k + 1] = 10





        fitness()

        # 更新个体极值和群体极值
        for j in range(n):

            # 更新个体最优值
            for k in range(4):
                if y[j] < p_best[j][k]:
                    p_best[j][k] = y[j]
                    p_best_position[j][k] = x[j][k]
            # 更新群体最优值
            if y[j] < g_best:
                g_best = y[j]
                g_best_position = x[j].copy()
                res[0], res[1], res[2], res[3] = x[j]










            # # print(x[j])
            # # print(y[j])
            # p_best[j][0] = min(p_best[j][0], y[j])
            # p_best[j][1] = min(p_best[j][1], y[j])
            # p_best[j][2] = min(p_best[j][2], y[j])
            # p_best[j][3] = min(p_best[j][3], y[j])
            # if y[j] < g_best:
            #     g_best = y[j]
            #     res[0],res[1],res[2],res[3] = x[j]




        print(res)
        print(g_best)

    return res, g_best






if __name__ == "__main__":







    random.seed(1)

    # -12.994,-4.752
    # 12.915,-24.941
    res, ans = pso1()
    print(res)
    print(ans)

    #
    stations = [[-12.994, -4.752], [12.915, -24.941], [res[0], res[1]], [res[2], res[3]]]
    # 输出ue坐标
    # ue_number, ts_list, be_list = read_test_data(test_file)
    # ts = read_txt()
    # ts_list = list(ts)
    ue_number, ts_list, be_list = read_test_data(test_file)
    # read_may_maybe_deviation
    maybe_deviation = []
    ret = read_py_txt(maybe_deviation_path)
    # print(len(ret))
    deviation_res = []
    # 调整距离差
    for i in range(ue_number):
        for j in range(4):
            ts_list[i][j] = ret[i][j]

    for i in range(len(ts_list)):
        ue_0, ue_1, ue_2, ue_3 = ts_list[i]
        r0_real = get_light_dis(ue_0)
        r1_real = get_light_dis(ue_1)
        r2_real = get_light_dis(ue_2)
        r3_real = get_light_dis(ue_3)
        # ri_real只是用来计算tds，实际上它会带有时钟误差，而这个误差我们不能直接得到
        tds = [r1_real - r0_real, r2_real - r0_real, r3_real - r0_real, r1_real - r2_real, r1_real - r3_real,
               r2_real - r3_real]

        position = tdoa(stations, tds, 0.5)
        ue_res.append(position)

    be_res = [[res[0], res[1]], [res[2], res[3]]]
    #
    print(be_res)
    print(ue_res)


    # 写入txt文件
    with open(folder_path + '/out_est.txt', 'w') as file:
        # 写入be
        for i in range(2):
            file.write(str(be_res[i][0]) + " " + str(be_res[i][1]) + "\n")

        # 写入ue
        for i in range(len(ue_res)):
            file.write(str(ue_res[i][0]) + " " + str(ue_res[i][1]) + "\n")
        # file.write(str(head) + "\n")
        # file.write("[\"io count\"]" + "\n")
        # file.write(str([io_count]) + "\n")
        # file.write("[\"io\":\"id\",\"wrap\",\"startLpos\",\"endLpos\"]\n")


    # caculate_score()









