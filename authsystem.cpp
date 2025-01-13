// authsystem.cpp : Defines the entry point for the application.
//

#include "authsystem.h"
#include "core/utils.h"

using namespace std;

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	utils::enableVirtualTerminalProcessing();
	utils::info("Loading Authsystem Generator, please wait...", __FILE__, __LINE__);

	utils::delay(100000);

	return 0;
}
