// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/InputSystem.hpp>
#include <CoreLib/Components/InputComponent.hpp>

namespace bw
{
	void InputSystem::Update(float /*elapsedTime*/)
	{
		auto view = m_registry.view<InputComponent>();
		for (auto [entity, inputComponent] : view.each())
		{
			if (const auto& controller = inputComponent.GetController())
			{
				if (auto inputOpt = controller->GenerateInputs(entt::handle(m_registry, entity)))
					inputComponent.UpdateInputs(*inputOpt);
			}
		}
	}
}
