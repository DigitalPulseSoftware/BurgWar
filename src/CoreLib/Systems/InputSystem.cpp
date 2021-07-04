// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/InputSystem.hpp>
#include <CoreLib/Components/InputComponent.hpp>

namespace bw
{
	InputSystem::InputSystem()
	{
		Requires<InputComponent>();
		SetMaximumUpdateRate(0);
		SetUpdateOrder(-100);
	}

	void InputSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& inputComponent = entity->GetComponent<InputComponent>();
			if (const auto& controller = inputComponent.GetController())
			{
				if (auto inputOpt = controller->GenerateInputs(entity))
					inputComponent.UpdateInputs(*inputOpt);
			}
		}
	}

	Ndk::SystemIndex InputSystem::systemIndex;
}
