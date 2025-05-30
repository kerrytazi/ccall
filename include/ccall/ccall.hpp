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
#if defined(ABI_X64_MS)
	constexpr static size_t MAX_IREGS = 4;
#elif defined(ABI_X64_AMD64)
	constexpr static size_t MAX_IREGS = 6;
	constexpr static size_t MAX_XMMS = 8;
#endif

	ireg iregs[MAX_IREGS];
#if defined(ABI_X64_AMD64)
	xmm xmms[MAX_XMMS];
#endif

	const uint8_t* extra;
	size_t extra_size;
};

struct CCallRet
{
#if defined(ABI_X64_MS)
	constexpr static size_t MAX_IREGS = 1;
	constexpr static size_t MAX_XMMS = 1;
#elif defined(ABI_X64_AMD64)
	constexpr static size_t MAX_IREGS = 2;
	constexpr static size_t MAX_XMMS = 2;
#endif

	ireg iregs[MAX_IREGS];
	xmm xmms[MAX_XMMS];
	void* big_struct;

	template <typename T>
	constexpr T get_result() const
	{
		if constexpr (sizeof(T) <= sizeof(iregs))
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

	template <typename T>
	void expect_result()
	{
		if constexpr (!std::is_same_v<T, void>)
			expect_result(sizeof(T));
	}

	template <typename T>
	void add_arg(T val)
	{
#if defined(ABI_X64_MS)
		if constexpr (sizeof(val) <= sizeof(CCallRet::iregs))
		{
			add_arg(&val, sizeof(val), std::is_floating_point_v<T>);
		}
		else
		{
			auto& big_arg = big_args.emplace_back(std::make_unique<uint8_t[]>(sizeof(val)));
			memcpy(big_arg.get(), &val, sizeof(val));
			add_arg(big_arg.get());
		}
#elif defined(ABI_X64_AMD64)
		if constexpr (sizeof(val) <= sizeof(CCallRet::iregs))
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
#endif
	}

	const CCallArgs* get_args() const
	{
		return &result;
	}

private:

	void expect_result(size_t size)
	{
		assert(iregs_count == 0);
#if defined(ABI_X64_AMD64)
		assert(xmms_count == 0);
#endif

		if (size <= sizeof(CCallRet::iregs))
			return;

		auto& ret_arg = big_args.emplace_back(std::make_unique<uint8_t[]>(size));

		result.iregs[0].ptr = (void*)ret_arg.get();
		iregs_count += 1;
	}

	void add_arg(const void* data, size_t size, bool is_float = false)
	{
#if defined(ABI_X64_MS)
		if (iregs_count >= CCallArgs::MAX_IREGS)
		{
			add_extra(data, size);
		}
		else
		{
			memcpy(&result.iregs[iregs_count], data, size);
			iregs_count += 1;
		}
#elif defined(ABI_X64_AMD64)
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
#endif
	}

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
#if defined(ABI_X64_AMD64)
	uint32_t xmms_count = 0;
#endif
	std::vector<uint8_t> extra;
	std::vector<std::unique_ptr<uint8_t[]>> big_args;
};

extern "C" void ccall(void* func, const CCallArgs* args, CCallRet* ret);
