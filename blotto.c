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


    // Reading over file to determine total number of entries and checking number of battlefields in each entry
    char *entry_line;
    char *ch;
    size_t len = 0;

    int num_entries = 0;
    int num_battlefields = 0;
    int curr_battlefields = 0;

    while (getline(&entry_line, &len, entries_file) != -1)
    {
        for (ch = entry_line; *ch != '\0'; ch++)
        {
            if (*ch == ',')
            {
                curr_battlefields++;
            }
        }
        num_entries++;

        if (num_entries > 0)
        {
            if (curr_battlefields != num_battlefields)
            {
                return 0; // Mismatch in number of battlefields, exit
            }
        }
        else
        {
            num_battlefields = curr_battlefields;
        }
    }


    // Creating map and reading over file to add entries to hash table
    gmap *map = gmap_create(duplicate, compare_keys, hash29, free);

    *entry_line = NULL;
    *ch = NULL;
    size_t len = 0;
    int id_len = 0;

    while (getline(&entry_line, &len, entries_file) != -1)
    {
        for (ch = entry_line; *ch != ','; ch++)
        {
            id_len++;
        }

        entry new_entry = entry_read(entries, id_len, num_battlefields);
        gmap_put(map, new_entry.id, new_entry.distribution);

        id_len = 0;
    }


    // Reading matchups
    *entry_line = NULL;
    *ch = NULL;
    size_t len = 0;
    int num_matchups = 0;

    while (getline(&entry_line, &len, entries_file) != -1)
    {
        num_matchups++;
    }

    match *match_results = malloc(sizeof(match) * num_matchups);
    char *p1_str = malloc(sizeof(p1_str));
    char *p2_str = malloc(sizeof(p2_str));

    int scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    for (int curr_match; scan != EOF; curr_match++)
    {
        match new_match = {p1_str, p2_str}
        match_results[curr_match] = new_match;
        scan = fscanf(matchups_file, "%s %s\n", p1_str, p2_str);
    }


    // Playing Blotto matches
    // for (int curr_match)
}