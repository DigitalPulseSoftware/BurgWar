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
#include <MapEditor/Enums.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <MapEditor/Widgets/EditorWindowPrefabs.hpp>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QUndoStack>
#include <tsl/hopscotch_map.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

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

			Map::Entity& CreateEntity(LayerIndex layerIndex, std::size_t entityIndex, Map::Entity entityData);
			Map::Layer& CreateLayer(LayerIndex layerIndex, Map::Layer layerData);
			Map::Entity DeleteEntity(LayerIndex layerIndex, std::size_t entityIndex);
			Map::Layer DeleteLayer(LayerIndex layerIndex);

			Nz::Vector2f GetCameraCenter() const;
			inline const std::optional<LayerIndex>& GetCurrentLayer() const;

			inline std::size_t GetEntityIndex(Ndk::EntityId entityId) const;
			inline const EditorEntityStore& GetEntityStore() const;

			inline MapCanvas* GetMapCanvas();
			inline const MapCanvas* GetMapCanvas() const;

			inline const std::vector<std::size_t>& GetSelectedEntities() const;

			inline Map& GetWorkingMapMut();
			inline const Map& GetWorkingMap() const;

			void MoveEntity(LayerIndex layerIndex, std::size_t entityIndex, LayerIndex targetLayer, std::size_t targetEntityIndex);

			void OpenEntityContextMenu(std::optional<std::size_t> entityIndexOpt, const QPoint& pos, QWidget* parent = nullptr);

			void PushCommand(QUndoCommand* command);
			template<typename T, typename... Args> void PushCommand(Args&&... args);
			void RefreshEntityPositionAndRotation(LayerIndex layerIndex, std::size_t entityIndex);

			void SelectEntity(std::size_t entityIndex, bool clearPrevious = true);
			void SelectEntities(const std::vector<std::size_t>& entityIndices);

			void SwapEntities(LayerIndex layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex);
			void SwapLayers(LayerIndex firstLayerIndex, LayerIndex secondLayerIndex);
			void SwitchToMode(std::shared_ptr<EditorMode> editorMode);

			void ToggleEntitySelection(std::size_t entityIndex);

			void UpdateEntity(LayerIndex layerIndex, std::size_t entityIndex, Map::Entity entityData, EntityInfoUpdateFlags updateFlags);
			void UpdateWorkingMap(Map map, std::filesystem::path mapPath = std::filesystem::path());

			NazaraSignal(OnLayerAlignmentUpdate, EditorWindow* /*emitter*/, LayerIndex /*layerIndex*/, const Nz::Vector2f& /*newAlignment*/);

		private:
			void closeEvent(QCloseEvent* event) override;
			bool event(QEvent* e) override;

			void AddToRecentFileList(const QString& mapFolder);
			void AlignLayerEntities(LayerIndex layerIndex);

			void BuildAssetList();
			void BuildEntityList(const std::string& editorAssetsFolder);
			void BuildLayerList(const std::string& editorAssetsFolder);
			void BuildMenu();
			void BuildToolbar(const std::string& editorAssetsFolder);

			bool CanCloseMap();

			EntityInfoDialog* GetEntityInfoDialog();

			void InvalidateMap();

			void OnAlignEntities();
			void OnCloneEntity(std::size_t entityIndex);
			void OnCloneEntity(std::size_t entityIndex, LayerIndex layerIndex);
			void OnCloneLayer(LayerIndex layerIndex);
			void OnCloseMap();
			void OnCompileMap();
			void OnCreateEntity();
			void OnCreateMap();
			void OnCreateLayer();
			bool OnDeleteEntities();
			bool OnDeleteEntities(const std::size_t* entityIndices, std::size_t entityCount);
			void OnDeleteLayer(LayerIndex layerIndex);
			void OnEditEntity(std::size_t entityIndex);
			void OnEditLayer(LayerIndex layerIndex);
			void OnEntityMovedDown();
			void OnEntityMovedUp();
			void OnEntitySelectionUpdate();
			void OnLayerChanged(int layerIndex);
			void OnLayerMovedDown();
			void OnLayerMovedUp();
			void OnMoveEntity(std::size_t entityIndex, LayerIndex targetLayer);
			void OnOpenMap();
			void OnOpenRecentMap();
			void OnPerspectiveSwitch(bool enable);
			void OnPlayMap();
			void OnSaveMap();
			void OnSetAlignment();

			void OpenMap(const QString& mapFolder);

			void RefreshLayerList();
			void RefreshRecentFileListMenu();
			void RefreshRecentFileListMenu(const QStringList& recentFileList);

			void RegisterEntity(std::size_t entityIndex);

			void ReloadScripts();

			bool SaveMap();

			struct List
			{
				QListWidget* listWidget;
				QPushButton* upArrowButton;
				QPushButton* downArrowButton;
			};

			std::filesystem::path m_workingMapPath;
			std::optional<ClientAssetStore> m_assetStore;
			std::optional<EditorEntityStore> m_entityStore;
			std::optional<LayerIndex> m_currentLayer;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<EditorMode> m_currentMode;
			std::shared_ptr<VirtualDirectory> m_assetFolder;
			std::shared_ptr<VirtualDirectory> m_scriptFolder;
			std::vector<QAction*> m_recentMapActions;
			std::vector<std::size_t> m_selectedEntities;
			tsl::hopscotch_map<Ndk::EntityId /*canvasIndex*/, std::size_t /*entityIndex*/> m_entityIndices;
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
			MapCanvas* m_canvas;
			PlayWindow* m_playWindow;
			EditorAppConfig m_configFile;
			EditorWindowPrefabs m_prefabs;
			Map m_workingMap;
			bool m_mapDirtyFlag;
	};
}

#include <MapEditor/Widgets/EditorWindow.inl>

#endif
