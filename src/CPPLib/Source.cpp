#include "Header.h"

std::string current_server_address = "";
std::string current_apikey = "";

json::JSON __stdcall call_api_with_json_input(std::string api, json::JSON json_input)
{
	boolean use_https = FALSE;
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
		return 900;
	}
	if (success_code == false)
	{
		int retValue = obj["error_code"].ToInt();
		free(data_response);
		InternetCloseHandle(site_connection);
		InternetCloseHandle(http_connection);
		InternetCloseHandle(internet_handle);
		return_json["success"] = false;
		return_json["error_code"] = retValue;
		return return_json;
	}
	InternetCloseHandle(site_connection);
	InternetCloseHandle(http_connection);
	InternetCloseHandle(internet_handle);
	free(data_response);
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