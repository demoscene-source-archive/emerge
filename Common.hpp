#pragma once

#include "gl3w/include/GL3/gl3w.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "Mat.hpp"
#include "Ran.hpp"

#ifdef NDEBUG
#define CHECK_FOR_GL_ERRORS(tag)
#define CHECK_FRAMEBUFFER(b, tag)
#else
#define CHECK_FOR_GL_ERRORS(tag) checkForErrors(__FILE__, __LINE__, tag);
#define CHECK_FRAMEBUFFER(b, tag) checkFramebufferForErrors(b, __FILE__, __LINE__, tag);
#endif

typedef unsigned int uint;
typedef long unsigned int uint32;
typedef TMat4<float> Mat4;
typedef TVec3<float> Vec3;
typedef TVec2<float> Vec2;

extern FILE* g_logfile;

#define STRING_FOR_GL_ERROR_CASE(e) case GL_##e: return #e;
static const char* stringForGLError(GLenum err)
{
	switch(err)
	{
		STRING_FOR_GL_ERROR_CASE(NO_ERROR)
		STRING_FOR_GL_ERROR_CASE(INVALID_ENUM)
		STRING_FOR_GL_ERROR_CASE(INVALID_VALUE)
		STRING_FOR_GL_ERROR_CASE(INVALID_OPERATION)
		//STRING_FOR_GL_ERROR_CASE(STACK_OVERFLOW)
		//STRING_FOR_GL_ERROR_CASE(STACK_UNDERFLOW)
		STRING_FOR_GL_ERROR_CASE(OUT_OF_MEMORY)
		//STRING_FOR_GL_ERROR_CASE(TABLE_TOO_LARGE)
		STRING_FOR_GL_ERROR_CASE(INVALID_FRAMEBUFFER_OPERATION)
	}
	return "<unknown>";
}
static const char* stringForGLFramebufferStatus(GLenum status)
{
	switch(status)
	{
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_COMPLETE)
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
		STRING_FOR_GL_ERROR_CASE(FRAMEBUFFER_UNSUPPORTED)
	}
	return "<unknown>";
}
#undef STRING_FOR_GL_ERROR_CASE


static void checkFramebufferForErrors(GLenum binding_point, const char* file_name, int line_num, const char* tag)
{
   const GLenum status = glCheckFramebufferStatus(binding_point);
   if(status != GL_FRAMEBUFFER_COMPLETE)
   {
      if(g_logfile)
      {
         fprintf(g_logfile, "OpenGL framebuffer error detected on line %d of %s: %s (0x%x) [%s]\r\n", line_num, file_name, stringForGLFramebufferStatus(status), status, tag);
         fclose(g_logfile);
      }
      exit(0);
   }
}

static void checkForErrors(const char* file_name, int line_num, const char* tag)
{
   const GLenum error = glGetError();
   if(error != GL_NO_ERROR)
   {
      if(g_logfile)
      {
         fprintf(g_logfile, "OpenGL error detected on line %d of %s: %s (0x%x) [%s]\r\n", line_num, file_name, stringForGLError(error), error, tag);
         fclose(g_logfile);
      }
      exit(0);
   }
}

static void log(const char* str, ...)
{
   if(!g_logfile)
      return;

   va_list l;
   va_start(l, str);
   vfprintf(g_logfile, str, l);
   va_end(l);

   fflush(g_logfile);
}

static uint32 getBytesForInternalFormat(GLenum internalformat)
{
   switch(internalformat)
   {
      case GL_R8:
         return 1;
      case GL_R8_SNORM:
         return 1;
      case GL_R16:
         return 2;
      case GL_R16_SNORM:
         return 2;
      case GL_RG8:
         return 2;
      case GL_RG8_SNORM:
         return 2;
      case GL_RG16:
         return 2;
      case GL_RG16_SNORM:
         return 2;
      case GL_R3_G3_B2:
         return 1;
      case GL_RGB4:
         return 2;
      case GL_RGB5:
         return 2;
      case GL_RGB8:
         return 3;
      case GL_RGB8_SNORM:
         return 3;
      case GL_RGB10:
         return 4;
      case GL_RGB12:
         return 5;
      case GL_RGB16_SNORM:
         return 6;
      case GL_RGBA2:
         return 1;
      case GL_RGBA4:
         return 2;
      case GL_RGB5_A1:
         return 2;
      case GL_RGBA8:
         return 4;
      case GL_RGBA8_SNORM:
         return 4;
      case GL_RGB10_A2:
         return 4;
      case GL_RGB10_A2UI:
         return 4;
      case GL_RGBA12:
         return 6;
      case GL_RGBA16:
         return 8;
      case GL_SRGB8:
         return 3;
      case GL_SRGB8_ALPHA8:
         return 4;
      case GL_R16F:
         return 2;
      case GL_RG16F:
         return 4;
      case GL_RGB16F:
         return 6;
      case GL_RGBA16F:
         return 8;
      case GL_R32F:
         return 4;
      case GL_RG32F:
         return 8;
      case GL_RGB32F:
         return 12;
      case GL_RGBA32F:
         return 16;
      case GL_R11F_G11F_B10F:
         return 4;
      case GL_RGB9_E5:
         return 4;
      case GL_R8I:
         return 1;
      case GL_R8UI:
         return 1;
      case GL_R16I:
         return 2;
      case GL_R16UI:
         return 2;
      case GL_R32I:
         return 4;
      case GL_R32UI:
         return 4;
      case GL_RG8I:
         return 2;
      case GL_RG8UI:
         return 2;
      case GL_RG16I:
         return 4;
      case GL_RG16UI:
         return 4;
      case GL_RG32I:
         return 8;
      case GL_RG32UI:
         return 8;
      case GL_RGB8I:
         return 3;
      case GL_RGB8UI:
         return 3;
      case GL_RGB16I:
         return 6;
      case GL_RGB16UI:
         return 6;
      case GL_RGB32I:
         return 12;
      case GL_RGB32UI:
         return 12;
      case GL_RGBA8I:
         return 4;
      case GL_RGBA8UI:
         return 4;
      case GL_RGBA16I:
         return 8;
      case GL_RGBA16UI:
         return 8;
      case GL_RGBA32I:
         return 16;
      case GL_RGBA32UI:
         return 16;
      case GL_DEPTH_COMPONENT16:
         return 2;
      case GL_DEPTH_COMPONENT24:
         return 3;
      case GL_DEPTH_COMPONENT32:
         return 4;
      case GL_RGB16:
         return 6;
   }
   log("Unknown internalformat %04x\n",internalformat);
   assert(false);
   return 0;
}


