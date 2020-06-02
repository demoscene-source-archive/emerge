
#include "gl3w/include/GL3/gl3w.h"
#include <GL/glext.h>
#include "Common.hpp"
#include "BVH.hpp"
#include <vector>
#include <map>

namespace
{
   const int g_node_tex_w = 128, g_node_tex_h = 128;
   const int g_triangle_tex_w = 128, g_triangle_tex_h = 128;
   const int g_edge_tex_w = 128, g_edge_tex_h = 128;
   const int g_max_nodes = 20 * 1024;
 }

struct Node
{
   bool leaf;
   unsigned long int triangle;
   GLint box_min[3], box_max[3];

   static const int max_children = 128;

   Node* children[max_children];
   int num_children;

   Node()
   {
      reset();
   }

   void reset()
   {
      leaf = true;
      triangle = 0;
      for(int i = 0; i < 3; ++i)
      {
         box_min[i] = 1e9;
         box_max[i] = -1e9;
      }
      num_children = 0;
   }

   unsigned long int morton() const
   {
      unsigned long int box_center[3] = { (box_min[0] + box_max[0]) / 2,
                                          (box_min[1] + box_max[1]) / 2,
                                          (box_min[2] + box_max[2]) / 2 };

      unsigned long int bits = 0;

      // Interleave the bits of each component of the center point.
      for(int i = 0; i < 32; ++i)
         bits |= (box_center[i % 3] & (1 << (i / 3))) << (i - i / 3);

      return bits;
   }

   static bool comparator(const Node* n0, const Node* n1)
   {
      return n0->morton() < n1->morton();
   }

   void shuffleChildrenRecursively()
   {
      std::random_shuffle(children, children + num_children);
      for(int i = 0; i < num_children; ++i)
         children[i]->shuffleChildrenRecursively();
   }

   unsigned long int write(GLint* node0_data, GLint* node1_data, unsigned long int index) const;

};


unsigned long int Node::write(GLint* node0_data, GLint* node1_data, unsigned long int index) const
{
   GLint* n0 = node0_data + index * 4;
   GLint* n1 = node1_data + index * 4;

   GLint box_size[3];

   // Get the size of the bounding box and write it.
   for(int k = 0; k < 3; ++k)
   {
      n0[k] = box_min[k];
      n1[k] = box_max[k];
      box_size[k] = n1[k] - n0[k];
   }

   // Nodes with degenerate boxes are skipped.
   if(box_size[0] * box_size[1] * box_size[2] < 1)
      return 0;

   n0[3] = 0;

   if(leaf)
   {
      // Leaves only need to store a triangle index.
      n1[3] = (((triangle % g_triangle_tex_w) | ((triangle / g_triangle_tex_w) << 16)) << 1) | 1;
      return 1;
   }

   // Since there is no triangle index for a branch, the unused space is used for the traversal pointer.
   // This pointer is followed if the node intersects the ray.
   n1[3] = (((index + 1) % g_node_tex_w) | (((index + 1) / g_node_tex_w) << 16)) << 1;

   // The node's children are written immediately after it.
   int c = 1, prev_child_index = -1;
   for(int i = 0; i < num_children; ++i)
   {
      int child_index = index + c;
      int n = children[i]->write(node0_data, node1_data, child_index);

      if(n > 0)
      {
         if(prev_child_index != -1)
         {
            // Write the skip pointer for the previous child.
            GLint* pn0 = node0_data + prev_child_index * 4;
            pn0[3] = (child_index % g_node_tex_w) | ((child_index / g_node_tex_w) << 16);
         }

         prev_child_index = child_index;
         c += n;
      }
   }

   // The skip pointer for the last child always points to the very next node in the node array.
   {
      GLint* pn0 = node0_data + prev_child_index * 4;
      pn0[3] = ((index + c) % g_node_tex_w) | (((index + c) / g_node_tex_w) << 16);
   }

   return c;
}


static int writeNodes(GLint *const node0_data, GLint *const node1_data, const std::vector<Node*>& nodes)
{
   int node_count = 0, prev_node_count = -1;

   // Write the top-level nodes into the node data arrays which will be uploaded to the GPU.
   for(std::vector<Node*>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
   {
      int n = (*i)->write(node0_data, node1_data, node_count);
      if(n > 0)
      {
         if(node_count > 0)
         {
            // Write the skip pointer for the previous top-level node.
            GLint* pn0 = node0_data + prev_node_count * 4;
            pn0[3] = (node_count % g_node_tex_w) | ((node_count / g_node_tex_w) << 16);
         }
         prev_node_count = node_count;
         node_count += n;
      }
   }

   return node_count;
}

BVHData createBVH(const GLfloat* vertices, int vertex_stride, const GLushort* indices, unsigned long int triangle_count, const int group_size, const int levels)
{
   int num_nodes = 0;
   unsigned long int node_leaf_count = 0;
   unsigned long int total_node_count = 0;
   unsigned long int edge_count = 0;

   std::vector<Node> stored_nodes(g_max_nodes);

   for(int i=0;i<g_max_nodes;++i)
      stored_nodes[i].reset();


   std::vector<unsigned long int> edges;
   std::map<unsigned long int, bool> edges_got;
   std::map<unsigned long int, unsigned long int> edges_map;

   // Get the unique edges
   for(int i = 0; i < triangle_count; ++i)
   {
      const GLushort* t = indices + i * 3;
      GLushort e0, e1;
      unsigned long int eh;

      for(int j = 0; j < 3; ++j)
      {
         e0 = t[j];
         e1 = t[(j + 1) % 3];

         if(e0 > e1)
            std::swap(e0, e1);

         eh = e0 | (e1 << 16);

         if(!edges_got[eh])
         {
            edges_got[eh] = true;
            edges_map[eh] = edges.size();
            edges.push_back(eh);
         }
      }
   }

   //log("%d edges\n", edges.size());

   edge_count = edges.size();


   GLushort* triangle_data = new GLushort[g_triangle_tex_w * g_triangle_tex_h * 4];

   GLfloat* edge0_data = new GLfloat[g_edge_tex_w * g_edge_tex_h * 3];
   GLfloat* edge1_data = new GLfloat[g_edge_tex_w * g_edge_tex_h * 3];

   memset(edge0_data, 0, g_edge_tex_w * g_edge_tex_h * 3 * sizeof(edge0_data[0]));
   memset(edge1_data, 0, g_edge_tex_w * g_edge_tex_h * 3 * sizeof(edge1_data[1]));

   memset(triangle_data, 0, g_triangle_tex_w * g_triangle_tex_h * 4 * sizeof(GLushort));

   //int node_count = 0;

   //unsigned long int st = glutGet(GLUT_ELAPSED_TIME);

   std::vector<Node*> nodes;

   for(int i = 0; i < triangle_count; ++i)
   {
      const GLushort* t = indices + i * 3;

      GLfloat box_min[3] = { +1e9f, +1e9f, +1e9f };
      GLfloat box_max[3] = { -1e9f, -1e9f, -1e9f };

      for(int j = 0; j < 3; ++j)
      {
         const GLfloat* v = vertices + t[j] * vertex_stride;
         for(int k = 0; k < 3; ++k)
         {
            if(box_min[k] > v[k])
               box_min[k] = v[k];

            if(box_max[k] < v[k])
               box_max[k] = v[k];
         }
      }

      // Expand the bounding box slightly
      for(int k = 0; k < 3; ++k)
      {
         box_min[k] -= 1.0f;
         box_max[k] += 1.0f;
      }


      //log("box_min = { %f %f %f }\n", box_min[0], box_min[1], box_min[2]);
      //log("box_max = { %f %f %f }\n", box_max[0], box_max[1], box_max[2]);

      // Create the node data

      {
         Node* node = &stored_nodes[num_nodes++];

         node->leaf = true;
         node->triangle = i;

         for(int k = 0; k < 3; ++k)
         {
            node->box_min[k] = box_min[k];
            node->box_max[k] = box_max[k];
         }

         nodes.push_back(node);
      }



      // Create the triangle data
      {
         const GLushort* t = indices + i * 3;
         GLushort e0, e1;
         unsigned long int eh;
         GLushort* out = triangle_data + i * 4;

         out[3] = 0;

         for(int j = 0; j < 3; ++j)
         {
            e0 = t[j];
            e1 = t[(j + 1) % 3];

            if(e0 > e1)
            {
               std::swap(e0, e1);
               out[3] |= 1 << j;
            }

            eh = e0 | (e1 << 16);

            GLuint k = edges_map[eh];

            out[j] = (k % g_edge_tex_w) | ((k / g_edge_tex_w) << 8);

            //log("triangle: k = %08x, out[%d] = %08x\n", k, j, out[j]);
         }
      }

   }

   // Create the edge data
   for(int i = 0; i < edges.size(); ++i)
   {
      unsigned long int v0 = edges[i] & 0xffff;
      unsigned long int v1 = edges[i] >> 16;

      GLfloat delta[3];

      for(int j = 0; j < 3; ++j)
      {
         edge0_data[i * 3 + j] = vertices[v0 * vertex_stride + j];
         edge1_data[i * 3 + j] = vertices[v1 * vertex_stride + j];

         delta[j] = edge1_data[i * 3 + j] - edge0_data[i * 3 + j];
      }

      //log("edge delta = { %f %f %f }\n", delta[0], delta[1], delta[2]);
   }

   //log("%d leaf nodes\n", nodes.size());

   node_leaf_count = nodes.size();


   // Sort the nodes so that nodes which are spatially close to each other will be
   // ordinally close to each other in the node array.
   std::sort(nodes.begin(), nodes.end(), Node::comparator);

   // Group nodes together into a bounding-volume hierarchy.

   std::vector<Node*> nodes2, *arr0 = &nodes, *arr1 = &nodes2;

   for(int l = 0; l < levels; ++l)
   {
      for(int i = 0; i < arr0->size(); i += group_size)
      {
         Node* node = &stored_nodes[num_nodes++];
         node->leaf = false;
         for(int j = 0; j < group_size && ((i + j) < arr0->size()); ++j)
         {
            node->children[j] = (*arr0)[i + j];

            for(int k = 0; k < 3; ++k)
            {
               if(node->box_min[k] > (*arr0)[i + j]->box_min[k])
                  node->box_min[k] = (*arr0)[i + j]->box_min[k];

               if(node->box_max[k] < (*arr0)[i + j]->box_max[k])
                  node->box_max[k] = (*arr0)[i + j]->box_max[k];
            }
            ++node->num_children;
         }

         arr1->push_back(node);
      }

      arr0 = arr1;
      arr1 = new std::vector<Node*>;
   }

#if 1
   // Shuffle the top-level nodes to lessen the chance of a ray traversing every single node.
   // This cannot be done during tree-building as a node's location in the array is related to its physical
   // proximity to nearby nodes in that array.
   std::random_shuffle(arr0->begin(), arr0->end());

   for(std::vector<Node*>::iterator i = arr0->begin(); i != arr0->end(); ++i)
      (*i)->shuffleChildrenRecursively();
#endif


   GLint* node0_data = new GLint[g_node_tex_w * g_node_tex_h * 4];
   GLint* node1_data = new GLint[g_node_tex_w * g_node_tex_h * 4];

   memset(node0_data, 0, g_node_tex_w * g_node_tex_h * 4 * sizeof(GLint));
   memset(node1_data, 0, g_node_tex_w * g_node_tex_h * 4 * sizeof(GLint));


   const int node_count = writeNodes(node0_data, node1_data, *arr0);

   //log("%d total nodes\n", node_count);

   total_node_count = node_count;

   //log("tree building took %d ms\n", glutGet(GLUT_ELAPSED_TIME) - st);

   // Upload the data
   BVHData data;

   glGenTextures(1, &data.node0_tex);
   glGenTextures(1, &data.node1_tex);
   glGenTextures(1, &data.triangle_tex);
   glGenTextures(1, &data.edge0_tex);
   glGenTextures(1, &data.edge1_tex);

   glBindTexture(GL_TEXTURE_2D, data.node0_tex);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32I, g_node_tex_w, g_node_tex_h);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_node_tex_w, g_node_tex_h, GL_RGBA_INTEGER, GL_INT, node0_data);
   CHECK_FOR_GL_ERRORS("creating a BVH: node0_tex");

   glBindTexture(GL_TEXTURE_2D, data.node1_tex);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32I, g_node_tex_w, g_node_tex_h);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_node_tex_w, g_node_tex_h, GL_RGBA_INTEGER, GL_INT, node1_data);
   CHECK_FOR_GL_ERRORS("creating a BVH: node1_tex");

   glBindTexture(GL_TEXTURE_2D, data.triangle_tex);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16I, g_triangle_tex_w, g_triangle_tex_h);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_triangle_tex_w, g_triangle_tex_h, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, triangle_data);
   CHECK_FOR_GL_ERRORS("creating a BVH: triangle_tex");

   glBindTexture(GL_TEXTURE_2D, data.edge0_tex);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, g_edge_tex_w, g_edge_tex_h);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_edge_tex_w, g_edge_tex_h, GL_RGB, GL_FLOAT, edge0_data);
   CHECK_FOR_GL_ERRORS("creating a BVH: edge0_tex");

   glBindTexture(GL_TEXTURE_2D, data.edge1_tex);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, g_edge_tex_w, g_edge_tex_w);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_edge_tex_w, g_edge_tex_h, GL_RGB, GL_FLOAT, edge1_data);
   glBindTexture(GL_TEXTURE_2D, 0);
   CHECK_FOR_GL_ERRORS("creating a BVH: edge1_tex");


   delete[] triangle_data;

   delete[] edge0_data;
   delete[] edge1_data;

   delete[] node0_data;
   delete[] node1_data;

   return data;
}
