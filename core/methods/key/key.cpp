#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "key.h"
#include "../../utils.h"

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

std::string fetchData(const std::wstring& url) {
    size_t protocolPos = url.find(L"://");
    if (protocolPos == std::wstring::npos) {
        throw std::runtime_error("Invalid URL, missing protocol.");
    }

    std::wstring domain = url.substr(protocolPos + 3);
    size_t pathPos = domain.find(L"/");

    std::wstring path = (pathPos == std::wstring::npos) ? L"/" : domain.substr(pathPos);
    domain = domain.substr(0, pathPos);

    HINTERNET hSession = WinHttpOpen(L"A WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
        throw std::runtime_error("Failed to open WinHTTP session");
    }

    HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
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

    utils::debug("Checking key for service: " + config.serviceName, __FILE__, __LINE__);
    utils::debug("Webhook URL: " + config.webhookUrl, __FILE__, __LINE__);
    utils::debug("Auth Type: " + config.auth_type, __FILE__, __LINE__);

    try {
        std::string jsonData = fetchData(finalUrl);

        if (jsonData.empty()) {
			utils::error("Fetched JSON data is empty!", __FILE__, __LINE__);
            return;
        }

        json parsedData = json::parse(jsonData);

        for (auto& [key, value] : parsedData.items()) {
			utils::debug("Key found:" + key, __FILE__, __LINE__);

            if (config.webhookType == "discord") {
                utils::sendDiscordEmbed(
                    config.webhookUrl,
                    (value["hwid"].is_null() ? "null" : value["hwid"].get<std::string>()),
                    (value["service"].is_null() ? "null" : value["service"].get<std::string>()),
                    (value["expires"].is_null() ? "null" : value["expires"].get<std::string>()),
                    (value["tier"].is_null() ? "null" : value["tier"].get<std::string>()),
                    (value["hwidLocked"].is_null() ? false : value["hwidLocked"].get<bool>()),
                    (value["serviceLocked"].is_null() ? false : value["serviceLocked"].get<bool>())
                );
            }
        }
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << " at byte " << e.byte << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
