// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerInputController.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> PlayerInputController::GenerateInputs(const Ndk::EntityHandle& entity) const
	{
		assert(entity);
		if (!entity->HasComponent<PlayerControlledComponent>())
			return std::nullopt;

		Player* player = entity->GetComponent<PlayerControlledComponent>().GetOwner();
		if (!player)
			// TODO: Log error
			return PlayerInputData{};

		return player->GetInputs();
	}
}
