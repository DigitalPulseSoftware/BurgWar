#include <QtWidgets/QApplication>
#include <MapEditor/EntityTypeRegistry.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Application.hpp>

int main(int argc, char *argv[])
{
	Ndk::Application ndkApp;
	QApplication app(argc, argv);

	bw::EntityTypeRegistry typeRegistry("../data/entity_types");

	bw::EditorWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
