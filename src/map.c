#include "map.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAP_CAPACITY 32

typedef struct map_entry {
    char *key;
    void *value;
    struct map_entry *next;
} map_entry;

struct map_t{
    size_t capacity;
    map_entry **buckets;
};

static unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 

    return hash;
}

map_t* create_map()
{
    map_t *m = malloc(sizeof(map_t));
    if (!m) return NULL;

    m->capacity = MAP_CAPACITY;
    m->buckets = calloc(m->capacity, sizeof(map_entry*));
    if (!m->buckets) {
        free(m);
        return NULL;
    }

    return m;
}

int map_set(map_t *map, const char *key, void *value)
{
    unsigned long h = hash(key) % map->capacity;

    map_entry *entry = map->buckets[h];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }

    map_entry *new_entry = malloc(sizeof(map_entry));
    if (!new_entry) return -1;

    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = map->buckets[h];

    map->buckets[h] = new_entry;
    return 0;
}

void* map_get(map_t *map, const char *key)
{
    unsigned long h = hash(key) % map->capacity;

    map_entry *entry = map->buckets[h];

    while (entry) {
        if (strcmp(entry->key, key) == 0)
            return entry->value;

        entry = entry->next;
    }

    return NULL;
}

void free_map(map_t *map)
{
    for (size_t i = 0; i < map->capacity; i++) {
        map_entry *entry = map->buckets[i];
        while (entry) {
            map_entry *tmp = entry;
            entry = entry->next;

            free(tmp->key);
            free(tmp);
        }
    }

    free(map->buckets);
    free(map);
}
