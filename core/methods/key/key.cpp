#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "key.h"
#include "../utils.h"

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

std::string fetchData(const std::wstring& url) {
    std::wstring domain, path;
    size_t domain_end = url.find(L"/", 8);
    if (domain_end != std::wstring::npos) {
        domain = url.substr(0, domain_end); 
        path = url.substr(domain_end);
    }
    else {
        domain = url;
        path = L"/";
    }

    HINTERNET hSession = WinHttpOpen(L"A WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
        throw std::runtime_error("Failed to open WinHTTP session");
    }

    HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT, 0);

    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to connect to server");
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to open request");
    }

    BOOL bResult = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0,
        0, 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to receive response");
    }

    std::string response;
    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        char buffer[4096] = { 0 };

        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            break;
        }

        if (!WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
            break;
        }

        response.append(buffer, dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

void checkKey(const Config& config, const std::wstring& url) {
    std::wstring finalUrl = url;

    if (finalUrl.empty()) {
        std::wcout << "Enter the full URL (e.g., https://github.com/Rewind-hub/authsystem/json): ";
        std::wcin >> finalUrl;
    }

    std::cout << "Checking key for service: " << config.serviceName << std::endl;
    std::cout << "Webhook URL: " << config.webhookUrl << std::endl;
    std::cout << "Auth Type: " << config.auth_type << std::endl;

    try {
        std::string jsonData = fetchData(finalUrl);

        json parsedData = json::parse(jsonData);

        for (auto& [key, value] : parsedData.items()) {
            std::cout << "Key: " << key << std::endl;
            std::cout << "HWID: " << (value["hwid"].is_null() ? "null" : value["hwid"].get<std::string>()) << std::endl;
            std::cout << "Service: " << value["service"].get<std::string>() << std::endl;
            std::cout << "Expires: " << value["expires"].get<std::string>() << std::endl;
            std::cout << "Tier: " << value["tier"].get<std::string>() << std::endl;
            std::cout << "HWID Locked: " << (value["hwidLocked"].get<bool>() ? "true" : "false") << std::endl;
            std::cout << "Service Locked: " << (value["serviceLocked"].get<bool>() ? "true" : "false") << std::endl;
            std::cout << "----------------------------------" << std::endl;

            if (config.webhookType == "discord") {
                std::cout << "Sending data to Discord webhook: " << config.webhookUrl << std::endl;
                utils::sendToDiscordWebhook(config.webhookUrl, "test message");
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
