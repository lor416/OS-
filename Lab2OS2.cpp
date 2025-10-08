#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

// Режим потомка
int RunChild() {
    // В потомке GetStdHandle(STD_INPUT_HANDLE) вернет дескриптор канала от родителя
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    int size = 0;
    DWORD bytesRead = 0;

    // Читаем размер массива из канала (Pipe1)
    if (!ReadFile(hStdin, &size, sizeof(size), &bytesRead, NULL)) {
        return 1;
    }

    std::vector<int> array(size);
    if (!ReadFile(hStdin, array.data(), size * sizeof(int), &bytesRead, NULL)) {
        return 1;
    }

    long long sum = 0;
    for (int num : array) {
        sum += num * num;
    }

    // Записываем результат в stdout (который перенаправлен в Pipe2)
    DWORD bytesWritten;
    WriteFile(hStdout, &sum, sizeof(sum), &bytesWritten, NULL);

    return 0;
}

// Режим родителя
int RunParent() {
    std::cout << "Parent:" << std::endl;

    int size;
    std::cout << "Enter array size: ";
    std::cin >> size;

    if (size <= 0) {
        std::cout << "Invalid size!" << std::endl;
        return 1;
    }
    std::vector<int> array(size);
    for (int i = 0; i < size; i++) {
        std::cin >> array[i];
    }

    HANDLE hReadFromChild, hWriteToChild;    // Pipe1: родитель -> потомок
    HANDLE hReadFromParent, hWriteToParent;  // Pipe2: потомок -> родитель

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    //  Pipe1
    if (!CreatePipe(&hReadFromChild, &hWriteToChild, &sa, 0)) {
        std::cout << "CreatePipe1 failed!" << std::endl;
        return 1;
    }

    //  Pipe2 
    if (!CreatePipe(&hReadFromParent, &hWriteToParent, &sa, 0)) {
        std::cout << "CreatePipe2 failed!" << std::endl;
        CloseHandle(hReadFromChild);
        CloseHandle(hWriteToChild);
        return 1;
    }

    // Настраиваем дочерний процесс
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    // Перенаправение ввода
    si.hStdInput = hReadFromChild;   // Потомок читает из Pipe1
    si.hStdOutput = hWriteToParent;  // Потомок пишет в Pipe2
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    // Получаем имя текущего исполняемого файла
    char currentExe[MAX_PATH];
    GetModuleFileNameA(NULL, currentExe, MAX_PATH);
    std::string commandLine = std::string(currentExe) + " child";

    // Запускаем дочерний процесс
    if (!CreateProcessA(NULL, const_cast<char*>(commandLine.c_str()),
        NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {

        std::cout << "CreateProcess failed!" << std::endl;
        CloseHandle(hReadFromChild);
        CloseHandle(hWriteToChild);
        CloseHandle(hReadFromParent);
        CloseHandle(hWriteToParent);
        return 1;
    }

    // Закрываем в родителе ненужные дескрипторы:
    CloseHandle(hReadFromChild); 
    CloseHandle(hWriteToParent);   

    // Передаем данные потомку через Pipe1
    DWORD written;
    WriteFile(hWriteToChild, &size, sizeof(size), &written, NULL);
    WriteFile(hWriteToChild, array.data(), size * sizeof(int), &written, NULL);

    // Закрываем запись в Pipe1
    CloseHandle(hWriteToChild);

    std::cout << "Data sent to child" << std::endl;
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Читаем результат из Pipe2
    long long result;
    DWORD bytesRead;
    if (ReadFile(hReadFromParent, &result, sizeof(result), &bytesRead, NULL)) {
        std::cout << "Result from child: " << result << std::endl;
    }
    else {
        std::cout << "Failed to read result" << std::endl;
    }

    // Закрываем все дескрипторы
    CloseHandle(hReadFromParent);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    system("pause");

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "child") == 0) {
        return RunChild();
    }
    else {
        return RunParent();
    }
    system("pause");
}