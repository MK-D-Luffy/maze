#include <reg52.h>
#include <intrins.h>

typedef unsigned char uchar;
typedef unsigned int uint;

uint i, j, k;
uchar f, s; //第一个，第二个
int l = -1, r = 1, b = 0;
int last = 0;

sfr P4 = 0xe8;
sbit A0 = P4 ^ 0;
sbit A1 = P2 ^ 0;
sbit A2 = P2 ^ 7;

// 接收到红外光是低电平
sbit irR1_C = P2 ^ 1; //前
sbit irR2_LU = P2 ^ 2;
sbit irR3_L = P2 ^ 3;
sbit irR4_R = P2 ^ 4;
sbit irR5_RU = P2 ^ 5;

int irL = 0, irLU = 0, irC = 0, irRU = 0, irR = 0;

sbit BEEP = P3 ^ 7;
sbit tube1 = P4 ^ 3;
sbit tube2 = P4 ^ 2;

uchar code table[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};
//右转的顺序
uchar code turn[] = {
    0x11,
    0x33,
    0x22,
    0x66,
    0x44,
    0xcc,
    0x88,
    0x99,
};
uchar code straight[] = {
    0x11,
    0x93,
    0x82,
    0xc6,
    0x44,
    0x6c,
    0x28,
    0x39,
};

void MOUSE_IR_ON(GROUP_NO)
{
    A0 = (GROUP_NO)&0x01;
    A1 = (GROUP_NO)&0x02;
    A2 = (GROUP_NO)&0x04;
}

void delay_ms(uint ms)
{
    while (ms--)
    {
        for (k = 110; k > 0; k--)
            ;
    }
}

void display(uint num)
{
    f = num / 10;
    s = num % 10;

    tube1 = 0;
    tube2 = 1;
    P0 = table[f];
    delay_ms(5);

    tube1 = 1;
    tube2 = 0;
    P0 = table[s];
    delay_ms(5);
}
// left -1 right 1
void turn_aroud(int loc)
{
    for (j = 0; j < 51; j++)
    {
        //左转
        if (loc == -1)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = turn[8 - i];
                delay_ms(3);
            }
        }
        //右转
        else if (loc == 1)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = turn[i];
                delay_ms(3);
            }
        }
        //向后转
        else if (loc == 0)
        {
            for (i = 0; i < 8; i++)
            {
                P1 = turn[i];
                delay_ms(3);
            }
            for (i = 0; i < 8; i++)
            {
                P1 = turn[i];
                delay_ms(3);
            }
        }
    }
}

void go_straight()
{
    for (j = 0; j < 100; j++)
    {
        for (i = 0; i < 8; i++)
        {
            P1 = straight[i];
            delay_ms(3);
        }
    }
}

// irC=0 有墙
int move(int irC, int irL, int irLU, int irR, int inRU)
{

    // 如果前面没有障碍就直走
    if (!irC)
    {
        go_straight();
    }
    else
    {
        // 如果前面每个方位都有障碍,则往后转
        // 当irC,irL,irR已经都赋值时向后转
        if (irL & irR)
        {
            turn_aroud(b);
            display(44);
            delay_ms(1500);
            return 0;
        }
        // 如果前面和左边有墙且右边没墙时就往右转
        if (irL & !irR)
        {
            turn_aroud(r);
            display(33);
            delay_ms(1500);
            return 0;
        }
        // 如果前面和右边有墙且左边没墙时就往左转
        if (!irL & irR)
        {
            turn_aroud(l);
            display(22);
            delay_ms(1500);
            return 0;
        }
        // 如果前面有墙,左右没墙就右转
        if (!irL & !irR)
        {
            display(11);
            delay_ms(1500);
            turn_aroud(r);
        }
    }
}

void detector()
{
    MOUSE_IR_ON(0);
    delay_ms(5);
    if (irR1_C) //无墙
    {
        irC = 0;
        display(last);
        //BEEP = 1;
    }
    else //有墙
    {
        irC = 1;
        last = irC;
        display(last);
        //BEEP = 0;
    }

    MOUSE_IR_ON(1);
    delay_ms(5);
    if (irR2_LU) //无墙
    {
        irLU = 0;
        display(last);
        //BEEP = 1;
    }
    else //有墙
    {
        irLU = 2;
        last = irLU;
        display(last);
        //BEEP = 0;
    }

    MOUSE_IR_ON(2);
    delay_ms(5);
    if (irR3_L) //无墙
    {
        irL = 0;
        display(last);
        //BEEP = 1;
    }
    else //有墙
    {
        irL = 3;
        last = irL;
        display(last);
        //BEEP = 0;
    }

    MOUSE_IR_ON(3);
    delay_ms(5);
    if (irR4_R) //无墙
    {
        irR = 0;
        display(last);
        //BEEP = 1;
    }
    else //有墙
    {
        irR = 4;
        last = irR;
        display(last);
        //BEEP = 0;
    }

    MOUSE_IR_ON(4);
    delay_ms(5);
    if (irR5_RU) //无墙
    {
        irRU = 0;
        display(last);
        //BEEP = 1;
    }
    else //有墙
    {
        irRU = 5;
        last = irRU;
        display(last);
        //BEEP = 0;
    }
    MOUSE_IR_ON(7);
    delay_ms(30);
    move(irC, irL, irLU, irR, irRU);
}

void main()
{
    while (1)
    {
        detector();
    }
}
