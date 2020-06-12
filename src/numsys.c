

int numberOfLeadingZeros(unsigned int i)
{
    if(i <= 0)
    {
        return i == 0 ? 32 : 0;
    }
    int n = 32;
    if (i >= 1 << 16) { n -= 16; i >>= 16; }
    if (i >= 1 <<  8) { n -=  8; i >>=  8; }
    if (i >= 1 <<  4) { n -=  4; i >>=  4; }
    if (i >= 1 <<  2) { n -=  2; i >>=  2; }
    return n - (i >> 1);
}

int numberOfTrailingZeros(unsigned int i)
{
    i = ~i & (i - 1);
    if (i <= 0)
        return i & 32;
    int n = 1;
    if (i > 1 << 16) { n += 16; i >>= 16; }
    if (i > 1 <<  8) { n +=  8; i >>=  8; }
    if (i > 1 <<  4) { n +=  4; i >>=  4; }
    if (i > 1 <<  2) { n +=  2; i >>=  2; }
    return n + (i >> 1);
}
