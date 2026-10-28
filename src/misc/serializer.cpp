#include "tc/misc/serializer.hpp"

namespace Tcs = Tc::Serializer;
namespace Tcj = Tc::Json;

//automatic (de)serialization logic - see 'serializer.hpp' for more details

//automatically serializes basic types as well as classes inherited from the 'Serializable'
//returned pointer points to the byte after the end of the serialized data
char* Tcs::AutoSerializeBasic(const Serializable& obj, char* data)
{
	const char* bytes = reinterpret_cast<const char*>(&obj);
	std::vector<Serializable*> classes;
	for (const FieldInfo& info : obj.GetFieldInfo())
	{
		const char* fieldPtr = bytes + info.Offset;

		if (info.IsClass)
		{
			if (info.IsString)
				data = AutoSerializeString(*reinterpret_cast<const std::string*>(fieldPtr), data);

			if (info.IsSerializable)
				data = static_cast<const Serializable*>(reinterpret_cast<const void*>(fieldPtr))->Serialize(data);
		}
		else
		{
			std::memcpy(data, fieldPtr, info.Size);
			data += info.Size;
		}
	}

	return data;
}

//automatically deserializes basic types as well as classes inherited from the 'Serializable'
//returned pointer points to the byte after the end of the deserialized data
char* Tcs::AutoDeserializeBasic(Serializable& obj, const char* data)
{
	char* bytes = reinterpret_cast<char*>(&obj);

	for (const FieldInfo& info : obj.GetFieldInfo())
	{
		char* fieldPtr = bytes + info.Offset;

		if (info.IsClass)
		{
			if (info.IsString)
				data = AutoDeserializeString(*reinterpret_cast<std::string*>(fieldPtr), data);

			if (info.IsSerializable)
				data = static_cast<Serializable*>(reinterpret_cast<void*>(fieldPtr))->Deserialize(data);
		}
		else
		{
			std::memcpy(fieldPtr, data, info.Size);
			data += info.Size;
		}
	}

	return const_cast<char*>(data);
}

char* Tcs::AutoSerializeString(const std::string& str, char* data)
{
	unsigned int length = str.length() + 1;
	std::memcpy(data, str.data(), length);
	data += length;
	return data;
}

char* Tcs::AutoDeserializeString(std::string& str, const char* data)
{
	unsigned int length = std::strlen(data) + 1;
	str.resize(std::strlen(data));
	std::memcpy(str.data(), data, length);
	data += length;
	return const_cast<char*>(data);
}

//'Serializable'

Tcs::Serializable::~Serializable() {}

const std::vector<Tcs::FieldInfo>& Tcs::Serializable::GetFieldInfo() const
{
	throw std::runtime_error("No field info provided for this type, use 'TCS_FIELD_INFO'.");
}

unsigned int Tcs::Serializable::GetSerializedDataSize()
{
	unsigned int sum = 0;

	for (const FieldInfo& info : GetFieldInfo())
	{
		sum += info.Size;
	}

	return sum;
}

//serializes 'obj' and writes the result to 'data'; amount of memory required can be obtained using 'GetSerializedDataSize'
//returned pointer points to the byte after the end of the serialized data
char* Tcs::Serializable::Serialize(char* data) const
{
	return AutoSerializeBasic(*this, data);
}

//deserializes 'obj' from 'data'; amount of memory required can be obtained using 'GetSerializedDataSize'
//returned pointer points to the byte after the end of the deserialized data
char* Tcs::Serializable::Deserialize(const char* data)
{
	return AutoDeserializeBasic(*this, data);
}

Tcj::JsonValue Tcs::Serializable::SerializeJson() const
{
	throw std::exception("Not implemented yet.");
}

void Tcs::Serializable::DeserializeJson(const Tcj::JsonValue& value)
{
	throw std::exception("Not implemented yet.");
}
