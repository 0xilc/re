#include "io/gltf/helpers/mesh_tangents.h"

#include <mikktspace.h>

#include <utility>
#include <vector>

#include "core/log.h"

namespace re {
namespace {

int corner_of(int face, int vert) { return face * 3 + vert; }

Mesh& mesh_of(const SMikkTSpaceContext* ctx) {
  return *static_cast<Mesh*>(ctx->m_pUserData);
}

int get_num_faces(const SMikkTSpaceContext* ctx) {
  return static_cast<int>(mesh_of(ctx).indices.size() / 3);
}

int get_num_vertices_of_face(const SMikkTSpaceContext*, const int) { return 3; }

void get_position(const SMikkTSpaceContext* ctx, float out[], const int face, const int vert) {
  const float3& p = mesh_of(ctx).positions[corner_of(face, vert)];
  out[0] = p.x;
  out[1] = p.y;
  out[2] = p.z;
}

void get_normal(const SMikkTSpaceContext* ctx, float out[], const int face, const int vert) {
  const float3& n = mesh_of(ctx).normals[corner_of(face, vert)];
  out[0] = n.x;
  out[1] = n.y;
  out[2] = n.z;
}

void get_texcoord(const SMikkTSpaceContext* ctx, float out[], const int face, const int vert) {
  const float2& uv = mesh_of(ctx).uvs[corner_of(face, vert)];
  out[0] = uv.x;
  out[1] = uv.y;
}

void set_tspace_basic(const SMikkTSpaceContext* ctx, const float tangent[], const float sign,
                      const int face, const int vert) {
  mesh_of(ctx).tangents[corner_of(face, vert)] =
      float4{tangent[0], tangent[1], tangent[2], sign};
}

void unweld(Mesh& mesh) {
  const size_t corner_count = mesh.indices.size();

  std::vector<float3> positions(corner_count);
  std::vector<float3> normals(corner_count);
  std::vector<float2> uvs(corner_count);

  for (size_t i = 0; i < corner_count; ++i) {
    const uint32_t v = mesh.indices[i];

    positions[i] = mesh.positions[v];
    normals[i]   = mesh.normals[v];
    uvs[i]       = mesh.uvs[v];
  }

  mesh.positions = std::move(positions);
  mesh.normals   = std::move(normals);
  mesh.uvs       = std::move(uvs);

  for (size_t i = 0; i < corner_count; ++i) mesh.indices[i] = static_cast<uint32_t>(i);
}

}  // namespace

bool generate_tangents(Mesh& mesh) {
  if (mesh.normals.empty() || mesh.uvs.empty()) return false;
  if (mesh.indices.empty() || mesh.indices.size() % 3 != 0) return false;

  unweld(mesh);
  mesh.tangents.assign(mesh.positions.size(), float4{0.0f, 0.0f, 0.0f, 1.0f});

  SMikkTSpaceInterface interface{};
  interface.m_getNumFaces          = get_num_faces;
  interface.m_getNumVerticesOfFace = get_num_vertices_of_face;
  interface.m_getPosition          = get_position;
  interface.m_getNormal            = get_normal;
  interface.m_getTexCoord          = get_texcoord;
  interface.m_setTSpaceBasic       = set_tspace_basic;

  SMikkTSpaceContext context{};
  context.m_pInterface = &interface;
  context.m_pUserData  = &mesh;

  if (genTangSpaceDefault(&context) == 0) {
    log_error("gltf: mikktspace could not generate tangents");
    mesh.tangents.clear();
    return false;
  }

  return true;
}

}  // namespace re
