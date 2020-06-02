#pragma once

#include "Common.hpp"
#include <string>

extern GLuint createProgram(const char* vsh_filename, const char* gsh_filename, const char* fsh_filename);
extern GLuint createProgramFromSourceStrings(const std::string& vs,const std::string& gs,const std::string& fs);
