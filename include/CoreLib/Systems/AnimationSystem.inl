// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/AnimationSystem.hpp>

namespace bw
{
	inline AnimationSystem::AnimationSystem(entt::registry& registry, SharedMatch& match) :
	m_registry(registry),
	m_match(match)
	{
	}
}
