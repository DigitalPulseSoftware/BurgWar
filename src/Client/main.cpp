// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Network/Network.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Client/BurgApp.hpp>
#include <Client/ServerConnection.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	Nz::Initializer<Nz::Network> network;
	bw::BurgApp app(argc, argv);

	/*bw::ServerConnection server(app, app.GetCommandStore());
	if (!server.Connect("localhost"))
	{
		std::cerr << "Failed to connect" << std::endl;
		std::getchar();
		return EXIT_FAILURE;
	}

	server.OnConnected.Connect([](bw::ServerConnection* server, Nz::UInt32)
	{
		bw::Packets::HelloWorld hello;
		hello.str = "world";

		server->SendPacket(hello);
	});

	server.OnHelloWorld.Connect([](bw::ServerConnection* server, const bw::Packets::HelloWorld& helloWorld)
	{
		std::cout << "Server answers " << helloWorld.str << std::endl;
	});*/

	return app.Run();
}
