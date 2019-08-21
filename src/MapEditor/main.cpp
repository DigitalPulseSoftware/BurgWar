#include <QtWidgets/QApplication>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Application.hpp>

int main(int argc, char *argv[])
{
	Ndk::Application ndkApp;
	QApplication app(argc, argv);
	app.setOrganizationName("DigitalPulseSoftware");
	app.setApplicationName("Burg'war map editor");
	app.setApplicationDisplayName("Burg'War Map Editor");

	bw::EditorWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
