#include "transform.h"

GEOSCoordSequence* TransformCoordSeq(
  Napi::Env env,
  Napi::Function fn,
  const GEOSCoordSequence* cs
)
{
	GEOSCoordSequence* coordSeq = GEOSCoordSeq_clone(cs);

  unsigned int size; // number of points, e.g. 75 points
  unsigned int dims; // dimension of one point, e.g. 2 dimensions, X and Y

  GEOSCoordSeq_getSize(coordSeq, &size);
  GEOSCoordSeq_getDimensions(coordSeq, &dims);

  double x;
  double y;
  for (unsigned int i = 0; i < size; i++) {
    GEOSCoordSeq_getXY(coordSeq, i, &x, &y);
    Napi::Object result = fn.Call(env.Global(), {
      Napi::Number::New(env, x),
      Napi::Number::New(env, y)
    }).ToObject();

    Napi::Value vx = result[(uint32_t)0];
    Napi::Value vy = result[(uint32_t)1];
    GEOSCoordSeq_setXY(
      coordSeq,
      i,
      vx.As<Napi::Number>().DoubleValue(),
      vy.As<Napi::Number>().DoubleValue()
    );
  }

  return coordSeq;
}

GEOSGeometry* TransformGeom(Napi::Env env, Napi::Function fn, const GEOSGeometry* g1) {
	GEOSCoordSequence* cs;
	GEOSGeometry* g2;
	GEOSGeometry* shell;
	const GEOSGeometry* gtmp;
	GEOSGeometry **geoms;
	unsigned int ngeoms, i;
	int type;

	/* Geometry reconstruction from CoordSeq */
	type = GEOSGeomTypeId(g1);

	switch ( type )
	{
		case GEOS_POINT:
			cs = TransformCoordSeq(env, fn, GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createPoint(cs);
			return g2;
			break;
		case GEOS_LINESTRING:
			cs = TransformCoordSeq(env, fn, GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createLineString(cs);
			return g2;
			break;
		case GEOS_LINEARRING:
			cs = TransformCoordSeq(env, fn, GEOSGeom_getCoordSeq(g1));
			g2 = GEOSGeom_createLinearRing(cs);
			return g2;
			break;
		case GEOS_POLYGON:
			gtmp = GEOSGetExteriorRing(g1);
			cs = TransformCoordSeq(env, fn, GEOSGeom_getCoordSeq(gtmp));
			shell = GEOSGeom_createLinearRing(cs);
			ngeoms = GEOSGetNumInteriorRings(g1);
			geoms = (GEOSGeometry**)malloc(ngeoms * sizeof(GEOSGeometry*));
			for (i = 0; i < ngeoms; i++) {
				gtmp = GEOSGetInteriorRingN(g1, i);
				cs = TransformCoordSeq(env, fn, GEOSGeom_getCoordSeq(gtmp));
				geoms[i] = GEOSGeom_createLinearRing(cs);
			}
			g2 = GEOSGeom_createPolygon(shell, geoms, ngeoms);
			free(geoms);
			return g2;
			break;
		case GEOS_MULTIPOINT:
		case GEOS_MULTILINESTRING:
		case GEOS_MULTIPOLYGON:
		case GEOS_GEOMETRYCOLLECTION:
			ngeoms = GEOSGetNumGeometries(g1);
			geoms = (GEOSGeometry**)malloc(ngeoms*sizeof(GEOSGeometry*));
			for (i = 0; i < ngeoms; i++) {
				gtmp = GEOSGetGeometryN(g1, i);
				geoms[i] = TransformGeom(env, fn, gtmp);
			}
			g2 = GEOSGeom_createCollection(type, geoms, ngeoms);
			free(geoms);
			return g2;
			break;
		default:
			return NULL;
	}
}