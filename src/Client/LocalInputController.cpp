// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalInputController.hpp>
#include <Nazara/Platform/Keyboard.hpp>

namespace bw
{
	InputData LocalInputController::Poll()
	{
		InputData inputData;

		switch (m_localPlayerIndex)
		{
			case 0:
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Space);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Q);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::D);
				break;

			case 1:
				inputData.isJumping = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Up);
				inputData.isMovingLeft = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Left);
				inputData.isMovingRight = Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Right);
				break;

			default:
				break;
		}

		return inputData;
	}
}
