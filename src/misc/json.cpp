#include <unordered_map>
#include <stdexcept>
#include <format>
#include "tc/misc/json.hpp"

namespace Tcj = Tc::Json;

//'JsonValue'

Tcj::JsonValue::JsonValue(std::unordered_map<std::string, JsonValue> v) : Type(JsonValueType::Object), Object(v) {}

Tcj::JsonValue::JsonValue(std::vector<JsonValue> v) : Type(JsonValueType::Array), Array(v) {}

Tcj::JsonValue::JsonValue(double v) : Type(JsonValueType::Number), Number(v) {}

Tcj::JsonValue::JsonValue(std::string v) : Type(JsonValueType::String), String(v) {}

Tcj::JsonValue::JsonValue(bool v) : Type(JsonValueType::Bool), Bool(v) {}

Tcj::JsonValue::JsonValue() : Type(JsonValueType::Null)
{
	std::memset(&Object, 0, sizeof(Object));
}

//copy constructor
Tcj::JsonValue::JsonValue(const JsonValue& other) :
	Type(other.Type),
	Object(other.Object)
{}

//move constructor
Tcj::JsonValue::JsonValue(JsonValue&& other) :
	Type(other.Type),
	Object(other.Object)
{
	other.Type = JsonValueType::Null;
}

Tcj::JsonValue::~JsonValue()
{
	if (Type == JsonValueType::Object)
		Object.~unordered_map();

	if (Type == JsonValueType::Array)
		Array.~vector();

	if (Type == JsonValueType::String)
		String.~basic_string();
}

//copy assignment
Tcj::JsonValue& Tcj::JsonValue::operator=(const JsonValue& other)
{
	std::memcpy(this, &other, sizeof(JsonValue));
	return *this;
}

//move assignment
Tcj::JsonValue& Tcj::JsonValue::operator=(JsonValue&& other) noexcept
{
	std::memcpy(this, &other, sizeof(JsonValue));
	other.Type = JsonValueType::Null;
	return *this;
}

//parser

unsigned int FindEndOfObject(std::string str, unsigned int offset)
{
	bool obj = str[offset] == '{';
	unsigned int depth = 0;
	for (unsigned int i = 1 + offset; i < str.size(); i++)
	{
		if (str[i] == (obj ? '}' : ']'))
		{
			if (!(--depth))
				return i;
		}

		if (str[i] == (obj ? '{' : '['))
			depth++;
	}

	throw std::runtime_error(std::format("No end of an object/array found - '{}...'.", str.substr(offset, 16)));
}

Tcj::JsonValue JsonParseAny(std::string str);

Tcj::JsonValue JsonParseArray(std::string str)
{
	std::vector<Tcj::JsonValue> values;

	unsigned int index = 1;
	while (true)
	{
		unsigned int length = FindEndOfObject(str, index) - index + 1;
		values.push_back(JsonParseAny(str.substr(index, length)));
		index += length;

		if (str[index] == ']')
		{
			return Tcj::JsonValue(values);
		}
		else if (str[index] != ',')
		{
			throw std::runtime_error(std::format("No comma found between values in an array - '{}...'.", str.substr(index, 16)));
		}

		length++; //advancing to the beginning of the next value
	}
}

Tcj::JsonValue JsonParseObject(std::string str)
{
	std::unordered_map<std::string, Tcj::JsonValue> values;

	unsigned int index = 1;
	while (true)
	{
		auto sep = std::find(str.begin() + index, str.end() + 1, ':');
		if (sep == str.end() + 1)
			throw std::runtime_error(std::format("No separator found in an object's pair - '{}...'.", str.substr(index, 16)));

		//'index' is currently pointing at the key's beginning (a double quote) so we must add 1 to it
		//and 'sep - str.begin()' gives us key's length including quotes so we must subtract 2 from it
		unsigned int keyLength = sep - str.begin() + index;
		std::string key = str.substr(index + 1, keyLength - 2);

		unsigned int valueIndex = index + keyLength + 1;
		unsigned int valueLength = FindEndOfObject(str, valueIndex) - index + 1;
		Tcj::JsonValue value = JsonParseAny(str.substr(valueIndex, valueLength));

		values[key] = value;
		index += keyLength + valueLength + 2; //need to account for the pair's separator - adding 1, index should be pointing at the symbol past the value's end - adding 1 again

		if (str[index] == '}')
		{
			return Tcj::JsonValue(values);
		}
		else if (str[index] != ',')
		{
			throw std::runtime_error(std::format("No comma found between values in an object - '{}...'.", str.substr(index, 16)));
		}
	}
}

Tcj::JsonValue JsonParseAny(std::string str)
{
	if (str[0] == '[')
		return JsonParseArray(str);

	if (str[0] == '{')
		return JsonParseObject(str);

	if (str[0] == '"')
	{
		char* copy;

		auto strEnd = std::find(str.begin(), str.end() + 1, '"');
		if (strEnd == str.end() + 1)
			throw std::runtime_error(std::format("No terminating double quotes found in a string - '{}...'.", str.substr(0, 16)));

		unsigned int length = strEnd - str.begin();
		copy = new char[length + 1];
		std::memcpy(copy, str.c_str(), length);
		copy[length - 1] = '\0';

		return Tcj::JsonValue(std::string(copy));
	}

	std::string sub = str.substr(0, 4);
	if (sub == "true")
		return Tcj::JsonValue(true);

	if (sub == "null")
		return Tcj::JsonValue();

	sub = str.substr(0, 5);
	if (sub == "false")
		return Tcj::JsonValue(false);

	char* end;
	double v = std::strtod(str.c_str(), &end);
	if (end != str.c_str())
		return Tcj::JsonValue(v);

	throw std::runtime_error(std::format("Failed to parse a value '{}'.", str));
}

Tcj::JsonValue Tcj::JsonParse(std::string str)
{
	str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end()); //removing all the whitespaces
	return JsonParseAny(str);
}
