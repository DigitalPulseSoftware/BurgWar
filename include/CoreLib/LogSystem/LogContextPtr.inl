// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/LogContextPtr.hpp>

namespace bw
{
	inline LogContextPtr::LogContextPtr(const Logger* owner, LogContext* context) :
	m_owner(owner),
	m_context(context)
	{
	}

	inline LogContext& LogContextPtr::operator*()
	{
		return *m_context;
	}
	
	inline LogContext* LogContextPtr::operator->()
	{
		return m_context;
	}
}
