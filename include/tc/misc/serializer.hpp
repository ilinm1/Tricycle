#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <format>
#include "json.hpp"
#include "macros.hpp"

#define _TCS_TYPE_NAME(type) template<> inline constexpr const char* Tc::Serializer::GetTypeName<type>() { return STRINGIFY(type); }

//use this macro so that type's name can be queried using 'GetTypeName'
#define TCS_TYPE_NAME(type) _TCS_TYPE_NAME(type) _TCS_TYPE_NAME(type ## *) _TCS_TYPE_NAME(type ## **)

#define _TCS_FIELD_INFO(field, type) \
	Tc::Serializer::FieldInfo \
	{ \
		STRINGIFY(field), \
		Tc::Serializer::GetTypeName<decltype(type ## :: ## field)>(), \
		sizeof(type ## :: ## field), \
		offsetof(type, field), \
		std::is_class<decltype(type ## :: ## field)>(), \
		std::is_same<std::string, decltype(type ## :: ## field)>(), \
		std::is_base_of<Tc::Serializer::Serializable, decltype(type ## :: ## field)>() \
	} \

//use this macro to automatically generate 'GetFieldInfo'; first argument is your class type, everything else are fields/members that should be added
#define TCS_FIELD_INFO(type, ...) \
	const std::vector<Tc::Serializer::FieldInfo>& GetFieldInfo() const override \
	{ \
		static const std::vector<Tc::Serializer::FieldInfo> info = { MAP_LIST_UD(_TCS_FIELD_INFO, type, __VA_ARGS__) }; \
		return info; \
	} \

namespace Tc::Serializer
{
	struct FieldInfo
	{
		const char* Name;
		const char* TypeName;
		const unsigned int Size;
		const unsigned int Offset;
		const bool IsClass;
		const bool IsString; //true if field is 'std::string'
		const bool IsSerializable; //true if field is a class that inherits 'Serializable'
	};

	/*
	default serialization/deserialization logic is able to handle classes which contain:
	fundamental types (bools, integers, floats...),
	basic compound types (arrays, pointers (obviously only copies the pointer's value, not the data it points to), enums...),
	STL strings,
	and nested classes which inherit from 'Serializable'
	if your class contains anything else (basically any data on heap that must be copied, STL containers, etc) you must write your own 'Serialize' and 'Deserialize' overrides
	(idk if it's possible to implement automatic serialization for STL containers using 'type_traits' or some other methods, my only idea is to parse the type name but I don't want to go that low)
	to reduce the amount of boilerplate you may call 'AutoSerializeBasic'/'AutoDeserializeBasic' to (de)serialize basic types
	and then use 'AutoSerializeVector'/'AutoSerializeMap'/custom logic to (de)serialize everything else
	*/
	struct Serializable
	{
		virtual ~Serializable();

		virtual const std::vector<FieldInfo>& GetFieldInfo() const;

		virtual unsigned int GetSerializedDataSize();

		//serializes 'obj' and writes the result to 'data'; amount of memory required can be obtained using 'GetSerializedDataSize'
		//returned pointer points to the byte after the end of the serialized data
		virtual char* Serialize(char* data) const;

		//deserializes 'obj' from 'data'; amount of memory required can be obtained using 'GetSerializedDataSize'
		//returned pointer points to the byte after the end of the deserialized data
		virtual char* Deserialize(const char* data);

		virtual Tc::Json::JsonValue SerializeJson() const;

		virtual void DeserializeJson(const Tc::Json::JsonValue& value);
	};

	//automatically serializes basic types (see list above) as well as classes inherited from the 'Serializable'
	//returned pointer points to the byte after the end of the serialized data
	char* AutoSerializeBasic(const Serializable& obj, char* data);

	//automatically deserializes basic types (see list above) as well as classes inherited from the 'Serializable'
	//returned pointer points to the byte after the end of the deserialized data
	char* AutoDeserializeBasic(Serializable& obj, const char* data);

	//'str' should be a null terminated string
	char* AutoSerializeString(const std::string& str, char* data);

	char* AutoDeserializeString(std::string& str, const char* data);

	template<class T>
	inline constexpr const char* GetTypeName() { return "unknown"; }

	template<class T>
	inline char* AutoSerializeVector(const std::vector<T>& vec, char* data)
	{
		if constexpr(std::is_class<T>() && !std::is_base_of<Serializable, T>() && !std::is_same<std::string, T>())
			throw std::runtime_error(std::format("Type '{}' (vector element) is not serializable.", STRINGIFY(T)));

		unsigned int size = vec.size();
		std::memcpy(data, &size, sizeof(unsigned int));
		data += sizeof(unsigned int);

		for (const T& value : vec)
		{
			if constexpr(std::is_same<std::string, T>())
			{
				data = AutoSerializeString(value, data);
			}
			else if constexpr(std::is_class<T>())
			{
				data = static_cast<const Serializable*>(&value)->Serialize(data);
			}
			else
			{
				std::memcpy(data, &value, sizeof(T));
				data += sizeof(T);
			}
		}

		return data;
	}

	template<class T>
	inline char* AutoDeserializeVector(std::vector<T>& vec, const char* data)
	{
		if constexpr (std::is_class<T>() && !std::is_base_of<Serializable, T>() && !std::is_same<std::string, T>())
			throw std::runtime_error(std::format("Type '{}' is not serializable.", STRINGIFY(T)));

		unsigned int size;
		std::memcpy(&size, data, sizeof(unsigned int));
		data += sizeof(unsigned int);

		vec.clear();
		for (; size; size--)
		{
			vec.resize(vec.size() + 1);
			T& value = vec.back();

			if constexpr(std::is_same<std::string, T>())
			{
				data = AutoDeserializeString(value, data);
			}
			else if constexpr(std::is_class<T>())
			{
				data = static_cast<Serializable*>(&value)->Deserialize(data);
			}
			else
			{
				std::memcpy(&value, data, sizeof(T));
				data += sizeof(T);
			}
		}

		return const_cast<char*>(data);
	}

	template<class K, class V>
	inline char* AutoSerializeMap(const std::unordered_map<K, V>& map, char* data)
	{
		std::vector<K> keys;
		std::vector<V> values;

		for (auto& pair : map)
		{
			keys.push_back(pair.first);
			values.push_back(pair.second);
		}

		data = AutoSerializeVector(keys, data);
		data = AutoSerializeVector(values, data);

		return data;
	}

	template<class K, class V>
	inline char* AutoDeserializeMap(std::unordered_map<K, V>& map, const char* data)
	{
		std::vector<K> keys;
		std::vector<V> values;

		data = AutoDeserializeVector(keys, data);
		data = AutoDeserializeVector(values, data);

		if (keys.size() != values.size())
			throw std::runtime_error("The amount of keys doesn't match the amount of values.");

		map.clear();
		for (unsigned int i = 0; i < keys.size(); i++)
		{
			map[keys[i]] = values[i];
		}

		return const_cast<char*>(data);
	}
}

//names of some of the basic types

TCS_TYPE_NAME(bool);
TCS_TYPE_NAME(char);
TCS_TYPE_NAME(unsigned char);
TCS_TYPE_NAME(int);
TCS_TYPE_NAME(unsigned int);
TCS_TYPE_NAME(float);
TCS_TYPE_NAME(double);
TCS_TYPE_NAME(std::string);
