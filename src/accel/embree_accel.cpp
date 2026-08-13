#include "accel/embree_accel.h"
#include <embree4/rtcore.h>
#include <cassert>
#include <embree4/rtcore_device.h>
#include <embree4/rtcore_geometry.h>
#include <embree4/rtcore_ray.h>
#include <embree4/rtcore_scene.h>
#include "core/log.h"


namespace re {

namespace {
void embree_error(void*, RTCError code, const char* message)  {
  log_error("embree error {}: {}", static_cast<int>(code), message);
}

RTCRayHit make_rayhit(const Ray& ray) {
  RTCRayHit rh;
  rh.ray.org_x = ray.origin.x;
  rh.ray.org_y = ray.origin.y;
  rh.ray.org_z = ray.origin.z;
  rh.ray.dir_x = ray.dir.x;
  rh.ray.dir_y = ray.dir.y;
  rh.ray.dir_z = ray.dir.z;
  rh.ray.tnear = ray.tmin;
  rh.ray.tfar  = ray.tmax;
  rh.ray.mask  = 0xFFFFFFFF;
  rh.ray.flags = 0;
  rh.ray.time  = 0.0f;
  rh.ray.id    = 0;

  rh.hit.geomID    = RTC_INVALID_GEOMETRY_ID;
  rh.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
  return rh;
}
} // namespace

EmbreeAccel::EmbreeAccel() {
  RTCDevice device = rtcNewDevice(nullptr);
  rtcSetDeviceErrorFunction(device, embree_error, nullptr);
  device_ = device;
}

EmbreeAccel::~EmbreeAccel() {
  if (scene_ != nullptr) rtcReleaseScene(static_cast<RTCScene>(scene_));
  if (device_ != nullptr) rtcReleaseDevice(static_cast<RTCDevice>(device_));
}

void EmbreeAccel::build(const GeometryView& geometry) {
  RTCDevice device = static_cast<RTCDevice>(device_);

  if (scene_ != nullptr) rtcReleaseScene(static_cast<RTCScene>(scene_));

  RTCScene    scene = rtcNewScene(device);
  RTCGeometry  mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

  rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                            geometry.positions, 0, sizeof(float3),
                            static_cast<size_t>(geometry.vertex_count));

  rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                            geometry.indices, 0, 3 * sizeof(uint32_t),
                            static_cast<size_t>(geometry.triangle_count));
  
  rtcCommitGeometry(mesh);

  const unsigned geom_id = rtcAttachGeometry(scene, mesh);
  assert(geom_id == 0);
  (void)geom_id;
  rtcReleaseGeometry(mesh);
  
  rtcCommitScene(scene);

  scene_          = scene;
  triangle_count_ = geometry.triangle_count;
}

Hit EmbreeAccel::intersect(const Ray& ray) const {
  Hit hit;

  RTCRayHit rh = make_rayhit(ray);
  rtcIntersect1(static_cast<RTCScene>(scene_), &rh, nullptr);

  if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID) return hit;

  hit.t = rh.ray.tfar;
  hit.u = rh.hit.u;
  hit.v = rh.hit.v;
  hit.primitive_id = static_cast<int>(rh.hit.primID);
  hit.instance_id = 0;

  return hit;
}

bool EmbreeAccel::occluded(const Ray& ray) const {
  RTCRay r;
  r.org_x = ray.origin.x;
  r.org_y = ray.origin.y;
  r.org_z = ray.origin.z;
  r.dir_x = ray.dir.x;
  r.dir_y = ray.dir.y;
  r.dir_z = ray.dir.z;
  r.tnear = ray.tmin;
  r.tfar  = ray.tmax;
  r.mask  = 0xFFFFFFFF;
  r.flags = 0;
  r.time  = 0.0f;
  r.id    = 0;

  rtcOccluded1(static_cast<RTCScene>(scene_), &r, nullptr);

  return r.tfar < 0.0f;
}
}