#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_PATH 1024

void press_enter_to_exit();
char* extract_name_from_path(const char* path);
int create_directory_recursive(const char* path);
int create_directory(const char* path);
char* get_file_extension(const char* filename);
int copy_file(const char* src_path, const char* dest_path);
bool is_directory(const char* path);
char* get_basename(const char* path);
char* get_parent_directory(const char* path);
char* replace_extension(const char* filename, const char* new_extension);
int remove_directory_recursive(const char* path);

#endif // UTILS_H_INCLUDED