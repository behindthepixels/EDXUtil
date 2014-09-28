#pragma once

#include "../EDXPrerequisites.h"
#include "../Math/Vector.h"
#include "../Math/Matrix.h"
#include "Texture.h"

#include "../Windows/Base.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glext.h>

namespace EDX
{
	namespace OpenGL
	{
		class GL
		{
		public:
			// OpenGL Extension Loader
			static PFNGLCREATESHADERPROC							glCreateShader;
			static PFNGLCREATEPROGRAMPROC							glCreateProgram;
			static PFNGLDELETESHADERPROC							glDeleteShader;
			static PFNGLDELETEPROGRAMPROC							glDeleteProgram;
			static PFNGLATTACHSHADERPROC							glAttachShader;
			static PFNGLDETACHSHADERPROC							glDetachShader;
			static PFNGLSHADERSOURCEPROC							glShaderSource;
			static PFNGLCOMPILESHADERPROC							glCompileShader;
			static PFNGLLINKPROGRAMPROC								glLinkProgram;
			static PFNGLVALIDATEPROGRAMPROC							glValidateProgram;
			static PFNGLUSEPROGRAMPROC								glUseProgram;
			static PFNGLGETSHADERINFOLOGPROC						glGetShaderInfoLog;
			static PFNGLGETPROGRAMINFOLOGPROC						glGetProgramInfoLog;
			static PFNGLGETSHADERIVPROC								glGetShaderiv;
			static PFNGLGETPROGRAMIVPROC							glGetProgramiv;
			static PFNGLUNIFORM1FPROC								glUniform1f;
			static PFNGLUNIFORM1FVPROC								glUniform1fv;
			static PFNGLUNIFORM2FPROC								glUniform2f;
			static PFNGLUNIFORM2FVPROC								glUniform2fv;
			static PFNGLUNIFORM3FPROC								glUniform3f;
			static PFNGLUNIFORM4FPROC								glUniform4f;
			static PFNGLUNIFORM1IPROC								glUniform1i;
			static PFNGLUNIFORM1IVPROC								glUniform1iv;
			static PFNGLUNIFORM2IPROC								glUniform2i;
			static PFNGLUNIFORM3IPROC								glUniform3i;
			static PFNGLUNIFORM4IPROC								glUniform4i;
			static PFNGLUNIFORMMATRIX3FVPROC						glUniformMatrix3fv;
			static PFNGLUNIFORMMATRIX4FVPROC						glUniformMatrix4fv;
			static PFNGLGETUNIFORMLOCATIONPROC						glGetUniformLocation;
			static PFNGLBINDFRAGDATALOCATIONEXTPROC					glBindFragDataLocation;
			static PFNGLISRENDERBUFFEREXTPROC						glIsRenderbuffer;
			static PFNGLBINDRENDERBUFFEREXTPROC						glBindRenderbuffer;
			static PFNGLDELETERENDERBUFFERSEXTPROC					glDeleteRenderbuffers;
			static PFNGLGENRENDERBUFFERSEXTPROC						glGenRenderbuffers;
			static PFNGLRENDERBUFFERSTORAGEEXTPROC					glRenderbufferStorage;
			static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		glRenderbufferStorageMultisample;
			static PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC			glGetRenderbufferParameteriv;
			static PFNGLISFRAMEBUFFEREXTPROC						glIsFramebuffer;
			static PFNGLBINDFRAMEBUFFEREXTPROC						glBindFramebuffer;
			static PFNGLDELETEFRAMEBUFFERSEXTPROC					glDeleteFramebuffers;
			static PFNGLGENFRAMEBUFFERSEXTPROC						glGenFramebuffers;
			static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC				glCheckFramebufferStatus;
			static PFNGLFRAMEBUFFERTEXTURE1DEXTPROC					glFramebufferTexture1D;
			static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC					glFramebufferTexture2D;
			static PFNGLFRAMEBUFFERTEXTURE3DEXTPROC					glFramebufferTexture3D;
			static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC				glFramebufferRenderbuffer;
			static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC	glGetFramebufferAttachmentParameteriv;
			static PFNGLBLITFRAMEBUFFEREXTPROC						glBlitFramebuffer;
			static PFNGLGENERATEMIPMAPEXTPROC						glGenerateMipmap;
			static PFNGLDRAWBUFFERSPROC								glDrawBuffers;
			static PFNGLACTIVETEXTUREPROC							glActiveTexture;
			static PFNGLCLIENTACTIVETEXTUREPROC						glClientActiveTexture;
			static PFNGLMULTITEXCOORD1FPROC							glMultiTexcoord1f;
			static PFNGLMULTITEXCOORD2FPROC							glMultiTexcoord2f;
			static PFNGLMULTITEXCOORD3FPROC							glMultiTexcoord3f;
			static PFNGLMULTITEXCOORD4FPROC							glMultiTexcoord4f;
			static PFNGLBINDBUFFERPROC								glBindBuffer;
			static PFNGLBUFFERDATAPROC								glBufferData;
			static PFNGLBUFFERSUBDATAPROC							glBufferSubData;
			static PFNGLDELETEBUFFERSPROC							glDeleteBuffers;
			static PFNGLGENBUFFERSPROC								glGenBuffers;
			static PFNGLMAPBUFFERPROC								glMapBuffer;
			static PFNGLUNMAPBUFFERPROC								glUnmapBuffer;
			static PFNGLBLENDEQUATIONPROC							glBlendEquation;
			static PFNGLBLENDCOLORPROC								glBlendColor;
			static PFNGLBLENDFUNCSEPARATEPROC						glBlendFuncSeperate;
			static PFNGLBLENDEQUATIONSEPARATEPROC					glBlendEquationSeperate;

			static void LoadGLExtensions()
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
				glUniformMatrix4fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix4fv");
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

				glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
				glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
				glMultiTexcoord1f = (PFNGLMULTITEXCOORD1FPROC)wglGetProcAddress("glMultiTexCoord1f");
				glMultiTexcoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
				glMultiTexcoord3f = (PFNGLMULTITEXCOORD3FPROC)wglGetProcAddress("glMultiTexCoord3f");
				glMultiTexcoord4f = (PFNGLMULTITEXCOORD4FPROC)wglGetProcAddress("glMultiTexCoord4f");

				glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
				glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
				glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
				glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");

				glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
				glBlendColor = (PFNGLBLENDCOLORPROC)wglGetProcAddress("glBlendColor");
				glBlendFuncSeperate = (PFNGLBLENDFUNCSEPARATEPROC)wglGetProcAddress("glBlendFuncSeperate");
				glBlendEquationSeperate = (PFNGLBLENDEQUATIONSEPARATEPROC)wglGetProcAddress("glBlendEquationSeperate");
			}
		};

		class GLObject : public Object
		{
		protected:
			unsigned int mHandle;

		public:
			inline unsigned int GetHandle()
			{
				return mHandle;
			}
		};

		enum class ShaderType
		{
			VertexShader = 0x8B31, FragmentShader = 0x8B30
		};

		class ShaderCompileException : public std::exception
		{
		public:
			string mErrorInfo;
			ShaderCompileException(const string& error)
				: std::exception(("GLSL error: " + error).c_str())
			{
				mErrorInfo = error;
			}
		};

		class Shader : public GLObject
		{
		public:
			Shader()
			{
			}
			Shader(ShaderType shaderType, const char* source)
			{
				Load(shaderType, source);
			}
			void Load(ShaderType shaderType, const char* source);
			virtual ~Shader();
		};

		class Program : public GLObject
		{
		public:
			Program();
			virtual ~Program();

			void AttachShader(Shader * shader);
			void Link();
			void SetUniform(const char* name, int value);
			void SetUniform(const char* name, float value);
			void SetUniform(const char* name, const Vector3& value);
			void SetUniform(const char* name, const Vector4& value);
			void SetUniform(const char* name, const Matrix& value, bool transpose = false);
			void SetUniform(const char * name, const int* value, int count);
			void SetUniform(const char * name, const float* value, int count);
			void SetUniform(const char * name, const Vector2* value, int count);
			void BindFragDataLocation(const char * name, int loc);
			void Use();
			static void Unuse();
		};

		template<int Target>
		class Buffer : public GLObject
		{
		public:
			Buffer()
			{
				GL::glGenBuffers(1, &mHandle);
			}
			virtual ~Buffer()
			{
				GL::glDeleteBuffers(1, &mHandle);
			}
			void SetData(int sizeInBytes, void * data)
			{
				GL::glBindBuffer(Target, mHandle);
				GL::glBufferData(Target, sizeInBytes, data, GL_STATIC_DRAW);
			}
			void Bind()
			{
				GL::glBindBuffer(Target, mHandle);
			}
			static void UnBind()
			{
				GL::glBindBuffer(Target, 0);
			}
		};

		typedef Buffer<GL_ARRAY_BUFFER> VertexBuffer;
		typedef Buffer<GL_ELEMENT_ARRAY_BUFFER> IndexBuffer;
		typedef Buffer<GL_UNIFORM_BUFFER> UniformBuffer;
		typedef Buffer<GL_TEXTURE_BUFFER> TextureBuffer;
		typedef Buffer<GL_TRANSFORM_FEEDBACK_BUFFER> TransformFeedbackBuffer;

		enum class ImageDataType
		{
			Byte = GL_UNSIGNED_BYTE, Float = GL_FLOAT
		};

		enum class ImageFormat
		{
			Luminance = GL_LUMINANCE, RGB = GL_RGB, RGBA = GL_RGBA,
			RGBA_32F = GL_RGBA32F, RGBA_16F = GL_RGBA16F,
			R11_G11_B10 = GL_R11F_G11F_B10F,
			RGB10_A2 = GL_RGB10_A2,
			Depth16 = GL_DEPTH_COMPONENT16,
			Depth24 = GL_DEPTH_COMPONENT24,
			Depth32 = GL_DEPTH_COMPONENT32,
			DXT1_RGB = GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			DXT1_RGBA = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
			DXT3_RGBA = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			DXT5_RGBA = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
		};

		template<int Target>
		class Texture : public GLObject
		{
		public:
			Texture()
			{
				glGenTextures(1, &mHandle);
				Bind();
				glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(Target, GL_TEXTURE_WRAP_R, GL_REPEAT);
				SetFilter(TextureFilter::Anisotropic16x);
			}
			~Texture()
			{
				glDeleteTextures(1, &mHandle);
			}
		public:
			void Load(ImageFormat internalFormat, ImageFormat inputFormat, ImageDataType inputType, void* data, int width)
			{
				glBindTexture(Target, mHandle);
				glTexImage1D(Target, 0, (int)inputFormat, width, 0, (int)internalFormat, (int)inputType, data);
				GL::glGenerateMipmap(Target);
			}
			void Load(ImageFormat internalFormat, ImageFormat inputFormat, ImageDataType inputType, void* data, int width, int height)
			{
				glBindTexture(Target, mHandle);
				glTexImage2D(Target, 0, (int)inputFormat, width, height, 0, (int)internalFormat, (int)inputType, data);
				GL::glGenerateMipmap(Target);
			}
			void ReadPixels(ImageFormat outputFormat, ImageDataType type, void* data)
			{
				Bind();
				glGetTexImage(Target, 0, (int)outputFormat, (int)type, data);
			}
			void ReadFromFrameBuffer(ImageFormat internalFormat, int width, int height)
			{
				Bind();
				glCopyTexImage2D(Target, 0, (int)internalFormat, 0, 0, width, height, 0);
				GL::glGenerateMipmap(Target);
			}
			void SetFilter(TextureFilter filter)
			{
				Bind();
				switch (filter)
				{
				case TextureFilter::Nearest:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					break;
				case TextureFilter::Linear:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					break;
				case TextureFilter::TriLinear:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					break;
				case TextureFilter::Anisotropic4x:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameterf(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
					break;
				case TextureFilter::Anisotropic8x:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameterf(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
					break;
				case TextureFilter::Anisotropic16x:
					glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameterf(Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
					break;
				}
			}
			void Bind()
			{
				glBindTexture(Target, mHandle);
			}
			static void UnBind()
			{
				glBindTexture(Target, 0);
			}
		};

		class Texture2D : public Texture<GL_TEXTURE_2D>
		{
		public:
			static Texture2D * Create(const char* fileName);
		};

		class Texture1D : public Texture<GL_TEXTURE_1D>
		{};

		class RenderBuffer : public GLObject
		{
		public:
			RenderBuffer()
			{
				GL::glGenRenderbuffers(1, &mHandle);
			}
			~RenderBuffer()
			{
				GL::glDeleteRenderbuffers(1, &mHandle);
			}
		public:
			void SetStorage(int w, int h, ImageFormat format, int multiSampleCount = 0)
			{
				GL::glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
				GL::glRenderbufferStorageMultisample(GL_RENDERBUFFER, multiSampleCount, (int)format, w, h);
				GL::glBindRenderbuffer(GL_RENDERBUFFER, 0);
			}
		};

		enum class FrameBufferTarget
		{
			Draw = GL_DRAW_FRAMEBUFFER,
			Read = GL_READ_FRAMEBUFFER
		};

		enum class FrameBufferAttachment
		{
			Color0 = GL_COLOR_ATTACHMENT0,
			Color1 = GL_COLOR_ATTACHMENT1,
			Color2 = GL_COLOR_ATTACHMENT2,
			Color3 = GL_COLOR_ATTACHMENT3,
			Color4 = GL_COLOR_ATTACHMENT4,
			Color5 = GL_COLOR_ATTACHMENT5,
			Color6 = GL_COLOR_ATTACHMENT6,
			Color7 = GL_COLOR_ATTACHMENT7,
			Color8 = GL_COLOR_ATTACHMENT8,
			Color9 = GL_COLOR_ATTACHMENT9,
			Color10 = GL_COLOR_ATTACHMENT10,
			Color11 = GL_COLOR_ATTACHMENT11,
			Color12 = GL_COLOR_ATTACHMENT12,
			Color13 = GL_COLOR_ATTACHMENT13,
			Color14 = GL_COLOR_ATTACHMENT14,
			Color15 = GL_COLOR_ATTACHMENT15,
			Depth = GL_DEPTH_ATTACHMENT,
			Stencil = GL_STENCIL_ATTACHMENT
		};

		class FrameBuffer : public GLObject
		{
		private:
			int mTarget;

		public:
			FrameBuffer()
			{
				mTarget = (int)FrameBufferTarget::Draw;
				GL::glGenFramebuffers(1, &mHandle);
			}
			virtual ~FrameBuffer()
			{
				GL::glDeleteFramebuffers(1, &mHandle);
			}
		public:
			void SetTarget(FrameBufferTarget target)
			{
				this->mTarget = (int)target;
			}
			void Attach(FrameBufferAttachment attachment, RenderBuffer* renderBuffer)
			{
				Bind();
				GL::glFramebufferRenderbuffer(mTarget, (int)attachment, GL_RENDERBUFFER, renderBuffer->GetHandle());
				UnBind();
			}
			void Attach(FrameBufferAttachment attachment, Texture2D* texture, int level = 0)
			{
				Bind();
				GL::glFramebufferTexture2D(mTarget, (int)attachment, GL_TEXTURE_2D, texture->GetHandle(), level);
				UnBind();
			}
			void Bind()
			{
				GL::glBindFramebuffer(mTarget, mHandle);
			}
			void UnBind()
			{
				GL::glBindFramebuffer(mTarget, 0);
			}
		};
		void InitializeOpenGLExtensions();
	}
}