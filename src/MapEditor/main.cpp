// Copyright(C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <QtWidgets/QApplication>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Application.hpp>
#include <Main/Main.hpp>

int BurgWarMapEditor(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("DigitalPulseSoftware");
	app.setApplicationName("Burgwar_mapeditor");
	app.setApplicationDisplayName("Burg'War Map Editor");

	Nz::Initializer<Nz::Network> netInit;

	bw::EditorWindow mainWindow(argc, argv);
	mainWindow.show();

	return app.exec();
}

BurgWarMain(BurgWarMapEditor)
