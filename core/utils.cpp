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
#include <nlohmann/json.hpp>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

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

    void sendToDiscordWebhook(const std::string& webhookUrl, const std::string& message) {
        json payload = {
            {"content", message}
        };

        std::wstring url = std::wstring(webhookUrl.begin(), webhookUrl.end());

        HINTERNET hSession = WinHttpOpen(L"A WinHTTP Example/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);

        if (!hSession) {
            throw std::runtime_error("Failed to open WinHTTP session");
        }

        size_t domain_end = url.find(L"/", 8);
        std::wstring domain = (domain_end != std::wstring::npos) ? url.substr(0, domain_end) : url;
        std::wstring path = (domain_end != std::wstring::npos) ? url.substr(domain_end) : L"/";

        HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);

        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to connect to server");
        }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to open request");
        }

        const wchar_t* headers = L"Content-Type: application/json\r\n";
        BOOL bResult = WinHttpAddRequestHeaders(hRequest, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD);

        if (!bResult) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to add request headers");
        }

        std::string payloadStr = payload.dump();
        bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            (LPVOID)payloadStr.c_str(), payloadStr.size(), payloadStr.size(), 0);

        if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to send message to Discord webhook");
        }

        std::cout << "Message sent to Discord webhook successfully!" << std::endl;

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
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