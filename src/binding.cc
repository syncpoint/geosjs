#include <napi.h>
#include <geos_c.h>
#include "geometry.h"

void notice_fn(const char *fmt, ...) {
	va_list ap;
  fprintf(stdout, "NOTICE: ");

	va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n" );
}

void error_fn(const char *fmt, ...) {
	va_list ap;
  fprintf(stdout, "ERROR: ");

	va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n");
	exit(1);
}

GEOSWKTReader *reader;
GEOSWKTWriter *writer;

Napi::Value ReadWKT(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "wkt undefined ").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string wkt = info[0].As<Napi::String>().Utf8Value();
  GEOSGeometry *geometry = GEOSWKTReader_read(reader, wkt.c_str());
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);

  return Geometry::NewInstance(env, external);
}

Napi::Value WriteWKT(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "geometry undefined").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Geometry* geometry = Napi::ObjectWrap<Geometry>::Unwrap(info[0].As<Napi::Object>());
  char *wkt = GEOSWKTWriter_write(writer, geometry->geometry);
  Napi::Value value = Napi::String::New(env, wkt);
  GEOSFree(wkt);
  return value;
}

Napi::Value CreateLineString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  // TODO: check arguments

  unsigned int size = info[0].As<Napi::Array>().Length();
  Napi::Object coords = info[0].ToObject();
  GEOSCoordSequence* coordSeq = GEOSCoordSeq_create(size, 2);

  double x;
  double y;

  for(uint32_t i = 0; i < size; i++) {
    // TODO: check for point geometry
    Napi::Value value = coords[i];
    Geometry* point = Napi::ObjectWrap<Geometry>::Unwrap(value.As<Napi::Object>());
    GEOSGeomGetX(point->geometry, &x);
    GEOSGeomGetY(point->geometry, &y);
    GEOSCoordSeq_setXY(coordSeq, i, x, y);
  }

	GEOSGeometry *geometry = GEOSGeom_createLineString(coordSeq);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value CreatePoint(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  // TODO: check arguments

  double x = info[0].As<Napi::Number>().DoubleValue();
  double y = info[1].As<Napi::Number>().DoubleValue();
  GEOSGeometry *geometry = GEOSGeom_createPointFromXY(x, y);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Value CreateCollection(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  // TODO: check arguments

  int ngeoms = info[0].As<Napi::Array>().Length();
  Napi::Object input = info[0].ToObject();

  GEOSGeometry** geoms = (GEOSGeometry**)malloc(ngeoms * sizeof(GEOSGeometry*));
  for(int i = 0; i < ngeoms; i++) {
    Napi::Value value = input[i];
    Geometry* geom = Napi::ObjectWrap<Geometry>::Unwrap(value.As<Napi::Object>());

    // Collection becomes owner of geometries, thus clone input geometries:
    geoms[i] = GEOSGeom_clone(geom->geometry);
  }

  GEOSGeometry* geometry = GEOSGeom_createCollection(
    GEOS_GEOMETRYCOLLECTION,
    geoms,
    ngeoms
  );

  free(geoms);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  initGEOS(notice_fn, error_fn);

  Geometry::Init(env, exports);

  exports.Set("GEOS_JTS_PORT", Napi::String::New(env, GEOS_JTS_PORT));
  exports.Set("GEOS_VERSION", Napi::String::New(env, GEOS_VERSION));
  exports.Set("GEOS_CAPI_VERSION", Napi::String::New(env, GEOS_CAPI_VERSION));

  exports.Set("GEOSBUF_CAP_ROUND", Napi::Number::New(env, GEOSBUF_CAP_ROUND));
  exports.Set("GEOSBUF_CAP_FLAT", Napi::Number::New(env, GEOSBUF_CAP_FLAT));
  exports.Set("GEOSBUF_CAP_SQUARE", Napi::Number::New(env, GEOSBUF_CAP_SQUARE));
  exports.Set("GEOSBUF_JOIN_ROUND", Napi::Number::New(env, GEOSBUF_JOIN_ROUND));
  exports.Set("GEOSBUF_JOIN_MITRE", Napi::Number::New(env, GEOSBUF_JOIN_MITRE));
  exports.Set("GEOSBUF_JOIN_BEVEL", Napi::Number::New(env, GEOSBUF_JOIN_BEVEL));

  exports.Set("readWKT", Napi::Function::New(env, ReadWKT));
  exports.Set("writeWKT", Napi::Function::New(env, WriteWKT));
  exports.Set("createLineString", Napi::Function::New(env, CreateLineString));
  exports.Set("createPoint", Napi::Function::New(env, CreatePoint));
  exports.Set("createCollection", Napi::Function::New(env, CreateCollection));

  reader = GEOSWKTReader_create();
  writer = GEOSWKTWriter_create();
  return exports;
}

// Self-register module 'geos':
NODE_API_MODULE(geos, Init)