#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <napi.h>
#include <geos_c.h>

GEOSGeometry* TransformGeom(Napi::Env env, Napi::Function fn, const GEOSGeometry* g1);

#endif /* __TRANSFORM_H */