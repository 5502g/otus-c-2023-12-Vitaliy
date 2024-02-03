#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "hash.h"

#define SIZE_BUF    30
#define ASCII_SIZE  128

/* операция:    чтение одного слова из файла
 * предусловия: fp - файл-источник
 *              str_buf - буфер для строки
 * постусловия: возвращает количество байтов в строке*/
static size_t read_word_utf8(char *str_buf, FILE *fp);

/* операция:    чтение символа (utf-8) из файла
 * предусловия: fp - файл-источник
 *              symb_buf - буфер для чтения из файла
 * постусловия: записывает в буфер прочитанный символ
 *              возвращает ширину символа в байтах
 *              или 0 при ошибке*/
static size_t read_symbol_utf8(uint8_t *symb_buf, FILE *fp);

/* операция:    распознание символа как буквы
 * предусловия: symb - строка символа
 *              width - ширина символа в байтах
 * постусловия: возвращает истину, если символ - буква
 *              (латинница или кириллица)
 *              иначе - ложь*/
static bool is_alpha_utf8(const uint8_t *symb, const size_t width);

int main(int argc, char **argv) {
    hash_t htab;
    FILE *fp;
    char str_buf[SIZE_BUF];

    /*проверка числа аргументов*/
    if (argc != 2) {
        fprintf(stderr, "Использование: %s файл\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    /*открытие файла для чтения*/
    if ((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Ошибка открытия %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    /*создание хеш-таблицы*/
    if ((htab = create_hash()) == NULL) {
        fprintf(stderr, "Ошибка создания хеш-таблицы\n");
        exit(EXIT_FAILURE);
    }
    /*заполнение хеш-таблицы*/
    while (read_word_utf8(str_buf, fp) != 0) {
        if ((htab = add_hash(str_buf, htab)) == NULL) {
            fclose(fp);
            fprintf(stderr, "Ошибка заполнения хеш-таблицы\n");
            exit(EXIT_FAILURE);
        }
    }
    /*вывод содержимого хеш-таблицы*/
    printf("Всего %ld слов(а)\n", print_hash(htab));

    fclose(fp);
    del_hash(htab);
    return 0;
}

/*чтение слова из файла*/
static size_t read_word_utf8(char *str_buf, FILE *fp) {
    uint8_t symb_buf[5];
    size_t width_str = 0;
    size_t width;

    /*поиск начала слова*/
    while ((width = read_symbol_utf8(symb_buf, fp))) {
        if (is_alpha_utf8(symb_buf, width)) {
            break;
        }
    }
    /*проверка на конец файла*/
    if (!width)
        return 0;

    str_buf[0] = '\0';

    /*чтение до первого небуквенного символа*/
    do {
        strcat(str_buf, (char *)symb_buf);
        width_str += width;
        if (!(width = read_symbol_utf8(symb_buf, fp)))
            break;
        if (!is_alpha_utf8(symb_buf, width))
            break;
    } while (width_str < (SIZE_BUF - 1));

    return width_str;
}

/*чтение символа utf-8 из файла*/
static size_t read_symbol_utf8(uint8_t *symb_buf, FILE *fp) {
    size_t width = 1;
    uint8_t byte_buf;

    if (!fread(&byte_buf, sizeof(uint8_t), 1, fp))
        return 0;
    /*если символ однобайтный*/
    if (byte_buf < ASCII_SIZE) {
        symb_buf[0] = byte_buf;
        symb_buf[1] = '\0';
        return width;
    /*иначе, определение количества байтов символа*/
    } else {
        symb_buf[0] = byte_buf;
        /*подсчёт байтов*/
        for (uint8_t bit = 0x40; (byte_buf & bit) || (width == 5); bit /= 2)
            width++;
        /*проверка на ошибку чтения символа*/
        if (width == 5)
            return 0;
        /*чтение остальных байтов*/
        for (size_t index = 1; index < width; index++) {
            if (!fread(&byte_buf, sizeof(uint8_t), 1, fp))
                return 0;
            symb_buf[index] = byte_buf;
        }
        symb_buf[width] = '\0';
    }
    return width;
}

/*распознание символа как буквы*/
static bool is_alpha_utf8(const uint8_t *symb, const size_t width) {
    uint16_t symb_tmp;

    /*если символ однобайтный (латинница)*/
    if (width == 1) {
        if (((symb[0] >= 0x41) && (symb[0] <= 0x5a))
                || ((symb[0] >= 0x61) && (symb[0] <= 0x7a)))
            return true;
    /*если двухбайтный (кириллица)*/
    } else if (width == 2) {
        symb_tmp = symb[0];
        symb_tmp <<= 8;
        symb_tmp |= symb[1];
        if (symb_tmp >= 0xd090 && symb_tmp <= 0xd18f)
            return true;
    } else {
        return false;
    }
    return false;
}
