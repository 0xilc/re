#include "io/gltf/material.h"

#include "core/color.h"
#include "core/log.h"
#include "io/gltf/context.h"
#include "io/gltf/texture.h"
#include "scene/material.h"

namespace re {
namespace {
const char* material_name(const cgltf_material& material) {
  return material.name != nullptr ? material.name : "material";
}
}

uint16_t import_material(GltfContext& ctx, const cgltf_material* material) {
  if (material == nullptr) return ctx.placeholder_material;

  const auto it = ctx.material_ids.find(material);
  if (it != ctx.material_ids.end()) return it->second;

  if (!material->has_pbr_metallic_roughness) {
    log_error("gltf: '{}' has no pbrMetallicRoughness, falling back to defaults", material_name(*material));
  }

  const cgltf_pbr_metallic_roughness& pbr = material->pbr_metallic_roughness;

  Material out = make_material(
    Spectrum{pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2]},
    pbr.metallic_factor,
    pbr.roughness_factor,
    Spectrum{material->emissive_factor[0], material->emissive_factor[1], material->emissive_factor[2]}
  );

  if (material->has_ior) out.ior = material->ior.ior;

  if (material->has_emissive_strength) {
    out.emission = out.emission * material->emissive_strength.emissive_strength;
  }

  out.base_color_tex = import_texture(ctx, pbr.base_color_texture, ColorSpace::SRGB);
  out.metallic_roughness_tex = import_texture(ctx, pbr.metallic_roughness_texture, ColorSpace::Linear);
  out.normal_tex = import_texture(ctx, material->normal_texture, ColorSpace::Linear);
  out.emissive_tex = import_texture(ctx, material->emissive_texture, ColorSpace::SRGB);

  if (material->occlusion_texture.texture != nullptr) {
    log_info("gltf: occlusion texture on '{}' ignored, path tracing computes it",
             material_name(*material));
  }

  if (material->alpha_mode != cgltf_alpha_mode_opaque) {
    log_error("gltf: '{}' wants alpha mode {}, rendered opaque for now",
              material_name(*material), static_cast<int>(material->alpha_mode));
  }

  const uint16_t id = ctx.scene->add_material(out);
  ctx.material_ids[material] = id;

  return id;
}

} // namespace re