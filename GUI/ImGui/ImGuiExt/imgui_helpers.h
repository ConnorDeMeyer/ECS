#pragma once

//Forward Declarations
//namespace glm
//{
//	enum precision : int;
//
//	template <typename T, precision P> struct tvec1;
//	template <typename T, precision P> struct tvec2;
//	template <typename T, precision P> struct tvec3;
//	template <typename T, precision P> struct tvec4;
//
//	template <typename T, precision P> struct tvec2;
//	template <typename T, precision P> struct tvec3;
//	template <typename T, precision P> struct tvec4;
//	template <typename T, precision P> struct tmat2x2;
//	template <typename T, precision P> struct tmat2x3;
//	template <typename T, precision P> struct tmat2x4;
//	template <typename T, precision P> struct tmat3x2;
//	template <typename T, precision P> struct tmat3x3;
//	template <typename T, precision P> struct tmat3x4;
//	template <typename T, precision P> struct tmat4x2;
//	template <typename T, precision P> struct tmat4x3;
//	template <typename T, precision P> struct tmat4x4;
//
//	typedef tvec1<float,	precision(0)>	highp_vec1;
//	typedef tvec1<double,	precision(0)>	highp_dvec1;
//	typedef tvec1<int,		precision(0)>	highp_ivec1;
//	typedef tvec1<unsigned, precision(0)>	highp_uvec1;
//	typedef tvec1<bool,		precision(0)>	highp_bvec1;
//
//	typedef tvec2<float,	precision(0)>	highp_vec2;
//	typedef tvec2<double,	precision(0)>	highp_dvec2;
//	typedef tvec2<int,		precision(0)>	highp_ivec2;
//	typedef tvec2<unsigned, precision(0)>	highp_uvec2;
//	typedef tvec2<bool,		precision(0)>	highp_bvec2;
//
//	typedef tvec3<float,	precision(0)>	highp_vec3;
//	typedef tvec3<double,	precision(0)>	highp_dvec3;
//	typedef tvec3<int,		precision(0)>	highp_ivec3;
//	typedef tvec3<unsigned, precision(0)>	highp_uvec3;
//	typedef tvec3<bool,		precision(0)>	highp_bvec3;
//
//	typedef tvec4<float,	precision(0)>	highp_vec4;
//	typedef tvec4<double,	precision(0)>	highp_dvec4;
//	typedef tvec4<int,		precision(0)>	highp_ivec4;
//	typedef tvec4<unsigned, precision(0)>	highp_uvec4;
//	typedef tvec4<bool,		precision(0)>	highp_bvec4;
//
//	typedef tmat2x2<float,	precision(0)>	highp_mat2x2;
//	typedef tmat2x3<float,	precision(0)>	highp_mat2x3;
//	typedef tmat2x4<float,	precision(0)>	highp_mat2x4;
//	typedef tmat3x2<float,	precision(0)>	highp_mat3x2;
//	typedef tmat3x3<float,	precision(0)>	highp_mat3x3;
//	typedef tmat3x4<float,	precision(0)>	highp_mat3x4;
//	typedef tmat4x2<float,	precision(0)>	highp_mat4x2;
//	typedef tmat4x3<float,	precision(0)>	highp_mat4x3;
//	typedef tmat4x4<float,	precision(0)>	highp_mat4x4;
//
//	typedef highp_vec2			vec2;
//	typedef highp_vec3			vec3;
//	typedef highp_vec4			vec4;
//	typedef highp_ivec2			ivec2;
//	typedef highp_ivec3			ivec3;
//	typedef highp_ivec4			ivec4;
//	typedef highp_mat2x2		mat2x2;
//	typedef highp_mat2x3		mat2x3;
//	typedef highp_mat2x4		mat2x4;
//	typedef highp_mat3x2		mat3x2;
//	typedef highp_mat3x3		mat3x3;
//	typedef highp_mat3x4		mat3x4;
//	typedef highp_mat4x2		mat4x2;
//	typedef highp_mat4x3		mat4x3;
//	typedef highp_mat4x4		mat4x4;
//}

#if __has_include(<glm/glm.hpp>)
#include <glm/glm.hpp>
namespace ImGui
{
	// GLM types

	bool CustomInput(const char* label, glm::vec2& input);
	bool CustomInput(const char* label, glm::vec3& input);
	bool CustomInput(const char* label, glm::vec4& input);
	bool CustomInput(const char* label, glm::ivec2& input);
	bool CustomInput(const char* label, glm::ivec3& input);
	bool CustomInput(const char* label, glm::ivec4& input);
	bool CustomInput(const char* label, glm::mat2x2& input);
	bool CustomInput(const char* label, glm::mat2x3& input);
	bool CustomInput(const char* label, glm::mat2x4& input);
	bool CustomInput(const char* label, glm::mat3x2& input);
	bool CustomInput(const char* label, glm::mat3x3& input);
	bool CustomInput(const char* label, glm::mat3x4& input);
	bool CustomInput(const char* label, glm::mat4x2& input);
	bool CustomInput(const char* label, glm::mat4x3& input);
	bool CustomInput(const char* label, glm::mat4x4& input);
}
#endif

struct SDL_Rect;
struct SDL_FRect;
struct SDL_Color;

namespace ImGui 
{
	// Fundamental C++ types

	bool CustomInput(const char* label, short& input);
	bool CustomInput(const char* label, unsigned short& input);
	bool CustomInput(const char* label, int& input);
	bool CustomInput(const char* label, unsigned int& input);
	bool CustomInput(const char* label, long long& input);
	bool CustomInput(const char* label, unsigned long long& input);
	bool CustomInput(const char* label, bool& input);
	bool CustomInput(const char* label, char& input);
	bool CustomInput(const char* label, unsigned char& input);
	bool CustomInput(const char* label, float& input);
	bool CustomInput(const char* label, double& input);

	// SDL types

	bool CustomInput(const char* label, SDL_Rect& input);
	bool CustomInput(const char* label, SDL_FRect& input);
	bool CustomInput(const char* label, SDL_Color& input);

}
