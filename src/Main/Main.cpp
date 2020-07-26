// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Main/Main.hpp>
#include <CoreLib/Utility/CrashHandler.hpp>
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
	bw::CrashHandler crashHandler;
	crashHandler.Install();

	try
	{
		return BurgMain(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Unhandled exception: " << e.what() << std::endl;
		throw;
	}
	catch (...)
	{
		std::cerr << "Unhandled non-standard exception" << std::endl;
		throw;
	}
}

//TODO: Handle WinMain
