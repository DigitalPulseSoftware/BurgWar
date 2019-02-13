#include <QtWidgets/QApplication>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Application.hpp>

int main(int argc, char *argv[])
{
	Ndk::Application ndkApp;
	QApplication app(argc, argv);

	bw::EditorWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
