// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP

#include <NDK/Component.hpp>

namespace bw
{
	class Match;

	class MatchComponent : public Ndk::Component<MatchComponent>
	{
		public:
			inline MatchComponent(Match& match, Nz::UInt16 layerIndex);
			~MatchComponent() = default;

			inline Nz::UInt16 GetLayer() const;
			inline Match& GetMatch() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Match& m_match;
			Nz::UInt16 m_layerIndex;
	};
}

#include <CoreLib/Components/MatchComponent.inl>

#endif
