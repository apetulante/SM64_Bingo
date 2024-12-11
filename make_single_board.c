#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define MAX_LINES 1000
#define LINE_LENGTH 256
#define BOARD_SIZE 25

// Structure to represent a bingo tile
typedef struct {
    char name[LINE_LENGTH];
    float score;
} Tile;

// Configuration parameters with default values
typedef struct {
    char input_file[256];
    int iterations;
    float tolerance;
    int seed;
    float minDifficulty;
    bool show_image;
} Config;

// Global variables
Tile tiles[MAX_LINES];
int total_tiles = 0;
Config config = {
    .input_file = "./data/16star.txt",
    .iterations = 100000000,
    .tolerance = 1.0,
    .seed = 0,
    .minDifficulty = 0.0,
    .show_image = false
};

// Function prototypes
void parse_arguments(int argc, char *argv[]);
void read_bingo_file();
bool is_magic_square_with_tolerance(float grid[5][5], float tolerance, float minDifficulty);
void generate_board();
void print_success();
void print_board_details(float grid[5][5]);

// Argument parsing
void parse_arguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            strcpy(config.input_file, argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            config.iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--tolerance") == 0 && i + 1 < argc) {
            config.tolerance = atof(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            config.seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--showImage") == 0) {
            config.show_image = true;
        } else if (strcmp(argv[i], "--minDifficulty") == 0 && i + 1 < argc) {
            config.minDifficulty = atof(argv[++i]);
        }
    }
}

// Read bingo file and parse tiles
void read_bingo_file() {
    FILE *file = fopen(config.input_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", config.input_file);
        exit(1);
    }

    char line[LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && total_tiles < MAX_LINES) {
        // Extract name and score from line
        char *score_start = strchr(line, '[');
        char *score_end = strchr(line, ']');
        
        if (score_start && score_end) {
            *score_end = '\0';
            tiles[total_tiles].score = atof(score_start + 1);
            *score_start = '\0';
            strcpy(tiles[total_tiles].name, line);
            total_tiles++;
        }
    }
    fclose(file);
}

// Check if grid meets magic square tolerance
// Also check if min score is above minDifficulty
bool is_magic_square_with_tolerance(float grid[5][5], float tolerance, float minVal) {
    float row_sums[5] = {0};
    float col_sums[5] = {0};
    float main_diag_sum = 0;
    float anti_diag_sum = 0;

    // Calculate row and column sums
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            row_sums[i] += grid[i][j];
            col_sums[j] += grid[i][j];
        }
        main_diag_sum += grid[i][i];
        anti_diag_sum += grid[i][4-i];
    }

    // Find max and min sums
    float max_sum = row_sums[0];
    float min_sum = row_sums[0];
    
    for (int i = 0; i < 5; i++) {
        max_sum = fmaxf(max_sum, row_sums[i]);
        min_sum = fminf(min_sum, row_sums[i]);
        max_sum = fmaxf(max_sum, col_sums[i]);
        min_sum = fminf(min_sum, col_sums[i]);
    }
    max_sum = fmaxf(max_sum, main_diag_sum);
    max_sum = fmaxf(max_sum, anti_diag_sum);
    min_sum = fminf(min_sum, main_diag_sum);
    min_sum = fminf(min_sum, anti_diag_sum);

    if (min_sum < minVal) {
        return false;
    }

    return (max_sum - min_sum) <= tolerance;
}

// Generate bingo board
void generate_board() {
    // Set random seed
    srand(config.seed ? config.seed : time(NULL));

    bool success = false;
    float scores_grid[5][5];
    int shuffled_indices[25];

    // Loop through iterations until a sufficient board is found
    for (int iter = 0; iter < config.iterations; iter++) {
        // Create unique random selection of 25 tiles
        for (int i = 0; i < 25; i++) {
            int index;
            bool unique;
            do {
                unique = true;
                index = rand() % total_tiles;
                for (int j = 0; j < i; j++) {
                    if (shuffled_indices[j] == index) {
                        unique = false;
                        break;
                    }
                }
            } while (!unique);
            shuffled_indices[i] = index;
        }

        // Create grid of scores
        for (int i = 0; i < 25; i++) {
            int row = i / 5;
            int col = i % 5;
            scores_grid[row][col] = tiles[shuffled_indices[i]].score;
        }

        // Check magic square conditions
        if (is_magic_square_with_tolerance(scores_grid, config.tolerance, config.minDifficulty)) {
            success = true;
            
            // Print success and board details
            print_success();
            print_board_details(scores_grid);

            // Print JSON-like output
            printf("\nFinal JSON:\n[");
            for (int i = 0; i < 25; i++) {
                Tile tile = tiles[shuffled_indices[i]];
                printf("%s{\"name\": \"%s\", \"score\": %.2f}", 
                       i > 0 ? ", " : "", tile.name, tile.score);
            }
            printf("]\n");
            break;
        }
    }

    if (!success) {
        printf("Failed to make board\n");
    }
}

// Print ASCII success art
void print_success() {
    printf(" _       __    _                   __                    _ __                       _       __\n");
    printf("| |     / /__ ( )   _____     ____/ /___  ____  ___     (_) /_   ____ _____ _____ _(_)___  / /\n");
    printf("| | /| / / _ \\|/ | / / _ \\   / __  / __ \\/ __ \\/ _ \\   / / __/  / __ `/ __ `/ __ `/ / __ \\/ / \n");
    printf("| |/ |/ /  __/ | |/ /  __/  / /_/ / /_/ / / / /  __/  / / /_   / /_/ / /_/ / /_/ / / / / /_/  \n");
    printf("|__/|__/\\___/  |___/\\___/   \\__,_/\\____/_/ /_/\\___/  /_/\\__/   \\__,_/\\__, /\\__,_/_/_/ /_(_)   \n");
    printf("                                                                    /____/                    \n");
}

// Print board details
void print_board_details(float grid[5][5]) {
    float row_sums[5] = {0};
    float col_sums[5] = {0};
    float main_diag_sum = 0;
    float anti_diag_sum = 0;

    // Calculate sums
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            row_sums[i] += grid[i][j];
            col_sums[j] += grid[i][j];
        }
        main_diag_sum += grid[i][i];
        anti_diag_sum += grid[i][4-i];
    }

    // Get min/max sums for printing
    float min_sum = INFINITY;
    float max_sum = -INFINITY;

    // Check row sums
    for (int i = 0; i < 5; i++) {
        min_sum = fminf(min_sum, row_sums[i]);
        max_sum = fmaxf(max_sum, row_sums[i]);
    }

    // Check column sums
    for (int j = 0; j < 5; j++) {
        min_sum = fminf(min_sum, col_sums[j]);
        max_sum = fmaxf(max_sum, col_sums[j]);
    }

    // Check diagonal sums
    min_sum = fminf(min_sum, main_diag_sum);
    max_sum = fmaxf(max_sum, main_diag_sum);
    min_sum = fminf(min_sum, anti_diag_sum);
    max_sum = fmaxf(max_sum, anti_diag_sum);

    printf("\nHardest Sum: %.1f\n", max_sum);
    printf("Easiest Sum: %.1f\n", min_sum);

    printf("\nFinal GRID:\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            printf("%.2f ", grid[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    parse_arguments(argc, argv);

    // Read bingo tile file
    read_bingo_file();

    // Generate board
    generate_board();

    return 0;
}