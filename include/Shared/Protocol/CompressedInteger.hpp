// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_NETWORK_COMPRESSEDINTEGER_HPP
#define BURGWAR_SHARED_NETWORK_COMPRESSEDINTEGER_HPP

#include <Nazara/Core/Algorithm.hpp>
#include <type_traits>

namespace bw
{
	template<typename T>
	class CompressedSigned
	{
		static_assert(std::is_signed_v<T>);

		public:
			explicit CompressedSigned(T value = 0);
			~CompressedSigned() = default;

			operator T() const;

			CompressedSigned& operator=(T value);

		private:
			T m_value;
	};

	template<typename T>
	class CompressedUnsigned
	{
		static_assert(std::is_unsigned_v<T>);

		public:
			explicit CompressedUnsigned(T value = 0);
			~CompressedUnsigned() = default;

			operator T() const;

			CompressedUnsigned& operator=(T value);

		private:
			T m_value;
	};
}

namespace Nz
{
	template<typename T> bool Serialize(SerializationContext& context, bw::CompressedSigned<T> value, TypeTag<bw::CompressedSigned<T>>);
	template<typename T> bool Serialize(SerializationContext& context, bw::CompressedUnsigned<T> value, TypeTag<bw::CompressedUnsigned<T>>);
	template<typename T> bool Unserialize(SerializationContext& context, bw::CompressedSigned<T>* value, TypeTag<bw::CompressedSigned<T>>);
	template<typename T> bool Unserialize(SerializationContext& context, bw::CompressedUnsigned<T>* value, TypeTag<bw::CompressedUnsigned<T>>);
}

#include <Shared/Protocol/CompressedInteger.inl>

#endif
