// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP

#include <Common/Map.hpp>
#include <QtWidgets/QMainWindow>
#include <filesystem>

class QAction;
class QListWidget;
class QListWidgetItem;

namespace bw
{
	class MapWidget;

	class EditorWindow : public QMainWindow
	{
		public:
			EditorWindow();
			~EditorWindow() = default;

			void ClearWorkingMap();
			void UpdateWorkingMap(Map map, std::filesystem::path mapPath = std::filesystem::path());

		private:
			void BuildMenu();
			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnEntityDoubleClicked(QListWidgetItem* item);
			void OnEntitySelected(QListWidgetItem* item);
			void OnLayerChanged(int layerIndex);
			void OnOpenMap();
			void OnSaveMap();

			std::filesystem::path m_workingMapPath;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QListWidget* m_entityList;
			QListWidget* m_layerList;
			Map m_workingMap;
			MapWidget* m_canvas;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif