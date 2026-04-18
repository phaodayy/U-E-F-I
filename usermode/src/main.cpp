#include <iostream>
#include <thread>
#include <string>

#include <Windows.h>

#include "commands/commands.h"
#include "hook/hook.h"
#include "logs/logs.h"
#include "system/system.h"
#include "hypercall/hypercall.h"

std::int32_t main()
{
	LOG_INFO("Starting Usermode...");
	
	if (!hypercall::init())
	{
		LOG_ERR("Failed to initialize hypervisor connection.");
		std::system("pause");
		return 1;
	}
	logs::set_up();

	LOG_INFO("Setting up system...");
	if (sys::set_up() == 0)
	{
		LOG_ERR("System set_up failed.");
		std::system("pause");

		return 1;
	}

	LOG_INFO("Parsing kernel modules...");
	sys::kernel::parse_modules();

	LOG_INFO("Ready!");
	while (true)
	{
		LOG_PRINTNOLINE("> ");

		std::string command;

		std::getline(std::cin, command);

		if (command == "exit")
		{
			break;
		}

		commands::process(command);

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	sys::clean_up();

	return 0;
}
