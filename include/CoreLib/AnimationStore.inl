// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AnimationStore.hpp>
#include <cassert>
#include <stdexcept>

namespace bw
{
	inline AnimationStore::AnimationStore(std::vector<AnimationData> animations)
	{
		for (std::size_t i = 0; i < animations.size(); ++i)
		{
			const auto& animData = animations[i];
			if (m_animationByName.find(animData.animationName) != m_animationByName.end())
				throw std::runtime_error(animData.animationName + " found twice");

			m_animationByName.emplace(animData.animationName, i);
		}

		// Move after the loop in case of exception
		m_animations = std::move(animations);
	}

	inline std::size_t AnimationStore::FindAnimationByName(const std::string& animationName) const
	{
		auto it = m_animationByName.find(animationName);
		if (it != m_animationByName.end())
			return it->second;
		else
			return InvalidId;
	}

	inline auto AnimationStore::GetAnimation(std::size_t animId) const -> const AnimationData&
	{
		assert(animId < m_animations.size());
		return m_animations[animId];
	}

	inline std::size_t AnimationStore::GetAnimationCount() const
	{
		return m_animations.size();
	}
}
