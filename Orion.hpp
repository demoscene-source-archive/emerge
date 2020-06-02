#pragma once

#include <vector>

#include "Common.hpp"

struct Entity
{
    Entity()
    {
        parent = NULL;
        local_origin = Vec3(0, 0, 0);
        local_euler_angles = Vec3(0, 0, 0);
        local_scale = Vec3(1, 1, 1);
        raytracer = false;
        invisible = false;
        parent_index = 0;
    }

    bool raytracer;
    bool invisible;
    Vec3 local_origin, local_euler_angles, local_scale;
    Mat4 global_xfrm, previous_global_xfrm;
    Entity* parent;
    unsigned long int parent_index;
};

struct MeshCacheItem
{
    GLuint array_buffer, element_array_buffer, element_array_adj_buffer;
    GLsizei count;
    GLenum type;
    const GLvoid* indices;
    const GLvoid* indices_adj;
};

struct ProgramTransformProperties
{
    ProgramTransformProperties()
    {
        object_to_world_matrix_location = -1;
        world_to_view_matrix_location = -1;
        view_to_clip_matrix_location = -1;
        object_to_view_matrix_location = -1;
        object_to_clip_matrix_location = -1;
        previous_object_to_clip_matrix_location = -1;
        normal_matrix_location = -1;
        material_index_location = -1;
    }

    GLint object_to_world_matrix_location;
    GLint world_to_view_matrix_location;
    GLint view_to_clip_matrix_location;
    GLint object_to_view_matrix_location;
    GLint object_to_clip_matrix_location;
    GLint previous_object_to_clip_matrix_location;
    GLint normal_matrix_location;
    GLint material_index_location;
};

struct Viewport
{
    uint32 camera_entity_index;
};

struct CameraConfig
{
    Mat4 world_to_view;
    Mat4 view_to_clip;
    Mat4 world_to_clip;
    Mat4 previous_world_to_clip;
};

struct Orion
{
    ~Orion();

    void uploadMeshes();
    void createEntities();
    void updateAnimations(const double time, Viewport& vp_out) const;
    void renderEntities(const CameraConfig&, const ProgramTransformProperties&, bool adjacency) const;
    void renderSingleEntity(const CameraConfig&, uint32 entity_index, const ProgramTransformProperties&, bool adjacency) const;
    Mat4 getGlobalTransformation(uint32 channel_index, const double time, Vec3* local_origin, Vec3* local_euler_angles) const;
    double getXFov(uint32 channel_index, const double time) const;
    uint32 getChannelIndexForEntity(const Entity*) const;
    Mat4 getEntityGlobalTransformationForTime(uint32 entity_index, const double time) const;
    double getEntityXFovForTime(uint32 entity_index, const double time) const;
    double getFirstKeyTime(uint32 channel_index) const;
    double getLastKeyTime(uint32 channel_index) const;
    double getMostRecentCameraMarkerTime(double time) const;

    std::vector<MeshCacheItem*> mesh_cache;
    std::vector<Entity*> entities;
};

