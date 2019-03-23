#pragma once
#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include "json.h"

#define EXPORT __declspec (dllexport)

EXPORT json::JSON __stdcall login(std::string server_address, std::string email, std::string password);
EXPORT json::JSON __stdcall scan(std::string file_path, std::string file_name, bool is_private = false, std::string file_origin = "");
EXPORT std::string __stdcall get_sha256(std::string file_path);
EXPORT json::JSON __stdcall results(std::string file_sha256, int scan_id);