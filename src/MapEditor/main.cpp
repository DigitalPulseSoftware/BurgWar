#include <QtWidgets/QApplication>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Application.hpp>
#include <Main/Main.hpp>

int BurgMain(int argc, char* argv[])
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
