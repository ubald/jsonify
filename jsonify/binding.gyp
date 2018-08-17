{
  "targets": [
    {
      "target_name": "jsonify",
      "sources": [
        "src/jsonify-addon.cpp",
        "src/jsonify.cpp"
      ],
      "cflags_cc!": [
        "-fno-rtti", "-std=c++11", "-std=c++0x", "-std=gnu++11", "-std=gnu++0x"
      ],
      "cflags_cc+": [
        "-std=c++14", "-frtti"
      ],
      "xcode_settings": {
        "MACOSX_DEPLOYMENT_TARGET": "10.9",
        "OTHER_CFLAGS": [
          "-std=c++14",
          "-stdlib=libc++",
          "-frtti"
        ]
      }
    }
  ]
}
