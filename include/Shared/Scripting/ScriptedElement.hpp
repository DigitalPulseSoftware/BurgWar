// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <string>

namespace bw
{
	struct ScriptedElement
	{
		int tableRef;
		std::string name;
		std::string fullName;
	};
}

#endif
