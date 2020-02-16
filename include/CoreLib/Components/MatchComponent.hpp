// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP

#include <CoreLib/LayerIndex.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class Match;

	class MatchComponent : public Ndk::Component<MatchComponent>
	{
		public:
			inline MatchComponent(Match& match, LayerIndex layerIndex, Nz::Int64 uniqueId);
			~MatchComponent() = default;

			inline LayerIndex GetLayerIndex() const;
			inline Match& GetMatch() const;
			inline Nz::Int64 GetUniqueId() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Int64 m_uniqueId;
			Match& m_match;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/Components/MatchComponent.inl>

#endif
