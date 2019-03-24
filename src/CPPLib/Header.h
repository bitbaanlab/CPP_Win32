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
EXPORT json::JSON __stdcall download_file(std::string hash_value);
EXPORT json::JSON __stdcall get_comments(std::string sha256, int page = 0, int per_page = 0);
EXPORT json::JSON __stdcall add_comment(std::string sha256, std::string description);
EXPORT json::JSON __stdcall edit_comment(int comment_id, std::string new_description);
EXPORT json::JSON __stdcall delete_comment(int comment_id);
EXPORT json::JSON __stdcall approve_comment(int comment_id);
EXPORT json::JSON __stdcall get_captcha();
EXPORT json::JSON __stdcall register_user(std::string first_name, std::string last_name, std::string username, std::string email, std::string password, std::string captcha);