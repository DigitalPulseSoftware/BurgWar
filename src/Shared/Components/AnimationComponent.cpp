// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/AnimationStore.hpp>

namespace bw
{
	void AnimationComponent::Play(std::size_t animId, Nz::UInt64 animStartTime)
	{
		const auto& animData = m_animationStore->GetAnimation(animId);

		auto& playingAnim = m_playingAnimation.emplace();
		playingAnim.animId = animId;
		playingAnim.endTime = animStartTime + animData.duration.count();
		playingAnim.startTime = animStartTime;

		OnAnimationStart(this);
	}

	void AnimationComponent::Update(Nz::UInt64 now)
	{
		if (!IsPlaying())
			return;

		if (now >= m_playingAnimation->endTime)
		{
			OnAnimationEnd(this);
			m_playingAnimation.reset();
		}
	}

	Ndk::ComponentIndex AnimationComponent::componentIndex;
}
