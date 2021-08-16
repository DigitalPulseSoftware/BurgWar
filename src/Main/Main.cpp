// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Main/Main.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/Utility/CrashHandler.hpp>
#include <fmt/format.h>
#include <exception>

int BurgMain(int argc, char* argv[], int(*mainFunc)(int argc, char* argv[]))
{
	fmt::print("BurgWar {0}.{1}.{2} {3} ({4}) - {5}\n", bw::GameMajorVersion, bw::GameMinorVersion, bw::GamePatchVersion, bw::BuildBranch, bw::BuildCommit, bw::BuildDate);

	std::unique_ptr<bw::CrashHandler> crashHandler = bw::CrashHandler::PlatformCrashHandler();
	crashHandler->Install();

	try
	{
		return mainFunc(argc, argv);
	}
	catch (const std::exception& e)
	{
		fmt::print(stderr, "unhandled exception: {0}\n", e.what());
		throw;
	}
	catch (...)
	{
		fmt::print(stderr, "unhandled non-standard exception\n");
		throw;
	}
}

//TODO: Handle WinMain
