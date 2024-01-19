#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "zip.h"

int main(int argc, char **argv) {
    struct stat stat_buf;   /* структура для статуса файла */
    eocd_t eocd_buf;        /* буфер для End of central ditectory record */
    cdfh_t cdfh_buf;        /* буфер для Central directory file header */
    bool_t flag = FALSE;    /* флаг архива */
    long offset, i = 1;
    FILE *fp;

    if (argc != 2) {        /* проверка аргументов командной строки */
        fprintf(stderr, "Использование: %s файл\n", argv[0]);
        exit(0);
    }
    if ((fp = fopen(argv[1], "rb")) == NULL) {       /* открытие файла */
        fprintf(stderr, "Ошибка открытия %s\n", argv[1]);
        exit(1);
    }
    if (fstat(fileno(fp), &stat_buf)) {         /* извлечение информации о файле */
        fprintf(stderr, "Ошибка извлечения информации о %s\n", argv[1]);
        exit(1);
    }

    /* поиск сигнатуры eocd 
     * расчёт начальной позиции eocd = размер_файла-структура_eocd_t-сигнатура 
     * если сигнатура найдена, то устанавливается флаг */
    for (offset = stat_buf.st_size - sizeof(eocd_t) - sizeof(uint32_t); offset != 0; --offset) {
        uint32_t signature = 0;

        fseek(fp, offset, 0);
        fread(&signature, sizeof(uint32_t), 1, fp);
        if (signature == 0x06054b50) {
            flag = TRUE;        /* файл является архивом */
            break;
        }
    }
    if (flag) {     /* если файл является архивом */
        printf("%s является архивом и содержит:\n", argv[1]);
        fread(&eocd_buf, sizeof(eocd_t), 1, fp);            /* чтение eocd в буфер */
        /* расчёт начала cdfh = размер_файла - всё_до_eocd + размер_всех_cdfh */
        offset = stat_buf.st_size - (ftell(fp) - sizeof(eocd_t) - sizeof(uint32_t))
                        + eocd_buf.size_of_central_directory;
        fseek(fp, -offset, 2);    /* смещение к первой записи cdfh с конца файла */
        /* количество итераций цикла соответствует number_central_directory_record */
        for (int total_cdfh = eocd_buf.number_central_directory_record; total_cdfh != 0; total_cdfh--) {
            uint32_t signature = 0;

            fread(&signature, sizeof(uint32_t), 1, fp);
            if (signature != 0x02014b50) {
                fprintf(stderr, "Ошибка чтения сигнатуры cdfh\n");
                exit(1);
            }
            fread(&cdfh_buf, sizeof(cdfh_t), 1, fp);    /* копирование cdfh в буфер */
            if (cdfh_buf.filename_length) {             /* считывание и вывод имени */
                char *filename = (char*)malloc(cdfh_buf.filename_length + 1);
                fread(filename, cdfh_buf.filename_length, 1, fp);
                filename[cdfh_buf.filename_length] = '\0';
                printf("%4ld. %s\n", i, filename);
                i++;
                free(filename);
            }
            if (cdfh_buf.extra_field_length)        /* пропуск поля с дополнительными данными */
                fseek(fp, cdfh_buf.extra_field_length, 1);
            if (cdfh_buf.file_comment_length)       /* пропуск поля с комментарием */
                fseek(fp, cdfh_buf.file_comment_length, 1);
        }

        fclose(fp);
        return 0;
    } else {        /* если файл не является архивом */
        printf("%s не является архивом\n", argv[1]);
        fclose(fp);
        return 0;
    }
}
