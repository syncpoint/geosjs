#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include <napi.h>
#include <geos_c.h>

typedef char (*predicate_t)(const GEOSGeometry*);
typedef char (*unary_predicate_t)(const GEOSGeometry*, const GEOSGeometry*);

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
  Napi::Value GetGeometryN(const Napi::CallbackInfo& info);
  Napi::Value Difference(const Napi::CallbackInfo& info);
  Napi::Value Union(const Napi::CallbackInfo& info);
  Napi::Value Intersection(const Napi::CallbackInfo& info);
  Napi::Value ConvexHull(const Napi::CallbackInfo& info);
  Napi::Value Buffer(const Napi::CallbackInfo& info);
  Napi::Value AsPolygon(const Napi::CallbackInfo& info);
  Napi::Value AsBoundary(const Napi::CallbackInfo& info);
  Napi::Value AsValid(const Napi::CallbackInfo& info);
  Napi::Value Interpolate(const Napi::CallbackInfo& info);
  Napi::Value InterpolateNormalized(const Napi::CallbackInfo& info);
  Napi::Value Transform(const Napi::CallbackInfo& info);
  Napi::Value IsValid(const Napi::CallbackInfo& info);

  // Predicates:
  Napi::Value IsEmpty(const Napi::CallbackInfo& info);
  Napi::Value IsSimple(const Napi::CallbackInfo& info);
  Napi::Value IsRing(const Napi::CallbackInfo& info);
  Napi::Value HasZ(const Napi::CallbackInfo& info);
  Napi::Value IsClosed(const Napi::CallbackInfo& info);


  // Unary predicates:
  Napi::Value Disjoint(const Napi::CallbackInfo& info);
  Napi::Value Touches(const Napi::CallbackInfo& info);
  Napi::Value Intersects(const Napi::CallbackInfo& info);
  Napi::Value Crosses(const Napi::CallbackInfo& info);
  Napi::Value Within(const Napi::CallbackInfo& info);
  Napi::Value Contains(const Napi::CallbackInfo& info);
  Napi::Value Overlaps(const Napi::CallbackInfo& info);
  Napi::Value Equals(const Napi::CallbackInfo& info);
  Napi::Value Covers(const Napi::CallbackInfo& info);
  Napi::Value CoveredBy(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  Napi::Value PredicateTemplate(const Napi::CallbackInfo& info, predicate_t fn);
  Napi::Value UnaryPredicateTemplate(const Napi::CallbackInfo& info, unary_predicate_t fn);
};


#endif /* __GEOMETRY_H */