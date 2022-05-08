// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/SoundEntity.hpp>

namespace bw
{
	inline entt::entity SoundEntity::GetEntity() const
	{
		return m_entity;
	}
}
