var fs = require('fs');
var os = require('os');
var spawn = require('child_process').spawn;
var path = require('path');
var request = require('request');

var ROOT = path.resolve(__dirname, '..');

if (!fs.existsSync(ROOT + path.sep + 'build' + path.sep + 'config.gypi')) {
  throw new Error('Run node-gyp rebuild instead of node build.js');
}

var PACKAGE = require(path.resolve(ROOT, 'package.json'));

var CHROMIUM = 'https://chromium.googlesource.com/external/webrtc.git@ca4ec2c3b9331e8f054facf400ebaeb9681831e2';
var CHROMIUM = 'https://chromium.googlesource.com/external/webrtc.git@571f2a6e17b999949d77cc7d51369efd2c2d606c';
var CHROMIUM = 'https://chromium.googlesource.com/external/webrtc.git@6294a7eb71c891e9ea41273a7a94113f6802d0da';
var USE_OPENSSL = false;
var USE_GTK = false;
var USE_X11 = false;

var PLATFORM = os.platform();
var SYSTEM = os.release();
var ARCH = process.argv[2].substring(14);
var NODEJS = path.resolve(process.argv[3]);
var NODELIB = process.argv[4].substring(3).split('.')[0];
var NODEGYP = process.argv[5];
var URL = 'http://cide.cc:8080/webrtc/';
var NODEVER = process.version.split('.');
var NODE_ZERO = (NODEVER[0] === 'v0');
var CROSSCOMPILE = (ARCH !== process.arch);
console.log("======",process.argv)
NODEVER[2] = 'x';
NODEVER = NODEVER.join('.');

URL += 'webrtc-' + PACKAGE.version + '-' + PLATFORM + '-' + ARCH + '-' + NODEVER + '.node';
  
if (fs.existsSync(ROOT + path.sep + 'nodejs.gypi')) {
  fs.unlinkSync(ROOT + path.sep + 'nodejs.gypi');
}

var COMMON = path.resolve(NODEJS, 'include', 'node', 'common.gypi');

if (fs.existsSync(COMMON)) {
  fs.linkSync(COMMON, ROOT + path.sep + 'nodejs.gypi');
} else {
  fs.linkSync(NODEJS + path.sep + 'common.gypi', ROOT + path.sep + 'nodejs.gypi');
}

var CONFIG = 'Release';

if (fs.existsSync(ROOT + path.sep + 'build' + path.sep + 'Debug')) {
  CONFIG = 'Debug';
}

var THIRD_PARTY = path.resolve(ROOT, 'third_party');
var DEPOT_TOOLS_REPO = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git';
var DEPOT_TOOLS = path.resolve(THIRD_PARTY, 'depot_tools');
var WEBRTC = path.resolve(THIRD_PARTY, 'webrtc');
var WEBRTC_SRC = path.resolve(WEBRTC, 'src');
var WEBRTC_OUT = path.resolve(WEBRTC_SRC, 'out', CONFIG);
var FETCH = path.resolve(DEPOT_TOOLS, (os.platform() == 'win32') ? 'fetch.bat' : 'fetch');
var GCLIENT = path.resolve(DEPOT_TOOLS, (os.platform() == 'win32') ? 'gclient.bat' : 'gclient');

if (os.platform() == 'win32' && ARCH == 'x64') {
  WEBRTC_OUT = path.resolve(WEBRTC_SRC, 'out', CONFIG + '_x64');
}

function install() {
  fs.linkSync(WEBRTC_OUT + path.sep + 'webrtc.node', path.resolve(ROOT, 'build', CONFIG, 'webrtc.node'));
  
  if (process.env['CIDE_CREDENTIALS']) {
    console.log('Uploading module.');
    
    var credentials = {
      'auth': {
        'user': 'cIDE',
        'pass': process.env['CIDE_CREDENTIALS'],
      }
    };
    
    fs.createReadStream(path.resolve(ROOT, 'build', CONFIG, 'webrtc.node')).pipe(request.put(URL, credentials, function(error, response, body) {
      if (!error && response.statusCode == 200) {
        console.log('Done! :)');
      } else {
        console.log('Upload Failed! :(');
      }
    }));
  } else {
    setTimeout(function() {
      console.log('Done! :)');
    }, 200);
  } 
}

function compile() {
console.log("process.env",  process.env['GYP_DEFINES']);
  var res = spawn('ninja', [ '-C', WEBRTC_OUT ], {
    cwd: WEBRTC_SRC,
    env: process.env,
    stdio: 'inherit',
  });

  res.on('close', function (code) {
    if (!code) {
//      return install();
    }

 //   process.exit(1);
  });
}
function build() {
console.log("build");
//var res = spawn('python', [ WEBRTC_SRC + path.sep + 'webrtc' + path.sep + 'build' + path.sep + 'gyp_webrtc', 'src' + path.sep + 'webrtc.gyp'
// gn gen out/Release
    var res = spawn("gn", ['gen', 'out/Release'], {
     cwd: WEBRTC_SRC,
     env: process.env,
     stdio: 'inherit',
   });

    res.on('close', function (code) {

      if (!code) {
        return compile();
      }

      process.exit(1);
    });
}

function sync() {
 if (!fs.existsSync(THIRD_PARTY + path.sep + 'webrtc_sync')) {
    var res = spawn(GCLIENT, ['sync', '--with_branch_heads'], {
      cwd: WEBRTC,
      env: process.env,
      stdio: 'inherit',
    });

    res.on('close', function (code) {
      if (!code) {
        fs.closeSync(fs.openSync(THIRD_PARTY + path.sep + 'webrtc_sync', 'w'));
        return build();
      }

      process.exit(1);
    });
  } else {
    build();
  } 
 //fs.closeSync(fs.openSync(THIRD_PARTY + path.sep + 'webrtc_sync', 'w'));
//build();
}

function configure() {
  if (fs.existsSync(WEBRTC_OUT + path.sep + 'webrtc.node')) {
    fs.unlinkSync(WEBRTC_OUT + path.sep + 'webrtc.node');
  }
  console.log("CONFIGURE");
  process.env['GYP_DEFINES'] += ' target_arch=' + ARCH;
  process.env['GYP_DEFINES'] += ' host_arch=' + process.arch;
  process.env['GYP_DEFINES'] += ' node_root_dir=' + NODEJS.replace(/\\/g, '\\\\');
  process.env['GYP_DEFINES'] += ' node_lib_file=' + NODELIB;
  process.env['GYP_DEFINES'] += ' node_gyp_dir=' + NODEGYP.replace(/\\/g, '\\\\');
  process.env['GYP_DEFINES'] += ' build_with_chromium=0';
  process.env['GYP_DEFINES'] += ' use_openssl=' + ((USE_OPENSSL) ? '1' : '0');
  process.env['GYP_DEFINES'] += ' use_gtk='+ ((USE_GTK) ? '1' : '0');
  process.env['GYP_DEFINES'] += ' use_x11=' + ((USE_X11) ? '1' : '0');
  process.env['GYP_DEFINES'] += ' ConfigurationName=' + CONFIG;  
  process.env['GYP_DEFINES'] += ' include_tests=0';
  
  switch (os.platform()) {
    case 'darwin':
      process.env['GYP_DEFINES'] += ' clang=1';

      break;
    case 'win32':
      process.env['DEPOT_TOOLS_WIN_TOOLCHAIN'] = 0;
      process.env['GYP_MSVS_VERSION'] = 2013;
      
      break;
    case 'linux':
      if (CROSSCOMPILE) {
        process.env['GYP_CROSSCOMPILE'] = 1;
        process.env['GYP_DEFINES'] += ' clang=0 use_system_expat=0';
        process.env['CXX'] = 'arm-linux-gnueabihf-g++-5';
        
        var CPATH = process.env['CPATH'];
        
        process.env['CPATH'] = '/usr/arm-linux-gnueabihf/include/c++/5/';
        process.env['CPATH'] += '/usr/arm-linux-gnueabihf/include/c++/5/arm-linux-gnueabihf/';
        process.env['CPATH'] += '/usr/arm-linux-gnueabihf/include/c++/5/backward/';
        process.env['CPATH'] += CPATH ? ':' + CPATH : '';
      } else {
         if (NODE_ZERO) {
console.log("NODE_ZERO");
          process.env['GYP_DEFINES'] += ' clang=0';
          process.env['CXX'] = 'g++-4.8';
          
          var CPATH = process.env['CPATH'];
          
          process.env['CPATH'] = '/usr/include/c++/4.8/';
          process.env['CPATH'] += '/usr/include/x86_64-linux-gnu/c++/4.8/';
          process.env['CPATH'] += '/usr/include/c++/4.8/backward/';
          process.env['CPATH'] += CPATH ? ':' + CPATH : '';
        } else {
console.log("!!!!NODE_ZERO");
          process.env['GYP_DEFINES'] += ' clang=1';
        }

        if (!process.env['JAVA_HOME']) {
          if (fs.existsSync('/usr/lib/jvm/java')) {
            process.env['JAVA_HOME'] = '/usr/lib/jvm/java';
          } else {
            process.env['JAVA_HOME'] = '/usr/lib/jvm/default-java';
          }
        } 
         /*     process.env['GYP_DEFINES'] += ' clang=0';

      if (!process.env['JAVA_HOME']) {
        if (fs.existsSync('/usr/lib/jvm/java')) {
          process.env['JAVA_HOME'] = '/usr/lib/jvm/java';
        } else {
          process.env['JAVA_HOME'] = '/usr/lib/jvm/default-java';
        }
      }*/
      }

      break;
    default:
      break;
  }

  console.log('target_arch =', ARCH);
  console.log('host_arch =', process.arch);
  console.log('configuration =', CONFIG);

  sync();
}

function config() {
  if (!fs.existsSync(WEBRTC) || !fs.existsSync(path.resolve(WEBRTC, '.gclient')) || !fs.existsSync(WEBRTC_SRC)) {
    if (!fs.existsSync(WEBRTC)) {
      fs.mkdirSync(WEBRTC);
    }
    
    var res = spawn(GCLIENT, ['config', '--name=src', CHROMIUM], {
      cwd: WEBRTC,
      env: process.env,
      stdio: 'inherit',
    });

    res.on('close', function (code) {
      if (!code) {
        return configure();
      }

      process.exit(1);
    });
  } else {
    configure();
  }
}

process.env['GYP_DEFINES'] = process.env['GYP_DEFINES'] ? process.env['GYP_DEFINES'] : '';

if (!fs.existsSync(THIRD_PARTY)) {
  fs.mkdirSync(THIRD_PARTY);
}

process.env['PATH'] = process.env['PATH'] + path.delimiter + DEPOT_TOOLS;

if (!fs.existsSync(DEPOT_TOOLS)) {
  var res = spawn('git', ['clone', DEPOT_TOOLS_REPO], {
    cwd: THIRD_PARTY,
    env: process.env,
    stdio: 'inherit',
  });

  res.on('close', function (code) {
    if (!code) {
      return config();
    }

    process.exit(1);
  });
} else {
  config();
}
