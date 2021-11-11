#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gmap.h"
#include "entry.h"
#include "string_key.h"

typedef struct _match
{
    char *p1_id;
    char *p2_id;
} match;

typedef struct _player
{
    int *distribution;
    double score;
    double wins;
    double battles;
} player;

#define BUFFER_SIZE 1000
#define MAX_ID_LEN 31
gmap *player_map;


player *player_create(int *dist)
{
    player *result = malloc(sizeof(player));
    result->distribution = dist;
    result->score = 0;
    result->wins = 0;
    result->battles = 0;

    return result;
}


void player_destroy(player *pl)
{
    if (pl != NULL)
    {
        free(pl->distribution);
        pl->distribution = NULL;
        free(pl);
        pl = NULL;
    }
}


int player_comp_wins(const void *player_1, const void *player_2)
{
    player *pl_1 = gmap_get(player_map, *(char **)player_1);
    player *pl_2 = gmap_get(player_map, *(char **)player_2);
    // printf("%s\n", (char *)player_1);
    // printf("%s\n", (char *)player_2);

    if (pl_1 == NULL || pl_2 == NULL)
    {
        fprintf(stderr, "Blotto: invalid comparsion of players");
        return 0;
    }

    if (pl_1->wins / pl_1->battles - pl_2->wins / pl_2->battles > 0)
    {
        return -1;
    }
    else if (pl_1->wins / pl_1->battles - pl_2->wins / pl_2->battles < 0)
    {
        return 1;
    }
    else
    {
        if (strcmp((char *)player_1, (char *)player_2) < 0)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
}


int player_comp_scores(const void *player_1, const void *player_2)
{
    player *pl_1 = gmap_get(player_map, *(char **)player_1);
    player *pl_2 = gmap_get(player_map, *(char **)player_2);
    // printf("%s\n", (char *)player_1);
    // printf("%s\n", (char *)player_2);

    if (pl_1 == NULL || pl_2 == NULL)
    {
        fprintf(stderr, "Blotto: invalid comparsion of players");
        return 0;
    }

    if (pl_1->score / pl_1->battles - pl_2->score / pl_2->battles > 0)
    {
        return -1;
    }
    else if (pl_1->score / pl_1->battles - pl_2->score / pl_2->battles < 0)
    {
        return 1;
    }
    else
    {
        if (strcmp((char *)player_1, (char *)player_2) < 0)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
}


int main(int argc, char *argv[])
{
    // Error checking
    if (argc < 5)
    {
        fprintf(stderr, "Blotto: missing argument(s)\n");
        return 0;
    }

    FILE *matchups_file = fopen(argv[1], "r");

    if (matchups_file == NULL)
    {
        fprintf(stderr, "Blotto: could not open file(s)\n");
        return 0;
    }

    int num_battles = argc - 3;
    int battle_values[num_battles];
    for (int i = 0; i < num_battles; i++)
    {
        battle_values[i] = atoi(argv[i + 3]);
    }


    // Creating map and reading over file to add entries to hash table
    player_map = gmap_create(duplicate, compare_keys, hash29, free);
    entry new_entry = entry_read(stdin, MAX_ID_LEN, num_battles);
    int num_entries = 0;

    do
    {
        if (new_entry.id == NULL || new_entry.distribution == NULL)
        {
            fprintf(stderr, "Blotto: invlaid entry");
            return 0;
        }

        player *new_player = player_create(new_entry.distribution);
        // printf("%s\n", new_entry.id);
        gmap_put(player_map, new_entry.id, new_player);
        num_entries++;

        new_entry = entry_read(stdin, MAX_ID_LEN, num_battles);
    } while (new_entry.id[0] != '\0');


    // Reading matchups
    char str[BUFFER_SIZE];
    int num_matches = 0;

    while (fgets(str, BUFFER_SIZE, matchups_file))
    {
        num_matches++;
    }

    match *matches = malloc(sizeof(match) * num_matches);
    char *p1_str = malloc(sizeof(p1_str));
    char *p2_str = malloc(sizeof(p2_str));

    rewind(matchups_file);
    int scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    for (int i = 0; i < num_matches; i++)
    {
        char *p1_id = malloc(sizeof(char *));
        strcpy(p1_id, p1_str);
        char *p2_id = malloc(sizeof(char *));
        strcpy(p2_id, p2_str);

        match new_match = {p1_id, p2_id};
        matches[i] = new_match;

        scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    }

    free(p1_str);
    free(p2_str);
    fclose(matchups_file);


    // Playing Blotto matches
    for (int i = 0; i < num_matches; i++)
    {
        const void *p1_key = matches[i].p1_id;
        const void *p2_key = matches[i].p2_id;
        // printf("%s\n", (char *)p1_key);
        // printf("%s\n", (char *)p2_key);
        player *player1 = gmap_get(player_map, p1_key);
        player *player2 = gmap_get(player_map, p2_key);

        if (player1 == NULL || player2 == NULL)
        {
            fprintf(stderr, "Blotto: player not found");
            return 0;
        }

        double p1_score = 0;
        double p2_score = 0;
        for (int battle = 0; battle < num_battles; battle++)
        {
            if (player1->distribution[battle] > player2->distribution[battle])
            {
                p1_score += battle_values[battle];
            }
            else if (player1->distribution[battle] < player2->distribution[battle])
            {
                p2_score += battle_values[battle];
            }
            else
            {
                p1_score += (double)battle_values[battle] / 2;
                p2_score += (double)battle_values[battle] / 2;
            }
        }
        player1->score += p1_score;
        player2->score += p2_score;

        if (p1_score > p2_score)
        {
            player1->wins++;
        }
        else if (p2_score > p1_score)
        {
            player2->wins++;
        }

        player1->battles++;
        player2->battles++;
    }


    // Retreive player keys, sort, and print out results
    const void **player_keys = gmap_keys(player_map);

    printf("keys:\n");
    for (int i = 0; i < num_entries; i++)
    {
        printf("%s\n", (char *)player_keys[i]);
    }

    if (strcmp(argv[2], "win") == 0)
    {
        qsort(player_keys, num_entries, sizeof(*player_keys), player_comp_wins);
        for (int key = 0; key < num_entries; key++)
        {
            player *pl = gmap_get(player_map, player_keys[key]);
            printf("%.3lf %s\n", pl->wins / pl->battles, (char *)player_keys[key]);
        }
    }
    else if (strcmp(argv[2], "score") == 0)
    {
        qsort(player_keys, num_entries, sizeof(*player_keys), player_comp_scores);
        for (int key = 0; key < num_entries; key++)
        {
            player *pl = gmap_get(player_map, player_keys[key]);
            printf("%.3lf %s\n", pl->score / pl->battles, (char *)player_keys[key]);
        }
    }
    else
    {
        fprintf(stderr, "Blotto: invalid argument (not win or score)\n");
        return 0;
    }
}