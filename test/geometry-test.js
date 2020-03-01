const GEOS = require('bindings')('geos')
const assert = require('assert')

describe('Geometry', function () {
  it("has 'type' function; see also GEOS::readWKT()", function () {
    const geometry = GEOS.createPoint(0, 0)
    assert.strictEqual(geometry.getType(), 'Point')
  })

  it("has 'numPoints' function", function () {
    const a = GEOS.createPoint(0, 0)
    const b = GEOS.createPoint(0, 0)
    const lineString = GEOS.createLineString([a, b])
    assert.strictEqual(lineString.getNumPoints(), 2)

    // -1 for geometries other than LineString.
    assert.strictEqual(a.getNumPoints(), -1)
  })

  it.only("has 'pointN' function", function () {
    const a = GEOS.createPoint(0, 0)
    const b = GEOS.createPoint(10, 10)
    const c = GEOS.createPoint(15, 15)

    const lineString = GEOS.createLineString([a, b, c])
    lineString.getPointN(0).equals(a)
    console.log(lineString.getPointN(0))
    console.log(lineString.getPointN(1))
    console.log(lineString.getPointN(2))
    console.log(lineString.getPointN(-1))
    console.log(lineString.getPointN(-2))
    console.log(lineString.getPointN(-3))
  })

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
      assert.throws(() => geometry.setSRID(), {
        name: "Error",
        message: "Missing argument: SRID"
      })
    })

    it('throws on invalid argument', function () {
      const geometry = GEOS.createPoint(26.417, 59.332)
      assert.throws(() => geometry.setSRID('x'), {
        name: "TypeError",
        message: "Invalid argument: SRID"
      })
    })
  })
})