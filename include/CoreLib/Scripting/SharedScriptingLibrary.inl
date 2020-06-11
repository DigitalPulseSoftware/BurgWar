// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Thirdparty/sol3/forward.hpp>
#include <cassert>

namespace bw
{
	inline SharedMatch& SharedScriptingLibrary::GetSharedMatch()
	{
		return m_match;
	}

	class BasicPlayerMovementController;
	class Constraint;
	class DampedSpringConstraint;
	class NoclipPlayerMovementController;
	class PinConstraint;
	class PlayerMovementController;
}

// Constraint
SOL_BASE_CLASSES(bw::PinConstraint, bw::Constraint);
SOL_BASE_CLASSES(bw::DampedSpringConstraint, bw::Constraint);
SOL_DERIVED_CLASSES(bw::Constraint, bw::DampedSpringConstraint, bw::PinConstraint);

// PlayerMovementController
SOL_BASE_CLASSES(bw::BasicPlayerMovementController, bw::PlayerMovementController);
SOL_BASE_CLASSES(bw::NoclipPlayerMovementController, bw::PlayerMovementController);
SOL_DERIVED_CLASSES(bw::PlayerMovementController, bw::BasicPlayerMovementController, bw::NoclipPlayerMovementController);

namespace sol
{
	//template <>
	//struct is_automagical<Ndk::EntityHandle> : std::false_type {};
}