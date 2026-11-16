#pragma once
#include <olectl.h>
// OLE also accepts bitmap/GIF data; portrait inputs specifically require JPEG.
inline IPicture* DecodePortraitJpeg(const BYTE* bytes,size_t size)
{
    if(size<2 || bytes[0]!=0xff || bytes[1]!=0xd8) return nullptr;
    HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,size);void* destination=GlobalLock(memory);
    memcpy(destination,bytes,size);GlobalUnlock(memory);
    IStream* stream=nullptr;
    if(FAILED(CreateStreamOnHGlobal(memory,TRUE,&stream))) {GlobalFree(memory);return nullptr;}
    IPicture* picture=nullptr;const HRESULT status=OleLoadPicture(stream,LONG(size),FALSE,IID_IPicture,(void**)&picture);stream->Release();
    return SUCCEEDED(status)?picture:nullptr;
}
