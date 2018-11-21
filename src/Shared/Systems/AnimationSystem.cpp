// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/AnimationSystem.hpp>
#include <Shared/BurgApp.hpp>
#include <Shared/Components/AnimationComponent.hpp>

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
