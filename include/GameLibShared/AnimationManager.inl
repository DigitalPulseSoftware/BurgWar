// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/AnimationManager.hpp>

namespace bw
{
	inline void AnimationManager::PushAnimation(float duration, UpdateCallback update, FinishCallback finish)
	{
		if (!update(0.f))
			return;

		Animation& anim = m_playingAnimations.emplace_back();
		anim.duration = duration;
		anim.elapsedtime = 0.f;
		anim.finishCallback = std::move(finish);
		anim.updateCallback = std::move(update);
	}

	inline void AnimationManager::Update(float elapsedTime)
	{
		// Keep animation count to prevent updating newly pushed animations
		std::size_t animationCount = m_playingAnimations.size();
		for (std::size_t i = 0; i < animationCount;)
		{
			Animation& anim = m_playingAnimations[i];
			anim.elapsedtime += elapsedTime;

			bool isRunning;
			if (anim.elapsedtime >= anim.duration)
			{
				if (anim.updateCallback(1.f))
					anim.finishCallback();

				isRunning = false;
			}
			else
				isRunning = anim.updateCallback(anim.elapsedtime / anim.duration);

			if (isRunning)
				++i;
			else
			{
				m_playingAnimations.erase(m_playingAnimations.begin() + i);
				i--;
				animationCount--;
			}
		}
	}
}
