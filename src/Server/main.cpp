// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Network/Network.hpp>
#include <Server/ServerApp.hpp>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
	Nz::Initializer<Nz::Network> network;
	bw::ServerApp app(argc, argv);

	return app.Run();
}
