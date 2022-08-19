#include <chrono>
#include <thread>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <map>
#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>

std::string get12(std::string key);
int delete1(std::string key);
int put(std::string key, std::string value);
void config_cache(int cachetype,int size);
void run();