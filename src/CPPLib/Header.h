#pragma once
#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include "json.h"

#define EXPORT __declspec (dllexport)

EXPORT json::JSON __stdcall login(std::string server_address, std::string email, std::string password);