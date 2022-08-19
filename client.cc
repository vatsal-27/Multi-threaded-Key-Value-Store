/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include<fstream>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <sys/time.h>
#include <chrono>
#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "keyval.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using keyval::KeyVal;
using keyval::GetRequest;
using keyval::GetReply;
using keyval::PutRequest;
using keyval::PutReply;
using keyval::DelRequest;
using keyval::DelReply;

class KeyValClient {
 public:
  explicit KeyValClient(std::shared_ptr<Channel> channel)
      : stub_(KeyVal::NewStub(channel)) {}

  // Assembles the client's payload and sends it to the server.
  void GET(const std::string& user) {
    // Data we are sending to the server.
    GetRequest request;
    request.set_key(user);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;
    call->type=0;
    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->response_reader =
        stub_->PrepareAsyncGET(&call->context, request, &cq_);
    gettimeofday(&call->time, 0);
    // StartCall initiates the RPC call
    call->response_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void PUT(const std::string& key,const std::string& val) {
    // Data we are sending to the server.
    PutRequest request;
    request.set_key(key);
    request.set_val(val);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;
    call->type=1;
    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->response_reader_put =
        stub_->PrepareAsyncPUT(&call->context, request, &cq_);

    // StartCall initiates the RPC call
    gettimeofday(&call->time, 0);
    call->response_reader_put->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader_put->Finish(&call->reply_put, &call->status_put, (void*)call);
  }

  void DEL(const std::string& key) {
    // Data we are sending to the server.
    DelRequest request;
    request.set_key(key);
    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->type=2;
    call->response_reader_del =
        stub_->PrepareAsyncDEL(&call->context, request, &cq_);

    // StartCall initiates the RPC call
    gettimeofday(&call->time, 0);
    call->response_reader_del->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader_del->Finish(&call->reply_del, &call->status_del, (void*)call);
  }
  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);
      struct timeval end;
      gettimeofday(&end, 0);
      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);
      if(call->type==0){
        if (call->status.ok())
        {
          if(call->reply.status()==200)
          std::cout << "Get received: " << call->reply.val() << std::endl;
        }
        else
          std::cout << call->reply.val() << std::endl;
      }
      else if(call->type==1){
        if (call->status.ok())
        {
          if(call->reply_put.status()==200)
          std::cout << "Put complete" << std::endl;
        }
        else
          std::cout << call->reply_put.err() << std::endl;
      }
      else{
        if (call->status.ok())
        {
          if(call->reply_del.status()==200)
          std::cout << "Del complete" << std::endl;
        }
        else
          std::cout << call->reply_del.err() << std::endl;
      }
      // Once we're complete, deallocate the call object.
      float interv=(end.tv_sec - call->time.tv_sec)+ (end.tv_usec - call->time.tv_usec)*1e-6;
      std::cout << "Time elapsed for this call is "<< interv << " secs \n";
      delete call;
    }
  }

 private:
  // struct for keeping state and data information
  struct AsyncClientCall {
    // Container for the data we expect from the server.
    int type;
    struct timeval time;
    GetReply reply;
    PutReply reply_put;
    DelReply reply_del;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Storage for the status of the RPC upon completion.
    Status status;
    Status status_put;
    Status status_del;

    std::unique_ptr<ClientAsyncResponseReader<GetReply>> response_reader;
    std::unique_ptr<ClientAsyncResponseReader<PutReply>> response_reader_put;
    std::unique_ptr<ClientAsyncResponseReader<DelReply>> response_reader_del;
  };

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<KeyVal::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
};
void handlebatchmode(std::string filename,KeyValClient *kv){
    std::fstream fin;
    fin.open(filename,std::ios::in);
    std::string line;
    std::string command;
    std::string key;
    std::string value;
    if(fin.is_open()){

    while(getline(fin,line))
    {   
        std::stringstream ss(line);
        ss>>command;
        ss>>key;
        if(command.compare("PUT")==0){
          ss>>value;
        
          kv->PUT(key,value);
        }
        else if(command.compare("DEL")==0){
          kv->DEL(key);
        }
        else if(command.compare("GET")==0){
          kv->GET(key);
        }
        else{
          std::cout << "Invalid command in file please check and rerun";
          exit(-1);
        }
        
    }
    }
    else{
      std::cout << "Error opening file\n";
      exit(-1);
    }
    fin.close();
}
int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  KeyValClient kv(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  // Spawn reader thread that loops indefinitely
  std::thread thread_ = std::thread(&KeyValClient::AsyncCompleteRpc, &kv);

  // for (int i = 0; i < 10; i++) {
  //   std::string user("world " + std::to_string(i));
  //   kv.GET(user);  // The actual RPC call!
  //   kv.PUT("abc","xyz");
  // }
  //looping infinetely
  int mode;
  std::string command;
  std::string key;
  std::string value;
  std::string filename;

  while (1)
  {
    std::cout << "Enter 1 for Batch mode and 2 for Interactive mode: \n";
    std::cout << "Press control-c to quit" << std::endl << std::endl;
    std::cin >> mode;
    if(mode==1){
      std::cout << "Enter filename: ";
      std::cin >> filename;
      handlebatchmode(filename,&kv);
    }
    else if(mode==2){
      std::cout << "Enter Command (e.g GET, PUT, DEL): ";
      std::cin >> command;
      if(command.compare("DEL")==0){
        std::cout << "Enter Key :\n";
        std::cin >> key;
        kv.DEL(key);
      }
      else if(command.compare("PUT")==0){
        std::cout << "Enter Key & Value: ";
        std::cin>>key>>value;
        kv.PUT(key,value);

      }
      else if(command.compare("GET")==0){
        std::cout << "Enter Key: ";
        std::cin>>key;
        kv.GET(key);
      }
      else{
        std::cout<< "Invalid Command\n";
        exit(-1);
      }
    }
    else{
      std::cout << "Invalid please Enter valid\n";
    }
  }
  
  std::cout << "Press control-c to quit" << std::endl << std::endl;
  thread_.join();  // blocks forever

  return 0;
}
