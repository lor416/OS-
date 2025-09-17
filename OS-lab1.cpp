#include <windows.h>    
#include <stdio.h>      // Ввод-вывод
#include <stdlib.h>     // Стандартные функции
#include <string.h>     // Функции для работы со строками
#include <locale.h>     
#include <process.h>    // Для функции _beginthreadex

// Структура для передачи данных в поток (вместо глобальных переменных)
typedef struct {
    char str[256];
    int length;
    int sleepTime;     
} ThreadData;

int isLatinChar(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Функция потока worker для CreateThread
DWORD WINAPI workerThreadCreate(LPVOID param) {
    ThreadData* data = (ThreadData*)param;

    printf("Поток worker начал работу (создан CreateThread)\n");
    printf("Латинские символы: ");

    int found = 0;
    for (int i = 0; i < data->length; i++) {
        char c = data->str[i];
        if (isLatinChar(c)) {
            printf("%c ", c);
            found++;
        }
    }

    if (found == 0) {
        printf("латинские символы не найдены");
    }

    printf("\nЗадержка %d мс после поиска...\n", data->sleepTime);
    Sleep(data->sleepTime);  

    printf("Поток worker завершил работу\n");
    return 0;
}

// Функция потока worker для _beginthreadex
unsigned __stdcall workerThreadBeginThread(void* param) {
    ThreadData* data = (ThreadData*)param;

    printf("Поток worker начал работу (создан _beginthreadex)\n");
    printf("Латинские символы: ");

    int found = 0;
    for (int i = 0; i < data->length; i++) {
        char c = data->str[i];
        if (isLatinChar(c)) {
            printf("%c ", c);
            found++;
        }
    }

    if (found == 0) {
        printf("латинские символы не найдены");
    }

    printf("\nЗадержка %d мс после поиска...\n", data->sleepTime);
    Sleep(data->sleepTime); 

    printf("Поток worker завершил работу\n");
    return 0;
}

int main() {
    setlocale(LC_ALL, "Rus");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int sleepTime;                 
    char inputString[256];
    HANDLE hThread = NULL;
    DWORD threadId;
    unsigned threadIdBegin;
    ThreadData data;

    printf("=== Лабораторная работа №1. Вариант 11 ===\n");
    printf("Тема: Создание потоков (WIN32API)\n\n");

    // 1. Создание массива (строки)
    printf("1. Введите строку: ");
    fflush(stdin);

    if (fgets(inputString, sizeof(inputString), stdin) == NULL) {
        printf("Ошибка чтения строки\n");
        return 1;
    }

    // Удаление символа новой строки
    int length = strlen(inputString);
    if (length > 0 && inputString[length - 1] == '\n') {
        inputString[length - 1] = '\0';
        length--;
    }

    if (length == 0) {
        printf("Строка пуста\n");
        return 1;
    }

    // 2. Ввод временного промежутка для остановки и запуска потока
    printf("2. Введите временной промежуток для задержки потока (мс): ");
    if (scanf_s("%d", &sleepTime) != 1) {
        printf("Ошибка ввода времени\n");
        return 1;
    }

    // Подготовка данных для передачи в поток
    strcpy_s(data.str, sizeof(data.str), inputString);
    data.length = length;
    data.sleepTime = sleepTime;  

    printf("\n3. Создание потока worker...\n");
    printf("4. Создание потока в подвешенном состоянии...\n");

    // ========== СПОСОБ 1: CreateThread ==========
    printf("\n--- Способ 1: CreateThread ---\n");

    hThread = CreateThread(
        NULL,
        0,
        workerThreadCreate,
        &data,
        CREATE_SUSPENDED,
        &threadId
    );

    if (hThread == NULL) {
        printf("Ошибка создания потока CreateThread. Код: %d\n", GetLastError());
        return 1;
    }

    printf("Поток создан с ID: %lu (подвешен)\n", threadId);
    printf("Дескриптор потока: %p\n", hThread);

    // Приостанавливаем главный поток на введенное время
    printf("Приостанавливаем главный поток на %d мс...\n", sleepTime);
    Sleep(sleepTime);

    // Запускаем worker поток
    printf("Запускаем поток (ResumeThread)...\n");
    if (ResumeThread(hThread) == (DWORD)-1) {
        printf("Ошибка возобновления потока. Код: %d\n", GetLastError());
        CloseHandle(hThread);
        return 1;
    }

    // 6. Ожидание завершения потока worker
    printf("\n6. Ожидание завершения потока worker...\n");
    DWORD waitResult = WaitForSingleObject(hThread, INFINITE);

    if (waitResult != WAIT_OBJECT_0) {
        printf("Ошибка ожидания потока. Код: %lu\n", waitResult);
    }
    
    // 7. Получение результата работы потока
    DWORD exitCode;
    if (!GetExitCodeThread(hThread, &exitCode)) {
        printf("Ошибка получения кода возврата потока\n");
    }
    else {
        printf("Поток завершился с кодом: %lu\n", exitCode);
    }

    CloseHandle(hThread);

    // ========== СПОСОБ 2: _beginthreadex ==========
    printf("\n--- Способ 2: _beginthreadex ---\n");

    HANDLE hThread2 = (HANDLE)_beginthreadex(
        NULL,
        0,
        workerThreadBeginThread,
        &data,
        CREATE_SUSPENDED,
        &threadIdBegin
    );

    if (hThread2 == NULL) {
        printf("Ошибка создания потока _beginthreadex\n");
        return 1;
    }

    printf("Поток создан с ID: %u (подвешен)\n", threadIdBegin);
    printf("Дескриптор потока: %p\n", hThread2);

    // Приостанавливаем на введенное время
    printf("Приостанавливаем главный поток на %d мс...\n", sleepTime);
    Sleep(sleepTime);

    // Запускаем поток
    printf("Запускаем поток (ResumeThread)...\n");
    ResumeThread(hThread2);

    // Ожидание завершения второго потока
    printf("\n6. Ожидание завершения потока worker...\n");
    waitResult = WaitForSingleObject(hThread2, INFINITE);

    if (waitResult != WAIT_OBJECT_0) {
        printf("Ошибка ожидания потока. Код: %lu\n", waitResult);
    }

    // Получение кода завершения
    if (!GetExitCodeThread(hThread2, &exitCode)) {
        printf("Ошибка получения кода возврата потока\n");
    }
    else {
        printf("Поток завершился с кодом: %lu\n", exitCode);
    }

    CloseHandle(hThread2);

    // 8. Завершение работы
    printf("\n8. Главный поток завершает работу\n");
    printf("Результат работы потока worker: выведены латинские символы из строки\n");

    printf("\nНажмите Enter для выхода...\n");
    while (getchar() != '\n');
    getchar();

    return 0;
}