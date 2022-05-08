// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTENTITYLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTENTITYLIBRARY_HPP

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientAssetStore;
	class Logger;

	class BURGWAR_CLIENTLIB_API ClientEntityLibrary : public SharedEntityLibrary
	{
		public:
			inline ClientEntityLibrary(const Logger& logger, ClientAssetStore& assetStore);
			~ClientEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			void InitRigidBody(lua_State* L, entt::entity entity, float mass) override;

		private:
			void RegisterClientLibrary(sol::table& elementMetatable);

			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEntityLibrary.inl>

#endif
