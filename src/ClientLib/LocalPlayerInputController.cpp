// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalPlayerInputController.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/LocalPlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> LocalPlayerInputController::GenerateInputs(entt::handle entity) const
	{
		assert(entity);
		LocalPlayerControlledComponent* controlledComponent = entity.try_get<LocalPlayerControlledComponent>();
		if (!controlledComponent)
			return std::nullopt; //< Don't override inputs (for non-local player controlled entities)

		ClientMatch& clientMatch = controlledComponent->GetClientMatch();
		Nz::UInt8 localPlayerIndex = controlledComponent->GetLocalPlayerIndex();

		return clientMatch.GetLocalPlayerInputs(localPlayerIndex);
	}
}
