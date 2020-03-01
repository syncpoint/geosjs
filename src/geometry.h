#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include <napi.h>
#include <geos_c.h>

class Geometry : public Napi::ObjectWrap<Geometry> {
 public:
  GEOSGeometry *geometry;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env, Napi::External<GEOSGeometry> geometry);
  Geometry(const Napi::CallbackInfo& info);
  ~Geometry();
  Napi::Value GetSRID(const Napi::CallbackInfo& info);
  void SetSRID(const Napi::CallbackInfo& info);
  Napi::Value GetType(const Napi::CallbackInfo& info);
  Napi::Value GetNumGeometries(const Napi::CallbackInfo& info);
  Napi::Value GetNumPoints(const Napi::CallbackInfo& info);
  Napi::Value GetPointN(const Napi::CallbackInfo& info);
  Napi::Value GetStartPoint(const Napi::CallbackInfo& info);
  Napi::Value GetEndPoint(const Napi::CallbackInfo& info);
  Napi::Value GetX(const Napi::CallbackInfo& info);
  Napi::Value GetY(const Napi::CallbackInfo& info);
  Napi::Value GetZ(const Napi::CallbackInfo& info);
  Napi::Value GetGeometryN(const Napi::CallbackInfo& info);
  Napi::Value Difference(const Napi::CallbackInfo& info);
  Napi::Value Union(const Napi::CallbackInfo& info);
  Napi::Value Buffer(const Napi::CallbackInfo& info);
  Napi::Value AsPolygon(const Napi::CallbackInfo& info);
  Napi::Value AsBoundary(const Napi::CallbackInfo& info);
  Napi::Value Interpolate(const Napi::CallbackInfo& info);
  Napi::Value InterpolateNormalized(const Napi::CallbackInfo& info);
  Napi::Value Transform(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
};


#endif /* __GEOMETRY_H */