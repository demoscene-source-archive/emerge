
#include <iostream>
// assimp include files. These three are usually needed.
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>

#include <cstdio>
#include <cassert>
#include <algorithm>
#include <memory>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <deque>

const aiScene* scene = NULL;

using namespace std;

//  0 - time

//  1 - position x value
//  2 - position x in tangent time
//  3 - position x in tangent value
//  2 - position x out tangent time
//  3 - position x out tangent value

//  1 - position y value
//  2 - position y in tangent time
//  3 - position y in tangent value
//  2 - position y out tangent time
//  3 - position y out tangent value

//  1 - position z value
//  2 - position z in tangent time
//  3 - position z in tangent value
//  2 - position z out tangent time
//  3 - position z out tangent value

//  1 - rotation x value
//  2 - rotation x in tangent time
//  3 - rotation x in tangent value
//  2 - rotation x out tangent time
//  3 - rotation x out tangent value

//  1 - rotation y value
//  2 - rotation y in tangent time
//  3 - rotation y in tangent value
//  2 - rotation y out tangent time
//  3 - rotation y out tangent value

//  1 - rotation z value
//  2 - rotation z in tangent time
//  3 - rotation z in tangent value
//  2 - rotation z out tangent time
//  3 - rotation z out tangent value

static ostream& operator<<(ostream& s, const aiVector3D& v)
{
   s << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
   return s;
}

void printNode(aiNode* n,const std::string& prefix="")
{
   cout << prefix << n->mName.C_Str() << endl;
   cout << prefix << "  mNumMeshes = " << n->mNumMeshes << endl;
   for(int i=0;i<n->mNumChildren;++i)
   {
      printNode(n->mChildren[i],prefix+"  ");
   }
}

unsigned long int countNodeMeshIndices(aiNode* n)
{
   unsigned long int c=n->mNumMeshes;
   for(int i=0;i<n->mNumChildren;++i)
   {
      c+=countNodeMeshIndices(n->mChildren[i]);
   }
   return c;
}

unsigned long int countNodeChildren(aiNode* n)
{
   unsigned long int c=n->mNumChildren;
   for(int i=0;i<n->mNumChildren;++i)
   {
      c+=countNodeChildren(n->mChildren[i]);
   }
   return c;
}

unsigned long int countNodes(aiNode* n)
{
   unsigned long int c=1;
   for(int i=0;i<n->mNumChildren;++i)
   {
      c+=countNodes(n->mChildren[i]);
   }
   return c;
}

void serializeNodes(aiNode* n,aiNode**& arr)
{
   *arr = n;
   ++arr;
   for(int i=0;i<n->mNumChildren;++i)
   {
      serializeNodes(n->mChildren[i],arr);
   }
}

struct V { float x, y, z; unsigned long int idx; float nx, ny, nz; bool operator==(const V& v) const { return x==v.x && y==v.y && z==v.z; } };
struct VComp { bool operator()(const V& a, const V& b) const { return a.x < b.x || (a.x == b.x && a.y < b.y) || (a.x == b.x && a.y == b.y && a.z < b.z); } };
//struct VComp { bool operator()(const V& a, const V& b) const { return a.idx < b.idx; } };

int main()
{
	scene = aiImportFile("part3_flyby_psy01.dae",0);

	if (!scene)
   {
      printf("ASSIMP import failure: '%s'\n",aiGetErrorString());
      return -1;
   }

   FILE* out=fopen("animations.cpp","w");
   if(!out)
      return -1;
   FILE* out2=fopen("animations.hpp","w");
   if(!out2)
   {
      fclose(out);
      return -1;
   }

   cout << "mNumCameras = " << scene->mNumCameras << endl;

   for(int i=0;i<scene->mNumCameras;++i)
   {
      cout << "Camera " << i << endl;
      cout << "  mName = " << scene->mCameras[i]->mName.C_Str() << endl;
      cout << "  mPosition = " << scene->mCameras[i]->mPosition << endl;
      cout << "  mUp = " << scene->mCameras[i]->mUp << endl;
      cout << "  mLookAt = " << scene->mCameras[i]->mLookAt << endl;
      cout << "  mHorizontalFOV = " << scene->mCameras[i]->mHorizontalFOV << endl;
      cout << "  mClipPlaneNear = " << scene->mCameras[i]->mClipPlaneNear << endl;
      cout << "  mClipPlaneFar = " << scene->mCameras[i]->mClipPlaneFar << endl;
      cout << "  mAspect = " << scene->mCameras[i]->mAspect << endl;
   }

   cout << "mNumAnimations = " << scene->mNumAnimations << endl;

   const unsigned long int nodecount=countNodes(scene->mRootNode);
   aiNode** node_ptrs=new aiNode*[nodecount];
   {
      aiNode** p=node_ptrs;
      serializeNodes(scene->mRootNode,p);
   }

   {
      FILE* markersfile=fopen("markers.csv","r");
      FILE* markersfileout=fopen("markers.hpp","w");
      fprintf(markersfileout,"struct Marker { const char* name; int frame; unsigned long int node_index; };\n");
      fprintf(markersfileout,"const Marker markers[]={\n");
      char line[1024];
      unsigned long int markercount=0;
      while(fgets(line,sizeof(line),markersfile))
      {
         char markername[256]="";
         char cameraname[256]="";
         int frame=0;
         sscanf(line,"%s %d %s",markername,&frame,cameraname);
         std::replace(cameraname,cameraname+strlen(cameraname),'.','_');
         unsigned long int node_index = 0;
         for (unsigned long int k=0;k<nodecount;++k)
         {
            if(node_ptrs[k]->mName == aiString(std::string(cameraname)))
            {
               node_index=k + 1;
            }
         }
         fprintf(markersfileout,"{ \"%s\", %d, %d }, // %s\n",markername,frame-1,node_index,cameraname);
         ++markercount;
      }
      fprintf(markersfileout,"};\n");
      fprintf(markersfileout,"static const unsigned long int marker_count = %d;\n",markercount);
      fclose(markersfileout);
      fclose(markersfile);
   }

   for(int i=0;i<scene->mNumAnimations;++i)
   {
      cout << "Animation " << i << endl;
      cout << "  mDuration = " << scene->mAnimations[i]->mDuration << endl;
      cout << "  mTicksPerSecond = " << scene->mAnimations[i]->mTicksPerSecond << endl;
      cout << "  mNumChannels = " << scene->mAnimations[i]->mNumChannels << endl;
      cout << "  mNumMeshChannels = " << scene->mAnimations[i]->mNumMeshChannels << endl;


      unsigned int totalkeys=0;
      for(int j=0;j<scene->mAnimations[i]->mNumChannels;++j)
         for(int kk=0;kk<7;++kk)
            totalkeys+=scene->mAnimations[i]->mChannels[j]->mNumKeys[kk];
      double* keydata = new double[totalkeys*6];
      unsigned long int keydata_offset=0;
      unsigned long int* keyoffsets = new unsigned long int[scene->mAnimations[i]->mNumChannels*7];
      unsigned long int* node_indices = new unsigned long int[scene->mAnimations[i]->mNumChannels];

      for(int j=0;j<scene->mAnimations[i]->mNumChannels;++j)
      {
         const aiNodeAnim* ch = scene->mAnimations[i]->mChannels[j];

         unsigned long int nodeindex=0;
         for (unsigned long int k=0;k<nodecount;++k)
         {
            if(node_ptrs[k]->mName == ch->mNodeName)
            {
               nodeindex=k;
            }
         }

         node_indices[j]=nodeindex;

         for(int kk=0;kk<7;++kk)
         {
            keyoffsets[j*7+kk]=keydata_offset;
            for(unsigned long int jj=0;jj<ch->mNumKeys[kk];++jj)
            {
               keydata[keydata_offset++] = ch->mKeys[kk][jj].mTime;
               keydata[keydata_offset++] = ch->mKeys[kk][jj].mValue;
               keydata[keydata_offset++] = ch->mInTangentKeys[kk][jj].mValueX;
               keydata[keydata_offset++] = ch->mInTangentKeys[kk][jj].mValueY;
               keydata[keydata_offset++] = ch->mOutTangentKeys[kk][jj].mValueX;
               keydata[keydata_offset++] = ch->mOutTangentKeys[kk][jj].mValueY;
            }
         }

         cout << "  Channel " << j << endl;
         cout << "    mNodeName = " << scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str() << endl;

      }

      fprintf(out2,"static const unsigned long int key_data_length=%d;\n",totalkeys*7);
      fprintf(out2,"static const unsigned long int channel_key_offsets_length=%d;\n",scene->mAnimations[i]->mNumChannels*7);
      fprintf(out2,"static const unsigned long int channel_nodes_length=%d;\n",scene->mAnimations[i]->mNumChannels);
      fprintf(out2,"extern const double key_data[key_data_length];\n");
      fprintf(out2,"extern const unsigned long int channel_key_offsets[channel_key_offsets_length + 1];\n");
      fprintf(out2,"extern const unsigned long int channel_nodes[channel_nodes_length];\n");

      fprintf(out,"#include \"animations.hpp\"\n");
      fprintf(out,"const double key_data[key_data_length]={\n");
      for(unsigned long int j=0;j<totalkeys;++j)
      {
         fprintf(out,"%g, %g, %g, %g, %g, %g, // %d\n",keydata[j*6+0],keydata[j*6+1],keydata[j*6+2],keydata[j*6+3],keydata[j*6+4],keydata[j*6+5],j * 6);
      }
      fprintf(out,"};\n");

      fprintf(out,"const unsigned long int channel_key_offsets[channel_key_offsets_length + 1]={\n");
      for(unsigned long int j=0;j<scene->mAnimations[i]->mNumChannels;++j)
      {
         fprintf(out,"%d, %d, %d, %d, %d, %d, %d, // %s\n",keyoffsets[j*7+0],keyoffsets[j*7+1],keyoffsets[j*7+2],keyoffsets[j*7+3],keyoffsets[j*7+4],keyoffsets[j*7+5],keyoffsets[j*7+6],scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str());
      }
      fprintf(out, "%d, ", totalkeys*7);
      fprintf(out,"};\n");

      fprintf(out,"const unsigned long int channel_nodes[channel_nodes_length]={\n");
      for(unsigned long int j=0;j<scene->mAnimations[i]->mNumChannels;++j)
      {
         fprintf(out,"%d, // %s\n",node_indices[j],scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str());
      }
      fprintf(out,"};\n");
   }

   cout << "mNumMeshes = " << scene->mNumMeshes << endl;
   unsigned long int totalverts=0,totalfaces=0,totalindices=0;
   for(int i=0;i<scene->mNumMeshes;++i)
   {
      aiMesh* m=scene->mMeshes[i];
      cout << "  mNumFaces = " << m->mNumFaces << endl;
      cout << "  mNumVertices = " << m->mNumVertices << endl;
      cout << "  mName = '" << m->mName.C_Str() << '\'' << endl;
      totalverts+=m->mNumVertices;
      totalfaces+=m->mNumFaces;
      for(int j=0;j<m->mNumFaces;++j)
         totalindices+=(m->mFaces[j].mNumIndices-2)*3;
   }
   cout << "totalverts = " << totalverts << endl;
   cout << "totalfaces = " << totalfaces << endl;
   cout << "totalindices = " << totalindices << endl;

   cout << "Nodes:" << endl;
   printNode(scene->mRootNode);


   V* vs=new V[totalverts];
   unsigned long int* is=new unsigned long int[totalindices];
   unsigned long int* isj=new unsigned long int[totalindices];
   unsigned long int* vs_to_is=new unsigned long int[totalverts];
   unsigned long int* bases=new unsigned long int[scene->mNumMeshes];
   memset(vs_to_is,0,totalverts*sizeof(vs_to_is[0]));
   memset(is,0,totalindices*sizeof(is[0]));
   memset(isj,0,totalindices*sizeof(isj[0]));
   unsigned long int idx=0,idx2=0;
   for(int i=0;i<scene->mNumMeshes;++i)
   {
      aiMesh* m=scene->mMeshes[i];
      const unsigned long int base_idx=idx;
      for(int j=0;j<m->mNumVertices;++j)
      {
         aiVector3D& vec=m->mVertices[j];
         vs[idx].x=vec.x;
         vs[idx].y=vec.y;
         vs[idx].z=vec.z;
         vs[idx].idx=idx;
         ++idx;
      }
      bases[i]=idx2;
      for(int j=0;j<m->mNumFaces;++j)
      {
         for(int k=0;k<m->mFaces[j].mNumIndices-2;++k)
         {
            const unsigned long int tri_indices[3] = { m->mFaces[j].mIndices[0], m->mFaces[j].mIndices[k+1], m->mFaces[j].mIndices[k+2] };
            for(int ti=0;ti<3;++ti)
            {
               is[idx2]=tri_indices[ti] + base_idx;
               if(is[idx2]>=totalverts)
                  exit(-23);
               // link this index into the list of indices that point at this vertex
               isj[idx2]=vs_to_is[is[idx2]];
               // update the mapping from vertex locations to index locations
               vs_to_is[is[idx2]]=idx2+1;
               ++idx2;
            }
         }
      }
   }

   std::sort(vs+0,vs+totalverts,VComp());

   unsigned long int* compact_list=new unsigned long int[totalverts];
   unsigned long int compact_num=0;
   for(unsigned long int i = 0; i < totalverts;)
   {
      const unsigned long int j=i;

      do
      {
         if(vs_to_is[vs[i].idx]==0)
            exit(-67);
         // go through the list of indices that point at this vertex's original location and
         // update them with the new post-merge location
         for(unsigned long int n=vs_to_is[vs[i].idx];n!=0;n=isj[n-1])
            is[n-1]=compact_num;
         ++i;

      } while(i < totalverts && vs[i-1]==vs[i]);

      compact_list[compact_num++]=j;
      //cout << (i-j) << " duplicates." << endl;
   }

   cout << "compact_num = " << compact_num << endl;

   unsigned long int num_meshes_adjusted = scene->mNumMeshes;
   map<unsigned long int,unsigned long int> mesh_map;
   map<unsigned long int,unsigned long int> mesh_map_inv;
   vector<unsigned long int> adjusted_mesh_bases;
   vector<unsigned long int> is2;

   {
      cout << "checking for duplicate meshes..." << endl;
      vector<pair<unsigned long int,unsigned long int> > meshes_to_remove;
      vector<bool> removed;
      map<unsigned long int,unsigned long int> mesh_map_inv_temp;
      removed.resize(scene->mNumMeshes);
      for(unsigned long int i=0;i<scene->mNumMeshes;++i)
      {
         const unsigned long int bi=bases[i];
         const unsigned long int si=((i==(scene->mNumMeshes-1)) ? totalindices : bases[i+1]) - bi;
         for(unsigned long int j=i+1;j<scene->mNumMeshes;++j)
         {
            const unsigned long int bj=bases[j];
            const unsigned long int sj=((j==(scene->mNumMeshes-1)) ? totalindices : bases[j+1]) - bj;
            if(sj != si || scene->mMeshes[i]->mMaterialIndex != scene->mMeshes[j]->mMaterialIndex)
               continue;
            unsigned long int k =0;
            for(;k<sj;++k)
            {
               if(is[bj+k]!=is[bi+k])
                  break;
            }
            if(k==sj)
            {
               meshes_to_remove.push_back(pair<unsigned long int,unsigned long int>(i, j));
               mesh_map_inv_temp[i]=j;
               removed[i] = true;
               break;
            }
         }
      }

      num_meshes_adjusted = 0;
      adjusted_mesh_bases.resize(scene->mNumMeshes-meshes_to_remove.size());

      for(unsigned long int i=0;i<scene->mNumMeshes;++i)
      {
         if(!removed[i])
         {
            mesh_map_inv[i] = num_meshes_adjusted;
            mesh_map[num_meshes_adjusted]=i;

            unsigned long int bi=bases[i];
            unsigned long int si=((i==(scene->mNumMeshes-1)) ? totalindices : bases[i+1]) - bi;

            adjusted_mesh_bases[num_meshes_adjusted] = is2.size();

            for(unsigned long int k=0;k<si;++k)
            {
               is2.push_back(is[bi+k]);
            }

            ++num_meshes_adjusted;
         }
      }

      for(unsigned long int i=0;i<scene->mNumMeshes;++i)
      {
         if(removed[i])
         {
            unsigned int j=i;
            while(removed[j])
               j=mesh_map_inv_temp[j];
            mesh_map_inv[i]=mesh_map_inv[j];
         }
      }

      cout << "removed " << meshes_to_remove.size() << endl;
   }

   {
      FILE* meshesout=fopen("meshes.cpp","w");
      FILE* meshesout2=fopen("meshes.hpp","w");

      fprintf(meshesout,"#include \"meshes.hpp\"\n");

      fprintf(meshesout2,"static const unsigned long int total_vertex_count = %d;\n",compact_num);

      V* vs2=new V[compact_num];
      for(unsigned long int j=0;j<compact_num;++j)
      {
        unsigned int i=compact_list[j];
        vs2[j] = vs[i];
        vs2[j].nx = 0;
        vs2[j].ny = 0;
        vs2[j].nz = 0;
      }
      for(unsigned long int i=0;i<is2.size();i+=3)
      {
         unsigned long int* tri = &is2[0] + i;
         double normal[3];
         normal[0]=(vs2[tri[1]].y-vs2[tri[0]].y) * (vs2[tri[2]].z-vs2[tri[0]].z) - (vs2[tri[1]].z-vs2[tri[0]].z) * (vs2[tri[2]].y-vs2[tri[0]].y);
         normal[1]=(vs2[tri[1]].z-vs2[tri[0]].z) * (vs2[tri[2]].x-vs2[tri[0]].x) - (vs2[tri[1]].x-vs2[tri[0]].x) * (vs2[tri[2]].z-vs2[tri[0]].z);
         normal[2]=(vs2[tri[1]].x-vs2[tri[0]].x) * (vs2[tri[2]].y-vs2[tri[0]].y) - (vs2[tri[1]].y-vs2[tri[0]].y) * (vs2[tri[2]].x-vs2[tri[0]].x);
        double l=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
        if(l==0)
            l=1;
        normal[0] /= l;
        normal[1] /= l;
        normal[2] /= l;
        for(unsigned long int j=0;j<3;++j)
        {
            vs2[tri[j]].nx+=normal[0];
            vs2[tri[j]].ny+=normal[1];
            vs2[tri[j]].nz+=normal[2];
        }
      }
      for(unsigned long int i=0;i<compact_num;++i)
      {
        double l=sqrt(vs2[i].nx*vs2[i].nx+vs2[i].ny*vs2[i].ny+vs2[i].nz*vs2[i].nz);
        if(l==0)
            l=1;
        vs2[i].nx /= l;
        vs2[i].ny /= l;
        vs2[i].nz /= l;
      }

      fprintf(meshesout2,"static const unsigned long int total_mesh_count = %d;\n",num_meshes_adjusted);

      {
         vector<float> local_box_min;
         vector<float> local_box_max;
         local_box_min.resize(num_meshes_adjusted * 3);
         local_box_max.resize(num_meshes_adjusted * 3);
         for(unsigned long int i=0;i<num_meshes_adjusted;++i)
         {
            unsigned long int b=adjusted_mesh_bases[i];
            unsigned long int s=((i==(num_meshes_adjusted-1)) ? is2.size() : adjusted_mesh_bases[i+1]) - b;

            local_box_min[i * 3 + 0] = +1e10f;
            local_box_min[i * 3 + 1] = +1e10f;
            local_box_min[i * 3 + 2] = +1e10f;

            local_box_max[i * 3 + 0] = -1e10f;
            local_box_max[i * 3 + 1] = -1e10f;
            local_box_max[i * 3 + 2] = -1e10f;

            for(unsigned long int j = 0; j < s; ++j)
            {
               local_box_min[i * 3 + 0] = min(local_box_min[i * 3 + 0], vs2[is2[b + j]].x);
               local_box_min[i * 3 + 1] = min(local_box_min[i * 3 + 1], vs2[is2[b + j]].y);
               local_box_min[i * 3 + 2] = min(local_box_min[i * 3 + 2], vs2[is2[b + j]].z);

               local_box_max[i * 3 + 0] = max(local_box_max[i * 3 + 0], vs2[is2[b + j]].x);
               local_box_max[i * 3 + 1] = max(local_box_max[i * 3 + 1], vs2[is2[b + j]].y);
               local_box_max[i * 3 + 2] = max(local_box_max[i * 3 + 2], vs2[is2[b + j]].z);
            }

            for(unsigned long int j = 0; j < 3; ++j)
            {
               local_box_max[i * 3 + j] += 1e-3f;
               local_box_min[i * 3 + j] -= 1e-3f;
            }
         }
         fprintf(meshesout2,"extern float bounding_boxes[total_mesh_count*6];\n");
         fprintf(meshesout,"float bounding_boxes[total_mesh_count*6]={\n");
         for(unsigned long int i=0;i<num_meshes_adjusted;++i)
         {
            const unsigned long int original_index = mesh_map[i];
            string names("");
            for(unsigned long int jj=0;jj<nodecount;++jj)
            {
               for(unsigned long int kk=0;kk<node_ptrs[jj]->mNumMeshes;++kk)
               {
                  if(node_ptrs[jj]->mMeshes[kk]==original_index)
                     names+=string("\"")+node_ptrs[jj]->mName.C_Str()+string("\" ");
               }
            }
            fprintf(meshesout,"%ff, %ff, %ff, %ff, %ff, %ff, // %s\n",local_box_min[i * 3 + 0],local_box_min[i * 3 + 1],local_box_min[i * 3 + 2],
                                                                      local_box_max[i * 3 + 0],local_box_max[i * 3 + 1],local_box_max[i * 3 + 2],names.c_str());
         }
         fprintf(meshesout,"};\n");

/*
         fprintf(meshesout2,"extern const char *const mesh_names[total_mesh_count];\n");
         fprintf(meshesout,"const char *const mesh_names[total_mesh_count]={\n");
         for(unsigned long int i=0;i<num_meshes_adjusted;++i)
         {
            fprintf(meshesout,"\"%s\", \n",scene->mMeshes[mesh_map[i]]->mName.C_Str());
         }
         fprintf(meshesout,"};\n");
*/
      }

      fprintf(meshesout2,"extern float vertices[total_vertex_count*6];\n");
      fprintf(meshesout,"float vertices[total_vertex_count*6]={\n");
      for(unsigned long int i=0;i<compact_num;++i)
      {
         fprintf(meshesout,"%ff, %ff, %ff, %ff, %ff, %ff,\n",vs2[i].x,vs2[i].y,vs2[i].z,vs2[i].nx,vs2[i].ny,vs2[i].nz);
      }
      fprintf(meshesout,"};\n");
      delete[] vs2;
      vs2=0;

      fprintf(meshesout2,"extern const unsigned long int mesh_index_bases[total_mesh_count + 1];\n");
      fprintf(meshesout,"const unsigned long int mesh_index_bases[total_mesh_count + 1]={\n");
      for(unsigned long int i=0;i<num_meshes_adjusted;++i)
      {
         fprintf(meshesout,"%d, ",adjusted_mesh_bases[i]);
      }
      fprintf(meshesout,"%d, ",is2.size());
      fprintf(meshesout,"};\n");

      fprintf(meshesout2,"extern const unsigned long int mesh_materials[total_mesh_count];\n");
      fprintf(meshesout,"const unsigned long int mesh_materials[total_mesh_count]={\n");
      for(unsigned long int i=0;i<num_meshes_adjusted;++i)
      {
         const unsigned long int original_index = mesh_map[i];
         string names("");
         for(unsigned long int jj=0;jj<nodecount;++jj)
         {
            for(unsigned long int kk=0;kk<node_ptrs[jj]->mNumMeshes;++kk)
            {
               if(node_ptrs[jj]->mMeshes[kk]==original_index)
                  names+=string("\"")+node_ptrs[jj]->mName.C_Str()+string("\" ");
            }
         }
         const unsigned long int matidx=scene->mMeshes[mesh_map[i]]->mMaterialIndex;
         fprintf(meshesout,"%d, // %s \n",(matidx==0xffff) ? 0 : (matidx + 1),names.c_str());
      }
      fprintf(meshesout,"};\n");

      fprintf(meshesout2,"static const unsigned long int total_index_count = %d;\n",is2.size());
      fprintf(meshesout2,"extern const unsigned short int indices[total_index_count];\n");
      fprintf(meshesout,"const unsigned short int indices[total_index_count]={\n");
      for(unsigned long int i=0;i<is2.size();++i)
      {
         if(is2[i]>=compact_num)
            exit(-88);
         fprintf(meshesout,"%d, ",is2[i]);
         if((i%3)==2)
            fprintf(meshesout,"\n");
      }
      fprintf(meshesout,"};\n");

/*
      fprintf(meshesout2,"extern const unsigned long int indices_adjacency[total_index_count * 2];\n");
      fprintf(meshesout,"const unsigned long int indices_adjacency[total_index_count * 2]={\n");
      {
          map<unsigned long int, unsigned long int> edges;
          unsigned long int* is_adj = new unsigned long int[is2.size() * 2];
          for(unsigned long int i=0;i<is2.size();i+=3)
          {
             unsigned long int* tri = &is2[0] + i;
             edges[tri[0] + tri[1] * compact_num] = tri[2];
             edges[tri[1] + tri[2] * compact_num] = tri[0];
             edges[tri[2] + tri[0] * compact_num] = tri[1];
          }
          for(unsigned long int i = 0; i < is2.size(); i+=3)
          {
             unsigned long int* tri = &is2[0] + i;
             unsigned long int* tri2 = is_adj + i * 2;

             tri2[0] = tri[0];
             tri2[1] = edges[tri[1] + tri[0] * compact_num];
             tri2[2] = tri[1];
             tri2[3] = edges[tri[2] + tri[1] * compact_num];
             tri2[4] = tri[2];
             tri2[5] = edges[tri[0] + tri[2] * compact_num];
          }
          for(unsigned long int i=0;i<is2.size() * 2;++i)
          {
             if(is_adj[i]>=compact_num)
                exit(-88);
             fprintf(meshesout,"%d, ",is_adj[i]);
             if((i%6)==5)
                fprintf(meshesout,"\n");
          }
          fprintf(meshesout,"};\n");
          delete[] is_adj;
      }
*/

/*
      fprintf(meshesout,"const char* mesh_names[total_mesh_count]={\n");
      for(unsigned long int i=0;i<scene->mNumMeshes;++i)
      {
         fprintf(meshesout,"\"%s\", \n",scene->mMeshes[i]->mName.C_Str());
      }
      fprintf(meshesout,"};\n");
*/
      fclose(meshesout);
      fclose(meshesout2);
   }
   {
      FILE* nodesout=fopen("nodes.cpp","w");
      FILE* nodesout2=fopen("nodes.hpp","w");

      fprintf(nodesout,"#include \"nodes.hpp\"\n");

      {
         float* node_transformations=new float[nodecount*16];

         unsigned long int node_child_index_count=countNodeChildren(scene->mRootNode);
         unsigned long int node_mesh_index_count=countNodeMeshIndices(scene->mRootNode);
         unsigned long int* node_child_offsets=new unsigned long int[nodecount];
         unsigned long int* node_mesh_offsets=new unsigned long int[nodecount];
         unsigned long int* node_child_indices=new unsigned long int[node_child_index_count];
         unsigned long int* node_mesh_indices=new unsigned long int[node_mesh_index_count];

         cout << "nodecount = " << nodecount << endl;
         cout << "node_child_index_count = " << node_child_index_count << endl;
         cout << "node_mesh_index_count = " << node_mesh_index_count << endl;


         unsigned long int ci=0;
         unsigned long int mi=0;
         for(unsigned long int i=0;i<nodecount;++i)
         {
            node_child_offsets[i]=ci;
            node_mesh_offsets[i]=mi;
            for(unsigned long int j=0;j<node_ptrs[i]->mNumChildren;++j)
            {
               for(unsigned long int k=0;k<nodecount;++k)
               {
                  if(node_ptrs[k]==node_ptrs[i]->mChildren[j])
                  {
                     node_child_indices[ci++]=k;
                     break;
                  }
               }
            }
            for(unsigned long int j=0;j<node_ptrs[i]->mNumMeshes;++j)
            {
               node_mesh_indices[mi++]=mesh_map_inv[node_ptrs[i]->mMeshes[j]];
            }
         }

         fprintf(nodesout2,"static const unsigned long int node_count = %d;\n",nodecount);
         fprintf(nodesout2,"static const unsigned long int node_child_index_count = %d;\n",node_child_index_count);
         fprintf(nodesout2,"static const unsigned long int node_mesh_index_count = %d;\n",node_mesh_index_count);

         // raytraced surfaces
         {
            typedef unsigned long int uint32;
            vector<pair<string,string> > raytraced_surface_names;
            raytraced_surface_names.push_back(pair<string,string>("water_surface1", "water_ground1"));
            raytraced_surface_names.push_back(pair<string,string>("water_surface2", "water_ground2"));
            raytraced_surface_names.push_back(pair<string,string>("water_surface3", "water_ground3"));
            fprintf(nodesout2,"static const unsigned long int raytraced_surface_node_count = %d;\n",raytraced_surface_names.size());
            fprintf(nodesout2,"extern const unsigned long int raytraced_surface_nodes[raytraced_surface_node_count * 2];\n");
            fprintf(nodesout,"const unsigned long int raytraced_surface_nodes[raytraced_surface_node_count * 2]={\n");
            for(;!raytraced_surface_names.empty();raytraced_surface_names.pop_back())
            {
               const pair<string,string>& ns = raytraced_surface_names.back();
               pair<uint32,uint32> is;
               for(uint32 i=0;i<nodecount;++i)
               {
                  if(node_ptrs[i]->mName == aiString(ns.first))
                     is.first = i;
                  if(node_ptrs[i]->mName == aiString(ns.second))
                     is.second = i;
               }
               fprintf(nodesout," %d, %d, // \"%s\", \"%s\"\n",is.first,is.second,ns.first.c_str(),ns.second.c_str());
            }
            fprintf(nodesout,"};\n");
         }

         // special nodes
         {
            for(unsigned long int i=0;i<nodecount;++i)
            {
               if(node_ptrs[i]->mName == aiString("revision_logo_001"))
                  fprintf(nodesout2,"static const unsigned long int revision_logo_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("blossom_trees"))
                  fprintf(nodesout2,"static const unsigned long int blossom_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("the_village"))
                  fprintf(nodesout2,"static const unsigned long int village_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("all_crystals"))
                  fprintf(nodesout2,"static const unsigned long int crystals_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("cave_001"))
                  fprintf(nodesout2,"static const unsigned long int cave_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("rocket_001"))
                  fprintf(nodesout2,"static const unsigned long int rocket_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("skyscrapers"))
                  fprintf(nodesout2,"static const unsigned long int skyscraper_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("windmill_vanes"))
                  fprintf(nodesout2,"static const unsigned long int windmill_vanes_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("Camera_003"))
                  fprintf(nodesout2,"static const unsigned long int camera_003_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("Camera_006"))
                  fprintf(nodesout2,"static const unsigned long int camera_006_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("Camera_007"))
                  fprintf(nodesout2,"static const unsigned long int camera_007_node_index = %d;\n",i);
               else if(node_ptrs[i]->mName == aiString("Camera_016"))
                  fprintf(nodesout2,"static const unsigned long int camera_016_node_index = %d;\n",i);
            }

         }

         // skyscrapers
         {
            vector<unsigned long int> skyscraper_indices;
            for(unsigned long int i=0;i<nodecount;++i)
            {
               string name(node_ptrs[i]->mName.C_Str());
               if(name.find("skyscraper")!=string::npos && name != "skyscrapers")
               {
                  skyscraper_indices.push_back(i);
               }
            }
            fprintf(nodesout2,"static const unsigned long int skyscraper_node_count = %d;\n",skyscraper_indices.size());
            fprintf(nodesout2,"extern const unsigned long int skyscraper_nodes[skyscraper_node_count];\n");
            fprintf(nodesout,"const unsigned long int skyscraper_nodes[skyscraper_node_count] = {\n");
            for(size_t i=0;i<skyscraper_indices.size();++i)
            {
               fprintf(nodesout,"%d, // %s\n",skyscraper_indices[i],node_ptrs[skyscraper_indices[i]]->mName.C_Str());
            }
            fprintf(nodesout,"};\n");
         }

         // chunked nodes
         {
            typedef unsigned long int uint32;
            vector<string> chunked_node_names;
            chunked_node_names.push_back("terrain_001");
            chunked_node_names.push_back("cover_001");
            chunked_node_names.push_back("Object004_001");
            chunked_node_names.push_back("water_ground1");
            chunked_node_names.push_back("water_ground2");
            chunked_node_names.push_back("water_ground3");
            chunked_node_names.push_back("water_surface1");
            chunked_node_names.push_back("water_surface2");
            chunked_node_names.push_back("water_surface3");
            chunked_node_names.push_back("cave-cover_001");
            chunked_node_names.push_back("cave_001");
            fprintf(nodesout2,"static const unsigned long int chunked_node_count = %d;\n",chunked_node_names.size());
            fprintf(nodesout2,"extern const unsigned long int chunked_nodes[chunked_node_count];\n");
            fprintf(nodesout,"const unsigned long int chunked_nodes[chunked_node_count]={\n");
            for(;!chunked_node_names.empty();chunked_node_names.pop_back())
            {
               const string& ns = chunked_node_names.back();
               uint32 j = 0;
               for(uint32 i=0;i<nodecount;++i)
               {
                  if(node_ptrs[i]->mName == aiString(ns))
                     j = i;
               }
               fprintf(nodesout," %d, // \"%s\"\n",j,ns.c_str());
            }
            fprintf(nodesout,"};\n");
         }

/*
         fprintf(nodesout2,"extern const char *const node_names[node_count];\n");
         fprintf(nodesout,"const char *const node_names[node_count]={\n");
         for(unsigned long int i=0;i<nodecount;++i)
         {
            fprintf(nodesout,"\"%s\",\n",node_ptrs[i]->mName.C_Str());
         }
         fprintf(nodesout,"};\n");
*/

         fprintf(nodesout2,"extern const unsigned long int node_child_indices[node_child_index_count];\n");
         fprintf(nodesout,"const unsigned long int node_child_indices[node_child_index_count]={\n");
         for(unsigned long int i=0;i<node_child_index_count;++i)
         {
            fprintf(nodesout,"%d, ",node_child_indices[i]);
         }
         fprintf(nodesout,"};\n");

         fprintf(nodesout2,"extern const unsigned long int node_mesh_indices[node_mesh_index_count];\n");
         fprintf(nodesout,"const unsigned long int node_mesh_indices[node_mesh_index_count]={\n");
         for(unsigned long int i=0;i<node_mesh_index_count;++i)
         {
            fprintf(nodesout,"%d, ",node_mesh_indices[i]);
         }
         fprintf(nodesout,"};\n");

         fprintf(nodesout2,"extern const unsigned long int node_child_offsets[node_count + 1];\n");
         fprintf(nodesout,"const unsigned long int node_child_offsets[node_count + 1]={\n");
         for(unsigned long int i=0;i<nodecount;++i)
         {
            fprintf(nodesout,"%d, ",node_child_offsets[i]);
         }
         fprintf(nodesout,"%d, ",node_child_index_count);
         fprintf(nodesout,"};\n");

         fprintf(nodesout2,"extern const unsigned long int node_mesh_offsets[node_count + 1];\n");
         fprintf(nodesout,"const unsigned long int node_mesh_offsets[node_count + 1]={\n");
         for(unsigned long int i=0;i<nodecount;++i)
         {
            fprintf(nodesout,"%d, ",node_mesh_offsets[i]);
         }
         fprintf(nodesout,"%d, ",node_mesh_index_count);
         fprintf(nodesout,"};\n");

         fprintf(nodesout2,"extern const float node_transformations[node_count*16];\n");
         fprintf(nodesout,"const float node_transformations[node_count*16]={\n");
         for(unsigned long int i=0;i<nodecount;++i)
         {
            const aiMatrix4x4& t=node_ptrs[i]->mTransformation;
            for(unsigned long int j=0;j<4;++j)
            {
               fprintf(nodesout,"%ff, %ff, %ff, %ff,",t[0][j+0],t[0][j+4],t[0][j+8],t[0][j+12]);
            }
            fprintf(nodesout,"\n");
         }
         fprintf(nodesout,"};\n");
      }

      fclose(nodesout);
      fclose(nodesout2);
   }

   {
      FILE* matout = fopen("materials.hpp","w");
      FILE* matout2 = fopen("materials.cpp","w");

      const unsigned long int num_materials = scene->mNumMaterials + 1;
      float* material_data = new float[num_materials * 4];
      cout << "scene->mNumMaterials = " << scene->mNumMaterials << endl;
      material_data[0] = 1;
      material_data[1] = 1;
      material_data[2] = 1;
      material_data[3] = 0;
      for(unsigned long int i = 0; i < scene->mNumMaterials; ++i)
      {
         aiMaterial* m = scene->mMaterials[i];
         material_data[(i + 1) * 4 + 0] = 1;
         material_data[(i + 1) * 4 + 1] = 1;
         material_data[(i + 1) * 4 + 2] = 1;
         material_data[(i + 1) * 4 + 3] = 0;
         //cout << "  mNumProperties = " << m->mNumProperties << endl;
         for(unsigned long int j = 0; j < m->mNumProperties; ++j)
         {
            aiMaterialProperty* p = m->mProperties[j];
            cout << "    mKey = " << p->mKey.C_Str() << endl;
            cout << "    mType = " << p->mType << endl;
            cout << "    mDataLength = " << p->mDataLength << endl;
            if(p->mKey == aiString("$clr.diffuse"))
            {
               if(p->mType != aiPTI_Float)
                  exit(-33);
               if(p->mDataLength != 16)
                  exit(-34);
               material_data[(i + 1) * 4 + 0] = ((float*)p->mData)[0];
               material_data[(i + 1) * 4 + 1] = ((float*)p->mData)[1];
               material_data[(i + 1) * 4 + 2] = ((float*)p->mData)[2];
            }
            else if(p->mKey == aiString("?mat.name"))
            {
               aiString str;
               aiGetMaterialString(m, "?mat.name", 0, 0, &str);
               cout << "NAME: " << str.C_Str() << endl;
               if(str==aiString("cave_inside_material0-material") || str==aiString("cave_inside_material1-material"))
                  material_data[(i + 1) * 4 + 3] = 1;
               else if(str==aiString("water_surface-material"))
               {
                  fprintf(matout,"static const unsigned long int water_surface_material = %d;\n",i+1);
                  material_data[(i + 1) * 4 + 3] = 2;
               }
               else if(str==aiString("mound-material"))
               {
                  fprintf(matout,"static const unsigned long int mound_material = %d;\n",i+1);
                  material_data[(i + 1) * 4 + 3] = 2;
               }
               else if(str==aiString("_3___Default-material") ||
                       str==aiString("_2___Default-material") ||
                       str==aiString("_7___Default-material") ||
                       str==aiString("wire_088144225-material") ||
                       str==aiString("_8___Default-material") ||
                       str==aiString("wire_086086086-material"))
               {
                  material_data[(i + 1) * 4 + 3] = 3;
               }
            }

         }

      }
      fprintf(matout2,"#include \"materials.hpp\"\n");
      fprintf(matout,"static const unsigned long int num_materials = %d;\n",num_materials);
      fprintf(matout,"extern const float material_data[num_materials * 4];\n");
      fprintf(matout2,"const float material_data[num_materials * 4]={\n");
      for(unsigned long int i = 0; i < num_materials; ++i)
      {
         fprintf(matout2,"%f, %f, %f, %f,\n",material_data[i*4+0],material_data[i*4+1],material_data[i*4+2],material_data[i*4+3]);
      }
      fprintf(matout2,"};\n");
      fclose(matout);
      fclose(matout2);
      delete[] material_data;
   }


   {
      FILE* meshtestobj=fopen("test.obj","w");
      for(unsigned long int i=0;i<compact_num;++i)
      {
         fprintf(meshtestobj,"v %f %f %f\n",vs[compact_list[i]].x,vs[compact_list[i]].y,vs[compact_list[i]].z);
      }
      for(unsigned long int i=0;i<num_meshes_adjusted;++i)
      {
         const unsigned long int b=adjusted_mesh_bases[i];
         const unsigned long int s=((i<num_meshes_adjusted-1) ? adjusted_mesh_bases[i+1] : is2.size()) - b;
         for(unsigned long int j=0;j<s;j+=3)
            fprintf(meshtestobj,"f %d %d %d\n",is2[b+j]+1,is2[b+j+1]+1,is2[b+j+2]+1);
      }
      fclose(meshtestobj);
   }



   fclose(out);
   fclose(out2);




   {
      FILE* voxelsout = fopen("voxels.hpp", "w");
      FILE* voxelsout2 = fopen("voxels.cpp", "w");
      fprintf(voxelsout2, "#include \"voxels.hpp\"\n");
      typedef struct { long col; unsigned short z; char vis, dir; } kv6voxtype;


      using namespace std;
         long xsiz, ysiz, zsiz;
         float xpiv, ypiv, zpiv;
         static unsigned long xlen[4096];
         static unsigned short ylen[4096][4096];
         long numvoxs;

         FILE *fil = fopen("revision2015e_collision.kv6","rb");

         char fileid[5];
         fread(fileid,4,1,fil); //'Kvxl' (= 0x6c78764b in Little Endian)
          fileid[4]='\0';
         printf("fileid=%s\n",fileid);

             //Voxel grid dimensions
         fread(&xsiz,4,1,fil); fread(&ysiz,4,1,fil); fread(&zsiz,4,1,fil);

         printf("xsiz=%d\n",xsiz);
         printf("ysiz=%d\n",ysiz);
         printf("zsiz=%d\n",zsiz);

         fprintf(voxelsout, "static const unsigned int voxels_xsiz = %d, voxels_ysiz = %d, voxels_zsiz = %d;\n", xsiz, ysiz,zsiz);

         //Pivot point. Floating point format. Voxel units.
         fread(&xpiv,4,1,fil); fread(&ypiv,4,1,fil); fread(&zpiv,4,1,fil);

         printf("xpiv=%f\n",xpiv);
         printf("ypiv=%f\n",ypiv);
         printf("zpiv=%f\n",zpiv);

         fread(&numvoxs,4,1,fil); //Total number of surface voxels
         printf("numvoxs=%d\n",numvoxs);
         vector<int> red(numvoxs),green(numvoxs),blue(numvoxs),height(numvoxs);

         fprintf(voxelsout, "static const unsigned int voxels_count = %d;\n", numvoxs);
         fprintf(voxelsout, "extern const unsigned short int voxel_heights[voxels_count];\n");
         fprintf(voxelsout2, "const unsigned short int voxel_heights[voxels_count] = {\n");

         for(int i=0;i<numvoxs;i++) //8 bytes per surface voxel, Z's must be sorted
         {
            red  [i]    = fgetc(fil); //Range: 0..255
            green[i]    = fgetc(fil); //"
            blue [i]    = fgetc(fil); //"
            int dummy       = fgetc(fil); //Always 128. Ignore.
            int height_low  = fgetc(fil); //Z coordinate of this surface voxel
            int height_high = fgetc(fil); //"
            int visibility  = fgetc(fil); //Low 6 bits say if neighbor is solid or air
            int normalindex = fgetc(fil); //Uses 256-entry lookup table

            height[i] = height_low | (height_high << 8);
            fprintf(voxelsout2, "%d, ", height[i]);
            if((i & 63)==63)
               fprintf(voxelsout2,"\n");
         }

         fprintf(voxelsout2, "};\n");

            //Number of surface voxels present in plane x (extra information)
         for(int x=0;x<xsiz;x++) fread(&xlen[x],4,1,fil);

         fprintf(voxelsout, "static const unsigned int voxel_column_length_count = %d;\n",xsiz*ysiz);
         fprintf(voxelsout, "extern const unsigned char voxel_column_lengths[voxel_column_length_count];\n");
         fprintf(voxelsout2, "const unsigned char voxel_column_lengths[voxel_column_length_count] = {\n");

         long int idx=0;
            //Number of surface voxels present in column (x,y)
         for(int x=0;x<xsiz;x++)
         {
            for(int y=0;y<ysiz;y++)
            {
               fread(&ylen[x][y],2,1,fil);
               fprintf(voxelsout2, "%d, ", ylen[x][y]);
            }
            fprintf(voxelsout2, "\n");
         }

         fprintf(voxelsout2, "};\n");

         fclose(fil);
         fil=NULL;


      fclose(voxelsout);
      fclose(voxelsout2);
   }











   return 0;
}
