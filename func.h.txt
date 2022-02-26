#ifndef FUNCTIONS
#define FUNCTIONS

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

void initialize_parameters(image *ph);

void initialize_filters(kernel *filt);

void loading_process(image **ph, int type1, int type2, int pos, char *file);

void reset(image **ph);

void load_image(image *ph, char *reading_file);

int verify_coordinates(char *coordinates);

void verify_lens(image **ph);

void verify_selection(image *ph, int *is_ok);

void select_pixels(image *ph, char *given_img);

void select_all(image *ph, int ok);

void rotate(double ***mat, int x1, int *x2, int y1, int *y2, int is_all);

void rotate_image(image *ph, char *string_angle);

void equalize_coord(int *x1, int *x2, int *y1, int *y2, image *ph);

void crop(image *ph);

int round_fc(double x);

double clamp(double x);

double filt_sum(double **mat, char *filt_name, int row, int col, kernel filt);

int verify_parameter(char *parameter);

void apply_filt(image *ph, char *filt_name, kernel filt);

void save_file(image *ph, char *saving_file);

void close_program(image *ph, kernel *filt);

void verify_cmd(int *len, char **cmd, char **operation, char **copy_of_cmd);

char *alloc_char(char **cmd, int len, char *copy_of_cmd);

#endif
