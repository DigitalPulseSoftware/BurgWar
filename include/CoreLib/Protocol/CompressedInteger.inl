// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Protocol/CompressedInteger.hpp>

namespace bw
{
	template<typename T>
	CompressedSigned<T>::CompressedSigned(T value) :
	m_value(value)
	{
	}

	template<typename T>
	CompressedSigned<T>::operator T() const
	{
		return m_value;
	}

	template<typename T>
	CompressedSigned<T>& CompressedSigned<T>::operator=(T value)
	{
		m_value = value;
		return *this;
	}

	template<typename T>
	CompressedSigned<T>& CompressedSigned<T>::operator++()
	{
		m_value++;
		return *this;
	}

	template<typename T>
	CompressedSigned<T> CompressedSigned<T>::operator++(int)
	{
		CompressedSigned copy(*this);
		m_value++;
		return copy;
	}

	template<typename T>
	CompressedSigned<T>& CompressedSigned<T>::operator--()
	{
		m_value--;
		return *this;
	}

	template<typename T>
	CompressedSigned<T> CompressedSigned<T>::operator--(int)
	{
		CompressedSigned copy(*this);
		m_value--;
		return copy;
	}


	template<typename T>
	CompressedUnsigned<T>::CompressedUnsigned(T value) :
	m_value(value)
	{
	}

	template<typename T>
	CompressedUnsigned<T>::operator T() const
	{
		return m_value;
	}

	template<typename T>
	CompressedUnsigned<T>& CompressedUnsigned<T>::operator=(T value)
	{
		m_value = value;
		return *this;
	}

	template<typename T>
	CompressedUnsigned<T>& CompressedUnsigned<T>::operator++()
	{
		m_value++;
		return *this;
	}

	template<typename T>
	CompressedUnsigned<T> CompressedUnsigned<T>::operator++(int)
	{
		CompressedUnsigned copy(*this);
		m_value++;
		return copy;
	}

	template<typename T>
	CompressedUnsigned<T>& CompressedUnsigned<T>::operator--()
	{
		m_value--;
		return *this;
	}

	template<typename T>
	CompressedUnsigned<T> CompressedUnsigned<T>::operator--(int)
	{
		CompressedUnsigned copy(*this);
		m_value--;
		return copy;
	}
}

namespace Nz
{
	template<typename T>
	bool Serialize(SerializationContext& context, bw::CompressedSigned<T> value, TypeTag<bw::CompressedSigned<T>>)
	{
		using UnsignedT = std::make_unsigned_t<T>;

		T signedValue = value;

		// ZigZag encoding:
		// https://developers.google.com/protocol-buffers/docs/encoding
		UnsignedT unsignedValue = (signedValue << 1) ^ (signedValue >> (CHAR_BIT * sizeof(UnsignedT) - 1));

		return Serialize(context, bw::CompressedUnsigned<UnsignedT>(unsignedValue));
	}

	template<typename T>
	bool Serialize(SerializationContext& context, bw::CompressedUnsigned<T> value, TypeTag<bw::CompressedUnsigned<T>>)
	{
		T integerValue = value;
		bool remaining;
		do 
		{
			Nz::UInt8 byteValue = integerValue & 0x7F;
			integerValue >>= 7;

			remaining = (integerValue > 0);
			if (remaining)
				byteValue |= 0x80;

			if (!Serialize(context, byteValue))
				return false;
		}
		while (remaining);

		return true;
	}

	template<typename T>
	bool Unserialize(SerializationContext& context, bw::CompressedSigned<T>* value, TypeTag<bw::CompressedSigned<T>>)
	{
		using UnsignedT = std::make_unsigned_t<T>;

		bw::CompressedUnsigned<UnsignedT> compressedValue;
		if (!Unserialize(context, &compressedValue))
			return false;

		// ZigZag decoding:
		// https://developers.google.com/protocol-buffers/docs/encoding
		UnsignedT unsignedValue = compressedValue;
		unsignedValue = (unsignedValue >> 1) - (unsignedValue & 1) * unsignedValue;

		*value = reinterpret_cast<T&>(unsignedValue);
		return true;
	}

	template<typename T>
	bool Unserialize(SerializationContext& context, bw::CompressedUnsigned<T>* value, TypeTag<bw::CompressedUnsigned<T>>)
	{
		T integerValue = 0;
		bool remaining;
		std::size_t i = 0;

		do
		{
			Nz::UInt8 byteValue;
			if (!Unserialize(context, &byteValue))
				return false;

			remaining = (byteValue & 0x80);
			if (remaining)
				byteValue &= ~Nz::UInt8(0x80);

			integerValue |= T(byteValue) << 7 * (i++);
		}
		while (remaining);

		*value = integerValue;

		return true;
	}
}
