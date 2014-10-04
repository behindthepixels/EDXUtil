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
		// OpenGL Extension Loader
		extern PFNGLCREATESHADERPROC							glCreateShader;
		extern PFNGLCREATEPROGRAMPROC							glCreateProgram;
		extern PFNGLDELETESHADERPROC							glDeleteShader;
		extern PFNGLDELETEPROGRAMPROC							glDeleteProgram;
		extern PFNGLATTACHSHADERPROC							glAttachShader;
		extern PFNGLDETACHSHADERPROC							glDetachShader;
		extern PFNGLSHADERSOURCEPROC							glShaderSource;
		extern PFNGLCOMPILESHADERPROC							glCompileShader;
		extern PFNGLLINKPROGRAMPROC								glLinkProgram;
		extern PFNGLVALIDATEPROGRAMPROC							glValidateProgram;
		extern PFNGLUSEPROGRAMPROC								glUseProgram;
		extern PFNGLGETSHADERINFOLOGPROC						glGetShaderInfoLog;
		extern PFNGLGETPROGRAMINFOLOGPROC						glGetProgramInfoLog;
		extern PFNGLGETSHADERIVPROC								glGetShaderiv;
		extern PFNGLGETPROGRAMIVPROC							glGetProgramiv;
		extern PFNGLUNIFORM1FPROC								glUniform1f;
		extern PFNGLUNIFORM1FVPROC								glUniform1fv;
		extern PFNGLUNIFORM2FPROC								glUniform2f;
		extern PFNGLUNIFORM2FVPROC								glUniform2fv;
		extern PFNGLUNIFORM3FPROC								glUniform3f;
		extern PFNGLUNIFORM4FPROC								glUniform4f;
		extern PFNGLUNIFORM1IPROC								glUniform1i;
		extern PFNGLUNIFORM1IVPROC								glUniform1iv;
		extern PFNGLUNIFORM2IPROC								glUniform2i;
		extern PFNGLUNIFORM3IPROC								glUniform3i;
		extern PFNGLUNIFORM4IPROC								glUniform4i;
		extern PFNGLUNIFORMMATRIX3FVPROC						glUniformMatrix3fv;
		extern PFNGLUNIFORMMATRIX4FVPROC						glUniformMatrix4fv;
		extern PFNGLGETUNIFORMLOCATIONPROC						glGetUniformLocation;
		extern PFNGLBINDFRAGDATALOCATIONEXTPROC					glBindFragDataLocation;
		extern PFNGLISRENDERBUFFEREXTPROC						glIsRenderbuffer;
		extern PFNGLBINDRENDERBUFFEREXTPROC						glBindRenderbuffer;
		extern PFNGLDELETERENDERBUFFERSEXTPROC					glDeleteRenderbuffers;
		extern PFNGLGENRENDERBUFFERSEXTPROC						glGenRenderbuffers;
		extern PFNGLRENDERBUFFERSTORAGEEXTPROC					glRenderbufferStorage;
		extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		glRenderbufferStorageMultisample;
		extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC			glGetRenderbufferParameteriv;
		extern PFNGLISFRAMEBUFFEREXTPROC						glIsFramebuffer;
		extern PFNGLBINDFRAMEBUFFEREXTPROC						glBindFramebuffer;
		extern PFNGLDELETEFRAMEBUFFERSEXTPROC					glDeleteFramebuffers;
		extern PFNGLGENFRAMEBUFFERSEXTPROC						glGenFramebuffers;
		extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC				glCheckFramebufferStatus;
		extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC					glFramebufferTexture1D;
		extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC					glFramebufferTexture2D;
		extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC					glFramebufferTexture3D;
		extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC				glFramebufferRenderbuffer;
		extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC	glGetFramebufferAttachmentParameteriv;
		extern PFNGLBLITFRAMEBUFFEREXTPROC						glBlitFramebuffer;
		extern PFNGLGENERATEMIPMAPEXTPROC						glGenerateMipmap;
		extern PFNGLDRAWBUFFERSPROC								glDrawBuffers;
		extern PFNGLDRAWRANGEELEMENTSPROC						glDrawRangeElements;
		extern PFNGLACTIVETEXTUREPROC							glActiveTexture;
		extern PFNGLCLIENTACTIVETEXTUREPROC						glClientActiveTexture;
		extern PFNGLMULTITEXCOORD1FPROC							glMultiTexcoord1f;
		extern PFNGLMULTITEXCOORD2FPROC							glMultiTexcoord2f;
		extern PFNGLMULTITEXCOORD3FPROC							glMultiTexcoord3f;
		extern PFNGLMULTITEXCOORD4FPROC							glMultiTexcoord4f;
		extern PFNGLLOADTRANSPOSEMATRIXFPROC					glLoadTransposeMatrixf;
		extern PFNGLBINDBUFFERPROC								glBindBuffer;
		extern PFNGLBUFFERDATAPROC								glBufferData;
		extern PFNGLBUFFERSUBDATAPROC							glBufferSubData;
		extern PFNGLDELETEBUFFERSPROC							glDeleteBuffers;
		extern PFNGLGENBUFFERSPROC								glGenBuffers;
		extern PFNGLMAPBUFFERPROC								glMapBuffer;
		extern PFNGLUNMAPBUFFERPROC								glUnmapBuffer;
		extern PFNGLBLENDEQUATIONPROC							glBlendEquation;
		extern PFNGLBLENDCOLORPROC								glBlendColor;
		extern PFNGLBLENDFUNCSEPARATEPROC						glBlendFuncSeperate;
		extern PFNGLBLENDEQUATIONSEPARATEPROC					glBlendEquationSeperate;

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
				glGenBuffers(1, &mHandle);
			}
			virtual ~Buffer()
			{
				glDeleteBuffers(1, &mHandle);
			}
			void SetData(int sizeInBytes, void * data)
			{
				glBindBuffer(Target, mHandle);
				glBufferData(Target, sizeInBytes, data, GL_STATIC_DRAW);
			}
			void Bind()
			{
				glBindBuffer(Target, mHandle);
			}
			static void UnBind()
			{
				glBindBuffer(Target, 0);
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
				glGenerateMipmap(Target);
			}
			void Load(ImageFormat internalFormat, ImageFormat inputFormat, ImageDataType inputType, void* data, int width, int height)
			{
				glBindTexture(Target, mHandle);
				glTexImage2D(Target, 0, (int)inputFormat, width, height, 0, (int)internalFormat, (int)inputType, data);
				glGenerateMipmap(Target);
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
				glGenerateMipmap(Target);
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
				glGenRenderbuffers(1, &mHandle);
			}
			~RenderBuffer()
			{
				glDeleteRenderbuffers(1, &mHandle);
			}
		public:
			void SetStorage(int w, int h, ImageFormat format, int multiSampleCount = 0)
			{
				glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multiSampleCount, (int)format, w, h);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
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
				glGenFramebuffers(1, &mHandle);
			}
			virtual ~FrameBuffer()
			{
				glDeleteFramebuffers(1, &mHandle);
			}
		public:
			void SetTarget(FrameBufferTarget target)
			{
				this->mTarget = (int)target;
			}
			void Attach(FrameBufferAttachment attachment, RenderBuffer* renderBuffer)
			{
				Bind();
				glFramebufferRenderbuffer(mTarget, (int)attachment, GL_RENDERBUFFER, renderBuffer->GetHandle());
				UnBind();
			}
			void Attach(FrameBufferAttachment attachment, Texture2D* texture, int level = 0)
			{
				Bind();
				glFramebufferTexture2D(mTarget, (int)attachment, GL_TEXTURE_2D, texture->GetHandle(), level);
				UnBind();
			}
			void Bind()
			{
				glBindFramebuffer(mTarget, mHandle);
			}
			void UnBind()
			{
				glBindFramebuffer(mTarget, 0);
			}
		};
		void InitializeOpenGLExtensions();
	}
}