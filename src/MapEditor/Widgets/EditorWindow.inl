// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw
{
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
}
