// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebRequest.hpp>

namespace bw
{
	inline void WebRequest::SetCallback(Callback callback)
	{
		m_callback = std::move(callback);
	}
	
	inline void WebRequest::SetHeader(std::string header, std::string value)
	{
		m_headers.insert_or_assign(std::move(header), std::move(value));
	}
	
	inline void WebRequest::AppendBodyResponse(const char* data, std::size_t length)
	{
		m_responseBody.append(data, length);
	}

	inline void WebRequest::TriggerCallback(unsigned int code)
	{
		m_callback(code, std::move(m_responseBody));
	}

	inline void WebRequest::TriggerCallback(std::string errorMessage)
	{
		m_callback({}, std::move(errorMessage));
	}
}
