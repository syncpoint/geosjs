#include "geometry.h"
#include "transform.h"

Napi::FunctionReference Geometry::constructor;

Napi::Object Geometry::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Geometry", {
    InstanceMethod("getSRID", &Geometry::GetSRID),
    InstanceMethod("setSRID", &Geometry::SetSRID),
    InstanceMethod("getNumPoints", &Geometry::GetNumPoints),
    InstanceMethod("getPointN", &Geometry::GetPointN),
    InstanceMethod("getStartPoint", &Geometry::GetStartPoint),
    InstanceMethod("getEndPoint", &Geometry::GetEndPoint),
    InstanceMethod("getX", &Geometry::GetX),
    InstanceMethod("getY", &Geometry::GetY),
    InstanceMethod("getZ", &Geometry::GetZ),
    InstanceMethod("getGeometryN", &Geometry::GetGeometryN),
    InstanceMethod("difference", &Geometry::Difference),
    InstanceMethod("union", &Geometry::Union),
    InstanceMethod("buffer", &Geometry::Buffer),
    InstanceMethod("asPolygon", &Geometry::AsPolygon),
    InstanceMethod("asBoundary", &Geometry::AsBoundary),
    InstanceMethod("interpolate", &Geometry::Interpolate),
    InstanceMethod("interpolateNormalized", &Geometry::InterpolateNormalized),
    InstanceMethod("transform", &Geometry::Transform)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  return exports;
}

Napi::Object Geometry::NewInstance(Napi::Env env, Napi::External<GEOSGeometry> geometry) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object object = constructor.New({ geometry });
  return scope.Escape(napi_value(object)).ToObject();
}

Geometry::Geometry(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Geometry>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Napi::External<GEOSGeometry> external = info[0].As<Napi::External<GEOSGeometry>>();
  this->geometry = external.Data();
}

Geometry::~Geometry() {
  GEOSGeom_destroy(this->geometry);
}

Napi::Value Geometry::GetSRID(const Napi::CallbackInfo& info) {
  int srid = GEOSGetSRID(this->geometry);
  return Napi::Number::New(info.Env(), srid);
}

void Geometry::SetSRID(const Napi::CallbackInfo& info) {
  // TODO: check arguments
  int srid = info[0].As<Napi::Number>().Int32Value();
  GEOSSetSRID(this->geometry, srid);
}

Napi::Value Geometry::GetNumPoints(const Napi::CallbackInfo& info) {
  int num = GEOSGeomGetNumPoints(this->geometry);
  return Napi::Number::New(info.Env(), num);
}

Napi::Value Geometry::GetPointN(const Napi::CallbackInfo& info) {
  // TODO: check arguments
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  // 0-based:
  double n = info[0].As<Napi::Number>().Int32Value();

  // support negative indexes:
  if (n < 0) {
    int num = GEOSGeomGetNumPoints(this->geometry);
    n = num + n;
  }

  GEOSGeometry *geometry = GEOSGeomGetPointN(this->geometry, n);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::GetStartPoint(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  GEOSGeometry *geometry = GEOSGeomGetStartPoint(this->geometry);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::GetEndPoint(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  GEOSGeometry *geometry = GEOSGeomGetEndPoint(this->geometry);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::GetX(const Napi::CallbackInfo& info) {
  double value;
  GEOSGeomGetX(this->geometry, &value);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value Geometry::GetY(const Napi::CallbackInfo& info) {
  double value;
  GEOSGeomGetY(this->geometry, &value);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value Geometry::GetZ(const Napi::CallbackInfo& info) {
  double value;
  GEOSGeomGetZ(this->geometry, &value);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value Geometry::GetGeometryN(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int n = info[0].As<Napi::Number>().Int64Value();
  // TODO: check bounds - int num = GEOSGetNumGeometries(this->geometry);

  GEOSGeometry* geometry = GEOSGeom_clone(GEOSGetGeometryN(this->geometry, n));
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::Difference(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  GEOSGeometry* g1 = this->geometry;
  GEOSGeometry* g2 = Napi::ObjectWrap<Geometry>::Unwrap(info[0].As<Napi::Object>())->geometry;
  GEOSGeometry* geometry = GEOSDifference(g1, g2);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::Union(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  GEOSGeometry* g1 = this->geometry;
  GEOSGeometry* g2 = Napi::ObjectWrap<Geometry>::Unwrap(info[0].As<Napi::Object>())->geometry;
  GEOSGeometry* geometry = GEOSUnion(g1, g2);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::Buffer(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  double width = info[0].As<Napi::Number>().DoubleValue();
  int quadsegs = info[1].As<Napi::Number>().Int64Value();
  int endCapStyle = info[2].As<Napi::Number>().Int64Value();
  int joinStyle = info[3].As<Napi::Number>().Int64Value();

  GEOSGeometry *geometry = GEOSBufferWithStyle(
    this->geometry,
    width,
    quadsegs,
    endCapStyle,
    joinStyle,
    0.0
  );

  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::AsPolygon(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  GEOSGeometry* shell = GEOSGeom_createLinearRing(GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(this->geometry)));
  GEOSGeometry** holes = NULL;
  unsigned int nholes = 0;
  GEOSGeometry* geometry = GEOSGeom_createPolygon(shell, holes, nholes);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::AsBoundary(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  GEOSGeometry* geometry = GEOSBoundary(this->geometry);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}


Napi::Value Geometry::Interpolate(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  double d = info[0].As<Napi::Number>().DoubleValue();
  GEOSGeometry *geometry = GEOSInterpolate(this->geometry, d);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::InterpolateNormalized(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  double d = info[0].As<Napi::Number>().DoubleValue();
  GEOSGeometry *geometry = GEOSInterpolateNormalized(this->geometry, d);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value Geometry::Transform(const Napi::CallbackInfo& info) {
  // TODO: check argument(s)

  Napi::Env env = info.Env();
  Napi::Function fn = info[0].As<Napi::Function>();

  GEOSGeometry *geometry = TransformGeom(env, fn, this->geometry);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}