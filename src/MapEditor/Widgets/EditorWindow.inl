// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>
#include <cassert>

namespace bw
{
	inline const std::optional<std::size_t>& EditorWindow::GetCurrentLayer() const
	{
		return m_currentLayer;
	}

	inline std::size_t EditorWindow::GetEntityIndex(Ndk::EntityId entityId) const
	{
		auto it = m_entityIndexes.find(entityId);
		assert(it != m_entityIndexes.end());

		return it.value();
	}

	inline const EditorEntityStore& EditorWindow::GetEntityStore() const
	{
		return *m_entityStore;
	}

	inline MapCanvas* EditorWindow::GetMapCanvas()
	{
		return m_canvas;
	}

	inline const MapCanvas* EditorWindow::GetMapCanvas() const
	{
		return m_canvas;
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
		return PushCommand(new T(std::forward<Args>(args)...));
	}

	template<typename T>
	void EditorWindow::ForeachEntityProperty(PropertyType type, T&& func)
	{
		m_workingMap.ForeachEntity([&](Map::Entity& entity)
		{
			if (std::size_t entityIndex = m_entityStore->GetElementIndex(entity.entityType); entityIndex != m_entityStore->InvalidIndex)
			{
				const auto& entityTypeInfo = m_entityStore->GetElement(entityIndex);

				for (auto propertyIt = entity.properties.begin(); propertyIt != entity.properties.end(); ++propertyIt)
				{
					if (auto it = entityTypeInfo->properties.find(propertyIt.key()); it != entityTypeInfo->properties.end())
					{
						const auto& propertyData = it->second;
						if (propertyData.type == type)
							func(entity, *entityTypeInfo, propertyData, propertyIt.value());
					}
				}
			}
			else
				bwLog(GetLogger(), LogLevel::Error, "Unknown entity type: {0}", entity.entityType);
		});
	}

	inline void EditorWindow::InvalidateMap()
	{
		m_mapDirtyFlag = true;
	}
}
