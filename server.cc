#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdio>
#include <fcntl.h>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include "interface.h"

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "keyval.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using keyval::DelReply;
using keyval::DelRequest;
using keyval::GetReply;
using keyval::GetRequest;
using keyval::KeyVal;
using keyval::PutReply;
using keyval::PutRequest;
using namespace std;

int num_threads;
std::string port;
std::string cache_type;
int cache_size;
int cache_block;

struct CallData
{
  KeyVal::AsyncService *service;
  grpc::ServerCompletionQueue *cq;
};

class Call
{
public:
  virtual void Proceed(bool ok) = 0;
};

class GetCall final : public Call
{
public:
  explicit GetCall(CallData *data)
      : data_(data), responder_(&ctx_), status_(REQUEST)
  {
    on_done = [&](bool ok)
    { OnDone(ok); };
    proceed = [&](bool ok)
    { Proceed(ok); };
    ctx_.AsyncNotifyWhenDone(&on_done);
    data_->service->RequestGET(&ctx_, &request_, &responder_, data_->cq,
                               data_->cq, &proceed);
  }

  void Proceed(bool ok)
  {
    switch (status_)
    {
    case REQUEST:
      if (!ok)
      {
        // Not ok in REQUEST means the server has been Shutdown
        // before the call got matched to an incoming RPC.
        delete this;
        break;
      }
      new GetCall(data_);
      //Impliment here
      //Start
      result = get12(request_.key());
      if(result=="4")
      {
        response_.set_status(400);
        response_.set_val("key or value is empty");
      }
      else if (result=="2")
      {
        response_.set_status(400);
        response_.set_val("key not found");
      }
      else if(result=="3")
      {
        response_.set_status(400);
        response_.set_val("key or value string byte is more than 256 byte");
      }
      else
      {
        response_.set_status(200);
        response_.set_val(result);
      }
      
      //End
      responder_.Finish(response_, grpc::Status::OK, &proceed);
      status_ = FINISH;
      break;

    case FINISH:
      finish_called_ = true;
      if (on_done_called_)
        delete this;
      break;
    }
  }

  void OnDone(bool ok)
  {
    assert(ok);
    if (ctx_.IsCancelled())
      std::cerr << ": Ping call cancelled" << std::endl;
    on_done_called_ = true;
    if (finish_called_)
      delete this;
    else
      status_ = FINISH;
  }

  std::function<void(bool)> proceed;
  std::function<void(bool)> on_done;

private:
  std::string result;
  CallData *data_;
  grpc::ServerContext ctx_;
  GetRequest request_;
  GetReply response_;
  ServerAsyncResponseWriter<GetReply> responder_;
  enum CallStatus
  {
    REQUEST,
    FINISH
  };
  CallStatus status_;
  bool finish_called_ = false;
  bool on_done_called_ = false;
};

class DelCall final : public Call
{
public:
  explicit DelCall(CallData *data)
      : data_(data), responder_(&ctx_), status_(REQUEST)
  {
    on_done = [&](bool ok)
    { OnDone(ok); };
    proceed = [&](bool ok)
    { Proceed(ok); };
    ctx_.AsyncNotifyWhenDone(&on_done);
    data_->service->RequestDEL(&ctx_, &request_, &responder_, data_->cq,
                               data_->cq, &proceed);
  }

  void Proceed(bool ok)
  {
    switch (status_)
    {
    case REQUEST:
      if (!ok)
      {
        // Not ok in REQUEST means the server has been Shutdown
        // before the call got matched to an incoming RPC.
        delete this;
        break;
      }
      new DelCall(data_);
      //Impliment here
      //Start
      result = delete1(request_.key());
      if(result==1)
      {
        response_.set_status(200);
        response_.set_err("Entry Deleted");
      }
      else if (result==2)
      {
        response_.set_status(400);
        response_.set_err("key not found");
      }
      else if(result==3)
      {
        response_.set_status(400);
        response_.set_err("key or value string byte is more than 256 byte");
      }
      else
      {
        response_.set_status(400);
        response_.set_err("key or value is empty");
      }
      //End
      responder_.Finish(response_, grpc::Status::OK, &proceed);
      status_ = FINISH;
      break;

    case FINISH:
      finish_called_ = true;
      if (on_done_called_)
        delete this;
      break;
    }
  }

  void OnDone(bool ok)
  {
    assert(ok);
    if (ctx_.IsCancelled())
      std::cerr << ": Ping call cancelled" << std::endl;
    on_done_called_ = true;
    if (finish_called_)
      delete this;
    else
      status_ = FINISH;
  }

  std::function<void(bool)> proceed;
  std::function<void(bool)> on_done;

private:
  int result;
  CallData *data_;
  grpc::ServerContext ctx_;
  DelRequest request_;
  DelReply response_;
  ServerAsyncResponseWriter<DelReply> responder_;
  enum CallStatus
  {
    REQUEST,
    FINISH
  };
  CallStatus status_;
  bool finish_called_ = false;
  bool on_done_called_ = false;
};

class PutCall final : public Call
{
public:
  explicit PutCall(CallData *data)
      : data_(data), responder_(&ctx_), status_(REQUEST)
  {
    on_done = [&](bool ok)
    { OnDone(ok); };
    proceed = [&](bool ok)
    { Proceed(ok); };
    ctx_.AsyncNotifyWhenDone(&on_done);
    data_->service->RequestPUT(&ctx_, &request_, &responder_, data_->cq,
                               data_->cq, &proceed);
  }

  void Proceed(bool ok)
  {
    switch (status_)
    {
    case REQUEST:
    {
      if (!ok)
      {
        // Not ok in REQUEST means the server has been Shutdown
        // before the call got matched to an incoming RPC.
        delete this;
        break;
      }
      new PutCall(data_);
      //Impliment here
      //Start
      result = put(request_.key(),request_.val());
      response_.set_status(200);
      //End
      responder_.Finish(response_, grpc::Status::OK, &proceed);
      status_ = FINISH;
    }
    break;

    case FINISH:
      finish_called_ = true;
      if (on_done_called_)
        delete this;
      break;
    }
  }

  void OnDone(bool ok)
  {
    assert(ok);
    if (ctx_.IsCancelled())
      std::cerr << ": Ping call cancelled" << std::endl;
    on_done_called_ = true;
    if (finish_called_)
      delete this;
    else
      status_ = FINISH;
  }

  std::function<void(bool)> proceed;
  std::function<void(bool)> on_done;

private:
  CallData *data_;
  int result;
  std::string key;
  std::string value;
  grpc::ServerContext ctx_;
  PutRequest request_;
  PutReply response_;
  ServerAsyncResponseWriter<PutReply> responder_;
  enum CallStatus
  {
    REQUEST,
    FINISH
  };
  CallStatus status_;
  bool finish_called_ = false;
  bool on_done_called_ = false;
};
void task1(string msg);
class ServerImpl final
{
public:
  ~ServerImpl()
  {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    for (int i = 0; i < num_threads; i++)
      cqs_[i]->Shutdown();
    migrate_cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run()
  {
    std::string server_address("0.0.0.0:" + port);

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    for (int i = 0; i < num_threads; i++)
      cqs_.emplace_back(builder.AddCompletionQueue());
    migrate_cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }
  void HandleRpcs()
  {
    // Spawn a new CallData instance to serve new clients.
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++)
    {
      CallData data{&service_, cqs_[i].get()};
      new GetCall(&data);
      new PutCall(&data);
      new DelCall(&data);
      threads.emplace_back(std::thread(&ServerImpl::ServeThread, this, i));
    }
    CallData migrate_data{&service_, migrate_cq_.get()};
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(migrate_cq_->Next(&tag, &ok));
      auto proceed = static_cast<std::function<void(bool)> *>(tag);
      (*proceed)(ok);
    }
    server_->Shutdown(std::chrono::system_clock::now());
    for (auto thr = threads.begin(); thr != threads.end(); thr++)
      thr->join();
  }

  void ServeThread(int i)
  {
    void *tag;
    bool ok;
    while (cqs_[i]->Next(&tag, &ok))
    {
      auto proceed = static_cast<std::function<void(bool)> *>(tag);
      (*proceed)(ok);
    }
  }

  std::vector<std::unique_ptr<ServerCompletionQueue>> cqs_;
  std::unique_ptr<ServerCompletionQueue> migrate_cq_;
  KeyVal::AsyncService service_;
  std::unique_ptr<Server> server_;
};

void config()
{
  std::ifstream cFile("config.txt");
  if (cFile.is_open())
  {
    std::string line;
    while (getline(cFile, line))
    {
      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);
      if (name == "LISTENING_PORT")
      {
        port = value;
      }
      else if (name == "CACHE_REPLACEMENT_TYPE")
      {
        cache_type = value;
      }
      else if (name == "CACHE_SIZE")
      {
        cache_size = stoi(value);
      }
      else if (name == "THREAD_POOL_SIZE")
      {
        num_threads = stoi(value);
      }
      else
      {
        std::cerr << "Property is not valid.\n";
      }
    }
  }
  else
  {
    std::cerr << "Couldn't open config file for reading.\n";
  }
}
int main(int argc, char **argv)
{
  freopen("log.txt", "w", stderr);
  config();
  run();
  cache_block = cache_size / 512;
  if(cache_type=="LFU")
  {
    config_cache(1,cache_block);
  }
  else
  {
    config_cache(2,cache_block);
  }
  ServerImpl server;
  server.Run();
  return 0;
}
