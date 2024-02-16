#include "wttr.h"

int main(int argc, char **argv) 
{
    char *addr_wttr;
    wttr_data_t *data_wttr;

    /*проверка числа аргументов*/
    if (argc != 2) {
        fprintf(stderr, "Использование: %s название_города\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    /*проверка корректности ввода названия города*/
    if (!read_arg(argv[1])) {
        fprintf(stderr, "Некорректный ввод названия города.\n"
                "Допускаются символы латинницы, кириллицы и \"-\"\n");
        exit(EXIT_SUCCESS);
    }
    /*проверка на ошибку выделения памяти при формировании строки адреса*/
    if ((addr_wttr = get_addr_wttr(argv[1])) == NULL)
        exit(EXIT_FAILURE);
    /*получение данных от wttr.in*/
    if ((data_wttr = get_data_wttr(addr_wttr)) == NULL) {
        free(addr_wttr);
        exit(EXIT_FAILURE);
    }

    print_json(data_wttr->data);

    free(addr_wttr);
    free(data_wttr);

    return 0;
}

/*вывод данных с помощью json-c*/
void print_json(char* data) 
{
    json_object *root;
    json_object *temp;
    size_t size_arr;

    root = json_tokener_parse(data);

    /*место*/
    json_object_object_get_ex(root, "nearest_area", &temp);
    size_arr = json_object_array_length(temp);
    if (size_arr) {
        temp = json_object_array_get_idx(temp, 0);
        json_object_object_get_ex(temp, "areaName", &temp);
        size_arr = json_object_array_length(temp);
        if (size_arr) {
            temp = json_object_array_get_idx(temp, 0);
            json_object_object_get_ex(temp, "value", &temp);
            printf("%s:\n", json_object_get_string(temp));
        }
    }

    json_object_object_get_ex(root, "current_condition", &temp);
    size_arr = json_object_array_length(temp);
    if (size_arr) {
        /*текстовое описание погоды*/
        json_object *current_condition = json_object_array_get_idx(temp, 0);
        json_object_object_get_ex(current_condition, "weatherDesc", &temp);
        size_arr = json_object_array_length(temp);
        if (size_arr) {
            temp = json_object_array_get_idx(temp, 0);
            json_object_object_get_ex(temp, "value", &temp);
            printf("    %s\n", json_object_get_string(temp));
        }
        /*направление ветра*/
        json_object_object_get_ex(current_condition, "winddir16Point", &temp);
        printf("    направление ветра: %s\n", json_object_get_string(temp));
        /*скорость ветра*/
        json_object_object_get_ex(current_condition, "windspeedKmph", &temp);
        printf("    скорость ветра: %sкм/ч\n", json_object_get_string(temp));
        /*температура*/
        json_object_object_get_ex(current_condition, "temp_C", &temp);
        printf("    температура: %sC\n", json_object_get_string(temp));
    }
}

/*функция обратного вызова для get_data_wttr() - стандартный пример из библиотеки curl*/
size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    size_t realsize = size * nmemb;
    wttr_data_t *mem = (wttr_data_t *)userp;
    char *ptr = (char *)realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 0;
    }
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';

    return realsize;
}

/*получение данных с помощью libcurl*/
wttr_data_t *get_data_wttr(char *str) 
{
    CURL *curl_handle;

    /*выделение памяти для типа wttr_data_t и инициализация его полей*/
    wttr_data_t *chunk = (wttr_data_t *)malloc(sizeof(wttr_data_t));
    if (chunk == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для wttr_data_t");
        return NULL;
    }
    if ((chunk->data = (char *)malloc(1)) == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для данных\n");
        free(chunk);
        return NULL;
    }
    chunk->size = 0;

    /*загрузка данных в память - стандартный пример из библиотеки curl*/
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        fprintf(stderr, "Что-то пошло не так в функции curl_global_init()\n");
        free(chunk->data);
        free(chunk);
        return NULL;
    }
    if ((curl_handle = curl_easy_init()) == NULL) {
        fprintf(stderr, "Что-то пошло не так в функции curl_easy_init()\n");
        free(chunk->data);
        free(chunk);
        return NULL;
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, str);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    if ((curl_easy_perform(curl_handle)) != CURLE_OK) {
        fprintf(stderr, "Что-то пошло не так в функции curl_easy_perform()\n");
        free(chunk->data);
        free(chunk);
        return NULL;
    }
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return chunk;
}

/*формирование строки адреса wttr*/
char *get_addr_wttr(const char *str) 
{
    /*выделение памяти под первую часть URL*/
    char *addr_wttr = (char *)malloc(sizeof(char) * (strlen(wttr1) + 1));
    if (addr_wttr == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для строки\n");
        return NULL;
    }
    strncpy(addr_wttr, wttr1, strlen(wttr1) + 1);
    /*добавление к URL названия города*/
    char *addr_wttr_tmp = (char *)realloc(addr_wttr, sizeof(char) * (strlen(addr_wttr) + strlen(str) + 1));
    if (addr_wttr_tmp == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для строки\n");
        free(addr_wttr);
        return NULL;
    }
    addr_wttr = addr_wttr_tmp;
    strcat(addr_wttr, str);
    /*добавление к URL формата вывода данных в виде json*/
    addr_wttr_tmp = (char *)realloc(addr_wttr, sizeof(char) * (strlen(addr_wttr) + strlen(wttr2) + 1));
    if (addr_wttr_tmp == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для строки\n");
        free(addr_wttr);
        return NULL;
    }
    addr_wttr = addr_wttr_tmp;
    strcat(addr_wttr, wttr2);
    return addr_wttr;
}

/*определение корректности аргумента командной строки*/
bool read_arg(const char *str) 
{
    char symb_buf[5];

    /*чтение и проверка символов*/
    for (size_t width; *str != '\0'; str += width) {
        width = read_symbol_utf8(symb_buf, str);
        if (!width) {
            fprintf(stderr, "Ошибка чтения символа аргумента командной строки\n");
            return false;
        }
        if (!is_alpha_or_line(symb_buf, width))
            return false;
    }
    return true;
}

/*чтение символа utf-8 из строки*/
size_t read_symbol_utf8(char *symb_buf, const char *str) 
{
    size_t width = 1;

    symb_buf[0] = *str;
    /*если символ однобайтный*/
    if ((uint8_t)*str < ASCII_SIZE) {
        symb_buf[1] = '\0';
        return width;
    /*иначе, определение количества байтов символа*/
    } else {
        /*подсчёт байтов*/
        for (uint8_t bit = 0x40; (*str & bit) || (width == 5); bit /= 2)
            width++;
        /*проверка на ошибку чтения символа*/
        if (width == 5)
            return 0;
        /*чтение остальных байтов*/
        for (size_t index = 1; index < width; index++) {
            symb_buf[index] = *(++str);
        }
        symb_buf[width] = '\0';
    }
    return width;
}

/*распознавание буквы или тире*/
bool is_alpha_or_line(const char *symb, size_t width) 
{
    uint16_t symb_tmp;

    /*если символ однобайтный (латинница или "-")*/
    if (width == 1) {
        if (((symb[0] >= 0x41) && (symb[0] <= 0x5a))
                || ((symb[0] >= 0x61) && (symb[0] <= 0x7a))
                || (symb[0] == 0x2d))
            return true;
    /*если двухбайтный (кириллица)*/
    } else if (width == 2) {
        symb_tmp = (uint8_t)symb[0];
        symb_tmp <<= 8;
        symb_tmp |= (uint8_t)symb[1];
        if (symb_tmp >= 0xd090 && symb_tmp <= 0xd18f)
            return true;
    } else {
        return false;
    }
    return false;
}
