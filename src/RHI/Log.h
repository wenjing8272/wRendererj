#pragma once
#include<cstdio>

#define LOG_INFO(...) std::printf(__VA_ARGS__) 
#define LOG_WARN(...) std::fprintf(stderr,"warn: " __VA_ARGS__)