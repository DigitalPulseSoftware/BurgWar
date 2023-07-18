// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOWPREFABS_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_EDITORWINDOWPREFABS_HPP

#include <QtCore/QObject>

class QMenu;
class QMenuBar;

namespace bw
{
	class EditorWindow;

	class EditorWindowPrefabs : public QObject
	{
		friend EditorWindow;

		public:
			inline EditorWindowPrefabs(EditorWindow* parent);
			~EditorWindowPrefabs() = default;

		private:
			void BuildMenu(QMenuBar* menuBar);
			void OnCreatePrefab();
			void OnLoadPrefab();

			EditorWindow* m_parent;
			QMenu* m_prefabMenu;
	};
}

#include <MapEditor/Widgets/EditorWindowPrefabs.inl>

#endif
