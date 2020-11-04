// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>
#include <Server/ServerApp.hpp>
#include <Main/Main.hpp>

int BurgWarServer(int argc, char* argv[])
{
	Nz::Initializer<Nz::Network> network;
	bw::ServerApp app(argc, argv);

	return app.Run();
}

BurgWarMain(BurgWarServer)
