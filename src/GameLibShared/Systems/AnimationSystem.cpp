// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Systems/AnimationSystem.hpp>
#include <GameLibShared/BurgApp.hpp>
#include <GameLibShared/Components/AnimationComponent.hpp>

namespace bw
{
	AnimationSystem::AnimationSystem(BurgApp& app) :
	m_application(app)
	{
		Requires<AnimationComponent>();
		SetMaximumUpdateRate(100.f);
	}

	void AnimationSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& animComponent = entity->GetComponent<AnimationComponent>();
			animComponent.Update(m_application.GetAppTime());
		}
	}

	Ndk::SystemIndex AnimationSystem::systemIndex;
}
