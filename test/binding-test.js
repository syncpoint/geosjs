const GEOS = require('bindings')('geos')
const assert = require('assert')

const assertStringProperty = object => name => function () {
  assert(typeof object[name] === 'string')
}

const assertIntegerProperty = object => name => function () {
  assert(Number.isInteger(object[name]))
}

const assertFunctionProperty = object => name => function () {
  assert(typeof object[name] === 'function')
}

describe('GEOS', function () {
  const assertString = assertStringProperty(GEOS)
  const assertInteger = assertIntegerProperty(GEOS)
  const assertFunction = assertFunctionProperty(GEOS)

  it('exports GEOS_JTS_PORT string propery', assertString('GEOS_JTS_PORT'))
  it('exports GEOS_VERSION string propery', assertString('GEOS_VERSION'))
  it('exports GEOS_CAPI_VERSION string propery', assertString('GEOS_CAPI_VERSION'))
  it('exports GEOSBUF_CAP_ROUND integer propery', assertInteger('GEOSBUF_CAP_ROUND'))
  it('exports GEOSBUF_CAP_FLAT integer propery', assertInteger('GEOSBUF_CAP_FLAT'))
  it('exports GEOSBUF_CAP_SQUARE integer propery', assertInteger('GEOSBUF_CAP_SQUARE'))
  it('exports GEOSBUF_JOIN_ROUND integer propery', assertInteger('GEOSBUF_JOIN_ROUND'))
  it('exports GEOSBUF_JOIN_MITRE integer propery', assertInteger('GEOSBUF_JOIN_MITRE'))
  it('exports GEOSBUF_JOIN_BEVEL integer propery', assertInteger('GEOSBUF_JOIN_BEVEL'))
  it('exports readWKT function propery', assertFunction('readWKT'))
  it('exports writeWKT function propery', assertFunction('writeWKT'))
  it('exports createLineString function propery', assertFunction('createLineString'))
  it('exports createPoint function propery', assertFunction('createPoint'))
  it('exports createCollection function propery', assertFunction('createCollection'))

  // https://www.ogc.org/standards/sfa

  const wkt = [
    'POINT (10 10)',
    'LINESTRING (10 10, 20 20, 30 40)',
    'POLYGON ((10 10, 10 20, 20 20, 20 15, 10 10))',
    'POLYGON ((0 0, 0 20, 20 20, 20 0, 0 0), (5 5, 5 15, 15 15, 15 5, 5 5))',
    'MULTIPOINT (0 0, 20 20, 60 60)',
    'MULTILINESTRING ((10 10, 20 20, 30 40), (15 15, 25 25, 35 45))',
    'MULTIPOLYGON (((0 0, 0 20, 20 20, 20 0, 0 0), (5 5, 5 15, 15 15, 15 5, 5 5)), ((30 30, 30 40, 40 40, 40 30, 30 30)))',
    'GEOMETRYCOLLECTION (POINT (10 10), LINESTRING (10 10, 20 20, 30 40))'
  ]

  const types = [
    'Point',
    'LineString',
    'Polygon',
    'Polygon',
    'MultiPoint',
    'MultiLineString',
    'MultiPolygon',
    'GeometryCollection'
  ]

  describe('GEOS::readWKT()', function () {
    it('translates any WKT into geometry', function () {
      const actual = wkt.map(GEOS.readWKT).map(geometry => geometry.getType())

      // TODO: Compare expected/actual WKT once (fixed) precision model is supported.
      assert.deepEqual(actual, types)
    })

    it('throws on missing argument', function () {
      assert.throws(() => GEOS.readWKT(), {
        name: "Error",
        message: "Missing argument: WKT"
      })
    })

    it('throws on invalid argument', function () {
      assert.throws(() => GEOS.readWKT(42), {
        name: "TypeError",
        message: "Invalid argument: WKT"
      })
    })

    it('throws on WKT parse error', function () {
      assert.throws(() => GEOS.readWKT('GARBAGE'), {
        name: "Error",
        message: "ParseException: Unknown type: 'GARBAGE'"
      })
    })
  })


  describe('GEOS::writeWKT()', function () {
    it('translates any geometry into WKT', function() {
      const actual = wkt
        .map(GEOS.readWKT)
        .map(geometry => GEOS.writeWKT(geometry))
        .filter(wkt => typeof wkt === 'string')
      assert.strictEqual(actual.length, wkt.length)
    })

    it('throws on missing argument', function () {
      assert.throws(() => GEOS.writeWKT(), {
        name: "Error",
        message: "Missing argument: Geometry"
      })
    })

    it('throws on invalid argument (Number)', function () {
      assert.throws(() => GEOS.writeWKT(42), {
        name: "TypeError",
        message: "Invalid argument: Geometry"
      })
    })

    it('throws on invalid argument (non-Geometry object)', function () {
      // NOTE: thrown by ObjectWrap<Geometry>::Unwrap(...)
      assert.throws(() => GEOS.writeWKT({}), {
        name: "Error",
        message: "Invalid argument"
      })
    })
  })


  describe('GEOS::createLineString()', function () {
    it('constructs LineString from [Point]', function() {
      const a = GEOS.createPoint(0, 0)
      const b = GEOS.createPoint(10, 10)
      const c = GEOS.createPoint(15, 10)
      const lineString = GEOS.createLineString([a, b, c])
      assert(lineString)
      assert.strictEqual(lineString.getType(), 'LineString')
      assert.strictEqual(lineString.getNumPoints(), 3)
    })

    it('throws on missing argument', function () {
      assert.throws(() => GEOS.createLineString(), {
        name: "Error",
        message: "Missing argument: [Point]"
      })
    })

    it('throws on invalid argument ([Point][0])', function () {
      assert.throws(() => GEOS.createLineString([]), {
        name: "Error",
        message: "Invalid argument: [Point]"
      })
    })

    it('throws on invalid argument ([Point][1])', function () {
      assert.throws(() => GEOS.createLineString([GEOS.createPoint(0, 0)]), {
        name: "Error",
        message: "Invalid argument: [Point]"
      })
    })

    it('throws on invalid argument ([non-Point])/1', function () {
      assert.throws(() => GEOS.createLineString(['x', 'y']), {
        name: "Error",
        message: "Invalid argument"
      })
    })

    it('throws on invalid argument ([non-Point])/2', function () {
      const a = GEOS.createPoint(0, 0)
      const b = GEOS.createPoint(10, 10)
      const ab = GEOS.createLineString([a, b])
      assert.throws(() => GEOS.createLineString([a, ab /* non-Point geometry */]), {
        name: "TypeError",
        message: "Invalid argument: [Point]"
      })
    })

    it('throws on invalid argument ([non-Point])/3', function () {
      const a = GEOS.createPoint(0, 0)
      assert.throws(() => GEOS.createLineString([a, {} /* non-Point geometry */]), {
        name: "Error",
        message: "Invalid argument"
      })
    })

    it('throws on invalid argument non-array', function () {
      const a = GEOS.createPoint(0, 0)
      const b = GEOS.createPoint(10, 10)
      assert.throws(() => GEOS.createLineString(a, b), {
        name: "Error",
        message: "An array was expected"
      })
    })
  })


  describe('GEOS::createPoint()', function () {
    it('constructs Point from x, y', function() {
      const point = GEOS.createPoint(0, 0)
      assert(point)
      assert.strictEqual(point.getType(), 'Point')
    })

    it('throws on missing argument/1', function () {
      assert.throws(() => GEOS.createPoint(), {
        name: "Error",
        message: "Missing argument(s): x, y"
      })
    })

    it('throws on missing argument/2', function () {
      assert.throws(() => GEOS.createPoint(0), {
        name: "Error",
        message: "Missing argument(s): x, y"
      })
    })

    it('throws on invalid argument (x)', function () {
      assert.throws(() => GEOS.createPoint('a', 0), {
        name: "TypeError",
        message: "Invalid argument: x"
      })
    })

    it('throws on invalid argument (y)', function () {
      assert.throws(() => GEOS.createPoint(0, 'a'), {
        name: "TypeError",
        message: "Invalid argument: y"
      })
    })
  })


  describe('GEOS::createCollection()', function () {
    it('constructs Collection from [Geometry]', function() {
      const a = GEOS.createPoint(0, 0)
      const b = GEOS.createPoint(10, 10)
      const lineString = GEOS.createLineString([a, b])

      const collection = GEOS.createCollection([a, b, lineString])
      assert.strictEqual(collection.getType(), 'GeometryCollection')
      assert.strictEqual(collection.getNumGeometries(), 3)
    })

    it('throws on missing argument', function () {
      assert.throws(() => GEOS.createCollection(), {
        name: "Error",
        message: "Missing argument: [Geometry]"
      })
    })

    it('throws on invalid argument (non-array)', function () {
      assert.throws(() => GEOS.createCollection(GEOS.createPoint(0, 0)), {
        name: "Error",
        message: "An array was expected"
      })
    })

    it('throws on invalid argument (non-geometry)', function () {
      assert.throws(() => GEOS.createCollection(['x']), {
        name: "Error",
        message: "Invalid argument"
      })
    })
  })
})
