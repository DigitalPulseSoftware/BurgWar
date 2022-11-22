// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ANIMATIONMANAGER_HPP
#define BURGWAR_CORELIB_ANIMATIONMANAGER_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class AnimationManager
	{
		public:
			using FinishCallback = std::function<void()>;
			using UpdateCallback = std::function<bool(float ratio)>;

			AnimationManager() = default;
			~AnimationManager() = default;

			inline void PushAnimation(float duration, UpdateCallback update, FinishCallback finish);

			inline void Update(float elapsedTime);

		private:
			struct Animation
			{
				std::function<void()> finishCallback;
				std::function<bool(float ratio)> updateCallback;
				float duration;
				float elapsedtime;
			};

			std::vector<Animation> m_newAnimations;
			std::vector<Animation> m_playingAnimations;
	};
}

#include <CoreLib/AnimationManager.inl>

#endif
