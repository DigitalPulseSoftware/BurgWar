// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalMatch.hpp>

namespace bw
{
	inline AnimationManager& bw::LocalMatch::GetAnimationManager()
	{
		return m_animationManager;
	}
}