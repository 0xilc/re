#include "scene/mesh.h"
#include "scene/mesh_view.h"

namespace re {

MeshView make_meshview(const Mesh& mesh) {
  MeshView mv;
  mv.positions = mesh.positions.data();
  mv.normals   = mesh.normals.empty() ? nullptr : mesh.normals.data();
  mv.indices   = mesh.indices.data();
  mv.mat_ids   = mesh.mat_ids.data();
  mv.uvs       = mesh.uvs.empty() ? nullptr : mesh.uvs.data();
  mv.vertex_count = static_cast<int>(mesh.positions.size());
  mv.triangle_count = static_cast<int>(mesh.indices.size() / 3);
  return mv;
};

} // namespace re