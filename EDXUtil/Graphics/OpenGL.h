#pragma once

#include "../EDXPrerequisites.h"

#include "../Windows/Base.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glext.h>

namespace EDX
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
		static PFNGLUNIFORM2FPROC								glUniform2f;
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
			glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
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
}