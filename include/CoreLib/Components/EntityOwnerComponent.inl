// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/EntityOwnerComponent.hpp>

namespace bw
{
	inline EntityOwnerComponent::EntityOwnerComponent(const EntityOwnerComponent&)
	{
	}

	inline void EntityOwnerComponent::Register(const Ndk::EntityHandle& entity)
	{
		if (entity)
		{
			// Try to reuse expired handles before adding an entry
			for (auto& ownerHandle : m_ownedEntities)
			{
				if (!ownerHandle)
				{
					ownerHandle = entity;
					return;
				}
			}

			m_ownedEntities.emplace_back(entity);
		}
	}
}
