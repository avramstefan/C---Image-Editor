#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASIC_SIZE 100
#define S1 sizeof(double)
#define S2 sizeof(double *)

typedef struct kernel {
	int *edge;
	int *blur;
	int *gauss;
	int *sharpen;
} kernel;

typedef struct image {
	char *magic_nr;
	int is_loaded;
	int is_all;
	int is_rgb;
	int is_first;
	int first_selection;
	int selection_counter;
	int *coord;
	int columns;
	int rows;
	int intensity;
	double **img;
	double **mat_r;
	double **mat_g;
	double **mat_b;
} image;

void initialize_parameters(image *ph)
{	ph->selection_counter = 0;
	ph->is_all = 0;
	ph->is_loaded = 0;
	ph->is_first = 0;
	ph->is_rgb = 0;
	ph->first_selection = 0;
}

void initialize_filters(kernel *filt)
{   filt->edge = (int *)malloc(9 * sizeof(int));
	filt->blur = (int *)malloc(9 * sizeof(int));
	filt->gauss = (int *)malloc(9 * sizeof(int));
	filt->sharpen = (int *)malloc(9 * sizeof(int));

	filt->edge[0] = -1;
	filt->edge[1] = -1;
	filt->edge[2] = -1;
	filt->edge[3] = -1;
	filt->edge[4] = 8;
	filt->edge[5] = -1;
	filt->edge[6] = -1;
	filt->edge[7] = -1;
	filt->edge[8] = -1;

	filt->sharpen[0] = 0;
	filt->sharpen[1] = -1;
	filt->sharpen[2] = 0;
	filt->sharpen[3] = -1;
	filt->sharpen[4] = 5;
	filt->sharpen[5] = -1;
	filt->sharpen[6] = 0;
	filt->sharpen[7] = -1;
	filt->sharpen[8] = 0;

	filt->blur[0] = 1;
	filt->blur[1] = 1;
	filt->blur[2] = 1;
	filt->blur[3] = 1;
	filt->blur[4] = 1;
	filt->blur[5] = 1;
	filt->blur[6] = 1;
	filt->blur[7] = 1;
	filt->blur[8] = 1;

	filt->gauss[0] = 1;
	filt->gauss[1] = 2;
	filt->gauss[2] = 1;
	filt->gauss[3] = 2;
	filt->gauss[4] = 4;
	filt->gauss[5] = 2;
	filt->gauss[6] = 1;
	filt->gauss[7] = 2;
	filt->gauss[8] = 1;
}

void loading_process(image **ph, int type1, int type2, int pos, char *file)
{   FILE * f;
	if (type1)
		f = fopen(file, "rt");
	else
		f = fopen(file, "rb");
	fseek(f, pos + 1, SEEK_SET);
	if (type2) {
		(*ph)->img = (double **)malloc((*ph)->rows * S2);
		for (int i = 0; i < (*ph)->rows; i++) {
			(*ph)->img[i] = (double *)malloc((*ph)->columns * S1);

			for (int j = 0; j < (*ph)->columns; j++) {
				unsigned char tmp_ch;
				if (type1)
					fscanf(f, "%hhu", &tmp_ch);
				else
					fread(&tmp_ch, sizeof(unsigned char), 1, f);

				int tmp = (int)tmp_ch;

				if (tmp > 255)
					tmp = tmp % 255 - 1;
				if (tmp < 0)
					tmp = 255 + tmp % 255 + 1;
				(*ph)->img[i][j] = (double)tmp;
			}
		}
		(*ph)->is_rgb = 0;
	} else {
		(*ph)->mat_r = (double **)malloc((*ph)->rows * S2);
		(*ph)->mat_g = (double **)malloc((*ph)->rows * S2);
		(*ph)->mat_b = (double **)malloc((*ph)->rows * S2);
		for (int i = 0; i < (*ph)->rows; i++) {
			(*ph)->mat_r[i] = (double *)malloc((*ph)->columns * S1);
			(*ph)->mat_g[i] = (double *)malloc((*ph)->columns * S1);
			(*ph)->mat_b[i] = (double *)malloc((*ph)->columns * S1);

			for (int j = 0; j < (*ph)->columns; j++) {
				unsigned char tmp1_ch, tmp2_ch, tmp3_ch;
				if (type1) {
					fscanf(f, "%hhu %hhu %hhu", &tmp1_ch, &tmp2_ch, &tmp3_ch);
				} else {
					fread(&tmp1_ch, sizeof(unsigned char), 1, f);
					fread(&tmp2_ch, sizeof(unsigned char), 1, f);
					fread(&tmp3_ch, sizeof(unsigned char), 1, f);
				}

				int tmp1 = (int)tmp1_ch;
				int tmp2 = (int)tmp2_ch;
				int tmp3 = (int)tmp3_ch;

				if (tmp1 > 255)
					tmp1 = tmp1 % 255 - 1;
				if (tmp2 > 255)
					tmp2 = tmp2 % 255 - 1;
				if (tmp3 > 255)
					tmp3 = tmp3 % 255 - 1;

				if (tmp1 < 0)
					tmp1 = 255 + tmp1 % 255 + 1;
				if (tmp2 < 0)
					tmp2 = 255 + tmp2 % 255 + 1;
				if (tmp3 < 0)
					tmp3 = 255 + tmp3 % 255 + 1;

				(*ph)->mat_r[i][j] = (double)tmp1;
				(*ph)->mat_g[i][j] = (double)tmp2;
				(*ph)->mat_b[i][j] = (double)tmp3;
			}
		}
		(*ph)->is_rgb = 1;
	}
	(*ph)->is_all = 1;
	(*ph)->is_loaded = 1;
	fclose(f);
}

void reset(image **ph)
{   if ((*ph)->is_rgb) {
		for (int i = 0; i < (*ph)->rows; i++) {
			free((*ph)->mat_r[i]);
			free((*ph)->mat_g[i]);
			free((*ph)->mat_b[i]);
		}
		free((*ph)->mat_r);
		free((*ph)->mat_g);
		free((*ph)->mat_b);
	} else {
		for (int i = 0; i < (*ph)->rows; i++)
			free((*ph)->img[i]);
		free((*ph)->img);
	}

	if ((*ph)->magic_nr)
		free((*ph)->magic_nr);

	if ((*ph)->is_first)
		free((*ph)->coord);

	(*ph)->is_loaded = 0;
	(*ph)->is_all = 0;
	(*ph)->is_rgb = 0;
	(*ph)->selection_counter = 0;
	(*ph)->is_first = 0;
	(*ph)->first_selection = 0;
}

void load_image(image *ph, char *reading_file)
{   FILE * f = fopen(reading_file, "rt");
		if (ph->is_loaded || !f) {
			if (!f) {
				if (ph->is_loaded)
					reset(&*&ph);
				printf("Failed to load %s\n", reading_file);
				return;
			}
			if (strcmp(ph->magic_nr, "P2") == 0) {
				for (int i = 0; i < ph->rows; i++)
					free(ph->img[i]);
				free(ph->img);
				free(ph->magic_nr);
				//printf("Salut fane\n");
			} else if (strcmp(ph->magic_nr, "P3") == 0) {
				for (int i = 0; i < ph->rows; i++) {
					free(ph->mat_r[i]);
					free(ph->mat_g[i]);
					free(ph->mat_b[i]);
				}
				free(ph->magic_nr);
				free(ph->mat_r);
				free(ph->mat_g);
				free(ph->mat_b);
			} else if (strcmp(ph->magic_nr, "P5") == 0) {
				for (int i = 0; i < ph->rows; i++)
					free(ph->img[i]);
				free(ph->img);
				free(ph->magic_nr);
			} else if (strcmp(ph->magic_nr, "P6") == 0) {
				for (int i = 0; i < ph->rows; i++) {
					free(ph->mat_r[i]);
					free(ph->mat_g[i]);
					free(ph->mat_b[i]);
				}
				free(ph->magic_nr);
				free(ph->mat_r);
				free(ph->mat_g);
				free(ph->mat_b);
			}
		}

	ph->magic_nr = (char *)malloc(3 * sizeof(char));
	fgets(ph->magic_nr, 3, f);

	fscanf(f, "%d%d", &ph->columns, &ph->rows);
	fscanf(f, "\n%d", &ph->intensity);
	int pos = ftell(f);
	fclose(f);

	int ok_1 = strcmp(ph->magic_nr, "P2");
	int ok_2 = strcmp(ph->magic_nr, "P3");
	int ok_3 = strcmp(ph->magic_nr, "P5");
	int ok_4 = strcmp(ph->magic_nr, "P6");

	if (ok_1 == 0)
		loading_process(&*&ph, 1, 1, pos, reading_file);
	if (ok_2 == 0)
		loading_process(&*&ph, 1, 0, pos, reading_file);
	if (ok_3 == 0)
		loading_process(&*&ph, 0, 1, pos, reading_file);
	if (ok_4 == 0)
		loading_process(&*&ph, 0, 0, pos, reading_file);

	printf("Loaded %s\n", reading_file);
}

int verify_coordinates(char *coordinates)
{   char *p;
	int how_many = 0;

	p = strtok(coordinates, " ");
	while (p) {
		int is_number = 1;
		for (int i = 0; p[i] != '\0'; i++) {
			if (p[i] >= 'a' && p[i] <= 'z')
				is_number = 0;
		}
		if (is_number == 0)
			break;
		how_many++;
		p = strtok(NULL, " ");
	}

	if (how_many == 4)
		return 1;
	else
		return 0;
}

void verify_lens(image **ph)
{	if ((*ph)->coord[0] > (*ph)->coord[2]) {
		(*ph)->coord[0] += (*ph)->coord[2];
		(*ph)->coord[2] = (*ph)->coord[0] - (*ph)->coord[2];
		(*ph)->coord[0] = (*ph)->coord[0] - (*ph)->coord[2];
	}

	if ((*ph)->coord[1] > (*ph)->coord[3]) {
		(*ph)->coord[1] += (*ph)->coord[3];
		(*ph)->coord[3] = (*ph)->coord[1] - (*ph)->coord[3];
		(*ph)->coord[1] = (*ph)->coord[1] - (*ph)->coord[3];
	}
}

void verify_selection(image *ph, int *is_ok)
{	for (int i = 0; i < 4; i++) {
		if (i % 2 == 0) {
			if (ph->coord[i] > ph->columns || ph->coord[i] < 0)
				*is_ok = 0;
		} else {
			if (ph->coord[i] > ph->rows || ph->coord[i] < 0)
				*is_ok = 0;
		}
	}

	int x1 = ph->coord[0], x2 = ph->coord[2];
	int y1 = ph->coord[1], y2 = ph->coord[3];

	if ((x1 == x2 && x1 == y1 && x1 == y2))
		if ((x2 == y1 && x2 == y2 && y1 == y2))
			*is_ok = 0;

	if (x1 == ph->columns || y1 == ph->rows)
		*is_ok = 0;
}

void select_pixels(image *ph, char *given_img)
{   int index = 0;
	char *tmp;
	char *copy_of_coord = (char *)malloc(BASIC_SIZE * sizeof(char));
	strcpy(copy_of_coord, given_img);
	int is_ok = verify_coordinates(copy_of_coord);
	free(copy_of_coord);

	if (is_ok == 0) {
		printf("Invalid command\n");
		return;
	}

	if (ph->is_loaded == 0) {
		printf("No image loaded\n");
		return;
	}

	if (ph->first_selection == 0)
		ph->first_selection = 1;

	if (ph->selection_counter == 0 && ph->is_first == 0)
		ph->coord = (int *)malloc(4 * sizeof(int));

	if (ph->is_first == 0)
		ph->is_first = 1;

	int *old_coord = (int *)malloc(4 * sizeof(int));
	if (ph->selection_counter)
		for (int i = 0; i < 4; i++)
			old_coord[i] = ph->coord[i];

	tmp = strtok(given_img, " ");
	while (tmp) {
		ph->coord[index++] = atoi(tmp);
		tmp = strtok(NULL, " ");
	}

	verify_lens(&*&ph);
	verify_selection(ph, &is_ok);

	if (is_ok == 1) {
		printf("Selected %d %d ", ph->coord[0], ph->coord[1]);
		printf("%d %d\n", ph->coord[2], ph->coord[3]);
		int is_all_1 = 0, is_all_2 = 0;
		if (ph->coord[2] - ph->coord[0] == ph->columns)
			is_all_1 = 1;
		if (ph->coord[3] - ph->coord[1] == ph->rows)
			is_all_2 = 1;
		if (is_all_1 && is_all_2) {
			ph->selection_counter = 0;
			ph->is_all = 1;
		} else {
			ph->selection_counter = 1;
			ph->is_all = 0;
		}
	} else {
		printf("Invalid set of coordinates\n");
		if (ph->selection_counter)
			for (int i = 0; i < 4; i++)
				ph->coord[i] = old_coord[i];
	}

	free(old_coord);
}

void select_all(image *ph, int ok)
{   if (ok) {
		if (ph->is_loaded) {
			ph->is_all = 1;
			ph->selection_counter = 0;
			printf("Selected ALL\n");
		} else {
			printf("No image loaded\n");
		}
	} else {
		ph->is_all = 0;
	}
}

void rotate(double ***mat, int x1, int *x2, int y1, int *y2, int is_all)
{   int nr_rows = *y2 - y1;
	int nr_columns = *x2 - x1;

	if (nr_rows != nr_columns && is_all == 0) {
		printf("The selection must be square\n");
		return;
	}

	double *tmp;
	int min_1, min_2, max_1, max_2;

	if (is_all) {
		min_1 = x1;
		max_1 = *x2;
		min_2 = y1;
		max_2 = *y2;
	} else {
		min_1 = y1;
		max_1 = *y2;
		min_2 = x1;
		max_2 = *x2;
	}

	int nr_alloc = nr_rows * nr_columns;

	if (nr_rows < nr_columns) {
		tmp = (double *)calloc(nr_alloc, S1);
		*mat = (double **)realloc(*mat, nr_columns * S2);
		for (int i = nr_rows; i < nr_columns; i++)
			(*mat)[i] = (double *)malloc(nr_columns * S1);
	} else {
		tmp = (double *)calloc(nr_alloc, S1);
		if (is_all == 1 && nr_rows != nr_columns) {
			for (int i = 0; i < nr_rows; i++)
				(*mat)[i] = (double *)realloc((*mat)[i], nr_rows * S1);
		}
	}

	int index = 0;
	for (int i = *x2 - 1; i >= x1; i--)
		for (int j = y1; j < *y2; j++)
			tmp[index++] = (*mat)[j][i];

	index = 0;
	for (int i = min_1; i < max_1; i++)
		for (int j = min_2; j < max_2; j++)
			(*mat)[i][j] = tmp[index++];

	free(tmp);

	if (is_all == 1) {
		if (nr_rows > nr_columns) {
			for (int i = nr_columns; i < nr_rows; i++)
				free((*mat)[i]);
			(*mat) = (double **)realloc((*mat), nr_columns * S2);
		} else if (nr_rows < nr_columns) {
			for (int i = 0; i < nr_columns; i++)
				(*mat)[i] = (double *)realloc((*mat)[i], nr_rows * S1);
		}
	}
}

void rotate_image(image *ph, char *string_angle)
{   int angle = atoi(string_angle);

	if (ph->is_loaded == 0) {
		printf("No image loaded\n");
		return;
	}

	if (angle % 90 || angle > 360 || angle < -360) {
		printf("Unsupported rotation angle\n");
		return;
	}

	int copy_of_angle = angle;
	if (angle < 0)
		angle *= (-1);
	else
		angle = 360 - angle;

	int rotate_ok = 0;
	if (ph->selection_counter || ph->is_all)
		rotate_ok = 1;

	while (angle % 90 == 0 && angle != 0 && rotate_ok) {
		if (ph->is_all == 1) {
			int x = ph->columns;
			int y = ph->rows;
			if (ph->is_rgb) {
				rotate(&ph->mat_r, 0, &x, 0, &y, 1);
				rotate(&ph->mat_g, 0, &x, 0, &y, 1);
				rotate(&ph->mat_b, 0, &x, 0, &y, 1);
			} else {
				rotate(&ph->img, 0, &x, 0, &y, 1);
			}
			ph->rows += ph->columns;
			ph->columns = ph->rows - ph->columns;
			ph->rows = ph->rows - ph->columns;
		} else {
			int x1 = ph->coord[0];
			int x2 = ph->coord[2];
			int y1 = ph->coord[1];
			int y2 = ph->coord[3];
			if (ph->is_rgb) {
				rotate(&ph->mat_r, x1, &x2, y1, &y2, 0);
				rotate(&ph->mat_g, x1, &x2, y1, &y2, 0);
				rotate(&ph->mat_b, x1, &x2, y1, &y2, 0);
			} else {
				rotate(&ph->img, x1, &x2, y1, &y2, 0);
			}
		}
		angle -= 90;
	}

	printf("Rotated %d\n", copy_of_angle);
}

void equalize_coord(int *x1, int *x2, int *y1, int *y2, image *ph)
{	if (ph->is_all) {
		*x1 = 0;
		*y1 = 0;
		*x2 = ph->columns;
		*y2 = ph->rows;
	} else {
		*x1 = ph->coord[0];
		*y1 = ph->coord[1];
		*x2 = ph->coord[2];
		*y2 = ph->coord[3];
	}
}

void crop(image *ph)
{   if (ph->is_loaded == 0) {
		printf("No image loaded\n");
		return;
	}

	if (ph->selection_counter == 0)
		return;

	int x1, x2, y1, y2;
	equalize_coord(&x1, &x2, &y1, &y2, ph);

	int nr_columns = x2 - x1;
	int nr_rows = y2 - y1;
	double *tmp, *tmp_r, *tmp_g, *tmp_b;

	if (ph->is_rgb) {
		tmp_r = (double *)malloc(nr_rows * nr_columns * S1);
		tmp_g = (double *)malloc(nr_rows * nr_columns * S1);
		tmp_b = (double *)malloc(nr_rows * nr_columns * S1);
	} else {
		tmp = (double *)malloc(nr_rows * nr_columns * S1);
	}

	int index = 0;
	for (int i = y1; i < y2; i++)
		for (int j = x1; j < x2; j++) {
			if (ph->is_rgb) {
				tmp_r[index] = ph->mat_r[i][j];
				tmp_g[index] = ph->mat_g[i][j];
				tmp_b[index++] = ph->mat_b[i][j];
			} else {
				tmp[index++] = ph->img[i][j];
			}
		}

	for (int i = nr_rows; i < ph->rows; i++) {
		if (ph->is_rgb) {
			free(ph->mat_r[i]);
			free(ph->mat_g[i]);
			free(ph->mat_b[i]);
		} else {
			free(ph->img[i]);
		}
	}

	if (ph->is_rgb) {
		ph->mat_r = (double **)realloc(ph->mat_r, nr_rows * S2);
		ph->mat_g = (double **)realloc(ph->mat_g, nr_rows * S2);
		ph->mat_b = (double **)realloc(ph->mat_b, nr_rows * S2);
	} else {
		ph->img = (double **)realloc(ph->img, nr_rows * S2);
	}

	index = 0;
	for (int i = 0; i < nr_rows; i++)
		for (int j = 0; j < nr_columns; j++) {
			if (ph->is_rgb) {
				ph->mat_r[i][j] = tmp_r[index];
				ph->mat_g[i][j] = tmp_g[index];
				ph->mat_b[i][j] = tmp_b[index++];
			} else {
				ph->img[i][j] = tmp[index++];
			}
		}
	if (ph->is_rgb) {
		free(tmp_r);
		free(tmp_g);
		free(tmp_b);
	} else {
		free(tmp);
	}

	ph->rows = nr_rows;
	ph->columns = nr_columns;

	ph->selection_counter = 0;
	ph->is_all = 1;

	printf("Image cropped\n");
}

int round_fc(double x)
{   /*int tmp = (int)x;
	double tmp2 = (2 * (double)tmp + 1) / 2;
	if (tmp2 < 0)
		tmp2 -= 1;
	if (tmp2 < x) {
		return tmp;
	} else {
		if (x < 0)
			return tmp - 1;
		else
			return tmp + 1;
	}*/
	return (int)(x + 0.5);
}

double clamp(double x)
{   if (x < 0)
		return 0;
	if (x > 255)
		return 255;
	return x;
}

double filt_sum(double **mat, char *filt_name, int row, int col, kernel filt)
{   double sum = 0;
	int index = 0;
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			if (strcmp(filt_name, "EDGE") == 0)
				sum = sum + mat[i][j] * (double)filt.edge[index];
			else if (strcmp(filt_name, "SHARPEN") == 0)
				sum = sum + mat[i][j] * (double)filt.sharpen[index];
			else if (strcmp(filt_name, "BLUR") == 0)
				sum = sum + mat[i][j] * (double)filt.blur[index] / 9;
			else if (strcmp(filt_name, "GAUSSIAN_BLUR") == 0)
				sum = sum + mat[i][j] * (double)filt.gauss[index] / 16;
			index++;
		}
	}

	sum = clamp(sum);

	return sum;
}

int verify_parameter(char *parameter)
{   int ok = 1;

	char *filt_names = (char *)malloc(40 * sizeof(char));
	strcpy(filt_names, "EDGE SHARPEN BLUR GAUSSIAN_BLUR");

	if (!strstr(filt_names, parameter))
		ok = 0;

	free(filt_names);

	return ok;
}

void apply_filt(image *ph, char *filt_name, kernel filt)
{   int nr_rows, nr_columns;
	int x1, y1, x2, y2;

	if (ph->is_all) {
		nr_rows = ph->rows;
		nr_columns = ph->columns;
		equalize_coord(&x1, &x2, &y1, &y2, ph);
	} else {
		if (ph->selection_counter == 0)
			return;
		equalize_coord(&x1, &x2, &y1, &y2, ph);
		nr_rows = y2 - y1;
		nr_columns = x2 - x1;
	}

	if (verify_parameter(filt_name) == 0) {
		printf("APPLY parameter invalid\n");
		return;
	}

	if (strcmp(ph->magic_nr, "P2") == 0 || strcmp(ph->magic_nr, "P5") == 0) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	double *tmp, *tmp_r, *tmp_g, *tmp_b;
	if (ph->is_rgb) {
		tmp_r = (double *)calloc(nr_rows * nr_columns, S1);
		tmp_g = (double *)calloc(nr_rows * nr_columns, S1);
		tmp_b = (double *)calloc(nr_rows * nr_columns, S1);
	} else {
		tmp = (double *)calloc(nr_rows * nr_columns, S1);
	}

	int index = 0;
	for (int i = y1; i < y2; i++) {
		for (int j = x1; j < x2; j++) {
			if (ph->is_rgb) {
				if (i > 0 && i < ph->rows - 1 && j > 0 && j < ph->columns - 1) {
					tmp_r[index] = filt_sum(ph->mat_r, filt_name, i, j, filt);
					tmp_g[index] = filt_sum(ph->mat_g, filt_name, i, j, filt);
					tmp_b[index] = filt_sum(ph->mat_b, filt_name, i, j, filt);
					index++;
				} else {
					tmp_r[index] = ph->mat_r[i][j];
					tmp_g[index] = ph->mat_g[i][j];
					tmp_b[index++] = ph->mat_b[i][j];
				}
			} else {
				if (i > 0 && i < ph->rows - 1 && j > 0 && j < ph->columns - 1)
					tmp[index++] = filt_sum(ph->img, filt_name, i, j, filt);
				else
					tmp[index++] = ph->img[i][j];
			}
		}
	}

	index = 0;
	for (int i = y1; i < y2; i++) {
		for (int j = x1; j < x2; j++) {
			if (ph->is_rgb) {
				ph->mat_r[i][j] = tmp_r[index];
				ph->mat_g[i][j] = tmp_g[index];
				ph->mat_b[i][j] = tmp_b[index++];
			} else {
				ph->img[i][j] = tmp[index++];
			}
		}
	}

	if (ph->is_rgb) {
		free(tmp_r);
		free(tmp_g);
		free(tmp_b);
	} else {
		free(tmp);
	}

	printf("APPLY %s done\n", filt_name);
}

void save_file(image *ph, char *saving_file)
{   if (ph->is_loaded == 0) {
		printf("No image loaded\n");
		return;
	}

	FILE * f;
	int is_binary = 0;

	if (strstr(saving_file, "ascii")) {
		char *file_name = strtok(saving_file, " ");
		f = fopen(file_name, "wt");
		printf("Saved %s\n", file_name);
	} else {
		f = fopen(saving_file, "wb");
		is_binary = 1;
		printf("Saved %s\n", saving_file);
	}

	if (strcmp(ph->magic_nr, "P2") == 0 && is_binary)
		strcpy(ph->magic_nr, "P5");
	else if (strcmp(ph->magic_nr, "P5") == 0 && is_binary == 0)
		strcpy(ph->magic_nr, "P2");

	if (strcmp(ph->magic_nr, "P3") == 0 && is_binary)
		strcpy(ph->magic_nr, "P6");
	else if (strcmp(ph->magic_nr, "P6") == 0 && is_binary == 0)
		strcpy(ph->magic_nr, "P3");

	fprintf(f, "%s\n%d %d\n", ph->magic_nr, ph->columns, ph->rows);
	fprintf(f, "%d\n", ph->intensity);

	if (ph->is_rgb) {
		for (int i = 0; i < ph->rows; i++) {
			for (int j = 0; j < ph->columns; j++) {
				if (is_binary) {
					unsigned char tmp1, tmp2, tmp3;
					tmp1 = (unsigned char)(round_fc(ph->mat_r[i][j]));
					tmp2 = (unsigned char)(round_fc(ph->mat_g[i][j]));
					tmp3 = (unsigned char)(round_fc(ph->mat_b[i][j]));
					fwrite(&tmp1, sizeof(unsigned char), 1, f);
					fwrite(&tmp2, sizeof(unsigned char), 1, f);
					fwrite(&tmp3, sizeof(unsigned char), 1, f);
				} else {
					fprintf(f, "%d ", round_fc(ph->mat_r[i][j]));
					fprintf(f, "%d ", round_fc(ph->mat_g[i][j]));
					fprintf(f, "%d ", round_fc(ph->mat_b[i][j]));
				}
			}
			if (is_binary == 0)
				fprintf(f, "\n");
		}
	} else {
		for (int i = 0; i < ph->rows; i++) {
			for (int j = 0; j < ph->columns; j++) {
				if (is_binary) {
					int a = round_fc(ph->img[i][j]);
					unsigned char tmp = (unsigned char)a;
					fwrite(&tmp, sizeof(unsigned char), 1, f);
				} else {
					fprintf(f, "%d ", round_fc(ph->img[i][j]));
				}
			}
			if (is_binary == 0)
				fprintf(f, "\n");
		}
	}
	fclose(f);
}

void close_program(image *ph, kernel *filt)
{   if (ph->is_loaded) {
		if (ph->is_rgb) {
			for (int i = 0; i < ph->rows; i++) {
				free(ph->mat_r[i]);
				free(ph->mat_g[i]);
				free(ph->mat_b[i]);
			}
			free(ph->mat_r);
			free(ph->mat_g);
			free(ph->mat_b);
		} else {
			for (int i = 0; i < ph->rows; i++)
				free(ph->img[i]);
			free(ph->img);
		}
		free(ph->magic_nr);
		if (ph->first_selection)
			free(ph->coord);
	}

	free(filt->edge);
	free(filt->blur);
	free(filt->gauss);
	free(filt->sharpen);
}

void verify_cmd(int *len, char **cmd, char **operation, char **copy_of_cmd)
{	int tmp = *len;

	while (1) {
		if ((*cmd)[tmp] != ' ' && (*cmd)[tmp] != '\n' && (*cmd)[tmp] != '\t')
			break;
		tmp--;
	}

	*len = tmp;
	(*cmd)[*len + 1] = '\0';

	strcpy(*copy_of_cmd, *cmd);
	*operation = strtok(*cmd, " ");
	if (*operation)
		*len = strlen(*operation);
}

char *alloc_char(char **cmd, int len, char *copy_of_cmd)
{	strcpy(*cmd, copy_of_cmd);

	char *tmp = (char *)malloc(BASIC_SIZE * sizeof(char));
	strcpy(tmp, *cmd + len + 1);

	return tmp;
}
