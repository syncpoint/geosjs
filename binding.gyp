{
   "targets": [],
   "conditions": [
      [
         "OS==\"linux\"",
         {
            "targets": [
               {
                  "target_name": "geos_linux",
                  "cflags!": [
                     "-fno-exceptions"
                  ],
                  "cflags_cc!": [
                     "-fno-exceptions"
                  ],
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
               },
               {
                  "target_name": "copy_job",
                  "dependencies": [
                     "geos_linux"
                  ],
                  "copies": [
                     {
                        "files": [
                           "<(PRODUCT_DIR)/geos_linux.node"
                        ],
                        "destination": "<(PRODUCT_DIR)/../../bin"
                     }
                  ]
               }
            ]
         },
         "OS==\"mac\"",
         {
            "targets": [
               {
                  "target_name": "geos_darwin",
                  "cflags!": [
                     "-fno-exceptions"
                  ],
                  "cflags_cc!": [
                     "-fno-exceptions"
                  ],
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
               },
               {
                  "target_name": "copy_job",
                  "dependencies": [
                     "geos_darwin"
                  ],
                  "copies": [
                     {
                        "files": [
                           "<(PRODUCT_DIR)/geos_darwin.node"
                        ],
                        "destination": "<(PRODUCT_DIR)/../../bin"
                     }
                  ]
               }
            ]
         },
         "OS==\"win\"",
         {
            "target_defaults": {
               "configurations": {
                 "Debug": {
                   "msvs_settings": {
                     "VCCLCompilerTool": {
                       # 0 - MultiThreaded (/MT)
                       # 1 - MultiThreadedDebug (/MTd)
                       # 2 - MultiThreadedDLL (/MD)
                       # 3 - MultiThreadedDebugDLL (/MDd)
                       "RuntimeLibrary": 0
                     }
                   }
                 },
                 "Release": {
                   "msvs_settings": {
                     "VCCLCompilerTool": {
                       "RuntimeLibrary": 0,  # shared release
                     }
                   }
                 }
               }
             },

            "targets": [
               {
                  "target_name": "geos_win",
                  "cflags!": [
                     "-fno-exceptions"
                  ],
                  "cflags_cc!": [
                     "-fno-exceptions"
                  ],
                  "sources": [
                     "src/binding.cc",
                     "src/geometry.cc",
                     "src/transform.cc"
                  ],
                  "libraries": [
                     "C:\syncpoint\geos-3.8.1\_build_vs2019x64\lib\Release\geos_c.lib",
                     "C:\syncpoint\geos-3.8.1\_build_vs2019x64\lib\Release\geos.lib"
                  ],
                  "include_dirs": [
                     "<!@(node -p \"require('node-addon-api').include\")",
                     "C:\syncpoint\geos-3.8.1\_build_vs2019x64\capi",
                     "C:\syncpoint\geos-3.8.1\include"
                  ],
                  "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS "
                  ]
               }
            ]
         }
      ]
   ]
}
