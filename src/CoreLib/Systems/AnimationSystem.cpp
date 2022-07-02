// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>

namespace bw
{
	void AnimationSystem::Update(float /*elapsedTime*/)
	{
		Nz::UInt64 now = m_match.GetCurrentTime();

		auto view = m_registry.view<AnimationComponent>();
		for (entt::entity entity : view)
		{
			auto& animComponent = view.get<AnimationComponent>(entity);
			animComponent.Update(now);
		}
	}
}
