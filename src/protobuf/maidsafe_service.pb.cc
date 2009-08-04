// Generated by the protocol buffer compiler.  DO NOT EDIT!

#include "maidsafe_service.pb.h"
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format_inl.h>

namespace maidsafe {

namespace {

const ::google::protobuf::ServiceDescriptor* MaidsafeService_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_maidsafe_5fservice_2eproto() {
  protobuf_AddDesc_maidsafe_5fservice_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "maidsafe_service.proto");
  GOOGLE_CHECK(file != NULL);
  MaidsafeService_descriptor_ = file->service(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_maidsafe_5fservice_2eproto);
}

void protobuf_RegisterTypes() {
  protobuf_AssignDescriptorsOnce();
}

}  // namespace

void protobuf_ShutdownFile_maidsafe_5fservice_2eproto() {
}

void protobuf_AddDesc_maidsafe_5fservice_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::maidsafe::protobuf_AddDesc_maidsafe_5fservice_5fmessages_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\026maidsafe_service.proto\022\010maidsafe\032\037maid"
    "safe_service_messages.proto2\271\005\n\017Maidsafe"
    "Service\022I\n\016StoreChunkPrep\022\032.maidsafe.Sto"
    "rePrepRequest\032\033.maidsafe.StorePrepRespon"
    "se\022A\n\010StoreIOU\022\031.maidsafe.StoreIOUReques"
    "t\032\032.maidsafe.StoreIOUResponse\022=\n\nStoreCh"
    "unk\022\026.maidsafe.StoreRequest\032\027.maidsafe.S"
    "toreResponse\022G\n\nCheckChunk\022\033.maidsafe.Ch"
    "eckChunkRequest\032\034.maidsafe.CheckChunkRes"
    "ponse\0222\n\003Get\022\024.maidsafe.GetRequest\032\025.mai"
    "dsafe.GetResponse\022;\n\006Update\022\027.maidsafe.U"
    "pdateRequest\032\030.maidsafe.UpdateResponse\022J"
    "\n\013GetMessages\022\034.maidsafe.GetMessagesRequ"
    "est\032\035.maidsafe.GetMessagesResponse\022;\n\006De"
    "lete\022\027.maidsafe.DeleteRequest\032\030.maidsafe"
    ".DeleteResponse\022P\n\rValidityCheck\022\036.maids"
    "afe.ValidityCheckRequest\032\037.maidsafe.Vali"
    "dityCheckResponse\022D\n\tSwapChunk\022\032.maidsaf"
    "e.SwapChunkRequest\032\033.maidsafe.SwapChunkR"
    "esponse", 767);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "maidsafe_service.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_maidsafe_5fservice_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_maidsafe_5fservice_2eproto {
  StaticDescriptorInitializer_maidsafe_5fservice_2eproto() {
    protobuf_AddDesc_maidsafe_5fservice_2eproto();
  }
} static_descriptor_initializer_maidsafe_5fservice_2eproto_;


// ===================================================================

MaidsafeService::~MaidsafeService() {}

const ::google::protobuf::ServiceDescriptor* MaidsafeService::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return MaidsafeService_descriptor_;
}

const ::google::protobuf::ServiceDescriptor* MaidsafeService::GetDescriptor() {
  protobuf_AssignDescriptorsOnce();
  return MaidsafeService_descriptor_;
}

void MaidsafeService::StoreChunkPrep(::google::protobuf::RpcController* controller,
                         const ::maidsafe::StorePrepRequest*,
                         ::maidsafe::StorePrepResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method StoreChunkPrep() not implemented.");
  done->Run();
}

void MaidsafeService::StoreIOU(::google::protobuf::RpcController* controller,
                         const ::maidsafe::StoreIOURequest*,
                         ::maidsafe::StoreIOUResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method StoreIOU() not implemented.");
  done->Run();
}

void MaidsafeService::StoreChunk(::google::protobuf::RpcController* controller,
                         const ::maidsafe::StoreRequest*,
                         ::maidsafe::StoreResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method StoreChunk() not implemented.");
  done->Run();
}

void MaidsafeService::CheckChunk(::google::protobuf::RpcController* controller,
                         const ::maidsafe::CheckChunkRequest*,
                         ::maidsafe::CheckChunkResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method CheckChunk() not implemented.");
  done->Run();
}

void MaidsafeService::Get(::google::protobuf::RpcController* controller,
                         const ::maidsafe::GetRequest*,
                         ::maidsafe::GetResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method Get() not implemented.");
  done->Run();
}

void MaidsafeService::Update(::google::protobuf::RpcController* controller,
                         const ::maidsafe::UpdateRequest*,
                         ::maidsafe::UpdateResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method Update() not implemented.");
  done->Run();
}

void MaidsafeService::GetMessages(::google::protobuf::RpcController* controller,
                         const ::maidsafe::GetMessagesRequest*,
                         ::maidsafe::GetMessagesResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method GetMessages() not implemented.");
  done->Run();
}

void MaidsafeService::Delete(::google::protobuf::RpcController* controller,
                         const ::maidsafe::DeleteRequest*,
                         ::maidsafe::DeleteResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method Delete() not implemented.");
  done->Run();
}

void MaidsafeService::ValidityCheck(::google::protobuf::RpcController* controller,
                         const ::maidsafe::ValidityCheckRequest*,
                         ::maidsafe::ValidityCheckResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method ValidityCheck() not implemented.");
  done->Run();
}

void MaidsafeService::SwapChunk(::google::protobuf::RpcController* controller,
                         const ::maidsafe::SwapChunkRequest*,
                         ::maidsafe::SwapChunkResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method SwapChunk() not implemented.");
  done->Run();
}

void MaidsafeService::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                             ::google::protobuf::RpcController* controller,
                             const ::google::protobuf::Message* request,
                             ::google::protobuf::Message* response,
                             ::google::protobuf::Closure* done) {
  GOOGLE_DCHECK_EQ(method->service(), MaidsafeService_descriptor_);
  switch(method->index()) {
    case 0:
      StoreChunkPrep(controller,
             ::google::protobuf::down_cast<const ::maidsafe::StorePrepRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::StorePrepResponse*>(response),
             done);
      break;
    case 1:
      StoreIOU(controller,
             ::google::protobuf::down_cast<const ::maidsafe::StoreIOURequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::StoreIOUResponse*>(response),
             done);
      break;
    case 2:
      StoreChunk(controller,
             ::google::protobuf::down_cast<const ::maidsafe::StoreRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::StoreResponse*>(response),
             done);
      break;
    case 3:
      CheckChunk(controller,
             ::google::protobuf::down_cast<const ::maidsafe::CheckChunkRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::CheckChunkResponse*>(response),
             done);
      break;
    case 4:
      Get(controller,
             ::google::protobuf::down_cast<const ::maidsafe::GetRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::GetResponse*>(response),
             done);
      break;
    case 5:
      Update(controller,
             ::google::protobuf::down_cast<const ::maidsafe::UpdateRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::UpdateResponse*>(response),
             done);
      break;
    case 6:
      GetMessages(controller,
             ::google::protobuf::down_cast<const ::maidsafe::GetMessagesRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::GetMessagesResponse*>(response),
             done);
      break;
    case 7:
      Delete(controller,
             ::google::protobuf::down_cast<const ::maidsafe::DeleteRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::DeleteResponse*>(response),
             done);
      break;
    case 8:
      ValidityCheck(controller,
             ::google::protobuf::down_cast<const ::maidsafe::ValidityCheckRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::ValidityCheckResponse*>(response),
             done);
      break;
    case 9:
      SwapChunk(controller,
             ::google::protobuf::down_cast<const ::maidsafe::SwapChunkRequest*>(request),
             ::google::protobuf::down_cast< ::maidsafe::SwapChunkResponse*>(response),
             done);
      break;
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      break;
  }
}

const ::google::protobuf::Message& MaidsafeService::GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const {
  GOOGLE_DCHECK_EQ(method->service(), descriptor());
  switch(method->index()) {
    case 0:
      return ::maidsafe::StorePrepRequest::default_instance();
    case 1:
      return ::maidsafe::StoreIOURequest::default_instance();
    case 2:
      return ::maidsafe::StoreRequest::default_instance();
    case 3:
      return ::maidsafe::CheckChunkRequest::default_instance();
    case 4:
      return ::maidsafe::GetRequest::default_instance();
    case 5:
      return ::maidsafe::UpdateRequest::default_instance();
    case 6:
      return ::maidsafe::GetMessagesRequest::default_instance();
    case 7:
      return ::maidsafe::DeleteRequest::default_instance();
    case 8:
      return ::maidsafe::ValidityCheckRequest::default_instance();
    case 9:
      return ::maidsafe::SwapChunkRequest::default_instance();
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      return *reinterpret_cast< ::google::protobuf::Message*>(NULL);
  }
}

const ::google::protobuf::Message& MaidsafeService::GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const {
  GOOGLE_DCHECK_EQ(method->service(), descriptor());
  switch(method->index()) {
    case 0:
      return ::maidsafe::StorePrepResponse::default_instance();
    case 1:
      return ::maidsafe::StoreIOUResponse::default_instance();
    case 2:
      return ::maidsafe::StoreResponse::default_instance();
    case 3:
      return ::maidsafe::CheckChunkResponse::default_instance();
    case 4:
      return ::maidsafe::GetResponse::default_instance();
    case 5:
      return ::maidsafe::UpdateResponse::default_instance();
    case 6:
      return ::maidsafe::GetMessagesResponse::default_instance();
    case 7:
      return ::maidsafe::DeleteResponse::default_instance();
    case 8:
      return ::maidsafe::ValidityCheckResponse::default_instance();
    case 9:
      return ::maidsafe::SwapChunkResponse::default_instance();
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      return *reinterpret_cast< ::google::protobuf::Message*>(NULL);
  }
}

MaidsafeService_Stub::MaidsafeService_Stub(::google::protobuf::RpcChannel* channel)
  : channel_(channel), owns_channel_(false) {}
MaidsafeService_Stub::MaidsafeService_Stub(
    ::google::protobuf::RpcChannel* channel,
    ::google::protobuf::Service::ChannelOwnership ownership)
  : channel_(channel),
    owns_channel_(ownership == ::google::protobuf::Service::STUB_OWNS_CHANNEL) {}
MaidsafeService_Stub::~MaidsafeService_Stub() {
  if (owns_channel_) delete channel_;
}

void MaidsafeService_Stub::StoreChunkPrep(::google::protobuf::RpcController* controller,
                              const ::maidsafe::StorePrepRequest* request,
                              ::maidsafe::StorePrepResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}
void MaidsafeService_Stub::StoreIOU(::google::protobuf::RpcController* controller,
                              const ::maidsafe::StoreIOURequest* request,
                              ::maidsafe::StoreIOUResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(1),
                       controller, request, response, done);
}
void MaidsafeService_Stub::StoreChunk(::google::protobuf::RpcController* controller,
                              const ::maidsafe::StoreRequest* request,
                              ::maidsafe::StoreResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(2),
                       controller, request, response, done);
}
void MaidsafeService_Stub::CheckChunk(::google::protobuf::RpcController* controller,
                              const ::maidsafe::CheckChunkRequest* request,
                              ::maidsafe::CheckChunkResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(3),
                       controller, request, response, done);
}
void MaidsafeService_Stub::Get(::google::protobuf::RpcController* controller,
                              const ::maidsafe::GetRequest* request,
                              ::maidsafe::GetResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(4),
                       controller, request, response, done);
}
void MaidsafeService_Stub::Update(::google::protobuf::RpcController* controller,
                              const ::maidsafe::UpdateRequest* request,
                              ::maidsafe::UpdateResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(5),
                       controller, request, response, done);
}
void MaidsafeService_Stub::GetMessages(::google::protobuf::RpcController* controller,
                              const ::maidsafe::GetMessagesRequest* request,
                              ::maidsafe::GetMessagesResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(6),
                       controller, request, response, done);
}
void MaidsafeService_Stub::Delete(::google::protobuf::RpcController* controller,
                              const ::maidsafe::DeleteRequest* request,
                              ::maidsafe::DeleteResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(7),
                       controller, request, response, done);
}
void MaidsafeService_Stub::ValidityCheck(::google::protobuf::RpcController* controller,
                              const ::maidsafe::ValidityCheckRequest* request,
                              ::maidsafe::ValidityCheckResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(8),
                       controller, request, response, done);
}
void MaidsafeService_Stub::SwapChunk(::google::protobuf::RpcController* controller,
                              const ::maidsafe::SwapChunkRequest* request,
                              ::maidsafe::SwapChunkResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(9),
                       controller, request, response, done);
}

}  // namespace maidsafe
