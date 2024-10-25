#include "file_processor.h"
#include "uasset_extractor.h"
#include "hcakey_generator.h"
#include "uasset_injector.h"
#include "file_extractor.h"
#include "file_packer.h"
#include "utils.h"
#include <string.h>

int process_input(const char* input) {
	if (is_directory(input)) {
		return process_directory(input);
	}

	char* ext = get_file_extension(input);
	if (ext == NULL) {
		printf("Invalid file: %s\n", extract_name_from_path(input));
		return 1;
	}
	if (strstr(input, "bgm_main")) {
		printf("BGM isn't supported by this tool.\n");
		return 0;
	}
	if (strstr(input, "se_battle") || strstr(input, "se_ui")
	        || strstr(input, "se_ADVIF")  ) {
		printf("Warning: This tool may not handle this file correctly.\n\n");
	}

	if (strcmp(ext, "acb") == 0) {
		return process_acb_file(input);
	} else if (strcmp(ext, "uasset") == 0) {
		return process_uasset_file(input);
	} else if (strcmp(ext, "awb") == 0) {
		return process_awb_file(input);
	} else {
		printf("Unsupported file type: %s\n", ext);
		return 0;
	}
}

int process_directory(const char* dir_path) {
	if (!check_pair_exists(dir_path, "acb")) {
		if (check_pair_exists(dir_path, "uasset")) {
			return handle_uasset_directory(dir_path);
		}
		printf("Warning: No .acb or .uasset pair found for %s\n",
		       extract_name_from_path(dir_path));
		return 1;
	}
	return pack_files(dir_path);
}

int handle_uasset_directory(const char* dir_path) {
	char* parent_dir = get_parent_directory(dir_path);
	char* folder_name = extract_name_from_path(dir_path);
	char uasset_path[MAX_PATH];
	snprintf(uasset_path, MAX_PATH, "%s/%s.uasset", parent_dir, folder_name);
	process_uasset(uasset_path);
	free(parent_dir);
	free(folder_name);
	return pack_files(dir_path);
}

int process_acb_file(const char* file_path) {
	if (!check_pair_exists(file_path, "awb")) {
		printf("Error: No .awb pair found for %s\n",
		       extract_name_from_path(file_path));
		return 1;
	}
	generate_hcakey(file_path);
	return extract_and_process(file_path);
}

int process_uasset_file(const char* file_path) {
	if (!check_pair_exists(file_path, "awb")) {
		printf("Error: No .awb pair found for %s\n",
		       extract_name_from_path(file_path));
		return 1;
	}

	if (!check_pair_exists(file_path, "acb")) {
		process_uasset(file_path);
	}

	generate_hcakey(file_path);
	return extract_and_process(file_path);
}

int process_awb_file(const char* file_path) {
	if (check_pair_exists(file_path, "acb")) {
		generate_hcakey(file_path);
		return extract_and_process(file_path);
	} else if (check_pair_exists(file_path, "uasset")) {
		char* uasset_path = replace_extension(file_path, "uasset");
		process_uasset(uasset_path);
		generate_hcakey(file_path);
		return extract_and_process(file_path);
	}
	printf("Warning: No .acb or .uasset pair found for %s\n",
	       extract_name_from_path(file_path));
	return 1;
}

int check_pair_exists(const char* path, const char* extension) {
	char pair_filename[MAX_PATH];

	// Get the parent directory and base name
	char* parent_dir = get_parent_directory(path);
	const char* filename = get_basename(path);

	// Construct the path to the expected file
	snprintf(pair_filename, MAX_PATH, "%s\\%s.%s", parent_dir, filename,
	         extension);

	FILE* file = fopen(pair_filename, "r");
	if (file) {
		fclose(file);
		free(parent_dir);
		return 1;
	}

	free(parent_dir);
	return 0;
}