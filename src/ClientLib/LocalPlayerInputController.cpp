// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalPlayerInputController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LocalPlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> LocalPlayerInputController::GenerateInputs(const Ndk::EntityHandle& entity) const
	{
		assert(entity);
		if (!entity->HasComponent<LocalPlayerControlledComponent>())
			return std::nullopt; //< Don't override inputs (for non-local player controlled entities)

		LocalPlayerControlledComponent& controlledComponent = entity->GetComponent<LocalPlayerControlledComponent>();
		LocalMatch& localMatch = controlledComponent.GetLocalMatch();
		Nz::UInt8 localPlayerIndex = controlledComponent.GetLocalPlayerIndex();

		return localMatch.GetLocalPlayerInputs(localPlayerIndex);
	}
}
