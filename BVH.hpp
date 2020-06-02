#pragma once

#include "gl3w/include/GL3/gl3w.h"
#include <GL/glext.h>

struct BVHData
{
   GLuint node0_tex;
   GLuint node1_tex;
   GLuint triangle_tex;
   GLuint edge0_tex;
   GLuint edge1_tex;
};

extern BVHData createBVH(const GLfloat* vertices, int vertex_stride, const GLushort* indices, unsigned long int triangle_count, const int group_size, const int levels);
