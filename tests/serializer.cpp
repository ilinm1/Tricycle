#include <string>
#include <vector>
#include <unordered_map>
#include "tc/misc.hpp"
#include "test.hpp"

namespace Tcs = Tc::Serializer;

struct SimpleType : Tcs::Serializable
{
	int FieldA;
	float FieldB;
	float* FieldC;
	char FieldD[16];

	TCS_FIELD_INFO(SimpleType, FieldA, FieldB, FieldC, FieldD);

	SimpleType() : FieldA(0), FieldB(0.0f), FieldC(nullptr) {}

	SimpleType(int fa, float fb, float* fc, const char* fd) : FieldA(fa), FieldB(fb), FieldC(fc) 
	{
		std::memset(FieldD, 0, sizeof(FieldD));
		std::memcpy(FieldD, fd, sizeof(FieldD));
	}

	SimpleType(const SimpleType& other) : FieldA(other.FieldA), FieldB(other.FieldB), FieldC(other.FieldC)
	{
		std::memset(FieldD, 0, sizeof(FieldD));
		std::memcpy(FieldD, other.FieldD, sizeof(FieldD));
	}

	bool operator==(const SimpleType& other) const
	{
		for (unsigned int i = 0; i < sizeof(FieldD); i++)
		{
			if (FieldD[i] != other.FieldD[i])
				return false;
		}

		return FieldA == other.FieldA && FieldB == other.FieldB && FieldC == other.FieldC;
	}
};
TCS_TYPE_NAME(SimpleType);

struct ComplexType : Tcs::Serializable
{
	std::string FieldA;
	SimpleType FieldB;
	std::vector<int> FieldC;
	std::vector<std::string> FieldD;
	std::vector<SimpleType> FieldE;
	std::unordered_map<std::string, SimpleType> FieldF;

	TCS_FIELD_INFO(ComplexType, FieldA, FieldB, FieldC, FieldD, FieldE, FieldF);

	ComplexType() {}

	ComplexType(
		std::string fa,
		SimpleType fb,
		std::vector<int> fc,
		std::vector<std::string> fd,
		std::vector<SimpleType> fe,
		std::unordered_map<std::string, SimpleType> ff) :
		FieldA(fa),
		FieldB(fb),
		FieldC(fc),
		FieldD(fd),
		FieldE(fe),
		FieldF(ff)
	{}

	ComplexType(const ComplexType& other) :
		FieldA(other.FieldA),
		FieldB(other.FieldB),
		FieldC(other.FieldC),
		FieldD(other.FieldD),
		FieldE(other.FieldE),
		FieldF(other.FieldF)
	{}

	char* Serialize(char* data) const override
	{
		data = Tcs::AutoSerializeBasic(*this, data);
		data = Tcs::AutoSerializeVector(FieldC, data);
		data = Tcs::AutoSerializeVector(FieldD, data);
		data = Tcs::AutoSerializeVector(FieldE, data);
		data = Tcs::AutoSerializeMap(FieldF, data);
		return data;
	}

	char* Deserialize(const char* data) override
	{
		data = Tcs::AutoDeserializeBasic(*this, data);
		data = Tcs::AutoDeserializeVector(FieldC, data);
		data = Tcs::AutoDeserializeVector(FieldD, data);
		data = Tcs::AutoDeserializeVector(FieldE, data);
		data = Tcs::AutoDeserializeMap(FieldF, data);
		return const_cast<char*>(data);
	}

	bool operator==(const ComplexType& other) const
	{
		return FieldA == other.FieldA && FieldB == other.FieldB && FieldC == other.FieldC && FieldD == other.FieldD && FieldE == other.FieldE && FieldF == other.FieldF;
	}
};
TCS_TYPE_NAME(ComplexType);

void SerializerTest()
{
	char data1[1024];
	char data2[1024];

	//simple type serialization (no nested classes)
	float x;
	SimpleType s1(5, 3.0f, nullptr, "abcdef");
	SimpleType s2(10, 0.0f, &x, "string");
	SimpleType s3;
	SimpleType s1c(s1);
	SimpleType s2c(s2);

	TCT_ASSERT(s1 == s1c);
	TCT_ASSERT(s2 == s2c);
	TCT_ASSERT(s1 != s3);
	TCT_ASSERT(s2 != s3);
	TCT_ASSERT(s1 != s2);

	s1.Serialize(data1);
	s2.Serialize(data2);
	TCT_ASSERT(s1 == s1c);
	TCT_ASSERT(s2 == s2c);

	s3.Deserialize(data1);
	TCT_ASSERT(s1 == s3);

	s2.Deserialize(data1);
	TCT_ASSERT(s1 == s2);

	s1.Deserialize(data2);
	TCT_ASSERT(s1 != s2);

	//complex type serialization
	ComplexType c1(
		"complex",
		SimpleType(5, 3.0f, nullptr, "abcdef"),
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		{ "abc", "123", "", "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." },
		{ SimpleType(1, 0.0f, &x, "string"), SimpleType(5, -1.0f, &x, ""), SimpleType() },
		{ {"abc", SimpleType()}, {"def", SimpleType(-1, 2.0f, &x, ":)")}, {"ghi", SimpleType()}, {"@#$&*?!!", SimpleType(10, 0.5f, nullptr, "...")}}
	);
	ComplexType c2(
		"complex two",
		SimpleType(10, 0.0f, &x, "..."),
		{ },
		{ "Lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing", "elit", "sed", "do", "eiusmod", "tempor", "incididunt", "ut", "labore", "et", "dolore", "magna", "aliqua" },
		{ SimpleType(-10, 10.0f, nullptr, ""), SimpleType(), SimpleType(0, 0.0f, nullptr, ":D"), SimpleType() },
		{ { "abc", SimpleType(3, 3.0f, &x, "123") }, { "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.", SimpleType(0, 0.0f, nullptr, ":)")}}
	);
	ComplexType c1c(c1);
	ComplexType c2c(c2);
	ComplexType c3;

	TCT_ASSERT(c1 == c1c);
	TCT_ASSERT(c2 == c2c);
	TCT_ASSERT(c1 != c3);
	TCT_ASSERT(c2 != c3);
	TCT_ASSERT(c1 != c2);

	c1.Serialize(data1);
	c2.Serialize(data2);
	TCT_ASSERT(c1 == c1c);
	TCT_ASSERT(c2 == c2c);

	c3.Deserialize(data1);
	TCT_ASSERT(c1 == c3);

	c2.Deserialize(data1);
	TCT_ASSERT(c1 == c2);

	c1.Deserialize(data2);
	TCT_ASSERT(c1 != c2);
}
TCT_REGISTER(SerializerTest);
