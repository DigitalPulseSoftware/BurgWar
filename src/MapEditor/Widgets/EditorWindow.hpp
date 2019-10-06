// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP

#include <NDK/Prerequisites.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <QtWidgets/QMainWindow>
#include <tsl/hopscotch_map.h>
#include <filesystem>
#include <memory>
#include <optional>

class QAction;
class QListWidget;
class QListWidgetItem;
class Gamemode;

namespace bw
{
	class EntityInfoDialog;
	class EditorMode;
	class MapCanvas;
	class ScriptingContext;

	class EditorWindow : public BurgApp, public QMainWindow
	{
		public:
			EditorWindow();
			~EditorWindow();

			void ClearWorkingMap();

			inline const EditorEntityStore& GetEntityStore() const;

			inline MapCanvas* GetMapCanvas();
			inline const MapCanvas* GetMapCanvas() const;

			void SelectEntity(Ndk::EntityId entityId);

			void SwitchToMode(std::shared_ptr<EditorMode> editorMode);
			void UpdateWorkingMap(Map map, std::filesystem::path mapPath = std::filesystem::path());

		private:
			virtual bool event(QEvent* e) override;

			void AddToRecentFileList(const QString& mapFolder);

			void BuildAssetList();
			void BuildMenu();

			EntityInfoDialog* GetEntityInfoDialog();

			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnDeleteEntity();
			void OnDeleteEntity(std::size_t entityIndex);
			void OnEntityDoubleClicked(QListWidgetItem* item);
			void OnEntitySelectionUpdate();
			void OnLayerChanged(int layerIndex);
			void OnLayerDoubleClicked(QListWidgetItem* item);

			void OnOpenMap();
			void OnOpenRecentMap();
			void OnSaveMap();

			void OpenMap(const QString& mapFolder);

			void RefreshRecentFileListMenu();
			void RefreshRecentFileListMenu(const QStringList& recentFileList);

			void RegisterEditorConfig();
			void RegisterEntity(std::size_t entityIndex);

			std::filesystem::path m_workingMapPath;
			std::optional<AssetStore> m_assetStore;
			std::optional<EditorEntityStore> m_entityStore;
			std::optional<int> m_currentLayer;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<EditorMode> m_currentMode;
			std::vector<QAction*> m_recentMapActions;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndexes;
			QAction* m_compileMap;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QListWidget* m_entityList;
			QListWidget* m_layerList;
			EntityInfoDialog* m_entityInfoDialog;
			Map m_workingMap;
			MapCanvas* m_canvas;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif