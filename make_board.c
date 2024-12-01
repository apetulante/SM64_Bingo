#define impl
#include "na.h"
#include <math.h>

const f64 MAX_SCORE = 999999999999999;
f64 min_threshold = 7;

typedef struct Entry Entry;
struct Entry
{
    String name;
    f64 score;
};

typedef struct Goals Goals;
struct Goals
{
    Entry *data;
    u64 count;
};

f64 scores[12] = {0};
Array_f64 score_board_runs(Goals board)
{
    Array_f64 result = {0};
    result.data = scores;
    result.count = 12;

    int sc = 0;

    // rows
    for (int y = 0; y < 5; y += 1)
    {
        f64 score = 0;
        for (int x = 0; x < 5; x += 1)
        {
            score += board.data[y*5+x].score;
        }

        scores[sc] = score;
        sc += 1;
    }

    // cols
    for (int x = 0; x < 5; x += 1)
    {
        f64 score = 0;
        for (int y = 0; y < 5; y += 1)
        {
            score += board.data[y*5+x].score;
        }

        scores[sc] = score;
        sc += 1;
    }

    // diagonal TL -> BR
    f64 score1 = 0;
    for (int i = 0; i < 5; i += 1)
    {
        score1 += board.data[5*i+i].score;
    }
    scores[sc] = score1;
    sc += 1;

    // diagonal TR -> BL
    f64 score2 = 0;
    for (int i = 0; i < 5; i += 1)
    {
        score2 += board.data[5*i + (5 - i)].score;
    }
    scores[sc] = score2;
    sc += 1;

    return result;
}

f64 compute_mean(f64 *array, int count)
{
    f64 sum = 0.0;
    for (int i = 0; i < count; i += 1)
    {
        sum += array[i];
    }

    f64 mean = sum/count;
    return mean;
}

f32 frobenius_norm(double *array, int count)
{
    f32 result = 0.0f;

    if (array != NULL && count > 0)
    {
        f32 sum = 0.0f;
        for (int i = 0; i < count; i++) {
            sum += array[i] * array[i];
        }

        result = sqrtf(sum);
    }
    
    return result;
}

f64 comptue_final_score(Array_f64 scores)
{
    for (int i = 0; i < scores.count; i += 1)
    {
        if (scores.data[i] < min_threshold)
        {
            return MAX_SCORE;
        }
    }

    f64 mean = compute_mean(scores.data, scores.count);

    for (int i = 0; i < scores.count; i += 1)
    {
        scores.data[i] -= mean;
    }

    return frobenius_norm(scores.data, scores.count);
}

f64 score_board(Goals board)
{
    Array_f64 scores = score_board_runs(board);
    return comptue_final_score(scores);
}

void print_board_to_json(Goals board)
{
    print("[");
    for (int i = 0; i < 25; i += 1)
    {
        String name = string_replace(temp_arena(), board.data[i].name, S("\""), S("\\\""), 0);
        f32 score = board.data[i].score;

        print("{ \"name\": \"%.*s\", \"score\": %.1f }", LIT(name), score);
        // print("{ \"name\": \"%.*s\" }", LIT(name));
        // print("{ \"name\": \"%.*s [%.1f]\" }", LIT(name), score);

        if (i < 25-1) { print(", "); }
    }
    print("]");
}

int main(int argc, char **argv)
{
    os_init();
    random_init();

    //
    // Parameters:
    //

    int iterations = 20000;
    min_threshold = 7;
    String bingo_file = S("16Star.txt");
    u64 seed = 0;

    if (argc > 1) {
        bingo_file = string_from_cstr(argv[1]);
    }

    // NOTE(nick): parse args
    for (int i = 0; i < argc-1; i += 1)
    {
        String key = string_from_cstr(argv[i]);
        String value = string_from_cstr(argv[i+1]);

        if (string_equals(key, S("-i")) || string_includes(key, S("-input")))
        {
            bingo_file = value;
            i += 1;
        }

        if (string_equals(key, S("-n")) || string_includes(key, S("-iterations")))
        {
            iterations = string_to_i64(value, 10);
            i += 1;
        }

        if (string_equals(key, S("-t")) || string_includes(key, S("-threshold")))
        {
            min_threshold = string_to_f64(value);
            i += 1;
        }

        if (string_equals(key, S("-s")) || string_includes(key, S("-seed")))
        {
            seed = (u64)string_to_i64(value, 10);
            i += 1;
        }
    }

    // NOTE(nick): verify file exists
    if (!os_file_exists(bingo_file)) {
        bingo_file = path_join(S("data"), bingo_file);
    }

    if (!os_file_exists(bingo_file)) {
        print("Error: File not found: %S\n", bingo_file);
        return 1;
    }

    if (seed != 0)
    {
        random_set_seed(seed);
    }

    //
    // NOTE(abbie): Rock and roll!
    //

    String text = os_read_entire_file(temp_arena(), bingo_file);
    text = string_trim_whitespace(text);

    String_Array lines = string_array_from_list(temp_arena(), string_split(temp_arena(), text, S("\n")));

    Goals goals = {0};
    goals.data = PushArrayZero(temp_arena(), Entry, lines.count);
    goals.count = lines.count;

    for (int i = 0; i < lines.count; i += 1)
    {
        String line = lines.data[i];
        String name = line;
        f64 score = 0.0;

        i64 index = string_find(line, S("["), 0, 0);
        if (index < line.count)
        {
            name = string_trim_whitespace(string_slice(line, 0, index));

            i64 end_index = string_find(line, S("]"), index, 0);
            String part = string_slice(line, index+1, end_index);
            score = (f64)string_to_f64(part);
        }
        // print("%.*s, %.2f\n", LIT(name), score);

        goals.data[i].name = name;
        goals.data[i].score = score;
    }

    Goals best_list = {0};
    best_list.count = goals.count;
    best_list.data = PushArrayZero(temp_arena(), Entry, goals.count);
    MemoryCopy(best_list.data, goals.data, sizeof(Entry)*goals.count);

    f64 best_score = MAX_SCORE;

    for (int i = 0; i < iterations; i += 1)
    {
        random_shuffle(goals.data, goals.count, sizeof(Entry));

        f64 score = score_board(goals);
        if (score < best_score)
        {
            best_score = score;
            MemoryCopy(best_list.data, goals.data, sizeof(Entry)*goals.count);
        }
    }

    // NOTE(nick): output results
    print("Iterations Run: %d\n", iterations);
    print("Best Score: %.2f\n", best_score);

    if (best_score == MAX_SCORE)
    {
        print("No solutions found!\n");
        return 1;
    }

    Array_f64 scores = score_board_runs(best_list);
    f64 mean = compute_mean(scores.data, scores.count);
    print("Difficulty: %.2f\n", mean);

    print("---\n");
    print_board_to_json(best_list);
    print("\n");
    print("---\n");

    return 0;
}