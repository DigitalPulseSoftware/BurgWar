// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>
#include <CoreLib/Utility/CrashHandler.hpp>
#include <Server/ServerApp.hpp>
#include <filesystem>

int main(int argc, char* argv[])
{
	bw::CrashHandler crashHandler;
	crashHandler.Install();

	Nz::Initializer<Nz::Network> network;
	bw::ServerApp app(argc, argv);

	return app.Run();
}
