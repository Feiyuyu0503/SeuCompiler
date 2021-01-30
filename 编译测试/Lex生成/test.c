int main()
{
    /*just
    a 
    test
    */
    int x;
    int y;
    int z;
	x = 9;
	y = 0x1eU;
    z = 3;
	x=x+1;
	if (x == 0)
    {
        y = y-1;
        z = y+1;
    }
    else
    { 
        y = x/2;
        z = y+5;
    };
    return 0;
}