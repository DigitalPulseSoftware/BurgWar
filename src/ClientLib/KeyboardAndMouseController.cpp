// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/KeyboardAndMouseController.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Platform/Mouse.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	KeyboardAndMouseController::KeyboardAndMouseController(Nz::Window& window, Nz::UInt8 localPlayerIndex) :
	m_window(window),
	m_localPlayerIndex(localPlayerIndex)
	{
		m_window.GetEventHandler().OnMouseWheelMoved.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseWheelEvent& event)
		{
			OnSwitchWeapon(this, event.delta > 0.f);
		});
	}

	PlayerInputData KeyboardAndMouseController::Poll(LocalMatch& localMatch, const LocalLayerEntityHandle& controlledEntity)
	{
		PlayerInputData inputData;

		switch (m_localPlayerIndex)
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

		inputData.aimDirection = Nz::Vector2f::Zero();

		if (controlledEntity)
		{
			const auto& entity = controlledEntity->GetEntity();
			
			Nz::Vector2i mousePosition = Nz::Mouse::GetPosition(m_window);

			const Ndk::EntityHandle& cameraEntity = localMatch.GetCameraEntity();
			if (cameraEntity)
			{
				auto& cameraComponent = cameraEntity->GetComponent<Ndk::CameraComponent>();

				Nz::Vector2f originPosition = controlledEntity->GetPosition();
				if (const auto& weaponEntity = controlledEntity->GetWeaponEntity())
				{
					auto& weaponScript = weaponEntity->GetEntity()->GetComponent<ScriptComponent>();
					const auto& weaponElement = static_cast<const ScriptedWeapon&>(*weaponScript.GetElement());
					originPosition += weaponElement.weaponOffset;
				}
				else if (entity->HasComponent<Ndk::GraphicsComponent>())
					originPosition = Nz::Vector2f(entity->GetComponent<Ndk::GraphicsComponent>().GetAABB().GetCenter());

				Nz::Vector3f worldPosition = cameraComponent.Unproject(Nz::Vector3f(float(mousePosition.x), float(mousePosition.y), 0.f));
				inputData.aimDirection = Nz::Vector2f::Normalize(Nz::Vector2f(worldPosition) - originPosition);
			}
		}
		
		return inputData;
	}
}
