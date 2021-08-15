// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebRequestResult.hpp>

namespace bw
{
	inline WebRequestResult::WebRequestResult(CURL* curl, std::string body) :
	m_curlHandle(curl),
	m_bodyOrErr(std::move(body))
	{
	}

	inline WebRequestResult::WebRequestResult(std::string errMessage) :
	m_curlHandle(nullptr),
	m_bodyOrErr(std::move(errMessage))
	{
	}

	inline std::string& WebRequestResult::GetBody()
	{
		assert(HasSucceeded());
		return m_bodyOrErr;
	}

	inline const std::string& WebRequestResult::GetBody() const
	{
		assert(HasSucceeded());
		return m_bodyOrErr;
	}

	inline const std::string& WebRequestResult::GetErrorMessage() const
	{
		assert(!HasSucceeded());
		return m_bodyOrErr;
	}

	inline bool WebRequestResult::HasSucceeded() const
	{
		return m_curlHandle != nullptr;
	}

	inline WebRequestResult::operator bool() const
	{
		return HasSucceeded();
	}

}
