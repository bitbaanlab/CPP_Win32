#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <Shlwapi.h>
#include "..\CPPLib\Header.h"

//clear screen:
void cls() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;
	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
	FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
	SetConsoleCursorPosition(console, topLeft);
}

void main()
{
	std::cout << " ____  _ _   ____                      __  __    _    _          _     " << std::endl;
	std::cout << "| __ )(_) |_| __ )  __ _  __ _ _ __   |  \\/  |  / \\  | |    __ _| |__  " << std::endl;
	std::cout << "|  _ \\| | __|  _ \\ / _` |/ _` | '_ \\  | |\\/| | / _ \\ | |   / _` | '_ \\ " << std::endl;
	std::cout << "| |_) | | |_| |_) | (_| | (_| | | | | | |  | |/ ___ \\| |__| (_| | |_) |" << std::endl;
	std::cout << "|____/|_|\\__|____/ \\__,_|\\__,_|_| |_| |_|  |_/_/   \\_\\_____\\__,_|_.__/ \n" << std::endl;

	std::string email = "", password = "", serveraddress = "", file_path="";
	std::cout << "Please insert API server address [Default=https://apimalab.bitbaan.com]: ";
	std::getline(std::cin, serveraddress);
	if (serveraddress.length() == 0)
		serveraddress = "https://apimalab.bitbaan.com";
	std::cout << "Please insert email address: ";
	std::getline(std::cin,email);
	std::cout << "Please insert your password: ";
	std::getline(std::cin, password);
	MALabLib* malab = new MALabLib(serveraddress);
	json::JSON input_json;
	input_json["email"] = email;
	input_json["password"] = password;
	json::JSON returnValue = malab->call_with_json_input("user/login", input_json);
	if (returnValue["success"].ToBool() == true)
		std::cout << "You are logged in successfully." << std::endl;
	else
	{
		std::cout << malab->get_error(returnValue);
		_getch();
		return;
	}
	std::string apikey = returnValue["apikey"].ToString();
	std::cout << "please enter the path of file to scan: ";
	std::getline(std::cin, file_path);
	std::string file_name = PathFindFileNameA(file_path.c_str());
	input_json["file_name"] = file_name;
	input_json["apikey"] = apikey;
	returnValue = malab->call_with_form_input("file/scan", input_json, "file_data", file_path);
	if (returnValue["success"].ToBool() == true)
	{
		//getting scan results:
		bool is_finished = false;
		std::string file_hash = malab->get_sha256(file_path);
		DWORD scan_id = returnValue["scan_id"].ToInt();
		while (is_finished == false) {
			std::cout << "waiting for getting results...";
			input_json["hash"] = file_hash;
			input_json["apikey"] = apikey;
			returnValue = malab->call_with_json_input("file/scan/result/get", input_json);
			if (returnValue["success"].ToBool() == false) {
				std::cout << malab->get_error(returnValue);
				return;
			}
			cls(); //clear screen
			for (int i = 0; i< returnValue["scan"]["results"].size(); i++)
			{
				json::JSON current_av_result = returnValue["scan"]["results"][i];
				if (current_av_result["result"].ToString() == "malware") // file is malware
					std::cout << current_av_result["av_name"].ToString() << " ==> " << current_av_result["malware_name"].ToString() << std::endl;
				else if(current_av_result["result"].ToString() == "clean") // file is clean
					std::cout << current_av_result["av_name"].ToString() << " ==> " << "clean" << std::endl;
			}
			is_finished = returnValue["scan"]["is_finished"].ToBool();
			Sleep(2000);
		}	
	}
	else
	{
		std::cout << malab->get_error(returnValue);
		_getch();
		return;
	}
	_getch();
}