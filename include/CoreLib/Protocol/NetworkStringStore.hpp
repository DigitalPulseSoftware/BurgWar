// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORK_NETWORKSTRINGSTORE_HPP
#define BURGWAR_CORELIB_NETWORK_NETWORKSTRINGSTORE_HPP

#include <CoreLib/Protocol/Packets.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <limits>
#include <optional>
#include <vector>

namespace bw
{
	class NetworkStringStore
	{
		public:
			inline NetworkStringStore();
			~NetworkStringStore() = default;

			Packets::NetworkStrings BuildPacket(Nz::UInt32 firstId = 0) const;

			inline void Clear();

			inline Nz::UInt32 CheckStringIndex(const std::string& string) const;

			void FillStore(Nz::UInt32 firstId, std::vector<std::string> strings);

			inline const std::string& GetString(Nz::UInt32 id) const;
			inline Nz::UInt32 GetStringIndex(const std::string& string) const;

			inline Nz::UInt32 RegisterString(std::string string);

			static constexpr Nz::UInt32 InvalidIndex = std::numeric_limits<Nz::UInt32>::max();

		private:
			tsl::hopscotch_map<std::string, Nz::UInt32> m_stringMap;
			std::vector<std::string> m_strings;
	};
}

#include <CoreLib/Protocol/NetworkStringStore.inl>

#endif
