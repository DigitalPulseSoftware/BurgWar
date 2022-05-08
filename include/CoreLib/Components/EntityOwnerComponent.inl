// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/EntityOwnerComponent.hpp>

namespace bw
{
	inline void EntityOwnerComponent::Register(entt::entity entity)
	{
		// Try to reuse expired handles before adding an entry
		m_ownedEntities.emplace_back(GetRegistry(), entity);
	}
}
