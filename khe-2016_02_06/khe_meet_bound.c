
/*****************************************************************************/
/*                                                                           */
/*  THE KHE HIGH SCHOOL TIMETABLING ENGINE                                   */
/*  COPYRIGHT (C) 2010 Jeffrey H. Kingston                                   */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either Version 3, or (at your option)      */
/*  any later version.                                                       */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston MA 02111-1307 USA   */
/*                                                                           */
/*  FILE:         khe_meet_bound.c                                           */
/*  DESCRIPTION:  Meet bounds                                                */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define KHE_MAX_DURATION 1000  /* for sanity checking */
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND - a meet bound                                            */
/*                                                                           */
/*****************************************************************************/

struct khe_meet_bound_rec {
  KHE_SOLN			soln;			/* enclosing soln    */
  int				soln_index;		/* index in soln     */
  int				reference_count;	/* reference count   */
  bool				occupancy;		/* occupancy         */
  KHE_TIME_GROUP		default_time_group;	/* default time group*/
  ARRAY_KHE_TIME_GROUP		time_groups;		/* indexed by durn-1 */
  ARRAY_KHE_MEET		meets;			/* meets             */
  KHE_MEET_BOUND		copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "creation and deletion"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND KheMeetBoundDoMake(void)                                  */
/*                                                                           */
/*  Obtain a new meet bound object from the memory allocator.                */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET_BOUND KheMeetBoundDoMake(void)
{
  KHE_MEET_BOUND res;
  MMake(res);
  MArrayInit(res->time_groups);
  MArrayInit(res->meets);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundUnMake(KHE_MEET_BOUND mb)                               */
/*                                                                           */
/*  Undo KheMeetBoundDoMake, returning mb's memory to the memory allocator.  */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundUnMake(KHE_MEET_BOUND mb)
{
  MArrayFree(mb->time_groups);
  MArrayFree(mb->meets);
  MFree(mb);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND KheMeetBoundDoGet(KHE_SOLN soln)                          */
/*                                                                           */
/*  Get a meet bound object, either from soln's free list or allocated.      */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET_BOUND KheMeetBoundDoGet(KHE_SOLN soln)
{
  KHE_MEET_BOUND res;
  res = KheSolnGetMeetBoundFromFreeList(soln);
  if( res == NULL )
    res = KheMeetBoundDoMake();
  res->reference_count = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundUnGet(KHE_MEET_BOUND mb)                                */
/*                                                                           */
/*  Undo KheMeetBoundDoGet, adding mb to its soln's free list.               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetBoundUnGet(KHE_MEET_BOUND mb)
{
  KheSolnAddMeetBoundToFreeList(mb->soln, mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundReferenceCountIncrement(KHE_MEET_BOUND mb)              */
/*                                                                           */
/*  Increment mb's reference count.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundReferenceCountIncrement(KHE_MEET_BOUND mb)
{
  mb->reference_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundReferenceCountDecrement(KHE_MEET_BOUND mb)              */
/*                                                                           */
/*  Decrement mb's reference count, and possibly add mb to the free list.    */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundReferenceCountDecrement(KHE_MEET_BOUND mb)
{
  MAssert(mb->reference_count >= 1,
    "KheMeetBoundReferenceCountDecrement internal error");
  if( --mb->reference_count == 0 )
    KheMeetBoundUnGet(mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundDoAdd(KHE_MEET_BOUND mb, KHE_SOLN soln,                 */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Initialize mb and add it to soln (NB reference_count is already set).    */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundDoAdd(KHE_MEET_BOUND mb, KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  if( DEBUG1 )
  {
    fprintf(stderr, "  KheMeetBoundDoAdd(mb %p, %s, ", (void *) mb,
      occupancy ? "true" : "false");
    KheTimeGroupDebug(dft_tg, 2, -1, stderr);
    fprintf(stderr, ")\n");
  }
  KheSolnAddMeetBound(soln, mb);  /* will set soln and soln_index */
  KheMeetBoundReferenceCountIncrement(mb);
  mb->occupancy = occupancy;
  mb->default_time_group = dft_tg;
  MArrayClear(mb->time_groups);
  MArrayClear(mb->meets);
  mb->copy = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundUnAdd(KHE_MEET_BOUND mb)                                */
/*                                                                           */
/*  Undo KheMeetBoundDoAdd, leaving mb unlinked from the solution.           */
/*                                                                           */
/*****************************************************************************/

static void KheMeetBoundUnAdd(KHE_MEET_BOUND mb)
{
  if( DEBUG1 )
    fprintf(stderr, "  KheMeetBoundUnAdd(mb %p)\n", (void *) mb);

  /* delete from soln */
  KheSolnDeleteMeetBound(mb->soln, mb);

  /* mb is now not referenced from solution (this call may free mb) */
  KheMeetBoundReferenceCountDecrement(mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelAdd(KHE_MEET_BOUND mb, KHE_SOLN soln,             */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Kernel operation which adds mb to soln (but does not make it).           */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelAdd(KHE_MEET_BOUND mb, KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  if( DEBUG3 )
    fprintf(stderr, "KheMeetBoundKernelAdd(mb %p, ...)\n", (void *) mb);
  KheMeetBoundDoAdd(mb, soln, occupancy, dft_tg);
  KheSolnOpMeetBoundAdd(soln, mb, occupancy, dft_tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelAddUndo(KHE_MEET_BOUND mb)                        */
/*                                                                           */
/*  Undo KheMeetBoundKernelAdd.                                              */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelAddUndo(KHE_MEET_BOUND mb)
{
  if( DEBUG3 )
    fprintf(stderr, "KheMeetBoundKernelAddUndo(mb %p)\n", (void *) mb);
  KheMeetBoundUnAdd(mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelDelete(KHE_MEET_BOUND mb)                         */
/*                                                                           */
/*  Kernel operation which deletes mb (but does not free it).                */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelDelete(KHE_MEET_BOUND mb)
{
  if( DEBUG3 )
    fprintf(stderr, "KheMeetBoundKernelDelete(mb %p)\n", (void *) mb);
  KheSolnOpMeetBoundDelete(mb->soln, mb, mb->occupancy, mb->default_time_group);
  KheMeetBoundUnAdd(mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelDeleteUndo(KHE_MEET_BOUND mb, KHE_SOLN soln,      */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Undo KheMeetBoundKernelDelete.                                           */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelDeleteUndo(KHE_MEET_BOUND mb, KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  if( DEBUG3 )
    fprintf(stderr, "KheMeetBoundKernelDeleteUndo(mb %p, ...)\n", (void *) mb);
  KheMeetBoundDoAdd(mb, soln, occupancy, dft_tg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND KheMeetBoundMake(KHE_SOLN soln,                           */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Make a meet bound with these attributes.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MEET_BOUND KheMeetBoundMake(KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  KHE_MEET_BOUND res;

  /* make and initialize a new meet bound object from scratch */
  MAssert(dft_tg != NULL, "KheMeetBoundMake: dft_tg == NULL");
  res = KheMeetBoundDoGet(soln);

  /* add it to the soln and return it */
  KheMeetBoundKernelAdd(res, soln, occupancy, dft_tg);
  if( DEBUG2 )
  {
    fprintf(stderr, "%p = KheMeetBoundMake(soln, %s, ", (void *) res,
      occupancy ? "true" : "false");
    KheTimeGroupDebug(dft_tg, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBoundDeleteCheck(KHE_MEET_BOUND mb)                          */
/*                                                                           */
/*  Check whether it's safe to delete mb.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheMeetBoundDeleteCheck(KHE_MEET_BOUND mb)
{
  KHE_MEET meet;  int i;
  MArrayForEach(mb->meets, &meet, &i)
    if( !KheMeetDeleteMeetBoundCheck(meet, mb) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBoundDelete(KHE_MEET_BOUND mb)                               */
/*                                                                           */
/*  Delete mb, if safe.                                                      */
/*                                                                           */
/*****************************************************************************/

bool KheMeetBoundDelete(KHE_MEET_BOUND mb)
{
  KHE_TIME_GROUP tg;  int i;

  /* ensure operation will succeed */
  if( !KheMeetBoundDeleteCheck(mb) )
    return false;

  /* delete mb from all its meets */
  while( MArraySize(mb->meets) > 0 )
    KheMeetDeleteMeetBound(MArrayFirst(mb->meets), mb);

  /* delete all mb's time groups (to record them for possible redo) */
  MArrayForEach(mb->time_groups, &tg, &i)
    KheMeetBoundKernelDeleteTimeGroup(mb, i + 1, tg);

  /* carry out the kernel delete operation */
  KheMeetBoundKernelDelete(mb);
  if( DEBUG2 )
    fprintf(stderr, "KheMeetBoundDelete(%p)\n", (void *) mb);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "copy"                                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND KheMeetBoundCopyPhase1(KHE_MEET_BOUND mb)                 */
/*                                                                           */
/*  Carry out phase 1 of copying mb.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MEET_BOUND KheMeetBoundCopyPhase1(KHE_MEET_BOUND mb)
{
  KHE_MEET_BOUND copy;  KHE_TIME_GROUP tg;  int i;  KHE_MEET meet;
  if( mb->copy == NULL )
  {
    MMake(copy);
    mb->copy = copy;
    copy->soln = KheSolnCopyPhase1(mb->soln);
    copy->soln_index = mb->soln_index;
    copy->reference_count = 1;  /* no paths, and mb is linked in */
    copy->occupancy = mb->occupancy;
    copy->default_time_group = mb->default_time_group;
    MArrayInit(copy->time_groups);
    MArrayForEach(mb->time_groups, &tg, &i)
      MArrayAddLast(copy->time_groups, tg);
    MArrayInit(copy->meets);
    MArrayForEach(mb->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    copy->copy = NULL;
  }
  return mb->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundCopyPhase2(KHE_MEET_BOUND mb)                           */
/*                                                                           */
/*  Carry out phase 2 of copying mb.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundCopyPhase2(KHE_MEET_BOUND mb)
{
  if( mb->copy != NULL )
  {
    mb->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "simple attributes"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheMeetBoundSoln(KHE_MEET_BOUND mb)                             */
/*                                                                           */
/*  Return the enclosing solution of mb.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheMeetBoundSoln(KHE_MEET_BOUND mb)
{
  return mb->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundSetSoln(KHE_MEET_BOUND mb, KHE_SOLN soln)               */
/*                                                                           */
/*  Set the soln attribute of mb.                                            */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundSetSoln(KHE_MEET_BOUND mb, KHE_SOLN soln)
{
  mb->soln = soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetBoundSolnIndex(KHE_MEET_BOUND mb)                             */
/*                                                                           */
/*  Return the soln_index attribute of mb.                                   */
/*                                                                           */
/*****************************************************************************/

int KheMeetBoundSolnIndex(KHE_MEET_BOUND mb)
{
  return mb->soln_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundSetSolnIndex(KHE_MEET_BOUND mb, int soln_index)         */
/*                                                                           */
/*  Set the soln_index attribute of mb.                                      */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundSetSolnIndex(KHE_MEET_BOUND mb, int soln_index)
{
  mb->soln_index = soln_index;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBoundOccupancy(KHE_MEET_BOUND mb)                            */
/*                                                                           */
/*  Return the occupancy attribute of mb.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheMeetBoundOccupancy(KHE_MEET_BOUND mb)
{
  return mb->occupancy;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheMeetBoundDefaultTimeGroup(KHE_MEET_BOUND mb)           */
/*                                                                           */
/*  Return the default time group of mb.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheMeetBoundDefaultTimeGroup(KHE_MEET_BOUND mb)
{
  return mb->default_time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time groups"                                                  */
/*                                                                           */
/*  Implementation notes.  For each duration that has been the subject of    */
/*  a call to KheMeetBoundAddTimeGroup or KheMeetBoundTimeGroup, there is    */
/*  a non-NULL entry in the time_groups array.  For other durations there    */
/*  is either a NULL entry or no entry at all.                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundDoAddTimeGroup(KHE_MEET_BOUND mb,                       */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Add tg to mb.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheMeetBoundDoAddTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  while( MArraySize(mb->time_groups) < duration )
    MArrayAddLast(mb->time_groups, NULL);
  MArrayPut(mb->time_groups, duration - 1, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundDoDeleteTimeGroup(KHE_MEET_BOUND mb,                    */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Delete tg from mb.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheMeetBoundDoDeleteTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  MAssert(MArraySize(mb->time_groups) >= duration,
    "KheMeetBoundDoDeleteTimeGroup internal error 1");
  MAssert(MArrayGet(mb->time_groups, duration - 1) == tg,
    "KheMeetBoundDoDeleteTimeGroup internal error 2");
  MArrayPut(mb->time_groups, duration - 1, NULL);
  while( MArraySize(mb->time_groups) > 0 && MArrayLast(mb->time_groups)==NULL )
    MArrayDropLast(mb->time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelAddTimeGroup(KHE_MEET_BOUND mb,                   */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Kernel operation for adding tg to mb.                                    */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelAddTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  KheSolnOpMeetBoundAddTimeGroup(mb->soln, mb, duration, tg);
  KheMeetBoundDoAddTimeGroup(mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelAddTimeGroupUndo(KHE_MEET_BOUND mb,               */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Kernel operation for undoing the addition of tg to mb.                   */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelAddTimeGroupUndo(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  KheMeetBoundDoDeleteTimeGroup(mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelDeleteTimeGroup(KHE_MEET_BOUND mb,                */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Kernel operation for deleting tg from mb.                                */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelDeleteTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  KheSolnOpMeetBoundDeleteTimeGroup(mb->soln, mb, duration, tg);
  KheMeetBoundDoDeleteTimeGroup(mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundKernelDeleteTimeGroupUndo(KHE_MEET_BOUND mb,            */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Kernel operation for undoing the deletion of tg from mb.                 */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundKernelDeleteTimeGroupUndo(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  KheMeetBoundDoAddTimeGroup(mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundAddTimeGroup(KHE_MEET_BOUND mb,                         */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Add a time group for this duration.                                      */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundAddTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  MAssert(duration <= KHE_MAX_DURATION,
    "KheMeetBoundAddTimeGroup: unreasonably large duration (%d)", duration);
  MAssert(MArraySize(mb->meets) == 0, "KheMeetBoundAddTimeGroup: mb has meets");
  KheMeetBoundKernelAddTimeGroup(mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheMeetBoundTimeGroup(KHE_MEET_BOUND mb, int duration)    */
/*                                                                           */
/*  Return the time group for this duration.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheMeetBoundTimeGroup(KHE_MEET_BOUND mb, int duration)
{
  KHE_TIME_GROUP res;
  MAssert(duration <= KHE_MAX_DURATION,
    "KheMeetBoundTimeGroup: unreasonably large duration (%d)", duration);
  while( MArraySize(mb->time_groups) < duration )
    MArrayAddLast(mb->time_groups, NULL);
  res = MArrayGet(mb->time_groups, duration - 1);
  if( res == NULL )
  {
    if( mb->occupancy )
      res = KheSolnStartingTimeGroup(mb->soln, duration,mb->default_time_group);
    else
      res = mb->default_time_group;
    MArrayPut(mb->time_groups, duration - 1, res);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnStartingTimeGroup(KHE_SOLN soln, int duration,     */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Return the set of all starting times that ensure that a meet of the      */
/*  given duration lies entirely within tg.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheSolnStartingTimeGroup(KHE_SOLN soln, int duration,
  KHE_TIME_GROUP tg)
{
  int i;  KHE_TIME time;
  if( duration == 1 )
    return tg;
  KheSolnTimeGroupBegin(soln);
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    time = KheTimeGroupTime(tg, i);
    if( KheTimeGroupOverlap(tg, time, duration) == duration )
      KheSolnTimeGroupAddTime(soln, time);
  }
  return KheSolnTimeGroupEnd(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundAddMeet(KHE_MEET_BOUND mb, KHE_MEET meet)               */
/*                                                                           */
/*  Add meet to mb.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundAddMeet(KHE_MEET_BOUND mb, KHE_MEET meet)
{
  MAssert(mb->reference_count > 0, "KheMeetBoundAddMeet internal error");
  MArrayAddLast(mb->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundDeleteMeet(KHE_MEET_BOUND mb, KHE_MEET meet)            */
/*                                                                           */
/*  Delete meet from mb.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheMeetBoundDeleteMeet(KHE_MEET_BOUND mb, KHE_MEET meet)
{
  int pos;
  MAssert(mb->reference_count > 0, "KheMeetBoundDeleteMeet internal error 1");
  if( !MArrayContains(mb->meets, meet, &pos) )
    MAssert(false, "KheMeetBoundDeleteMeet internal error 2");
  MArrayDropAndPlug(mb->meets, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetBoundMeetCount(KHE_MEET_BOUND mb)                             */
/*                                                                           */
/*  Return the number of meets that mb applies to.                           */
/*                                                                           */
/*****************************************************************************/

int KheMeetBoundMeetCount(KHE_MEET_BOUND mb)
{
  return MArraySize(mb->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetBoundMeet(KHE_MEET_BOUND mb, int i)                      */
/*                                                                           */
/*  Return the ith meet that mb applies to.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetBoundMeet(KHE_MEET_BOUND mb, int i)
{
  return MArrayGet(mb->meets, i);
}
