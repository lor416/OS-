#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

// Режим потомка
int RunChild() {
    std::cout << "=== CHILD PROCESS ===" << std::endl;

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    int size = 0;
    DWORD bytesRead = 0;

    if (!ReadFile(hStdin, &size, sizeof(size), &bytesRead, NULL) || bytesRead != sizeof(size)) {
        std::cout << "Failed to read size" << std::endl;
        return 1;
    }

    std::vector<int> array(size);
    if (!ReadFile(hStdin, array.data(), size * sizeof(int), &bytesRead, NULL)) {
        std::cout << "Failed to read array" << std::endl;
        return 1;
    }

    long long sum = 0;
    for (int num : array) {
        sum += num * num;
    }

    std::cout << "Sum of squares: " << sum << std::endl;
    return (int)sum;
}

// Режим родителя
int RunParent() {
    std::cout << "=== PARENT PROCESS ===" << std::endl;

    int size;
    std::cout << "Enter array size: ";
    std::cin >> size;

    if (size <= 0) {
        std::cout << "Invalid size!" << std::endl;
        return 1;
    }

    // Ввод элементов массива
    std::vector<int> array(size);
    std::cout << "Enter " << size << " elements:" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cin >> array[i];
    }

    // Создаем каналы
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        std::cout << "CreatePipe failed!" << std::endl;
        return 1;
    }

    // Настраиваем дочерний процесс
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    si.hStdInput = hReadPipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    // Получаем имя текущего исполняемого файла
    char currentExe[MAX_PATH];
    GetModuleFileNameA(NULL, currentExe, MAX_PATH);

    std::string commandLine = std::string(currentExe) + " child";

    // Запускаем дочерний процесс
    if (!CreateProcessA(
        NULL,
        const_cast<char*>(commandLine.c_str()),
        NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {

        std::cout << "CreateProcess failed! Error: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    CloseHandle(hReadPipe);

    // Передаем данные
    DWORD written;

    WriteFile(hWritePipe, &size, sizeof(size), &written, NULL);
    WriteFile(hWritePipe, array.data(), size * sizeof(int), &written, NULL);
    CloseHandle(hWritePipe);

    // Ждем завершения
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    std::cout << "Child process result: " << exitCode << std::endl;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "child") == 0) {
        return RunChild();
    }
    else {
        return RunParent();
    }
}