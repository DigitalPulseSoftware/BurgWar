// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_ANIMATIONSTORE_HPP
#define BURGWAR_SHARED_ANIMATIONSTORE_HPP

#include <hopscotch/hopscotch_map.h>
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

#include <Shared/AnimationStore.inl>

#endif
