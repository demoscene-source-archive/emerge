#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace std;
int main(int argc,char** argv)
{
    if(argc<2)
    {
        printf("obj2c <obj file>\n");
        return 0;
    }
    FILE* in=fopen(argv[1],"r");
    if(!in)
        return -1;
        
    FILE* outc=fopen("mesh.cpp","w");
    FILE* outh=fopen("mesh.hpp","w");
        
    char str[1024];
    vector<float> vertices;
    vector<long unsigned int> indices;
    int linenum=0;
    while(fgets(str,sizeof(str),in))
    {
        if(str[0]=='v')
        {
            float x=0,y=0,z=0;
            if(sscanf(str,"v %f %f %f",&x,&y,&z)!=3)
            {
                printf("error on line %d.\n",linenum);
                break;
            }
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
        else if(str[0]=='f')
        {
            int x=0,y=0,z=0;
            if(sscanf(str,"f %d %d %d",&x,&y,&z)!=3)
            {
                printf("error on line %d.\n",linenum);
                break;
            }
            indices.push_back(x);
            indices.push_back(y);
            indices.push_back(z);
        }
        ++linenum;
    }
    
    fprintf(outh,"static const unsigned long int num_vertices = %d;\n",vertices.size()/3);
    fprintf(outh,"static const unsigned long int num_indices = %d;\n",indices.size());
    fprintf(outh,"extern const float vertices[num_vertices];\n");
    fprintf(outh,"extern const unsigned long int indices[num_indices];\n");

    fprintf(outc,"const float vertices[num_vertices * 3] = {\n");
    for(size_t i=0;i<vertices.size();++i)
    {
        fprintf(outc,"%f, ",vertices[i]);
        if((i%3)==2)
            fprintf(outc,"\n");
    }
    fprintf(outc,"};\n");
    
    fprintf(outc,"const unsigned long int indices[num_indices] = {\n");
    for(size_t i=0;i<indices.size();++i)
    {
        fprintf(outc,"%d, ",indices[i] - 1);
        if((i%3)==2)
            fprintf(outc,"\n");
    }
    fprintf(outc,"};\n");
    
    fclose(outc);
    fclose(outh);
    fclose(in);
    return 0;
}