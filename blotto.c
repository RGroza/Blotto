#include <stdio.h>
#include <stdlib.h>
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
} player;

#define BUFFER_SIZE 1000


player *player_create(int *dist)
{
    player *result = malloc(sizeof(player));
    result->distribution = dist;
    result->score = 0;
    result->wins = 0;

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


int main(int argc, char *argv[])
{
    // Error checking
    if (argc < 5)
    {
        fprintf(stderr, "Blotto: missing argument(s)\n");
        return 0;
    }

    FILE *matchups_file = fopen(argv[1], "r");
    FILE *entries_file = fopen(argv[argc - 1], "r");

    if (matchups_file == NULL || entries_file == NULL)
    {
        fprintf(stderr, "Blotto: could not open file(s)\n");
        return 0;
    }

    int num_battles = argc - 2;
    int battle_values[num_battles];
    for (int i = 1; i < argc - 1; i++)
    {
        battle_values[i] = atoi(argv[i]);
    }

    // Reading over file to determine total number of entries and checking number of battlefields in each entry
    char str[BUFFER_SIZE];
    char *ch;

    int num_entries = 0;
    int curr_battle = 0;

    while (fgets(str, BUFFER_SIZE, entries_file))
    {
        for (ch = str; *ch != '\0'; ch++)
        {
            if (*ch == ',')
            {
                curr_battle++;
            }
        }
        num_entries++;

        if (num_entries > 0)
        {
            if (curr_battle != num_battles)
            {
                return 0; // Mismatch in number of battlefields, exit
            }
        }
        else
        {
            num_battles = curr_battle;
        }
    }


    // Creating map and reading over file to add entries to hash table
    gmap *player_map = gmap_create(duplicate, compare_keys, hash29, free);
    int id_len = 0;

    while (fgets(str, BUFFER_SIZE, entries_file))
    {
        for (ch = str; *ch != ','; ch++)
        {
            id_len++;
        }

        entry new_entry = entry_read(entries_file, id_len, num_battles);
        player *new_player = player_create(new_entry.distribution);
        gmap_put(player_map, new_entry.id, new_player);

        id_len = 0;
    }


    // Reading matchups
    int num_matches = 0;

    while (fgets(str, BUFFER_SIZE, matchups_file))
    {
        num_matches++;
    }

    match *matches = malloc(sizeof(match) * num_matches);
    char *p1_str = malloc(sizeof(p1_str));
    char *p2_str = malloc(sizeof(p2_str));

    int scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    for (int i = 0; scan != EOF; i++)
    {
        match new_match = {p1_str, p2_str};
        matches[i] = new_match;
        scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    }


    // Playing Blotto matches
    for (int i = 0; i < num_matches; i++)
    {
        player *player1 = gmap_get(player_map, matches[i].p1_id);
        player *player2 = gmap_get(player_map, matches[i].p2_id);

        for (int battle = 0; battle < num_battles; battle++)
        {
            
        }
    }
}