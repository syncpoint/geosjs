#include <napi.h>
#include <geos_c.h>
#include "geometry.h"

#ifdef __GNUC__
 #include <cstdarg>
#endif


char last_error[256];

char* get_last_error() {
  return last_error;
}

void notice_fn(const char *fmt, ...) {
	va_list ap;
  fprintf(stdout, "NOTICE: ");

	va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n" );
}

void error_fn(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
  vsnprintf(last_error, 255, fmt, args);
  va_end(args);
}

GEOSWKTReader *reader;
GEOSWKTWriter *writer;


/**
 * Parse a WKT string returning a Geometry.
 * info[0] : String - WKT
 */
Napi::Value ReadWKT(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::Error::New(env, "Missing argument: WKT").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Invalid argument: WKT").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string wkt = info[0].As<Napi::String>().Utf8Value();
  GEOSGeometry *geometry = GEOSWKTReader_read(reader, wkt.c_str());

  if (geometry == NULL) {
    Napi::Error::New(env, get_last_error()).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}


/**
 * Returns WKT string for the given Geometry.
 * info[0] : Geometry
 */
Napi::Value WriteWKT(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::Error::New(env, "Missing argument: Geometry").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsObject()) {
    Napi::TypeError::New(env, "Invalid argument: Geometry").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // Throws "Error: Invalid argument" if not a Geometry object:
  Geometry* geometry = Napi::ObjectWrap<Geometry>::Unwrap(info[0].As<Napi::Object>());
  char *wkt = GEOSWKTWriter_write(writer, geometry->geometry);
  Napi::Value value = Napi::String::New(env, wkt);
  GEOSFree(wkt);

  return value;
}


/**
 * Construct a LineString from Point array.
 * info[0] : [Point]
 */
Napi::Value CreateLineString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::Error::New(env, "Missing argument: [Point]").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  unsigned int size = info[0].As<Napi::Array>().Length();

  if (size < 2) {
    Napi::Error::New(env, "Invalid argument: [Point]").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object points = info[0].ToObject();

  // Check if supplied geometries are points:
  for(uint32_t i = 0; i < size; i++) {
    Napi::Value value = points[i];
    Geometry* geometry = Napi::ObjectWrap<Geometry>::Unwrap(value.As<Napi::Object>());
    if (GEOSGeomTypeId(geometry->geometry) != GEOS_POINT) {
      Napi::TypeError::New(env, "Invalid argument: [Point]").ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  GEOSCoordSequence* coordSeq = GEOSCoordSeq_create(size, 2);

  double x;
  double y;

  for(uint32_t i = 0; i < size; i++) {
    Napi::Value value = points[i];
    Geometry* point = Napi::ObjectWrap<Geometry>::Unwrap(value.As<Napi::Object>());
    GEOSGeomGetX(point->geometry, &x);
    GEOSGeomGetY(point->geometry, &y);
    GEOSCoordSeq_setXY(coordSeq, i, x, y);
  }

  // coordSeq is now owned by new geometry:
	GEOSGeometry *geometry = GEOSGeom_createLineString(coordSeq);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}


/**
 * Construct a Point from x, y.
 * info[0] : double - x
 * info[1] : double - y
 */
Napi::Value CreatePoint(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::Error::New(env, "Missing argument(s): x, y").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Invalid argument: x").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Invalid argument: y").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  double x = info[0].As<Napi::Number>().DoubleValue();
  double y = info[1].As<Napi::Number>().DoubleValue();
  GEOSGeometry *geometry = GEOSGeom_createPointFromXY(x, y);
  Napi::External<GEOSGeometry> external = Napi::External<GEOSGeometry>::New(env, geometry);
  return Geometry::NewInstance(env, external);
}


/**
 * Construct a geometry collection from geometry array.
 * info[0] : [Geometry]
 */
Napi::Value CreateCollection(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::Error::New(env, "Missing argument: [Geometry]").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int ngeoms = info[0].As<Napi::Array>().Length();
  Napi::Object input = info[0].ToObject();

  // Check if we are dealing with geometries only:
  for(int i = 0; i < ngeoms; i++) {
    Napi::Value value = input[i];
    // Throws "Error: Invalid argument" on non-Geometry object value:
    Napi::ObjectWrap<Geometry>::Unwrap(value.As<Napi::Object>());
  }

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

/**
 *
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  initGEOS(notice_fn, error_fn);

  Geometry::Init(env, exports);

  exports.Set("GEOS_JTS_PORT", Napi::String::New(env, GEOS_JTS_PORT));
  exports.Set("GEOS_VERSION", Napi::String::New(env, GEOS_VERSION));
  exports.Set("GEOS_CAPI_VERSION", Napi::String::New(env, GEOS_CAPI_VERSION));

  exports.Set("CAP_ROUND", Napi::Number::New(env, GEOSBUF_CAP_ROUND));
  exports.Set("CAP_FLAT",  Napi::Number::New(env, GEOSBUF_CAP_FLAT));
  exports.Set("CAP_SQUARE", Napi::Number::New(env, GEOSBUF_CAP_SQUARE));
  exports.Set("JOIN_ROUND", Napi::Number::New(env, GEOSBUF_JOIN_ROUND));
  exports.Set("JOIN_MITRE", Napi::Number::New(env, GEOSBUF_JOIN_MITRE));
  exports.Set("JOIN_BEVEL", Napi::Number::New(env, GEOSBUF_JOIN_BEVEL));

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
