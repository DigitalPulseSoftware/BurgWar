// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALCLIENT_HPP
#define BURGWAR_CLIENT_LOCALCLIENT_HPP

#include <Shared/SessionBridge.hpp>

namespace bw
{
	class LocalClientSession
	{
		public:
			LocalClientSession() = default;
			~LocalClientSession() = default;

		private:
	};
}

#include <Client/LocalClientSession.inl>

#endif