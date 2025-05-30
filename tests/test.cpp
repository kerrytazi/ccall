#include "ccall/ccall.hpp"

#include <iostream>

int32_t add_i32(int32_t a, int32_t b)
{
	return a + b;
}

float add_f32(float a, float b)
{
	return a + b;
}

void print_i32(int32_t i1)
{
	std::cout << "print_i32: " << i1 << "\n";
}

struct Vector3
{
	float x, y, z;
};

struct Vector5
{
	float x, y, z, w, i;
};

struct Vector6
{
	float x, y, z, w, i, k;
};

template <>
constexpr bool CCallArgTypeIsFloatStruct<Vector3> = true;

double math_10(int32_t i1, float f2, int32_t i3, Vector3 v4, int64_t i5, float f6, int32_t i7, double f8, int64_t i9, Vector3 v10)
{
	return i1 + v4.x + i3 / v4.y - i7 - v4.z + v10.x * i9 + f2 - v10.y / i5 - f6 + f8 * v10.z;
}

double math_9(int32_t i1, float f2, int32_t i3, Vector3 v4, int64_t i5, float f6, int32_t i7, double f8, int64_t i9)
{
	return i1 + v4.x + i3 / v4.y - i7 - v4.z * i9 + f2 / i5 - f6 + f8;
}

double math_8(int32_t i1, float f2, int32_t i3, Vector3 v4, int64_t i5, float f6, int32_t i7, double f8)
{
	return i1 + v4.x + i3 / v4.y - i7 - v4.z + f2 / i5 - f6 + f8;
}

Vector3 math_vec(Vector3 v1, Vector3 v2)
{
	return Vector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

Vector5 math_vec5(Vector5 v1, Vector5 v2)
{
	return Vector5{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w, v1.i + v2.i };
}

float math_vec5_2(Vector5 v1, Vector5 v2)
{
	return v1.x + v2.x;
}

int main()
{
	if (true)
	{
		Vector5 arg0 = { 44.4f, 55.5f, 66.6f, 12.1f, 32.2f };
		Vector5 arg1 = { 77.7f, 74.1f, 41.2f, 6.32f, 14.5f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<float>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&math_vec5_2, args_builder.get_args(), &result);
		
		std::cout << "math_vec5_2:\n";
		auto r = result.get_result<float>();
		std::cout << "result: " << r << "\n";
		auto c = math_vec5_2(arg0, arg1);
		std::cout << "check:  " << c << "\n";
	}

	if (true)
	{
		Vector5 arg0 = { .1f, .2f, .3f, .4f, .5f };
		Vector5 arg1 = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<Vector5>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&math_vec5, args_builder.get_args(), &result);

		std::cout << "math_vec5:\n";
		auto r = result.get_result<Vector5>();
		std::cout << "result: " << r.x << ", " << r.y << ", " << r.z << ", " << r.w << ", " << r.i << "\n";
		auto c = math_vec5(arg0, arg1);
		std::cout << "check:  " << c.x << ", " << c.y << ", " << c.z << ", " << c.w << ", " << c.i << "\n";
	}

	if (true)
	{
		Vector3 arg0 = { 44.4f, 55.5f, 66.6f };
		Vector3 arg1 = { 77.7f, 74.1f, 41.2f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<Vector3>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&math_vec, args_builder.get_args(), &result);

		std::cout << "math_vec:\n";
		auto r = result.get_result<Vector3>();
		std::cout << "result: " << r.x << ", " << r.y << ", " << r.z << "\n";
		auto c = math_vec(arg0, arg1);
		std::cout << "check:  " << c.x << ", " << c.y << ", " << c.z << "\n";
	}

	if (true)
	{
		int32_t arg0 = 123;
		float arg1 = 12.3f;
		int32_t arg2 = 65;
		Vector3 arg3 = { 1.3f, 2.8f, 3.7f };
		int64_t arg4 = 77;
		float arg5 = 65.7f;
		int32_t arg6 = 3;
		double arg7 = 0.1234253;

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<double>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);
		args_builder.add_arg(arg2);
		args_builder.add_arg(arg3);
		args_builder.add_arg(arg4);
		args_builder.add_arg(arg5);
		args_builder.add_arg(arg6);
		args_builder.add_arg(arg7);

		ccall((void*)&math_8, args_builder.get_args(), &result);

		std::cout << "math_8:\n";
		std::cout << "result: " << result.get_result<double>() << "\n";
		std::cout << "check:  " << math_8(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) << "\n";
	}

	if (true)
	{
		int32_t arg0 = 123;
		float arg1 = 12.3f;
		int32_t arg2 = 65;
		Vector3 arg3 = { 1.3f, 2.8f, 3.7f };
		int64_t arg4 = 77;
		float arg5 = 65.7f;
		int32_t arg6 = 3;
		double arg7 = 0.1234253;
		int64_t arg8 = -99;

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<double>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);
		args_builder.add_arg(arg2);
		args_builder.add_arg(arg3);
		args_builder.add_arg(arg4);
		args_builder.add_arg(arg5);
		args_builder.add_arg(arg6);
		args_builder.add_arg(arg7);
		args_builder.add_arg(arg8);

		ccall((void*)&math_9, args_builder.get_args(), &result);

		std::cout << "math_9:\n";
		std::cout << "result: " << result.get_result<double>() << "\n";
		std::cout << "check:  " << math_9(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) << "\n";
	}

	if (true)
	{
		int32_t arg0 = 123;
		float arg1 = 12.3f;
		int32_t arg2 = 65;
		Vector3 arg3 = { 1.3f, 2.8f, 3.7f };
		int64_t arg4 = 77;
		float arg5 = 65.7f;
		int32_t arg6 = 3;
		double arg7 = 0.1234253;
		int64_t arg8 = -99;
		Vector3 arg9 = { 44.4f, 55.5f, 66.6f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<double>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);
		args_builder.add_arg(arg2);
		args_builder.add_arg(arg3);
		args_builder.add_arg(arg4);
		args_builder.add_arg(arg5);
		args_builder.add_arg(arg6);
		args_builder.add_arg(arg7);
		args_builder.add_arg(arg8);
		args_builder.add_arg(arg9);

		ccall((void*)&math_10, args_builder.get_args(), &result);

		std::cout << "math_10:\n";
		std::cout << "result: " << result.get_result<double>() << "\n";
		std::cout << "check:  " << math_10(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) << "\n";
	}

	if (true)
	{
		int32_t arg0 = 123;
		int32_t arg1 = 234;

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&add_i32, args_builder.get_args(), &result);

		std::cout << "add_i32:\n";
		std::cout << "result: " << result.iregs->i32 << "\n";
		std::cout << "check:  " << add_i32(arg0, arg1) << "\n";
	}

	if (true)
	{
		int32_t arg0 = 123;

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<void>();
		args_builder.add_arg(arg0);

		std::cout << "print_i32:\n";
		ccall((void*)&print_i32, args_builder.get_args(), &result);

		print_i32(arg0);
	}
}
