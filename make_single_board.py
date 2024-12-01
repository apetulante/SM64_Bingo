# Create a bingo board json from a txt
# Ex.
# python make_single_board.py --iterations 50000000 --tolerance 1 --showImage

import numpy as np
import sys
import os
import json
import argparse
import re
import time
from tqdm import tqdm
import matplotlib.pyplot as plt
import platform

# Parse arguments
parser = argparse.ArgumentParser(description="parameters")
parser.add_argument("--input", type=str, help="Path to tile list .txt", default="16star.txt")
parser.add_argument("--iterations", type=int, help="Max iterations to attempt", default=1000000)
parser.add_argument("--tolerance", type=float, help="Max allowed difference in difficulty between easiest and hardest row/col", default=1)
parser.add_argument("--seed", type=int, help="Random seed")
parser.add_argument("--showImage", action='store_true', help="Show debug image of final board")
args = parser.parse_args()
bingo_file = args.input
iterations = args.iterations
tolerance = args.tolerance
showImage = args.showImage
seed = args.seed if args.seed is not None else int(time.time())

# Check if file exists
if not os.path.exists(bingo_file):
    bingo_file = "data/" + bingo_file
if not os.path.exists(bingo_file):
    print("Error: file not found:", bingo_file)
    exit(1)

# Convert board to json format
def board_to_json(board):
    result = []
    for i in range(25):
        result.append({ 'name': board[i]['name'], 'score': board[i]['score'] })

    return json.dumps(result)

def is_magic_square_with_tolerance(arr, tolerance):
    row_sums = arr.sum(axis=1)
    col_sums = arr.sum(axis=0)
    main_diag_sum = np.sum(np.diagonal(arr))
    anti_diag_sum = np.sum(np.diagonal(np.fliplr(arr)))
    
    # Combine sums into a single array
    all_sums = np.concatenate((row_sums, col_sums, [main_diag_sum, anti_diag_sum]))
    return (np.max(all_sums) - np.min(all_sums)) <= tolerance

# Display a debug image showing grid with tile names
def show_board_preview(names):
    # Reshape the list into a 5x5 grid (2D array)
    names_grid = np.array(names).reshape(5, 5)

    # Create a plot with no axes
    fig, ax = plt.subplots(figsize=(6, 6))
    ax.axis('off')

    # Display each name in the grid
    for i in range(5):
        for j in range(5):
            ax.text(j, 4-i, names_grid[i, j], ha='center', va='center', fontsize=5)

    # Add gridlines between cells
    ax.set_xticks(np.arange(0, 6, 1))  # X grid lines
    ax.set_yticks(np.arange(0, 6, 1))  # Y grid lines
    ax.grid(which='both', color='black', linestyle='-', linewidth=1)  # Customize gridline appearance

    # Show the plot
    plt.show()

def clear_console():
    if platform.system() == "Windows":
        os.system("cls")
    else:  # For macOS and Linux
        os.system("clear")

def print_success():
    print(" _       __    _                   __                    _ __                       _       __")
    print("| |     / /__ ( )   _____     ____/ /___  ____  ___     (_) /_   ____ _____ _____ _(_)___  / /")
    print("| | /| / / _ \|/ | / / _ \   / __  / __ \/ __ \/ _ \   / / __/  / __ `/ __ `/ __ `/ / __ \/ / ")
    print("| |/ |/ /  __/ | |/ /  __/  / /_/ / /_/ / / / /  __/  / / /_   / /_/ / /_/ / /_/ / / / / /_/  ")
    print("|__/|__/\___/  |___/\___/   \__,_/\____/_/ /_/\___/  /_/\__/   \__,_/\__, /\__,_/_/_/ /_(_)   ")
    print("                                                                    /____/                    ")

# List of every tile name with difficulty score
lines = np.genfromtxt(bingo_file.strip(), dtype='str', delimiter='/n')

# Get all difficulty scores
# Score is the value between "[]"
difficulty_scores = []
for line in lines:
    match = re.search(r"\[([\d.]+)\]", line)        # find value between "[]"
    difficulty_scores.append(float(match.group(1))) # extract value between "[]" and append to list

# Shuffle tiles
# Break if conditions are met for a good board
difficulty_scores = np.array(difficulty_scores)
lenVal = len(difficulty_scores)
t0 = time.time()
success = False
for _ in tqdm(range(iterations), desc="Crunching the Numbers"):

    # Get random order for candidate board
    shuffled_indices = np.random.choice(lenVal, size=25, replace=False)

    # Create random board
    scores_grid = difficulty_scores[shuffled_indices].reshape(5, 5)

    # Check that the sums of each row/col pass conditions
    if is_magic_square_with_tolerance(scores_grid, tolerance=tolerance):
        success = True
        break

# Print results
clear_console()
if success:
    print_success()
    row_sums = np.sum(scores_grid, axis=1)
    col_sums = np.sum(scores_grid, axis=0)
    main_diag_sum = np.sum(np.diagonal(scores_grid))
    anti_diag_sum = np.sum(np.diagonal(np.fliplr(scores_grid)))
    all_sums = np.concatenate([row_sums, col_sums, [main_diag_sum, anti_diag_sum]])
    max_sum = np.max(all_sums)
    min_sum = np.min(all_sums)
    print("\nHardest Difficulty Row Sum: ", max_sum)
    print("Easiest Difficulty Row Sum: ", min_sum)

    # Print final scores grid
    print("\nFinal GRID:")
    print(scores_grid)

    # Create json
    out = []
    names_list = []
    cnt = 0
    scores_rand = difficulty_scores[shuffled_indices]
    for i, score in enumerate(scores_rand):
        name = lines[shuffled_indices[i]]
        out.append({ 'name': name, 'score': score })
        names_list.append(name)
        cnt += 1
        if cnt == 25:
            break
    out = np.array(out)
    print("\nFinal JSON:")
    print(board_to_json(out))

else:
    print("Failed to make board")

# Print time to run
print("\nTook ", np.round(time.time()-t0,2), " sec")

# Show debug image
if showImage:
    show_board_preview(names_list)