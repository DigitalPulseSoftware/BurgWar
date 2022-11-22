// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerInputController.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> PlayerInputController::GenerateInputs(entt::handle entity) const
	{
		PlayerControlledComponent* playerControlled = entity.try_get<PlayerControlledComponent>();
		if (!playerControlled)
			return std::nullopt;

		Player* player = playerControlled->GetOwner();
		if (!player)
			// TODO: Log error
			return PlayerInputData{};

		return player->GetInputs();
	}
}
