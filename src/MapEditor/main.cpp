#include <QtWidgets/QApplication>
#include <CoreLib/Utility/CrashHandler.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Network/Network.hpp>
#include <NDK/Application.hpp>

int main(int argc, char *argv[])
{
	bw::CrashHandler crashHandler;
	crashHandler.Install();

	QApplication app(argc, argv);
	app.setOrganizationName("DigitalPulseSoftware");
	app.setApplicationName("Burgwar_mapeditor");
	app.setApplicationDisplayName("Burg'War Map Editor");

	Nz::Initializer<Nz::Network> netInit;

	bw::EditorWindow mainWindow(argc, argv);
	mainWindow.show();

	return app.exec();
}
