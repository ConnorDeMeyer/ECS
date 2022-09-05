#pragma once

#include <GL/glew.h>
#include <filesystem>
#include <glm/glm.hpp>

/**
 * layout (std140) uniform Globals
 * {\n
 *	mat3 view	// 48\n
 *	mat3 invView	// 96\n
 *	float time	// 100\n
 *	float deltaTime	// 104\n
 * }
 */
struct GLSL_GlobalVariables
{
	glm::mat3x4 view;
	glm::mat3x4 invView;
	float time;
	float deltaTime;
};
static_assert(sizeof(GLSL_GlobalVariables) == 104);

enum class ShaderType : GLenum
{
	VertexShader			= GL_VERTEX_SHADER,
	FragmentShader			= GL_FRAGMENT_SHADER,
	GeometryShader			= GL_GEOMETRY_SHADER,
	ComputeShader			= GL_COMPUTE_SHADER,
	TessControlShader		= GL_TESS_CONTROL_SHADER,
	TessEvaluationShader	= GL_TESS_EVALUATION_SHADER,
};

class Shader
{
public:

	Shader(const std::filesystem::path& fragmentPath = {}, const std::filesystem::path& vertexPath = {}, const std::filesystem::path& geometryPath = {});
	Shader(const char* fragmentCode = {}, const char* vertexCode = {}, const char* geometryCode = {});
	virtual ~Shader();

	Shader(const Shader&)				= delete;
	Shader(Shader&&)					= delete;
	Shader& operator=(const Shader&)	= delete;
	Shader& operator=(Shader&&)			= delete;

public:

	GLint GetId() const { return m_Id; }

	void SetActive() const;

	GLint GetUniformLocation(const char* name)							const;
	GLint GetUniformLocation(const std::string& name)					const;

	void SetBool(	const char* name, bool value)						const;
	void SetInt(	const char* name, int value)						const;
	void SetUInt(	const char* name, uint32_t value)					const;
	void SetFloat(	const char* name, float value)						const;
	void SetVec2(	const char* name, const glm::vec2& value)			const;
	void SetVec3(	const char* name, const glm::vec3& value)			const;
	void SetVec4(	const char* name, const glm::vec4& value)			const;
	void SetIVec2(	const char* name, const glm::ivec2& value)			const;
	void SetIVec3(	const char* name, const glm::ivec3& value)			const;
	void SetIVec4(	const char* name, const glm::ivec4& value)			const;
	void SetUVec2(	const char* name, const glm::uvec2& value)			const;
	void SetUVec3(	const char* name, const glm::uvec3& value)			const;
	void SetUVec4(	const char* name, const glm::uvec4& value)			const;
	void SetMat2(	const char* name, const glm::mat2& value)			const;
	void SetMat3(	const char* name, const glm::mat3& value)			const;
	void SetMat4(	const char* name, const glm::mat4& value)			const;
	void SetMat2x3(	const char* name, const glm::mat2x3& value)			const;
	void SetMat2x4(	const char* name, const glm::mat2x4& value)			const;
	void SetMat3x2(	const char* name, const glm::mat3x2& value)			const;
	void SetMat3x4(	const char* name, const glm::mat3x4& value)			const;
	void SetMat4x2(	const char* name, const glm::mat4x2& value)			const;
	void SetMat4x3(	const char* name, const glm::mat4x3& value)			const;

	void SetBool(	const std::string& name, bool value)				const;
	void SetInt(	const std::string& name, int value)					const;
	void SetUInt(	const std::string& name, uint32_t value)			const;
	void SetFloat(	const std::string& name, float value)				const;
	void SetVec2(	const std::string& name, const glm::vec2& value)	const;
	void SetVec3(	const std::string& name, const glm::vec3& value)	const;
	void SetVec4(	const std::string& name, const glm::vec4& value)	const;
	void SetIVec2(	const std::string& name, const glm::ivec2& value)	const;
	void SetIVec3(	const std::string& name, const glm::ivec3& value)	const;
	void SetIVec4(	const std::string& name, const glm::ivec4& value)	const;
	void SetUVec2(	const std::string& name, const glm::uvec2& value)	const;
	void SetUVec3(	const std::string& name, const glm::uvec3& value)	const;
	void SetUVec4(	const std::string& name, const glm::uvec4& value)	const;
	void SetMat2(	const std::string& name, const glm::mat2& value)	const;
	void SetMat3(	const std::string& name, const glm::mat3& value)	const;
	void SetMat4(	const std::string& name, const glm::mat4& value)	const;
	void SetMat2x3(	const std::string& name, const glm::mat2x3& value)	const;
	void SetMat2x4(	const std::string& name, const glm::mat2x4& value)	const;
	void SetMat3x2(	const std::string& name, const glm::mat3x2& value)	const;
	void SetMat3x4(	const std::string& name, const glm::mat3x4& value)	const;
	void SetMat4x2(	const std::string& name, const glm::mat4x2& value)	const;
	void SetMat4x3(	const std::string& name, const glm::mat4x3& value)	const;

protected:



private:

	GLint m_Id{};

};

[[nodiscard]] GLint CompileShader(const char* code, ShaderType type);
[[nodiscard]] std::string GetShaderCode(const std::filesystem::path& path);

void SetUniformValue(GLint location, bool value);
void SetUniformValue(GLint location, int value);
void SetUniformValue(GLint location, uint32_t value);
void SetUniformValue(GLint location, float value);
void SetUniformValue(GLint location, const glm::vec2& value);
void SetUniformValue(GLint location, const glm::vec3& value);
void SetUniformValue(GLint location, const glm::vec4& value);
void SetUniformValue(GLint location, const glm::ivec2& value);
void SetUniformValue(GLint location, const glm::ivec3& value);
void SetUniformValue(GLint location, const glm::ivec4& value);
void SetUniformValue(GLint location, const glm::uvec2& value);
void SetUniformValue(GLint location, const glm::uvec3& value);
void SetUniformValue(GLint location, const glm::uvec4& value);
void SetUniformValue(GLint location, const glm::mat2& value);
void SetUniformValue(GLint location, const glm::mat3& value);
void SetUniformValue(GLint location, const glm::mat4& value);
void SetUniformValue(GLint location, const glm::mat2x3& value);
void SetUniformValue(GLint location, const glm::mat2x4& value);
void SetUniformValue(GLint location, const glm::mat3x2& value);
void SetUniformValue(GLint location, const glm::mat3x4& value);
void SetUniformValue(GLint location, const glm::mat4x2& value);
void SetUniformValue(GLint location, const glm::mat4x3& value);

