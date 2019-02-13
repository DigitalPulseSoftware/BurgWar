// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP

#include <NDK/Prerequisites.hpp>
#include <CoreLib/Map.hpp>
#include <QtWidgets/QMainWindow>
#include <hopscotch/hopscotch_map.h>
#include <filesystem>
#include <optional>

class QAction;
class QListWidget;
class QListWidgetItem;

namespace bw
{
	class MapCanvas;

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
			void OnEntitySelectionUpdate();
			void OnLayerChanged(int layerIndex);
			void OnLayerDoubleClicked(QListWidgetItem* item);

			void OnOpenMap();
			void OnSaveMap();

			void RegisterEntity(std::size_t entityIndex);

			std::filesystem::path m_workingMapPath;
			std::optional<int> m_currentLayer;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndexes;
			QAction* m_compileMap;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QListWidget* m_entityList;
			QListWidget* m_layerList;
			Map m_workingMap;
			MapCanvas* m_canvas;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif