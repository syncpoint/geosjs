const GEOS = require('bindings')('geos')
const assert = require('assert')

const assertError = message => fn => assert.throws(fn, { message, name: "Error" })
const assertTypeError = message => fn => assert.throws(fn, { message, name: "TypeError" })

describe('Geometry', function () {
  it('::getType() - see also GEOS::readWKT()', function () {
    const geometry = GEOS.createPoint(0, 0)
    assert.strictEqual(geometry.getType(), 'Point')
  })

  it('::getNumPoints() - LineString', function () {
    const a = GEOS.createPoint(0, 0)
    const b = GEOS.createPoint(0, 0)
    const lineString = GEOS.createLineString([a, b])
    assert.strictEqual(lineString.getNumPoints(), 2)

    // -1 for geometries other than LineString.
    assert.strictEqual(a.getNumPoints(), -1)
  })

  it('::getPointN() - LineString', function () {
    const a = GEOS.createPoint(5, 5)
    const b = GEOS.createPoint(10, 10)
    const c = GEOS.createPoint(15, 15)
    const lineString = GEOS.createLineString([a, b, c])

    ;[
      [0, a], [1, b], [2, c],
      [-1, c], [-2, b], [-3, a]
    ].forEach(([i, geometry]) => assert(lineString.getPointN(i).equals(geometry)))

    assertError('Missing argument: n')(() => lineString.getPointN())
    assertTypeError('Invalid argument: n')(() => lineString.getPointN('x'))
    assertTypeError('Unexpected geometry: expected LineString')(() => a.getPointN(0))
    assertError('Invalid range: n')(() => lineString.getPointN(4))
  })

  it('::getStartPoint() - LineString', function () {
    const a = GEOS.createPoint(5, 5)
    const b = GEOS.createPoint(10, 10)
    const lineString = GEOS.createLineString([a, b])

    assert(lineString.getStartPoint().equals(a))
    assertTypeError('Unexpected geometry: expected LineString')(() => a.getStartPoint())
  })

  it('::getEndPoint() - LineString', function () {
    const a = GEOS.createPoint(5, 5)
    const b = GEOS.createPoint(10, 10)
    const lineString = GEOS.createLineString([a, b])

    assert(lineString.getEndPoint().equals(b))
    assertTypeError('Unexpected geometry: expected LineString')(() => a.getEndPoint())
  })

  it('::getX() - Point', function () {
    const x = 5
    const y = 10
    const a = GEOS.createPoint(x, y)
    assert.strictEqual(a.getX(), x)
  })

  it('::getY() - Point', function () {
    const x = 5
    const y = 10
    const a = GEOS.createPoint(x, y)
    assert.strictEqual(a.getY(), y)
  })

  it('::getNumGeometries()', function () {
    const a = GEOS.createPoint(5, 5)
    const b = GEOS.createPoint(10, 10)
    const lineString = GEOS.createLineString([a, b])
    const geometry = GEOS.createCollection([a, b, lineString])
    assert.strictEqual(geometry.getNumGeometries(), 3)
  })

  it('::getGeometryN()', function () {
    const a = GEOS.createPoint(5, 5)
    const b = GEOS.createPoint(10, 10)
    const lineString = GEOS.createLineString([a, b])
    const geometry = GEOS.createCollection([a, b, lineString])
    assert(geometry.getGeometryN(1).equals(b))

    assertError('Missing argument: n')(() => geometry.getGeometryN())
    assertTypeError('Invalid argument: n')(() => geometry.getGeometryN('x'))
    assertError('Invalid range: n')(() => geometry.getGeometryN(10))
  })

  it('::difference()', function () {
    const a = GEOS.createPoint(0, 0).buffer(10)
    const b = GEOS.createPoint(0, 5).buffer(10)
    const geometry = a.difference(b)
    assert(geometry)

    assertError('Missing argument: geometry')(() => a.difference())
    assertError('Invalid argument')(() => a.difference('x'))
  })

  it('::union()', function () {
    const a = GEOS.createPoint(0, 0).buffer(10, 16)
    const b = GEOS.createPoint(0, 5).buffer(10, 16)
    const geometry = a.union(b)
    assert(geometry)

    assertError('Missing argument: geometry')(() => a.union())
    assertError('Invalid argument')(() => a.union('x'))
  })

  it('::buffer()', function () {
    const a = GEOS.createPoint(0, 0)
    assert(a.buffer(10))
    assert(a.buffer(10, 8))
    assert(a.buffer(10, 8, GEOS.CAP_FLAT))
    assert(a.buffer(10, 8, GEOS.CAP_FLAT, GEOS.JOIN_MITRE))

    assertError('Missing argument: width')(() => a.buffer())
    assertError('Invalid argument')(() => a.union('x'))
  })

  it('::asBoundary()', function () {
    const a = GEOS.createPoint(0, 0).buffer(10).asBoundary()
    assert.strictEqual(a.getType(), 'LineString')
  })

  it('::asPolygon()', function () {
    const a = GEOS.createPoint(0, 0).buffer(10).asBoundary().asPolygon()
    assert.strictEqual(a.getType(), 'Polygon')
  })

  it('::asValid()', function () {
    // Invalid, i.e. self-intersecting polygon:
    const invalid = GEOS.readWKT('POLYGON ((0 0, 0 1, 1 0, 1 1, 0 0))')
    const valid = invalid.asValid()

    // Should now be a valid geometry with two polygons:
    assert.strictEqual(valid.isValid(), true)
    assert.strictEqual(valid.getNumGeometries(), 2)
  })

  // TODO: interpolate()
  // TODO: interpolateNormalized()
  // TODO: transform()

  it('::isValid()', function () {
    ;[
      ['POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))', true], // valid
      ['POLYGON ((0 0, 0 1, 1 0, 1 1, 0 0))', false] // invalid: self-intersecting
    ].forEach(([wkt, expected]) => assert.strictEqual(GEOS.readWKT(wkt).isValid(), expected))
  })

  // TODO: remaining predicates

  describe('Geometry::getSRID()', function () {
    it('returns 0 when not set', function () {
      const geometry = GEOS.createPoint(26.417, 59.332)
      assert.strictEqual(geometry.getSRID(), 0)
    })

    it('returns current value when set', function () {
      const geometry = GEOS.createPoint(26.417, 59.332)
      const expected = 4326
      geometry.setSRID(expected)
      assert.strictEqual(geometry.getSRID(), expected)
    })
  })

  describe('Geometry::setSRID()', function () {
    it('throws on missing argument', function () {
      const geometry = GEOS.createPoint(26.417, 59.332)
      assertError('Missing argument: SRID')(() => geometry.setSRID())
    })

    it('throws on invalid argument', function () {
      const geometry = GEOS.createPoint(26.417, 59.332)
      assertTypeError('Invalid argument: SRID')(() => geometry.setSRID('x'))
    })
  })
})