// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Nazara/Platform/Keyboard.hpp>

namespace bw
{
	bool PlayerControlledComponent::IsJumping() const
	{
		return Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Space);
	}

	bool PlayerControlledComponent::IsMovingLeft() const
	{
		return Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Q);
	}

	bool PlayerControlledComponent::IsMovingRight() const
	{
		return Nz::Keyboard::IsKeyPressed(Nz::Keyboard::D);
	}

	Ndk::ComponentIndex PlayerControlledComponent::componentIndex;
}

