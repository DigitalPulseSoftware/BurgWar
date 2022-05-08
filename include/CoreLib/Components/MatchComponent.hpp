// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>

namespace bw
{
	class Match;

	class BURGWAR_CORELIB_API MatchComponent
	{
		public:
			inline MatchComponent(Match& match, LayerIndex layerIndex, EntityId uniqueId);
			~MatchComponent() = default;

			inline LayerIndex GetLayerIndex() const;
			inline Match& GetMatch() const;
			inline EntityId GetUniqueId() const;

		private:
			EntityId m_uniqueId;
			Match& m_match;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/Components/MatchComponent.inl>

#endif
