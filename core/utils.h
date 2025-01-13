#pragma once
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

#define RESET_COLOR "\033[0m"
#define BLUE_COLOR "\033[94m"
#define LIGHT_BLUE_COLOR "\033[34m"
#define YELLOW_COLOR "\033[33m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define WHITE_COLOR "\033[37m"

namespace utils
{
	void enableVirtualTerminalProcessing();

	void delay(int milliseconds);

	void countdownAndExit(int start);

	void debug(const std::string& message, const char* file, int line);
	void warning(const std::string& message, const char* file, int line);
	void error(const std::string& message, const char* file, int line);
	void info(const std::string& message, const char* file, int line);

	void sendToDiscordWebhook(const std::string& webhookUrl, const std::string& message);
}