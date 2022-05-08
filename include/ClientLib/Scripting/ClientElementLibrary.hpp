// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTELEMENTLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTELEMENTLIBRARY_HPP

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientAssetStore;

	class BURGWAR_CLIENTLIB_API ClientElementLibrary : public SharedElementLibrary
	{
		public:
			inline ClientElementLibrary(const Logger& logger, ClientAssetStore& assetStore);
			~ClientElementLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			virtual void RegisterClientLibrary(sol::table& elementTable);
			void SetScale(entt::entity entity, float newScale) override;

		private:
			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientElementLibrary.inl>

#endif
