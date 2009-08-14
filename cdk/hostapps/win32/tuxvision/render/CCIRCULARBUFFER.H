#define MAX_BUFFER_ITEMS    16

typedef struct 
    {
    DWORD    isUsed;
    LONGLONG llStartTime;
    BYTE    *BufferStartAddress;
    DWORD    BufferSize;
    } BufferItemExtension;

class CCircularBuffer 
{
public:
    CCircularBuffer();
   ~CCircularBuffer();

    HRESULT Initialize(int dwBufferSize, int numReadPointer);
    HRESULT DeInitialize();
    void    Interrupt();

    BOOL    canWrite(long len);
    void    Write(unsigned char *src, long len);
    BOOL    canRead(int which, long len);
    int     Read(int which, unsigned char *dest, long len);
    BOOL    Clear();
    int     Skip(int which, long len);
    int     Remain(int which, long *len);

private:


    int     m_BufferSize;
    BYTE   *m_Buffer;
    int    *m_RPtr;
    int     m_WPtr;
    int     m_numReadPointer;

    int     m_BufferExtensionRIndex;
    int     m_BufferExtensionWIndex;

    BufferItemExtension* m_BufferItemArray[MAX_BUFFER_ITEMS];

    CRITICAL_SECTION m_CS; 
    HANDLE           m_Event;
    BOOL             m_Interrupt;
};