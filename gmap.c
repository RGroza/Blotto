#include "gmap.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define GMAP_INITIAL_CAPACITY 100
#define TABLESIZE_MULTIPLIER 2

typedef struct _gmap_node
{
    void *key;
    void *value;
    struct _gmap_node *next;
    void *(*cp)(const void *);
} gmap_node;

struct _gmap
{
    gmap_node **table;
    size_t size;
    size_t num_chains;

    size_t (*hash)(const void *);
    void *(*cp)(const void *);
    int (*comp)(const void *, const void *);
    void (*f)(void *);
};


gmap *gmap_create(void *(*cp)(const void *), int (*comp)(const void *, const void *), size_t (*h)(const void *s), void (*f)(void *));
size_t gmap_size(const gmap *m);
void *gmap_put(gmap *m, const void *key, void *value);
void *gmap_remove(gmap *m, const void *key);
bool gmap_contains_key(const gmap *m, const void *key);
void *gmap_get(gmap *m, const void *key);
void gmap_for_each(gmap *m, void (*f)(const void *, void *, void *), void *arg);
const void **gmap_keys(gmap *m);
void gmap_destroy(gmap *m);

size_t gmap_compute_index(const void *key, size_t (*hash)(const void *), size_t num_chains);
gmap_node **gmap_table_find_key(const gmap *m, const void *key);
void gmap_table_add(gmap_node **table, gmap_node *n, size_t (*hash)(const void *), size_t num_chains);
void gmap_embiggen(gmap *m, size_t n);


gmap *gmap_create(void *(*cp)(const void *), int (*comp)(const void *, const void *), size_t (*h)(const void *s), void (*f)(void *))
{
    gmap *result = malloc(sizeof(gmap));
    if (result != NULL)
    {
        result->size = 0;
        result->cp = cp;
        result->comp = comp;
        result->hash = h;
        result->f = f;
        result->table = malloc(sizeof(gmap_node *) * GMAP_INITIAL_CAPACITY);
        result->num_chains = (result->table != NULL ? GMAP_INITIAL_CAPACITY : 0);
        for (size_t i = 0; i < result->num_chains; i++)
	    {
	        result->table[i] = NULL;
	    }
    }
    return result;
}


size_t gmap_size(const gmap *m)
{  
    if (m == NULL)
    {
        return 0;
    }

    return m->size;
}


size_t gmap_compute_index(const void *key, size_t (*hash)(const void *), size_t num_chains)
{
    return hash(key) % num_chains;
}


gmap_node **gmap_table_find_key(const gmap *m, const void *key)
{
    size_t i = gmap_compute_index(key, m->hash, m->num_chains);
    gmap_node *curr = m->table[i];
    gmap_node *prev = curr;

    while (curr != NULL)
    {
        if (curr->key != NULL && m->comp(curr->key, key) == 0)
        {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    gmap_node **nodes = malloc(sizeof(gmap_node *) * 2);
    nodes[0] = prev;
    nodes[1] = curr;

    return nodes;
}


void gmap_table_add(gmap_node **table, gmap_node *n, size_t (*hash)(const void *), size_t num_chains)
{
    size_t i = gmap_compute_index(n->key, hash, num_chains);

    n->next = table[i];
    table[i] = n;
}


void gmap_embiggen(gmap *m, size_t n)
{
    gmap_node **old_table;
    size_t old_size;
    gmap_node *curr_node;
    gmap_node *next_node;

    old_table = m->table;
    old_size = m->size;

    m->size *= TABLESIZE_MULTIPLIER;
    m->table = malloc(sizeof(*(m->table)) * m->size);
    if (m->table == NULL)
    {
        m->table = old_table;
        m->size = old_size;
        return;
    }

    for (int i = 0; i < m->size; i++)
    {
        m->table[i] = NULL;
    }

    for (int i = 0; i < old_size; i++)
    {
        for (curr_node = old_table[i]; curr_node !=NULL; curr_node = next_node)
        {
            next_node = curr_node->next;
            gmap_table_add(old_table, curr_node, m->hash, m->num_chains);
        }
    }

    free(old_table);
}


bool gmap_contains_key(const gmap *m, const void *key)
{
    if (m == NULL || key == NULL)
    {
        return false;
    }

    gmap_node **targets = gmap_table_find_key(m, key);
    gmap_node *curr = targets[1];
    targets[0] = NULL;
    targets[1] = NULL;
    free(targets);

    return curr != NULL;
}


void *gmap_get(gmap *m, const void *key)
{
    if (m == NULL || key == NULL)
    {
        return NULL;
    }

    gmap_node *n = gmap_table_find_key(m, key)[1];
    if (n != NULL)
    {
        return n->value;
    }
    else
    {
        return NULL;
    }
}


void *gmap_put(gmap *m, const void *key, void *value)
{
    if (m == NULL || key == NULL)
    {
        return false;
    }

    gmap_node *n = gmap_table_find_key(m, key)[1];
    if (n != NULL)
    {
        void *old_value = n->value;
        n->value = value;
        return old_value;
    }
    else
    {
        void *copy = malloc(sizeof(key));

        if (copy != NULL)
        {
            copy = m->cp(key);

            if (m->size >= m->num_chains)
            {
                gmap_embiggen(m, m->num_chains * 2);
            }

            gmap_node *n = malloc(sizeof(gmap_node));
            if (n != NULL)
            {
                n->key = copy;
                n->value = value;
                gmap_table_add(m->table, n, m->hash, m->num_chains);
                m->size++;
                return NULL;
            }
            else
            {
                free(copy);
                return "error";
            }
        }
        else
        {
            return "error";
        }
    }
}


void gmap_for_each(gmap *m, void (*f)(const void *, void *, void *), void *arg)
{
    if (m == NULL || f == NULL)
    {
        return;
    }

    for (size_t chain = 0; chain < m->num_chains; chain++)
    {
        gmap_node *curr = m->table[chain];
        while (curr != NULL)
	    {
	        f(curr->key, curr->value, arg);
	        curr = curr->next;
	    }
    }
}


void gmap_destroy(gmap *m)
{
    if (m == NULL)
    {
        return;
    }

    for (size_t i = 0; i < m->num_chains; i++)
    {
        gmap_node *curr = m->table[i];
        while (curr != NULL)
        {
            m->f(curr->key);

            gmap_node *next = curr->next;

            free(curr);

            curr = next;
        }
    }

    free(m->table);
    free(m);
}


void *gmap_remove(gmap *m, const void *key)
{
    gmap_node **targets = gmap_table_find_key(m, key);
    if (targets[1] == NULL)
    {
        return NULL;
    }

    void *val = targets[1]->value;
    gmap_node *next = targets[1]->next;
    gmap_node *prev = targets[0];
    free(targets[1]);

    targets[0]->next = next;
    m->size--;

    targets[0] = NULL;
    targets[1] = NULL;
    free(targets);

    return val;
}


const void **gmap_keys(gmap *m)
{
    return 0;
}