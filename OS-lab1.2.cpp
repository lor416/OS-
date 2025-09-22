#include <windows.h>    
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>    
#include <locale.h>     
#include <process.h>    // Для функции _beginthreadex

// Структура для передачи данных в поток (вместо глобальных переменных)
typedef struct {
    char str[256];
    int length;
    //int sleepTime;
} ThreadData;

int isLatinChar(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Функция потока worker для CreateThread
DWORD WINAPI workerThreadCreate(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    int sleepTime = 1000;

    printf("\nПоток worker начал работу (создан CreateThread)\n");
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
    printf("\nРезультат работы потока worker: выведены латинские символы из строки\n");

    return 0;
}

// Функция потока worker для _beginthreadex
unsigned __stdcall workerThreadBeginThread(void* param) {
    ThreadData* data = (ThreadData*)param;
    int sleepTime = 1000;

    printf("\nПоток worker начал работу (создан _beginthreadex)\n");
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

  
    printf("\nРезультат работы потока worker: выведены латинские символы из строки\n");

    return 0;
}

int main() {
    setlocale(LC_ALL, "Rus");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int sleepTime;
    char inputString[256];
    HANDLE hThread = NULL; // идентификатор
    DWORD threadId; // дескриптор
    unsigned threadIdBegin;
    ThreadData data;
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
  
    hThread = CreateThread( //дескриптор
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

    Sleep(sleepTime);
    if (ResumeThread(hThread) == (DWORD)-1) {
        printf("Ошибка возобновления потока. Код: %d\n", GetLastError());
        CloseHandle(hThread);
        return 1;
    }
    DWORD waitResult = WaitForSingleObject(hThread, INFINITE);

    if (waitResult != WAIT_OBJECT_0) {
        printf("Ошибка ожидания потока. Код: %lu\n", waitResult);
    }

    DWORD exitCode;
    if (!GetExitCodeThread(hThread, &exitCode)) {
        printf("Ошибка получения кода возврата потока\n");
    }
    CloseHandle(hThread);


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

    Sleep(sleepTime);

    ResumeThread(hThread2);
    waitResult = WaitForSingleObject(hThread2, INFINITE);

    if (waitResult != WAIT_OBJECT_0) {
        printf("Ошибка ожидания потока. Код: %lu\n", waitResult);
    }

    if (!GetExitCodeThread(hThread2, &exitCode)) {
        printf("Ошибка получения кода возврата потока\n");
    }

    CloseHandle(hThread2);

    return 0;
}