// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_MATCHCOMPONENT_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Utils/MovablePtr.hpp>

namespace bw
{
	class Match;

	class BURGWAR_CORELIB_API MatchComponent
	{
		public:
			inline MatchComponent(Match& match, LayerIndex layerIndex, EntityId uniqueId);
			MatchComponent(const MatchComponent&) = delete;
			MatchComponent(MatchComponent&&) noexcept = default;
			~MatchComponent() = default;

			inline LayerIndex GetLayerIndex() const;
			inline Match& GetMatch() const;
			inline EntityId GetUniqueId() const;

			MatchComponent& operator=(const MatchComponent&) = delete;
			MatchComponent& operator=(MatchComponent&&) noexcept = default;

		private:
			Nz::MovablePtr<Match> m_match;
			EntityId m_uniqueId;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/Components/MatchComponent.inl>

#endif
