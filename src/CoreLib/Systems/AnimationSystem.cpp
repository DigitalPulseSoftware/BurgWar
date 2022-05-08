// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>

namespace bw
{
	AnimationSystem::AnimationSystem(SharedMatch& match) :
	m_match(match)
	{
	}

	void AnimationSystem::OnUpdate(float /*elapsedTime*/)
	{
		/*Nz::UInt64 now = m_match.GetCurrentTime();

		for (entt::entity entity : GetEntities())
		{
			auto& animComponent = entity->GetComponent<AnimationComponent>();
			animComponent.Update(now);
		}*/
	}
}
