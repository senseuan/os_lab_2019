/* process_memory.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Макрос для вывода адреса переменной */
#define SHW_ADR(ID, I) (printf("Идентификатор %s \t находится по виртуальному адресу: %p\n", ID, (void*)&I))

/* Внешние глобальные переменные для памяти процесса */
extern int etext, edata, end;

char *cptr = "Это сообщение выводится функцией showit()\n";
char buffer1[25]; // Увеличим размер буфера

int showit(char *p);

int main() {
    int i = 0;

    printf("\nАдреса сегментов программы:\n");
    printf("Адрес etext (конец текстового сегмента/кода): %p \n", (void*)&etext);
    printf("Адрес edata (конец инициализированных данных): %p \n", (void*)&edata);
    printf("Адрес end (конец BSS сегмента): %p \n", (void*)&end);

    SHW_ADR("main", main);
    SHW_ADR("showit", showit);
    SHW_ADR("cptr", cptr);
    SHW_ADR("buffer1", buffer1);
    SHW_ADR("i", i);
    
    // Используем strncpy для безопасности
    strncpy(buffer1, "Демонстрация работы\n", sizeof(buffer1) - 1);
    buffer1[sizeof(buffer1) - 1] = '\0';
    
    write(1, buffer1, strlen(buffer1));
    
    showit(cptr);
    
    return 0;
}

int showit(char *p) {
    char *buffer2;
    
    SHW_ADR("buffer2", buffer2);
    
    if ((buffer2 = (char *)malloc(strlen(p) + 1)) != NULL) {
        printf("Выделена память по адресу %p (в куче)\n", (void*)buffer2);
        strcpy(buffer2, p);
        printf("%s", buffer2);
        free(buffer2);
    } else {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    
    return 0;
}