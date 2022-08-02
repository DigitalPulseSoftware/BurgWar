// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Core/Modules.hpp>
#include <Nazara/Network/Network.hpp>
#include <Nazara/Utility/Utility.hpp>
#include <Server/ServerApp.hpp>
#include <Main/Main.hpp>

int BurgWarServer(int argc, char* argv[])
{
	Nz::Modules<Nz::Core, Nz::Network, Nz::Utility> nazara;
	bw::ServerApp app(argc, argv);

	return app.Run();
}

BurgWarMain(BurgWarServer)
