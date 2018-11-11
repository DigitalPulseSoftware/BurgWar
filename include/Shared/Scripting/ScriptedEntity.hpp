// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDENTITY_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDENTITY_HPP

#include <Shared/Scripting/ScriptedElement.hpp>

namespace bw
{
	struct ScriptedEntity : ScriptedElement
	{
		bool isNetworked;
	};
}

#endif
