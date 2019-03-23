#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "..\CPPLib\Header.h"

void main()
{
	std::cout << " ____  _ _   ____                      __  __    _    _          _     " << std::endl;
	std::cout << "| __ )(_) |_| __ )  __ _  __ _ _ __   |  \\/  |  / \\  | |    __ _| |__  " << std::endl;
	std::cout << "|  _ \\| | __|  _ \\ / _` |/ _` | '_ \\  | |\\/| | / _ \\ | |   / _` | '_ \\ " << std::endl;
	std::cout << "| |_) | | |_| |_) | (_| | (_| | | | | | |  | |/ ___ \\| |__| (_| | |_) |" << std::endl;
	std::cout << "|____/|_|\\__|____/ \\__,_|\\__,_|_| |_| |_|  |_/_/   \\_\\_____\\__,_|_.__/ \n" << std::endl;

	std::string email = "", password = "", serveraddress = "";
	CHAR file_path[1024] = { 0 };
	std::cout << "Please insert API server address [Default=https://malab.bitbaan.com]: ";
	std::getline(std::cin, serveraddress);
	if (serveraddress.length() == 0)
		serveraddress = "http://multiavbeta.bitbaan.com";
	std::cout << "Please insert email address: ";
	std::getline(std::cin,email);
	std::cout << "Please insert your password: ";
	std::getline(std::cin, password);
	json::JSON returnValue = login(serveraddress, email, password);
	if (returnValue["success"].ToBool() == true)
		std::cout << "You are logged in successfully." << std::endl;
	else
	{
		std::cout << "error code " << returnValue["error_code"] <<  " occurred.\n";
		_getch();
		return;
	}
}