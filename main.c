#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(void)
{   kernel filt;
	image ph;

	char *cmd = (char *)malloc(BASIC_SIZE * sizeof(char));
	char *copy_of_cmd = (char *)malloc(BASIC_SIZE * sizeof(char));

	initialize_filters(&filt);
	initialize_parameters(&ph);
	while (1) {
		char *operation;
		fgets(cmd, BASIC_SIZE, stdin);

		if (strcmp(cmd, "\n") == 0)
			continue;

		int len = strlen(cmd) - 1;
		verify_cmd(&len, &cmd, &operation, &copy_of_cmd);

		if (strcmp(operation, "LOAD") == 0) {
			char *tmp = alloc_char(&cmd, len, copy_of_cmd);
			select_all(&ph, 0);
			load_image(&ph, tmp);

			free(tmp);
		} else if (strcmp(operation, "SELECT") == 0) {
			char *tmp = alloc_char(&cmd, len, copy_of_cmd);

			if (strcmp(copy_of_cmd, "SELECT ALL"))
				select_pixels(&ph, tmp);
			else
				select_all(&ph, 1);

			free(tmp);
		} else if (strcmp(operation, "ROTATE") == 0) {
			char *tmp = alloc_char(&cmd, len, copy_of_cmd);
			rotate_image(&ph, tmp);

			free(tmp);
		} else if (strcmp(cmd, "CROP") == 0) {
			if (ph.selection_counter) {
				crop(&ph);
			} else {
				if (ph.is_loaded == 0)
					printf("No image loaded\n");
				else
					printf("Image cropped\n");
			}
		} else if (strcmp(operation, "SAVE") == 0) {
			char *tmp = alloc_char(&cmd, len, copy_of_cmd);
			save_file(&ph, tmp);

			free(tmp);
		} else if (strcmp(operation, "EXIT") == 0) {
			close_program(&ph, &filt);
			free(cmd);
			free(copy_of_cmd);

			if (ph.is_loaded == 0)
				printf("No image loaded\n");

			break;
		} else if (strcmp(operation, "APPLY") == 0) {
			char *tmp = alloc_char(&cmd, len, copy_of_cmd);
			int apply_len = strlen(cmd);

			if (ph.is_loaded) {
				if (apply_len > 5)
					apply_filt(&ph, tmp, filt);
				else
					printf("Invalid command\n");
			} else {
				printf("No image loaded\n");
			}
			free(tmp);
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}
