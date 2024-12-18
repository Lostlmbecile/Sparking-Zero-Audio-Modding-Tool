#pragma once
#ifndef PAK_GENERATOR_H
#define PAK_GENERATOR_H
#include "config.h"
#include "utils.h"

extern char unrealpak_path[];
extern char unrealpak_path_no_compression[];
extern char program_directory[];
extern Config config;

// Generate PAK and folder structure for the given file
int pak_generate(const char* file_path, const char* mod_name);

// Create directory structure and copy files
int pak_create_structure(const char* file_path, const char* mod_name);

// Generate pak file and clean up
int pak_package_and_cleanup(const char* mod_name);

#endif // PAK_GENERATOR_H
