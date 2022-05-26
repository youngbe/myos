#include "terminal.h"

class A 
{
public:
    A()
    {
        tputs("A gouzao\n");
    }
    ~A()
    {
        tputs("A xigou\n");
    }
};

A a;
