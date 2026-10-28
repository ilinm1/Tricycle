#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>

//a primitive JSON parser
//https://www.json.org/json-en.html
//https://www.json.org/example.html
namespace Tc::Json
{
	enum JsonValueType
	{
		Object,
		Array,
		Number,
		String,
		Bool,
		Null
	};

	struct JsonValue
	{
		JsonValueType Type;

		union
		{
			std::unordered_map<std::string, JsonValue> Object;
			std::vector<JsonValue> Array;
			std::string String;
			double Number;
			bool Bool;
		};

		JsonValue(std::unordered_map<std::string, JsonValue> v);

		JsonValue(std::vector<JsonValue> v);

		JsonValue(double v);

		JsonValue(std::string v);

		JsonValue(bool v);

		JsonValue();

		JsonValue(const JsonValue& other);

		JsonValue(JsonValue&& other);

		~JsonValue();

		JsonValue& operator=(const JsonValue& other);

		JsonValue& operator=(JsonValue&& other) noexcept;
	};

	//parses the first object/array it sees, 'str' is modified in the process
	JsonValue JsonParse(std::string str);

	std::string JsonGenerate(JsonValue value);
}
