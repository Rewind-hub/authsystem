#include "utils.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <fstream>
#include <json/json.h>
#include <curl/curl.h>
#include <ctime>
#include <thread>
#include <TlHelp32.h>
#include <filesystem>

namespace utils
{
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void enableVirtualTerminalProcessing() {
        DWORD dwMode = 0;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return;

        if (!GetConsoleMode(hOut, &dwMode)) return;

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void delay(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void countdownAndExit(int start) {
        for (int i = start; i >= 0; --i) {
            std::cout << "\r[" << i << "] Exiting Program.." << std::flush;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "\r[0] Program Exited.  " << std::flush << std::endl;
        exit(0);
    }

    std::string getFileName(const char* file) {
        const char* base = strrchr(file, '\\');
        if (base == nullptr) base = strrchr(file, '/');
        return (base == nullptr) ? file : base + 1;
    }

    void debug(const std::string& message, const char* file, int line) {
        std::cout << WHITE_COLOR "[" BLUE_COLOR "DEBUG" WHITE_COLOR "] [" << getFileName(file) << ":" << line << "] " << message << RESET_COLOR << std::endl;
    }

    void warning(const std::string& message, const char* file, int line) {
        std::cout << WHITE_COLOR "[" YELLOW_COLOR "WARNING" WHITE_COLOR "] [" << getFileName(file) << ":" << line << "] " << message << RESET_COLOR << std::endl;
    }

    void error(const std::string& message, const char* file, int line) {
        std::cout << WHITE_COLOR "[" RED_COLOR "ERROR" WHITE_COLOR "] [" << getFileName(file) << ":" << line << "] " << message << RESET_COLOR << std::endl;
    }

    void info(const std::string& message, const char* file, int line) {
        std::cout << WHITE_COLOR "[" GREEN_COLOR "INFO" WHITE_COLOR "] [" << getFileName(file) << ":" << line << "] " << message << RESET_COLOR << std::endl;
    }
}