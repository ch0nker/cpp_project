#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    char* key;
    void (*callback)(char*);
} MapItem;

typedef struct {
    unsigned long long size;
    MapItem* items;
} Map;

Map* create_map() {
    Map* map = malloc(sizeof(Map) + sizeof(MapItem));
    if (!map) {
        perror("Failed to allocate memory.\n");
        return NULL;
    }

    map->size = 0;
    map->items = NULL;

    return map;
}

MapItem* add_item(Map* map, const char* key, void(*callback)(char*)) {
    map->items = realloc(map->items, sizeof(MapItem) * (map->size + 1));
    if (!map->items) {
        perror("Failed to reallocate memory for items.\n");
        return NULL;
    }

    map->items[map->size].key = strdup(key);
    if (!map->items[map->size].key) {
        perror("Failed to allocate memory for key.\n");
        return NULL;
    }

    map->items[map->size].callback = callback;
    map->size++;

    return &map->items[map->size - 1];
}

MapItem* get_item(Map* map, const char* key) {
    for (unsigned long long i = 0; i < map->size; i++) {
        if (strcmp(key, map->items[i].key) == 0) {
            return &map->items[i];
        }
    }

    return NULL;
}

void free_map(Map* map) {
    if (!map)
        return;

    for (unsigned long long i = 0; i < map->size; ++i) {
        free(map->items[i].key);
    }

    free(map->items);
    free(map);
}

char* get_value(char* flag, size_t* size, size_t* value_size) {
    char* equals_pos = strchr(flag, '=');
    if (!equals_pos || equals_pos == flag || equals_pos[0] == '\0') {
        return NULL;
    }

    *value_size = *size - (equals_pos - flag + 1);
    char* value = strndup(equals_pos + 1, *value_size);
    if (!value) {
        perror("Failed to allocate memory.\n");
        return NULL;
    }

    *size = equals_pos - flag;
    *equals_pos = '\0';
    return value;
}

void get_flag(char** flag, size_t* flag_size, char** value, size_t* value_size) {
    int count = 0;

    while(count < *flag_size && (*flag)[count] == '-')
        count++;

    *flag_size -= count;

    char* new_ptr = realloc(*flag, *flag_size + 1);
    if(new_ptr == NULL) {
        perror("Memory allocation failed.\n");
        return;
    }

    *flag = new_ptr; // this is meaningless since shrinking the memory keeps it at the same address
                    //  but I'll keep it for how realloc should usually be written.

    memmove(*flag, *flag + count, *flag_size); 
    (*flag)[*flag_size] = '\0';

    *value = get_value(*flag, flag_size, value_size);
}

int check_flag(char* flag, const char* short_hand, const char* full) {
    return (
        strcmp(flag, short_hand) == 0 ||
        strcmp(flag, full) == 0
    );
}

void help_function(char* usage) {
    printf("%s", usage);
}

char* copy_value(char* value) {
    size_t value_size = strlen(value) + 1;

    char* new_value = malloc(value_size);

    memmove(new_value, value, value_size);

    return new_value;
}

void handle_flag(Map* map, char* raw_flag, size_t flag_size, char* usage) {
    size_t value_size = 0;

    char* value = NULL;
    char* flag = malloc(flag_size);

    if(flag == NULL) {
        perror("Memory allocation failed\n");
        return;
    }

    strcpy(flag, raw_flag);

    get_flag(&flag, &flag_size, &value, &value_size);

    int flag_len = strlen(flag);
    for(int i = 0; i < map->size; i++) {
        MapItem item = map->items[i];
        int result = strncmp(flag, item.key, flag_len);

        if(result == 0) {
            if(strcmp(item.key, "help") == 0) {
                if(value)
                    free(value);

                value = usage;
            }

            item.callback(value);

            break;
        }
    }

    if(value != usage && value != NULL)
        free(value);

    free(flag);
}

int handle_args(int argc, char** argv, Map* map, char* usage, int required_params) {
    MapItem* help_item = get_item(map, "help");

    if(help_item == NULL) {
        add_item(map, "help", help_function);
    }

    if(argc > 1) {
        if(argv[1][0] == '-') {
            handle_flag(map, argv[1], strlen(argv[1]) + 1, usage);
            exit(EXIT_SUCCESS);
            return 1;
        }

        if(argc > required_params) {
            for(int i = required_params; i < argc; i++) {
                handle_flag(map, argv[i], strlen(argv[i]) + 1, usage);
            }
        }
        return 1;
    } else {
        printf("%s", usage);
        return 0;
    }
}