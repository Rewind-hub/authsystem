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

    void sendDiscordEmbed(
        const std::string& webhookUrl,
        const std::string& hwid,
        const std::string& service,
        const std::string& expires,
        const std::string& tier,
        bool hwidLocked,
        bool serviceLocked
    ) {
        // Create the embed payload
        json embed = {
            {"title", "Authentication Key Info"},
            {"color", 3447003},  // Blue color
            {"fields", json::array({
                {{"name", "HWID"}, {"value", hwid}, {"inline", false}},
                {{"name", "Service"}, {"value", service}, {"inline", false}},
                {{"name", "Expires"}, {"value", expires}, {"inline", false}},
                {{"name", "Tier"}, {"value", tier}, {"inline", false}},
                {{"name", "HWID Locked"}, {"value", hwidLocked ? "true" : "false"}, {"inline", false}},
                {{"name", "Service Locked"}, {"value", serviceLocked ? "true" : "false"}, {"inline", false}}
            }) }
        };

        json payload = {
            {"embeds", json::array({embed})}
        };

        std::string payloadStr = payload.dump();

        // Initialize cURL
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize cURL");
        }

        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr); // No custom headers, using default

        // Set content type header for JSON
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Error sending to Discord: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            throw std::runtime_error("Failed to send message to Discord webhook");
        }

        std::cout << "Message sent to Discord webhook successfully!" << std::endl;

        // Cleanup cURL
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
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