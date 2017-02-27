
/*****************************************************************************/
/*                                                                           */
/*  THE KTS TIMETABLING SYSTEM                                               */
/*  COPYRIGHT (C) 2004, 2008 Jeffrey H. Kingston                             */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  FILE:         lset.h                                                     */
/*  MODULE:       Sets represented as arbitrary length bit vectors           */
/*                                                                           */
/*****************************************************************************/
#ifndef LSET_HEADER_FILE
#define LSET_HEADER_FILE

#include <stdio.h>
#include <stdbool.h>
#include "m.h"

typedef struct lset_rec *LSET;

extern LSET LSetNew(void);
extern LSET LSetCopy(LSET s);
extern void LSetShift(LSET s, LSET *res, int delta, int lim);
extern void LSetClear(LSET s);
extern void LSetInsert(LSET *s, unsigned int i);
extern void LSetDelete(LSET s, unsigned int i);
extern void LSetAssign(LSET *target, LSET source);
extern void LSetUnion(LSET *target, LSET source);
extern void LSetIntersection(LSET target, LSET source);
extern void LSetDifference(LSET target, LSET source);
extern bool LSetEmpty(LSET s);
extern bool LSetEqual(LSET s1, LSET s2);
extern bool LSetSubset(LSET s1, LSET s2);
extern bool LSetDisjoint(LSET s1, LSET s2);
extern bool LSetContains(LSET s, unsigned int i);
extern unsigned int LSetMin(LSET s);
extern unsigned int LSetMax(LSET s);
extern int LSetLexicalCmp(LSET s1, LSET s2);
extern unsigned int LSetHash(LSET s);
extern void LSetExpand(LSET s, ARRAY_SHORT *add_to);
extern void LSetFree(LSET s);
extern char *LSetShow(LSET s);
extern void LSetTest(FILE *fp);

/* hash tables indexed by lsets */
typedef struct lset_table_rec *LSET_TABLE;

extern LSET_TABLE LSetTableMake(void);
extern void LSetTableFree(LSET_TABLE lt);
extern void LSetTableInsert(LSET_TABLE lt, LSET s, void *val);
extern bool LSetTableRetrieve(LSET_TABLE lt, LSET s, void **val);

#endif
