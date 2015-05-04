#include "OpenGL.h"
#include "../Windows/Bitmap.h"
#include "../Windows/Debug.h"
namespace EDX
{
	namespace OpenGL
	{
		PFNGLCREATESHADERPROC							glCreateShader;
		PFNGLCREATEPROGRAMPROC							glCreateProgram;
		PFNGLDELETESHADERPROC							glDeleteShader;
		PFNGLDELETEPROGRAMPROC							glDeleteProgram;
		PFNGLATTACHSHADERPROC							glAttachShader;
		PFNGLDETACHSHADERPROC							glDetachShader;
		PFNGLSHADERSOURCEPROC							glShaderSource;
		PFNGLCOMPILESHADERPROC							glCompileShader;
		PFNGLLINKPROGRAMPROC							glLinkProgram;
		PFNGLVALIDATEPROGRAMPROC						glValidateProgram;
		PFNGLUSEPROGRAMPROC								glUseProgram;
		PFNGLGETSHADERINFOLOGPROC						glGetShaderInfoLog;
		PFNGLGETPROGRAMINFOLOGPROC						glGetProgramInfoLog;
		PFNGLGETSHADERIVPROC							glGetShaderiv;
		PFNGLGETPROGRAMIVPROC							glGetProgramiv;
		PFNGLUNIFORM1FPROC								glUniform1f;
		PFNGLUNIFORM1FVPROC								glUniform1fv;
		PFNGLUNIFORM2FPROC								glUniform2f;
		PFNGLUNIFORM2FVPROC								glUniform2fv;
		PFNGLUNIFORM3FPROC								glUniform3f;
		PFNGLUNIFORM4FPROC								glUniform4f;
		PFNGLUNIFORM1IPROC								glUniform1i;
		PFNGLUNIFORM1IVPROC								glUniform1iv;
		PFNGLUNIFORM2IPROC								glUniform2i;
		PFNGLUNIFORM3IPROC								glUniform3i;
		PFNGLUNIFORM4IPROC								glUniform4i;
		PFNGLUNIFORMMATRIX3FVPROC						glUniformMatrix3fv;
		PFNGLUNIFORMMATRIX4FVPROC						glUniformMatrix4fv;
		PFNGLGETUNIFORMLOCATIONPROC						glGetUniformLocation;
		PFNGLBINDFRAGDATALOCATIONEXTPROC				glBindFragDataLocation;
		PFNGLISRENDERBUFFEREXTPROC						glIsRenderbuffer;
		PFNGLBINDRENDERBUFFEREXTPROC					glBindRenderbuffer;
		PFNGLDELETERENDERBUFFERSEXTPROC					glDeleteRenderbuffers;
		PFNGLGENRENDERBUFFERSEXTPROC					glGenRenderbuffers;
		PFNGLRENDERBUFFERSTORAGEEXTPROC					glRenderbufferStorage;
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		glRenderbufferStorageMultisample;
		PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC			glGetRenderbufferParameteriv;
		PFNGLISFRAMEBUFFEREXTPROC						glIsFramebuffer;
		PFNGLBINDFRAMEBUFFEREXTPROC						glBindFramebuffer;
		PFNGLDELETEFRAMEBUFFERSEXTPROC					glDeleteFramebuffers;
		PFNGLGENFRAMEBUFFERSEXTPROC						glGenFramebuffers;
		PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC				glCheckFramebufferStatus;
		PFNGLFRAMEBUFFERTEXTURE1DEXTPROC				glFramebufferTexture1D;
		PFNGLFRAMEBUFFERTEXTURE2DEXTPROC				glFramebufferTexture2D;
		PFNGLFRAMEBUFFERTEXTURE3DEXTPROC				glFramebufferTexture3D;
		PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC				glFramebufferRenderbuffer;
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC	glGetFramebufferAttachmentParameteriv;
		PFNGLBLITFRAMEBUFFEREXTPROC						glBlitFramebuffer;
		PFNGLGENERATEMIPMAPEXTPROC						glGenerateMipmap;
		PFNGLDRAWBUFFERSPROC							glDrawBuffers;
		PFNGLDRAWRANGEELEMENTSPROC						glDrawRangeElements;
		PFNGLACTIVETEXTUREPROC							glActiveTexture;
		PFNGLCLIENTACTIVETEXTUREPROC					glClientActiveTexture;
		PFNGLMULTITEXCOORD1FPROC						glMultiTexcoord1f;
		PFNGLMULTITEXCOORD2FPROC						glMultiTexcoord2f;
		PFNGLMULTITEXCOORD3FPROC						glMultiTexcoord3f;
		PFNGLMULTITEXCOORD4FPROC						glMultiTexcoord4f;
		PFNGLLOADTRANSPOSEMATRIXFPROC					glLoadTransposeMatrixf;
		PFNGLBINDBUFFERPROC								glBindBuffer;
		PFNGLBUFFERDATAPROC								glBufferData;
		PFNGLBUFFERSUBDATAPROC							glBufferSubData;
		PFNGLDELETEBUFFERSPROC							glDeleteBuffers;
		PFNGLGENBUFFERSPROC								glGenBuffers;
		PFNGLMAPBUFFERPROC								glMapBuffer;
		PFNGLUNMAPBUFFERPROC							glUnmapBuffer;
		PFNGLBLENDEQUATIONPROC							glBlendEquation;
		PFNGLBLENDCOLORPROC								glBlendColor;
		PFNGLBLENDFUNCSEPARATEPROC						glBlendFuncSeperate;
		PFNGLBLENDEQUATIONSEPARATEPROC					glBlendEquationSeperate;

		void InitializeOpenGLExtensions()
		{
			glCreateShader = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
			glCreateProgram = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
			glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
			glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
			glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
			glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
			glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
			glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
			glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
			glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wglGetProcAddress("glValidateProgram");
			glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
			glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
			glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
			glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
			glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
			glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
			glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
			glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
			glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
			glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
			glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
			glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
			glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
			glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");
			glUniform3i = (PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i");
			glUniform4i = (PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i");
			glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
			glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
			glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
			glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONEXTPROC)wglGetProcAddress("glBindFragDataLocationEXT");

			glIsRenderbuffer = (PFNGLISRENDERBUFFEREXTPROC)wglGetProcAddress("glIsRenderbuffer");
			glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbuffer");
			glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffers");
			glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffers");
			glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorage");
			glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
			glIsFramebuffer = (PFNGLISFRAMEBUFFEREXTPROC)wglGetProcAddress("glIsFramebuffer");
			glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebuffer");
			glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffers");
			glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffers");
			glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatus");
			glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)wglGetProcAddress("glFramebufferTexture1D");
			glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2D");
			glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)wglGetProcAddress("glFramebufferTexture3D");
			glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbuffer");
			glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
			glBlitFramebuffer = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebuffer");
			glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
			glGenerateMipmap = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmap");
			glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
			glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)wglGetProcAddress("glDrawRangeElements");

			glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
			glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
			glMultiTexcoord1f = (PFNGLMULTITEXCOORD1FPROC)wglGetProcAddress("glMultiTexCoord1f");
			glMultiTexcoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
			glMultiTexcoord3f = (PFNGLMULTITEXCOORD3FPROC)wglGetProcAddress("glMultiTexCoord3f");
			glMultiTexcoord4f = (PFNGLMULTITEXCOORD4FPROC)wglGetProcAddress("glMultiTexCoord4f");
			glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC)wglGetProcAddress("glLoadTransposeMatrixf");

			glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
			glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
			glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
			glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");

			glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
			glBlendColor = (PFNGLBLENDCOLORPROC)wglGetProcAddress("glBlendColor");
			glBlendFuncSeperate = (PFNGLBLENDFUNCSEPARATEPROC)wglGetProcAddress("glBlendFuncSeperate");
			glBlendEquationSeperate = (PFNGLBLENDEQUATIONSEPARATEPROC)wglGetProcAddress("glBlendEquationSeperate");
		}

		void Shader::Load(ShaderType shaderType, const char* source)
		{
			mHandle = glCreateShader((int)shaderType);
			int length = (int)strlen(source);
			glShaderSource(mHandle, 1, &source, &length);
			glCompileShader(mHandle);

			glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			vector<char> buffer;
			buffer.resize(length);
			glGetShaderInfoLog(mHandle, length, &length, buffer.data());
			int compileStatus;
			glGetShaderiv(mHandle, GL_COMPILE_STATUS, &compileStatus);

			if (length > 0)
				Debug::WriteLine(buffer.data());

			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}
		}

		Shader::~Shader()
		{
			glDeleteShader(mHandle);
		}

		Program::Program()
		{
			mHandle = glCreateProgram();
		}

		Program::~Program()
		{
			glDeleteProgram(mHandle);
		}

		void Program::AttachShader(Shader * shader)
		{
			glAttachShader(mHandle, shader->GetHandle());
		}

		void Program::Link()
		{
			int length, compileStatus;
			vector<char> buffer;

			glLinkProgram(mHandle);

			glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			buffer.resize(length);
			glGetProgramInfoLog(mHandle, length, &length, buffer.data());
			glGetProgramiv(mHandle, GL_LINK_STATUS, &compileStatus);

			if (length > 0)
				Debug::WriteLine(buffer.data());

			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}

			glValidateProgram(mHandle);
			glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &length);
			buffer.resize(length);
			glGetProgramInfoLog(mHandle, length, &length, buffer.data());
			if (length > 0)
				Debug::WriteLine(buffer.data());

			glGetProgramiv(mHandle, GL_VALIDATE_STATUS, &compileStatus);
			if (compileStatus != GL_TRUE)
			{
				throw ShaderCompileException(buffer.data());
			}
		}

		void Program::SetUniform(const char * name, int value)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform1i(loc, value);
		}

		void Program::SetUniform(const char * name, float value)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform1f(loc, value);
		}

		void Program::SetUniform(const char * name, const Vector3& value)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform3f(loc, value.x, value.y, value.z);
		}

		void Program::SetUniform(const char * name, const Vector4& value)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform4f(loc, value.x, value.y, value.z, value.w);
		}

		void Program::SetUniform(const char * name, const int* value, int count)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform1iv(loc, count, value);
		}

		void Program::SetUniform(const char * name, const float* value, int count)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform1fv(loc, count, value);
		}

		void Program::SetUniform(const char * name, const Vector2* value, int count)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniform2fv(loc, count, (float*)value);
		}

		void Program::SetUniform(const char * name, const Matrix& value, bool transpose)
		{
			int loc = glGetUniformLocation(mHandle, name);
			glUniformMatrix4fv(loc, 1, transpose, (float*)value.m);
		}

		void Program::BindFragDataLocation(const char * name, int loc)
		{
			glBindFragDataLocation(mHandle, loc, name);
		}

		void Program::Use()
		{
			glUseProgram(mHandle);
		}

		void Program::Unuse()
		{
			glUseProgram(0);
		}

		Color4b GammaCorrect(Color4b tIn, float fGamma = 1.0f)
		{
			return Color4b(Math::Pow(Color(tIn), fGamma));
		}

		Texture2D * Texture2D::Create(const char* fileName)
		{
			int width, height;
			int channel;
			Color4b* pRawTex = Bitmap::ReadFromFile<Color4b>(fileName, &width, &height, &channel);
			if (!pRawTex)
			{
				throw std::exception("Texture file load failed.");
			}
			for (auto i = 0; i < width * height; i++)
				pRawTex[i] = GammaCorrect(pRawTex[i]);

			auto rs = new Texture2D();
			rs->Load(ImageFormat::RGBA, ImageFormat::RGBA, ImageDataType::Byte, pRawTex, width, height);

			SafeDeleteArray(pRawTex);

			return rs;
		}
	}
}