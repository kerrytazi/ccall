#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <cstring>
#include <bit>
#include <type_traits>
#include <cassert>

#if defined(_M_X64) || defined(__x86_64__)
	#if defined(_WIN32) || defined(_WIN64)
		#define ABI_X64_MS // ABI: Microsoft x64 (Windows)
	#endif

	#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
		#define ABI_X64_AMD64 // ABI: System V AMD64 (Linux/macOS)
	#endif
#endif

#if !defined(ABI_X64_MS) && !defined(ABI_X64_AMD64)
	#error Unknown ABI
#endif


template <typename T>
constexpr bool CCallArgTypeIsFloatStruct = false;

union ireg
{
	char c_char;
	wchar_t c_wchar;
	int8_t i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;

	void* ptr;
	
	int8_t arr_i8[8];
	int16_t arr_i16[4];
	int32_t arr_i32[2];
	uint8_t arr_u8[8];
	uint16_t arr_u16[4];
	uint32_t arr_u32[2];

	uint8_t bytes[8];
};

union xmm
{
	float f32;
	double f64;

	float arr_f32[2];

	uint8_t bytes[8];
};

struct CCallArgs
{
	constexpr static size_t MAX_IREGS = 6;
	constexpr static size_t MAX_XMMS = 8;

	ireg iregs[MAX_IREGS];
	xmm xmms[MAX_XMMS];
	const uint8_t* extra;
	size_t extra_size;
};

struct CCallRet
{
	ireg iregs[2];
	xmm xmms[2];
	void* big_struct;

	template <typename T>
	constexpr T get_result() const
	{
		if constexpr (sizeof(T) <= 16)
		{
			if constexpr (std::is_floating_point_v<T> || CCallArgTypeIsFloatStruct<T>)
				return *std::bit_cast<const T*>(&xmms);
			else
				return *std::bit_cast<const T*>(&iregs);
		}
		else
		{
			return *std::bit_cast<const T*>(big_struct);
		}
	}
};

class CCallArgsBuilder
{
public:

	void expect_result(size_t size)
	{
		assert(iregs_count == 0);
		assert(xmms_count == 0);

		if (size <= 16)
			return;

		auto& ret_arg = big_args.emplace_back(std::make_unique<uint8_t[]>(size));

		result.iregs[0].ptr = (void*)ret_arg.get();
		iregs_count += 1;
	}

	template <typename T>
	void expect_result()
	{
		expect_result(sizeof(T));
	}

	void add_arg(const void* data, size_t size, bool is_float = false)
	{
		bool use_two_regs = size > 8;

		if (is_float)
		{
			if (xmms_count > CCallArgs::MAX_XMMS ||
				(use_two_regs && (xmms_count + 1 > CCallArgs::MAX_XMMS)))
			{
				add_extra(data, size);
			}
			else
			{
				memcpy(&result.xmms[xmms_count], data, size);

				if (use_two_regs)
					xmms_count += 2;
				else
					xmms_count += 1;
			}
		}
		else
		{
			if (iregs_count > CCallArgs::MAX_IREGS ||
				(use_two_regs && (iregs_count + 1 > CCallArgs::MAX_IREGS)))
			{
				add_extra(data, size);
			}
			else
			{
				memcpy(&result.iregs[iregs_count], data, size);

				if (use_two_regs)
					iregs_count += 2;
				else
					iregs_count += 1;
			}
		}
	}

	template <typename T>
	void add_arg(T val)
	{
		if constexpr (sizeof(val) <= 16)
		{
			add_arg(&val, sizeof(val), std::is_floating_point_v<T> || CCallArgTypeIsFloatStruct<T>);
		}
		else
		{
			size_t offset = add_extra((const void*)&val, sizeof(val));

			// NOTE: Possible bug. need to pass pointer to argument in register
			// TODO: place pointer to rsp+? instead of creating new memory
			//auto& big_arg = big_args.emplace_back(std::make_unique<uint8_t[]>(sizeof(val)));
			//memcpy(big_arg.get(), &val, sizeof(val));

			//add_arg(offset);
		}
	}

	const CCallArgs* get_args() const
	{
		return &result;
	}

private:

	size_t add_extra(const void* data, size_t size)
	{
		size_t prev_extra = extra.size();

		const uint8_t* u = (const uint8_t*)data;

		for (size_t i = 0; i < size; i += 8, u += 8)
		{
			uint64_t tmp = 0;
			memcpy(&tmp, u, size >= (i + 8) ? 8 : size - i);
			const uint8_t* t = (const uint8_t*)&tmp;
			extra.insert(extra.end(), t, t + 8);
		}

		result.extra = extra.data();
		result.extra_size = extra.size();

		return prev_extra;
	}

	CCallArgs result{};
	uint32_t iregs_count = 0;
	uint32_t xmms_count = 0;
	std::vector<uint8_t> extra;
	std::vector<std::unique_ptr<uint8_t[]>> big_args;
};

extern "C" void ccall(void* func, const CCallArgs* args, CCallRet* ret);




#include <iostream>

int32_t add_i32(int32_t a, int32_t b)
{
	return a + b;
}

float add_f32(float a, float b)
{
	return a + b;
}

double math_8(int32_t i1, float f2, int32_t i3, int32_t i4, float f5, int32_t i6, float f7, int32_t i8)
{
	return i1 + i3 - i6 * i8 + f2 / i4 - f5 + f7;
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
		Vector6 arg0_ = { 44.4f, 44.4f, 44.4f, 44.4f, 44.4f, 44.4f };
		Vector6 arg1_ = { 77.7f, 77.7f, 77.7f, 77.7f, 77.7f, 77.7f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<float>();
		args_builder.add_arg(arg0_);
		args_builder.add_arg(arg1_);

		ccall((void*)&math_vec5_2, args_builder.get_args(), &result);

		auto r = result.get_result<float>();
		std::cout << "result: " << r << "\n";
		auto c = math_vec5_2(arg0, arg1);
		std::cout << "check:  " << c << "\n";
	}

	if (true)
	{
		Vector5 arg0 = { .1f, .2f, .3f, .4f, .5f };
		Vector5 arg1 = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
		Vector6 arg0_ = { .1f, .2f, .3f, .4f, .5f, .6 };
		Vector6 arg1_ = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };

		CCallArgsBuilder args_builder;
		CCallRet result{};

		args_builder.expect_result<Vector5>();
		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&math_vec5, args_builder.get_args(), &result);

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

		args_builder.add_arg(arg0);
		args_builder.add_arg(arg1);

		ccall((void*)&math_vec, args_builder.get_args(), &result);

		std::cout << "result: " << result.xmms[0].arr_f32[0] << ", " << result.xmms[0].arr_f32[1] << ", " << result.xmms[1].arr_f32[0] << "\n";
		auto r = math_vec(arg0, arg1);
		std::cout << "check:  " << r.x << ", " << r.y << ", " << r.z << "\n";
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

		std::cout << "result: " << result.iregs->i32 << "\n";
		std::cout << "check:  " << add_i32(arg0, arg1) << "\n";
	}
}
