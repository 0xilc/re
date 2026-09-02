#include "io/gltf/texture.h"

#include <cstring>
#include <filesystem>
#include <string>
#include <utility>

#include "core/log.h"
#include "scene/material.h"

namespace re {
namespace {
WrapMode to_wrap(cgltf_wrap_mode mode) {
  switch (mode) {
    case cgltf_wrap_mode_clamp_to_edge:    return WrapMode::Clamp;
    case cgltf_wrap_mode_mirrored_repeat:  return WrapMode::Repeat;  // desteklenmiyor
    case cgltf_wrap_mode_repeat:
    default:                               return WrapMode::Repeat;
  }
}

TexFilter to_filter(cgltf_filter_type min_filter) {
  switch (min_filter) {
    case cgltf_filter_type_nearest:
    case cgltf_filter_type_nearest_mipmap_nearest:
    case cgltf_filter_type_nearest_mipmap_linear:
      return TexFilter::Nearest;

    case cgltf_filter_type_linear:
    case cgltf_filter_type_linear_mipmap_nearest:
      return TexFilter::Bilinear;

    case cgltf_filter_type_linear_mipmap_linear:
    case cgltf_filter_type_undefined:
    default:
      return TexFilter::Trilinear;
  }
}

bool fill_source(const GltfContext& ctx, const cgltf_image& image, TextureDesc& desc) {
  if (image.buffer_view != nullptr) {
    const uint8_t* data = cgltf_buffer_view_data(image.buffer_view);
    if (data == nullptr) return false;

    desc.bytes.assign(data, data + image.buffer_view->size);
    return true;
  }

  if (image.uri == nullptr) return false;

  if (std::strncmp(image.uri, "data:", 5) == 0) {
    log_error("gltf: data: uri images are not supported yet ('{}')", desc.name);
    return false;
  }

  std::string uri = image.uri;
  uri.resize(cgltf_decode_uri(uri.data()));

  desc.path = (std::filesystem::path(ctx.base_dir) / uri).string();
  return true;
}

const char* image_name(const cgltf_texture& texture) {
  if (texture.image != nullptr && texture.image->name != nullptr) return texture.image->name;
  if (texture.name != nullptr) return texture.name;
  return "texture";
}

} // namespace 

uint32_t import_texture(GltfContext& ctx, const cgltf_texture_view& view, ColorSpace space,
                        bool is_normal_map) {
  if (view.texture == nullptr) return kNoTexture;

  const cgltf_texture& texture = *view.texture;

  if (view.texcoord != 0) {
    log_error("gltf: '{}' wants TEXCOORD_{}, only set 0 is read", image_name(texture),
              view.texcoord);
  }

  if (view.has_transform) {
    log_error("gltf: KHR_texture_transform on '{}' is ignored", image_name(texture));
  }

  const auto key = std::make_pair(view.texture, space);
  const auto it = ctx.texture_ids.find(key);
  if (it != ctx.texture_ids.end()) return it->second;

  if (texture.image == nullptr) {
    log_error("gltf: texture '{}' has no image", image_name(texture));
    return kNoTexture;
  }

  if (texture.has_basisu || texture.has_webp) {
    log_error("gltf: '{}' is basisu/webp, which the decoder cannot read", image_name(texture));
    return kNoTexture;
  }

  TextureDesc desc;
  desc.name           = image_name(texture);
  desc.color_space    = space;
  desc.is_normal_map  = is_normal_map;

  if (texture.sampler != nullptr) {
    desc.wrap_u = to_wrap(texture.sampler->wrap_s);
    desc.wrap_v = to_wrap(texture.sampler->wrap_t);
    desc.filter = to_filter(texture.sampler->min_filter);
  }

  if (!fill_source(ctx, *texture.image, desc)) {
    log_error("gltf: image '{}' has no readable source", desc.name);
    return kNoTexture;
  }

  const uint32_t id = static_cast<uint32_t>(ctx.scene->texture_descs.size());
  ctx.scene->texture_descs.push_back(std::move(desc));
  ctx.texture_ids[key] = id;

  return id;
}

} // namespace re