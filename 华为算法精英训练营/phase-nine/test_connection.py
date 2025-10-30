# -*- coding: utf-8 -*-
'''
@project  : github_1
@File     : test_connection.py
@Author   : XiaoJuLi
@IDE      : PyCharm
@Software : PyCharm
@Date     :2024/10/9 14:30
'''
import torch
print(torch.__version__)
print(torch.cuda.is_available())
print(torch.cuda.get_device_name())