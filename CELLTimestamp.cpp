#include"stdAfx.h"
#include "CELLTimestamp.hpp"


CELLTimestamp::CELLTimestamp()
{
	update();
}

CELLTimestamp::~CELLTimestamp()
{
}

double CELLTimestamp::getElapsedTimeSecond()
{
	return getElapsedTimeInMicroSec() * 0.000001;
}

double CELLTimestamp::getElapsedTimeMilliSec()
{
	return getElapsedTimeInMicroSec() * 0.001;
}

long long CELLTimestamp::getElapsedTimeInMicroSec()
{
	return  duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
}

void CELLTimestamp::update()
{
	_begin = high_resolution_clock::now();
}


time_t CELLTimestamp::getCurrentTimeStampSec()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
}

int64_t CELLTimestamp::getCurrentTimeStampMiliSec()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string CELLTimestamp::getTimeStampToNomalTime(time_t timeStamp)
{
    struct tm *nomalTime;
    nomalTime = localtime(&timeStamp); /*转换为struct tm结构的本地时间*/

    // std::cout <<"test: ";
    // std::cout << nomalTime->tm_mon << std::endl;

    std::string strNomolTime = std::to_string(nomalTime->tm_year + 1900) + "/" +std::to_string(nomalTime->tm_mon+1) + "/"
    +   std::to_string(nomalTime->tm_mday) + " " /*+ wDays[nomalTime->tm_wday]*/ + " "
    +   std::to_string(nomalTime->tm_hour) + ":" + std::to_string(nomalTime->tm_min) + ":" + std::to_string(nomalTime->tm_sec);

    return strNomolTime;
}

std::string CELLTimestamp::getCurrentNomalTime()
{
    return getTimeStampToNomalTime(getCurrentTimeStampSec());
}
