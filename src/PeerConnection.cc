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

#include <nan.h>
#include "Global.h"
#include "Platform.h"
#include "PeerConnection.h"
#include "DataChannel.h"
#include "MediaStream.h"
#include "Stats.h"
#include <iostream>
#include <string>
#include "webrtc/pc/test/fakertccertificategenerator.h"
using namespace v8;
using namespace WebRTC;
using namespace std;
std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value) {
  std::string value;
  const char* env_var = getenv(env_var_name);
  if (env_var)
    value = env_var;

  if (value.empty())
    value = default_value;

  return value;
}
static const char kStunIceServer[] = "stun:stun.l.google.com:19302";
static const char kTurnIceServer[] = "turn:test%40hello.com@test.com:1234";
static const char kTurnIceServerWithTransport[] =
    "turn:test@hello.com?transport=tcp";
static const char kSecureTurnIceServer[] =
    "turns:test@hello.com?transport=tcp";
static const char kSecureTurnIceServerWithoutTransportParam[] =
    "turns:test_no_transport@hello.com:443";
static const char kSecureTurnIceServerWithoutTransportAndPortParam[] =
    "turns:test_no_transport@hello.com";
static const char kTurnIceServerWithNoUsernameInUri[] =
    "turn:test.com:1234";
static const char kTurnPassword[] = "turnpassword";
static const int kDefaultStunPort = 3478;
static const int kDefaultStunTlsPort = 5349;
static const char kTurnUsername[] = "test";
static const char kStunIceServerWithIPv4Address[] = "stun:1.2.3.4:1234";
static const char kStunIceServerWithIPv4AddressWithoutPort[] = "stun:1.2.3.4";
static const char kStunIceServerWithIPv6Address[] = "stun:[2401:fa00:4::]:1234";
static const char kStunIceServerWithIPv6AddressWithoutPort[] =
    "stun:[2401:fa00:4::]";
static const char kTurnIceServerWithIPv6Address[] =
    "turn:test@[2401:fa00:4::]:1234";
std::string GetPeerConnectionString() {
  return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
}

void PeerConnection::Init(Handle<Object> exports) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  Nan::HandleScope scope;
  
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(PeerConnection::New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("RTCPeerConnection").ToLocalChecked());
 
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", PeerConnection::AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "addStream", PeerConnection::AddStream);
  Nan::SetPrototypeMethod(tpl, "addTrack", PeerConnection::AddTrack);
  Nan::SetPrototypeMethod(tpl, "close", PeerConnection::Close);
  Nan::SetPrototypeMethod(tpl, "createAnswer", PeerConnection::CreateAnswer);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", PeerConnection::CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "createOffer", PeerConnection::CreateOffer);
  Nan::SetPrototypeMethod(tpl, "generateCertificate", PeerConnection::GenerateCertificate);
  Nan::SetPrototypeMethod(tpl, "getConfiguration", PeerConnection::GetConfiguration);
  Nan::SetPrototypeMethod(tpl, "peerIdentity", PeerConnection::PeerIdentity);
  Nan::SetPrototypeMethod(tpl, "getReceivers", PeerConnection::GetReceivers);
  Nan::SetPrototypeMethod(tpl, "getSenders", PeerConnection::GetSenders);
  Nan::SetPrototypeMethod(tpl, "getStats", PeerConnection::GetStats);
  Nan::SetPrototypeMethod(tpl, "getTransceivers", PeerConnection::GetTransceivers);
  Nan::SetPrototypeMethod(tpl, "RemoveTrack", PeerConnection::RemoveTrack);
  Nan::SetPrototypeMethod(tpl, "removeStream", PeerConnection::RemoveStream);
  Nan::SetPrototypeMethod(tpl, "setConfiguration", PeerConnection::SetConfiguration);
  Nan::SetPrototypeMethod(tpl, "setIdentityProvider", PeerConnection::SetIdentityProvider);
  Nan::SetPrototypeMethod(tpl, "setLocalDescription", PeerConnection::SetLocalDescription);
  Nan::SetPrototypeMethod(tpl, "setRemoteDescription", PeerConnection::SetRemoteDescription);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("canTrickleIceCandidates").ToLocalChecked(), PeerConnection::CanTrickleIceCandidates);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("connectionState").ToLocalChecked(), PeerConnection::ConnectionState);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentLocalDescription").ToLocalChecked(), PeerConnection::CurrentLocalDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentRemoteDescription").ToLocalChecked(), PeerConnection::CurrentRemoteDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("defaultIceServers").ToLocalChecked(), PeerConnection::DefaultIceServers);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), PeerConnection::GetIceConnectionState);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), PeerConnection::GetIceGatheringState);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), PeerConnection::GetLocalDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("peerIdentity").ToLocalChecked(), PeerConnection::PeerIdentity);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingLocalDescription").ToLocalChecked(), PeerConnection::PendingLocalDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingRemoteDescription").ToLocalChecked(), PeerConnection::PendingRemoteDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), PeerConnection::GetRemoteDescription);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("sctp").ToLocalChecked(), PeerConnection::GetSctp);  
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), PeerConnection::GetSignalingState);  

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onconnectionstatechange").ToLocalChecked(), PeerConnection::GetOnConnectionStateChange, PeerConnection::SetOnConnectionStateChange);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ondatachannel").ToLocalChecked(), PeerConnection::GetOnDataChannel, PeerConnection::SetOnDataChannel);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onnegotiationneeded").ToLocalChecked(), PeerConnection::GetOnNegotiationNeeded, PeerConnection::SetOnNegotiationNeeded);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onicecandidate").ToLocalChecked(), PeerConnection::GetOnIceCandidate, PeerConnection::SetOnIceCandidate);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onicecandidateerror").ToLocalChecked(), PeerConnection::GetOnIceCandidateError, PeerConnection::SetOnIceCandidateError);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("oniceconnectionstatechange").ToLocalChecked(), PeerConnection::GetOnIceConnectionStateChange, PeerConnection::SetOnIceConnectionStateChange);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onicegatheringstatechange").ToLocalChecked(), PeerConnection::GetOnIceGatheringStateChange, PeerConnection::SetOnIceGatheringStateChange);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onsignalingstatechange").ToLocalChecked(), PeerConnection::GetOnSignalingStateChange, PeerConnection::SetOnSignalingStateChange);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ontrack").ToLocalChecked(), PeerConnection::GetOnTrack, PeerConnection::SetOnTrack);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onaddstream").ToLocalChecked(), PeerConnection::GetOnAddStream, PeerConnection::SetOnAddStream);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onidentityresult").ToLocalChecked(), PeerConnection::GetOnIdentityResult, PeerConnection::SetOnIdentityResult);  
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onidpassertionerror").ToLocalChecked(), PeerConnection::GetOnIdpAssertionError, PeerConnection::SetOnIdpAssertionError);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onidpvalidationerror").ToLocalChecked(), PeerConnection::GetOnIdpValidationError, PeerConnection::SetOnIdpValidationError);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onpeeridentity").ToLocalChecked(), PeerConnection::GetOnPeerIdentity, PeerConnection::SetOnPeerIdentity);  
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onremovestream").ToLocalChecked(), PeerConnection::GetOnRemoveStream, PeerConnection::SetOnRemoveStream);

  constructor.Reset<Function>(tpl->GetFunction());
  exports->Set(Nan::New("RTCPeerConnection").ToLocalChecked(), tpl->GetFunction());
}

Nan::Persistent<Function> PeerConnection::constructor;

PeerConnection::PeerConnection(const Configuration config) : _config(config) { 
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  _stats = new rtc::RefCountedObject<StatsObserver>(this);
  _offer = new rtc::RefCountedObject<OfferObserver>(this);
  _answer = new rtc::RefCountedObject<AnswerObserver>(this);
  _local = new rtc::RefCountedObject<LocalDescriptionObserver>(this);
  _remote = new rtc::RefCountedObject<RemoteDescriptionObserver>(this);
  _peer = new rtc::RefCountedObject<PeerConnectionObserver>(this); 
  LOG(LS_INFO) <<  "BEFORE PEERCONNECTION FACTORY CREATED";
  _factory  = webrtc::CreatePeerConnectionFactory(
        Platform::GetWorker(), Platform::GetWorker(), Platform::GetSignal(),
        nullptr, nullptr, nullptr);

//LOG(LS_INFO) <<  "PEERCONNECTION FACTORY CREATED";

//_factory  = webrtc::CreatePeerConnectionFactory();
// AddStream();
/*  webrtc::PeerConnectionInterface::RTCConfiguration  Config;
  webrtc::PeerConnectionInterface::IceServer server;
  server.uri = "stun:stun.l.google.com:19302";
  Config.servers.push_back(server);

  webrtc::FakeConstraints constraints;
constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                            "false");
*/

  //_socket = _factory->CreatePeerConnection(
    //  config, nullptr, nullptr, _peer.get());

//_socket = _factory->CreatePeerConnection(
 //     config, NULL, NULL, _peer.get());

  if (!_factory.get()) {
    LOG(LS_INFO) <<  "PEERCONNECTION _factory null";
  }

  
 

}

PeerConnection::~PeerConnection() {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  if (_socket.get()) {
    webrtc::PeerConnectionInterface::SignalingState state(_socket->signaling_state());

    if (state != webrtc::PeerConnectionInterface::kClosed) {
      _socket->Close();
    }
  }
  
  _stats->RemoveListener(this);
  _offer->RemoveListener(this);
  _answer->RemoveListener(this);
  _local->RemoveListener(this);
  _remote->RemoveListener(this);
  _peer->RemoveListener(this);
}

webrtc::PeerConnectionInterface *PeerConnection::GetSocket() {

  RTC_DCHECK(_factory.get() != nullptr);
  RTC_DCHECK(_peer.get() != nullptr);

  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  if (!_socket.get()) {
    if (_factory.get()) {
 
  //Configuration config;
  
   //config.ice_server.uri = "stun:stun.l.google.com:19302";
LOG(LS_INFO) <<  "BEFORE PEERCONNECTION CREATED ";
  //config.servers.push_back(config.ice_server);
  //config.servers.push_back(config.ice_server);
/*  config.servers[1].uri = kTurnIceServer;
  config.servers[1].password = kTurnPassword;
//  config.servers.push_back(config.ice_server);
  config.servers[2].uri = kTurnIceServerWithTransport;
  config.servers[2].password = kTurnPassword;
  //config.servers.push_back(config.ice_server);*/
  //std::unique_ptr<FakeRTCCertificateGenerator> cert_generator(
 //     new FakeRTCCertificateGenerator());
//LOG(LS_INFO) <<  "BEFORE PEERCONNECTION CREATED "<<_config.ice_candidate_pool_size ;
//_config.ice_candidate_pool_size = 3;
//LOG(LS_INFO) <<  "BEFORE PEERCONNECTION CREATED "<<_config.ice_candidate_pool_size ;
  /*rtc::scoped_refptr<webrtc::PeerConnectionInterface> _socket(_factory->CreatePeerConnection(
      _config, nullptr, nullptr, nullptr,
      _peer.get()));

*/
    //  EventEmitter::SetReference(true);
 //string servers1 = "stun:stun.l.google.com:19302";
//LOG(LS_INFO)  << "sever is :: "<< _config.servers[0].uri;

 /* webrtc::PeerConnectionInterface::RTCConfiguration  config;
 // webrtc::PeerConnectionInterface::RTCConfiguration  config;
  webrtc::PeerConnectionInterface::IceServer server;
//  server.urls.push_back(std::string("stun:stun.l.google.com:19302"));
  server.uri = std::string("stun:stun.l.google.com:19302");
  //config.media_config.video.suspend_below_min_bitrate = false;                    
  config.servers.push_back(server);
  config.disable_ipv6_on_wifi = false;
 // _socket = _factory->CreatePeerConnection(_config, 0, 0, _peer.get());
//_factory->SetConfiguration(Config);
  webrtc::FakeConstraints constraints;
constraints.SetAllowDtlsSctpDataChannels();
constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                            "false");
  std::unique_ptr<FakeRTCCertificateGenerator> cert_generator(
      new FakeRTCCertificateGenerator());
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _socket(_factory->CreatePeerConnection(
      config, nullptr, nullptr, std::move(cert_generator), _peer.get()));
*/
_config.rtcp_mux_policy = webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyNegotiate;
webrtc::FakeConstraints constraints;
constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                            "true");
		constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, "false");
		constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, "false");
LOG(LS_INFO)  << "sever is :: "<< _config.servers[0].uri;
 _socket = _factory->CreatePeerConnection(_config,  nullptr, nullptr, _peer.get());
 //AddStream(_socket.get());
// _socket = _factory->CreatePeerConnection(config, &constraints,nullptr, nullptr, _peer.get());
LOG(LS_INFO) <<  "AFTER PEERCONNECTION CREATED";
      if (!_socket.get()) {
        Nan::ThrowError("Internal Socket Error");
      }
    } else {
      Nan::ThrowError("Internal Factory Error");
    }
  }
   
  return _socket.get();
}

void PeerConnection::New(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;





// v8::Local<v8::Object> param1 = info[0]->ToObject();
//v8::Local<v8::Value> name_ = param1->Get(New<v8::String>("name").ToLocalChecked());
    if (!info[0].IsEmpty() && info[0]->IsObject()) {
    Local<Object> desc = Local<Object>::Cast(info[0]);
 Local<Value> servers = desc->Get(Nan::New("servers").ToLocalChecked());


v8::String::Utf8Value param1(servers->ToString());

    // convert it to string
    std::string foo = std::string(*param1);    

LOG(LS_INFO) << "val is" << foo;
}     
    // convert it to string
    



  if (info.IsConstructCall()) {
Configuration config;
 Local<Object> _config = Local<Object>::Cast(info[0]);
Local<Value> iceservers_value = _config->Get(Nan::New("iceServers").ToLocalChecked());
    if (!iceservers_value.IsEmpty() && iceservers_value->IsArray()) {
      Local<Array> list = Local<Array>::Cast(iceservers_value);

      for (unsigned int index = 0; index < list->Length(); index++) {
        Local<Value> server_value = list->Get(index);
Local<Object> server = Local<Object>::Cast(server_value);
          Local<Value> url_value = server->Get(Nan::New("url").ToLocalChecked());
          
          if (!url_value.IsEmpty() && url_value->IsString()) {
            v8::String::Utf8Value url(url_value->ToString());
            webrtc::PeerConnectionInterface::IceServer entry;

            entry.uri = *url;

            LOG(LS_INFO) << "val is"<<entry.uri;

            config.servers.push_back(entry);
LOG(LS_INFO) << "val is"<<config.servers[0].uri;

}}
}
    PeerConnection* peer = new PeerConnection(config);
    peer->Wrap(info.This(), "PeerConnection");
 LOG(LS_INFO) << "peerconnection connected";
    return info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 2;
    Local<Value> argv[argc] = {
      info[0]
    };
    
    Local<Function> instance = Nan::New(PeerConnection::constructor);
    return info.GetReturnValue().Set(instance->NewInstance(argc, argv));
  }
}

void PeerConnection::AddIceCandidate(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  
  const char *error = 0;
  Local<Value> argv[1];

  if (!info[0].IsEmpty() && info[0]->IsObject()) {
    Local<Object> desc = Local<Object>::Cast(info[0]);
    Local<Value> sdpMid_value = desc->Get(Nan::New("sdpMid").ToLocalChecked());
    Local<Value> sdpMLineIndex_value = desc->Get(Nan::New("sdpMLineIndex").ToLocalChecked());
    Local<Value> sdp_value = desc->Get(Nan::New("candidate").ToLocalChecked());
    
    if (!sdpMid_value.IsEmpty() && sdpMid_value->IsString()) {
      if (!sdpMLineIndex_value.IsEmpty() && sdpMLineIndex_value->IsInt32()) {
        if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
          Local<Int32> sdpMLineIndex(sdpMLineIndex_value->ToInt32());
          String::Utf8Value sdpMid(sdpMid_value->ToString());
          String::Utf8Value sdp(sdp_value->ToString());
          
          std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(*sdpMid, sdpMLineIndex->Value(), *sdp, 0));
  
          if (candidate.get()) {
            if (socket) {
              if (socket->AddIceCandidate(candidate.get())) {
                if (!info[1].IsEmpty() && info[1]->IsFunction()) {
                  Local<Function> success = Local<Function>::Cast(info[1]);
                  success->Call(info.This(), 0, argv);
                }
              } else {            
                error = "Failed to add ICECandidate";
              }
            } else {
              error = "Internal Error";
            }
          } else {
            error = "Invalid ICECandidate";
          }
        } else {
          error = "Invalid candidate";
        }
      } else {
        error = "Invalid sdpMLineIndex";
      }
    } else {
      error = "Invalid sdpMid";
    }
  }
  
  if (error) {
    if (!info[2].IsEmpty() && info[2]->IsFunction()) {
      argv[0] = Nan::Error(error);
      
      Local<Function> onerror = Local<Function>::Cast(info[2]);
      onerror->Call(info.This(), 1, argv);
    } else {
      Nan::ThrowError(error);
    }
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::AddTrack(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO():
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::Close(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection"); 
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  
  if (socket) {
    socket->Close();
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::CreateAnswer(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  
  if (!info[0].IsEmpty() && info[0]->IsFunction()) {
    self->_answerCallback.Reset<Function>(Local<Function>::Cast(info[0]));
  } else {
    self->_answerCallback.Reset();
  }
  
  if (!info[1].IsEmpty() && info[1]->IsFunction()) {
    self->_answerErrorCallback.Reset<Function>(Local<Function>::Cast(info[1]));
  } else {
    self->_answerErrorCallback.Reset();
  }
  
  if (socket) {
    // TODO(): RTCOfferAnswerOptions
    socket->CreateAnswer(self->_answer.get(), 0);
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::CreateDataChannel(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  std::string label;
  webrtc::DataChannelInit config;
  
  if (!info[0].IsEmpty() && info[0]->IsString()) {
    String::Utf8Value label_utf8(info[0]->ToString());
    label = *label_utf8;
  }
  
  if (!info[1].IsEmpty() && info[1]->IsObject()) {
    Local<Object> config_obj = Local<Object>::Cast(info[0]);
    
    Local<Value> reliable_value = config_obj->Get(Nan::New("reliable").ToLocalChecked());
    Local<Value> ordered_value = config_obj->Get(Nan::New("ordered").ToLocalChecked());
    Local<Value> maxRetransmitTime_value = config_obj->Get(Nan::New("maxRetransmitTime").ToLocalChecked());
    Local<Value> maxRetransmits_value = config_obj->Get(Nan::New("maxRetransmits").ToLocalChecked());
    Local<Value> protocol_value = config_obj->Get(Nan::New("protocol").ToLocalChecked());
    Local<Value> id_value = config_obj->Get(Nan::New("id").ToLocalChecked());

    if (!reliable_value.IsEmpty()) {
      if (reliable_value->IsTrue()) {
        config.reliable = true;
      } else {
        config.reliable = false;
      }
    }
    
    if (!ordered_value.IsEmpty()) {
      if (ordered_value->IsTrue()) {
        config.ordered = true;
      } else {
        config.ordered = false;
      }
    }
    
    if (!maxRetransmitTime_value.IsEmpty() && maxRetransmitTime_value->IsInt32()) {
      Local<Int32> maxRetransmitTime(maxRetransmitTime_value->ToInt32());
      config.maxRetransmitTime = maxRetransmitTime->Value();
    }
    
    if (!maxRetransmits_value.IsEmpty() && maxRetransmits_value->IsInt32()) {
      Local<Int32> maxRetransmits(maxRetransmits_value->ToInt32());
      config.maxRetransmits = maxRetransmits->Value();
    }
    
    if (!protocol_value.IsEmpty() && protocol_value->IsString()) {
      String::Utf8Value protocol(protocol_value->ToString());
      config.protocol = *protocol;
    }
    
    if (!id_value.IsEmpty() && id_value->IsInt32()) {
      Local<Int32> id(id_value->ToInt32());
      config.id = id->Value();
    }
  }
  
  if (socket) {
    rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel = socket->CreateDataChannel(label, &config);
    
    if (dataChannel.get()) {
      return info.GetReturnValue().Set(DataChannel::New(dataChannel));
    }
  }
  
  Nan::ThrowError("Internal Error");
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::CreateOffer(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  
  if (!info[0].IsEmpty() && info[0]->IsFunction()) {
    self->_offerCallback.Reset<Function>(Local<Function>::Cast(info[0]));
  } else {
    self->_offerCallback.Reset();
  }
  
  if (!info[1].IsEmpty() && info[1]->IsFunction()) {
    self->_offerErrorCallback.Reset<Function>(Local<Function>::Cast(info[1]));
  } else {
    self->_offerErrorCallback.Reset();
  }
  
  if (socket) {
    // TODO(): RTCOfferAnswerOptions
    socket->CreateOffer(self->_offer.get(), 0);
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GenerateCertificate(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetConfiguration(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");

  info.GetReturnValue().Set(self->_config.ToConfiguration());
}

void PeerConnection::PeerIdentity(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetReceivers(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetSenders(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetStats(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (!info[0].IsEmpty() && info[0]->IsFunction()) {
    self->_onstats.Reset<Function>(Local<Function>::Cast(info[0]));

    if (socket) {
      if (!socket->GetStats(self->_stats.get(), 0, webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
        Local<Function> callback = Nan::New<Function>(self->_onstats);
        Local<Value> argv[1] = { Nan::Null() };

        callback->Call(info.This(), 1, argv);
        self->_onstats.Reset();
      }
    } else {
      Nan::ThrowError("Internal Error");
    }
  } else {
    Nan::ThrowError("Missing Callback");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetTransceivers(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::RemoveTrack(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetConfiguration(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  if (!info[0].IsEmpty()) {
    PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
    webrtc::PeerConnectionInterface *socket = self->GetSocket();
    self->_config = Configuration(Local<Object>::Cast(info[0]));
    socket->SetConfiguration(self->_config);
  }

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetIdentityProvider(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetLocalDescription(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  const char *error = "Invalid SessionDescription";

  if (!info[0].IsEmpty() && info[0]->IsObject()) {
    Local<Object> desc_obj = Local<Object>::Cast(info[0]);
    Local<Value> type_value = desc_obj->Get(Nan::New("type").ToLocalChecked());
    Local<Value> sdp_value = desc_obj->Get(Nan::New("sdp").ToLocalChecked());
    
    if (!type_value.IsEmpty() && type_value->IsString()) {
      if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
        if (!info[1].IsEmpty() && info[1]->IsFunction()) {
          self->_localCallback.Reset<Function>(Local<Function>::Cast(info[1]));
        } else {
          self->_localCallback.Reset();
        }

        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
          self->_localErrorCallback.Reset<Function>(Local<Function>::Cast(info[2]));
        } else {
          self->_localErrorCallback.Reset();
        }
    
        String::Utf8Value type(type_value->ToString());
        String::Utf8Value sdp(sdp_value->ToString());

        webrtc::SessionDescriptionInterface *desc(webrtc::CreateSessionDescription(*type, *sdp, 0));
        
        if (desc) {
          if (socket) {
            self->_localsdp.Reset<Object>(desc_obj);
            socket->SetLocalDescription(self->_local.get(), desc);
            error = 0;
          } else {
            error = "Internal Error";
          }
        }
      }
    }
  }
  
  if (error) {
    if (!info[2].IsEmpty() && info[2]->IsFunction()) {
      Local<Value> argv[1] = {
        Nan::Error(error)
      };

      Local<Function> onerror = Local<Function>::Cast(info[2]);
      onerror->Call(info.This(), 1, argv);
    } else {
      Nan::ThrowError(error);
    }
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetRemoteDescription(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();
  const char *error = "Invalid SessionDescription";
  
  if (!info[0].IsEmpty() && info[0]->IsObject()) {
    Local<Object> desc_obj = Local<Object>::Cast(info[0]);
    Local<Value> type_value = desc_obj->Get(Nan::New("type").ToLocalChecked());
    Local<Value> sdp_value = desc_obj->Get(Nan::New("sdp").ToLocalChecked());
    
    if (!type_value.IsEmpty() && type_value->IsString()) {
      if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
        if (!info[1].IsEmpty() && info[1]->IsFunction()) {
          self->_remoteCallback.Reset<Function>(Local<Function>::Cast(info[1]));
        } else {
          self->_remoteCallback.Reset();
        }

        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
          self->_remoteErrorCallback.Reset<Function>(Local<Function>::Cast(info[2]));
        } else {
          self->_remoteErrorCallback.Reset();
        }
    
        String::Utf8Value type(type_value->ToString());
        String::Utf8Value sdp(sdp_value->ToString());

        webrtc::SessionDescriptionInterface *desc(webrtc::CreateSessionDescription(*type, *sdp, 0));
        
        if (desc) {
          if (socket) {
            self->_remotesdp.Reset<Object>(desc_obj);
            socket->SetRemoteDescription(self->_remote.get(), desc);
            error = 0;
          } else {
            error = "Internal Error";
          }
        }
      }
    }
  }

  if (error) {
    if (!info[2].IsEmpty() && info[2]->IsFunction()) {
      Local<Value> argv[1] = {
        Nan::Error(error)
      };

      Local<Function> onerror = Local<Function>::Cast(info[2]);
      onerror->Call(info.This(), 1, argv);
    } else {
      Nan::ThrowError(error);
    }
  }
  
  info.GetReturnValue().SetUndefined();
}

/* Supporting old API for now */
std::unique_ptr<cricket::VideoCapturer> PeerConnection::OpenVideoCaptureDevice() {
  std::vector<std::string> device_names;
  {
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!info) {
      return nullptr;
    }
    int num_devices = info->NumberOfDevices();
    for (int i = 0; i < num_devices; ++i) {
      const uint32_t kSize = 256;
      char name[kSize] = {0};
      char id[kSize] = {0};
      if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
        device_names.push_back(name);
      }
    }
  }

  cricket::WebRtcVideoDeviceCapturerFactory factory;
  std::unique_ptr<cricket::VideoCapturer> capturer;
  for (const auto& name : device_names) {
    capturer = factory.Create(cricket::Device(name, 0));
    if (capturer) {
      break;
    }
  }
  return capturer;
}
/*void PeerConnection::AddStream(webrtc::PeerConnectionInterface *socket) {
LOG(LS_INFO) << __PRETTY_FUNCTION__;
  if (active_streams_.find(kStreamLabel) != active_streams_.end())
    return;  // Already added.

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      _factory->CreateAudioTrack(
          kAudioLabel, _factory->CreateAudioSource(NULL)));
LOG(LS_INFO) << "audio_track";
  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
      _factory->CreateVideoTrack(
          kVideoLabel,
          _factory->CreateVideoSource(OpenVideoCaptureDevice(),
                                                      NULL)));
 
LOG(LS_INFO) << "video_track";
  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
      _factory->CreateLocalMediaStream(kStreamLabel);

  stream->AddTrack(audio_track);
  stream->AddTrack(video_track);
  
  typedef std::pair<std::string,
                    rtc::scoped_refptr<webrtc::MediaStreamInterface> >
      MediaStreamPair;
  active_streams_.insert(MediaStreamPair(stream->label(), stream));

socket->AddStream(stream);
  
}*/

void PeerConnection::AddStream(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = MediaStream::Unwrap(info[0]);

  if (mediaStream.get()) {
    webrtc::PeerConnectionInterface *socket = self->GetSocket();

    if (socket) {
      if (!socket->AddStream(mediaStream)) {
        Nan::ThrowError("AddStream Failed");
      }
    } else {
      Nan::ThrowError("Internal Error");
    }
  } else {
    Nan::ThrowError("Invalid MediaStream Object");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::RemoveStream(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = MediaStream::Unwrap(info[0]);

  if (mediaStream.get()) {
    webrtc::PeerConnectionInterface *socket = self->GetSocket();

    if (socket) {
      socket->RemoveStream(mediaStream);
    } else {
      Nan::ThrowError("Internal Error");
    }
  } else {
    Nan::ThrowError("Invalid MediaStream Object");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetLocalStreams(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    rtc::scoped_refptr<webrtc::StreamCollectionInterface> local = socket->local_streams();

    if (local.get()) {
      Local<Array> list = Nan::New<Array>();
      uint32_t index = 0;
      size_t count;

      for (count = 0; count < local->count(); count++) {
        list->Set(index, MediaStream::New(local->at(count)));
      }

      return info.GetReturnValue().Set(list);
    }
  }
    
  Nan::ThrowError("Internal Error");
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetRemoteStreams(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    rtc::scoped_refptr<webrtc::StreamCollectionInterface> remote = socket->remote_streams();

    if (remote.get()) {
      Local<Array> list = Nan::New<Array>();
      uint32_t index = 0;
      size_t count;

      for (count = 0; count < remote->count(); count++) {
        list->Set(index, MediaStream::New(remote->at(count)));
      }

      return info.GetReturnValue().Set(list);
    }
  }

  Nan::ThrowError("Internal Error");
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetStreamById(const Nan::FunctionCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.This(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    if (info.Length() >= 1 && info[0]->IsString()) {
      v8::String::Utf8Value idValue(info[0]->ToString());
      std::string id(*idValue);

      rtc::scoped_refptr<webrtc::StreamCollectionInterface> local = socket->local_streams();
      rtc::scoped_refptr<webrtc::StreamCollectionInterface> remote = socket->remote_streams();
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;

      if (local.get()) {
        stream = local->find(id);
      }

      if (remote.get() && !stream.get()) {
        stream = remote->find(id);
      }

      if (stream.get()) {
        return info.GetReturnValue().Set(MediaStream::New(stream));
      } else {
        return info.GetReturnValue().Set(Nan::Null());
      }
    } else {
      Nan::ThrowError("Invalid Argument");
    }
  }

  Nan::ThrowError("Internal Error");
  info.GetReturnValue().SetUndefined();
}

/* <<>> */

void PeerConnection::CanTrickleIceCandidates(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::ConnectionState(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::CurrentLocalDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::CurrentRemoteDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::DefaultIceServers(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetIceConnectionState(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    webrtc::PeerConnectionInterface::IceConnectionState state(socket->ice_connection_state());

    switch (state) {
      case webrtc::PeerConnectionInterface::kIceConnectionNew:
        return info.GetReturnValue().Set(Nan::New("new").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceConnectionChecking:
        return info.GetReturnValue().Set(Nan::New("checking").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceConnectionConnected:
        return info.GetReturnValue().Set(Nan::New("connected").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
        return info.GetReturnValue().Set(Nan::New("completed").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceConnectionFailed:
        return info.GetReturnValue().Set(Nan::New("failed").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
        return info.GetReturnValue().Set(Nan::New("disconnected").ToLocalChecked());
        break;
      default:
        return info.GetReturnValue().Set(Nan::New("closed").ToLocalChecked());
        break;
    }
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetIceGatheringState(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    webrtc::PeerConnectionInterface::IceGatheringState state(socket->ice_gathering_state());

    switch (state) {
      case webrtc::PeerConnectionInterface::kIceGatheringNew:
        return info.GetReturnValue().Set(Nan::New("new").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kIceGatheringGathering:
        return info.GetReturnValue().Set(Nan::New("gathering").ToLocalChecked());
        break;
      default:
        return info.GetReturnValue().Set(Nan::New("complete").ToLocalChecked());
        break;
    }   
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetLocalDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Object>(self->_localsdp));
}

void PeerConnection::PeerIdentity(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::PendingLocalDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::PendingRemoteDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetRemoteDescription(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Object>(self->_remotesdp));
}

void PeerConnection::GetSctp(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetSignalingState(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  webrtc::PeerConnectionInterface *socket = self->GetSocket();

  if (socket) {
    webrtc::PeerConnectionInterface::SignalingState state(socket->signaling_state());
    
    switch (state) {
      case webrtc::PeerConnectionInterface::kStable:
        return info.GetReturnValue().Set(Nan::New("stable").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kHaveLocalOffer:
        return info.GetReturnValue().Set(Nan::New("have-local-offer").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer:
        return info.GetReturnValue().Set(Nan::New("have-local-pranswer").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kHaveRemoteOffer:
        return info.GetReturnValue().Set(Nan::New("have-remote-offer").ToLocalChecked());
        break;
      case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer:
        return info.GetReturnValue().Set(Nan::New("have-remote-pranswer").ToLocalChecked());
        break;
      default: 
        return info.GetReturnValue().Set(Nan::New("closed").ToLocalChecked());
        break;
    }
  } else {
    Nan::ThrowError("Internal Error");
  }
  
  info.GetReturnValue().SetUndefined();
}

void PeerConnection::GetOnConnectionStateChange(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnConnectionStateChange(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnDataChannel(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_ondatachannel));
}

void PeerConnection::SetOnDataChannel(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_ondatachannel.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_ondatachannel.Reset();
  }
}

void PeerConnection::GetOnNegotiationNeeded(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_onnegotiationneeded));
}

void PeerConnection::SetOnNegotiationNeeded(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_onnegotiationneeded.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_onnegotiationneeded.Reset();
  }
}

void PeerConnection::GetOnIceCandidate(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_onicecandidate));
}

void PeerConnection::SetOnIceCandidate(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_onicecandidate.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_onicecandidate.Reset();
  }
}

void PeerConnection::GetOnIceCandidateError(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnIceCandidateError(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnIceConnectionStateChange(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_oniceconnectionstatechange));
}

void PeerConnection::SetOnIceConnectionStateChange(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_oniceconnectionstatechange.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_oniceconnectionstatechange.Reset();
  }
}

void PeerConnection::GetOnIceGatheringStateChange(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnIceGatheringStateChange(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnSignalingStateChange(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_onsignalingstatechange));
}

void PeerConnection::SetOnSignalingStateChange(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_onsignalingstatechange.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_onsignalingstatechange.Reset();
  }
}

void PeerConnection::GetOnTrack(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnTrack(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnAddStream(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_onaddstream));
}

void PeerConnection::SetOnAddStream(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_onaddstream.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_onaddstream.Reset();
  }
}

void PeerConnection::GetOnIdentityResult(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnIdentityResult(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnIdpAssertionError(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnIdpAssertionError(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnIdpValidationError(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnIdpValidationError(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnPeerIdentity(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;

  // TODO(): Implement This

  info.GetReturnValue().SetUndefined();
}

void PeerConnection::SetOnPeerIdentity(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  // TODO(): Implement This
}

void PeerConnection::GetOnRemoveStream(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");
  return info.GetReturnValue().Set(Nan::New<Function>(self->_onremovestream));
}

void PeerConnection::SetOnRemoveStream(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void> &info) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  PeerConnection *self = RTCWrap::Unwrap<PeerConnection>(info.Holder(), "PeerConnection");

  if (!value.IsEmpty() && value->IsFunction()) {
    self->_onremovestream.Reset<Function>(Local<Function>::Cast(value));
  } else {
    self->_onremovestream.Reset();
  }
}

void PeerConnection::On(Event *event) {
  LOG(LS_INFO) << __PRETTY_FUNCTION__;
  
  Nan::HandleScope scope;
  PeerConnectionEvent type = event->Type<PeerConnectionEvent>();
  Local<Function> callback;
  Local<Object> container;
  Local<Value> argv[1];
  bool isError = false;
  std::string data;
  int argc = 0;
  
  switch (type) {
    case kPeerConnectionCreateClosed:
      EventEmitter::SetReference(false);
      
      break;
    case kPeerConnectionCreateOffer:
      callback = Nan::New<Function>(_offerCallback);
      
      _offerCallback.Reset();
      _offerErrorCallback.Reset();

      data = event->Unwrap<std::string>();
      argv[0] = JSON::Parse(Nan::New(data.c_str()).ToLocalChecked());
      argc = 1;
      
      break;
    case kPeerConnectionCreateOfferError:
      callback = Nan::New<Function>(_offerErrorCallback);
      
      _offerCallback.Reset();
      _offerErrorCallback.Reset();
      
      isError = true;
      data = event->Unwrap<std::string>();
      argv[0] = Nan::Error(data.c_str());
      argc = 1;
      
      break;
    case kPeerConnectionCreateAnswer:
      callback = Nan::New<Function>(_answerCallback);
      
      _answerCallback.Reset();
      _answerErrorCallback.Reset();
      
      data = event->Unwrap<std::string>();
      argv[0] = JSON::Parse(Nan::New(data.c_str()).ToLocalChecked());
      argc = 1;
      
      break;
    case kPeerConnectionCreateAnswerError:
      callback = Nan::New<Function>(_answerErrorCallback);
      
      _answerCallback.Reset();
      _answerErrorCallback.Reset();
      
      isError = true;
      data = event->Unwrap<std::string>();
      argv[0] = Nan::Error(data.c_str());
      argc = 1;
      
      break;
    case kPeerConnectionSetLocalDescription:
      callback = Nan::New<Function>(_localCallback);
      
      _localCallback.Reset();
      _localErrorCallback.Reset();
      
      break;
    case kPeerConnectionSetLocalDescriptionError:
      callback = Nan::New<Function>(_localErrorCallback);
      
      _localCallback.Reset();
      _localErrorCallback.Reset();
      _localsdp.Reset();
      
      isError = true;
      data = event->Unwrap<std::string>();
      argv[0] = Nan::Error(data.c_str());
      argc = 1;
      
      break;
    case kPeerConnectionSetRemoteDescription:
      callback = Nan::New<Function>(_remoteCallback);
      
      _remoteCallback.Reset();
      _remoteErrorCallback.Reset();
      
      break;
    case kPeerConnectionSetRemoteDescriptionError:
      callback = Nan::New<Function>(_remoteErrorCallback);
      
      _remoteCallback.Reset();
      _remoteErrorCallback.Reset();
      _remotesdp.Reset();
      
      isError = true;
      data = event->Unwrap<std::string>();
      argv[0] = Nan::Error(data.c_str());
      argc = 1;
      
      break;
    case kPeerConnectionIceCandidate:
      callback = Nan::New<Function>(_onicecandidate);
      container = Nan::New<Object>();
      
      data = event->Unwrap<std::string>();
      
      if (data.empty()) {
        container->Set(Nan::New("candidate").ToLocalChecked(), Nan::Null());
      } else {
        container->Set(Nan::New("candidate").ToLocalChecked(), JSON::Parse(Nan::New(data.c_str()).ToLocalChecked()));
      }
      
      argv[0] = container;
      argc = 1;
      
      break;
    case kPeerConnectionSignalChange:
      callback = Nan::New<Function>(_onsignalingstatechange);
      
      break;
    case kPeerConnectionIceChange:
      callback = Nan::New<Function>(_oniceconnectionstatechange);
      
      break;
    case kPeerConnectionIceGathering:
      
      break;
    case kPeerConnectionDataChannel:
      callback = Nan::New<Function>(_ondatachannel);
      
      container = Nan::New<Object>();
      container->Set(Nan::New("channel").ToLocalChecked(), DataChannel::New(event->Unwrap<rtc::scoped_refptr<webrtc::DataChannelInterface> >()));

      argv[0] = container;
      argc = 1;
      
      break;
    case kPeerConnectionAddStream:
      callback = Nan::New<Function>(_onaddstream);

      container = Nan::New<Object>();
      container->Set(Nan::New("stream").ToLocalChecked(), MediaStream::New(event->Unwrap<rtc::scoped_refptr<webrtc::MediaStreamInterface> >()));
      
      argv[0] = container;
      argc = 1;

      break;
    case kPeerConnectionRemoveStream:
      callback = Nan::New<Function>(_onremovestream);
      
      container = Nan::New<Object>();
      container->Set(Nan::New("stream").ToLocalChecked(), MediaStream::New(event->Unwrap<rtc::scoped_refptr<webrtc::MediaStreamInterface> >()));
      
      argv[0] = container;
      argc = 1;

      break;
    case kPeerConnectionRenegotiation:
      callback = Nan::New<Function>(_onnegotiationneeded);
      
      break;
    case kPeerConnectionStats:
      callback = Nan::New<Function>(_onstats);

      argv[0] = RTCStatsResponse::New(event->Unwrap<webrtc::StatsReports>());
      argc = 1;

      break;
  }
  
  if (!callback.IsEmpty() && callback->IsFunction()) {
    callback->Call(RTCWrap::This(), argc, argv);
  } else if (isError) {
    Nan::ThrowError(argv[0]);
  }
}

bool PeerConnection::IsStable() {
  webrtc::PeerConnectionInterface *socket = PeerConnection::GetSocket();

  if (socket) {
    webrtc::PeerConnectionInterface::SignalingState state(socket->signaling_state());
    
    if (state == webrtc::PeerConnectionInterface::kStable) {
      return true;
    }
  }
  
  return false;
}



