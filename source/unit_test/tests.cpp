#include <cstdio>
#include "gtest/gtest.h"

#include "Core/math/Vector4.h"
#include "Core/math/Vector3.h"
#include "Core/math/Vector2.h"
#include "Core/containers/GrowingArray.h"
#include "Core/containers/Array.h"
/*
	different macros for unit tests

	EXPECT_EQ // equal
	EXPECT_NE // not equal
	EXPECT_LT // less than
	EXPECT_LE // lesser or equal than
	EXPECT_GT // greater than
	EXPECT_GE // greater or equal than

	Expect does not assume fatal

	ASSERT_TRUE //fatal
	ASSERT_FALSE //fatal
*/

TEST(Vector4, Length)
{
	constexpr float a = 193.f, b = 284.f, c = 321.f, d = 461.f;
	constexpr float constexpr_result = (a * a) + (b * b) + (c * c) + (d * d);
	Core::Vector4<float> vector(a, b, c, d);
	ASSERT_EQ(vector.Length(), sqrtf(constexpr_result));
}

TEST(Vector4, Length2)
{
	constexpr float a = 193.f, b = 284.f, c = 321.f, d = 461.f;
	constexpr float constexpr_result = (a * a) + (b * b) + (c * c) + (d * d);
	Core::Vector4<float> vector(a, b, c, d);
	ASSERT_EQ(vector.Length2(), constexpr_result);
}

TEST(Vector4, dot)
{
	constexpr float a = 193.f, b = 284.f, c = 321.f, d = 461.f;
	constexpr float e = 913.f, f = 378.f, g = 245.f, h = 560.f;
	constexpr float dot_result = (a * e) + (b * f) + (c * g) + (d * h);

	Core::Vector4<float> first(a, b, c, d);
	Core::Vector4<float> second(e, f, g, h);

	ASSERT_EQ(Core::Dot(first, second), dot_result);
}

TEST(GrowingArray, create)
{
	Core::GrowingArray<float> array;
	ASSERT_NE(array.Size(), array.Capacity());
	ASSERT_EQ(array.Size(), 0);
	ASSERT_EQ(array.Capacity(), 10); // if the default capacity ever changes this test will break
}

TEST(GrowingArray, addOne)
{
	Core::GrowingArray<float> array;
	array.Add(1.f);
	ASSERT_EQ(array.Size(), 1);
}

TEST(GrowingArray, ComplexTypeAdd)
{
	class Foo
	{
	public:
		Foo() = default;
		~Foo() = default;
		int apa = 1;
		float apa2 = 0.f;
		int* pApa = nullptr;
	};

	Core::GrowingArray<Foo> arr(2);

	arr.Add(Foo());
	ASSERT_EQ(arr.Size(), 1);

	ASSERT_EQ(arr[0].apa, 1);
	ASSERT_EQ(arr[0].apa2, 0.f);
	ASSERT_EQ(arr[0].pApa, nullptr);

}

TEST(GrowingArray, fill)
{
	Core::GrowingArray<float> array;
	for(int i = 0; i < array.Capacity(); i++)
		array.Add(1.f);

	ASSERT_EQ(array.Size(), array.Capacity());
}

TEST(GrowingArray, Copy)
{
	Core::GrowingArray<float> array;
	for(int i = 0; i < array.Capacity(); i++)
		array.Add(1.f);

	Core::GrowingArray<float> array2(array);
	ASSERT_TRUE(&array[0] != nullptr);
	ASSERT_TRUE(&array2[0] != nullptr);
	ASSERT_NE(&array[0], &array2[0]);
}

TEST(GrowingArray, InitList)
{
	Core::GrowingArray<float> arr({ 1.f, 2.f, 3.f, 4.f, 5.f });
	printf("{ Size: %d, Capacity: %d }\n", arr.Size(), arr.Capacity());
	for(float& f : arr)
		printf("%.3f, ", f);

	printf("\n");
	ASSERT_EQ(arr.Size(), 5);
	ASSERT_EQ(arr.Capacity(), 5);
}

TEST(Array, InitList) 
{
	Core::Array<int> arr({1,2,3,4,5,6,7,8,9, 10 });

	ASSERT_EQ(arr.Size(), 10);
	ASSERT_EQ(arr.Capacity(), 10);
}


GTEST_API_ int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
