// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/InputData.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <cassert>

namespace bw
{
	inline SharedMatch& SharedScriptingLibrary::GetSharedMatch()
	{
		return m_match;
	}
}
