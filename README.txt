Team members
1. Jaimin Chauhan
2. Rupasai Rangaraju
3. Vatsal Dani

Run:
1. Change the path in CMakeLists.txt. In that change line 8 as "include(/Path/To/Grpc/grpc/examples/cpp/cmake/common.cmake)"
2. mkdir cmake/build
3. cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
4. make
We assumes that the environment variable MY_INSTALL_DIR holds grpc directory path.

For log do like this after perfoming "make"

export GRPC_VERBOSITY=DEBUG

Ref:

https://grpc.github.io/grpc/cpp/md_doc_environment_variables.html
https://isocpp.org/
https://grpc.io/docs/languages/cpp/quickstart/
