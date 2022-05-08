// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_ANIMATIONSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_ANIMATIONSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <vector>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API AnimationSystem
	{
		public:
			AnimationSystem(SharedMatch& match);
			~AnimationSystem() = default;

		private:
			void OnUpdate(float elapsedTime);

			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/AnimationSystem.inl>

#endif
