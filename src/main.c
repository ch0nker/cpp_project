#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* USAGE = "Usage:\n\tcpp_project <project_name> [flags]\n\nFlags:\n\t-h, --help\t\t\t: Outputs this message.\n\t-n, --name=<name>\t\t: Sets the project binary's name.\n\t-d, --description=<desc>\t: Sets the project's description\n\t-v, --version=<ver>\t\t: Sets the project version.\n\t-s, --shared\t\t\t: Makes the project a shared library.";
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

void handle_flag(Flags* flags, char* raw_flag, size_t flag_size) {
    size_t value_size = 0;

    char* value = NULL;
    char* flag = malloc(flag_size);

    if(flag == NULL) {
        perror("Memory allocation failed\n");
        return;
    }

    strcpy(flag, raw_flag);

    get_flag(&flag, &flag_size, &value, &value_size);

    if(value != NULL) {
        if (check_flag(flag, "n", "name")) {
            if(flags->name && flags->name != flags->project_name)
                free(flags->name);

            flags->name = value;
        } else if (check_flag(flag, "d", "description")) {
            if(flags->description)
                free(flags->description);

            flags->description = value;
        } else if (check_flag(flag, "v", "version")) {
            if(flags->version)
                free(flags->version);

            flags->version = value;
        } else {
            free(value);
        }
    } else {
        if(check_flag(flag, "h", "help")) {
            printf("%s\n", USAGE);
            free(flag);
            exit(EXIT_SUCCESS);
        } else if(check_flag(flag, "s", "shared")) {
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

void cleanup(Flags* flags, char* project_directory, char* source_directory, char* include_directory, char* main_path, char* cmakelists_path) {
    free_flags(flags);

    if(project_directory != NULL)
        free(project_directory);

    if(source_directory != NULL)
        free(source_directory);

    if(include_directory != NULL)
        free(include_directory);

    if(main_path != NULL)
        free(main_path);

    if(cmakelists_path != NULL)
        free(cmakelists_path);
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
            return EXIT_FAILURE;
        }

        flags.name = flags.project_name;
        flags.version = strdup("1.0.0");
        if(flags.version == NULL) {
            free(flags.project_name);
            perror("Failed to allocate memory.\n");
            return EXIT_FAILURE;
        }

        if(flags.name[0] == '-') {
            handle_flag(&flags, flags.project_name, strlen(flags.project_name) + 1);
            free_flags(&flags);
            return EXIT_FAILURE;
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
                    cleanup(&flags, project_directory, NULL, NULL, NULL, NULL);

                    return EXIT_FAILURE;
                }

                char* include_directory = join_paths(project_directory, "include/");
                if(stat(include_directory, &st) == -1) {
                    if(mkdir(include_directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                        perror("Failed to create include directory.\n");
                        cleanup(&flags, project_directory, NULL, include_directory, NULL, NULL);

                        return EXIT_FAILURE;
                    }
                }

                char* source_directory = join_paths(project_directory, "src/");
                if(stat(source_directory, &st) == -1) {
                    if(mkdir(source_directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                        perror("Failed to create source directory.\n");
                        cleanup(&flags, project_directory, source_directory, include_directory, NULL, NULL);

                        return EXIT_FAILURE;
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
                    cleanup(&flags, project_directory, source_directory, include_directory, NULL, cmakelists_path);
                    return EXIT_FAILURE;
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
                    cleanup(&flags, project_directory, source_directory, include_directory, main_path, cmakelists_path);

                    return EXIT_FAILURE;
                }

                fprintf(main_file, "#include <cstdio>\n\nint main(int argc, char* argv[]) {\n\tprintf(\"Hello, world!\\n\");\n}");
                fclose(main_file);

                cleanup(&flags, project_directory, source_directory, include_directory, main_path, cmakelists_path);
            } else {
                perror("Project directory already exists.\n");
                cleanup(&flags, project_directory, NULL, NULL, NULL, NULL);

                return EXIT_FAILURE;
            }
        } else {
            free_flags(&flags);
            perror("Couldn't get cwd.\n");
            return EXIT_FAILURE;
        }
    } else {
        printf("%s\n", USAGE);
        return EXIT_SUCCESS;
    }
}