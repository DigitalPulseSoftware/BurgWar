#include <QtWidgets/QApplication>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Application.hpp>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("DigitalPulseSoftware");
	app.setApplicationName("Burgwar_mapeditor");
	app.setApplicationDisplayName("Burg'War Map Editor");

	bw::EditorWindow mainWindow(argc, argv);
	mainWindow.show();

	return app.exec();
}
