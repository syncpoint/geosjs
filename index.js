/*var geos = null;

console.log(process.platform)
console.log(process.arch)

if(process.platform == "win32" && process.arch == "x64") {
	geos = require('./bin/geos_win');  
} else if(process.platform == "linux")  {
	geos = require('./bin/geos_linux');  
} else if(process.platform == "darwin"  && process.arch == "x64")  {
	geos = require('./bin/geos_darwin');  
}
else {
	throw Error("Geosjs for architecture ${process.platform}-${process.arch} not found!")
}

module.exports = geos;



//module.exports = require('bindings')('geos')
*/

module.exports = require('node-gyp-build')(__dirname)
