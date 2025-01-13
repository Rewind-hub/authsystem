#ifndef KEY_HPP
#define KEY_HPP

#include <string>
#include <nlohmann/json.hpp>

struct Config {
    std::string auth_type;
    bool hwidStatus;
    bool serviceLocked;
    std::string serviceName;
    std::string webhookUrl;
    std::string webhookType;
};

void checkKey(const Config& config, const std::wstring& url = L"");

#endif
