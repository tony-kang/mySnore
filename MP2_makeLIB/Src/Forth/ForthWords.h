#ifndef __FORTH_WORD_H_
#define __FORTH_WORD_H_

char Add(void);
char  dec(void);
char  Mul(void);
char  Division(void);
char  ViewStack(void);
char Pop(void);
char ClrStack(void);
char Words(void);
char ClearScreen(void);
char Store(void);
int Fetch(void);
char  Hex(void);
char  Decimal(void);
int CFetch(void);
int WFetch(void);
char CStore(void);
char WStore(void);
char Dump(void);
char dup(void);
char Swap(void);
char Over(void);
char Rot(void);
char Drop(void);
char RotRot(void);
char Drop2(void);
char Dup2(void);
char Mod(void);
char Pad(void);
char Fill(void);
char Erase(void);
char Cmove(void);
char AddStore(void);
char CAddStore(void);
char Equal(void);
char LessThanZero(void);
char MoreThanZero(void);
char Invert(void);
char Ms(void);
char UPop(void);
char Cr(void);
char Blank(void);
char BlankN(void);
char RPop(void);
char Key(void);
char Emit(void);
char Type(void);
char HOME(void);
char Constant(void);
char Variable(void);
char Equ(void);
char Cell(void);
char Cells(void);
char Array(void);
char CArray(void);
char Help(void);
char GetValue(void);
char Forget(void);
unsigned int PStack(int *data);
unsigned int SStack(unsigned int Data);
char At(void);
char Not(void);
char Bounds(void);
char LogSwitch(void);

#endif
