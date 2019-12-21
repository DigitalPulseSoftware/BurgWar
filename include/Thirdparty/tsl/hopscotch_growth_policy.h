// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

// This file is used to disable some warnings in external dependencies if required

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning( disable : 4127) // conditional expression is constant
#endif

#include <tsl/hopscotch_growth_policy.h>

#ifdef _MSC_VER
	#pragma warning( pop ) 
#endif