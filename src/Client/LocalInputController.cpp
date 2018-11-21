// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalInputController.hpp>
#include <Client/ClientApp.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Platform/Mouse.hpp>

namespace bw
{
	InputData LocalInputController::Poll()
	{
		InputData inputData;

		if (m_application.GetMainWindow().HasFocus())
		{
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
		}

		return inputData;
	}
}
