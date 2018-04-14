/*
 * epilog.c
 *
 * The contents of this file are added at the en of any generated 
 * 'C' code for 'Win32Sim' platform
 */

void initActors()
{
  static _Main	mainActor;

  _Main_constructor(&mainActor, NULL);
}

