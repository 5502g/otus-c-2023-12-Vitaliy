/*копия предоставленного файла main.asm на языке си*/
#include "list.h"

int main()
{
    node_t *nodep = NULL;

    for (int64_t index = sizeof(data)/8; index > 0; index--)
        nodep = add_element(data[index-1], nodep);
    m(nodep, print_int);
    puts("");
    m(f(nodep, NULL, p), print_int);
    puts("");

    return 0;
}

/*создание и инициализация узла*/
node_t *add_element(int64_t num, node_t *nodep)
{
    node_t *ret_val = (node_t *)malloc(sizeof(node_t));
    if (ret_val == NULL) {
        abort();
    }
    ret_val->number = num;
    ret_val->next = nodep;
    return ret_val;
}

/*применение функции к каждому элементу списка*/
void m(node_t *nodep, void (*pfun)(node_t node))
{
    if (nodep == NULL)
        return;
    pfun(*nodep);
    m(nodep->next, pfun);
}

/*вывод данных узла*/
void print_int(node_t node)
{
    printf("%ld ", node.number);
    fflush(NULL);
}

/*создание нового списка только с нечётными числами*/
node_t *f(node_t *nodep, node_t *prev_nodep, int64_t (*pfun)(int64_t num))
{
    node_t *tmp = prev_nodep;

    if (nodep == NULL)
        return tmp;
    if (pfun(nodep->number) != 0) {
        tmp = add_element(nodep->number, tmp);
    }
    return f(nodep->next, tmp, pfun);
}

/*проверка числа на чётность*/
int64_t p(int64_t num)
{
    return num & 1;
}
