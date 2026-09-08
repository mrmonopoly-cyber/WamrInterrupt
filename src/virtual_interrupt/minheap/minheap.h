#pragma once

#define MINHEAP_EXTRACT_CAP(MINHEAP) ( sizeof((MINHEAP)->data) / sizeof(*(MINHEAP)->data) )

#define MINHEAP_TEMPLATE(T, CAP) struct {T data[(CAP);}
