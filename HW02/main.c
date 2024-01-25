#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "to_utf8.h"

int main(int argc, char **argv) {
    const uint8_t (*table_p)[];      /*указатель на массив кодов*/
    uint8_t ch_buf;         /*буфер для прочитанного символа*/
    FILE *src, *dst;

    /*проверка числа аргументов*/
    if (argc != 3) {
        fprintf(stderr, "Использование: %s файл кодировка\n"
                "Варианты кодировок: cp1251, koi8, iso-8859-5\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /*определение входящей кодировки*/
    if(!strcmp(argv[2], "cp1251"))
        table_p = &cp1251;
    else if(!strcmp(argv[2], "koi8"))
        table_p = &koi8;
    else if(!strcmp(argv[2], "iso-8859-5"))
        table_p = &iso8859;
    else {
        fprintf(stderr, "Аргумент \"кодировка\" - варианты написания:\n"
                "    cp1251\n    koi8\n    iso-8859-5\n");
        exit(EXIT_SUCCESS);
    }

    /*открытие файлов*/
    if ((src = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Ошибка открытия %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    if ((dst = fopen("utf8.txt", "wb")) == NULL) {
        fclose(src);
        fprintf(stderr, "Ошибка открытия utf8.txt\n");
        exit(EXIT_FAILURE);
    }

    /*цикл чтения и записи символа*/
    while (fread(&ch_buf, sizeof(uint8_t), 1, src)) {
        symbol_t symbol = search_utf8(ch_buf, *table_p);
        if (symbol.width != write_utf8(symbol, dst)) {
            fprintf(stderr, "Ошибка записи в файл utf8.txt\n");
            break;
        }
    }
    fclose(src);
    fclose(dst);

    return 0;
}

symbol_t search_utf8(uint8_t ch, const uint8_t *table) {
    size_t addr_byte = 0, count_symb = 0;   /*адрес в массиве и счётчик символов*/
    symbol_t ret_val;

    if (ch < ASCII_SIZE) {         /*если совпадает с ascii то символ возвращается как есть*/
        ret_val.data[0] = ch;
        ret_val.width = 1;
        return ret_val;
    }

    ch -= ASCII_SIZE;

    /*цикл поиска символа в массиве и определение его размера*/
    while (count_symb != ch) {
        ret_val.width = 2;                  /*минимальная ширина символа*/
        if (table[addr_byte] & 0x20) {      /*(0b00100000) для проверки на трёхбайтный символ*/
            ret_val.width++;
            if (table[addr_byte] == 0xff)   /*проверка на ячейку 0x98 cp1251*/
                ret_val.width = 1;
        }
        addr_byte += ret_val.width;         /*адрес следующего символа в массиве*/
        count_symb++;
    }

    /*запись кода символа в структуру ret_val*/
    for(size_t index = 0; index != ret_val.width; index++)
        ret_val.data[index] = table[addr_byte + index];

    return ret_val;
}

size_t write_utf8(symbol_t symbol, FILE *dst) {
    size_t ret_val = 0;

    /*побайтная запись символа в файл*/
    for (size_t index = 0; index != symbol.width; index++)
        ret_val += fwrite(&symbol.data[index], sizeof(uint8_t), 1, dst);

    return ret_val;
}
