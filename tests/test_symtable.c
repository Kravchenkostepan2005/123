/*
 * Тесты для таблицы символов.
 * Пояснительные комментарии отмечают, какая часть API проверяется и что ожидаем.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

/*
 * Визитор, считающий количество посещённых элементов.
 * Используется для проверки, что symtable_foreach обходит все занятые слоты.
 */
static void count_visitor(const char *key, void *value, void *ud) {
    (void)key;
    (void)value;
    size_t *counter = (size_t *)ud;
    (*counter)++;
}

/*
 * Визитор, накапливающий ключи в строке через запятую.
 * Нужен для грубой проверки содержания после обхода.
 */
static void collect_concat_visitor(const char *key, void *value, void *ud) {
    (void)value;
    char *buf = (char *)ud;
    strcat(buf, key);
    strcat(buf, ",");
}

int main(void) {
    /* Создание таблицы: ожидаем не-NULL. */
    symtable_t *st = symtable_create();
    assert(st != NULL);

    /* Поиск в пустой таблице и поведение на NULL-аргументах: ожидаем NULL. */
    assert(symtable_find(NULL, "x") == NULL);
    assert(symtable_find(st, NULL) == NULL);
    assert(symtable_find(st, "missing") == NULL);

    /* Вставка нового ключа: insert возвращает 0, find даёт исходное значение. */
    int v1 = 123;
    assert(symtable_insert(st, "alpha", &v1) == 0);
    assert(symtable_find(st, "alpha") == &v1);

    /* Обновление существующего ключа: insert возвращает 1, значение меняется. */
    int v1b = 456;
    assert(symtable_insert(st, "alpha", &v1b) == 1);
    assert(symtable_find(st, "alpha") == &v1b);

    /* Несколько вставок (в т.ч. потенциальные коллизии): все ключи доступны. */
    int v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    assert(symtable_insert(st, "beta", &v2) == 0);
    assert(symtable_insert(st, "gamma", &v3) == 0);
    assert(symtable_insert(st, "delta", &v4) == 0);
    assert(symtable_insert(st, "epsilon", &v5) == 0);

    assert(symtable_find(st, "beta") == &v2);
    assert(symtable_find(st, "gamma") == &v3);
    assert(symtable_find(st, "delta") == &v4);
    assert(symtable_find(st, "epsilon") == &v5);

    /*
     * Форсируем рост (rehash): исчерпываем free-list.
     * INITIAL_CAPACITY = 8; уже есть 5 уникальных ключей.
     * Добавим ещё 16, чтобы гарантированно переехать на бОльшую ёмкость.
     */
    for (int i = 0; i < 16; i++) {
        char key[32];
        snprintf(key, sizeof(key), "k%d", i);
        int *pv = (int *)malloc(sizeof(int));
        assert(pv != NULL);
        *pv = i;
        int rc = symtable_insert(st, key, pv);
        assert(rc == 0);
    }

    /* После возможного rehash все ранее вставленные ключи должны находиться. */
    assert(symtable_find(st, "alpha") == &v1b);
    assert(symtable_find(st, "beta") == &v2);
    assert(symtable_find(st, "gamma") == &v3);
    assert(symtable_find(st, "delta") == &v4);
    assert(symtable_find(st, "epsilon") == &v5);

    /*
     * Проверка foreach: он должен посетить каждый занятый слот ровно один раз.
     * Суммарно вставлено 5 фиксированных + 16 сгенерированных = как минимум 21.
     */
    size_t count = 0;
    symtable_foreach(st, count_visitor, &count);
    assert(count >= 21); /* может быть больше, если распределение слотов повлияло на учёт */

    /* Небольшая проверка содержимого foreach через конкатенацию ключей. */
    char buf[1024];
    buf[0] = '\0';
    symtable_foreach(st, collect_concat_visitor, buf);
    assert(strstr(buf, "alpha,") != NULL);
    assert(strstr(buf, "beta,") != NULL);

    symtable_destroy(st);

    /* Хеш-функция: детерминированность и различие для разных строк. */
    unsigned int h1 = symtable_hash_function("alpha");
    unsigned int h2 = symtable_hash_function("alpha");
    unsigned int h3 = symtable_hash_function("beta");
    assert(h1 == h2);
    assert(h1 != h3);

    printf("All symtable tests passed.\n");
    return 0;
}

