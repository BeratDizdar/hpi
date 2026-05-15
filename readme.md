# HPI

```c
#include "hpi.h"
int main() {

    IPlatform* p = HPI_Create(L"Test", 800, 600, 0);
    unsigned int* mem = p->AllocateFramebuffer();

    for (int i = 0; i < 800*600; i++) mem[i] = 0x255353; // boktan learnopengl.com rengi

    while (p->Update())
    {
        if (p->KeyPressed(27)) break; // Escape işi

        
        p->BlitFramebuffer();
    }
    
    p->FreeFramebuffer();
    p->Release();
    return 0;
}
```