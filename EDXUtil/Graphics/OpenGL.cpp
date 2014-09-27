#include "OpenGL.h"
#include "../Windows/Bitmap.h"
#include "../Windows/Debug.h"
namespace EDX
{
	namespace OpenGL
	{
		PFNGLCREATESHADERPROC							GL::glCreateShader;
		PFNGLCREATEPROGRAMPROC							GL::glCreateProgram;
		PFNGLDELETESHADERPROC							GL::glDeleteShader;
		PFNGLDELETEPROGRAMPROC							GL::glDeleteProgram;
		PFNGLATTACHSHADERPROC							GL::glAttachShader;
		PFNGLDETACHSHADERPROC							GL::glDetachShader;
		PFNGLSHADERSOURCEPROC							GL::glShaderSource;
		PFNGLCOMPILESHADERPROC							GL::glCompileShader;
		PFNGLLINKPROGRAMPROC							GL::glLinkProgram;
		PFNGLVALIDATEPROGRAMPROC						GL::glValidateProgram;
		PFNGLUSEPROGRAMPROC								GL::glUseProgram;
		PFNGLGETSHADERINFOLOGPROC						GL::glGetShaderInfoLog;
		PFNGLGETPROGRAMINFOLOGPROC						GL::glGetProgramInfoLog;
		PFNGLGETSHADERIVPROC							GL::glGetShaderiv;
		PFNGLGETPROGRAMIVPROC							GL::glGetProgramiv;
		PFNGLUNIFORM1FPROC								GL::glUniform1f;
		PFNGLUNIFORM2FPROC								GL::glUniform2f;
		PFNGLUNIFORM3FPROC								GL::glUniform3f;
		PFNGLUNIFORM4FPROC								GL::glUniform4f;
		PFNGLUNIFORM1IPROC								GL::glUniform1i;
		PFNGLUNIFORM1IVPROC								GL::glUniform1iv;
		PFNGLUNIFORM2IPROC								GL::glUniform2i;
		PFNGLUNIFORM3IPROC								GL::glUniform3i;
		PFNGLUNIFORM4IPROC								GL::glUniform4i;
		PFNGLUNIFORMMATRIX3FVPROC						GL::glUniformMatrix3fv;
		PFNGLUNIFORMMATRIX4FVPROC						GL::glUniformMatrix4fv;
		PFNGLGETUNIFORMLOCATIONPROC						GL::glGetUniformLocation;
		PFNGLBINDFRAGDATALOCATIONEXTPROC				GL::glBindFragDataLocation;
		PFNGLISRENDERBUFFEREXTPROC						GL::glIsRenderbuffer;
		PFNGLBINDRENDERBUFFEREXTPROC					GL::glBindRenderbuffer;
		PFNGLDELETERENDERBUFFERSEXTPROC					GL::glDeleteRenderbuffers;
		PFNGLGENRENDERBUFFERSEXTPROC					GL::glGenRenderbuffers;
		PFNGLRENDERBUFFERSTORAGEEXTPROC					GL::glRenderbufferStorage;
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		GL::glRenderbufferStorageMultisample;
		PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC			GL::glGetRenderbufferParameteriv;
		PFNGLISFRAMEBUFFEREXTPROC						GL::glIsFramebuffer;
		PFNGLBINDFRAMEBUFFEREXTPROC						GL::glBindFramebuffer;
		PFNGLDELETEFRAMEBUFFERSEXTPROC					GL::glDeleteFramebuffers;
		PFNGLGENFRAMEBUFFERSEXTPROC						GL::glGenFramebuffers;
		PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC				GL::glCheckFramebufferStatus;
		PFNGLFRAMEBUFFERTEXTURE1DEXTPROC				GL::glFramebufferTexture1D;
		PFNGLFRAMEBUFFERTEXTURE2DEXTPROC				GL::glFramebufferTexture2D;
		PFNGLFRAMEBUFFERTEXTURE3DEXTPROC				GL::glFramebufferTexture3D;
		PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC				GL::glFramebufferRenderbuffer;
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC	GL::glGetFramebufferAttachmentParameteriv;
		PFNGLBLITFRAMEBUFFEREXTPROC						GL::glBlitFramebuffer;
		PFNGLGENERATEMIPMAPEXTPROC						GL::glGenerateMipmap;
		PFNGLDRAWBUFFERSPROC							GL::glDrawBuffers;
		PFNGLACTIVETEXTUREPROC							GL::glActiveTexture;
		PFNGLCLIENTACTIVETEXTUREPROC					GL::glClientActiveTexture;
		PFNGLMULTITEXCOORD1FPROC						GL::glMultiTexcoord1f;
		PFNGLMULTITEXCOORD2FPROC						GL::glMultiTexcoord2f;
		PFNGLMULTITEXCOORD3FPROC						GL::glMultiTexcoord3f;
		PFNGLMULTITEXCOORD4FPROC						GL::glMultiTexcoord4f;
		PFNGLBINDBUFFERPROC								GL::glBindBuffer;
		PFNGLBUFFERDATAPROC								GL::glBufferData;
		PFNGLBUFFERSUBDATAPROC							GL::glBufferSubData;
		PFNGLDELETEBUFFERSPROC							GL::glDeleteBuffers;
		PFNGLGENBUFFERSPROC								GL::glGenBuffers;
		PFNGLMAPBUFFERPROC								GL::glMapBuffer;
		PFNGLUNMAPBUFFERPROC							GL::glUnmapBuffer;
		PFNGLBLENDEQUATIONPROC							GL::glBlendEquation;
		PFNGLBLENDCOLORPROC								GL::glBlendColor;
		PFNGLBLENDFUNCSEPARATEPROC						GL::glBlendFuncSeperate;
		PFNGLBLENDEQUATIONSEPARATEPROC					GL::glBlendEquationSeperate;

		void InitializeOpenGLExtensions()
		{
			GL::LoadGLExtensions();
		}

		void Shader::Load(ShaderType shaderType, const char* source)
		{
			mHandle = GL::glCreateShader((int)shaderType);
			int length = (int)strlen(source);
			GL::glShaderSource(mHandle, 1, &source, &length);
			GL::glCompileShader(mHandle);

			GL::glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			vector<char> buffer;
			buffer.resize(length);
			GL::glGetShaderInfoLog(mHandle, length, &length, buffer.data());
			int compileStatus;
			GL::glGetShaderiv(mHandle, GL_COMPILE_STATUS, &compileStatus);

			if (length > 0)
				Debug::WriteLine(buffer.data());

			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}
		}

		Shader::~Shader()
		{
			GL::glDeleteShader(mHandle);
		}

		Program::Program()
		{
			mHandle = GL::glCreateProgram();
		}

		Program::~Program()
		{
			GL::glDeleteProgram(mHandle);
		}

		void Program::AttachShader(Shader * shader)
		{
			GL::glAttachShader(mHandle, shader->GetHandle());
		}

		void Program::Link()
		{
			int length, compileStatus;
			vector<char> buffer;

			GL::glLinkProgram(mHandle);

			GL::glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			buffer.resize(length);
			GL::glGetProgramInfoLog(mHandle, length, &length, buffer.data());
			GL::glGetProgramiv(mHandle, GL_LINK_STATUS, &compileStatus);

			if (length > 0)
				Debug::WriteLine(buffer.data());

			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}

			GL::glValidateProgram(mHandle);
			GL::glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			buffer.resize(length);
			GL::glGetProgramInfoLog(mHandle, length, &length, buffer.data());
			if (length > 0)
				Debug::WriteLine(buffer.data());

			GL::glGetProgramiv(mHandle, GL_VALIDATE_STATUS, &compileStatus);
			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}
		}

		void Program::SetUniform(const char * name, int value)
		{
			int loc = GL::glGetUniformLocation(mHandle, name);
			GL::glUniform1i(loc, value);
		}

		void Program::SetUniform(const char * name, float value)
		{
			int loc = GL::glGetUniformLocation(mHandle, name);
			GL::glUniform1f(loc, value);
		}

		void Program::SetUniform(const char * name, const Vector3& value)
		{
			int loc = GL::glGetUniformLocation(mHandle, name);
			GL::glUniform3f(loc, value.x, value.y, value.z);
		}

		void Program::SetUniform(const char * name, const Vector4& value)
		{
			int loc = GL::glGetUniformLocation(mHandle, name);
			GL::glUniform4f(loc, value.x, value.y, value.z, value.w);
		}

		void Program::SetUniform(const char * name, const Matrix& value, bool transpose)
		{
			int loc = GL::glGetUniformLocation(mHandle, name);
			GL::glUniformMatrix4fv(loc, 16, transpose, (float*)value.m);
		}

		void Program::BindFragDataLocation(const char * name, int loc)
		{
			GL::glBindFragDataLocation(mHandle, loc, name);
		}

		void Program::Use()
		{
			GL::glUseProgram(mHandle);
		}

		void Program::Unuse()
		{
			GL::glUseProgram(0);
		}

		Texture2D * Texture2D::Create(const char* fileName)
		{
			int width, height;
			int channel;
			_byte* pRawTex = Bitmap::ReadFromFileByte(fileName, &width, &height, &channel);
			if (!pRawTex)
			{
				throw std::exception("Texture file load failed.");
			}

			auto rs = new Texture2D();
			rs->Load(ImageFormat::RGBA, ImageFormat::RGBA, ImageDataType::Byte, pRawTex, width, height);

			return rs;
		}
	}
}