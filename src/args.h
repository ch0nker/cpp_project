typedef struct {
    char* key;
    void (*callback)(void*);
} MapItem;

typedef struct {
    unsigned long long size;
    MapItem items[];
} Map;

Map* create_map();
MapItem* get_item(Map* map, const char* key);
void free_map(Map* map);
MapItem* add_item(Map* map_ptr, const char* key, void(*callback)(char*));
int handle_args(int argc, char** argv, Map* map, const char* usage, int required_params);
char* copy_value(char* value);