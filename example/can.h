/*
    POD数据可以直接使用c语言的mem函数
*/
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef CAN
#define CAN
#include <QString>
#pragma pack(1)

struct CMD_XXXXX
{
    uchar a;
    uchar b:4;
    uchar c:1;
    uchar :3;
    uchar :8;
    uchar :8;
    uchar :8;
    uchar :8;
    ushort h;

};

#pragma pack()

#endif
