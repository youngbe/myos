#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <terminal_io.h>

static void move_up();
static size_t pos=0;


void clear_t()
{
    size_t* temp=(size_t *)0xb8000;
    while ( temp<(size_t *)(0xb8000+80*25*2) )
    {
        *temp=0x0700070007000700;
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
        }
        ((char *)0xb8000)[pos<<1]=c;
        //*(char *)((uint16_t *)0xb8000+pos)=*str;
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
            }
            ((char *)0xb8000)[pos<<1]=*str;
            //*(char *)((uint16_t *)0xb8000+pos)=*str;
            ++pos;
        }
        ++str;
    }
}

void move_up()
{
    if ( pos <= 80 )
    {
        clear_t();
        return;
    }
    pos-=80;
    for (size_t* i=(size_t *)0xb8000, *max=(size_t *)((uint16_t *)0xb8000+pos); i<max; ++i)
    {
        *i=*(size_t *)((uint8_t*)i+160);
    }
}
