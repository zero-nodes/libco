#ifndef MAP_H
#define MAP_H

typedef struct map_t map_t;

extern map_t* create_map();
extern void* map_get(map_t *map, const char *key);
extern int map_set(map_t *map, const char *key, void *val);
extern void free_map(map_t *map);

#endif
