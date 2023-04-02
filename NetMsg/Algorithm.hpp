//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   Algorithm.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-12
//FileDescription	:	功能性算法定义

//模块说明：提供给服务器的一些功能性函数封装(编码转换，随机数生成等等)

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//============================字符转换相关功能函数============================BEGIN

#include<cstdlib>
#include<time.h>
#include<string>

#include"Define.hpp"


namespace KKDD
{
//随机字符串生成
void srand_str(string& nonce, size_t len)
{
	srand(time(0));
	int ch, type;
	for (size_t i = 0; i < len; i++)
	{
		char s[2] = { 0 };
		type = rand() % 3;
		if (type == 0)//判断随机类型生成大小写或者字母
		{
			ch = rand() % ('Z' - 'A' + 1) + 'A';
		}
		else if (type == 1) {
			ch = rand() % ('z' - 'a' + 1) + 'a';
		}
		else if (type == 2) {
			ch = rand() % ('9' - '0' + 1) + '0';
		}
		sprintf(s, "%c", ch);
		nonce.append(s);
	}
}


}; // namespace KKDD

