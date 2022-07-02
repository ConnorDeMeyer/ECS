#pragma once

#include <string_view>
#include <cstdint>

//https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way

namespace reflection
{


	template <typename T> constexpr std::string_view type_name();

	template <>
	constexpr std::string_view type_name<void>()
	{
		return "void";
	}

	namespace detail
	{
		using type_name_prober = void;

		template <typename T>
		constexpr std::string_view wrapped_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		}

		constexpr std::size_t wrapped_type_name_prefix_length()
		{
			return wrapped_type_name<type_name_prober>().find(type_name<type_name_prober>());
		}

		constexpr std::size_t wrapped_type_name_suffix_length()
		{
			return wrapped_type_name<type_name_prober>().length()
				- wrapped_type_name_prefix_length()
				- type_name<type_name_prober>().length();
		}
	}

	template <typename T>
	constexpr std::string_view type_name()
	{
		constexpr auto wrapped_name = detail::wrapped_type_name<T>();
		constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}

	//https://gist.github.com/Lee-R/3839813

	// FNV-1a 32bit hashing algorithm.
	constexpr std::uint32_t fnv1a_32(char const* s, std::size_t count)
	{
		return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}

	constexpr std::uint32_t operator"" _hash(char const* s, std::size_t count)
	{
		return fnv1a_32(s, count);
	}

	constexpr uint32_t hash(std::string_view str)
	{
		return fnv1a_32(str.data(), str.length());
	}

	template <typename T>
	constexpr uint32_t type_id()
	{
		return hash(type_name<T>());
	}

	//enum HashTestEnum : uint32_t
	//{
	//	CrcVal01 = hash("class RenderComponent"),
	//	CrcVal02 = hash("class SpriteComponent")
	//};
	//
	//constexpr std::string_view GetTypeName() const
	//{
	//#ifdef __clang__
	//	constexpr std::string_view functionSig{ __PRETTY_FUNCTION__ };
	//#elif defined(__GNUC__)
	//	constexpr std::string_view functionSig{ __PRETTY_FUNCTION__ };
	//#elif defined(_MSC_VER)
	//	constexpr std::string_view functionSig{ __FUNCSIG__ };
	//#else
	//#error "Unsupported compiler"
	//#endif
	//
	//	constexpr size_t end{ functionSig.find("::GetTypeName(") };
	//	constexpr size_t begin{ functionSig.rfind(" ", end) };
	//	constexpr std::string_view typeName = functionSig.substr(begin, end - begin);
	//	return typeName;
	//}
}