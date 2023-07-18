// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Core/AppFilesystemComponent.hpp>
#include <Nazara/Core/Application.hpp>
#include <Nazara/Core/SignalHandlerAppComponent.hpp>
#include <Nazara/Network/Network.hpp>
#include <Nazara/Utility/Utility.hpp>
#include <Server/ServerAppComponent.hpp>
#include <Main/Main.hpp>

int BurgWarServer(int argc, char* argv[])
{
	Nz::Application<Nz::Core, Nz::Network, Nz::Utility> app(argc, argv);
	app.AddComponent<Nz::AppFilesystemComponent>();
	app.AddComponent<Nz::SignalHandlerAppComponent>();
	
	app.AddComponent<bw::ServerAppComponent>();

	return app.Run();
}

BurgWarMain(BurgWarServer)
