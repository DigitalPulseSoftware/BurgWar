// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOW_HPP

#include <NDK/Prerequisites.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Map.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientEditorApp.hpp>
#include <MapEditor/EditorAppConfig.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QUndoStack>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <filesystem>
#include <memory>
#include <optional>

class QAction;
class QListWidget;
class QListWidgetItem;
class QPushButton;
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

			void ClearSelectedEntity();
			void ClearWorkingMap();

			Map::Entity& CreateEntity(std::size_t layerIndex, std::size_t entityIndex, Map::Entity entityData);
			Map::Entity DeleteEntity(std::size_t layerIndex, std::size_t entityIndex);

			inline const std::optional<std::size_t>& GetCurrentLayer() const;

			inline std::size_t GetEntityIndex(Ndk::EntityId entityId) const;
			inline const EditorEntityStore& GetEntityStore() const;

			inline MapCanvas* GetMapCanvas();
			inline const MapCanvas* GetMapCanvas() const;

			inline Map& GetWorkingMapMut();
			inline const Map& GetWorkingMap() const;

			void OpenEntityContextMenu(std::optional<std::size_t> entityIndexOpt, const QPoint& pos, QWidget* parent = nullptr);

			void PushCommand(QUndoCommand* command);
			template<typename T, typename... Args> void PushCommand(Args&&... args);
			void RefreshEntityPositionAndRotation(std::size_t layerIndex, std::size_t entityIndex);

			void SelectEntity(Ndk::EntityId entityId);

			void SwitchToMode(std::shared_ptr<EditorMode> editorMode);
			void UpdateWorkingMap(Map map, std::filesystem::path mapPath = std::filesystem::path());

			NazaraSignal(OnLayerAlignmentUpdate, EditorWindow* /*emitter*/, std::size_t /*layerIndex*/, const Nz::Vector2f& /*newAlignment*/);

		private:
			void closeEvent(QCloseEvent* event) override;
			bool event(QEvent* e) override;

			void AddToRecentFileList(const QString& mapFolder);
			void AlignLayerEntities(std::size_t layerIndex);

			void BuildAssetList();
			void BuildEntityList(const std::string& editorAssetsFolder);
			void BuildLayerList(const std::string& editorAssetsFolder);
			void BuildMenu();
			void BuildToolbar(const std::string& editorAssetsFolder);

			bool CanCloseMap();

			template<typename T> void ForeachEntityProperty(PropertyType type, T&& func);

			EntityInfoDialog* GetEntityInfoDialog();

			inline void InvalidateMap();

			void OnAlignEntities();
			void OnCloneEntity(std::size_t entityIndex);
			void OnCloneEntity(std::size_t entityIndex, std::size_t layerIndex);
			void OnCloneLayer(std::size_t layerIndex);
			void OnCloseMap();
			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnCreateLayer();
			bool OnDeleteEntity();
			bool OnDeleteEntity(std::size_t entityIndex);
			void OnDeleteLayer(std::size_t layerIndex);
			void OnEditEntity(std::size_t entityIndex);
			void OnEditLayer(std::size_t layerIndex);
			void OnEntityMovedDown();
			void OnEntityMovedUp();
			void OnEntitySelectionUpdate(int entityIndex);
			void OnLayerChanged(int layerIndex);
			void OnLayerMovedDown();
			void OnLayerMovedUp();
			void OnMoveEntity(std::size_t entityIndex, std::size_t targetLayer);
			void OnOpenMap();
			void OnOpenRecentMap();
			void OnPerspectiveSwitch(bool enable);
			void OnPlayMap();
			void OnSaveMap();
			void OnSetAlignment();

			void OpenMap(const QString& mapFolder);

			void RebuildUniqueIds();
			void RefreshLayerList();
			void RefreshRecentFileListMenu();
			void RefreshRecentFileListMenu(const QStringList& recentFileList);

			void RegisterEntity(std::size_t entityIndex);

			void ReloadScripts();

			bool SaveMap();

			void SwapEntities(std::size_t oldPosition, std::size_t newPosition);
			void SwapLayers(std::size_t oldPosition, std::size_t newPosition);

			struct List
			{
				QListWidget* listWidget;
				QPushButton* upArrowButton;
				QPushButton* downArrowButton;
			};

			std::filesystem::path m_workingMapPath;
			std::optional<ClientAssetStore> m_assetStore;
			std::optional<EditorEntityStore> m_entityStore;
			std::optional<std::size_t> m_currentLayer;
			PlayWindow* m_playWindow;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<EditorMode> m_currentMode;
			std::shared_ptr<VirtualDirectory> m_assetFolder;
			std::shared_ptr<VirtualDirectory> m_scriptFolder;
			std::vector<QAction*> m_recentMapActions;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndexes;
			List m_entityList;
			List m_layerList;
			QAction* m_closeMap;
			QAction* m_compileMap;
			QAction* m_createEntityAction;
			QAction* m_createEntityActionToolbar;
			QAction* m_perspectiveAction;
			QAction* m_playMap;
			QAction* m_saveMap;
			QAction* m_saveMapToolbar;
			QAction* m_showBackgroundColor;
			QAction* m_showColliders;
			QAction* m_showGrid;
			QMenu* m_layerMenu;
			QMenu* m_mapMenu;
			QTabWidget* m_centralTab;
			QUndoStack m_undoStack;
			EntityInfoDialog* m_entityInfoDialog;
			EditorAppConfig m_configFile;
			Map m_workingMap;
			MapCanvas* m_canvas;
			bool m_mapDirtyFlag;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif
