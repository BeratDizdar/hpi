/* Host Platform Interface */
#pragma once

//lpVtbl yapmak ile uğraşmayacağım şuanda bana bu yeterli
//birden fazla örnek yaratmanız şiddetle önerilmez.

typedef struct HPI {
    void (*Release)();
    void* (*GetPtr)();
    int (*Update)();
    int (*GetWidth)();
    int (*GetHeight)();
    
    void* (*AllocateFramebuffer)();
    void (*BlitFramebuffer)();
    void (*FreeFramebuffer)();

    int (*KeyDown)(int k);
    int (*KeyPressed)(int k);
    int (*KeyReleased)(int k);
    void (*MouseLock)();
    void (*MouseUnlock)();
    void (*MousePos)(int* x, int* y);
    int (*MouseLeft)();
    int (*MouseRight)();
} IPlatform;
IPlatform* HPI_Create(const unsigned short* title, int w, int h, int bFullscreen);