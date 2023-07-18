// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Core/Application.hpp>
#include <Nazara/Core/AppEntitySystemComponent.hpp>
#include <Nazara/Core/AppFilesystemComponent.hpp>
#include <Nazara/Graphics/Graphics.hpp>
#include <Nazara/Platform/AppWindowingComponent.hpp>
#include <Nazara/Network/Network.hpp>
#include <Nazara/Widgets/Widgets.hpp>
#include <Client/ClientAppComponent.hpp>
#include <Main/Main.hpp>

int BurgWarGame(int argc, char* argv[])
{
	Nz::Renderer::Config config;
	config.preferredAPI = Nz::RenderAPI::OpenGL;

	Nz::Application<Nz::Audio, Nz::Graphics, Nz::Network, Nz::Widgets> app(argc, argv, config);
	app.AddComponent<Nz::AppEntitySystemComponent>();
	app.AddComponent<Nz::AppFilesystemComponent>();
	app.AddComponent<Nz::AppWindowingComponent>();
	app.AddComponent<bw::ClientAppComponent>(argc, argv);
	//app.EnableFPSCounter(true);

	//Nz::Audio::SetSpeedOfSound(343.3f * 100.f);

	/*Nz::SoundBufferParams params;
	params.forceMono = true;

	Nz::SoundBufferManager::SetDefaultParameters(params);*/

	return app.Run();
}

BurgWarMain(BurgWarGame)
