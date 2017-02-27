
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
/*  FILE:         khe_ss_time_equiv.c                                        */
/*  DESCRIPTION:  Time-equivalence of events and resources                   */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG6 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_FRAME - a record of what one meet is assigned to                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_meet_frame_rec {
  KHE_MEET		leader_meet;		/* at end of fixed chain     */
  int			offset;			/* offset into leader meet   */
  int			duration;		/* duration of orig meet     */
} KHE_MEET_FRAME;

typedef MARRAY(KHE_MEET_FRAME) ARRAY_KHE_MEET_FRAME;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_FRAME - a record of what one event is assigned to              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_event_frame_rec {
  KHE_EVENT		event;			/* the event                 */
  ARRAY_KHE_MEET_FRAME	meet_frames;		/* the frames of its meets   */
} *KHE_EVENT_FRAME;

typedef MARRAY(KHE_EVENT_FRAME) ARRAY_KHE_EVENT_FRAME;


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_FRAME                                                       */
/*                                                                           */
/*  A resource and the events it is preassigned to, after they have been     */
/*  adjusted to bring linked events together.                                */
/*                                                                           */
/*****************************************************************************/
typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;

typedef struct khe_resource_frame_rec {
  KHE_RESOURCE			resource;		/* the resource      */
  ARRAY_KHE_EVENT		events;			/* events            */
} *KHE_RESOURCE_FRAME;

typedef MARRAY(KHE_RESOURCE_FRAME) ARRAY_KHE_RESOURCE_FRAME;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_EQUIV - solver object for time equivalence                      */
/*                                                                           */
/*****************************************************************************/
typedef MARRAY(KHE_EVENT_GROUP) ARRAY_KHE_EVENT_GROUP;
typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_RESOURCE_GROUP) ARRAY_KHE_RESOURCE_GROUP;

struct khe_time_equiv_rec {
  KHE_SOLN			soln;
  ARRAY_KHE_EVENT_FRAME		curr_event_frames;
  ARRAY_KHE_EVENT_FRAME		free_event_frames;		/* free list */
  ARRAY_KHE_RESOURCE_FRAME	curr_resource_frames;
  ARRAY_KHE_RESOURCE_FRAME	free_resource_frames;		/* free list */
  ARRAY_KHE_EVENT_GROUP		event_groups_by_event;
  ARRAY_KHE_EVENT_GROUP		event_groups;
  ARRAY_INT			event_group_indexes_by_event;
  ARRAY_KHE_RESOURCE_GROUP	resource_groups_by_resource;
  ARRAY_KHE_RESOURCE_GROUP	resource_groups;
  ARRAY_INT			resource_group_indexes_by_resource;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet and event frames"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheMeetFrameCmp(const void *t1, const void *t2)                      */
/*                                                                           */
/*  Comparison function for sorting an array of meet frames into a           */
/*  canonical order.                                                         */
/*                                                                           */
/*****************************************************************************/

static int KheMeetFrameCmp(const void *t1, const void *t2)
{
  KHE_MEET_FRAME *mf1 = (KHE_MEET_FRAME *) t1;
  KHE_MEET_FRAME *mf2 = (KHE_MEET_FRAME *) t2;
  if( mf1->leader_meet != mf2->leader_meet )
    return KheMeetSolnIndex(mf1->leader_meet) -
      KheMeetSolnIndex(mf2->leader_meet);
  else if( mf1->offset != mf2->offset )
    return mf1->offset - mf2->offset;
  else
    return mf1->duration - mf2->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_FRAME KheEventFrameMake(KHE_EVENT e, KHE_TIME_EQUIV te)        */
/*                                                                           */
/*  Return a new event frame object for e, with its meet frames all set.     */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_FRAME KheEventFrameMake(KHE_EVENT e, KHE_TIME_EQUIV te)
{
  KHE_EVENT_FRAME res;  int i;  KHE_MEET meet;  KHE_MEET_FRAME mf;

  /* get memory and carry out the basic initialization */
  if( MArraySize(te->free_event_frames) > 0 )
  {
    res = MArrayRemoveLast(te->free_event_frames);
    MArrayClear(res->meet_frames);
  }
  else
  {
    MMake(res);
    MArrayInit(res->meet_frames);
  }
  res->event = e;

  /* add one meet frame for each of e's meets */
  for( i = 0;  i < KheEventMeetCount(te->soln, e);  i++ )
  {
    meet = KheEventMeet(te->soln, e, i);
    mf.leader_meet = KheMeetLastFixed(meet, &mf.offset);
    mf.duration = KheMeetDuration(meet);
    MArrayAddLast(res->meet_frames, mf);
  }

  /* sort the meet frames and return */
  MArraySort(res->meet_frames, &KheMeetFrameCmp);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventFrameFree(KHE_EVENT_FRAME ef)                               */
/*                                                                           */
/*  Free ef.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventFrameFree(KHE_EVENT_FRAME ef)
{
  MArrayFree(ef->meet_frames);
  MFree(ef);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventFrameCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting an array of event frames, to bring       */
/*  event frames with the same meet frames together.                         */
/*                                                                           */
/*****************************************************************************/

static int KheEventFrameCmp(const void *t1, const void *t2)
{
  int i, cmp;  KHE_MEET_FRAME *mf1, *mf2;
  KHE_EVENT_FRAME ef1 = * (KHE_EVENT_FRAME *) t1;
  KHE_EVENT_FRAME ef2 = * (KHE_EVENT_FRAME *) t2;
  if( MArraySize(ef1->meet_frames) != MArraySize(ef2->meet_frames) )
    return MArraySize(ef1->meet_frames) - MArraySize(ef2->meet_frames);
  for( i = 0;  i < MArraySize(ef1->meet_frames);  i++ )
  {
    mf1 = &MArrayGet(ef1->meet_frames, i);
    mf2 = &MArrayGet(ef2->meet_frames, i);
    cmp = KheMeetFrameCmp((void *) mf1, (void *) mf2);
    if( cmp != 0 )
      return cmp;
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventFrameEqual(KHE_EVENT_FRAME ef1, KHE_EVENT_FRAME ef2)        */
/*                                                                           */
/*  Return true if ef1 and ef2 are equal.                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheEventFrameEqual(KHE_EVENT_FRAME ef1, KHE_EVENT_FRAME ef2)
{
  KHE_EVENT_FRAME xef1 = ef1;
  KHE_EVENT_FRAME xef2 = ef2;
  return KheEventFrameCmp(&xef1, &xef2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event grouping"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupUpdateByEvent(KHE_EVENT_GROUP eg, KHE_TIME_EQUIV te)   */
/*                                                                           */
/*  Update te->event_groups_by_event to reflect the fact that the events     */
/*  of eg lie in eg.                                                         */
/*                                                                           */
/*****************************************************************************/

static void KheEventGroupUpdateByEvent(KHE_EVENT_GROUP eg, KHE_TIME_EQUIV te)
{
  int i;  KHE_EVENT e;
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    MArrayPut(te->event_groups_by_event, KheEventIndex(e), eg);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDoMergeRelatedEvents(KHE_MEET meet, KHE_EVENT_GROUP *first_eg,   */
/*    MERGE_STATE *merge_state, KHE_TIME_EQUIV te)                           */
/*                                                                           */
/*  Merge the event groups of the events that are related by fixed           */
/*  assignments to meet, directly or indirectly.                             */
/*                                                                           */
/*  Here *merge_state holds the current state of the merge, saying whether   */
/*  no events have been included yet, or one, or more than one.  When        */
/*  *merge_state == MERGE_ONE, *first_eg is the first event's event group.   */
/*  When *merge_state == MERGE_MANY, the events are held in a solution event */
/*  group which is currently under construction.                             */
/*                                                                           */
/*****************************************************************************/

typedef enum { MERGE_NONE, MERGE_ONE, MERGE_MANY } MERGE_STATE;

static void KheDoMergeRelatedEvents(KHE_MEET meet, KHE_EVENT_GROUP *first_eg,
  MERGE_STATE *merge_state, KHE_TIME_EQUIV te)
{
  int i;  KHE_MEET sub_meet;  KHE_EVENT e;  KHE_EVENT_GROUP eg;

  /* take account of meet itself, if it is derived from an event */
  e = KheMeetEvent(meet);
  if( e != NULL )
  {
    eg = MArrayGet(te->event_groups_by_event, KheEventIndex(e));
    switch( *merge_state )
    {
      case MERGE_NONE:

	*first_eg = eg;
        *merge_state = MERGE_ONE;
	break;

      case MERGE_ONE:

	if( !KheEventGroupEqual(*first_eg, eg) )
	{
	  KheSolnEventGroupBegin(te->soln);
	  KheSolnEventGroupUnion(te->soln, *first_eg);
	  KheSolnEventGroupUnion(te->soln, eg);
	  *merge_state = MERGE_MANY;
	}
	break;

      case MERGE_MANY:

	KheSolnEventGroupUnion(te->soln, eg);
	break;

      default:

	MAssert(false, "KheDoMergeRelatedEvents internal error");
    }
  }

  /* take account of the fixed assignments to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    sub_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetAssignIsFixed(sub_meet) )
      KheDoMergeRelatedEvents(sub_meet, first_eg, merge_state, te);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMergeRelatedEvents(KHE_MEET meet, KHE_TIME_EQUIV te)             */
/*                                                                           */
/*  Merge the event groups of the events related by fixed assignments to     */
/*  meet, directly or indirectly.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheMergeRelatedEvents(KHE_MEET meet, KHE_TIME_EQUIV te)
{
  KHE_EVENT_GROUP first_eg, eg;  MERGE_STATE merge_state;
  merge_state = MERGE_NONE;
  first_eg = NULL;
  KheDoMergeRelatedEvents(meet, &first_eg, &merge_state, te);
  if( merge_state == MERGE_MANY )
  {
    eg = KheSolnEventGroupEnd(te->soln);
    KheEventGroupUpdateByEvent(eg, te);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupBreakUp(KHE_EVENT_GROUP eg, KHE_TIME_EQUIV te)         */
/*                                                                           */
/*  Check whether the events of nontrivial event group eg really must run    */
/*  at the same time, and break up the group if not.                         */
/*                                                                           */
/*  This function works for event groups with any number of events, but      */
/*  it is only called for event groups with at least one event.              */
/*                                                                           */
/*****************************************************************************/

static void KheEventGroupBreakUp(KHE_EVENT_GROUP eg, KHE_TIME_EQUIV te)
{
  int i, j, k;  KHE_EVENT e;  KHE_EVENT_GROUP eg2;  KHE_EVENT_FRAME ef1, ef2;

  /* build an event frame for each event of eg */
  MArrayClear(te->curr_event_frames);
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    MArrayAddLast(te->curr_event_frames, KheEventFrameMake(e, te));
  }

  /* sort the event frames */
  MArraySort(te->curr_event_frames, &KheEventFrameCmp);

  /* handle each run of equal event frames */
  for( i = 0;  i < MArraySize(te->curr_event_frames);  i = j )
  {
    ef1 = MArrayGet(te->curr_event_frames, i);
    for( j = i + 1;  j < MArraySize(te->curr_event_frames);  j++ )
    {
      ef2 = MArrayGet(te->curr_event_frames, j);
      if( !KheEventFrameEqual(ef1, ef2) )
	break;
    }

    /* now the run of event frames from i to j-1 is equal */
    if( (i - j) != MArraySize(te->curr_event_frames) )
    {
      /* run is not the whole group, needs a new group */
      if( (i - j) == 1 )
      {
	/* singleton, just use e's own event group */
	eg2 = KheEventSingletonEventGroup(ef1->event);
      }
      else
      {
	/* more than one event, build a new event group */
	KheSolnEventGroupBegin(te->soln);
	for( k = i;  k < j;  k++ )
	{
	  ef2 = MArrayGet(te->curr_event_frames, k);
	  KheSolnEventGroupAddEvent(te->soln, ef2->event);
	}
	eg2 = KheSolnEventGroupEnd(te->soln);
      }
      KheEventGroupUpdateByEvent(eg2, te);
    }
  }

  /* reclaim memory */
  MArrayAppend(te->free_event_frames, te->curr_event_frames, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource frames"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventIndexCmp(const void *p1, const void *p2)                     */
/*                                                                           */
/*  Comparison function for sorting an array of events by increasing index.  */
/*                                                                           */
/*****************************************************************************/

static int KheEventIndexCmp(const void *p1, const void *p2)
{
  KHE_EVENT e1 = * (KHE_EVENT *) p1;
  KHE_EVENT e2 = * (KHE_EVENT *) p2;
  return KheEventIndex(e1) - KheEventIndex(e2);
}

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_FRAME KheResourceFrameMake(KHE_RESOURCE r,                  */
/*    KHE_TIME_EQUIV te)                                                     */
/*                                                                           */
/*  Make a new resource frame for r; use te for uniqueifying linked events.  */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_FRAME KheResourceFrameMake(KHE_RESOURCE r,
  KHE_TIME_EQUIV te)
{
  KHE_RESOURCE_FRAME res;  KHE_EVENT e;  KHE_EVENT_GROUP eg;  int i;

  /* make and initialize the basic object */
  if( MArraySize(te->free_resource_frames) > 0 )
  {
    res = MArrayRemoveLast(te->free_resource_frames);
    MArrayClear(res->events);
  }
  else
  {
    MMake(res);
    MArrayInit(res->events);
  }
  res->resource = r;

  /* add uniqueified events */
  for( i = 0;  i < KheResourceLayerEventCount(r);  i++ )
  {
    e = KheResourceLayerEvent(r, i);
    eg = MArrayGet(te->event_groups_by_event, KheEventIndex(e));
    MArrayAddLast(res->events, KheEventGroupEvent(eg, 0));
  }

  /* sort events by increasing event index */
  MArraySort(res->events, &KheEventIndexCmp);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceFrameFree(KHE_RESOURCE_FRAME rf)                         */
/*                                                                           */
/*  Free rf.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheResourceFrameFree(KHE_RESOURCE_FRAME rf)
{
  MArrayFree(rf->events);
  MFree(rf);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceFrameCmp(const void *p1, const void *p2)                  */
/*                                                                           */
/*  Comparison function for sorting an array of resource frames to           */
/*  bring equal frames together.                                             */
/*                                                                           */
/*****************************************************************************/

static int KheResourceFrameCmp(const void *p1, const void *p2)
{
  KHE_RESOURCE_FRAME rf1 = * (KHE_RESOURCE_FRAME *) p1;
  KHE_RESOURCE_FRAME rf2 = * (KHE_RESOURCE_FRAME *) p2;
  int count1 = MArraySize(rf1->events);
  int count2 = MArraySize(rf2->events);
  int i;  KHE_EVENT e1, e2;
  if( count1 != count2 )
    return count1 - count2;
  for( i = 0;  i < count1;  i++ )
  {
    e1 = MArrayGet(rf1->events, i);
    e2 = MArrayGet(rf2->events, i);
    if( e1 != e2 )
      return KheEventIndex(e1) - KheEventIndex(e2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceFrameEqual(KHE_RESOURCE_FRAME rf1,                       */
/*    KHE_RESOURCE_FRAME rf2)                                                */
/*                                                                           */
/*  Return true if rf1 and rf2 have equal sets of events.                    */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceFrameEqual(KHE_RESOURCE_FRAME rf1,
  KHE_RESOURCE_FRAME rf2)
{
  KHE_RESOURCE_FRAME x1, x2;
  x1 = rf1;
  x2 = rf2;
  return KheResourceFrameCmp((const void *) &x1, (const void *) &x2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource grouping"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeEquivGroupPreassignedResources(KHE_TIME_EQUIV te,            */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Group resources of type rt, known to be preassigned.  Use te for         */
/*  uniqueifying linked events.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheTimeEquivGroupPreassignedResources(KHE_TIME_EQUIV te,
  KHE_RESOURCE_TYPE rt)
{
  KHE_RESOURCE r;  int i, j, k, groups;  KHE_RESOURCE_GROUP rg;
  KHE_RESOURCE_FRAME rfi, rfj, rf;

  /* get the resource frames, sorted to bring equal frames together */
  if( DEBUG6 )
    fprintf(stderr, "[ KheTimeEquivGroupPreassignedResources(te, %s)\n",
      KheResourceTypeId(rt) == NULL ? "-" : KheResourceTypeId(rt));
  MArrayClear(te->curr_resource_frames);
  for( i = 0;  i < KheResourceTypeResourceCount(rt);  i++ )
  {
    r = KheResourceTypeResource(rt, i);
    MArrayAddLast(te->curr_resource_frames, KheResourceFrameMake(r, te));
  }
  MArraySort(te->curr_resource_frames, &KheResourceFrameCmp);

  /* find runs of resources with equal frames */
  groups = 0;
  for( i = 0;  i < MArraySize(te->curr_resource_frames);  i = j )
  {
    rfi = MArrayGet(te->curr_resource_frames, i);
    for( j = i + 1;  j < MArraySize(te->curr_resource_frames);  j++ )
    {
      rfj = MArrayGet(te->curr_resource_frames, j);
      if( !KheResourceFrameEqual(rfi, rfj) )
	break;
    }
    groups++;

    /* at this point, rfi ... rfj -1 can be grouped */
    if( DEBUG6 )
    {
      fprintf(stderr, "  group of %d resources:", j - i);
      for( k = i;  k < j;  k++ )
      {
	rf = MArrayGet(te->curr_resource_frames, k);
	fprintf(stderr, " %s", KheResourceId(rf->resource) == NULL ? "-" :
	  KheResourceId(rf->resource));
      }
      fprintf(stderr, "\n");
    }

    if( (j - i) > 1 )
    {
      /* not singleton, so build a resource group holding the resources */
      KheSolnResourceGroupBegin(te->soln, rt);
      for( k = i;  k < j;  k++ )
      {
	rf = MArrayGet(te->curr_resource_frames, k);
	KheSolnResourceGroupAddResource(te->soln, rf->resource);
      }
      rg = KheSolnResourceGroupEnd(te->soln);

      /* add the resource group to te->resource_groups_by_resource */
      for( k = 0;  k < KheResourceGroupResourceCount(rg);  k++ )
      {
	r = KheResourceGroupResource(rg, k);
	MArrayPut(te->resource_groups_by_resource,
	  KheResourceInstanceIndex(r), rg);
      }
    }
  }

  /* reclaim memory */
  MArrayAppend(te->free_resource_frames, te->curr_resource_frames, i);
  if( DEBUG6 )
  {
    fprintf(stderr, "  %d resources, %d groups\n", 
      KheResourceTypeResourceCount(rt), groups);
    fprintf(stderr, "] KheTimeEquivGroupPreassignedResources returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "public functions"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_EQUIV KheTimeEquivMake(void)                                    */
/*                                                                           */
/*  Make a new time equivalence solver.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_EQUIV KheTimeEquivMake(void)
{
  KHE_TIME_EQUIV res;
  MMake(res);
  res->soln = NULL;  /* set during solving */
  MArrayInit(res->curr_event_frames);
  MArrayInit(res->free_event_frames);
  MArrayInit(res->curr_resource_frames);
  MArrayInit(res->free_resource_frames);
  MArrayInit(res->event_groups_by_event);
  MArrayInit(res->event_groups);
  MArrayInit(res->event_group_indexes_by_event);
  MArrayInit(res->resource_groups_by_resource);
  MArrayInit(res->resource_groups);
  MArrayInit(res->resource_group_indexes_by_resource);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeEquivDelete(KHE_TIME_EQUIV te)                               */
/*                                                                           */
/*  Delete te, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

void KheTimeEquivDelete(KHE_TIME_EQUIV te)
{
  MArrayFree(te->curr_event_frames);
  while( MArraySize(te->free_event_frames) > 0 )
    KheEventFrameFree(MArrayRemoveLast(te->free_event_frames));
  MArrayFree(te->free_event_frames);

  MArrayFree(te->curr_resource_frames);
  while( MArraySize(te->free_resource_frames) > 0 )
    KheResourceFrameFree(MArrayRemoveLast(te->free_resource_frames));
  MArrayFree(te->free_resource_frames);

  MArrayFree(te->event_groups_by_event);
  MArrayFree(te->event_groups);
  MArrayFree(te->event_group_indexes_by_event);
  MArrayFree(te->resource_groups_by_resource);
  MArrayFree(te->resource_groups);
  MArrayFree(te->resource_group_indexes_by_resource);
  MFree(te);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeEquivSolve(KHE_TIME_EQUIV te, KHE_SOLN soln)                 */
/*                                                                           */
/*  Solve the time-equivalence problem for soln.                             */
/*                                                                           */
/*****************************************************************************/

void KheTimeEquivSolve(KHE_TIME_EQUIV te, KHE_SOLN soln)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;
  KHE_EVENT_GROUP eg;  KHE_MEET meet;  KHE_RESOURCE_TYPE rt;  int i, j;

  /* initialize te->event_groups_by_event to singleton event groups */
  if( DEBUG1 )
    fprintf(stderr, "[ KheTimeEquivSolve(soln)\n");
  te->soln = soln;
  MArrayClear(te->event_groups_by_event);
  ins = KheSolnInstance(te->soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    MArrayAddLast(te->event_groups_by_event, KheEventSingletonEventGroup(e));
    MArrayAddLast(te->event_group_indexes_by_event, -1);
  }

  /* merge event groups that might be time-equivalent */
  for( i = 0;  i < KheSolnMeetCount(te->soln);  i++ )
  {
    meet = KheSolnMeet(te->soln, i);
    if( !KheMeetIsCycleMeet(meet) && !KheMeetAssignIsFixed(meet) )
      KheMergeRelatedEvents(meet, te);
  }

  /* break up event groups with two or more elements, as needed */
  MArrayForEach(te->event_groups_by_event, &eg, &i)
    if( KheEventIndex(KheEventGroupEvent(eg, 0)) == i &&
	KheEventGroupEventCount(eg) >= 2 )
      KheEventGroupBreakUp(eg, te);

  /* te->event_groups are the unique elements of te->event_groups_by_event */
  MArrayClear(te->event_groups);
  MArrayForEach(te->event_groups_by_event, &eg, &i)
    if( KheEventIndex(KheEventGroupEvent(eg, 0)) == i )
      MArrayAddLast(te->event_groups, eg);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final event groups:\n");
    MArrayForEach(te->event_groups, &eg, &i)
      KheEventGroupDebug(eg, 2, 2, stderr);
  }

  /* set te->event_group_indexes by event */
  MArrayForEach(te->event_groups, &eg, &i)
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      MArrayPut(te->event_group_indexes_by_event, KheEventIndex(e), i);
    }

  /* initialize te->resource_groups_by_resource to singleton resource groups */
  MArrayClear(te->resource_groups_by_resource);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    MArrayAddLast(te->resource_groups_by_resource,
      KheResourceSingletonResourceGroup(r));
    MArrayAddLast(te->resource_group_indexes_by_resource, -1);
  }

  /* update te->resource_groups_by_resource for preassigned resource types */
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( KheResourceTypeDemandIsAllPreassigned(rt) )
      KheTimeEquivGroupPreassignedResources(te, rt);
  }

  /* te->resource_groups are unique elts of te->resource_groups_by_resource */
  MArrayClear(te->resource_groups);
  MArrayForEach(te->resource_groups_by_resource, &rg, &i)
    if( KheResourceInstanceIndex(KheResourceGroupResource(rg, 0)) == i )
      MArrayAddLast(te->resource_groups, rg);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final resource groups:\n");
    MArrayForEach(te->resource_groups, &rg, &i)
      KheResourceGroupDebug(rg, 2, 2, stderr);
  }

  /* set te->resource_group_indexes by resource */
  MArrayForEach(te->resource_groups, &rg, &i)
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      MArrayPut(te->resource_group_indexes_by_resource,
	KheResourceInstanceIndex(r), i);
    }

  if( DEBUG1 )
    fprintf(stderr, "] KheTimeEquivSolve returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeEquivEventGroupCount(KHE_TIME_EQUIV te)                       */
/*                                                                           */
/*  Return the number of time-equivalent event_groups of events.             */
/*                                                                           */
/*****************************************************************************/

int KheTimeEquivEventGroupCount(KHE_TIME_EQUIV te)
{
  return MArraySize(te->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheTimeEquivEventGroup(KHE_TIME_EQUIV te, int i)         */
/*                                                                           */
/*  Return the ith time-equivalent event group.                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheTimeEquivEventGroup(KHE_TIME_EQUIV te, int i)
{
  return MArrayGet(te->event_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheTimeEquivEventEventGroup(KHE_TIME_EQUIV te,           */
/*    KHE_EVENT e)                                                           */
/*                                                                           */
/*  Return the event group containing e.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheTimeEquivEventEventGroup(KHE_TIME_EQUIV te, KHE_EVENT e)
{
  return MArrayGet(te->event_groups_by_event, KheEventIndex(e));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeEquivEventEventGroupIndex(KHE_TIME_EQUIV te, KHE_EVENT e)     */
/*                                                                           */
/*  Return the index in te->event_groups of the event group containing e.    */
/*                                                                           */
/*****************************************************************************/

int KheTimeEquivEventEventGroupIndex(KHE_TIME_EQUIV te, KHE_EVENT e)
{
  return MArrayGet(te->event_group_indexes_by_event, KheEventIndex(e));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeEquivResourceGroupCount(KHE_TIME_EQUIV te)                    */
/*                                                                           */
/*  Return the number of time-equivalent resource groups.                    */
/*                                                                           */
/*****************************************************************************/

int KheTimeEquivResourceGroupCount(KHE_TIME_EQUIV te)
{
  return MArraySize(te->resource_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTimeEquivResourceGroup(KHE_TIME_EQUIV te, int i)   */
/*                                                                           */
/*  Return the ith time-equivalent class of resources.                       */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTimeEquivResourceGroup(KHE_TIME_EQUIV te, int i)
{
  return MArrayGet(te->resource_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTimeEquivResourceResourceGroup(KHE_TIME_EQUIV te,  */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the equivalence class containing r.                               */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTimeEquivResourceResourceGroup(KHE_TIME_EQUIV te,
  KHE_RESOURCE r)
{
  return MArrayGet(te->resource_groups_by_resource,
    KheResourceInstanceIndex(r));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeEquivResourceResourceGroupIndex(KHE_TIME_EQUIV te,            */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the index in te->resource_groups of the resource group            */
/*  containing r.                                                            */
/*                                                                           */
/*****************************************************************************/

int KheTimeEquivResourceResourceGroupIndex(KHE_TIME_EQUIV te,
  KHE_RESOURCE r)
{
  return MArrayGet(te->resource_group_indexes_by_resource,
    KheResourceInstanceIndex(r));
}
