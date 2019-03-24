#include "Header.h"

std::string current_server_address = "";
std::string current_apikey = "";

EXPORT std::string __stdcall get_sha256(std::string file_path)
{
	std::string out_file_sha256 = "";
	BOOL bResult = FALSE;
	HANDLE hFile = NULL;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbFile[1024];
	DWORD cbRead = 0;
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	CHAR temp_sha256[68] = { 0 };

	hFile = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return "";
	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0))
	{
		CloseHandle(hFile);
		return "";
	}
	if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
	{
		DWORD err = GetLastError();
		CryptReleaseContext(hProv, 0);
		CloseHandle(hFile);
		return "";
	}
	while (bResult = ReadFile(hFile, rgbFile, 1024, &cbRead, NULL))
	{
		if (0 == cbRead)
			break;
		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return "";
		}
	}
	if (!bResult)
	{
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return "";
	}
	cbHash = 64;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)temp_sha256, &cbHash, 0))
	{
		CloseHandle(hFile);
		return "";
	}
	else
	{
		for (DWORD i = 0; i < cbHash; i++)
		{
			out_file_sha256 += (char)rgbDigits[((unsigned char)temp_sha256[i]) >> 4];
			out_file_sha256 += (char)rgbDigits[((unsigned char)temp_sha256[i]) & 0xf];
		}
	}
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);
	return out_file_sha256;
}

json::JSON __stdcall call_api_with_json_input(std::string api, json::JSON json_input)
{
	bool use_https = FALSE;
	json::JSON return_json;
	std::string server_domain = "";
	if (current_server_address.find("http://") == 0)
	{
		server_domain = current_server_address;
		server_domain.erase(0, 7);
	}
	else if (current_server_address.find("https://") == 0)
	{
		use_https = TRUE;
		server_domain = current_server_address;
		server_domain.erase(0, 8);
	}
	else {
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	HINTERNET internet_handle = InternetOpen(TEXT("BitBaan-API-Sample-CPP"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (internet_handle == NULL)
	{
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	HINTERNET http_connection = InternetConnectA(internet_handle, server_domain.c_str(), use_https == TRUE ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_NO_CALLBACK, 0);
	if (http_connection == NULL)
	{
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	int open_request_flags = INTERNET_FLAG_RELOAD;
	if (use_https == TRUE)
		open_request_flags += INTERNET_FLAG_SECURE;
	HINTERNET site_connection = HttpOpenRequestA(http_connection, "POST", ("/" + api).c_str(), NULL, NULL, NULL, open_request_flags, 0);
	if (site_connection == NULL)
	{
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	std::string hdrs = "Content-Type: application/json";
	std::string frmdata = json_input.dump();
	if (HttpSendRequestA(site_connection, hdrs.c_str(), hdrs.length(), (PVOID)frmdata.c_str(), frmdata.length()) == FALSE)
	{
		InternetCloseHandle(site_connection);
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	DWORD readed_data_count = 0;
	DWORD data_available = 0;
	PCHAR data_response = NULL;
	do {
		InternetQueryDataAvailable(site_connection, &data_available, 0, 0);
		readed_data_count = data_available + readed_data_count;
		if (readed_data_count == 0)
		{
			InternetCloseHandle(site_connection);
			InternetCloseHandle(http_connection);
			InternetCloseHandle(internet_handle);
			return_json["success"] = false;
			return_json["error_code"] = 900; //unknown error code
			return return_json;
		}
		if (data_available != 0)
		{
			if (data_response == NULL)
			{
				data_response = (PCHAR)malloc(readed_data_count);
				ZeroMemory(data_response, readed_data_count);
			}
			else
			{
				data_response = (PCHAR)realloc(data_response, readed_data_count);
			}
			DWORD data_read = 0;
			InternetReadFile(site_connection, data_response + readed_data_count - data_available, data_available, &data_read);
			if (data_read == 0)
			{
				InternetCloseHandle(site_connection);
				InternetCloseHandle(http_connection);
				InternetCloseHandle(internet_handle);
				return_json["success"] = false;
				return_json["error_code"] = 900; //unknown error code
				return return_json;
			}
		}
	} while (data_available != 0);
	json::JSON obj;
	obj = json::JSON::Load(data_response);
	bool success_code;
	if (obj.hasKey("success") == true)
		success_code = obj["success"].ToBool();
	else
	{
		free(data_response);
		InternetCloseHandle(site_connection);
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	return obj;
}

json::JSON __stdcall call_api_with_form_input(std::string api, json::JSON data_input, std::string file_param_name, std::string file_path)
{
	bool use_https = FALSE;
	json::JSON return_json;
	std::string server_domain = "";
	if (current_server_address.find("http://") == 0)
	{
		server_domain = current_server_address;
		server_domain.erase(0, 7);
	}
	else if (current_server_address.find("https://") == 0)
	{
		use_https = TRUE;
		server_domain = current_server_address;
		server_domain.erase(0, 8);
	}
	else {
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	HINTERNET internet_handle = InternetOpen(TEXT("BitBaan-API-Sample-CPP"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (internet_handle == NULL)
	{
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	HINTERNET http_connection = InternetConnectA(internet_handle, server_domain.c_str(), use_https == TRUE ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_NO_CALLBACK, 0);
	if (http_connection == NULL)
	{
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	int open_request_flags = INTERNET_FLAG_RELOAD;
	if (use_https == TRUE)
		open_request_flags += INTERNET_FLAG_SECURE;
	HINTERNET site_connection = HttpOpenRequestA(http_connection, "POST", ("/" + api).c_str(), NULL, NULL, NULL, open_request_flags, 0);
	if (site_connection == NULL)
	{
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	std::string boundary = "---------------------------293582696224464";
	std::string hdrs = "Content-Type: multipart/form-data; boundary=" + boundary;
	std::string frmdata_begin = "", frmdata_end = "";
	auto data_all = data_input.ObjectRange();
	for (auto iterator = data_all.begin(), e = data_all.end(); iterator != e; ++iterator)
	{
		frmdata_begin += ("--" + boundary + "\r\nContent-Disposition: form-data; name=\"");
		frmdata_begin += iterator->first;
		frmdata_begin += "\"\r\n\r\n";
		frmdata_begin += iterator->second.ToString() + "\r\n";
	}
	frmdata_begin += ("--" + boundary + "\r\nContent-Disposition: form-data; name=\"" + file_param_name + "\"; filename=\"file\"\r\n\r\n");
	frmdata_end = "\r\n--" + boundary + "--\r\n";
	//read file data:
	HANDLE hFile = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ + FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	DWORD file_size = GetFileSize(hFile, NULL);
	PBYTE file_data = (PBYTE)malloc(file_size);
	ZeroMemory(file_data, file_size);
	DWORD BytesRead = 0;
	ReadFile(hFile, file_data, file_size, &BytesRead, NULL);
	CloseHandle(hFile);
	if (BytesRead != file_size)
	{
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	DWORD data_to_send_size = frmdata_begin.length() + frmdata_end.length() + file_size;
	PBYTE data_to_send = (PBYTE)malloc(data_to_send_size+1);
	ZeroMemory(data_to_send, data_to_send_size + 1);
	memcpy_s(data_to_send, data_to_send_size, frmdata_begin.c_str(), frmdata_begin.length());
	memcpy_s(data_to_send + frmdata_begin.length(), data_to_send_size - frmdata_begin.length(), file_data, file_size);
	memcpy_s(data_to_send + data_to_send_size - frmdata_end.length(), frmdata_end.length(), frmdata_end.c_str(), frmdata_end.length());
	free(file_data);
	if (HttpSendRequestA(site_connection, hdrs.c_str(), hdrs.length(), data_to_send, data_to_send_size) == FALSE)
	{
		free(data_to_send);
		InternetCloseHandle(site_connection);
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = 900; //unknown error code
		return return_json;
	}
	free(data_to_send);
	DWORD readed_data_count = 0;
	DWORD data_available = 0;
	PCHAR data_response = NULL;
	do {
		InternetQueryDataAvailable(site_connection, &data_available, 0, 0);
		readed_data_count = data_available + readed_data_count;
		if (readed_data_count == 0)
		{
			InternetCloseHandle(site_connection);
			InternetCloseHandle(http_connection);
			InternetCloseHandle(internet_handle);
			return_json["success"] = false;
			return_json["error_code"] = 900; //unknown error code
			return return_json;
		}
		if (data_available != 0)
		{
			if (data_response == NULL)
			{
				data_response = (PCHAR)malloc(readed_data_count);
				ZeroMemory(data_response, readed_data_count);
			}
			else
			{
				data_response = (PCHAR)realloc(data_response, readed_data_count);
			}
			DWORD data_read = 0;
			InternetReadFile(site_connection, data_response + readed_data_count - data_available, data_available, &data_read);
			if (data_read == 0)
			{
				InternetCloseHandle(site_connection);
				InternetCloseHandle(http_connection);
				InternetCloseHandle(internet_handle);
				return_json["success"] = false;
				return_json["error_code"] = 900; //unknown error code
				return return_json;
			}
		}
	} while (data_available != 0);
	json::JSON obj;
	obj = json::JSON::Load(data_response);
	bool success_code;
	if (obj.hasKey("success") == true)
		success_code = obj["success"].ToBool();
	else
	{
		free(data_response);
		InternetCloseHandle(site_connection);
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return 900;
	}
	return obj;
}

EXPORT json::JSON __stdcall login(std::string server_address, std::string email, std::string password)
{
	current_server_address = server_address;
	json::JSON json_frmdata;
	json_frmdata["email"] = email;
	json_frmdata["password"] = password;
	json::JSON retValue = call_api_with_json_input("api/v1/user/login", json_frmdata);
	if (retValue["success"].ToBool() == true)
		current_apikey = retValue["apikey"].ToString();
	return retValue;
}

EXPORT json::JSON __stdcall scan(std::string file_path, std::string file_name, bool is_private, std::string file_origin)
{
	json::JSON json_frmdata;
	json_frmdata["filename"] = file_name;
	json_frmdata["apikey"] = current_apikey;
	if (is_private == true)
		json_frmdata["is_private"] = true;
	if (file_origin.length() != 0)
		json_frmdata["fileorigin"] = file_origin;
	return call_api_with_form_input("api/v1/scan", json_frmdata, "filedata", file_path);
}

EXPORT json::JSON __stdcall rescan(std::string file_sha256)
{
	json::JSON json_frmdata;
	json_frmdata["sha256"] = file_sha256;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/rescan", json_frmdata);
}

EXPORT json::JSON __stdcall results(std::string file_sha256, int scan_id)
{
	json::JSON json_frmdata;
	json_frmdata["sha256"] = file_sha256;
	json_frmdata["apikey"] = current_apikey;
	json_frmdata["scan_id"] = scan_id;
	return call_api_with_json_input("api/v1/search/scan/results", json_frmdata);
}

EXPORT json::JSON __stdcall search_by_hash(std::string hash, int ot, int ob, int page, int per_page)
{
	json::JSON json_frmdata;
	json_frmdata["hash"] = hash;
	json_frmdata["apikey"] = current_apikey;
	if(ot != 0)
		json_frmdata["ot"] = ot;
	if (ob != 0)
		json_frmdata["ob"] = ob;
	if (page != 0)
		json_frmdata["page"] = page;
	if (per_page != 0)
		json_frmdata["per_page"] = per_page;
	return call_api_with_json_input("api/v1/search/scan/hash", json_frmdata);
}

EXPORT json::JSON __stdcall search_by_malware_name(std::string malware_name, int ot, int ob, int page, int per_page)
{
	json::JSON json_frmdata;
	json_frmdata["malware_name"] = malware_name;
	json_frmdata["apikey"] = current_apikey;
	if (ot != 0)
		json_frmdata["ot"] = ot;
	if (ob != 0)
		json_frmdata["ob"] = ob;
	if (page != 0)
		json_frmdata["page"] = page;
	if (per_page != 0)
		json_frmdata["per_page"] = per_page;
	return call_api_with_json_input("api/v1/search/scan/malware-name", json_frmdata);
}


EXPORT json::JSON __stdcall download_file(std::string hash_value)
{
	json::JSON json_frmdata;
	json_frmdata["hash"] = hash_value;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/file/download", json_frmdata);
}

EXPORT json::JSON __stdcall get_comments(std::string sha256, int page, int per_page)
{
	json::JSON json_frmdata;
	json_frmdata["sha256"] = sha256;
	json_frmdata["apikey"] = current_apikey;
	if (page != 0)
		json_frmdata["page"] = page;
	if (per_page != 0)
		json_frmdata["per_page"] = per_page;
	return call_api_with_json_input("api/v1/comment", json_frmdata);
}

EXPORT json::JSON __stdcall add_comment(std::string sha256, std::string description)
{
	json::JSON json_frmdata;
	json_frmdata["sha256"] = sha256;
	json_frmdata["description"] = description;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/comment/add", json_frmdata);
}

EXPORT json::JSON __stdcall edit_comment(int comment_id, std::string new_description)
{
	json::JSON json_frmdata;
	json_frmdata["comment_id"] = comment_id;
	json_frmdata["description"] = new_description;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/comment/edit", json_frmdata);
}

EXPORT json::JSON __stdcall delete_comment(int comment_id)
{
	json::JSON json_frmdata;
	json_frmdata["comment_id"] = comment_id;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/comment/delete", json_frmdata);
}

EXPORT json::JSON __stdcall approve_comment(int comment_id)
{
	json::JSON json_frmdata;
	json_frmdata["comment_id"] = comment_id;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/comment/approve", json_frmdata);
}

EXPORT json::JSON __stdcall get_captcha()
{
	json::JSON json_frmdata;
	return call_api_with_json_input("api/v1/captcha", json_frmdata);
}

EXPORT json::JSON __stdcall register_user(std::string first_name, std::string last_name, std::string username, std::string email, std::string password, std::string captcha)
{
	json::JSON json_frmdata;
	json_frmdata["firstname"] = first_name;
	json_frmdata["lastname"] = last_name;
	json_frmdata["username"] = username;
	json_frmdata["email"] = email;
	json_frmdata["password"] = password;
	json_frmdata["captcha"] = captcha;
	return call_api_with_json_input("api/v1/user/register", json_frmdata);
}

EXPORT json::JSON __stdcall get_av_list()
{
	json::JSON json_frmdata;
	json_frmdata["apikey"] = current_apikey;
	return call_api_with_json_input("api/v1/search/av_list", json_frmdata);
}
