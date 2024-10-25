#include "audio_converter.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>

uint64_t extract_hca_key(const char* folder) {
	char hcakey_path[MAX_PATH];
	snprintf(hcakey_path, sizeof(hcakey_path), "%s/.hcakey", folder);
	FILE* file = fopen(hcakey_path, "rb");
	if (!file) {
		perror("Error opening .hcakey file");
		return 0;
	}

	uint64_t key = 0;
	uint8_t byte;
	// Read bytes in reverse order
	for (int i = 7; i >= 0; i--) {
		if (fread(&byte, 1, 1, file) != 1) {
			perror("Error reading from .hcakey file");
			fclose(file);
			return 0;
		}
		key |= ((uint64_t)byte << (i * 8));
	}
	fclose(file);
	return key;
}

int process_wav_files(const char* folder, uint64_t hca_key) {
    DIR* dir = opendir(folder);
    if (!dir) {
        perror("Error opening directory");
        return -1;
    }

    // Create a batch file for all conversions
    char batch_path[MAX_PATH];
    snprintf(batch_path, sizeof(batch_path), "%s\\convert_wav.bat", folder);
    FILE* batch_file = fopen(batch_path, "w");
    if (!batch_file) {
        perror("Error creating batch file");
        closedir(dir);
        return -1;
    }

    // Write batch file header
    fprintf(batch_file, "@echo off\n");
    fprintf(batch_file, "echo Starting WAV to HCA conversion...\n");

    struct dirent* entry;
    char wav_path[MAX_PATH];
    char hca_path[MAX_PATH];
    int has_files = 0;

    while ((entry = readdir(dir)) != NULL) {
        const char* ext = get_file_extension(entry->d_name);
        if (ext != NULL && strcasecmp(ext, "wav") == 0) {
            has_files = 1;
            // Construct full paths
            snprintf(wav_path, sizeof(wav_path), "%s\\%s", folder, entry->d_name);
            char* basename = get_basename(entry->d_name);
            snprintf(hca_path, sizeof(hca_path), "%s\\%s.hca", folder, basename);

            // Write conversion command to batch file
            fprintf(batch_file, "echo Converting %s.wav to HCA\n", basename);
            fprintf(batch_file, "\"%s\" \"%s\" \"%s\" --keycode %" PRIu64 " --out-format hca\n",
                    vgaudio_cli_path, wav_path, hca_path, hca_key);
        }
    }

    // Add completion message and auto-exit
    fprintf(batch_file, "echo Conversion complete!\n");
    fprintf(batch_file, "del \"%s\\convert_wav.bat\"\n", folder);
    fprintf(batch_file, "exit\n");
    fclose(batch_file);
    closedir(dir);

    if (!has_files) {
        remove(batch_path);
        return 0;
    }

    // Execute the batch file in a new window and wait for completion
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), "start \"\" /wait cmd /C \"%s\"", batch_path);
    return system(command);
}

int process_hca_files(const char* folder) {
    DIR* dir = opendir(folder);
    if (!dir) {
        perror("Error opening directory");
        return -1;
    }

    // Create a batch file for all conversions
    char batch_path[MAX_PATH];
    snprintf(batch_path, sizeof(batch_path), "%s\\convert_hca.bat", folder);
    FILE* batch_file = fopen(batch_path, "w");
    if (!batch_file) {
        perror("Error creating batch file");
        closedir(dir);
        return -1;
    }

    // Write batch file header
    fprintf(batch_file, "@echo off\n");
    fprintf(batch_file, "echo Starting HCA to WAV conversion...\n");

    struct dirent* entry;
    char hca_path[MAX_PATH];
    char wav_path[MAX_PATH];
    int has_files = 0;

    while ((entry = readdir(dir)) != NULL) {
        const char* ext = get_file_extension(entry->d_name);
        if (ext != NULL && strcasecmp(ext, "hca") == 0 && strcmp(entry->d_name, "00000.hca") != 0) {
            has_files = 1;
            // Construct full paths
            snprintf(hca_path, sizeof(hca_path), "%s\\%s", folder, entry->d_name);
            char* basename = get_basename(entry->d_name);
            snprintf(wav_path, sizeof(wav_path), "%s\\%s.wav", folder, basename);

            // Write conversion and deletion logic to batch file
            fprintf(batch_file, "echo Converting %s.hca to WAV\n", basename);
            fprintf(batch_file, "\"%s\" -i \"%s\" -o \"%s\"\n", vgmstream_path, hca_path, wav_path);

            // Check if conversion was successful and delete HCA if it was
            fprintf(batch_file, "if %%errorlevel%% equ 0 (\n");
            fprintf(batch_file, "    echo Conversion successful - deleting %s.hca\n", basename);
            fprintf(batch_file, "    del \"%s\"\n", hca_path);
            fprintf(batch_file, ") else (\n");
            fprintf(batch_file, "    echo Conversion failed for %s.hca - keeping original file\n", basename);
            fprintf(batch_file, ")\n");
        }
    }

    // Add completion message and cleanup
    fprintf(batch_file, "echo Conversion complete!\n");
    fprintf(batch_file, "del \"%s\\convert_hca.bat\" && exit\n", folder);
    fclose(batch_file);
    closedir(dir);

    if (!has_files) {
        remove(batch_path);
        return 0;
    }

    // Execute the batch file in a new window
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), "start \"HCA to WAV Conversion\" \"%s\"", batch_path);
    system(command);

    return 0;
}
