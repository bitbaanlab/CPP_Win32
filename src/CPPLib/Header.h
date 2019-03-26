#pragma once
#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include "json.hpp"

#define EXPORT __declspec (dllexport)

EXPORT json::JSON __stdcall malab_login(std::string server_address, std::string email, std::string password);
EXPORT json::JSON __stdcall malab_scan(std::string file_path, std::string file_name, bool is_private = false, std::string file_origin = "");
EXPORT std::string __stdcall malab_get_sha256(std::string file_path);
EXPORT json::JSON __stdcall malab_results(std::string file_sha256, int scan_id);
EXPORT json::JSON __stdcall malab_rescan(std::string file_sha256);
EXPORT json::JSON __stdcall malab_search_by_hash(std::string hash, int ot = 0, int ob = 0, int page = 0, int per_page = 0);
EXPORT json::JSON __stdcall malab_search_by_malware_name(std::string malware_name, int ot = 0, int ob = 0, int page = 0, int per_page = 0);
EXPORT json::JSON __stdcall malab_download_file(std::string hash_value);
EXPORT json::JSON __stdcall malab_get_comments(std::string sha256, int page = 0, int per_page = 0);
EXPORT json::JSON __stdcall malab_add_comment(std::string sha256, std::string description);
EXPORT json::JSON __stdcall malab_edit_comment(int comment_id, std::string new_description);
EXPORT json::JSON __stdcall malab_delete_comment(int comment_id);
EXPORT json::JSON __stdcall malab_approve_comment(int comment_id);
EXPORT json::JSON __stdcall malab_get_captcha();
EXPORT json::JSON __stdcall malab_register_user(std::string first_name, std::string last_name, std::string username, std::string email, std::string password, std::string captcha);
EXPORT json::JSON __stdcall malab_get_av_list();
EXPORT json::JSON __stdcall malab_advanced_search(int scan_id = 0, std::string file_name = "", std::string malware_name = "", std::string hash = "", std::string origin = "", std::string analyzed = "", std::string has_origin = "", int ot = 0, int ob = 0, int page = 0, int per_page = 0);