#pragma once
#include<cstdio>

#define LOG_INFO(fmt,...) printf("%s:%d% %s",fmt,"\n",__FILE__,__LINE__,__func__,##__AV_ARGS__) 