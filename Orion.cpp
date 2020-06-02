#include "Orion.hpp"
#include "animations.hpp"
#include "markers.hpp"
#include "meshes.hpp"
#include "nodes.hpp"
#include "materials.hpp"
#include <algorithm>
#include <stack>
#include <cmath>
#include <utility>

using namespace std;

Orion::~Orion()
{
    for(; !mesh_cache.empty(); mesh_cache.pop_back())
        delete mesh_cache.back();

    for(; !entities.empty(); entities.pop_back())
        delete entities.back();
}

void Orion::uploadMeshes()
{
    for(; !mesh_cache.empty(); mesh_cache.pop_back())
        delete mesh_cache.back();

    GLuint array_buffer = 0, element_array_buffer = 0, element_array_adj_buffer = 0;

    assert(total_vertex_count < 65536);

    glGenBuffers(1, &array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
    glBufferData(GL_ARRAY_BUFFER, total_vertex_count * 6 * sizeof(float), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &element_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_index_count * sizeof(indices[0]), indices, GL_STATIC_DRAW);

    //glGenBuffers(1, &element_array_adj_buffer);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_adj_buffer);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_index_count * 2 * sizeof(indices_adjacency[0]), indices_adjacency, GL_STATIC_DRAW);

    for(uint32 i = 0; i < total_mesh_count; ++i)
    {
        MeshCacheItem* m = new MeshCacheItem;
        mesh_cache.push_back(m);

        m->array_buffer = array_buffer;
        m->element_array_buffer = element_array_buffer;
        //m->element_array_adj_buffer = element_array_adj_buffer;
        m->count = mesh_index_bases[i + 1] - mesh_index_bases[i];
        m->type = GL_UNSIGNED_SHORT;
        m->indices = (const GLvoid*)(mesh_index_bases[i] * sizeof(indices[0]));
        m->indices_adj = (const GLvoid*)(mesh_index_bases[i] * 2 * sizeof(indices[0]));
    }

    CHECK_FOR_GL_ERRORS("end of mesh upload");
}

void Orion::renderSingleEntity(const CameraConfig& cc, uint32 i, const ProgramTransformProperties& transform_properties, bool adjacency) const
{
   assert(adjacency == false);

    GLuint array_buffer = 0, element_array_buffer = 0;

    if(transform_properties.world_to_view_matrix_location >= 0)
      glUniformMatrix4fv(transform_properties.world_to_view_matrix_location, 1, GL_FALSE, cc.world_to_view.e);

    if(transform_properties.view_to_clip_matrix_location >= 0)
      glUniformMatrix4fv(transform_properties.view_to_clip_matrix_location, 1, GL_FALSE, cc.view_to_clip.e);

    glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

   assert(i < entities.size());

     const Entity* ent = entities[i];
     assert(ent);

     const uint32 mesh_count = node_mesh_offsets[i + 1] - node_mesh_offsets[i];

     if(transform_properties.object_to_world_matrix_location >= 0)
         glUniformMatrix4fv(transform_properties.object_to_world_matrix_location, 1, GL_FALSE, ent->global_xfrm.e);

     if(transform_properties.object_to_view_matrix_location >= 0)
         glUniformMatrix4fv(transform_properties.object_to_view_matrix_location, 1, GL_FALSE, (cc.world_to_view * ent->global_xfrm).e);

     if(transform_properties.object_to_clip_matrix_location >= 0)
         glUniformMatrix4fv(transform_properties.object_to_clip_matrix_location, 1, GL_FALSE, (cc.world_to_clip * ent->global_xfrm).e);

     if(transform_properties.previous_object_to_clip_matrix_location >= 0)
         glUniformMatrix4fv(transform_properties.previous_object_to_clip_matrix_location, 1, GL_FALSE, (cc.previous_world_to_clip * ent->previous_global_xfrm).e);

     for(uint32 j = 0; j < mesh_count; ++j)
     {
         const uint32 mesh_index = node_mesh_indices[node_mesh_offsets[i] + j];
         const MeshCacheItem* m = mesh_cache[mesh_index];

         assert(mesh_index < total_mesh_count);

        if(transform_properties.material_index_location >= 0)
            glUniform1f(transform_properties.material_index_location, (float(mesh_materials[mesh_index] + 1) + 0.5f) / (float)((num_materials + 1) * 2));

         if(m->array_buffer != array_buffer)
         {
             array_buffer = m->array_buffer;
             glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
             glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const GLvoid*)(sizeof(float) * 3));
         }

         if(adjacency)
         {
             if(m->element_array_adj_buffer != element_array_buffer)
             {
                 element_array_buffer = m->element_array_adj_buffer;
                 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
             }

             glDrawElements(GL_TRIANGLES_ADJACENCY, m->count * 2, m->type, m->indices_adj);
         }
         else
         {
             if(m->element_array_buffer != element_array_buffer)
             {
                 element_array_buffer = m->element_array_buffer;
                 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
             }

             glDrawElements(GL_TRIANGLES, m->count, m->type, m->indices);
         }

         CHECK_FOR_GL_ERRORS("end of single mesh render (renderSingleEntity)");
     }

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void Orion::renderEntities(const CameraConfig& cc, const ProgramTransformProperties& transform_properties, bool adjacency) const
{
   assert(adjacency == false);

    GLuint array_buffer = 0, element_array_buffer = 0;

    if(transform_properties.world_to_view_matrix_location >= 0)
      glUniformMatrix4fv(transform_properties.world_to_view_matrix_location, 1, GL_FALSE, cc.world_to_view.e);

    if(transform_properties.view_to_clip_matrix_location >= 0)
      glUniformMatrix4fv(transform_properties.view_to_clip_matrix_location, 1, GL_FALSE, cc.view_to_clip.e);

    if(transform_properties.normal_matrix_location >= 0)
    {
      const Mat4 normal_matrix = cc.world_to_view.transpose().inverse();
      glUniformMatrix4fv(transform_properties.normal_matrix_location, 1, GL_FALSE, normal_matrix.e);
    }

    glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    for(uint32 i = 0; i < node_count; ++i)
    {
        const Entity* ent = entities[i];

        if(ent->raytracer || ent->invisible || ent->parent_index == crystals_node_index || i == rocket_node_index)
           continue; // skipped because it requires a special shader program (must be rendered explicitly)

        const uint32 mesh_count = node_mesh_offsets[i + 1] - node_mesh_offsets[i];

        if(transform_properties.object_to_world_matrix_location >= 0)
            glUniformMatrix4fv(transform_properties.object_to_world_matrix_location, 1, GL_FALSE, ent->global_xfrm.e);

        if(transform_properties.object_to_view_matrix_location >= 0)
            glUniformMatrix4fv(transform_properties.object_to_view_matrix_location, 1, GL_FALSE, (cc.world_to_view * ent->global_xfrm).e);

        if(transform_properties.object_to_clip_matrix_location >= 0)
            glUniformMatrix4fv(transform_properties.object_to_clip_matrix_location, 1, GL_FALSE, (cc.world_to_clip * ent->global_xfrm).e);

        if(transform_properties.previous_object_to_clip_matrix_location >= 0)
            glUniformMatrix4fv(transform_properties.previous_object_to_clip_matrix_location, 1, GL_FALSE, (cc.previous_world_to_clip * ent->previous_global_xfrm).e);

        for(uint32 j = 0; j < mesh_count; ++j)
        {
            const uint32 mesh_index = node_mesh_indices[node_mesh_offsets[i] + j];
            const MeshCacheItem* m = mesh_cache[mesh_index];

            assert(mesh_index < total_mesh_count);

           if(transform_properties.material_index_location >= 0)
               glUniform1f(transform_properties.material_index_location, float((mesh_materials[mesh_index] + 1) + 0.5) / (float)((num_materials + 1) * 2));

            if(m->array_buffer != array_buffer)
            {
                array_buffer = m->array_buffer;
                glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const GLvoid*)(sizeof(float) * 3));
            }

            if(adjacency)
            {
                if(m->element_array_adj_buffer != element_array_buffer)
                {
                    element_array_buffer = m->element_array_adj_buffer;
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
                }

                glDrawElements(GL_TRIANGLES_ADJACENCY, m->count * 2, m->type, m->indices_adj);
            }
            else
            {
                if(m->element_array_buffer != element_array_buffer)
                {
                    element_array_buffer = m->element_array_buffer;
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
                }

                glDrawElements(GL_TRIANGLES, m->count, m->type, m->indices);
            }

            CHECK_FOR_GL_ERRORS("end of single mesh render (renderEntities)");
        }
    }

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}


void Orion::createEntities()
{
    for(; !entities.empty(); entities.pop_back())
        delete entities.back();

    for(uint32 i = 0; i < node_count; ++i)
    {
        Entity* e = new Entity;
        entities.push_back(e);
    }

    stack<uint32> node_stack;
    node_stack.push(0);
    while(!node_stack.empty())
    {
        const uint32 i = node_stack.top();
        node_stack.pop();
        const uint32 child_count = node_child_offsets[i + 1] - node_child_offsets[i];
        for(uint32 j = 0; j < child_count; ++j)
        {
            const uint32 c = node_child_indices[node_child_offsets[i] + j];
            entities[c]->parent = entities[i];
            entities[c]->parent_index = i;
            node_stack.push(c);
        }
    }
}


// -------- THE CODE SECTION BELOW IS COPIED FROM BLENDER, FOR THE PURPOSE OF MATCHING MOTION CURVES BETWEEN BLENDER ANIMATION AND DEMO PLAYBACK CORRECTLY --------


/* The total length of the handles is not allowed to be more
 * than the horizontal distance between (v1-v4).
 * This is to prevent curve loops.
 */
void correct_bezpart(float v1[2], float v2[2], float v3[2], float v4[2])
{
	float h1[2], h2[2], len1, len2, len, fac;

	/* calculate handle deltas */
	h1[0] = v1[0] - v2[0];
	h1[1] = v1[1] - v2[1];

	h2[0] = v4[0] - v3[0];
	h2[1] = v4[1] - v3[1];

	/* calculate distances:
	 *  - len	= span of time between keyframes
	 *	- len1	= length of handle of start key
	 *	- len2  = length of handle of end key
	 */
	len = v4[0] - v1[0];
	len1 = fabsf(h1[0]);
	len2 = fabsf(h2[0]);

	/* if the handles have no length, no need to do any corrections */
	if ((len1 + len2) == 0.0f)
		return;

	/* the two handles cross over each other, so force them
	 * apart using the proportion they overlap
	 */
	if ((len1 + len2) > len) {
		fac = len / (len1 + len2);

		v2[0] = (v1[0] - fac * h1[0]);
		v2[1] = (v1[1] - fac * h1[1]);

		v3[0] = (v4[0] - fac * h2[0]);
		v3[1] = (v4[1] - fac * h2[1]);
	}
}

double sqrt3d(double d)
{
	if      (d == 0.0) return 0.0;
	else if (d <  0.0) return -exp(log(-d) / 3.0);
	else                         return  exp(log( d) / 3.0);
}

#define SMALL -1.0e-10

/* find root ('zero') */
static int findzero(float x, float q0, float q1, float q2, float q3, float *o)
{
	double c0, c1, c2, c3, a, b, c, p, q, d, t, phi;
	int nr = 0;

	c0 = q0 - x;
	c1 = 3.0f * (q1 - q0);
	c2 = 3.0f * (q0 - 2.0f * q1 + q2);
	c3 = q3 - q0 + 3.0f * (q1 - q2);

	if (c3 != 0.0) {
		a = c2 / c3;
		b = c1 / c3;
		c = c0 / c3;
		a = a / 3;

		p = b / 3 - a * a;
		q = (2 * a * a * a - a * b + c) / 2;
		d = q * q + p * p * p;

		if (d > 0.0) {
			t = sqrt(d);
			o[0] = (float)(sqrt3d(-q + t) + sqrt3d(-q - t) - a);

			if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) return 1;
			else return 0;
		}
		else if (d == 0.0) {
			t = sqrt3d(-q);
			o[0] = (float)(2 * t - a);

			if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) nr++;
			o[nr] = (float)(-t - a);

			if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) return nr + 1;
			else return nr;
		}
		else {
			phi = acos(-q / sqrt(-(p * p * p)));
			t = sqrt(-p);
			p = cos(phi / 3);
			q = sqrt(3 - 3 * p * p);
			o[0] = (float)(2 * t * p - a);

			if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) nr++;
			o[nr] = (float)(-t * (p + q) - a);

			if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) nr++;
			o[nr] = (float)(-t * (p - q) - a);

			if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) return nr + 1;
			else return nr;
		}
	}
	else {
		a = c2;
		b = c1;
		c = c0;

		if (a != 0.0) {
			/* discriminant */
			p = b * b - 4 * a * c;

			if (p > 0) {
				p = sqrt(p);
				o[0] = (float)((-b - p) / (2 * a));

				if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) nr++;
				o[nr] = (float)((-b + p) / (2 * a));

				if ((o[nr] >= (float)SMALL) && (o[nr] <= 1.000001f)) return nr + 1;
				else return nr;
			}
			else if (p == 0) {
				o[0] = (float)(-b / (2 * a));
				if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) return 1;
				else return 0;
			}
		}
		else if (b != 0.0) {
			o[0] = (float)(-c / b);

			if ((o[0] >= (float)SMALL) && (o[0] <= 1.000001f)) return 1;
			else return 0;
		}
		else if (c == 0.0) {
			o[0] = 0.0;
			return 1;
		}

		return 0;
	}
}

static void berekeny(float f1, float f2, float f3, float f4, float *o, int b)
{
	float t, c0, c1, c2, c3;
	int a;

	c0 = f1;
	c1 = 3.0f * (f2 - f1);
	c2 = 3.0f * (f1 - 2.0f * f2 + f3);
	c3 = f4 - f1 + 3.0f * (f2 - f3);

	for (a = 0; a < b; a++) {
		t = o[a];
		o[a] = c0 + t * c1 + t * t * c2 + t * t * t * c3;
	}
}

// -------- THE CODE SECTION ABOVE IS COPIED FROM BLENDER, FOR THE PURPOSE OF MATCHING MOTION CURVES BETWEEN BLENDER ANIMATION AND DEMO PLAYBACK CORRECTLY --------




float evaluateAnimationCurve(float evaltime,float v10,float v11,float v20,float v21,float v30,float v31,float v40,float v41)
{
   float v1[2], v2[2], v3[2], v4[2];

    v1[0]=v10;
    v1[1]=v11;
    v2[0]=v20;
    v2[1]=v21;
    v3[0]=v30;
    v3[1]=v31;
    v4[0]=v40;
    v4[1]=v41;

#if 0
   /* bezier interpolation */
   /* (v1, v2) are the first keyframe and its 2nd handle */
   v1[0] = prevbezt->vec[1][0];
   v1[1] = prevbezt->vec[1][1];
   v2[0] = prevbezt->vec[2][0];
   v2[1] = prevbezt->vec[2][1];
   /* (v3, v4) are the last keyframe's 1st handle + the last keyframe */
   v3[0] = bezt->vec[0][0];
   v3[1] = bezt->vec[0][1];
   v4[0] = bezt->vec[1][0];
   v4[1] = bezt->vec[1][1];
#endif

   /* adjust handles so that they don't overlap (forming a loop) */
   correct_bezpart(v1, v2, v3, v4);

   float opl[32];
   float cvalue=0;

   /* try to get a value for this position - if failure, try another set of points */
   int b = findzero(evaltime, v1[0], v2[0], v3[0], v4[0], opl);
   if (b) {
      berekeny(v1[1], v2[1], v3[1], v4[1], opl, 1);
      cvalue = opl[0];
      return cvalue;
   }
   return cvalue;
}

double Orion::getFirstKeyTime(uint32 channel_index) const
{
   double mintime=1e9;
   for(uint32 j = 0; j < 7; ++j)
   {
       const uint32 offset = channel_key_offsets[channel_index * 7 + j];
       const uint32 key_count = (channel_key_offsets[channel_index * 7 + j + 1] - offset) / 6;
       if (key_count > 0)
       {
          const double* ch = key_data + offset;

          for(uint32 k = 0; k < key_count; ++k)
          {
             if(ch[k * 6 + 0] < mintime)
                mintime = ch[k * 6 + 0];
          }
       }
   }
   return mintime;
}

double Orion::getLastKeyTime(uint32 channel_index) const
{
   double maxtime=0;
   for(uint32 j = 0; j < 7; ++j)
   {
       const uint32 offset = channel_key_offsets[channel_index * 7 + j];
       const uint32 key_count = (channel_key_offsets[channel_index * 7 + j + 1] - offset) / 6;
       if (key_count > 0)
       {
          const double* ch = key_data + offset;

          for(uint32 k = 0; k < key_count; ++k)
          {
             if(ch[k * 6 + 0] > maxtime)
                maxtime = ch[k * 6 + 0];
          }
       }
   }
   return maxtime;
}

Mat4 Orion::getGlobalTransformation(uint32 channel_index, const double time, Vec3* local_origin, Vec3* local_euler_angles) const
{
   assert(channel_index < channel_nodes_length);

   const Entity* ent = entities[channel_nodes[channel_index]];

   assert(ent);

   double data[6];

   if (local_origin)
   {
      data[0] = local_origin->x;
      data[1] = local_origin->y;
      data[2] = local_origin->z;
   }

   if (local_euler_angles)
   {
      data[3] = local_euler_angles->x;
      data[4] = local_euler_angles->y;
      data[5] = local_euler_angles->z;
   }

   for(uint32 j = 0; j < 6; ++j)
   {
       const uint32 offset = channel_key_offsets[channel_index * 7 + j];
       const uint32 key_count = (channel_key_offsets[channel_index * 7 + j + 1] - offset) / 6;
       if (key_count > 0)
       {
           const double* ch = key_data + offset;
           if(time <= ch[0 * 6 + 0])
           {
               data[j] = ch[0 * 6 + 1];
           }
           else if(time >= ch[(key_count - 1) * 6 + 0])
           {
               data[j] = ch[(key_count - 1) * 6 + 1];
           }
           else
           {
               assert(key_count > 1);

               // Check invariants.
               for(uint32 k = 1; k < key_count; ++k)
               {
                  assert(ch[k * 6 + 0] > ch[(k - 1) * 6 + 0]);
               }

#if 1
               // Perform a binary search for the corresponding pair of keys.
               uint32 l = 0, r = key_count - 1;

               while((r - l) > 1)
               {
                   const uint32 m = (l + r) >> 1;
                   const double kt = ch[m * 6 + 0];
                   if(kt <= time)
                       l = m;
                   else if(kt > time)
                       r = m;
               }

               assert((r - l) == 1);
               assert(r < key_count);

               const double *first_key = ch + l * 6;
               const double *second_key = ch + r * 6;

#else
               // Perform a linear search for the corresponding pair of keys.
               double latest_kt = 0;
               const double* first_key = ch;
               for(uint32 k = 0; k < (key_count - 1); ++k)
               {
                  const double kt = ch[k * 6 + 0];
                  if(kt >= latest_kt && kt <= time)
                  {
                     latest_kt = kt;
                     first_key = ch + k * 6;
                  }
               }
               const double* second_key = first_key + 6;
#endif
               assert(second_key[0] >= first_key[0]);

               if((second_key[0] - first_key[0]) < 1.0 / 24.0)
               {
                   // Interpret this as an instant step.
                   const double mt = (second_key[0] + first_key[0]) / 2.0;
                   data[j] = (time > mt) ? second_key[1] : first_key[1];
               }
               else
               {
                   if(first_key[2] == 0 && first_key[3] == 0 && first_key[4] == 0 && first_key[5] == 0)
                   //if(second_key[2] == 0 && second_key[3] == 0 && first_key[4] == 0 && first_key[5] == 0)
                   {
                      // Linear.
                      const double x = (time - first_key[0]) / (second_key[0] - first_key[0]);
                      data[j] = first_key[1] * (1.0 - x) + second_key[1] * x;
                      //log("%d=%f\n",j,data[j]);
                   }
                   else
                   {
                      // Bezier.
                      data[j] = evaluateAnimationCurve(time, first_key[0], first_key[1], first_key[4], first_key[5], second_key[2], second_key[3], second_key[0], second_key[1]);
                   }
               }
           }
       }
   }

   const Vec3 o = Vec3(data[0], data[1], data[2]);

   if(local_origin)
      *local_origin = o;

   if(local_euler_angles)
   {
      local_euler_angles->x = data[3];
      local_euler_angles->y = data[4];
      local_euler_angles->z = data[5];
   }

   const Mat4 xrot = Mat4::rotation(data[3] * M_PI / 180.0f, Vec3(1, 0, 0));
   const Mat4 yrot = Mat4::rotation(data[4] * M_PI / 180.0f, Vec3(0, 1, 0));
   const Mat4 zrot = Mat4::rotation(data[5] * M_PI / 180.0f, Vec3(0, 0, 1));

   // note: nested and animated transformations don't work yet!
   return (ent->parent ? ent->parent->global_xfrm : Mat4::identity()) * Mat4::translation(o) * zrot * yrot * xrot * Mat4::scale(ent->local_scale);
}


double Orion::getXFov(uint32 channel_index, const double time) const
{
   assert(channel_index < channel_nodes_length);

   const Entity* ent = entities[channel_nodes[channel_index]];

   assert(ent);

   double data = 0;

   const uint32 j = 6;
   {
       const uint32 offset = channel_key_offsets[channel_index * 7 + j];
       const uint32 key_count = (channel_key_offsets[channel_index * 7 + j + 1] - offset) / 6;
       if (key_count > 0)
       {
           const double* ch = key_data + offset;
           if(time <= ch[0 * 6 + 0])
           {
               data = ch[0 * 6 + 1];
           }
           else if(time >= ch[(key_count - 1) * 6 + 0])
           {
               data = ch[(key_count - 1) * 6 + 1];
           }
           else
           {
               assert(key_count > 1);

               // Check invariants.
               for(uint32 k = 1; k < key_count; ++k)
               {
                  assert(ch[k * 6 + 0] > ch[(k - 1) * 6 + 0]);
               }

#if 1
               // Perform a binary search for the corresponding pair of keys.
               uint32 l = 0, r = key_count - 1;

               while((r - l) > 1)
               {
                   const uint32 m = (l + r) >> 1;
                   const double kt = ch[m * 6 + 0];
                   if(kt <= time)
                       l = m;
                   else if(kt > time)
                       r = m;
               }

               assert((r - l) == 1);
               assert(r < key_count);

               const double *first_key = ch + l * 6;
               const double *second_key = ch + r * 6;

#else
               // Perform a linear search for the corresponding pair of keys.
               double latest_kt = 0;
               const double* first_key = ch;
               for(uint32 k = 0; k < (key_count - 1); ++k)
               {
                  const double kt = ch[k * 6 + 0];
                  if(kt >= latest_kt && kt <= time)
                  {
                     latest_kt = kt;
                     first_key = ch + k * 6;
                  }
               }
               const double* second_key = first_key + 6;
#endif
               assert(second_key[0] >= first_key[0]);

               if((second_key[0] - first_key[0]) < 1.0 / 24.0)
               {
                   // Interpret this as an instant step.
                   const double mt = (second_key[0] + first_key[0]) / 2.0;
                   data = (time > mt) ? second_key[1] : first_key[1];
               }
               else
               {
                   if(first_key[2] == 0 && first_key[3] == 0 && first_key[4] == 0 && first_key[5] == 0)
                   //if(second_key[2] == 0 && second_key[3] == 0 && first_key[4] == 0 && first_key[5] == 0)
                   {
                      // Linear.
                      const double x = (time - first_key[0]) / (second_key[0] - first_key[0]);
                      data = first_key[1] * (1.0 - x) + second_key[1] * x;
                   }
                   else
                   {
                      // Bezier.
                      data = evaluateAnimationCurve(time, first_key[0], first_key[1], first_key[4], first_key[5], second_key[2], second_key[3], second_key[0], second_key[1]);
                   }
               }
           }
       }
   }

   return data;
}

uint32 Orion::getChannelIndexForEntity(const Entity* sent) const
{
   for(uint32 i = 0; i < channel_nodes_length; ++i)
   {
      const Entity* ent = entities[channel_nodes[i]];
      if(ent == sent)
         return i;
   }
   assert(false);
   return 0;
}

double Orion::getMostRecentCameraMarkerTime(double time) const
{
    uint32 camera_entity_index = 0;
     const uint32 frame = (uint32)std::floor(time * 24.0);
     uint32 latest_marker_frame = 0;
     for(uint32 i = 0; i < marker_count; ++i)
     {
         const Marker& m = markers[i];
         const uint32 f=max(0,m.frame);
         if(f <= frame && m.node_index != 0 && f >= latest_marker_frame)
         {
             camera_entity_index = m.node_index - 1;
             latest_marker_frame = f;
         }
     }
   return double(latest_marker_frame) / 24.0;
}


void Orion::updateAnimations(const double time, Viewport& vp_out) const
{
    // Process markers.
    {
        uint32 camera_entity_index = 0;
        const uint32 frame = (uint32)std::floor(time * 24.0);
        uint32 latest_marker_frame = 0;
        for(uint32 i = 0; i < marker_count; ++i)
        {
            const Marker& m = markers[i];
            const uint32 f=max(0,m.frame);
            if(f <= frame && m.node_index != 0 && f >= latest_marker_frame)
            {
                camera_entity_index = m.node_index - 1;
                latest_marker_frame = f;
            }
        }
        vp_out.camera_entity_index = camera_entity_index;
    }

    // Initialise entity transformations from static node transformations.
    {
        stack<pair<uint32, Mat4> > node_stack;
        node_stack.push(pair<uint32, Mat4>(0, Mat4::identity()));

        while(!node_stack.empty())
        {
            const uint32 i = node_stack.top().first;
            const Mat4 x = node_stack.top().second * Mat4(node_transformations + i * 16);

            entities[i]->global_xfrm = x;
            entities[i]->previous_global_xfrm = x;
            entities[i]->local_scale = Vec3(x.column(0).length(), x.column(1).length(), x.column(2).length());

            node_stack.pop();
            const uint32 child_count = node_child_offsets[i + 1] - node_child_offsets[i];
            for(uint32 j = 0; j < child_count; ++j)
                node_stack.push(pair<uint32, Mat4>(node_child_indices[node_child_offsets[i] + j], x));
        }
    }

    // Apply animation channels.

     for(uint32 i = 0; i < channel_nodes_length; ++i)
     {
         Entity* ent = entities[channel_nodes[i]];

         // note: nested and animated transformations don't work yet!
         ent->global_xfrm = getGlobalTransformation(i, time, &ent->local_origin, &ent->local_euler_angles);
         ent->previous_global_xfrm = getGlobalTransformation(i, time - 1.0 / 24.0, NULL, NULL); // for motion blur
     }


     // Apply special animations.
     {
        assert(windmill_vanes_node_index < entities.size());
        Entity* ent = entities[windmill_vanes_node_index];
        assert(ent);
        static const float vel = 0.6f;
        static const Vec3 axis(1,0,0);
        ent->global_xfrm = ent->global_xfrm * Mat4::rotation(time * vel, axis);
        ent->previous_global_xfrm = ent->previous_global_xfrm * Mat4::rotation((time - 1.0 / 24.0) * vel, axis);
     }

}

double Orion::getEntityXFovForTime(uint32 entity_index, const double time) const
{
    // Apply animation channels.

     for(uint32 i = 0; i < channel_nodes_length; ++i)
     {
         if(channel_nodes[i] == entity_index)
         {
            return getXFov(i, time);
         }
     }

   return 0;
}

Mat4 Orion::getEntityGlobalTransformationForTime(uint32 entity_index, const double time) const
{
   Mat4 global_xfrm = Mat4::identity();

    // Initialise entity transformations from static node transformations.
    {
        stack<pair<uint32, Mat4> > node_stack;
        node_stack.push(pair<uint32, Mat4>(0, Mat4::identity()));

        while(!node_stack.empty())
        {
            const uint32 i = node_stack.top().first;
            const Mat4 x = node_stack.top().second * Mat4(node_transformations + i * 16);

            if(entity_index == i)
            {
               global_xfrm = x;
            }

            node_stack.pop();
            const uint32 child_count = node_child_offsets[i + 1] - node_child_offsets[i];
            for(uint32 j = 0; j < child_count; ++j)
                node_stack.push(pair<uint32, Mat4>(node_child_indices[node_child_offsets[i] + j], x));
        }
    }

    // Apply animation channels.

     for(uint32 i = 0; i < channel_nodes_length; ++i)
     {
         if(channel_nodes[i] == entity_index)
         {
            // note: nested and animated transformations don't work yet!
            global_xfrm = getGlobalTransformation(i, time, NULL, NULL);
         }
     }

   return global_xfrm;
}



