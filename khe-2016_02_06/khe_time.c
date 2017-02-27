
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
/*  FILE:         khe_time.c                                                 */
/*  DESCRIPTION:  A time                                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME - a time                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_time_rec {
  void			*back;			/* back pointer              */
  KHE_INSTANCE		instance;		/* enclosing instance        */
  char			*id;			/* Id                        */
  char			*name;			/* Name                      */
  bool			break_after;		/* true if break after       */
  int			index;			/* in `all' group and lsets  */
  KHE_TIME_GROUP	singleton_time_group;	/* singleton time group      */
};


/*****************************************************************************/
/*                                                                           */
/*  Construction and query                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,                 */
/*    bool break_after, KHE_TIME *t)                                         */
/*                                                                           */
/*  Make a time object with these atrributes and add it to ins.              */
/*                                                                           */
/*****************************************************************************/

bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,
  bool break_after, KHE_TIME *t)
{
  KHE_TIME res;
  MAssert(!KheInstanceComplete(ins),
    "KheTimeMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveTime(ins, id, &res) )
  {
    *t = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->break_after = break_after;
  res->index = KheInstanceTimeCount(ins);
  res->singleton_time_group = KheTimeGroupMakeInternal(ins,
    KHE_TIME_GROUP_TYPE_SINGLETON, NULL, KHE_TIME_GROUP_KIND_ORDINARY,
    NULL, NULL, LSetNew());
  KheTimeGroupAddTimeInternal(res->singleton_time_group, res);
  KheTimeGroupAddTimeInternal(KheInstanceFullTimeGroupInternal(ins), res);
  KheInstanceAddTime(ins, res);
  *t = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSetBack(KHE_TIME t, void *back)                              */
/*                                                                           */
/*  Set the back pointer of t.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTimeSetBack(KHE_TIME t, void *back)
{
  MAssert(!KheInstanceComplete(t->instance),
    "KheTimeSetBack called after KheInstanceMakeEnd");
  t->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheTimeBack(KHE_TIME t)                                            */
/*                                                                           */
/*  Return the back pointer of t.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheTimeBack(KHE_TIME t)
{
  return t->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheTimeInstance(KHE_TIME t)                                 */
/*                                                                           */
/*  Return the enclosing instance of t.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheTimeInstance(KHE_TIME t)
{
  return t->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeId(KHE_TIME t)                                              */
/*                                                                           */
/*  Return the id attribute of t.                                            */
/*                                                                           */
/*****************************************************************************/

char *KheTimeId(KHE_TIME t)
{
  return t->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeName(KHE_TIME t)                                            */
/*                                                                           */
/*  Return the name attribute of t.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheTimeName(KHE_TIME t)
{
  return t->name;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeBreakAfter(KHE_TIME t)                                       */
/*                                                                           */
/*  Return the break_after attribute of t.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheTimeBreakAfter(KHE_TIME t)
{
  return t->break_after;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeIndex(KHE_TIME t)                                             */
/*                                                                           */
/*  Return the index number of t.                                            */
/*                                                                           */
/*****************************************************************************/

int KheTimeIndex(KHE_TIME t)
{
  return t->index;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeHasNeighbour(KHE_TIME t, int delta)                          */
/*                                                                           */
/*  Return true if t has a neighbouring time, delta places away.             */
/*                                                                           */
/*****************************************************************************/

bool KheTimeHasNeighbour(KHE_TIME t, int delta)
{
  int index;
  index = KheTimeIndex(t) + delta;
  return index >= 0 && index < KheInstanceTimeCount(t->instance);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta)                         */
/*                                                                           */
/*  Return the neighbouring time delta places away.                          */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta)
{
  MAssert(KheTimeHasNeighbour(t, delta),
    "KheTimeNeighbour: specified neighbour does not exist");
  return KheInstanceTime(t->instance, KheTimeIndex(t) + delta);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeLE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*  bool KheTimeLT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*  bool KheTimeGT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*  bool KheTimeGE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*  bool KheTimeEQ(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*  bool KheTimeNE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)   */
/*                                                                           */
/*  KheTimeNeighbour(time1, delta1) and KheTimeNeighbour(time2, delta2)      */
/*  comparison functions.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheTimeLE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 <= time2->index + delta2;
}

bool KheTimeLT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 < time2->index + delta2;
}

bool KheTimeGT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 > time2->index + delta2;
}

bool KheTimeGE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 >= time2->index + delta2;
}

bool KheTimeEQ(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 == time2->index + delta2;
}

bool KheTimeNE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2)
{
  return time1->index + delta1 != time2->index + delta2;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeIntervalsOverlap(KHE_TIME time1, int durn1,                   */
/*    KHE_TIME time2, int durn2)                                             */
/*                                                                           */
/*  Return the number of places where these two time intervals overlap.      */
/*                                                                           */
/*****************************************************************************/

int KheTimeIntervalsOverlap(KHE_TIME time1, int durn1,
  KHE_TIME time2, int durn2)
{
  int overlap_start, overlap_stop, start1, start2, stop1, stop2;

  /* the first place they can overlap is max(start1, start2) */
  start1 = time1->index;
  start2 = time2->index;
  overlap_start = (start1 < start2 ? start2 : start1);

  /* the first place they can't overlap is min(stop1, stop2) */
  stop1 = time1->index + durn1;
  stop2 = time2->index + durn2;
  overlap_stop = (stop1 < stop2 ? stop1 : stop2);

  /* the overlap is the larger of overlap_stop - overlap_start and 0 */
  return (overlap_stop <= overlap_start ? 0 : overlap_stop - overlap_start);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeIntervalsOverlap(KHE_TIME time1, int durn1,                   */
/*    KHE_TIME time2, int durn2)                                             */
/*                                                                           */
/*  Return the number of places where these two time intervals overlap.      */
/*                                                                           */
/*****************************************************************************/

bool KheTimeIntervalsOverlapInterval(KHE_TIME time1, int durn1,
  KHE_TIME time2, int durn2, KHE_TIME *overlap_time, int *overlap_durn)
{
  int overlap_start, overlap_stop, start1, start2, stop1, stop2;

  /* the first place they can overlap is max(start1, start2) */
  start1 = time1->index;
  start2 = time2->index;
  overlap_start = (start1 < start2 ? start2 : start1);

  /* the first place they can't overlap is min(stop1, stop2) */
  stop1 = time1->index + durn1;
  stop2 = time2->index + durn2;
  overlap_stop = (stop1 < stop2 ? stop1 : stop2);

  /* the overlap is the larger of overlap_stop - overlap_start and 0 */
  if( overlap_stop <= overlap_start )
    return false;
  *overlap_time = (start1 < start2 ? time2 : time1);
  *overlap_durn = overlap_stop - overlap_start;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Auto-generated time group                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t)                     */
/*                                                                           */
/*  Return a time group containing just t.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t)
{
  return t->singleton_time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Reading and writing                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins,KML_ERROR *ke)*/
/*                                                                           */
/*  Add a time to ins based on time_elt.                                     */
/*                                                                           */
/*****************************************************************************/

bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name, *ref;  KML_ELT elt, e;  KHE_TIME t;
  KHE_TIME_GROUP tg;  int j;
  if( !KmlCheck(time_elt, "Id : $Name +Week +Day +TimeGroups", ke) )
    return false;
  id = KmlExtractAttributeValue(time_elt, 0);
  name = KmlExtractText(KmlChild(time_elt, 0));
  if( !KheTimeMake(ins, id, name, false, &t) )
    return KmlError(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
      "<Time> Id \"%s\" used previously", id);

  /* link to the time's Week time subgroup */
  if( KmlContainsChild(time_elt, "Week", &e) )
  {
    if( !KmlCheck(e, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(e, 0);
    if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
      return KmlError(ke, KmlLineNum(e), KmlColNum(e),
	"<Week> Reference \"%s\" unknown", ref);
    KheTimeGroupAddTime(tg, t);
  }

  /* link to the time's Day time subgroup */
  if( KmlContainsChild(time_elt, "Day", &e) )
  {
    if( !KmlCheck(e, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(e, 0);
    if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
      return KmlError(ke, KmlLineNum(e), KmlColNum(e),
	"<Day> Reference \"%s\" unknown", ref);
    KheTimeGroupAddTime(tg, t);
  }

  /* link to the time's TimeGroup time subgroups */
  if( KmlContainsChild(time_elt, "TimeGroups", &elt) )
  {
    if( !KmlCheck(elt, ": *TimeGroup", ke) )
      return false;
    for( j = 0;  j < KmlChildCount(elt);  j++ )
    {
      e = KmlChild(elt, j);
      if( !KmlCheck(e, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(e, 0);
      if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
	return KmlError(ke, KmlLineNum(e), KmlColNum(e),
	  "<TimeGroup> Reference \"%s\" unknown", ref);
      KheTimeGroupAddTime(tg, t);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeWrite(KHE_TIME t, KML_FILE kf)                               */
/*                                                                           */
/*  Write t to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTimeWrite(KHE_TIME t, KML_FILE kf)
{
  static ARRAY_KHE_TIME_GROUP time_groups;
  KHE_TIME_GROUP tg;  int i;

  /* header and name */
  KmlBegin(kf, "Time");
  MAssert(t->id != NULL, "KheArchiveWrite: Id missing from Time");
  KmlAttribute(kf, "Id", t->id);
  MAssert(t->name != NULL, "KheArchiveWrite: Name missing from Time");
  KmlEltPlainText(kf, "Name", t->name);

  /* find the (user-defined) time groups containing t */
  MArrayInit(time_groups);
  for( i = 0;  i < KheInstanceTimeGroupCount(t->instance);  i++ )
  {
    tg = KheInstanceTimeGroup(t->instance, i);
    if( KheTimeGroupContains(tg, t) )
      MArrayAddLast(time_groups, tg);
  }

  /* if the first is a Week, remove it and print it */
  if( MArraySize(time_groups) > 0 )
  {
    tg = MArrayFirst(time_groups);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_WEEK )
    {
      MArrayRemoveFirst(time_groups);
      KmlEltAttribute(kf, "Week", "Reference", KheTimeGroupId(tg));
    }
  }

  /* if the first remaining is a Day, remove it and print it */
  if( MArraySize(time_groups) > 0 )
  {
    tg = MArrayFirst(time_groups);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_DAY )
    {
      MArrayRemoveFirst(time_groups);
      KmlEltAttribute(kf, "Day", "Reference", KheTimeGroupId(tg));
    }
  }

  /* if there are any remaining time groups, print them ordinarily */
  if( MArraySize(time_groups) > 0 )
  {
    KmlBegin(kf, "TimeGroups");
    MArrayForEach(time_groups, &tg, &i)
      KmlEltAttribute(kf, "TimeGroup", "Reference", KheTimeGroupId(tg));
    KmlEnd(kf, "TimeGroups");
  }

  /* print footer and return */
  KmlEnd(kf, "Time");
}
