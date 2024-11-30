#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jul 30 20:23:07 2021

@author: petulaa
"""

import numpy as np
import sys
import os



""" Should be in format  """
bingo_file = None

if len(sys.argv) > 1:
    bingo_file = sys.argv[1]
else:
    bingo_file = '16Star.txt'

if not os.path.exists(bingo_file):
    bingo_file = "data/" + bingo_file

if not os.path.exists(bingo_file):
    print("Error: file not found")
    exit(1)

#
# NOTE(abbie): Rock and roll!
#

goals_list = np.genfromtxt(bingo_file, dtype='str', delimiter='/n')

n_goals = len(goals_list)

card_list = []
chosen_goals = []
while len(chosen_goals) < 25:
    goal_num = np.random.randint(0,n_goals)
    if goal_num not in chosen_goals:
        chosen_goals.append(goal_num)
        card_list.append('{"name": "' + goals_list[goal_num] + '"}') 
        
        
print ('[')
print(*card_list, sep =', ')
print(']')
