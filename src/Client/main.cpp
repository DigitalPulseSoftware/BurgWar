// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Network/Network.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Client/ClientApp.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	Nz::Initializer<Nz::Network> network;
	bw::ClientApp app(argc, argv);
	app.EnableFPSCounter(true);

	return app.Run();
}
