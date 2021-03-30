// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>
#include <cassert>

namespace bw
{
	inline const std::optional<LayerIndex>& EditorWindow::GetCurrentLayer() const
	{
		return m_currentLayer;
	}

	inline std::size_t EditorWindow::GetEntityIndex(Ndk::EntityId entityId) const
	{
		auto it = m_entityIndices.find(entityId);
		assert(it != m_entityIndices.end());

		return it.value();
	}

	inline MapCanvas* EditorWindow::GetMapCanvas()
	{
		return m_canvas;
	}

	inline const MapCanvas* EditorWindow::GetMapCanvas() const
	{
		return m_canvas;
	}

	inline const std::vector<std::size_t>& EditorWindow::GetSelectedEntities() const
	{
		return m_selectedEntities;
	}

	inline Map& EditorWindow::GetWorkingMapMut()
	{
		assert(m_workingMap.IsValid());

		InvalidateMap();
		return m_workingMap;
	}

	inline const Map& EditorWindow::GetWorkingMap() const
	{
		return m_workingMap;
	}

	template<typename T, typename... Args> 
	void EditorWindow::PushCommand(Args&&... args)
	{
		return PushCommand(new T(*this, std::forward<Args>(args)...));
	}
}
