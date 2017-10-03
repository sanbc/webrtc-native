{
'includes': [
    './third_party/webrtc/src/build/common.gypi',
    './build/config.gypi',
    './nodejs.gypi',
    './src/addon.gypi',
  ],
'targets': [
    {
      'target_name': 'webrtc',
      'sources': [
        'src/Platform.cc',
        'src/Global.cc',
        'src/BackTrace.cc',
        'src/Configuration.cc',
        'src/EventEmitter.cc',
        'src/Observers.cc',
        'src/Module.cc',
        'src/PeerConnection.cc',
        'src/DataChannel.cc',
        'src/MediaStream.cc',
        'src/MediaStreamTrack.cc',
        'src/Stats.cc',
        'src/GetUserMedia.cc',
        'src/MediaConstraints.cc'
      ],

      'cflags': [
        '-pthread',
        '-march=x86-64',
        '-fno-exceptions',
        '-fno-strict-aliasing',
        '-Wall',
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
        '-Wextra',
        '-Wno-unused-local-typedefs',
        '-Wno-uninitialized',
        '-Wno-unused-variable',
        '-Wno-unused-but-set-variable',
        '-pipe',
        '-fno-ident',
        '-fdata-sections',
        '-ffunction-sections',
        '-fPIC',
        '-fpermissive',
        '-std=gnu++11',
        '-D_GLIBCXX_USE_CXX11_ABI=0',
        '-D_GLIBCXX_DEBUG=1',
        '-D_LARGEFILE_SOURCE',
        '-D_LARGEFILE64_SOURCE',
        '-D_DEBUG -DDYNAMIC_ANNOTATIONS_ENABLED -DWTF_USE_DYNAMIC_ANNOTATIONS=1',
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'OTHER_CFLAGS': [
          '-std=gnu++0x',
          '-Wno-c++0x-extensions',
          '-Wno-c++11-extensions',
          '-stdlib=libc++',
        ],
        'OTHER_LDFLAGS': [
          '-stdlib=libc++',
        ]
      },
      'defines': [
#        'TRACING',
#        'LARGEFILE_SOURCE',
#        '_FILE_OFFSET_BITS=64',
        'WEBRTC_THREAD_RR',
        'EXPAT_RELATIVE_PATH',
        'GTEST_RELATIVE_PATH',
        'JSONCPP_RELATIVE_PATH',

        'rtc_base_config',
        'WEBRTC_RELATIVE_PATH',
#        '__STDC_FORMAT_MACROS',
#        'DYNAMIC_ANNOTATIONS_ENABLED=0',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            '_GLIBCXX_USE_CXX11_ABI=0',
            'WEBRTC_LINUX',
            'WEBRTC_POSIX=1',
          ],
        }],
        ['OS=="mac"', {
          'defines': [
            'WEBRTC_MAC',
            'WEBRTC_IOS',
            'WEBRTC_POSIX=1',
          ],
        }],
        ['OS=="win"', {
          'defines': [
            'WEBRTC_WIN',
            'NOGDI',
            'NOMINMAX',
          ],
        }],
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        './third_party/webrtc/src',
#'<(DEPTH)/third_party/libsrtp/srtp',
#        '<(DEPTH)/third_party/libyuv/include',
'./third_party/webrtc/src/third_party/jsoncpp/source/include',
#'./third_party/webrtc/src/build/linux/debian_jessie_i386-sysroot/usr/include/jsoncpp',
      ],
      'link_settings': {
        'conditions': [
          ['OS=="mac"', {
            'libraries': [
              '-framework AppKit',
            ],
          }],
          ['OS=="win"', {
            'libraries': [
              '../third_party/webrtc/lib/libwebrtc.lib',
              'dmoguids.lib',
              'msdmo.lib',
              'secur32.lib',
              'winmm.lib',
              'wmcodecdspuuid.lib',
              'ws2_32.lib',
            ],
          }, {
            'libraries': [
             # '../third_party/webrtc/lib/libwebrtc.a',
            '../third_party/webrtc/src/out/Release/obj/webrtc/libwebrtc.a',
             # '../third_party/webrtc/src/out/Release/obj/webrtc/pc/librtc_pc.a',
              #'../third_party/webrtc/src/build/linux/debian_jessie_i386-sysroot/usr/lib/libjsoncpp.a',
            ],
          }],
        ],
      },
      
          
          
    },

  ]
}
