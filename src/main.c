#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* USAGE = "Usage:\n\tcpp_project <project_name> [flags]\n\nFlags:\n\t-h, --help\t\t: Outputs this message.\n\t-n, --name=<name>\t\t: Sets the project binary's name.\n\t-d, --description=<description>\t: Sets the project's description\n\t-v, --version=<version>\t\t: Sets the project version.\n\t-s, --shared\t\t: Makes the project a shared library.";
const int REQUIRED_PARAMS = 1;

typedef struct {
    char* project_name;
    char* name;
    char* version;
    char* description;
    int8_t shared;
} Flags;

char* get_value(char* flag, size_t* size, size_t* value_size) {
    char* equals_pos = strchr(flag, '=');
    if (!equals_pos || equals_pos == flag || *(equals_pos + 1) == '\0') {
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


void get_flag(char* flag, size_t* flag_size, char** value, size_t* value_size) {
    for(int j = 0; j < 2; j++) {
        if(flag[0] != '-')
            break;

        void* new_ptr = realloc(flag, --*flag_size);
        if(new_ptr == NULL) {
            free(flag);
            perror("Memory allocation failed\n");
            return;
        }

        flag = (char*) new_ptr;

        memmove(flag, flag + 1, *flag_size);
    }

    *value = get_value(flag, flag_size, value_size);
}

void handle_flag(Flags* flags, char* raw_flag, size_t flag_size) {
    size_t value_size = 0;

    char* value = NULL;
    char* flag = (char*) calloc(flag_size, sizeof(char));

    if(flag == NULL) {
        perror("Memory allocation failed\n");
        return;
    }

    strcpy(flag, raw_flag);
    get_flag(flag, &flag_size, &value, &value_size);

    if(value != NULL) {
        if (
            strcmp(flag, "name") == 0 ||
            strcmp(flag, "n") == 0
        ) {
            if(flags->name && flags->name != flags->project_name)
                free(flags->name);

            flags->name = value;
        } else if (
            strcmp(flag, "description") == 0 ||
            strcmp(flag, "d") == 0
        ) {
            if(flags->description)
                free(flags->description);

            flags->description = value;
        } else if (
            strcmp(flag, "version") == 0 ||
            strcmp(flag, "v") == 0
        ) {
            if(flags->version)
                free(flags->version);

            flags->version = value;
        } else {
            free(value);
        }
    } else {
        if(
            strcmp(flag, "help") == 0 ||
            strcmp(flag, "h") == 0
        ) {
            printf("%s\n", USAGE);
            free(flag);
            exit(0);
        } else if(
            strcmp(flag, "shared") == 0 ||
            strcmp(flag, "s") == 0
        ) {
            flags->shared = 1;
        }
    }

    free(flag);
} 

void free_flags(const Flags* flags) {
    if(flags->name != NULL && flags->project_name != flags->name)
        free(flags->name);

    free(flags->project_name);

    if(flags->version)
        free(flags->version);

    if(flags->description)
        free(flags->description);
}

char* join_paths(const char* path_1, const char* path_2) {
    size_t len = strlen(path_1) + strlen(path_2) + 2;
    char* joined_path = malloc(len);

    if (joined_path == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    snprintf(joined_path, len, "%s/%s", path_1, path_2);

    return joined_path;
}

int main(int argc, char* argv[]) {
    if(argc > 1) {
        Flags flags = { .project_name = NULL, .name = NULL, .version = NULL, .description = NULL, .shared = 0 };

        flags.project_name = strdup(argv[1]);
        if(flags.project_name == NULL) {
            perror("Failed to allocate memory.\n");
            return 1;
        }

        flags.name = flags.project_name;
        flags.version = strdup("1.0.0");
        if(flags.version == NULL) {
            free(flags.project_name);
            perror("Failed to allocate memory.\n");
            return 1;
        }

        if(flags.name[0] == '-') {
            handle_flag(&flags, flags.project_name, strlen(flags.project_name) + 1);
            free_flags(&flags);
            return 0;
        } 

        printf("Project Information:\n");

        if(argc > REQUIRED_PARAMS + 1) {
            for(int i = REQUIRED_PARAMS + 1; i < argc; i++) {
                handle_flag(&flags, argv[i], strlen(argv[i]) + 1);
            }
        }


        printf("\tName: %s\n", flags.name);
        printf("\tVerison: %s\n", flags.version);
        if(flags.description != NULL) {
            printf("\tDescription: %s\n", flags.description);
        }
        printf("\tShared: %i\n", flags.shared);
        

        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            char* project_directory = join_paths(cwd, flags.project_name);
            struct stat st = {0};

            printf("\tDirectory: %s\n", project_directory);

            if(stat(project_directory, &st) == -1) {
                if(mkdir(project_directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                    perror("Failed to create project directory.\n");
                    free_flags(&flags);
                    free(project_directory);
                    return 1;
                }

                char* include_directory = join_paths(project_directory, "include/");
                if(stat(include_directory, &st) == -1) {
                    if(mkdir(include_directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                        perror("Failed to create include directory.\n");
                        free_flags(&flags);
                        free(project_directory);
                        free(include_directory);
                        return 1;
                    }
                }

                char* source_directory = join_paths(project_directory, "src/");
                if(stat(source_directory, &st) == -1) {
                    if(mkdir(source_directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                        perror("Failed to create source directory.\n");
                        free_flags(&flags);
                        free(project_directory);
                        free(include_directory);
                        free(source_directory);
                        return 1;
                    }
                }

                if(flags.description == NULL) {
                    flags.description = strdup("");
                }

                FILE* cmakelists;
                char* cmakelists_path = join_paths(project_directory, "CMakeLists.txt");

                cmakelists = fopen(cmakelists_path, "w");
                if (cmakelists == NULL) {
                    perror("Failed to open CMakeLists.txt for writing.\n");
                    free_flags(&flags);
                    free(source_directory);
                    free(include_directory);
                    free(cmakelists_path);
                    free(project_directory);
                    return 1;
                }
                if(flags.shared == 0) {
                    fprintf(cmakelists, "cmake_minimum_required(VERSION 3.10)\n\nproject(%s\n\t\tVERSION %s\n\t\tDESCRIPTION \"%s\"\n\t\tLANGUAGES CXX)\n\nset(CMAKE_CXX_STANDARD 17)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\n\ninclude_directories(include)\n\nfile(GLOB_RECURSE SOURCE_FILES \"src/*.cpp\" \"src/*.c\")\nadd_executable(%s ${SOURCE_FILES})", flags.project_name, flags.version, flags.description, flags.name);
                } else {
                    fprintf(cmakelists, "cmake_minimum_required(VERSION 3.10)\n\nproject(%s\n\t\tVERSION %s\n\t\tDESCRIPTION \"%s\"\n\t\tLANGUAGES CXX)\n\nset(CMAKE_CXX_STANDARD 17)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\n\ninclude_directories(include)\n\nfile(GLOB_RECURSE SOURCE_FILES \"src/*.cpp\" \"src/*.c\")\nadd_library(%s SHARED ${SOURCE_FILES})", flags.project_name, flags.version, flags.description, flags.name);
                }
                fclose(cmakelists);

                FILE* main_file;
                char* main_path = join_paths(source_directory, "main.cpp");

                main_file = fopen(main_path, "w");

                if (main_file == NULL) {
                    perror("Failed to open src/main.cpp for writing.\n");
                    free_flags(&flags);
                    free(main_file);
                    free(source_directory);
                    free(include_directory);
                    free(cmakelists_path);
                    free(project_directory);

                    return 1;
                }

                fprintf(main_file, "#include <cstdio>\n\nint main(int argc, char* argv[]) {\n\tprintf(\"Hello, world!\\n\");\n}");
                fclose(main_file);

                free_flags(&flags);

                free(project_directory);
                free(source_directory);
                free(include_directory);
                free(main_path);
                free(cmakelists_path);
            } else {
                perror("Project directory already exists.\n");
                free_flags(&flags);
                free(project_directory);
                return 1;
            }
        } else {
            free_flags(&flags);
            perror("Couldn't get cwd.\n");
            return 1;
        }
    } else {
        printf("%s\n", USAGE);
    }
}