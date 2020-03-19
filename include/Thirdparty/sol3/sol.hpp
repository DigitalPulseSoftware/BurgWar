// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

// This file is used to disable some warnings in external dependencies if required

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning( disable : 4996) // inheriting std::iterator warning
#endif

#include <sol3/sol.hpp>

#ifdef _MSC_VER
	#pragma warning( pop ) 
#endif