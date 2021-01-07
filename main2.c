#include <reg52.h>
#define MOUSE_IR_ON(GROUP_NO) \
    do                        \
    {                         \
        A0 = (GROUP_NO)&0x01; \
        A1 = (GROUP_NO)&0x02; \
        A2 = (GROUP_NO)&0x04; \
    } while (0)

typedef unsigned char uchar;
typedef unsigned int uint;

sfr P4 = 0xe8;
sbit BEEP = P3 ^ 7;
sbit A0 = P4 ^ 0;
sbit A1 = P2 ^ 0;
sbit A2 = P2 ^ 7;
sbit irR1_C = P2 ^ 1;
sbit irR2_LU = P2 ^ 2;
sbit irR3_L = P2 ^ 3;
sbit irR4_R = P2 ^ 4;
sbit irR5_RU = P2 ^ 5;
sbit tube1 = P4 ^ 3;
sbit tube2 = P4 ^ 2;
uchar irL = 0, irLU = 0, irC = 0, irRU = 0, irR = 0;
uchar code table[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};
uchar code left[] = {0x11, 0x99, 0x88, 0xcc, 0x44, 0x66, 0x22, 0x33};
uchar code right[] = {0x11, 0x33, 0x22, 0x66, 0x44, 0xcc, 0x88, 0x99};
uchar code straight[] = {0x11, 0x93, 0x82, 0xc6, 0x44, 0x6c, 0x28, 0x39};

uchar i, j, k, t;
uchar y_axis = 0, x_axis = 1, y_axis_r = 2, x_axis_r = 3;
uchar x = 0, y = 0, returnbegin = 0;
uchar axis = 0;
uchar fis, sec; //第一个，第二个
//记录可行路线的个数
uchar num = 0, flag = 0;

//记录迷宫信息默认初始化有墙
//迷宫规则
//高四位记录来的方向
//低四位记录当前墙的有无
uchar Maze[8][8];
//对应方位为
//      上(0)
//左(3)  车    右(1)
//      下(2)
uchar f = 0, r = 1, b = 2, l = 3;
uchar oneWay = 0;

void initMaze()
{
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
            Maze[i][j] = 0xff;
        }
    }
}

void delay_ms(uint ms)
{
    while (ms--)
    {
        for (t = 0; t < 110; t++)
            ;
    }
}

void display(uint fis, uint sec)
{

    tube1 = 1;
    tube2 = 0;
    P0 = table[fis];
    delay_ms(5);

    tube1 = 0;
    tube2 = 1;
    P0 = table[sec];
    delay_ms(5);
}

void initTime2(uint us)
{
    //T2中断允许
    EA = 1;
    ET2 = 1;
    TH2 = RCAP2H = (65536 - us) / 256;
    TL2 = RCAP2L = (65536 - us) % 256;
    TR2 = 1;
}

void time2() interrupt 5
{
    static uint ir = 0, lastIr = 1;
    TF2 = 0; //中断溢出标志位,溢出时设置TF2为1,并请求中断
    if (ir != lastIr)
    {
        MOUSE_IR_ON(ir % 5);
        ir++;
    }
    else
    {
        lastIr++;
        switch ((ir - 1) % 5)
        {
        case 0:
            irC = irR1_C == 0 ? 1 : 0;
            break;
        case 1:
            irLU = irR2_LU == 0 ? 2 : 0;
            break;
        case 2:
            irL = irR3_L == 0 ? 3 : 0;
            break;
        case 3:
            irR = irR4_R == 0 ? 4 : 0;
            break;
        case 4:
            irRU = irR5_RU == 0 ? 5 : 0;
            break;
        }
    }
}

void amend()
{
    if (!irC)
    {
        if (irLU)
        {
            display(2, 2);
            for (i = 0; i < 8; i++)
            {
                P1 = (right[i]);
                delay_ms(2);
            }
            // }
        }
        if (irRU)
        {
            display(5, 5);
            for (i = 0; i < 8; i++)
            {
                P1 = (left[i]);
                delay_ms(2);
            }
        }
    }
}

// left -1 right 1
void turn_aroud(uchar loc)
{
    //左转
    if (loc == 3)
    {
        for (j = 0; j < 51; j++)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = left[i];
                delay_ms(1);
            }
        }
        axis = (axis + 3) % 4;
    }
    //右转
    else if (loc == 1)
    {
        for (j = 0; j < 51; j++)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = right[i];
                delay_ms(1);
            }
        }
        axis = (axis + 1) % 4;
    }
    //向后转
    else if (loc == 2)
    {
        for (j = 0; j < 51; j++)
        {
            for (i = 0; i < 16; i++)
            {
                P1 = right[i % 8];
                delay_ms(1);
            }
        }
        axis = (axis + 2) % 4;
    }
}

void go_straight()
{
    j = 0;
    // j=106
    while (j < 106)
    {
        for (i = 0; i < 8; i++)
        {
            P1 = straight[i];
            delay_ms(1);
        }
        amend();
        j++;
    }

    if (axis == x_axis)
        x++;
    else if (axis == y_axis)
        y++;
    else if (axis == x_axis_r)
        x--;
    else if (axis == y_axis_r)
        y--;
}

//传入小车的相对方向(判断小车的该相对反向能否前进)
uint judgeLoc(uint pos)
{
    uint tmp;
    //转弯后的绝对方向 =（转弯前的绝对方向(axis) + 转弯数值(pos)）%4
    pos = (axis + pos) % 4;
    switch (pos)
    {
    case 0:
        return tmp = (y != 7 && Maze[x][y + 1] == 0xff) ? 1 : 0;
    case 1:
        return tmp = (x != 7 && Maze[x + 1][y] == 0xff) ? 1 : 0;
    case 2:
        return tmp = (y != 0 && Maze[x][y - 1] == 0xff) ? 1 : 0;
    case 3:
        return tmp = (x != 0 && Maze[x - 1][y] == 0xff) ? 1 : 0;
    }
}

void save()
{
    num = 0;
    //记录当前位置有几条路可以走
    if (!irL && judgeLoc(l))
    {
        num++;
        oneWay = 3;
    }
    if (!irR && judgeLoc(r))
    {
        num++;
        oneWay = 1;
    }
    if (!irC && judgeLoc(f))
    {
        num++;
        oneWay = 0;
    }

    if (Maze[x][y] == 0xff)
    {
        // 记录低四位
        Maze[x][y] = Maze[x][y] & 0xf0;
        if (irC)
            Maze[x][y] = Maze[x][y] | 0xf8;
        if (irR)
            Maze[x][y] = Maze[x][y] | 0xf4;
        if (irL)
            Maze[x][y] = Maze[x][y] | 0xf1;

        //记录高四位
        if (axis == 0)
            //res:0111
            Maze[x][y] = Maze[x][y] & 0x7f;
        if (axis == 1)
            //res:1011
            Maze[x][y] = Maze[x][y] & 0xbf;
        if (axis == 2)
            //res:1101
            Maze[x][y] = Maze[x][y] & 0xdf;
        if (axis == 3)
            //res:1110
            Maze[x][y] = Maze[x][y] & 0xef;
    }
}

int move()
{
    if (x == 0 && y == 0 && returnbegin == 0)
    {
        returnbegin = 1;
        go_straight();
    }
    if (returnbegin == 1)
    {
        if (x == 7 && y == 7)
        {
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
        }

        if (x == 0 && y == 0)
        {
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
            delay_ms(800);
            turn_aroud();
            returnbegin = 0;
        }

        if (!irR)
        {
            turn_aroud(r);
            go_straight();
        }
        else
        {
            if (!irC)
            {
                go_straight();
            }
            else
            {
                if (!irL)
                {
                    turn_aroud(l);
                    go_straight();
                }
                else
                {
                    turn_aroud(b);
                    go_straight();
                }
            }
        }
    }
    else if (returnbegin == 0)
    {
        if (x == 7 && y == 7)
        {
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
            BEEP = 0;
            delay_ms(150);
            BEEP = 1;
        }
        if (x == 0 && y == 0 && flag == 1 && num == 0)
        {
            BEEP = 0;
            delay_ms(200);
            BEEP = 1;
            BEEP = 0;
            delay_ms(200);
            BEEP = 1;
            delay_ms(100000);
        }
        if (x == 0 && y == 0 && flag == 0)
            flag = 1;
        //记录电脑鼠所在迷宫格的挡板信息和电脑鼠来的方向
        save();
        //有两条以上的路可以走
        display(x, y);
        if (num >= 2)
        {
            BEEP = 0;
            delay_ms(75);
            BEEP = 1;
            //对应策略选择路径
            if (!irR && judgeLoc(r))
            {
                turn_aroud(r);
                go_straight();
            }
            else
            {
                if (!irC && judgeLoc(f))
                {
                    go_straight();
                }
                else
                {
                    if (!irL && judgeLoc(l))
                    {
                        turn_aroud(l);
                        go_straight();
                    }
                }
            }
        }
        else if (num == 1)
        {
            //选择唯一路径
            if (oneWay == 0)
                go_straight();
            else
            {
                turn_aroud(oneWay);
                go_straight();
            }
        }
        //无路可走
        else if (num == 0)
        {

            //回退一格
            //pos:进入的方向
            //axis:车头的方向
            uchar pos1 = 0;
            uchar pos2 = 0;
            //回退一格
            //1.找到进入的绝对方向
            if (Maze[x][y] >> 4 == 0xd)
                pos1 = 0;
            else if (Maze[x][y] >> 4 == 0xe)
                pos1 = 1;
            else if (Maze[x][y] >> 4 == 0x7)
                pos1 = 2;
            else if (Maze[x][y] >> 4 == 0xb)
                pos1 = 3;
            //需要的相对方向=(之前来的绝对方向(pos)+4-现在的绝对方向)%4
            //TODO pos
            pos2 = (pos1 + 4 - axis) % 4;
            if (pos2 == 0)
            {
                go_straight();
            }
            else
            {
                turn_aroud(pos2);
                go_straight();
            }
        }
    }
}
void main()
{
    initTime2(5000);
    initMaze();
    while (1)
    {
        delay_ms(50);
        move();
    }
}
