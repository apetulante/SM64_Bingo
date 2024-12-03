#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jul 30 20:23:07 2021

@author: petulaa
"""

import numpy as np
import sys
import os
import json

#
# Parameters:
#

iterations = 20_000
min_threshold = 7
bingo_file = '16Star.txt'
seed = None

if len(sys.argv) > 1:
    bingo_file = sys.argv[1]

# NOTE(nick): parse args
args = " ".join(sys.argv[1:]).replace("=", " ").replace("--", "-").split(" ")
for i in range(len(args)):
    key = args[i]
    int_param = None
    try:
        int_param = int(args[i+1])
    except:
        pass

    if key == "-i" or key == "-input":
        bingo_file = args[i+1]

    if int_param is not None:
        if key == "-n" or key == "-iterations":
            iterations = int_param

        if key == "-t" or key == "-threshold":
            min_threshold = int_param

        if key == "-s" or key == "-seed":
            seed = int_param


# NOTE(nick): verify file exists
if not os.path.exists(bingo_file):
    bingo_file = "data/" + bingo_file

if not os.path.exists(bingo_file):
    print("Error: file not found:", bingo_file)
    exit(1)

MAX_SCORE = 999999999999999

def score_run(nums):
    return np.sum(nums)

def compute_final_score(nums):
    if np.any(np.less(nums, min_threshold)):
        return MAX_SCORE

    avg = np.mean(nums)
    return np.linalg.norm(nums - avg)

def score_board_runs(board):
    scores = np.zeros(5+5+2)
    sc = 0
    nums = np.zeros(5)

    # rows
    for y in range(5):
        for x in range(5):
            nums[x] = board[y*5+x]['score']
        scores[sc] = score_run(nums)
        sc += 1

    # cols
    for x in range(5):
        for y in range(5):
            nums[y] = board[y*5+x]['score']
        scores[sc] = score_run(nums)
        sc += 1

    # diagonal TL -> BR
    for i in range(5):
        nums[i] = board[5*i+i]['score']
    scores[sc] = score_run(nums)
    sc += 1

    # diagonal TR -> BL
    for i in range(5):
        nums[i] = board[5*i + (5 - i)]['score']
    scores[sc] = score_run(nums)
    sc += 1

    return scores


def score_board(board):
    scores = score_board_runs(board)
    result = compute_final_score(scores)
    return result

def board_to_json(board):
    result = []
    for i in range(25):
        result.append({ 'name': board[i]['name'], 'score': board[i]['score'] })
        # result.append({ 'name': board[i]['name'] })
        # result.append({ 'name': board[i]['name'] + " [" + str(board[i]['score']) + "]" })

    return json.dumps(result)



#
# NOTE(abbie): Rock and roll!
#

lines = np.genfromtxt(bingo_file.strip(), dtype='str', delimiter='/n')

goals = []
for i in range(len(lines)):
    line = lines[i]
    parts = line.split('[', maxsplit=1)
    name = parts[0].strip()
    score = 0
    if len(parts) > 1:
        score = parts[1].split(']', maxsplit=1)[0]
        score = float(score)
    goals.append({ 'name': name, 'score': score })

goals = np.array(goals)

best_list = np.copy(goals)
best_score = MAX_SCORE

if seed is not None:
    np.random.seed(seed)

i = 0
while i < iterations:
    np.random.shuffle(goals)

    score = score_board(goals)
    if score < best_score:
        best_score = score
        best_list = np.copy(goals)

    i += 1

# NOTE(nick): output results
scores = score_board_runs(best_list)
avg = np.mean(scores)

print("Iterations Run:", iterations)
print("Best Score:", best_score)

if best_score == MAX_SCORE:
    print("No solutions found!")
    exit(1)

print("Difficulty:", avg)
minx = np.min(scores)
maxx = np.max(scores)
print("Spread:", maxx-minx, "(min: " + str(minx) + ", max: " + str(maxx) + ")")

print("---")
print(board_to_json(best_list))
print("---")
