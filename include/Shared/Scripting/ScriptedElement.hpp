// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <memory>
#include <string>

namespace bw
{
	struct ScriptedElement : std::enable_shared_from_this<ScriptedElement>
	{
		int tableRef;
		int tickFunction;
		std::string name;
		std::string fullName;
	};
}

#endif
