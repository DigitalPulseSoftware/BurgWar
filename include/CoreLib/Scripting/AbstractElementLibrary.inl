// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <cassert>

namespace bw
{
	inline AbstractElementLibrary::AbstractElementLibrary(const Logger& logger) :
	m_logger(logger)
	{
	}

	inline const Logger& AbstractElementLibrary::GetLogger() const
	{
		return m_logger;
	}
}