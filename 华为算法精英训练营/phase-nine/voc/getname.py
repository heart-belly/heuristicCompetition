# -*- coding: utf-8 -*-
'''
@project  : pythonCode
@File     : getname.py
@Author   : XiaoJuLi
@IDE      : PyCharm
@Software : PyCharm
@Date     :2024/10/4 19:49
'''

import os


def get_files_in_directory(directory):
    files = []
    for root, dirs, filenames in os.walk(directory):
        for filename in filenames:
            # 使用 os.path.splitext 分离文件名和后缀
            base_name = os.path.splitext(filename)[0]
            files.append(base_name)
    return files


directory_path = './JPEGImages/'
file_names = [os.path.basename(file) for file in get_files_in_directory(directory_path)]
for name in file_names:
    print(name)