#include "base.h"


FILE *LOGG_F_USE_ONLY_FOR_LOGG = NULL; // ИСПОЛЬЗОВАТЬ ТОЛЬКО ПО НАЗНАЧЕНИЮ


void universal_swap(const void* restrict ptr_a, const void* restrict ptr_b, size_t elem_sz) {
    HARD_ASSERT(ptr_a != NULL, NULL_ERROR);
    HARD_ASSERT(ptr_b != NULL, NULL_ERROR);

    uint64_t buf[8] = {0};
    size_t buf_sz = sizeof(buf);
    char* a = (char*)ptr_a;
    char* b = (char*)ptr_b;

    while (elem_sz > 0) {
        buf_sz = MIN(buf_sz, elem_sz);

        memcpy(buf, b, buf_sz);
        memcpy(b, a, buf_sz);
        memcpy(a, buf, buf_sz);

        a += buf_sz;
        b += buf_sz;
        elem_sz -= buf_sz;
    }
}

void print_line(FILE* out_stream) {
    HARD_ASSERT(out_stream != NULL, NULL_ERROR);

    for (int i = 0; i < lEN_OF_PRINT_LINE; i++)
        fprintf(out_stream, "=");
    
    fprintf(out_stream, "\n");
}

void print_line_color(FILE* out_stream, const char* color) {
    HARD_ASSERT(out_stream != NULL, NULL_ERROR);
    HARD_ASSERT(color != NULL, NULL_ERROR);

    fprintf(out_stream, color);
    print_line(out_stream);
    fprintf(out_stream, "%s", RESET_COLOR_TEXT);
}

size_t get_file_size_by_identifier(const int file) {
    struct stat stat_file = {0};
    fstat(file, &stat_file);
    
    return stat_file.st_size;
}

size_t get_file_size_by_file_name(const char* file_name) {
    struct stat stat_file = {0};
    stat(file_name, &stat_file);
    
    return stat_file.st_size;
}



size_t count_symbolsin_text(const char* text, const char targ_ch) {
    HARD_ASSERT(text != NULL, NULL_ERROR);

    size_t cnt_targ_ch = 0;

    for (;*text != '\0'; text++)
        if (*text == targ_ch)
            cnt_targ_ch++;
    
    return cnt_targ_ch;
}

void read_file_in_one_buf(const int file_in, struct Text* text) {
    ALOGG;
    HARD_VERIFY(file_in != -1, CANT_OPEN_FILE);
    HARD_ASSERT(text != NULL, NULL_ERROR);

    text->data_sz = get_file_size_by_identifier(file_in);
    text->data = (char*)calloc(text->data_sz + 1, sizeof(char));
    HARD_VERIFY(text->data != NULL, NULL_MEMORY_ERR);

    // realloc for \r
    text->data_sz = read(file_in, text->data, text->data_sz);
    HARD_VERIFY(text->data_sz >= 0, CANT_OPEN_FILE);
    text->data = (char*)realloc(text->data, (text->data_sz + 1) * sizeof(char));
    HARD_VERIFY(text->data != NULL, NULL_MEMORY_ERR);

    text->data[text->data_sz] = '\0'; // add symbol end of string
}

void pars_text_to_lines(struct Text* text) {
    ALOGG;
    HARD_ASSERT(text != NULL, NULL_ERROR);

    size_t sz_data_ind = count_symbolsin_text(text->data, '\n') + 1;
    text->data_ind = (struct String*)calloc(sz_data_ind, sizeof(struct String));
    HARD_VERIFY(text->data_ind != NULL, NULL_MEMORY_ERR);

    size_t n_of_lines = 0;

    //==================================== Парсинг текста ====================================//
    char* last_ind = text->data;
    for (size_t i = 0; i <= text->data_sz; i++) {
        if (text->data[i] == '\n' || text->data[i] == '\0') {
            text->data_ind[n_of_lines].str = last_ind;
            text->data_ind[n_of_lines].len = (size_t)(&text->data[i] - last_ind);
            last_ind = text->data + i + 1;
            n_of_lines += 1;
            text->data[i] = '\0';
        }
    }

    text->n_lines = n_of_lines;
}

void read_file_by_lines_one_buf(struct Text* text, const char* file_name) {
    ALOGG;
    HARD_ASSERT(file_name != NULL, NULL_ERROR);
    HARD_ASSERT(text != NULL, NULL_ERROR);

    int file_in = open(file_name, O_RDONLY);
    HARD_VERIFY(file_in != -1, CANT_OPEN_FILE);

    size_t sz_text = 0;
    read_file_in_one_buf(file_in, text);

    pars_text_to_lines(text);

    close(file_in);
}

void print_text(FILE* file_out, struct Text* text) {
    ALOGG;
    HARD_ASSERT(file_out != NULL, NULL_ERROR);
    HARD_ASSERT(text != NULL, NULL_ERROR);

    for (size_t i = 0; i < text->data_sz - 1; i++) {
        if (text->data[i] == '\0') {
            text->data[i] = '\n';
        }
    }

    fprintf(file_out, "%s\n", text->data);

    for (size_t i = 0; i < text->data_sz - 1; i++) {
        if (text->data[i] == '\n') {
            text->data[i] = '\0';
        }
    }

    print_line(file_out);
}

void free_text(struct Text* text) {
    HARD_ASSERT(text != NULL, NULL_ERROR);
    
    free(text->data);
    text->data = NULL;
    
    free(text->data_ind);
    text->data_ind = NULL;
}