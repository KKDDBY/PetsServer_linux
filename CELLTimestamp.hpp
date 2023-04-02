//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   CELLTimestamp.h
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-12
//FileDescription	:	定时器

//模块说明：定时器,高精度的，可以定制定时任务
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CELLTIMESTAMP
#define CELLTIMESTAMP

#include <chrono>
#include <string>
#include <iostream>

using namespace std::chrono;	//使用时间标准库

class CELLTimestamp
{
public:
	CELLTimestamp();
	~CELLTimestamp();
	double getElapsedTimeMilliSec();
	double getElapsedTimeSecond();
	long long getElapsedTimeInMicroSec();
    time_t getCurrentTimeStampSec();       							//获取当前时间戳（1970年01月01日00时00分00秒(北京时间1970年01月01日08时00分00秒)起至现在的总秒数）
    int64_t getCurrentTimeStampMiliSec();       					//获取当前时间戳（1970年01月01日00时00分00秒(北京时间1970年01月01日08时00分00秒)起至现在的总毫秒数）

    std::string getTimeStampToNomalTime(time_t timeStamp);  //将时间戳转换成时钟时间
    std::string getCurrentNomalTime();  					//获取当前日期时间
	void update();

private:
	time_point<high_resolution_clock> _begin;	//高分辨率时间点 high_resolution_clock表示高分辨率达到微妙的 
};



#endif // !CELLTIMESTAMP

