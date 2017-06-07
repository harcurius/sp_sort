static void
MemoryCopy(void *Dest, void *Src, umm Size)
{
    u8 *d = (u8*)Dest;
    u8 *s = (u8*)Src;
    for(umm ByteIndex = 0;
        ByteIndex < Size;
        ++ByteIndex)
    {
        *d++ = *s++;
    }
}

static void
MemorySet(void *Ptr, int Value, umm Size)
{
    u8 *d = (u8*)Ptr;
    for(umm ByteIndex = 0;
        ByteIndex < Size;
        ++ByteIndex)
    {
        *d++ = Value;
    }
}