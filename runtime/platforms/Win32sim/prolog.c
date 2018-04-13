/*
 * prolog.c
 *
 * The contents of this file are added before any generated 'C' code for
 * 'Win32Sim' platform
 */

#include <stdlib.h>

typedef struct {
  void *actorPtr;
  void *inputPtr;
}MessageSlot;

void postMessage (const MessageSlot* address, const void* params, size_t paramsSize);
void initPcr ();
void runScheduler ();

typedef unsigned char bool;
static const bool true = 1;
static const bool false = 0;

void main(){
  initPcr();
  runScheduler();
}
