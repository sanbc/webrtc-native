/*
* The MIT License (MIT)
*
* Copyright (c) 2015 vmolsa <ville.molsa@gmail.com> (http://github.com/vmolsa)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include "uv.h"
#include "Core.h"

#ifdef WIN32
#include "webrtc/base/win32socketinit.h"
#endif

using namespace WebRTC;

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;
rtc::scoped_ptr<cricket::DeviceManagerInterface> _manager;
uv_check_t _msg;

static void ProcessMessages(uv_check_t* handle) {
  rtc::Thread::Current()->ProcessMessages(0);
}

void Core::Init() {
#ifdef WIN32
  rtc::EnsureWinsockInit();
#endif
  uv_check_init(uv_default_loop(), &_msg);
  uv_check_start(&_msg, ProcessMessages);
  uv_unref(reinterpret_cast<uv_handle_t*>(&_msg));

  rtc::InitializeSSL();
  _factory = webrtc::CreatePeerConnectionFactory();
  _manager.reset(cricket::DeviceManagerFactory::Create());

  if (!_manager->Init()) {
    _manager.release();
  }
}

void Core::Dispose() {
  uv_check_stop(&_msg);
  _factory.release();

  if (_manager.get()) {
    _manager->Terminate();
  }

  _manager.release();
}

webrtc::PeerConnectionFactoryInterface* Core::GetFactory() {
  return _factory.get();
}

cricket::DeviceManagerInterface* Core::GetManager() {
  return _manager.get();
}