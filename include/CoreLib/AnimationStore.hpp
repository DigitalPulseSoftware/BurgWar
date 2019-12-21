// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ANIMATIONSTORE_HPP
#define BURGWAR_CORELIB_ANIMATIONSTORE_HPP

#include <Thirdparty/tsl/hopscotch_map.h>
#include <chrono>
#include <limits>
#include <vector>

namespace bw
{
	class AnimationStore
	{
		public:
			struct AnimationData;

			inline AnimationStore(std::vector<AnimationData> animations);
			~AnimationStore() = default;

			inline std::size_t FindAnimationByName(const std::string& animationName) const;

			inline const AnimationData& GetAnimation(std::size_t animId) const;
			inline std::size_t GetAnimationCount() const;

			struct AnimationData
			{
				std::string animationName;
				std::chrono::milliseconds duration;
			};

			static constexpr std::size_t InvalidId = std::numeric_limits<std::size_t>::max();

		private:
			tsl::hopscotch_map<std::string /*animName*/, std::size_t /*animId*/> m_animationByName;
			std::vector<AnimationData> m_animations;
	};
}

#include <CoreLib/AnimationStore.inl>

#endif
