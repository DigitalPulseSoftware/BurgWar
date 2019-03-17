// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/KeyboardAndMouseController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Platform/Mouse.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	InputData KeyboardAndMouseController::Poll(LocalMatch& localMatch, Nz::UInt8 localPlayerIndex, const Ndk::EntityHandle& controlledEntity)
	{
		InputData inputData;

		switch (localPlayerIndex)
		{
			case 0:
				inputData.isAttacking = Nz::Mouse::IsButtonPressed(Nz::Mouse::Left);
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Space);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Q);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::D);
				break;

			case 1:
				inputData.isAttacking = Nz::Mouse::IsButtonPressed(Nz::Mouse::Right);
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Up);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Left);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Right);
				break;

			default:
				break;
		}

		if (controlledEntity)
		{
			Nz::Vector2i mousePosition = Nz::Mouse::GetPosition(m_window);

			const Ndk::EntityHandle& cameraEntity = localMatch.GetCamera();
			auto& cameraComponent = cameraEntity->GetComponent<Ndk::CameraComponent>();

			Nz::Vector3f worldPosition = cameraComponent.Unproject(Nz::Vector3f(float(mousePosition.x), float(mousePosition.y), 0.f));

			auto& controlledEntityNode = controlledEntity->GetComponent<Ndk::NodeComponent>();

			inputData.aimDirection = Nz::Vector2f::Normalize(Nz::Vector2f(worldPosition - controlledEntityNode.GetPosition()));
		}
		else
			inputData.aimDirection = Nz::Vector2f::Zero();
		
		return inputData;
	}
}
