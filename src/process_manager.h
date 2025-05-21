#pragma once
#include <vector>

/**
 *! Менеджер процессов
 *  Добавление, удаление процессов по их ID
 *  Название процесса, логирование, вывод всех процессов
 */
class ProcessManager() {
public:
    uint8_t addProcess(const char* name, bool background, uint32_t interval);
    uint8_t startProcess();
    uint8_t stopProcess();
    uint8_t killProcess();

    vector<uint8_t> listProcess();
private:
    vector<uint8_t> _processes;

};