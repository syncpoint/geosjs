{
  "targets": [
    {
      "target_name": "geos",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      },
      "sources": [
        "src/binding.cc",
        "src/geometry.cc",
        "src/transform.cc"
      ],
      "libraries": [
        "<!@(geos-config --clibs)"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!@(geos-config --includes)"
      ]
    }
  ]
}
