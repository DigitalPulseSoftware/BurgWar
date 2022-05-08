// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerInputController.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> PlayerInputController::GenerateInputs(entt::registry& registry, entt::entity entity) const
	{
		PlayerControlledComponent* playerControlled = registry.try_get<PlayerControlledComponent>(entity);
		if (!playerControlled)
			return std::nullopt;

		Player* player = playerControlled->GetOwner();
		if (!player)
			// TODO: Log error
			return PlayerInputData{};

		return player->GetInputs();
	}
}
