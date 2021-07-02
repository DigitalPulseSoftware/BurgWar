// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/KeyboardAndMousePoller.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <ClientLib/Camera.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Platform/Mouse.hpp>
#include <NDK/Components/GraphicsComponent.hpp>

namespace bw
{
	KeyboardAndMousePoller::KeyboardAndMousePoller(Nz::Window& window, Nz::UInt8 localPlayerIndex) :
	m_window(window),
	m_localPlayerIndex(localPlayerIndex)
	{
		m_onMouseWheelMovedSlot.Connect(m_window.GetEventHandler().OnMouseWheelMoved, [this](const Nz::EventHandler*, const Nz::WindowEvent::MouseWheelEvent& event)
		{
			OnSwitchWeapon(this, event.delta > 0.f);
		});
	}

	PlayerInputData KeyboardAndMousePoller::Poll(LocalMatch& localMatch, const LocalLayerEntityHandle& controlledEntity)
	{
		PlayerInputData inputData;

		switch (m_localPlayerIndex)
		{
			case 0:
				inputData.isAttacking = Nz::Mouse::IsButtonPressed(Nz::Mouse::Left);
				inputData.isCrouching = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::LControl);
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::Space);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Scancode::A);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Scancode::D);
				break;

			case 1:
				inputData.isAttacking = Nz::Mouse::IsButtonPressed(Nz::Mouse::Right);
				inputData.isCrouching = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::Down);
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::Up);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::Left);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::VKey::Right);
				break;

			default:
				break;
		}

		inputData.aimDirection = Nz::Vector2f::Zero();

		if (controlledEntity)
		{
			const auto& entity = controlledEntity->GetEntity();
			
			const Camera& camera = localMatch.GetCamera();

			Nz::Vector2f originPosition = controlledEntity->GetPosition();
			float lookSwitch = originPosition.x;

			if (entity->HasComponent<WeaponWielderComponent>())
			{
				auto& wielderComponent = entity->GetComponent<WeaponWielderComponent>();
				Nz::Vector2f weaponOffset = wielderComponent.GetWeaponOffset();

				if (!controlledEntity->IsFacingRight())
					weaponOffset.x = -weaponOffset.x;

				originPosition += weaponOffset;
			}
			else if (entity->HasComponent<Ndk::GraphicsComponent>())
			{
				originPosition = Nz::Vector2f(entity->GetComponent<Ndk::GraphicsComponent>().GetAABB().GetCenter());
				lookSwitch = originPosition.x;
			}

			Nz::Vector2f mousePosition = Nz::Vector2f(Nz::Mouse::GetPosition(m_window));
			Nz::Vector2f worldPosition = camera.Unproject(mousePosition);
			inputData.aimDirection = Nz::Vector2f::Normalize(Nz::Vector2f(worldPosition) - originPosition);
			inputData.isLookingRight = worldPosition.x >= lookSwitch;
		}
		
		return inputData;
	}
}
