#include "private_hash.h"
#include "hash.h"

/*создание элемента хеш-таблицы*/
static h_item_t *create_item_hash(const char *str) {
    /*выделение памяти для элемента*/
    h_item_t *ret_val = (h_item_t *)malloc(sizeof(h_item_t));
    if (ret_val == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для элемента хеш-таблицы\n");
        return ret_val;
    }
    /*выделение памяти для строки*/
    char *tmp = (char *)malloc(sizeof(char) * (strlen(str) + 1));
    if (tmp == NULL) {
        free(ret_val);
        fprintf(stderr, "Ошибка выделения памяти для строки\n");
        return ret_val = NULL;
    }
    /*инициализация элемента*/
    ret_val->count = 1;
    strcpy(tmp, str);
    ret_val->word = tmp;
    return ret_val;
}

/*рехеширование*/
static hash_t re_hash(hash_t old_htab) {
    hash_t new_htab;
    size_t addr_item;

    /*проверка корректности индекса массива размеров таблицы*/
    if (i_hash + 1 >= sizeof(h_sizes) / (sizeof(size_t)))
        return NULL;
    /*размер новой таблицы*/
    i_hash++;
    /*проверка на ошибку создания новой таблицы*/
    if ((new_htab = create_hash()) == NULL) {
        i_hash--;
        return NULL;
    }
    /*перенос из старой таблицы в новую*/
    for (addr_item = 0; addr_item < h_sizes[i_hash - 1]; addr_item++) {
        /*к следующей итерации, если элемент отсутствует*/
        if (old_htab[addr_item] == NULL)
            continue;
        /*вставка элемента в новую таблицу*/
        if (add_hash(old_htab[addr_item]->word, new_htab) == NULL) {
            i_hash--;
            return NULL;
        }
    }
    printf("рехеширование %ld\n", i_hash);
    /*удаление старой таблицы*/
    i_hash--;
    del_hash(old_htab);
    i_hash++;
    /*возвращение адреса новой таблицы*/
    return new_htab;
}

/*хеш-функция*/
static uint32_t hashly(const char *str) {
    uint32_t hash = 0;

    for(; *str != '\0'; str++)
        hash = (hash * 1664525) + (uint8_t)(*str) + 1013904223;
    return hash;
}

/*реализация функций публичного интерфейса*/

/*создание и инициализация хеш-таблицы*/
hash_t create_hash(void) {
    hash_t ret_val;

    /*выделение памяти для массива указателей типа hash_t*/
    if ((ret_val = (hash_t)malloc(sizeof(h_item_t *) * h_sizes[i_hash])) == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для хеш-таблицы\n");
        return ret_val;
    }
    /*инициализация элементов массива*/
    for (size_t index = 0; index < h_sizes[i_hash]; index++)
        ret_val[index] = NULL;
    return ret_val;
}

/*добавление элемента в хеш-таблицу*/
hash_t add_hash(const char *str, hash_t htab) {
    size_t addr_item;
    hash_t new_htab;

    /*поиск свободного адреса или совпадения*/
    for (int count_probe = 0; count_probe <= h_sizes[i_hash]; count_probe++) {
        /*линейное пробирование*/
        addr_item = (hashly(str) + count_probe) % h_sizes[i_hash];
        /*если найдена свободная ячейка таблицы*/
        if (htab[addr_item] == NULL) {
            /*добавление в неё элемента*/
            if ((htab[addr_item] = create_item_hash(str)) == NULL) {
                fprintf(stderr, "Ошибка добавления элемента в таблицу\n");
                del_hash(htab);
                return NULL;
            }
            return htab;
        }
        /*если найдено совпадение*/
        if (strcmp(htab[addr_item]->word, str) == 0) {
            htab[addr_item]->count++;
            return htab;
        }
        /*если 3 коллизии подряд, то попытка рехеширования*/
        if (count_probe == 3) {
            /*при ошибке рехеширования продолжается линейное пробирование*/
            if ((new_htab = re_hash(htab)) == NULL) {
                continue;
            }
            /*иначе, элемент добавляется в новую таблицу*/
            count_probe = -1;
            htab = new_htab;
        }
    }
    /*если нет места в таблице, а рехеширование невозможно*/
    fprintf(stderr, "Таблица переполнена\n");
    del_hash(htab);
    return NULL;
}

/*вывод элементов хэш-таблицы*/
size_t print_hash(hash_t htab) {
    size_t ret_val = 0;

    for (size_t index = 0; index < h_sizes[i_hash]; index++) {
        if (htab[index] != NULL) {
            printf("    %s - %ld\n", htab[index]->word, htab[index]->count);
            ret_val++;
        }
    }
    return ret_val;
}

/*удаление хеш-таблицы*/
void del_hash(hash_t htab) {
    for (size_t index = 0; index < h_sizes[i_hash]; index++)
        if (htab[index] != NULL) {
            free(htab[index]->word);    /*освобождение памяти строки*/
            free(htab[index]);          /*освобождение памяти элемента*/
        }
    free(htab);         /*и самой таблицы*/
}
