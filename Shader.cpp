
#include "Shader.hpp"

#include <vector>
#include <algorithm>
#include <string>

#define COMPRESS_SHADERS 0

struct ShaderSourceCode
{
   static const int max_lines = 1024;
   static const int max_line_length = 1024;
   unsigned char str_array[max_lines*max_line_length];
   int len_array[max_lines];
   std::string name;

   ShaderSourceCode()
   {
      memset(len_array,0,sizeof(len_array));
   }
};
#if COMPRESS_SHADERS
static ShaderSourceCode source_codes[1024];
static uint32 num_source_codes=0;
struct Symbol
{
   bool terminal;
   bool pl;
   unsigned char c;
   uint32 left;
   uint32 right;
   uint32 parent;
   uint32 f;
   uint32 uid;
   bool operator<(const Symbol& s) const { return f > s.f; }
   Symbol(){ c='\0'; f=0; left=0; right=0; parent=0; terminal=true;; pl=false; uid=0; }
};
std::vector<Symbol> dict;
Symbol* findSymbol(uint32 uid)
{
   size_t sz=dict.size();
   for(size_t i=0;i<sz;++i)
   {
      if(dict[i].uid==uid)
         return &dict[i];
   }
   return NULL;
}
int findSymbolIndex(uint32 uid)
{
   size_t sz=dict.size();
   for(size_t i=0;i<sz;++i)
   {
      if(dict[i].uid==uid)
         return i;
   }
   return -1;
}
void encodeShaderSources()
{
   uint32 uid=0;

   FILE* outh=fopen("shader_data.hpp","w");
   FILE* outc=fopen("shader_data.cpp","w");
   fprintf(outc,"#include \"shader_data.hpp\"\n");

   fprintf(outh,"struct ShaderSymbol\n"
"{\n"
"    char character;\n"
"    unsigned short int frequency;\n"
"};\n");

   Symbol symbols[256];
   std::vector<Symbol> heap;
   for(int i=0;i<256;++i)
   {
      symbols[i].c=i;
      symbols[i].uid=++uid;
   }
   for(uint32 k=0;k<num_source_codes;++k)
   {
      const ShaderSourceCode* it=source_codes+k;
      for(int i=0;i<ShaderSourceCode::max_lines;++i)
      {
         int l=it->len_array[i];
         for(int j=0;j<l;++j)
         {
            symbols[it->str_array[i*ShaderSourceCode::max_line_length+j]].f++;
         }
         symbols[0].f++;
      }
   }

   for(int i=0;i<256;++i)
      if(symbols[i].f>0)
         heap.push_back(symbols[i]);

   std::vector<Symbol> heap2=heap;
   const int sz=heap2.size();
   fprintf(outh,"static const int shader_symbol_count=%d;\n",sz);
   fprintf(outh,"extern const ShaderSymbol shader_symbols[shader_symbol_count];\n");
   fprintf(outc,"const ShaderSymbol shader_symbols[shader_symbol_count]={\n");
   uint32 psf=0;
   for(int i=0;i<sz;++i)
   {
      Symbol s=heap2[i];
      fprintf(outc,"{ %d, %d },\n",(int)s.c,s.f);
      log("heap %02x: %d\n",(int)s.c,s.f);
      psf=s.f;
   }
   fprintf(outc,"};\n");

   std::make_heap(heap.begin(),heap.end());

   Symbol root;
   while(!heap.empty())
   {
      Symbol s0=heap.front();
      dict.push_back(s0);
      std::pop_heap(heap.begin(),heap.end());
      heap.pop_back();
      if(heap.empty())
      {
         // this is the root
         root = s0;
         break;
      }
      Symbol s1=heap.front();
      dict.push_back(s1);
      std::pop_heap(heap.begin(),heap.end());
      heap.pop_back();

      Symbol ns;
      ns.uid=++uid;
      ns.terminal=false;
      ns.left=s0.uid;
      ns.right=s1.uid;
      ns.f=s0.f+s1.f;
      findSymbol(ns.left)->parent=ns.uid;
      findSymbol(ns.left)->pl=true;
      findSymbol(ns.right)->parent=ns.uid;
      findSymbol(ns.right)->pl=false;

      heap.push_back(ns);
      std::push_heap(heap.begin(),heap.end());
   }

   assert(!root.terminal);
   dict.push_back(root);

//   fprintf(outc,"const ShaderDictEntry shader_dictionary[%d]={\n",dict.size());
   for(size_t i=0;i<dict.size();++i)
   {
      log("%02x uid:%d f:%d l:%d r:%d\n",dict[i].c,dict[i].uid,dict[i].f,dict[i].left,dict[i].right);
  //    fprintf(outc,"{ %d, %d, %d },\n",dict[i].c,findSymbolIndex(dict[i].left),findSymbolIndex(dict[i].right));
   }
   //fprintf(outc,"};\n");

   uint32 total_uncompressed=0,total_compressed=0;
   for(uint32 k=0;k<num_source_codes;++k)
   {
      std::vector<unsigned char> encoded(1024*1024);
      uint32 encoded_bit_count=0,unencoded_bytes=0;

      const ShaderSourceCode* it=source_codes+k;

      for(int n=0;n<ShaderSourceCode::max_lines;++n)
      {
         int l=it->len_array[n];
         for(int j=0;j<l;++j)
         {
            unsigned char c=it->str_array[n*ShaderSourceCode::max_line_length+j];
            ++unencoded_bytes;
            uint32 code=0,codelen=0;
            const int sz=dict.size();
            for(int i=0;i<sz;++i)
            {
               if(dict[i].c==c && dict[i].terminal)
               {
                  Symbol* s=&dict[i];
                  assert(s->parent);
                  while(s->parent)
                  {
                     if(s->pl)
                        code=(code<<1)|0;
                     else
                        code=(code<<1)|1;

                     ++codelen;
                     s=findSymbol(s->parent);
                     assert(s);
                     assert(!s->terminal);
                  }
                  break;
               }
            }
            assert(codelen>0);
//log("codelen=%d\n",codelen);
            while(codelen-->0)
            {
               int b=code&1;
               code>>=1;
               encoded[encoded_bit_count/8]|=b<<(encoded_bit_count&7);
               encoded_bit_count++;
            }

         }
      }

      total_uncompressed+=unencoded_bytes;

      const uint32 num_encoded_bytes=(encoded_bit_count+7)/8;

      total_compressed+=num_encoded_bytes;

      fprintf(outh,"extern const unsigned char %s[%d];\n",it->name.c_str(),num_encoded_bytes);
      fprintf(outh,"static const unsigned long int %s_bitcount = %d;\n",it->name.c_str(),encoded_bit_count);
      fprintf(outc,"const unsigned char %s[%d]={",it->name.c_str(),num_encoded_bytes);
      for(uint32 i=0;i<num_encoded_bytes;++i)
         fprintf(outc,"%d, ",encoded[i]);
      fprintf(outc,"};\n");

      log("%d bytes in un-encoded shaders\n",unencoded_bytes);
      log("%d bytes in encoded shaders\n",num_encoded_bytes);
      uint32 unencoded_bit_count=0;
      while(unencoded_bit_count<encoded_bit_count)
      {
         Symbol* s=&root;

         while(!s->terminal)
         {
            int b=encoded[unencoded_bit_count/8]&(1<<(unencoded_bit_count&7));
            if(b)
               s=findSymbol(s->right);
            else
               s=findSymbol(s->left);
            assert(s);
            ++unencoded_bit_count;
         }

         log("%c",s->c);
      }
   }

   log("total_compressed = %d\n",total_compressed);
   log("total_uncompressed = %d\n",total_uncompressed);
}
#endif

static void shaderSourceFromFile(GLuint shader, const char* filename)
{
   FILE* in = fopen(filename, "r");

   if(!in)
   {
      log("Could not open shader file '%s'.\n", filename);
      return;
   }

   ShaderSourceCode source_code;

   int num_lines = 0;

   while(fgets((char*)(source_code.str_array+num_lines*ShaderSourceCode::max_line_length), ShaderSourceCode::max_line_length, in))
   {
      assert(num_lines < ShaderSourceCode::max_lines);
      source_code.len_array[num_lines] = strlen((char*)(source_code.str_array+num_lines*ShaderSourceCode::max_line_length));
      assert(source_code.len_array[num_lines] < ShaderSourceCode::max_line_length);
      ++num_lines;
   }

   source_code.name = std::string(filename);
   std::replace(source_code.name.begin(),source_code.name.end(),'.','_');

#if COMPRESS_SHADERS
   source_codes[num_source_codes++]=source_code;
#endif

   const GLchar *strp_array[num_lines];

   for(int i = 0; i < num_lines; ++i)
      strp_array[i] = (char*)(source_code.str_array+i*ShaderSourceCode::max_line_length);

   glShaderSource(shader, num_lines, strp_array, source_code.len_array);

   fclose(in);

   CHECK_FOR_GL_ERRORS("shaderSourceFromFile");
}

static void shaderSourceFromString(GLuint shader, const std::string& source_string)
{
   const char* strs[1] = { source_string.c_str() };
   glShaderSource(shader, 1, strs, NULL);
   CHECK_FOR_GL_ERRORS("shaderSourceFromString");
}

GLuint createProgramFromSourceStrings(const std::string& vs,const std::string& gs,const std::string& fs)
{
   static char buf[4096];

   GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER),
          geometry_shader = glCreateShader(GL_GEOMETRY_SHADER),
          fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

   CHECK_FOR_GL_ERRORS("createProgram");

   GLuint prog = glCreateProgram();

   if(!vs.empty())
   {
      shaderSourceFromString(vertex_shader, vs);

      glCompileShader(vertex_shader);

      CHECK_FOR_GL_ERRORS("createProgram: vertex shader compile");

      GLint p = 0;
      glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nVertex ShaderInfoLog: (%s)\n\n", vs.c_str());
         glGetShaderInfoLog(vertex_shader, sizeof(buf), NULL, buf);
         log(buf);
         exit(-2);
      }

      glAttachShader(prog, vertex_shader);

      CHECK_FOR_GL_ERRORS("createProgram: vertex shader attach");
   }

   if(!gs.empty())
   {
      shaderSourceFromString(geometry_shader, gs);

      glCompileShader(geometry_shader);

      CHECK_FOR_GL_ERRORS("createProgram: geometry shader compile");

      GLint p = 0;
      glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nGeometry ShaderInfoLog: (%s)\n\n", gs.c_str());
         glGetShaderInfoLog(geometry_shader, sizeof(buf), NULL, buf);
         log(buf);
         exit(-2);
      }

      glAttachShader(prog, geometry_shader);

      CHECK_FOR_GL_ERRORS("createProgram: geometry shader attach");
   }

   if(!fs.empty())
   {
      shaderSourceFromString(fragment_shader, fs);

      glCompileShader(fragment_shader);

      CHECK_FOR_GL_ERRORS("createProgram: fragment shader compile");

      GLint p = 0;
      glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nFragment ShaderInfoLog: (%s)\n\n", fs.c_str());
         glGetShaderInfoLog(fragment_shader, sizeof(buf), NULL, buf);
         log(buf);
         exit(-2);
      }

      glAttachShader(prog, fragment_shader);

      CHECK_FOR_GL_ERRORS("createProgram: fragment shader attach");
   }

   glLinkProgram(prog);

   CHECK_FOR_GL_ERRORS("createProgram: link program");

   {
      GLint p = 0;
      glGetProgramiv(prog, GL_LINK_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nProgramInfoLog: \n\n");
         glGetProgramInfoLog(prog, sizeof(buf), NULL, buf);
         log(buf);
         exit(-2);
      }
   }

   if(!vs.empty())
      glDetachShader(prog, vertex_shader);

   if(!gs.empty())
      glDetachShader(prog, geometry_shader);

   if(!fs.empty())
      glDetachShader(prog, fragment_shader);

   if(vertex_shader)
      glDeleteShader(vertex_shader);

   if(geometry_shader)
      glDeleteShader(geometry_shader);

   if(fragment_shader)
      glDeleteShader(fragment_shader);

   glUseProgram(prog);

   CHECK_FOR_GL_ERRORS("createProgram: use program");

   return prog;
}

static std::string addDotIfNeeded(const std::string& s)
{
   size_t d=s.find_last_of('.');
   if(d!=std::string::npos)
      return s;
   size_t u=s.find_last_of('_');
   std::string s2(s);
   s2[u]='.';
   return s2;
}

GLuint createProgram(const char* vsh_filename, const char* gsh_filename, const char* fsh_filename)
{
   static char buf[4096];

   if(vsh_filename && strcmp(vsh_filename,"NULL")==0) vsh_filename=NULL;
   if(gsh_filename && strcmp(gsh_filename,"NULL")==0) gsh_filename=NULL;
   if(fsh_filename && strcmp(fsh_filename,"NULL")==0) fsh_filename=NULL;

   std::string vsh_filename2=vsh_filename?addDotIfNeeded(vsh_filename):"";
   std::string gsh_filename2=gsh_filename?addDotIfNeeded(gsh_filename):"";
   std::string fsh_filename2=fsh_filename?addDotIfNeeded(fsh_filename):"";

   GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER),
          geometry_shader = glCreateShader(GL_GEOMETRY_SHADER),
          fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

   CHECK_FOR_GL_ERRORS("createProgram");

   GLuint prog = glCreateProgram();

   if(vsh_filename)
   {
      shaderSourceFromFile(vertex_shader, vsh_filename2.c_str());

      glCompileShader(vertex_shader);

      CHECK_FOR_GL_ERRORS("createProgram: vertex shader compile");

      GLint p = 0;
      glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nVertex ShaderInfoLog: (%s)\n\n", vsh_filename2.c_str());
         glGetShaderInfoLog(vertex_shader, sizeof(buf), NULL, buf);
         log(buf);
      }

      glAttachShader(prog, vertex_shader);

      CHECK_FOR_GL_ERRORS("createProgram: vertex shader attach");
   }

   if(gsh_filename)
   {
      shaderSourceFromFile(geometry_shader, gsh_filename2.c_str());

      glCompileShader(geometry_shader);

      CHECK_FOR_GL_ERRORS("createProgram: geometry shader compile");

      GLint p = 0;
      glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nGeometry ShaderInfoLog: (%s)\n\n", gsh_filename2.c_str());
         glGetShaderInfoLog(geometry_shader, sizeof(buf), NULL, buf);
         log(buf);
      }

      glAttachShader(prog, geometry_shader);

      CHECK_FOR_GL_ERRORS("createProgram: geometry shader attach");
   }

   if(fsh_filename)
   {
      shaderSourceFromFile(fragment_shader, fsh_filename2.c_str());

      glCompileShader(fragment_shader);

      CHECK_FOR_GL_ERRORS("createProgram: fragment shader compile");

      GLint p = 0;
      glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nFragment ShaderInfoLog: (%s)\n\n", fsh_filename2.c_str());
         glGetShaderInfoLog(fragment_shader, sizeof(buf), NULL, buf);
         log(buf);
      }

      glAttachShader(prog, fragment_shader);

      CHECK_FOR_GL_ERRORS("createProgram: fragment shader attach");
   }

   glLinkProgram(prog);

   CHECK_FOR_GL_ERRORS("createProgram: link program");

   {
      GLint p = 0;
      glGetProgramiv(prog, GL_LINK_STATUS, &p);
      if(p == GL_FALSE)
      {
         log("\nProgramInfoLog: \n\n");
         glGetProgramInfoLog(prog, sizeof(buf), NULL, buf);
         log(buf);
      }
   }

   if(vsh_filename)
      glDetachShader(prog, vertex_shader);

   if(gsh_filename)
      glDetachShader(prog, geometry_shader);

   if(fsh_filename)
      glDetachShader(prog, fragment_shader);

   if(vsh_filename)
      glDeleteShader(vertex_shader);

   if(gsh_filename)
      glDeleteShader(geometry_shader);

   if(fsh_filename)
      glDeleteShader(fragment_shader);

   glUseProgram(prog);

   CHECK_FOR_GL_ERRORS("createProgram: use program");

   return prog;
}
