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
EXPORT json::JSON __stdcall rescan(std::string file_sha256);
EXPORT json::JSON __stdcall search_by_hash(std::string hash, int ot = 0, int ob = 0, int page = 0, int per_page = 0);
EXPORT json::JSON __stdcall search_by_malware_name(std::string malware_name, int ot = 0, int ob = 0, int page = 0, int per_page = 0);