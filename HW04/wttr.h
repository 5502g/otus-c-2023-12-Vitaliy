#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

const uint8_t ASCII_SIZE = 128;
const char wttr1[] = "https://wttr.in/";
const char wttr2[] = "?format=j1";

/*структура для данных с wttr.in*/
typedef struct {
    char *data;
    size_t size;
} wttr_data_t;

/* операция:    вывод на экран данных из json
 * предусловия: data - строка данных
 * постусловия: выводит на экран определённые поля из json
 *              с помощью библиотеки json-c;
 *              ничего не возвращает */
void print_json(char* data);

/*функция обратного вызова для get_data_wttr() - стандартный пример из библиотеки curl*/
size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp);

/* операция:    получение данных с помощью libcurl
 * предусловия: str - url в виде строки
 * постусловия: возвращает указатель на структуру wttr_data_t
 *              с данными json */
wttr_data_t *get_data_wttr(char *str);

/* операция:    формирование строки адреса wttr
 * предусловия: str - название города в виде строки
 * постусловия: возвращает полный url для get_data_wttr() */
char *get_addr_wttr(const char *str);

/* операция:    определение корректности аргумента командной строки
 * предусловия: str - название города в виде строки
 * постусловия: возвращает true, если строка состоит из
 *              букв латинского или кириллического алфавита;
 *              иначе - false */
bool read_arg(const char *str);

/* операция:    чтение символа utf-8 из строки
 * предусловия: str - строка-источник символа
 *              symb_buf - буфер, в который функция поместит символ
 * постусловия: возвращает ширину символа или 0 при ошибке*/
size_t read_symbol_utf8(char *symb_buf, const char *str);

/* операция:    распознавание буквы или тире
 * предусловия: symb_buf - символ
 *              width - ширина символа
 * постусловия: возвращает true, если символ - буква
 *              латинницы или кириллицы или тире,
 *              иначе - false */
bool is_alpha_or_line(const char *symb_buf, size_t width);
