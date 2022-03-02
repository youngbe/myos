#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <terminal_io.h>

static void move_up();
static size_t pos=0;


void clear_t()
{
    size_t* temp=(size_t *)0xb8000;
    while ( true )
    {
        *(volatile size_t *)temp=0x0700070007000700;
        if ( temp >= (size_t *)(0xb8000+80*25*2-sizeof(size_t)) )
        {
            break;
        }
        ++temp;
    }
    pos=0;
}

void putchar_t(char c)
{
    if ( c == '\0' )
    {
    }
    else if ( c == '\n' )
    {
        if ( pos/80 == 24 )
        {
            move_up();
            pos=80*24;
        }
        else
        {
            pos=(pos/80+1)*80;
        }
    }
    else
    {
        if ( pos == 80*25-2 )
        {
            move_up();
            pos-=80;
        }
        ((volatile char *)0xb8000)[pos<<1]=c;
        //*(volatile char *)((uint16_t *)0xb8000+pos)=*str;
        ++pos;
    }
}

void puts_t(const char * str)
{
    while ( true )
    {
        if ( *str == '\0' )
        {
            break;
        }
        else if ( *str == '\n' )
        {
            if ( pos/80 == 24 )
            {
                move_up();
                pos=80*24;
            }
            else
            {
                pos=(pos/80+1)*80;
            }
        }
        else
        {
            if ( pos == 80*25-1 )
            {
                move_up();
                pos-=80;
            }
            ((volatile char *)0xb8000)[pos<<1]=*str;
            //*(volatile char *)((uint16_t *)0xb8000+pos)=*str;
            ++pos;
        }
        ++str;
    }
}

void move_up()
{
    for (size_t* i=(size_t *)0xb8000; ;)
    {
        *(volatile size_t *)i=*(size_t *)((uint8_t*)i+160);
        if ( i >= (size_t *)(0xb8000+160*24-sizeof(size_t)) )
        {
            break;
        }
        ++i;
    }
    for ( size_t* i=(size_t *)(0xb8000+160*24); ; )
    {
        *(volatile size_t *)i=0x0700070007000700;
        if ( i >= (size_t *)(0xb8000+160*25-sizeof(size_t)) )
        {
            break;
        }
        ++i;
    }
}
