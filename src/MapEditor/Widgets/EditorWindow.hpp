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
#include <ClientLib/ClientEditorApp.hpp>
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
class QTabWidget;

namespace bw
{
	class EntityInfoDialog;
	class EditorMode;
	class MapCanvas;
	class PlayWindow;
	class ScriptingContext;
	class VirtualDirectory;

	class EditorWindow : public ClientEditorApp, public QMainWindow
	{
		public:
			EditorWindow(int argc, char* argv[]);
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
			void BuildToolbar(const std::string& editorAssetsFolder);

			EntityInfoDialog* GetEntityInfoDialog();

			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnCreateLayer();
			void OnDeleteEntity();
			void OnDeleteEntity(std::size_t entityIndex);
			void OnEditEntity(QListWidgetItem* item);
			void OnEditLayer(QListWidgetItem* item);
			void OnEntitySelectionUpdate();
			void OnLayerChanged(int layerIndex);
			void OnPlayMap();

			void OnOpenMap();
			void OnOpenRecentMap();
			void OnSaveMap();

			void OpenMap(const QString& mapFolder);

			void RefreshRecentFileListMenu();
			void RefreshRecentFileListMenu(const QStringList& recentFileList);

			void RegisterEditorConfig();
			void RegisterEntity(std::size_t entityIndex);
			void RefreshLayerList();

			std::filesystem::path m_workingMapPath;
			std::optional<AssetStore> m_assetStore;
			std::optional<EditorEntityStore> m_entityStore;
			std::optional<int> m_currentLayer;
			PlayWindow* m_playWindow;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<EditorMode> m_currentMode;
			std::shared_ptr<VirtualDirectory> m_assetFolder;
			std::shared_ptr<VirtualDirectory> m_scriptFolder;
			std::vector<QAction*> m_recentMapActions;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndexes;
			QAction* m_compileMap;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_playMap;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QMenu* m_mapMenu;
			QListWidget* m_entityList;
			QListWidget* m_layerList;
			QTabWidget* m_centralTab;
			EntityInfoDialog* m_entityInfoDialog;
			Map m_workingMap;
			MapCanvas* m_canvas;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif