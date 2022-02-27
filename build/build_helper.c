#include <stdio.h>

int main(int, char *argv[])
{
    long int sector_nums;
    {
        FILE *f_in=fopen(argv[1], "rb");
        if ( f_in == NULL )
        {
            return -1;
        }
        if (fseek(f_in, 0, SEEK_END) != 0 )
        {
            fclose( f_in );
            return -1;
        }
        sector_nums=ftell(f_in);
        if ( sector_nums==-1 )
        {
            fclose( f_in );
            return -1;
        }
        if ( fclose( f_in ) != 0 )
        {
            return -1;
        }
    }
    if ( sector_nums%512 == 0 )
    {
        sector_nums/=512;
    }
    else
    {
        sector_nums=sector_nums/512+1;
    }
    {
        FILE *f_out=fopen(argv[2], "wb");
        if ( f_out==NULL )
        {
            return -1;
        }
        if ( fwrite(&sector_nums, sizeof(sector_nums), 1, f_out) != 1 )
        {
            fclose(f_out);
            return -1;
        }
        if ( fclose(f_out) != 0 )
        {
            return -1;
        }
    }
    return 0;
}
