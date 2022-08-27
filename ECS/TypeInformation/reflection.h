#pragma once

#include <string_view>
#include <cstdint>
#include <array>

//https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way

namespace reflection
{

	template <typename T>
	concept _Function = std::is_function_v<T>;

	template <typename T>
	concept _Member_function_pointer = std::is_member_function_pointer_v<T>;

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

	//https://gist.github.com/Lee-R/3839813

	// FNV-1a 32bit hashing algorithm.
	constexpr uint32_t fnv1a_32(char const* s, size_t count)
	{
		return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}

	constexpr uint32_t fnv1a_32_s2(std::string_view v0, std::string_view v1, size_t count)
	{
		const auto& string = (count > v0.size() - 1) ? v1.data() : v0.data();
		size_t offset = (count > v0.size() - 1) ? v0.size() : 0;
		return ((count ? fnv1a_32_s2(v0, v1, count - 1) : 2166136261u) ^ string[count - offset]) * 16777619u;
	}

	constexpr uint32_t hash(std::string_view str)
	{
		return fnv1a_32(str.data(), str.length() - 1);
	}

	constexpr uint32_t hash(std::string_view v0, std::string_view v1)
	{
		return fnv1a_32_s2(v0, v1, v0.size() + v1.size() - 1);
	}

	static_assert(hash("class Foo") == hash("class ", "Foo"));

	template <typename T>
	constexpr std::string_view type_name()
	{
		constexpr auto wrapped_name = detail::wrapped_type_name<T>();
		constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}

	template <typename T>
	constexpr uint32_t type_id()
	{
		return hash(type_name<T>());
	}

	template <size_t size, typename T, typename... Types>
	constexpr void FillTypeIdsHelper(std::array<uint32_t, size>& types, size_t counter)
	{
		if constexpr (sizeof...(Types) != 0)
		{
			types[counter] = type_id<T>();
			FillTypeIdsHelper<size, Types...>(types, ++counter);
		}
		else
		{
			types[counter] = type_id<T>();
		}
	}

	template <typename... Types>
	constexpr std::array<uint32_t, sizeof...(Types)> Type_ids() requires(sizeof...(Types) >= 2)
	{
		std::array<uint32_t, sizeof...(Types)> types;
		FillTypeIdsHelper<sizeof...(Types), Types...>(types, 0);
		return types;
	}

	template <_Function Function>
	constexpr std::string_view function_return_type_name()
	{
		auto name = type_name<Function>();
		return name.substr(0, name.find('('));
	}

	template <_Function Function>
	constexpr uint32_t function_return_type_id()
	{
		return hash(function_return_type_name<Function>());
	}

	template <_Function Function>
	constexpr size_t function_parameter_amount()
	{
		auto name = type_name<Function>();
		if (name.find("(void)") != name.npos)
			return 0;
		size_t counter{1};
		for (auto& character : name)
			if (character == ',')
				++counter;
		return counter;
	}

	template <_Function Function, size_t pos = 0>
	constexpr std::string_view function_parameter_type_name()
	{
		static_assert(pos < function_parameter_amount<Function>());
		auto name = type_name<Function>();
		auto start = name.find('(') + 1;
		auto end = name.substr(start).find(',');
		size_t counter{ pos };
		if (end == name.npos)
		{
			return name.substr(start, name.size() - start - 1);
		}
		while (counter > 0)
		{
			start += end + 1;
			end = name.substr(start).find(',');
			if (end == name.npos)
				end = name.substr(start).find(')');
			--counter;
		}
		return name.substr(start, end);
	}

	template <_Function Function, size_t pos = 0>
	constexpr uint32_t function_parameter_type_id()
	{
		return hash(function_parameter_type_name<Function, pos>());
	}

	template <_Member_function_pointer T>
	constexpr std::string_view member_function_class_type_name()
	{
		auto name = type_name<T>();
		std::string_view functionIdentifier{ "__cdecl " };

		auto offset = name.find(functionIdentifier) + functionIdentifier.size();
		auto end = name.find("::* )");
		return name.substr(offset, end - offset);
	}

	template <_Member_function_pointer T>
	constexpr uint32_t member_function_class_type_id()
	{
		return hash("class ", member_function_class_type_name<T>());
	}

	template <typename TypePointer, typename Type>
	constexpr std::string_view class_member_object_type_name()
	{
		static_assert(std::is_member_pointer_v<TypePointer> && std::is_object_v<Type>);
		auto pointerName = type_name<TypePointer>();
		auto typeName = type_name<Type>();
		size_t size = pointerName.rfind("::");
		return pointerName.substr(typeName.size(), size - typeName.size());
	}

	template <typename TypePointer, typename Type>
	constexpr uint32_t class_member_object_type_id()
	{
		return hash("class ", class_member_object_type_id<TypePointer, Type>());
	}

	template <_Function Function>
	constexpr std::array<std::string_view, function_parameter_amount<Function>()> function_parameters_type_name()
	{
		constexpr size_t paramAmount{ function_parameter_amount<Function>() };
		static_assert(paramAmount != 0, "The given function has to have at least 1 parameters");

		std::array<std::string_view, paramAmount> paramArray;
		auto name = type_name<Function>();
		auto start = name.find('(') + 1;
		auto end = name.substr(start).find(',');
		if (end == name.npos)
		{
			paramArray[0] = name.substr(start, name.size() - start - 1);
			return paramArray;
		}
		paramArray[0] = name.substr(start, end);
		for (size_t i{1}; i < paramAmount; ++i)
		{
			start += end + 1;
			end = name.substr(start).find(',');
			if (end == name.npos)
				end = name.substr(start).find(')');
			paramArray[i] = name.substr(start, end);
		}
		return paramArray;
	}

	template <_Function Function>
	constexpr std::array<uint32_t, function_parameter_amount<Function>()> function_parameters_type_id()
	{
		constexpr size_t paramAmount{ function_parameter_amount<Function>() };
		std::array<uint32_t, paramAmount> paramIds;
		auto paramNames = function_parameter_amount<Function>();
		for (auto& name : paramNames)
		{
			paramIds = hash(name);
		}
		return paramIds;
	}

	//template <typename Function>
	//std::array<uint32_t, function_parameter_amount<Function>()> function_parameters_id()
	//{
	//	constexpr size_t paramAmount{ function_parameter_amount<Function>() };
	//	std::array<uint32_t, paramAmount> paramArray{};
	//	for (size_t i{}; i < paramAmount; ++i)
	//	{
	//		paramArray[i] = hash(function_parameter_type_name<Function, i>());
	//	}
	//}

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