// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebRequest.hpp>

namespace bw
{
	inline void WebRequest::SetDataCallback(DataCallback callback)
	{
		m_dataCallback = std::move(callback);
	}

	inline void WebRequest::SetResultCallback(ResultCallback callback)
	{
		m_resultCallback = std::move(callback);
	}

	inline void WebRequest::SetHeader(std::string header, std::string value)
	{
		m_headers.insert_or_assign(std::move(header), std::move(value));
	}
	
	inline bool WebRequest::OnBodyResponse(const char* data, std::size_t length)
	{
		if (!m_dataCallback)
		{
			m_responseBody.append(data, length);
			return true;
		}

		return m_dataCallback(data, length);
	}

	inline void WebRequest::TriggerCallback()
	{
		m_resultCallback(WebRequestResult(m_curlHandle.Get(), std::move(m_responseBody)));
		m_responseBody.clear();
	}

	inline void WebRequest::TriggerCallback(std::string errorMessage)
	{
		m_resultCallback(WebRequestResult(std::move(errorMessage)));
	}
}
