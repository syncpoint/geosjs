const { Pool } = require('pg')
const GEOS = require('bindings')('geos')
const { WGS84 } = require('geographiclib').Geodesic
const proj4 = require('proj4')

const DEG2RAD = Math.PI / 180
const RAD2DEG = 180 / Math.PI
const HALF_PI = Math.PI / 2
const proj = proj4('EPSG:4326', 'EPSG:3857')
const radians = deg => deg * DEG2RAD

const pool = new Pool({
  database: 'scen'
})

const ST_Azimuth = (A, B) => WGS84.Inverse(A.getY(), A.getX(), B.getY(), B.getX()).azi1 * DEG2RAD
const ST_DistanceSpheroid = (A, B) => WGS84.Inverse(A.getY(), A.getX(), B.getY(), B.getX()).s12

const InverseRadians = (A, B) => {
  const r = WGS84.Inverse(A.getY(), A.getX(), B.getY(), B.getX())
  return {
    azimuth: r.azi1 * DEG2RAD,
    distanceSphere: r.s12
  }
}

const DirectRadians = (A, distance, azimuth) => {
  const r = WGS84.Direct(A.getY(), A.getX(), azimuth * RAD2DEG, distance)
  return { x: r.lon2, y: r.lat2 }
}

const ST_Project = (point, distance, azimuth) => {
  const { x, y } = DirectRadians(point, distance, azimuth)
  return GEOS.createPoint(x, y)
}

const G_G_OLAGM = (wkt, width) => {
  const halfWidth = width / 2
  const geometry = GEOS.readWKT(wkt)
  const P_A = geometry.getPointN(-2) // point before last
  const P_B = geometry.getPointN(-1) // last point
  const L_AB = GEOS.createLineString([P_A, P_B]) // last segment
  const { azimuth: ALPHA, distanceSphere: D_AB } = InverseRadians(P_A, P_B)
  const ARROW_LENGTH = 0.76 * width
  const ARROW_L_AB_RATIO = ARROW_LENGTH / D_AB
  const P_C = L_AB.interpolateNormalized(1 - ARROW_L_AB_RATIO)
  const L_BC = GEOS.createLineString([P_B, P_C])

  const BUFFER = geometry
    .transform((x, y) => proj.forward([x, y]))
    .buffer(halfWidth, 16, GEOS.CAP_FLAT, GEOS.JOIN_ROUND)
    .transform((x, y) => proj.inverse([x, y]))

  // TODO: directly create POLYGON not LINE_STRING
  const ARROW = GEOS.createLineString([
    P_B,
    ST_Project(P_C, width, ALPHA - HALF_PI),
    ST_Project(P_C, halfWidth, ALPHA - HALF_PI),
    L_BC.interpolateNormalized(0.5),
    ST_Project(P_C, halfWidth, ALPHA + HALF_PI),
    ST_Project(P_C, width, ALPHA + HALF_PI),
    P_B
  ]).asPolygon()

  // TODO: support asBoundary() for arbitrary geometries
  return GEOS.createCollection([
    BUFFER.difference(ARROW).getGeometryN(2).asBoundary(),
    ARROW.asBoundary()
  ])
}

const G_G_OLAGS = (wkt, width) => {
  const halfWidth = width / 2
  const geometry = GEOS.readWKT(wkt)
  const P_A = geometry.getPointN(-2) // point before last
  const P_B = geometry.getPointN(-1) // last point
  const L_AB = GEOS.createLineString([P_A, P_B]) // last segment
  const { azimuth: ALPHA, distanceSphere: D_AB } = InverseRadians(P_A, P_B)
  const ARROW_LENGTH = 0.76 * width
  const ARROW_L_AB_RATIO = ARROW_LENGTH / D_AB
  const P_C = L_AB.interpolateNormalized(1 - ARROW_L_AB_RATIO)

  const BUFFER = geometry
    .transform((x, y) => proj.forward([x, y]))
    .buffer(halfWidth, 16, GEOS.CAP_FLAT, GEOS.JOIN_ROUND)
    .transform((x, y) => proj.inverse([x, y]))

  const CUTOUT = GEOS.createLineString([
    ST_Project(P_B, halfWidth, ALPHA - HALF_PI),
    ST_Project(P_B, halfWidth, ALPHA + HALF_PI)
  ])
    .transform((x, y) => proj.forward([x, y]))
    .buffer(halfWidth, 16, GEOS.CAP_SQUARE, GEOS.JOIN_ROUND)
    .transform((x, y) => proj.inverse([x, y]))

  const ARROW = GEOS.createLineString([
    P_B,
    ST_Project(P_C, width, ALPHA - HALF_PI),
    ST_Project(P_C, width, ALPHA + HALF_PI),
    P_B
  ]).asPolygon()

  return BUFFER.difference(CUTOUT).union(ARROW).asBoundary()
}

const G_G_OAF = (wkt, width) => {
  const geometry = GEOS.readWKT(wkt)
  const P_A = geometry.getPointN(-2) // point before last
  const P_B = geometry.getPointN(-1) // last point
  const { azimuth: ALPHA, distanceSphere: D_AB } = InverseRadians(P_A, P_B)
  const P_A1 = ST_Project(P_A, width / 2, ALPHA - HALF_PI)
  const P_A2 = ST_Project(P_A, width / 2, ALPHA + HALF_PI)
  const P_A3 = ST_Project(P_A1, width / 3, ALPHA - radians(135))
  const P_A4 = ST_Project(P_A2, width / 3, ALPHA + radians(135))
  const P_B1 = ST_Project(P_B, width / 4, ALPHA - radians(140))
  const P_B2 = ST_Project(P_B, width / 4, ALPHA + radians(140))

  return GEOS.createCollection([
    geometry,
    GEOS.createLineString([P_A3, P_A1, P_A2, P_A4]),
    GEOS.createLineString([P_B1, P_B, P_B2])
  ])
}

const geometryFactories = {
  'G*G*OLAGM-': G_G_OLAGM,
  'G*G*OLAGS-': G_G_OLAGS,
  'G*G*OAF---': G_G_OAF
}

;(async () => {
  // WHERE  sidc IN ('G*G*OAF---')
  const result = await pool.query(`
    SELECT id, sidc, width, ST_AsText(the_geog) AS wkt
    FROM   tmp.corridor
    WHERE  sidc IN ('G*G*OLAGM-', 'G*G*OLAGS-', 'G*G*OAF---')
    ORDER  BY id
  `)

  const geometry = row => (geometryFactories[row.sidc] || (() => null))(row.wkt, row.width)
  const geometries = result.rows.map(geometry).filter(x => x !== null)
  console.log(GEOS.writeWKT(GEOS.createCollection(geometries)))

  pool.end()
})()
