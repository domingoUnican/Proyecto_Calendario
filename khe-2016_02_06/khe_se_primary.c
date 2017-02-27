
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
/*  FILE:         khe_se_grouping.c                                          */
/*  DESCRIPTION:  Primary grouping for ejection chains                       */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG10 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS (private) - a class of time-equivalent events            */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;

typedef struct khe_event_class_rec {
  ARRAY_KHE_EVENT	events;
} *KHE_EVENT_CLASS;

typedef MARRAY(KHE_EVENT_CLASS) ARRAY_KHE_EVENT_CLASS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR_AND_CLASSES - a monitor and the event classes it monitors    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_monitor_and_classes_rec {
  KHE_MONITOR			monitor;		/* the monitor       */
  ARRAY_KHE_EVENT_CLASS		event_classes;		/* its event classes */
} *KHE_MONITOR_AND_CLASSES;

typedef MARRAY(KHE_MONITOR_AND_CLASSES) ARRAY_MONITOR_AND_CLASSES;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS_SOLVER - solver object for event monitor grouping        */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_MONITOR) ARRAY_KHE_MONITOR;

typedef struct khe_event_class_solver_rec {
  KHE_SOLN			soln;
  ARRAY_KHE_EVENT_CLASS		classes_by_event;
  ARRAY_KHE_EVENT_CLASS		distinct_classes;
  ARRAY_KHE_EVENT_CLASS		final_classes;
  ARRAY_KHE_MEET		tmp_leader_meets;
  ARRAY_INT			tmp_offsets;
  ARRAY_INT			tmp_durations;
  ARRAY_INT			tmp_multiplicities;
  ARRAY_KHE_MONITOR		tmp_monitors;
  ARRAY_MONITOR_AND_CLASSES	monitor_and_classes_array;
} *KHE_EVENT_CLASS_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_CLASS - class of resource-equivalent event resources  */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_EVENT_RESOURCE) ARRAY_KHE_EVENT_RESOURCE;

typedef struct khe_event_resource_class_rec {
  ARRAY_KHE_EVENT_RESOURCE	event_resources;
} *KHE_EVENT_RESOURCE_CLASS;

typedef MARRAY(KHE_EVENT_RESOURCE_CLASS) ARRAY_KHE_EVENT_RESOURCE_CLASS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_CLASS_SOLVER - a solver for event resources           */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;

typedef struct khe_event_resource_class_solver_rec {
  KHE_SOLN				soln;
  ARRAY_KHE_EVENT_RESOURCE_CLASS	classes_by_event_resource;
  ARRAY_KHE_EVENT_RESOURCE_CLASS	distinct_classes;
  ARRAY_KHE_EVENT_RESOURCE_CLASS	final_classes;
  ARRAY_KHE_TASK			tmp_leader_tasks;
  ARRAY_INT				tmp_multiplicities;
  ARRAY_KHE_MONITOR			tmp_monitors;
} *KHE_EVENT_RESOURCE_CLASS_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_CLASS_LAYER                                                 */
/*                                                                           */
/*  A resource and the events it is preassigned to, after they have been     */
/*  adjusted to bring linked events together.                                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_resource_class_layer_rec {
  KHE_RESOURCE			resource;		/* the resource      */
  ARRAY_KHE_EVENT		events;			/* events            */
} *KHE_RESOURCE_CLASS_LAYER;

typedef MARRAY(KHE_RESOURCE_CLASS_LAYER) ARRAY_KHE_RESOURCE_CLASS_LAYER;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "helper functions for monitor grouping generally"              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorCmp(const void *t1, const void *t2)                        */
/*                                                                           */
/*  Comparison function for sorting monitors.                                */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorCmp(const void *t1, const void *t2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) t1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) t2;
  return KheMonitorSolnIndex(m1) - KheMonitorSolnIndex(m2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMonitorGroup(KHE_SOLN soln, ARRAY_KHE_MONITOR *monitors,  */
/*    int first, int last, KHE_SUBTAG_STANDARD_TYPE sub_tag, bool debug)     */
/*                                                                           */
/*  Link *monitors[first..last] to soln as a group.  If there are no         */
/*  monitors in the range first .. last inclusive, do nothing; if there      */
/*  is one monitor, link it directly as a child of soln; if there are two    */
/*  or more monitors, make a group monitor with the given sub-tag, link it   */
/*  to soln, and make *monitors[first..last] its children.                   */
/*                                                                           */
/*  The idea is that *monitors will all be unlinked from soln when this      */
/*  function is called.  But that is the caller's responsibility.            */
/*                                                                           */
/*****************************************************************************/

static void KheSolnAddMonitorGroup(KHE_SOLN soln, ARRAY_KHE_MONITOR *monitors,
  int first, int last, KHE_SUBTAG_STANDARD_TYPE sub_tag, bool debug)
{
  KHE_MONITOR m;  int i, len;  KHE_GROUP_MONITOR gm;
  len = last - first + 1;
  if( len == 1 )
  {
    m = MArrayGet(*monitors, first);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, m);
    if( debug )
      KheMonitorDebug(m, 2, 2, stderr);
  }
  else if( len >= 2 )
  {
    gm = KheGroupMonitorMake(soln, sub_tag, KheSubTagLabel(sub_tag));
    for( i = first;  i <= last;  i++ )
    {
      m = MArrayGet(*monitors, i);
      KheGroupMonitorAddChildMonitor(gm, m);
    }
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) gm);
    if( debug )
      KheMonitorDebug((KHE_MONITOR) gm, 2, 2, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitor grouping - event classes"                       */
/*                                                                           */
/*  An event class is a set of time-equivalent events, that is, events       */
/*  whose fixed meet assignments prove that they run simultaneously.         */
/*                                                                           */
/*  It is an invariant that each event e lies in exactly one class at any    */
/*  time, and that ecs->classes_by_event(KheEventIndex(e)) is that class.    */
/*  Also, whenever a class becomes empty it is deleted.                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS KheEventClassMake(void)                                  */
/*                                                                           */
/*  Make an empty event class.  (It will gain an event immediately after.)   */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_CLASS KheEventClassMake(void)
{
  KHE_EVENT_CLASS res;
  MMake(res);
  MArrayInit(res->events);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassDelete(KHE_EVENT_CLASS ec)                             */
/*                                                                           */
/*  Delete event class ec, reclaiming its memory.                            */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassDelete(KHE_EVENT_CLASS ec)
{
  MArrayFree(ec->events);
  MFree(ec);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassAddEvent(KHE_EVENT_CLASS ec, KHE_EVENT e,              */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Add e to ec, updating ecs->classes_by_event accordingly.                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassAddEvent(KHE_EVENT_CLASS ec, KHE_EVENT e,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  MArrayAddLast(ec->events, e);
  MArrayPut(ecs->classes_by_event, KheEventIndex(e), ec);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassMerge(KHE_EVENT e, KHE_EVENT other_e,                  */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Merge the event classes of e and other_e, unless already equal.          */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassMerge(KHE_EVENT_CLASS ec, KHE_EVENT other_e,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  KHE_EVENT_CLASS other_ec;  KHE_EVENT e2;  int i;
  /* ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e)); */
  other_ec = MArrayGet(ecs->classes_by_event, KheEventIndex(other_e));
  if( ec != other_ec )
  {
    MArrayForEach(other_ec->events, &e2, &i)
      KheEventClassAddEvent(ec, e2, ecs);
    KheEventClassDelete(other_ec);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMergeRelatedEvents(KHE_MEET meet, KHE_EVENT_CLASS *first_ec,     */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Merge the classes of the events that are related by fixed assignments    */
/*  to meet, directly or indirectly.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheMergeRelatedEvents(KHE_MEET meet, KHE_EVENT_CLASS *first_ec,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  int i;  KHE_MEET sub_meet;  KHE_EVENT e;

  /* take account of meet itself, if it is derived from an event */
  e = KheMeetEvent(meet);
  if( e != NULL )
  {
    if( *first_ec == NULL )
      *first_ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e));
    else
      KheEventClassMerge(*first_ec, KheMeetEvent(meet), ecs);
  }

  /* take account of the fixed assignments to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    sub_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetAssignIsFixed(sub_meet) )
      KheMergeRelatedEvents(sub_meet, first_ec, ecs);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventCmp(const void *t1, const void *t2)                          */
/*                                                                           */
/*  Comparison function for sorting an array of events.                      */
/*                                                                           */
/*****************************************************************************/

static int KheEventCmp(const void *t1, const void *t2)
{
  KHE_EVENT e1 = * (KHE_EVENT *) t1;
  KHE_EVENT e2 = * (KHE_EVENT *) t2;
  return KheEventIndex(e1) - KheEventIndex(e2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventClassCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting an array of (sorted) event classes.      */
/*                                                                           */
/*****************************************************************************/

static int KheEventClassCmp(const void *t1, const void *t2)
{
  int i;  KHE_EVENT e1, e2;
  KHE_EVENT_CLASS ec1 = * (KHE_EVENT_CLASS *) t1;
  KHE_EVENT_CLASS ec2 = * (KHE_EVENT_CLASS *) t2;
  if( MArraySize(ec1->events) != MArraySize(ec2->events) )
    return MArraySize(ec1->events) - MArraySize(ec2->events);
  for( i = 0;  i < MArraySize(ec1->events);  i++ )
  {
    e1 = MArrayGet(ec1->events, i);
    e2 = MArrayGet(ec2->events, i);
    if( e1 != e2 )
      return KheEventIndex(e1) - KheEventIndex(e2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassDebug(KHE_EVENT_CLASS ec, int indent, FILE *fp)        */
/*                                                                           */
/*  Debug print of ec onto fp with the given indent.                         */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassDebug(KHE_EVENT_CLASS ec, int indent, FILE *fp)
{
  KHE_EVENT e;  int i;
  if( indent >= 0 )
    fprintf(fp, "%*s", indent, "");
  fprintf(fp, "{");
  MArrayForEach(ec->events, &e, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    fprintf(fp, "%s", KheEventName(e));
  }
  fprintf(fp, "}");
  if( indent >= 0 )
    fprintf(fp, "\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitor grouping - monitor-and-classes"                 */
/*                                                                           */
/*  A monitor-and-classes object is an event monitor plus the classes of     */
/*  the events it monitors.                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR_AND_CLASSES KheMonitorAndClassesMake(KHE_MONITOR m)          */
/*                                                                           */
/*  Make a monitor-and-classes object for monitor m.                         */
/*                                                                           */
/*****************************************************************************/

static KHE_MONITOR_AND_CLASSES KheMonitorAndClassesMake(KHE_MONITOR m)
{
  KHE_MONITOR_AND_CLASSES res;
  MMake(res);
  res->monitor = m;
  MArrayInit(res->event_classes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAndClassesAddClass(KHE_MONITOR_AND_CLASSES mac,           */
/*    KHE_EVENT_CLASS ec)                                                    */
/*                                                                           */
/*  Add ec to mac.                                                           */
/*                                                                           */
/*****************************************************************************/

static void KheMonitorAndClassesAddClass(KHE_MONITOR_AND_CLASSES mac,
  KHE_EVENT_CLASS ec)
{
  MArrayAddLast(mac->event_classes, ec);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAndClassesSortClasses(KHE_MONITOR_AND_CLASSES mac)        */
/*                                                                           */
/*  Sort the event classes of mac into a canonical order.                    */
/*                                                                           */
/*****************************************************************************/

static void KheMonitorAndClassesSortClasses(KHE_MONITOR_AND_CLASSES mac)
{
  MArraySort(mac->event_classes, &KheEventClassCmp);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorAndClassesCmp(const void *t1, const void *t2)              */
/*                                                                           */
/*  Comparison function for sorting monitors.                                */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorAndClassesCmp(const void *t1, const void *t2)
{
  KHE_MONITOR_AND_CLASSES mac1 = * (KHE_MONITOR_AND_CLASSES *) t1;
  KHE_MONITOR_AND_CLASSES mac2 = * (KHE_MONITOR_AND_CLASSES *) t2;
  int i, cmp;  KHE_EVENT_CLASS ec1, ec2;
  if( MArraySize(mac1->event_classes) != MArraySize(mac2->event_classes) )
    return MArraySize(mac1->event_classes) - MArraySize(mac2->event_classes);
  for( i = 0;  i < MArraySize(mac1->event_classes);  i++ )
  {
    ec1 = MArrayGet(mac1->event_classes, i);
    ec2 = MArrayGet(mac2->event_classes, i);
    cmp = KheEventClassCmp((void *) &ec1, (void *) &ec2);
    if( cmp != 0 )
      return cmp;
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorAndClassesGroupable(KHE_MONITOR_AND_CLASSES mac1,         */
/*    KHE_MONITOR_AND_CLASSES mac2)                                          */
/*                                                                           */
/*  Return true if mac1 and mac2 are groupable, because the have the         */
/*  same (sorted) event classes.                                             */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorAndClassesGroupable(KHE_MONITOR_AND_CLASSES mac1,
  KHE_MONITOR_AND_CLASSES mac2)
{
  void *t1 = mac1;
  void *t2 = mac2;
  return KheMonitorAndClassesCmp(&t1, &t2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAndClassesDelete(KHE_MONITOR_AND_CLASSES mac)             */
/*                                                                           */
/*  Delete monitor-and-classes object mac, reclaiming its memory.            */
/*                                                                           */
/*****************************************************************************/

static void KheMonitorAndClassesDelete(KHE_MONITOR_AND_CLASSES mac)
{
  MArrayFree(mac->event_classes);
  MFree(mac);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAndClassesDebug(KHE_MONITOR_AND_CLASSES mac,              */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of mac onto fp with the given indent.                        */
/*                                                                           */
/*****************************************************************************/

static void KheMonitorAndClassesDebug(KHE_MONITOR_AND_CLASSES mac,
  int indent, FILE *fp)
{
  KHE_EVENT_CLASS ec;  int i;
  fprintf(fp, "%*s[ MonitorAndClasses\n", indent, "");
  KheMonitorDebug(mac->monitor, 2, indent + 2, fp);
  MArrayForEach(mac->event_classes, &ec, &i)
    KheEventClassDebug(ec, indent + 2, fp);
  fprintf(fp, "%*s]\n", indent, "");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitor grouping - event class solver"                  */
/*                                                                           */
/*  An event class solver is an object used when solving the problem         */
/*  of grouping event monitors.                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS_SOLVER KheEventClassSolverMake()                         */
/*                                                                           */
/*  Make a new event class solver.                                           */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_CLASS_SOLVER KheEventClassSolverMake(KHE_SOLN soln)
{
  KHE_EVENT_CLASS_SOLVER res;
  MMake(res);
  res->soln = soln;
  MArrayInit(res->classes_by_event);
  MArrayInit(res->distinct_classes);
  MArrayInit(res->final_classes);
  MArrayInit(res->tmp_leader_meets);
  MArrayInit(res->tmp_offsets);
  MArrayInit(res->tmp_durations);
  MArrayInit(res->tmp_multiplicities);
  MArrayInit(res->tmp_monitors);
  MArrayInit(res->monitor_and_classes_array);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassSolverDelete(KHE_EVENT_CLASS_SOLVER ecs)               */
/*                                                                           */
/*  Delete ecs, reclaiming its memory.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassSolverDelete(KHE_EVENT_CLASS_SOLVER ecs)
{
  MArrayFree(ecs->classes_by_event);
  MArrayFree(ecs->distinct_classes);
  MArrayFree(ecs->final_classes);
  MArrayFree(ecs->tmp_leader_meets);
  MArrayFree(ecs->tmp_offsets);
  MArrayFree(ecs->tmp_durations);
  MArrayFree(ecs->tmp_multiplicities);
  MArrayFree(ecs->monitor_and_classes_array);
  MFree(ecs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventClassAccepts(KHE_EVENT_CLASS_SOLVER ecs,                    */
/*    KHE_EVENT_CLASS ec, KHE_EVENT e)                                       */
/*                                                                           */
/*  Return true if e is running at the same times as the events of ec.       */
/*                                                                           */
/*****************************************************************************/

static bool KheEventClassAccepts(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_EVENT_CLASS ec, KHE_EVENT e)
{
  int j, k, offset;  KHE_MEET meet, leader_meet;  KHE_EVENT ecs_e;

  /* find the leader meets, durations, and offsets of the first event of ecs */
  MArrayClear(ecs->tmp_durations);
  MArrayClear(ecs->tmp_leader_meets);
  MArrayClear(ecs->tmp_offsets);
  MArrayClear(ecs->tmp_multiplicities);
  ecs_e = MArrayFirst(ec->events);
  for( j = 0;  j < KheEventMeetCount(ecs->soln, ecs_e);  j++ )
  {
    meet = KheEventMeet(ecs->soln, ecs_e, j);
    leader_meet = KheMeetFirstMovable(meet, &offset);
    if( leader_meet == NULL )
      return false;
    MArrayAddLast(ecs->tmp_durations, KheMeetDuration(meet));
    MArrayAddLast(ecs->tmp_leader_meets, leader_meet);
    MArrayAddLast(ecs->tmp_offsets, offset);
    MArrayAddLast(ecs->tmp_multiplicities, 1);
  }

  /* check consistency with the incoming meet */
  for( j = 0;  j < KheEventMeetCount(ecs->soln, e);  j++ )
  {
    meet = KheEventMeet(ecs->soln, e, j);

    /* search for a leader meet that meet matches with, not already nabbed */
    leader_meet = KheMeetFirstMovable(meet, &offset);
    if( leader_meet == NULL )
      return false;
    for( k = 0;  k < MArraySize(ecs->tmp_leader_meets);  k++ )
      if( KheMeetDuration(meet) == MArrayGet(ecs->tmp_durations, k) &&
	  leader_meet == MArrayGet(ecs->tmp_leader_meets, k) &&
	  offset == MArrayGet(ecs->tmp_offsets, k) &&
	  1 == MArrayGet(ecs->tmp_multiplicities, k) )
	break;

    /* if no luck, exit with failure */
    if( k >= MArraySize(ecs->tmp_leader_meets) )
      return false;

    /* success, but nab index k so it isn't matched twice by e's meets */
    MArrayPreInc(ecs->tmp_multiplicities, k);
  }

  /* all correct */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassBreakUp(KHE_EVENT_CLASS_SOLVER ecs,                    */
/*    KHE_EVENT_CLASS ec)                                                    */
/*                                                                           */
/*  Check whether the events of event class ec really are constrained to     */
/*  run at the same time, and break up the class appropriately if not.       */
/*  Place the final classes onto the end of ecs->final_classes.              */
/*                                                                           */
/*  This function also deletes ec afterwards, since it's no longer needed.   */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassBreakUp(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_EVENT_CLASS ec)
{
  int start, i, j;  KHE_EVENT e;  KHE_EVENT_CLASS ec2;
  start = MArraySize(ecs->final_classes);
  MArrayForEach(ec->events, &e, &i)
  {
    for( j = start;  j < MArraySize(ecs->final_classes);  j++ )
    {
      ec2 = MArrayGet(ecs->final_classes, j);
      if( KheEventClassAccepts(ecs, ec2, e) )
      {
        KheEventClassAddEvent(ec2, e, ecs);
	break;
      }
    }
    if( j >= MArraySize(ecs->final_classes) )
    {
      ec2 = KheEventClassMake();
      KheEventClassAddEvent(ec2, e, ecs);
      MArrayAddLast(ecs->final_classes, ec2);
    }
  }
  KheEventClassDelete(ec);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitor grouping - main functions"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventDoSplitGroup(KHE_EVENT_CLASS_SOLVER ecs,                    */
/*    KHE_EVENT_CLASS ec)                                                    */
/*                                                                           */
/*  Group together the split events and distribute split events monitors of  */
/*  ec.  Also remove all parents from these monitors.                        */
/*                                                                           */
/*****************************************************************************/

static void KheEventDoSplitGroup(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_EVENT_CLASS ec)
{
  KHE_EVENT e;  int i, j;  KHE_MONITOR m;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheEventDoSplitGroup(ecs, ");
    KheEventClassDebug(ec, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MArrayClear(ecs->tmp_monitors);
  MArrayForEach(ec->events, &e, &i)
    for( j = 0;  j < KheSolnEventMonitorCount(ecs->soln, e);  j++ )
    {
      m = KheSolnEventMonitor(ecs->soln, e, j);
      if( KheMonitorTag(m) == KHE_SPLIT_EVENTS_MONITOR_TAG ||
          KheMonitorTag(m) == KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	MArrayAddLast(ecs->tmp_monitors, m);
      }
    }
  MArraySortUnique(ecs->tmp_monitors, &KheMonitorCmp);
  KheSolnAddMonitorGroup(ecs->soln, &ecs->tmp_monitors,
    0, MArraySize(ecs->tmp_monitors) - 1, KHE_SUBTAG_SPLIT_EVENTS, DEBUG2);
  if( DEBUG2 )
    fprintf(stderr, "] KheEventDoSplitGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventDoAssignTimeGroup(KHE_EVENT_CLASS_SOLVER ecs,               */
/*    KHE_EVENT_CLASS ec)                                                    */
/*                                                                           */
/*  Group together the assign time monitors of ec.                           */
/*  Also remove all parents from these monitors.                             */
/*                                                                           */
/*****************************************************************************/

static void KheEventDoAssignTimeGroup(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_EVENT_CLASS ec)
{
  KHE_EVENT e;  int i, j;  KHE_MONITOR m;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheEventDoAssignTimeGroup(ecs, ");
    KheEventClassDebug(ec, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MArrayClear(ecs->tmp_monitors);
  MArrayForEach(ec->events, &e, &i)
    for( j = 0;  j < KheSolnEventMonitorCount(ecs->soln, e);  j++ )
    {
      m = KheSolnEventMonitor(ecs->soln, e, j);
      if( KheMonitorTag(m) == KHE_ASSIGN_TIME_MONITOR_TAG )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	MArrayAddLast(ecs->tmp_monitors, m);
      }
    }
  MArraySortUnique(ecs->tmp_monitors, &KheMonitorCmp);
  KheSolnAddMonitorGroup(ecs->soln, &ecs->tmp_monitors,
    0, MArraySize(ecs->tmp_monitors) - 1, KHE_SUBTAG_ASSIGN_TIME, DEBUG2);
  if( DEBUG2 )
    fprintf(stderr, "] KheEventDoAssignTimeGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesMonitorCmp(const void *t1, const void *t2)             */
/*                                                                           */
/*  Comparison function for sorting an array of prefer times monitors        */
/*  so as to bring together monitors which request the same set of times.    */
/*                                                                           */
/*****************************************************************************/

static int KhePreferTimesMonitorCmp(const void *t1, const void *t2)
{
  KHE_PREFER_TIMES_MONITOR m1 = * (KHE_PREFER_TIMES_MONITOR *) t1;
  KHE_PREFER_TIMES_MONITOR m2 = * (KHE_PREFER_TIMES_MONITOR *) t2;
  KHE_PREFER_TIMES_CONSTRAINT c1 = KhePreferTimesMonitorConstraint(m1);
  KHE_PREFER_TIMES_CONSTRAINT c2 = KhePreferTimesMonitorConstraint(m2);
  KHE_TIME_GROUP tg1 = KhePreferTimesConstraintDomain(c1);
  KHE_TIME_GROUP tg2 = KhePreferTimesConstraintDomain(c2);
  KHE_TIME time1, time2;  int i;
  if( KheTimeGroupTimeCount(tg1) != KheTimeGroupTimeCount(tg2) )
    return KheTimeGroupTimeCount(tg1) - KheTimeGroupTimeCount(tg2);
  for( i = 0;  i < KheTimeGroupTimeCount(tg1);  i++ )
  {
    time1 = KheTimeGroupTime(tg1, i);
    time2 = KheTimeGroupTime(tg2, i);
    if( time1 != time2 )
      return KheTimeIndex(time1) - KheTimeIndex(time2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesMonitorEqualTimeGroups(KHE_MONITOR m1, KHE_MONITOR m2)*/
/*                                                                           */
/*  Return true if m1 and m2 (assumed to be prefer times monitors) request   */
/*  the same time groups.                                                    */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesMonitorEqualTimeGroups(KHE_MONITOR m1, KHE_MONITOR m2)
{
  KHE_MONITOR tm1, tm2;
  tm1 = m1;  tm2 = m2;
  return KhePreferTimesMonitorCmp(&tm1, &tm2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventDoPreferTimesGroup(KHE_EVENT_CLASS_SOLVER ecs,              */
/*    KHE_EVENT_CLASS ec)                                                    */
/*                                                                           */
/*  Group together those prefer times monitors of ec that request the same   */
/*  times.                                                                   */
/*                                                                           */
/*****************************************************************************/

static void KheEventDoPreferTimesGroup(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_EVENT_CLASS ec)
{
  KHE_EVENT e;  int i, j;  KHE_MONITOR m, m2;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheEventDoPreferTimesGroup(ecs, ");
    KheEventClassDebug(ec, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* for each monitor of ec with the given tag, remove parents and save */
  /* in tmp_monitors */
  MArrayClear(ecs->tmp_monitors);
  MArrayForEach(ec->events, &e, &i)
    for( j = 0;  j < KheSolnEventMonitorCount(ecs->soln, e);  j++ )
    {
      m = KheSolnEventMonitor(ecs->soln, e, j);
      if( KheMonitorTag(m) == KHE_PREFER_TIMES_MONITOR_TAG )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	MArrayAddLast(ecs->tmp_monitors, m);
      }
    }
  MArraySortUnique(ecs->tmp_monitors, &KheMonitorCmp);

  /* sort tmp_monitors to bring equal time groups together */
  MArraySort(ecs->tmp_monitors, &KhePreferTimesMonitorCmp);

  /* visit each run of monitors with equal time groups and group them */
  for( i = 0;  i < MArraySize(ecs->tmp_monitors);  i = j )
  {
    m = MArrayGet(ecs->tmp_monitors, i);
    for( j = i + 1;  j < MArraySize(ecs->tmp_monitors);  j++ )
    {
      m2 = MArrayGet(ecs->tmp_monitors, j);
      if( !KhePreferTimesMonitorEqualTimeGroups(m, m2) )
	break;
    }
    KheSolnAddMonitorGroup(ecs->soln, &ecs->tmp_monitors, i, j-1,
      KHE_SUBTAG_PREFER_TIMES, DEBUG2);
  }

  if( DEBUG2 )
    fprintf(stderr, "] KheEventDoPreferTimesGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheFindAllEventMonitors(KHE_EVENT_CLASS_SOLVER ecs,                 */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Set ecs->tmp_monitors to the set of all event monitors in soln with      */
/*  the given tag.  Also remove all parents from these monitors.             */
/*                                                                           */
/*****************************************************************************/

static void KheFindAllEventMonitors(KHE_EVENT_CLASS_SOLVER ecs,
  KHE_MONITOR_TAG tag)
{
  KHE_EVENT e;  KHE_INSTANCE ins;  int i, j;  KHE_MONITOR m;
  if( DEBUG3 )
    fprintf(stderr, "[ KheFindAllEventMonitors(ecs, %s)\n",
      KheMonitorTagShow(tag));
  MArrayClear(ecs->tmp_monitors);
  ins = KheSolnInstance(ecs->soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheSolnEventMonitorCount(ecs->soln, e);  j++ )
    {
      m = KheSolnEventMonitor(ecs->soln, e, j);
      if( KheMonitorTag(m) == tag )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	MArrayAddLast(ecs->tmp_monitors, m);
      }
    }
  }
  MArraySortUnique(ecs->tmp_monitors, &KheMonitorCmp);
  if( DEBUG3 )
  {
    MArrayForEach(ecs->tmp_monitors, &m, &i)
      KheMonitorDebug(m, 2, 2, stderr);
    fprintf(stderr, "] KheFindAllEventMonitors returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupSpreadEventsMonitors(KHE_EVENT_CLASS_SOLVER ecs)            */
/*                                                                           */
/*  Group equivalent attached spread events monitors.                        */
/*                                                                           */
/*****************************************************************************/

static void KheGroupSpreadEventsMonitors(KHE_EVENT_CLASS_SOLVER ecs)
{
  int i, j, k;  KHE_MONITOR m;  KHE_SPREAD_EVENTS_MONITOR sem;
  KHE_MONITOR_AND_CLASSES mac, first_mac;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_CLASS ec;  KHE_GROUP_MONITOR gm;
  if( DEBUG4 )
    fprintf(stderr, "[ KheGroupSpreadEventsMonitors(ecs)\n");

  /* find all the spread events monitors */
  KheFindAllEventMonitors(ecs, KHE_SPREAD_EVENTS_MONITOR_TAG);

  /* build one monitor-and-class object for each monitor */
  MArrayForEach(ecs->tmp_monitors, &m, &i)
  {
    mac = KheMonitorAndClassesMake(m);
    MArrayAddLast(ecs->monitor_and_classes_array, mac);
    sem = (KHE_SPREAD_EVENTS_MONITOR) m;
    eg = KheSpreadEventsMonitorEventGroup(sem);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e));
      KheMonitorAndClassesAddClass(mac, ec);
    }
    KheMonitorAndClassesSortClasses(mac);
  }

  /* sort monitor-and-classes array to bring equal sets of classes together */
  MArraySort(ecs->monitor_and_classes_array, &KheMonitorAndClassesCmp);
  if( DEBUG4 )
  {
    fprintf(stderr, "  sorted monitors and classes:\n");
    MArrayForEach(ecs->monitor_and_classes_array, &mac, &i)
      KheMonitorAndClassesDebug(mac, 2, stderr);
  }

  /* group adjacent monitor objects with equal event classes */
  if( DEBUG4 )
    fprintf(stderr, "  grouped monitors:\n");
  for( i = 0;  i < MArraySize(ecs->monitor_and_classes_array);  i = j )
  {
    /* find a maximum run of adjacent groupable macs starting at i */
    first_mac = MArrayGet(ecs->monitor_and_classes_array, i);
    for( j = i + 1;  j < MArraySize(ecs->monitor_and_classes_array);  j++ )
    {
      mac = MArrayGet(ecs->monitor_and_classes_array, j);
      if( !KheMonitorAndClassesGroupable(first_mac, mac) )
	break;
    }

    /* here (j-i) >= 1 and the (j-i) macs [i .. j-1] are groupable */
    if( j - i == 1 )
    {
      /* one monitor, make it a child of the solution */
      mac = MArrayGet(ecs->monitor_and_classes_array, i);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) ecs->soln,
	mac->monitor);
      if( DEBUG4 )
	KheMonitorDebug(mac->monitor, 2, 2, stderr);
    }
    else
    {
      /* two or more monitors, make a new group monitor */
      gm = KheGroupMonitorMake(ecs->soln, KHE_SUBTAG_SPREAD_EVENTS,
	KheSubTagLabel(KHE_SUBTAG_SPREAD_EVENTS));
      for( k = i;  k < j;  k++ )
      {
	mac = MArrayGet(ecs->monitor_and_classes_array, k);
	KheGroupMonitorAddChildMonitor(gm, mac->monitor);
      }
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) ecs->soln,
	(KHE_MONITOR) gm);
      if( DEBUG4 )
	KheMonitorDebug((KHE_MONITOR) gm, 2, 2, stderr);
    }
  }

  /* remove monitor-and-classes objects and return */
  MArrayForEach(ecs->monitor_and_classes_array, &mac, &i)
    KheMonitorAndClassesDelete(mac);
  MArrayClear(ecs->monitor_and_classes_array);
  if( DEBUG4 )
    fprintf(stderr, "] KheGroupSpreadEventsMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupOrderEventsMonitors(KHE_EVENT_CLASS_SOLVER ecs)             */
/*                                                                           */
/*  Group equivalent attached order events monitors.                         */
/*                                                                           */
/*  Implementation note.  This code is untested.  It is similar to the       */
/*  code for spread events monitors, the main difference being that it       */
/*  does not sort the event classes associated with each monitor.            */
/*                                                                           */
/*****************************************************************************/

static void KheGroupOrderEventsMonitors(KHE_EVENT_CLASS_SOLVER ecs)
{
  int i, j, k;  KHE_MONITOR m;  KHE_ORDER_EVENTS_MONITOR oem;
  KHE_MONITOR_AND_CLASSES mac, first_mac;  KHE_EVENT e;
  KHE_EVENT_CLASS ec;  KHE_GROUP_MONITOR gm;
  if( DEBUG5 )
    fprintf(stderr, "[ KheGroupOrderEventsMonitors(ecs)\n");

  /* find all the order events monitors */
  KheFindAllEventMonitors(ecs, KHE_ORDER_EVENTS_MONITOR_TAG);

  /* build one monitor-and-class object for each monitor */
  MArrayForEach(ecs->tmp_monitors, &m, &i)
  {
    mac = KheMonitorAndClassesMake(m);
    MArrayAddLast(ecs->monitor_and_classes_array, mac);
    oem = (KHE_ORDER_EVENTS_MONITOR) m;
    e = KheOrderEventsMonitorFirstEvent(oem);
    ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e));
    KheMonitorAndClassesAddClass(mac, ec);
    e = KheOrderEventsMonitorSecondEvent(oem);
    ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e));
    KheMonitorAndClassesAddClass(mac, ec);
    /* no sorting this time KheMonitorAndClassesSortClasses(mac); */
  }

  /* sort monitor-and-classes array to bring equal sets of classes together */
  MArraySort(ecs->monitor_and_classes_array, &KheMonitorAndClassesCmp);
  if( DEBUG5 )
  {
    fprintf(stderr, "  sorted order events monitors and classes:\n");
    MArrayForEach(ecs->monitor_and_classes_array, &mac, &i)
      KheMonitorAndClassesDebug(mac, 2, stderr);
  }

  /* group adjacent monitor objects with equal event classes */
  if( DEBUG5 )
    fprintf(stderr, "  grouped order events monitors:\n");
  for( i = 0;  i < MArraySize(ecs->monitor_and_classes_array);  i = j )
  {
    /* find a maximum run of adjacent groupable macs starting at i */
    first_mac = MArrayGet(ecs->monitor_and_classes_array, i);
    for( j = i + 1;  j < MArraySize(ecs->monitor_and_classes_array);  j++ )
    {
      mac = MArrayGet(ecs->monitor_and_classes_array, j);
      if( !KheMonitorAndClassesGroupable(first_mac, mac) )
	break;
    }

    /* here (j-i) >= 1 and the (j-i) macs [i .. j-1] are groupable */
    if( j - i == 1 )
    {
      /* one monitor, make it a child of the solution */
      mac = MArrayGet(ecs->monitor_and_classes_array, i);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) ecs->soln,
	mac->monitor);
      if( DEBUG5 )
	KheMonitorDebug(mac->monitor, 2, 2, stderr);
    }
    else
    {
      /* two or more monitors, make a new group monitor */
      gm = KheGroupMonitorMake(ecs->soln, KHE_SUBTAG_ORDER_EVENTS,
	KheSubTagLabel(KHE_SUBTAG_ORDER_EVENTS));
      for( k = i;  k < j;  k++ )
      {
	mac = MArrayGet(ecs->monitor_and_classes_array, k);
	KheGroupMonitorAddChildMonitor(gm, mac->monitor);
      }
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) ecs->soln,
	(KHE_MONITOR) gm);
      if( DEBUG5 )
	KheMonitorDebug((KHE_MONITOR) gm, 2, 2, stderr);
    }
  }

  /* remove monitor-and-classes objects and return */
  MArrayForEach(ecs->monitor_and_classes_array, &mac, &i)
    KheMonitorAndClassesDelete(mac);
  MArrayClear(ecs->monitor_and_classes_array);
  if( DEBUG5 )
    fprintf(stderr, "] KheGroupOrderEventsMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS_SOLVER KhePrepareEventMonitors(KHE_SOLN soln)            */
/*                                                                           */
/*  Group the event monitors of soln, and return the event class solver      */
/*  used when doing it (it is re-used when grouping resource monitors).      */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_CLASS_SOLVER KhePrepareEventMonitors(KHE_SOLN soln)
{
  int i;  KHE_INSTANCE ins;  KHE_EVENT e;  KHE_EVENT_CLASS ec;
  KHE_MEET meet;  KHE_EVENT_CLASS_SOLVER ecs;

  /* build an initial set of event classes, one per event */
  if( DEBUG1 )
    fprintf(stderr, "[ KhePrepareEventMonitors(soln)\n");
  ecs = KheEventClassSolverMake(soln);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    ec = KheEventClassMake();
    MArrayAddLast(ecs->classes_by_event, ec);
    KheEventClassAddEvent(ec, e, ecs);
  }

  /* tentatively merge classes that might be time-equivalent */
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet = KheSolnMeet(soln, i);
    if( !KheMeetIsCycleMeet(meet) && !KheMeetAssignIsFixed(meet) )
    {
      ec = NULL;
      KheMergeRelatedEvents(meet, &ec, ecs);
    }
  }

  /* build a separate array of all distinct classes */
  MArrayInit(ecs->distinct_classes);
  MArrayForEach(ecs->classes_by_event, &ec, &i)
    if( i == KheEventIndex(MArrayFirst(ec->events)) )
      MArrayAddLast(ecs->distinct_classes, ec);
  if( DEBUG2 )
  {
    fprintf(stderr, "  distinct classes:\n");
    MArrayForEach(ecs->distinct_classes, &ec, &i)
      KheEventClassDebug(ec, 2, stderr);
  }

  /* break up classes that don't work; sort the events of the final classes */
  MArrayForEach(ecs->distinct_classes, &ec, &i)
    KheEventClassBreakUp(ecs, ec);
  MArrayForEach(ecs->final_classes, &ec, &i)
    MArraySort(ec->events, &KheEventCmp);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final classes:\n");
    MArrayForEach(ecs->final_classes, &ec, &i)
      KheEventClassDebug(ec, 2, stderr);
  }

  /* group split and distribute split, assign time, and prefer times monitors */
  MArrayForEach(ecs->final_classes, &ec, &i)
  {
    KheEventDoSplitGroup(ecs, ec);
    KheEventDoAssignTimeGroup(ecs, ec);
    KheEventDoPreferTimesGroup(ecs, ec);
  }

  /* group spread events monitors and order events monitors */
  KheGroupSpreadEventsMonitors(ecs);
  KheGroupOrderEventsMonitors(ecs);
  return ecs;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnPrepareEventMonitors(KHE_SOLN soln)                            */
/*                                                                           */
/*  Remove the event monitors added (probably) by KhePrepareEventMonitors.   */
/*                                                                           */
/*****************************************************************************/

static void KheUnPrepareEventMonitors(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  int i, j;  KHE_EVENT e;  KHE_MONITOR m;
  KHE_GROUP_MONITOR gm;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheSolnEventMonitorCount(soln, e);  j++ )
    {
      m = KheSolnEventMonitor(soln, e, j);
      switch( KheMonitorTag(m) )
      {
	case KHE_ASSIGN_TIME_MONITOR_TAG:

	  if( KheMonitorHasParent(m, KHE_SUBTAG_PREFER_TIMES, &gm) )
	    KheGroupMonitorBypassAndDelete(gm);
	  break;

	case KHE_PREFER_TIMES_MONITOR_TAG:

	  if( KheMonitorHasParent(m, KHE_SUBTAG_PREFER_TIMES, &gm) )
	    KheGroupMonitorBypassAndDelete(gm);
	  break;

	case KHE_SPREAD_EVENTS_MONITOR_TAG:

	  if( KheMonitorHasParent(m, KHE_SUBTAG_SPREAD_EVENTS, &gm) )
	    KheGroupMonitorBypassAndDelete(gm);
	  break;

	case KHE_ORDER_EVENTS_MONITOR_TAG:

	  if( KheMonitorHasParent(m, KHE_SUBTAG_ORDER_EVENTS, &gm) )
	    KheGroupMonitorBypassAndDelete(gm);
	  break;

	default:

	  /* not interested */
	  break;
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource monitor grouping - event resource classes"     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_CLASS KheEventResourceClassMake(void)                 */
/*                                                                           */
/*  Make an empty event resource class.  (It will gain an event resource     */
/*  immediately after.)                                                      */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_RESOURCE_CLASS KheEventResourceClassMake(void)
{
  KHE_EVENT_RESOURCE_CLASS res;
  MMake(res);
  MArrayInit(res->event_resources);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassDelete(KHE_EVENT_RESOURCE_CLASS erc)           */
/*                                                                           */
/*  Delete event resource class erc, reclaiming its memory.                  */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassDelete(KHE_EVENT_RESOURCE_CLASS erc)
{
  MArrayFree(erc->event_resources);
  MFree(erc);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassAddEventResource(KHE_EVENT_RESOURCE_CLASS erc, */
/*    KHE_EVENT_RESOURCE er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)           */
/*                                                                           */
/*  Add er to erc, updating ercs->classes_by_event_resource accordingly.     */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassAddEventResource(KHE_EVENT_RESOURCE_CLASS erc,
  KHE_EVENT_RESOURCE er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)
{
  MArrayAddLast(erc->event_resources, er);
  MArrayPut(ercs->classes_by_event_resource,
    KheEventResourceInstanceIndex(er), erc);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassMerge(KHE_EVENT_RESOURCE er,                   */
/*    KHE_EVENT_RESOURCE other_er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)     */
/*                                                                           */
/*  Merge the classes of er and other_er, unless already equal.              */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassMerge(KHE_EVENT_RESOURCE er,
  KHE_EVENT_RESOURCE other_er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)
{
  KHE_EVENT_RESOURCE_CLASS erc, other_erc;  KHE_EVENT_RESOURCE er2;  int i;
  erc = MArrayGet(ercs->classes_by_event_resource,
    KheEventResourceInstanceIndex(er));
  other_erc = MArrayGet(ercs->classes_by_event_resource,
    KheEventResourceInstanceIndex(other_er));
  if( erc != other_erc )
  {
    MArrayForEach(other_erc->event_resources, &er2, &i)
      KheEventResourceClassAddEventResource(erc, er2, ercs);
    KheEventResourceClassDelete(other_erc);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMergeRelatedEventResources(KHE_TASK task,                        */
/*    KHE_EVENT_RESOURCE *first_er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)    */
/*                                                                           */
/*  Merge the classes of the event resources that are related by fixed       */
/*  assignments to task, directly or indirectly.                             */
/*                                                                           */
/*****************************************************************************/

static void KheMergeRelatedEventResources(KHE_TASK task,
  KHE_EVENT_RESOURCE *first_er, KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)
{
  int i;  KHE_TASK sub_task;

  /* take account of task itself, if it is derived from an event */
  if( KheTaskEventResource(task) != NULL )
  {
    if( *first_er == NULL )
      *first_er = KheTaskEventResource(task);
    else
      KheEventResourceClassMerge(*first_er, KheTaskEventResource(task), ercs);
  }

  /* take account of the fixed assignments to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    sub_task = KheTaskAssignedTo(task, i);
    if( KheTaskAssignIsFixed(sub_task) )
      KheMergeRelatedEventResources(sub_task, first_er, ercs);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceCmp(const void *t1, const void *t2)                  */
/*                                                                           */
/*  Comparison function for sorting an array of event resources.             */
/*                                                                           */
/*****************************************************************************/

static int KheEventResourceCmp(const void *t1, const void *t2)
{
  KHE_EVENT_RESOURCE er1 = * (KHE_EVENT_RESOURCE *) t1;
  KHE_EVENT_RESOURCE er2 = * (KHE_EVENT_RESOURCE *) t2;
  return KheEventResourceInstanceIndex(er1)-KheEventResourceInstanceIndex(er2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassDebug(KHE_EVENT_RESOURCE_CLASS erc,            */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of erc onto fp with the given indent.                        */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassDebug(KHE_EVENT_RESOURCE_CLASS erc,
  int indent, FILE *fp)
{
  KHE_EVENT_RESOURCE er;  int i;
  if( indent >= 0 )
    fprintf(fp, "%*s", indent, "");
  fprintf(fp, "{");
  MArrayForEach(erc->event_resources, &er, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    fprintf(fp, "%s", KheEventName(KheEventResourceEvent(er)));
  }
  fprintf(fp, "}");
  if( indent >= 0 )
    fprintf(fp, "\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource monitor grouping - event resource solver"      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_CLASS_SOLVER KheEventResourceClassSolverMake(         */
/*    KHE_SOLN soln)                                                         */
/*                                                                           */
/*  Make a new event resource class solver.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_RESOURCE_CLASS_SOLVER KheEventResourceClassSolverMake(
  KHE_SOLN soln)
{
  KHE_EVENT_RESOURCE_CLASS_SOLVER res;
  MMake(res);
  res->soln = soln;
  MArrayInit(res->classes_by_event_resource);
  MArrayInit(res->distinct_classes);
  MArrayInit(res->final_classes);
  MArrayInit(res->tmp_leader_tasks);
  MArrayInit(res->tmp_multiplicities);
  MArrayInit(res->tmp_monitors);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassSolverDelete(                                  */
/*    KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)                                  */
/*                                                                           */
/*  Delete ercs, reclaiming its memory.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassSolverDelete(
  KHE_EVENT_RESOURCE_CLASS_SOLVER ercs)
{
  KHE_EVENT_RESOURCE_CLASS erc;  int i;
  MArrayFree(ercs->classes_by_event_resource);
  MArrayFree(ercs->distinct_classes);
  MArrayForEach(ercs->final_classes, &erc, &i)
    KheEventResourceClassDelete(erc);
  MArrayFree(ercs->final_classes);
  MArrayFree(ercs->tmp_leader_tasks);
  MArrayFree(ercs->tmp_multiplicities);
  MArrayFree(ercs->tmp_monitors);
  MFree(ercs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceClassAccepts(KHE_EVENT_RESOURCE_CLASS_SOLVER ercs,  */
/*    KHE_EVENT_RESOURCE_CLASS erc, KHE_EVENT_RESOURCE er)                   */
/*                                                                           */
/*  Return true if er assigns the same resources as the event resources of   */
/*  erc.                                                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheEventResourceClassAccepts(KHE_EVENT_RESOURCE_CLASS_SOLVER ercs,
  KHE_EVENT_RESOURCE_CLASS erc, KHE_EVENT_RESOURCE er)
{
  int j, k;  KHE_TASK task, leader_task;  KHE_EVENT_RESOURCE ercs_er;

  /* find the leader tasks, durations, and offsets of the first event of ercs */
  MArrayClear(ercs->tmp_leader_tasks);
  MArrayClear(ercs->tmp_multiplicities);
  ercs_er = MArrayFirst(erc->event_resources);
  for( j = 0;  j < KheEventResourceTaskCount(ercs->soln, ercs_er);  j++ )
  {
    task = KheEventResourceTask(ercs->soln, ercs_er, j);
    leader_task = KheTaskFirstUnFixed(task);
    if( leader_task == NULL )
      return false;
    MArrayAddLast(ercs->tmp_leader_tasks, leader_task);
    MArrayAddLast(ercs->tmp_multiplicities, 1);
  }

  /* check consistency with the incoming task */
  for( j = 0;  j < KheEventResourceTaskCount(ercs->soln, er);  j++ )
  {
    task = KheEventResourceTask(ercs->soln, er, j);

    /* search for a leader task that task matches with, not already nabbed */
    leader_task = KheTaskFirstUnFixed(task);
    if( leader_task == NULL )
      return false;
    for( k = 0;  k < MArraySize(ercs->tmp_leader_tasks);  k++ )
      if( leader_task == MArrayGet(ercs->tmp_leader_tasks, k) &&
	  1 == MArrayGet(ercs->tmp_multiplicities, k) )
	break;

    /* if no luck, exit with failure */
    if( k >= MArraySize(ercs->tmp_leader_tasks) )
      return false;

    /* success, but nab index k so it isn't matched twice by e's tasks */
    MArrayPreInc(ercs->tmp_multiplicities, k);
  }

  /* all correct */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassBreakUp(KHE_EVENT_RESOURCE_CLASS_SOLVER ercs,  */
/*    KHE_EVENT_RESOURCE_CLASS erc)                                          */
/*                                                                           */
/*  Check whether the event resources of event resource class erc really     */
/*  are constrained to be assigned the same resources, and break up the      */
/*  class appropriately if not.  Place the final classes onto the end of     */
/*  ercs->final_classes.                                                     */
/*                                                                           */
/*  This function also deletes erc afterwards, since it's no longer needed.  */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassBreakUp(KHE_EVENT_RESOURCE_CLASS_SOLVER ercs,
  KHE_EVENT_RESOURCE_CLASS erc)
{
  int start, i, j;  KHE_EVENT_RESOURCE er;  KHE_EVENT_RESOURCE_CLASS erc2;
  start = MArraySize(ercs->final_classes);
  MArrayForEach(erc->event_resources, &er, &i)
  {
    for( j = start;  j < MArraySize(ercs->final_classes);  j++ )
    {
      erc2 = MArrayGet(ercs->final_classes, j);
      if( KheEventResourceClassAccepts(ercs, erc2, er) )
      {
        KheEventResourceClassAddEventResource(erc2, er, ercs);
	break;
      }
    }
    if( j >= MArraySize(ercs->final_classes) )
    {
      erc2 = KheEventResourceClassMake();
      KheEventResourceClassAddEventResource(erc2, er, ercs);
      MArrayAddLast(ercs->final_classes, erc2);
    }
  }
  KheEventResourceClassDelete(erc);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource monitor grouping - main functions"             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceDoAssignResourceGroup(                              */
/*    KHE_EVENT_RESOURCE_CLASS_SOLVER ercs, KHE_EVENT_RESOURCE_CLASS erc)    */
/*                                                                           */
/*  Group together the attached assign resource monitors of erc.             */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceDoAssignResourceGroup(
  KHE_EVENT_RESOURCE_CLASS_SOLVER ercs, KHE_EVENT_RESOURCE_CLASS erc)
{
  KHE_EVENT_RESOURCE er;  int i, j, k;  KHE_MONITOR m;
  if( DEBUG8 )
  {
    fprintf(stderr, "[ KheEventResourceDoAssignResourceGroup(ercs, ");
    KheEventResourceClassDebug(erc, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* for each monitor of erc with the given tag, remove parents, then if */
  /* unattached make it a child of soln, and if attached save in tmp_monitors */
  MArrayClear(ercs->tmp_monitors);
  MArrayForEach(erc->event_resources, &er, &i)
  {
    for( j = 0;  j < KheSolnEventResourceMonitorCount(ercs->soln, er);  j++ )
    {
      m = KheSolnEventResourceMonitor(ercs->soln, er, j);
      if( KheMonitorTag(m) == KHE_ASSIGN_RESOURCE_MONITOR_TAG )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	if( !KheMonitorAttachedToSoln(m) )
          KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) ercs->soln, m);
	else if( !MArrayContains(ercs->tmp_monitors, m, &k) )
	  MArrayAddLast(ercs->tmp_monitors, m);
      }
    }
  }

  KheSolnAddMonitorGroup(ercs->soln, &ercs->tmp_monitors,
    0, MArraySize(ercs->tmp_monitors) - 1, KHE_SUBTAG_ASSIGN_RESOURCE, DEBUG8);
  if( DEBUG8 )
    fprintf(stderr, "] KheEventResourceDoAssignResourceGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesMonitorCmp(const void *t1, const void *t2)         */
/*                                                                           */
/*  Comparison function for sorting an array of prefer resources monitors    */
/*  so as to bring together monitors which request the same resources.       */
/*                                                                           */
/*****************************************************************************/

static int KhePreferResourcesMonitorCmp(const void *t1, const void *t2)
{
  KHE_PREFER_RESOURCES_MONITOR m1 = * (KHE_PREFER_RESOURCES_MONITOR *) t1;
  KHE_PREFER_RESOURCES_MONITOR m2 = * (KHE_PREFER_RESOURCES_MONITOR *) t2;
  KHE_PREFER_RESOURCES_CONSTRAINT c1 = KhePreferResourcesMonitorConstraint(m1);
  KHE_PREFER_RESOURCES_CONSTRAINT c2 = KhePreferResourcesMonitorConstraint(m2);
  KHE_RESOURCE_GROUP rg1 = KhePreferResourcesConstraintDomain(c1);
  KHE_RESOURCE_GROUP rg2 = KhePreferResourcesConstraintDomain(c2);
  KHE_RESOURCE r1, r2;  int i;
  if( KheResourceGroupResourceCount(rg1) != KheResourceGroupResourceCount(rg2) )
   return KheResourceGroupResourceCount(rg1)-KheResourceGroupResourceCount(rg2);
  for( i = 0;  i < KheResourceGroupResourceCount(rg1);  i++ )
  {
    r1 = KheResourceGroupResource(rg1, i);
    r2 = KheResourceGroupResource(rg2, i);
    if( r1 != r2 )
      return KheResourceInstanceIndex(r1) - KheResourceInstanceIndex(r2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesMonitorEqualResourceGroups(KHE_MONITOR m1,        */
/*    KHE_MONITOR m2)                                                        */
/*                                                                           */
/*  Return true if m1 and m2 (assumed to be prefer resources monitors)       */
/*  request the same resource groups.                                        */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferResourcesMonitorEqualResourceGroups(KHE_MONITOR m1,
  KHE_MONITOR m2)
{
  KHE_MONITOR tm1, tm2;
  tm1 = m1;  tm2 = m2;
  return KhePreferResourcesMonitorCmp(&tm1, &tm2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceDoPreferResourcesGroup(                             */
/*    KHE_EVENT_RESOURCE_CLASS_SOLVER ercs, KHE_EVENT_RESOURCE_CLASS erc)    */
/*                                                                           */
/*  Group together those attached prefer resources monitors of erc which     */
/*  request the same resources.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceDoPreferResourcesGroup(
  KHE_EVENT_RESOURCE_CLASS_SOLVER ercs, KHE_EVENT_RESOURCE_CLASS erc)
{
  KHE_EVENT_RESOURCE er;  int i, j, k;  KHE_MONITOR m, m2;
  if( DEBUG8 )
  {
    fprintf(stderr, "[ KheEventResourceDoPreferResourcesGroup(ercs, ");
    KheEventResourceClassDebug(erc, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* for each monitor of erc with the given tag, remove parents, then */
  /* save in tmp_monitors */
  MArrayClear(ercs->tmp_monitors);
  MArrayForEach(erc->event_resources, &er, &i)
  {
    for( j = 0;  j < KheSolnEventResourceMonitorCount(ercs->soln, er);  j++ )
    {
      m = KheSolnEventResourceMonitor(ercs->soln, er, j);
      if( KheMonitorTag(m) == KHE_PREFER_RESOURCES_MONITOR_TAG )
      {
        KheMonitorDeleteAllParentsRecursive(m);
	if( !MArrayContains(ercs->tmp_monitors, m, &k) )
	  MArrayAddLast(ercs->tmp_monitors, m);
      }
    }
  }

  /* sort tmp_monitors to bring equal time groups together */
  MArraySort(ercs->tmp_monitors, &KhePreferResourcesMonitorCmp);

  /* visit each run of monitors with equal time groups and group them */
  for( i = 0;  i < MArraySize(ercs->tmp_monitors);  i = j )
  {
    m = MArrayGet(ercs->tmp_monitors, i);
    for( j = i + 1;  j < MArraySize(ercs->tmp_monitors);  j++ )
    {
      m2 = MArrayGet(ercs->tmp_monitors, j);
      if( !KhePreferResourcesMonitorEqualResourceGroups(m, m2) )
	break;
    }
    KheSolnAddMonitorGroup(ercs->soln, &ercs->tmp_monitors, i, j-1,
      KHE_SUBTAG_PREFER_RESOURCES, DEBUG2);
  }
  if( DEBUG8 )
    fprintf(stderr, "] KheEventResourceDoPreferResourcesGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrepareEventResourceMonitors(KHE_SOLN soln,                      */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Prepare event resource monitors.                                         */
/*                                                                           */
/*****************************************************************************/

static void KhePrepareEventResourceMonitors(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt)
{
  int i;  KHE_INSTANCE ins;  KHE_EVENT_RESOURCE er, first_er;  KHE_TASK task;
  KHE_EVENT_RESOURCE_CLASS erc; KHE_EVENT_RESOURCE_CLASS_SOLVER ercs;

  /* build an initial set of event resource classes, one per event */
  if( DEBUG7 )
    fprintf(stderr, "[ KhePrepareEventResourceMonitors(soln, %s)\n",
      rt == NULL ? "NULL" : KheResourceTypeId(rt));
  ercs = KheEventResourceClassSolverMake(soln);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( rt == NULL || KheEventResourceResourceType(er) == rt )
    {
      erc = KheEventResourceClassMake();
      MArrayAddLast(ercs->classes_by_event_resource, erc);
      KheEventResourceClassAddEventResource(erc, er, ercs);
    }
    else
      MArrayAddLast(ercs->classes_by_event_resource, NULL);
  }

  /* tentatively merge classes that might be time-equivalent */
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    if( rt == NULL || KheTaskResourceType(task) == rt )
    {
      if( !KheTaskIsCycleTask(task) && !KheTaskAssignIsFixed(task) )
      {
	first_er = NULL;
	KheMergeRelatedEventResources(task, &first_er, ercs);
      }
    }
  }

  /* build a separate array of all distinct classes */
  MArrayInit(ercs->distinct_classes);
  MArrayForEach(ercs->classes_by_event_resource, &erc, &i)
    if( erc != NULL &&
	i == KheEventResourceInstanceIndex(MArrayFirst(erc->event_resources)) )
      MArrayAddLast(ercs->distinct_classes, erc);
  if( DEBUG7 )
  {
    fprintf(stderr, "  distinct classes:\n");
    MArrayForEach(ercs->distinct_classes, &erc, &i)
      KheEventResourceClassDebug(erc, 2, stderr);
  }

  /* break up classes that don't work; sort the events of the final classes */
  MArrayForEach(ercs->distinct_classes, &erc, &i)
    KheEventResourceClassBreakUp(ercs, erc);
  MArrayForEach(ercs->final_classes, &erc, &i)
    MArraySort(erc->event_resources, &KheEventResourceCmp);
  if( DEBUG7 )
  {
    fprintf(stderr, "  final classes:\n");
    MArrayForEach(ercs->final_classes, &erc, &i)
      KheEventResourceClassDebug(erc, 2, stderr);
  }

  /* group assign resource and prefer resource monitors */
  MArrayForEach(ercs->final_classes, &erc, &i)
  {
    KheEventResourceDoAssignResourceGroup(ercs, erc);
    KheEventResourceDoPreferResourcesGroup(ercs, erc);
  }

  KheEventResourceClassSolverDelete(ercs);
  if( DEBUG7 )
    fprintf(stderr, "] KhePrepareEventResourceMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnPrepareEventResourceMonitors(KHE_SOLN soln,                    */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Undo what KhePrepareEventResourceMonitors did.                           */
/*                                                                           */
/*****************************************************************************/

static void KheUnPrepareEventResourceMonitors(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt)
{
  KHE_INSTANCE ins;  int i, j;  KHE_EVENT_RESOURCE er;  KHE_MONITOR m;
  KHE_GROUP_MONITOR gm;
  if( DEBUG7 )
    fprintf(stderr, "[ KheUnPrepareEventResourceMonitors(soln, %s)\n",
      rt == NULL ? "NULL" : KheResourceTypeId(rt));
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( rt == NULL || KheEventResourceResourceType(er) == rt )
      for( j = 0;  j < KheSolnEventResourceMonitorCount(soln, er);  j++ )
      {
	m = KheSolnEventResourceMonitor(soln, er, j);
	switch( KheMonitorTag(m) )
	{
	  case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

	    if( KheMonitorHasParent(m, KHE_SUBTAG_ASSIGN_RESOURCE, &gm) )
	      KheGroupMonitorBypassAndDelete(gm);
	    break;

	  case KHE_PREFER_RESOURCES_MONITOR_TAG:

	    if( KheMonitorHasParent(m, KHE_SUBTAG_PREFER_RESOURCES, &gm) )
	      KheGroupMonitorBypassAndDelete(gm);
	    break;

	  default:

	    /* not interested */
	    break;
	}
      }
  }
  if( DEBUG7 )
    fprintf(stderr, "] KheUnPrepareEventResourceMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource monitor grouping - resource class layers"            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceClassLayerCmp(const void *p1, const void *p2)             */
/*                                                                           */
/*  Comparison function for sorting an array of resource class layers to     */
/*  bring elements with equal layers together.                               */
/*                                                                           */
/*****************************************************************************/

static int KheResourceClassLayerCmp(const void *p1, const void *p2)
{
  KHE_RESOURCE_CLASS_LAYER rcl1 = * (KHE_RESOURCE_CLASS_LAYER *) p1;
  KHE_RESOURCE_CLASS_LAYER rcl2 = * (KHE_RESOURCE_CLASS_LAYER *) p2;
  int count1 = MArraySize(rcl1->events);
  int count2 = MArraySize(rcl2->events);
  int i;  KHE_EVENT e1, e2;
  if( count1 != count2 )
    return count2 - count1;
  for( i = 0;  i < count1;  i++ )
  {
    e1 = MArrayGet(rcl1->events, i);
    e2 = MArrayGet(rcl2->events, i);
    if( e1 != e2 )
      return KheEventIndex(e1) - KheEventIndex(e2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceClassLayersEqual(KHE_RESOURCE_CLASS_LAYER rcl1,          */
/*    KHE_RESOURCE_CLASS_LAYER rcl2)                                         */
/*                                                                           */
/*  Return true if rcl1 and rcl2 have equal sets of events.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceClassLayersEqual(KHE_RESOURCE_CLASS_LAYER rcl1,
  KHE_RESOURCE_CLASS_LAYER rcl2)
{
  KHE_RESOURCE_CLASS_LAYER x1, x2;
  x1 = rcl1;
  x2 = rcl2;
  return KheResourceClassLayerCmp((const void *) &x1, (const void *) &x2) == 0;
}


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
/*  KHE_RESOURCE_CLASS_LAYER KheResourceClassLayerMake(KHE_RESOURCE r,       */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Make a new resource class layer for r, using ecs for uniqueifying        */
/*  linked events.                                                           */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_CLASS_LAYER KheResourceClassLayerMake(KHE_RESOURCE r,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  KHE_RESOURCE_CLASS_LAYER res;  KHE_EVENT e;  KHE_EVENT_CLASS ec;  int i;

  /* make the basic object */
  MMake(res);
  res->resource = r;
  MArrayInit(res->events);

  /* add uniqueified events */
  for( i = 0;  i < KheResourceLayerEventCount(r);  i++ )
  {
    e = KheResourceLayerEvent(r, i);
    ec = MArrayGet(ecs->classes_by_event, KheEventIndex(e));
    e = MArrayFirst(ec->events);
    MArrayAddLast(res->events, e);
  }

  /* sort events by increasing event index */
  MArraySort(res->events, &KheEventIndexCmp);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceClassLayerDelete(KHE_RESOURCE_CLASS_LAYER rcl)           */
/*                                                                           */
/*  Delete rcl.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheResourceClassLayerDelete(KHE_RESOURCE_CLASS_LAYER rcl)
{
  MArrayFree(rcl->events);
  MFree(rcl);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource monitor grouping - main functions"                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitWorkloadMonitorFindCeiling(KHE_LIMIT_WORKLOAD_MONITOR lwm)   */
/*                                                                           */
/*  Find a suitable value for lwm's ceiling attribute:  the number of        */
/*  times in the cycle minus the number of workload demand monitors for      */
/*  lwm's resource.                                                          */
/*                                                                           */
/*****************************************************************************/

static int KheLimitWorkloadMonitorFindCeiling(KHE_LIMIT_WORKLOAD_MONITOR lwm)
{
  KHE_RESOURCE r;  KHE_SOLN soln;  int i, res;  KHE_MONITOR m;
  r = KheLimitWorkloadMonitorResource(lwm);
  soln = KheMonitorSoln((KHE_MONITOR) lwm);
  res = KheInstanceTimeCount(KheSolnInstance(soln));
  for( i = 0;  i < KheSolnResourceMonitorCount(soln, r);  i++ )
  {
    m = KheSolnResourceMonitor(soln, r, i);
    if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
      res--;
  }
  if( res < 0 )
    res = 0;
  if( DEBUG10 )
    fprintf(stderr, "  KheLimitWorkloadMonitorFindCeiling(%s) = %d\n",
      KheResourceId(r), res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDetachSomeResourceMonitors(KHE_SOLN soln, KHE_RESOURCE r)        */
/*                                                                           */
/*  Detach some of r's resource monitors, or set their ceilings.             */
/*                                                                           */
/*****************************************************************************/

static void KheDetachSomeResourceMonitors(KHE_SOLN soln, KHE_RESOURCE r)
{
  int i;  KHE_MONITOR m, om;  KHE_LIMIT_WORKLOAD_MONITOR lwm, done_lwm;
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT lbtc;  KHE_LIMIT_BUSY_TIMES_MONITOR lbtm;
  /* KHE_LIMIT_WORKLOAD_CONSTRAINT lwc; */
  done_lwm = NULL;
  for( i = 0;  i < KheSolnResourceMonitorCount(soln, r);  i++ )
  {
    m = KheSolnResourceMonitor(soln, r, i);
    switch( KheMonitorTag(m) )
    {
      case KHE_AVOID_CLASHES_MONITOR_TAG:

	if( KheMonitorAttachedToSoln(m) )
	  KheMonitorDetachFromSoln(m);
	break;

      case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	om = KheWorkloadDemandMonitorOriginatingMonitor(
	  (KHE_WORKLOAD_DEMAND_MONITOR) m);
	if( KheMonitorAttachedToSoln(om) ) switch( KheMonitorTag(om) )
	{
	  case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

            KheMonitorDetachFromSoln(om);
	    break;

	  case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

            lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) om;
	    lbtc = KheLimitBusyTimesMonitorConstraint(lbtm);
	    if( KheLimitBusyTimesConstraintMinimum(lbtc) == 0 )
              KheMonitorDetachFromSoln(om);
	    else
	      KheLimitBusyTimesMonitorSetCeiling(lbtm,
		KheLimitBusyTimesConstraintMaximum(lbtc));
	    /* *** obsolete version
            lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) om;
	    lbtc = KheLimitBusyTimesMonitorConstraint(lbtm);
	    if( KheLimitBusyTimesConstraintMinimum(lbtc) == 0 )
              KheMonitorDetachFromSoln(om);
	    else if( KheLimitBusyTimesMonitorMaximumAttachedToSoln(lbtm) )
	      KheLimitBusyTimesMonitorMaximumDetachFromSoln(lbtm);
	    *** */
	    break;

	  case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

            lwm = (KHE_LIMIT_WORKLOAD_MONITOR) om;
	    if( lwm != done_lwm )
	    {
	      KheLimitWorkloadMonitorSetCeiling(lwm,
		KheLimitWorkloadMonitorFindCeiling(lwm));
	      done_lwm = lwm;
	    }
	    /* ***
	    lwc = KheLimitWorkloadMonitorConstraint(lwm);
	    if( KheLimitWorkloadConstraintMinimum(lwc) == 0 )
              KheMonitorDetachFromSoln(om);
	    else
	    *** */
	    /* *** obsolete version
            lwm = (KHE_LIMIT_WORKLOAD_MONITOR) om;
	    lwc = KheLimitWorkloadMonitorConstraint(lwm);
	    if( KheLimitWorkloadConstraintMinimum(lwc) == 0 )
              KheMonitorDetachFromSoln(om);
	    else if( KheLimitWorkloadMonitorMaximumAttachedToSoln(lwm) )
	      KheLimitWorkloadMonitorMaximumDetachFromSoln(lwm);
	    *** */
	    break;

	  default:

	    /* nothing to do otherwise */
	    break;
	}
	break;

      default:

	/* nothing to do otherwise */
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAttachSomeResourceMonitors(KHE_SOLN soln, KHE_RESOURCE r)        */
/*                                                                           */
/*  Attach some of r's resource monitors.                                    */
/*                                                                           */
/*****************************************************************************/

static void KheAttachSomeResourceMonitors(KHE_SOLN soln, KHE_RESOURCE r)
{
  int i;  KHE_MONITOR m, om;  KHE_LIMIT_BUSY_TIMES_MONITOR lbtm;
  KHE_LIMIT_WORKLOAD_MONITOR lwm;
  for( i = 0;  i < KheSolnResourceMonitorCount(soln, r);  i++ )
  {
    m = KheSolnResourceMonitor(soln, r, i);
    switch( KheMonitorTag(m) )
    {
      case KHE_AVOID_CLASHES_MONITOR_TAG:

	if( !KheMonitorAttachedToSoln(m) )
	  KheMonitorAttachToSoln(m);
	break;

      case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	om = KheWorkloadDemandMonitorOriginatingMonitor(
	  (KHE_WORKLOAD_DEMAND_MONITOR) m);
	switch( KheMonitorTag(om) )
	{
	  case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

	    if( !KheMonitorAttachedToSoln(om) )
	      KheMonitorAttachToSoln(om);
	    break;

	  case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

            lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) om;
	    KheLimitBusyTimesMonitorSetCeiling(lbtm, INT_MAX);
	    if( !KheMonitorAttachedToSoln(om) )
	      KheMonitorAttachToSoln(om);
	    /* *** obsolete version
	    if( !KheMonitorAttachedToSoln(om) )
	      KheMonitorAttachToSoln(om);
            lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) om;
	    if( !KheLimitBusyTimesMonitorMaximumAttachedToSoln(lbtm) )
	      KheLimitBusyTimesMonitorMaximumAttachToSoln(lbtm);
	    *** */
	    break;

	  case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

            lwm = (KHE_LIMIT_WORKLOAD_MONITOR) om;
	    KheLimitWorkloadMonitorSetCeiling(lwm, INT_MAX);
	    if( !KheMonitorAttachedToSoln(om) )
	      KheMonitorAttachToSoln(om);
	    /* *** obsolete version
	    if( !KheMonitorAttachedToSoln(om) )
	      KheMonitorAttachToSoln(om);
            lwm = (KHE_LIMIT_WORKLOAD_MONITOR) om;
	    if( !KheLimitWorkloadMonitorMaximumAttachedToSoln(lwm) )
	      KheLimitWorkloadMonitorMaximumAttachToSoln(lwm);
	    *** */
	    break;

	  default:

	    /* nothing to do otherwise */
	    break;
	}
	break;

      default:

	/* nothing to do otherwise */
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorConstraintCmp(const void *p1, const void *p2)              */
/*                                                                           */
/*  Comparison function for sorting an array of monitors so as to bring      */
/*  monitors for the same constraint together.                               */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorConstraintCmp(const void *p1, const void *p2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) p1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) p2;
  return KheConstraintIndex(KheMonitorConstraint(m1)) -
    KheConstraintIndex(KheMonitorConstraint(m2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupResourceMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,   */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Group resource monitors for resources of this type, which are known      */
/*  to be all preassigned.  Use ecs for uniqueifying linked events.          */
/*                                                                           */
/*****************************************************************************/

static void KheSolnGroupResourceMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  KHE_RESOURCE r;  int i, j, k, n, groups;  ARRAY_KHE_MONITOR monitors;
  KHE_RESOURCE_CLASS_LAYER rcli, rclj, rcl;  KHE_MONITOR m;
  ARRAY_KHE_RESOURCE_CLASS_LAYER layers;
  if( DEBUG6 )
    fprintf(stderr, "[ KheSolnGroupResourceMonitors(soln, %s)\n",
      KheResourceTypeId(rt) == NULL ? "-" : KheResourceTypeId(rt));

  /* get the resource class layers, sorted to bring equal layers together */
  MArrayInit(layers);
  MArrayInit(monitors);
  for( i = 0;  i < KheResourceTypeResourceCount(rt);  i++ )
  {
    r = KheResourceTypeResource(rt, i);
    MArrayAddLast(layers, KheResourceClassLayerMake(r, ecs));
  }
  MArraySort(layers, &KheResourceClassLayerCmp);

  /* find runs of resources with equal layers */
  groups = 0;
  for( i = 0;  i < MArraySize(layers);  i = j )
  {
    rcli = MArrayGet(layers, i);
    for( j = i + 1;  j < MArraySize(layers);  j++ )
    {
      rclj = MArrayGet(layers, j);
      if( !KheResourceClassLayersEqual(rcli, rclj) )
	break;
    }
    groups++;

    /* at this point, rcli ... rclj -1 can be grouped */
    if( DEBUG6 )
    {
      fprintf(stderr, "  group of %d resources:", j - i);
      for( k = i;  k < j;  k++ )
      {
	rcl = MArrayGet(layers, k);
	fprintf(stderr, " %s", KheResourceId(rcl->resource) == NULL ? "-" :
	  KheResourceId(rcl->resource));
      }
      fprintf(stderr, "\n");
    }

    if( i < j - 1 )
    {
      /* grab relevant monitors of resources i ... j - 1 and sort */
      MArrayClear(monitors);
      for( k = i;  k < j;  k++ )
      {
	rcl = MArrayGet(layers, k);
	for( n = 0; n < KheSolnResourceMonitorCount(soln, rcl->resource); n++ )
	{
	  m = KheSolnResourceMonitor(soln, rcl->resource, n);
	  switch( KheMonitorTag(m) )
	  {
	    case KHE_AVOID_CLASHES_MONITOR_TAG:
	    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
	    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
	    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
	    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
	    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	      MArrayAddLast(monitors, m);
	      KheMonitorDeleteAllParentsRecursive(m);
	      break;

	    default:

	      /* ignore any other monitors */
	      break;
	  }
	}
      }
      MArraySort(monitors, &KheMonitorConstraintCmp);

      /* group monitors for the same constraint together */
      for( k = 0;  k < MArraySize(monitors);  k = n )
      {
	m = MArrayGet(monitors, k);
	for( n = k + 1;  n < MArraySize(monitors);  n++ )
	{
	  if( KheMonitorConstraint(m) !=
	      KheMonitorConstraint(MArrayGet(monitors, n)) )
	    break;
	}
	KheSolnAddMonitorGroup(soln, &monitors, k, n - 1,
	  KheSubTagFromTag(KheMonitorTag(m)), DEBUG6);
      }
    }
  }

  /* reclaim memory */
  MArrayForEach(layers, &rcl, &i)
    KheResourceClassLayerDelete(rcl);
  MArrayFree(layers);
  MArrayFree(monitors);

  if( DEBUG6 )
  {
    fprintf(stderr, "  %d resources, %d groups\n",
      KheResourceTypeResourceCount(rt), groups);
    fprintf(stderr, "] KheSolnGroupResourceMonitors returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnUnGroupResourceMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt) */
/*                                                                           */
/*  Undo the effect of KheSolnGroupResourceMonitors.                         */
/*                                                                           */
/*****************************************************************************/

static void KheSolnUnGroupResourceMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  KHE_RESOURCE r;  int i, j;  KHE_MONITOR m;  KHE_GROUP_MONITOR gm;
  if( DEBUG6 )
    fprintf(stderr, "[ KheSolnUnGroupResourceMonitors(soln, %s)\n",
      KheResourceTypeId(rt) == NULL ? "-" : KheResourceTypeId(rt));
  for( i = 0;  i < KheResourceTypeResourceCount(rt);  i++ )
  {
    r = KheResourceTypeResource(rt, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      switch( KheMonitorTag(m) )
      {
	case KHE_AVOID_CLASHES_MONITOR_TAG:
	case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
	case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
	case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
	case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
	case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	  if( KheMonitorHasParent(m, KheSubTagFromTag(KheMonitorTag(m)), &gm) )
	    KheGroupMonitorBypassAndDelete(gm);
	  break;

	default:

	  /* ignore any other monitors */
	  break;
      }
    }
  }
  if( DEBUG6 )
    fprintf(stderr, "] KheSolnUnGroupResourceMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrepareResourceMonitors(KHE_SOLN soln,                           */
/*    KHE_EVENT_CLASS_SOLVER ecs)                                            */
/*                                                                           */
/*  Prepare resource monitors, including some detaching.                     */
/*                                                                           */
/*****************************************************************************/

static void KhePrepareResourceMonitors(KHE_SOLN soln,
  KHE_EVENT_CLASS_SOLVER ecs)
{
  int i;  KHE_INSTANCE ins;  KHE_RESOURCE_TYPE rt;

  /* group resource monitors with all-preassigned resource types */
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( KheResourceTypeDemandIsAllPreassigned(rt) )
      KheSolnGroupResourceMonitors(soln, rt, ecs);
  }

  /* detach some resource monitors */
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
    KheDetachSomeResourceMonitors(soln, KheInstanceResource(ins, i));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnPrepareResourceMonitors(KHE_SOLN soln)                         */
/*                                                                           */
/*  Undo KhePrepareResourceMonitors.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheUnPrepareResourceMonitors(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  int i;  KHE_RESOURCE_TYPE rt;

  /* ungroup resource monitors with all-preassigned resource types */
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( KheResourceTypeDemandIsAllPreassigned(rt) )
      KheSolnUnGroupResourceMonitors(soln, rt);
  }

  /* reattach some resource monitors */
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
    KheAttachSomeResourceMonitors(soln, KheInstanceResource(ins, i));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand monitor grouping"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheFindDemandMonitors(KHE_MEET meet, KHE_SOLN soln,                 */
/*    ARRAY_KHE_MONITOR *monitors)                                           */
/*                                                                           */
/*  For each demand monitor associated with meet; add them to *monitors.     */
/*                                                                           */
/*****************************************************************************/

static void KheFindDemandMonitors(KHE_MEET meet, KHE_SOLN soln,
  ARRAY_KHE_MONITOR *monitors)
{
  KHE_MEET sub_meet;  int i, j;  KHE_TASK task;  KHE_MONITOR m;

  /* handle meet itself */
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    task = KheMeetTask(meet, i);
    for( j = 0;  j < KheTaskDemandMonitorCount(task);  j++ )
    {
      m = (KHE_MONITOR) KheTaskDemandMonitor(task, j);
      KheMonitorDeleteAllParentsRecursive(m);
      MArrayAddLast(*monitors, m);
    }
  }

  /* handle meets whose assignment is fixed to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    sub_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetAssignIsFixed(sub_meet) )
      KheFindDemandMonitors(sub_meet, soln, monitors);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrepareOrdinaryDemandMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Group ordinary demand monitors by leader meet.                           */
/*                                                                           */
/*****************************************************************************/

static void KhePrepareOrdinaryDemandMonitors(KHE_SOLN soln)
{
  int i;  KHE_MEET meet;  ARRAY_KHE_MONITOR monitors;
  if( DEBUG9 )
    fprintf(stderr, "[ KhePrepareOrdinaryDemandMonitors(soln)\n");
  MArrayInit(monitors);
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet = KheSolnMeet(soln, i);
    if( !KheMeetIsCycleMeet(meet) && !KheMeetAssignIsFixed(meet) )
    {
      MArrayClear(monitors);
      KheFindDemandMonitors(meet, soln, &monitors);
      KheSolnAddMonitorGroup(soln, &monitors, 0, MArraySize(monitors) - 1,
	KHE_SUBTAG_ORDINARY_DEMAND, DEBUG9);
    }
  }
  MArrayFree(monitors);
  if( DEBUG9 )
    fprintf(stderr, "] KhePrepareOrdinaryDemandMonitors ret\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDeleteDemandMonitors(KHE_MEET meet)                              */
/*                                                                           */
/*  Delete meet demand monitors associated with meet, recursively.           */
/*  Return true when the job is done.                                        */
/*                                                                           */
/*****************************************************************************/

static bool KheDeleteDemandMonitors(KHE_MEET meet)
{
  KHE_MEET sub_meet;  int i;  KHE_TASK task;  KHE_MONITOR m;
  KHE_GROUP_MONITOR gm;

  /* search for a demand monitor in meet itself, and use it if present */
  if( KheMeetTaskCount(meet) > 0 )
  {
    task = KheMeetTask(meet, 0);
    m = (KHE_MONITOR) KheTaskDemandMonitor(task, 0);
    if( KheMonitorHasParent(m, KHE_SUBTAG_ORDINARY_DEMAND, &gm) )
      KheGroupMonitorBypassAndDelete(gm);
    return true;
  }

  /* search in meets whose assignment is fixed to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    sub_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetAssignIsFixed(sub_meet) && KheDeleteDemandMonitors(sub_meet) )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnPrepareOrdinaryDemandMonitors(KHE_SOLN soln)                   */
/*                                                                           */
/*  Undo what KhePrepareOrdinaryDemandMonitors did.                          */
/*                                                                           */
/*****************************************************************************/

static void KheUnPrepareOrdinaryDemandMonitors(KHE_SOLN soln)
{
  int i;  KHE_MEET meet;
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet = KheSolnMeet(soln, i);
    if( !KheMeetIsCycleMeet(meet) && !KheMeetAssignIsFixed(meet) )
      KheDeleteDemandMonitors(meet);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorUnLink(KHE_MONITOR m, KHE_SOLN soln)                      */
/*                                                                           */
/*  Ensure that m is unlinked from soln.                                     */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheMonitorUnLink(KHE_MONITOR m, KHE_SOLN soln)
{
  KHE_GROUP_MONITOR parent_gm;  int i;
  RESTART:
  for( i = 0;  i < KheMonitorParentMonitorCount(m);  i++ )
  {
    parent_gm = KheMonitorParentMonitor(m, i);
    if( KheMonitorDescendant((KHE_MONITOR) parent_gm, (KHE_MONITOR) soln) )
    {
      ** m is linked to soln via parent_gm, so remove that link **
      KheGroupMonitorDeleteChildMonitor(parent_gm, m);
      goto RESTART;
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorLink(KHE_MONITOR m, KHE_SOLN soln)                        */
/*                                                                           */
/*  Ensure that m is linked to soln.  This code assumes that it is unlinked. */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheMonitorLink(KHE_MONITOR m, KHE_SOLN soln)
{
  KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, m);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheUnLinkAllDemandMonitors(KHE_SOLN soln)                           */
/*                                                                           */
/*  Ensure that all demand monitors are unlinked from the solution,          */
/*  without detaching them.                                                  */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheUnLinkAllDemandMonitors(KHE_SOLN soln)
{
  int i, j;  KHE_TASK task;  KHE_MONITOR m;  KHE_INSTANCE ins;  KHE_RESOURCE r;

  ** do it for all ordinary demand monitors **
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    for( j = 0;  j < KheTaskDemandMonitorCount(task);  j++ )
    {
      m = (KHE_MONITOR) KheTaskDemandMonitor(task, j);
      KheMonitorUnLink(m, soln);
    }
  }

  ** do it for workload demand monitors **
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
	KheMonitorUnLink(m, soln);
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkAllDemandMonitors(KHE_SOLN soln)                             */
/*                                                                           */
/*  Ensure that all demand monitors are linked to the solution, without      */
/*  attaching them.                                                          */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheLinkAllDemandMonitors(KHE_SOLN soln)
{
  int i, j;  KHE_TASK task;  KHE_MONITOR m;  KHE_INSTANCE ins;  KHE_RESOURCE r;

  ** do it for all ordinary demand monitors **
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    for( j = 0;  j < KheTaskDemandMonitorCount(task);  j++ )
    {
      m = (KHE_MONITOR) KheTaskDemandMonitor(task, j);
      KheMonitorLink(m, soln);
    }
  }

  ** do it for workload demand monitors **
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
	KheMonitorLink(m, soln);
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDetachAllDemandMonitors(KHE_SOLN soln)                           */
/*                                                                           */
/*  Detach all the demand monitors of soln that are not already detached.    */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheDetachAllDemandMonitors(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  int i, j;  KHE_RESOURCE r;  KHE_MONITOR m;
  KheSolnMatchingDetachAllOrdinaryDemandMonitors(soln);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      if( (KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
	  KheMonitorTag(m) ==  KHE_WORKLOAD_DEMAND_MONITOR_TAG) &&
	  KheMonitorAttachedToSoln(m) )
	KheMonitorDetachFromSoln(m);
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheAttachAllDemandMonitors(KHE_SOLN soln)                           */
/*                                                                           */
/*  Attach all the demand monitors of soln that are not already attached.    */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheAttachAllDemandMonitors(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  int i, j;  KHE_RESOURCE r;  KHE_MONITOR m;
  KheSolnMatchingAttachAllOrdinaryDemandMonitors(soln);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      if( (KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
	  KheMonitorTag(m) ==  KHE_WORKLOAD_DEMAND_MONITOR_TAG) &&
	  !KheMonitorAttachedToSoln(m) )
	KheMonitorAttachToSoln(m);
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupWorkloadMonitors(KHE_MONITOR orig_m, KHE_RESOURCE r,        */
/*    ARRAY_KHE_MONITOR *monitors)                                           */
/*                                                                           */
/*  Group together orig_m and the workload demand monitors of r that have    */
/*  orig_m as originating monitor.  Use *monitors as a scratch array.        */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheGroupWorkloadMonitors(KHE_MONITOR orig_m, KHE_RESOURCE r,
  ARRAY_KHE_MONITOR *monitors)
{
  int i;  KHE_SOLN soln;  KHE_MONITOR m;  KHE_WORKLOAD_DEMAND_MONITOR wdm;

  ** get all the relevant monitors into *monitors **
  soln = KheMonitorSoln(orig_m);
  MArrayClear(*monitors);
  ** MArrayAddLast(*monitors, orig_m); not this one now **
  for( i = 0;  i < KheSolnResourceMonitorCount(soln, r);  i++ )
  {
    m = KheSolnResourceMonitor(soln, r, i);
    if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    {
      wdm = (KHE_WORKLOAD_DEMAND_MONITOR) m;
      if( KheWorkloadDemandMonitorOriginatingMonitor(wdm) == orig_m )
	MArrayAddLast(*monitors, m);
    }
  }

  ** group them **
  KheSolnAddMonitorGroup(soln, monitors, 0, MArraySize(*monitors) - 1,
    KHE_SUBTAG_WORKLOAD_DEMAND, DEBUG9);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KhePrepareWorkloadDemandMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Group workload demand monitors by originating monitor.                   */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KhePrepareWorkloadDemandMonitors(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  int i, j;  KHE_RESOURCE r;  KHE_CONSTRAINT c;
  KHE_MONITOR orig_m;  ARRAY_KHE_MONITOR monitors;
  if( DEBUG9 )
    fprintf(stderr,
      "[ KhePrepareWorkloadDemandMonitors(soln)\n");
  MArrayInit(monitors);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      orig_m = KheSolnResourceMonitor(soln, r, j);
      switch( KheMonitorTag(orig_m) )
      {
	case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
	case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
	case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	  c = KheMonitorConstraint(orig_m);
	  if( KheConstraintCombinedWeight(c) >= KheCost(1, 0) )
	    KheGroupWorkloadMonitors(orig_m, r, &monitors);
	  break;

	default:

	  break;
      }
    }
  }
  MArrayFree(monitors);
  if( DEBUG9 )
    fprintf(stderr,
      "] KhePrepareWorkloadDemandMonitors ret\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main grouping and ungrouping functions"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectionChainPrepareMonitors(KHE_SOLN soln)                      */
/*                                                                           */
/*  Prepare the monitors of soln for an ejection chain repair, by carrying   */
/*  out the usual primary grouping and detaching.                            */
/*                                                                           */
/*****************************************************************************/

void KheEjectionChainPrepareMonitors(KHE_SOLN soln)
{
  KHE_EVENT_CLASS_SOLVER ecs;
  if( DEBUG1 )
    fprintf(stderr, "[ KheEjectionChainPrepareMonitors(soln)\n");
  ecs = KhePrepareEventMonitors(soln);
  KhePrepareEventResourceMonitors(soln, NULL);
  KhePrepareResourceMonitors(soln, ecs);
  KheEventClassSolverDelete(ecs);
  KhePrepareOrdinaryDemandMonitors(soln);
  /* KheUnLinkAllDemandMonitors(soln); */
  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainPrepareMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectionChainUnPrepareMonitors(KHE_SOLN soln)                    */
/*                                                                           */
/*  Undo primary grouping.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheEjectionChainUnPrepareMonitors(KHE_SOLN soln)
{
  if( DEBUG1 )
    fprintf(stderr, "[ KheEjectionChainUnPrepareMonitors(soln)\n");
  KheUnPrepareEventMonitors(soln);
  KheUnPrepareEventResourceMonitors(soln, NULL);
  KheUnPrepareResourceMonitors(soln);
  KheUnPrepareOrdinaryDemandMonitors(soln);
  /* KheLinkAllDemandMonitors(soln); */
  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainUnPrepareMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceRepairPrepareMonitors(KHE_SOLN soln,                     */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Primary grouping.                                                        */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheResourceRepairPrepareMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  int i;  KHE_INSTANCE ins;  KHE_EVENT_RESOURCE er, first_er;  KHE_TASK task;
  KHE_EVENT_RESOURCE_CLASS erc; KHE_EVENT_RESOURCE_CLASS_SOLVER ercs;

  ** build an initial set of event resource classes, one per event **
  if( DEBUG7 )
    fprintf(stderr, "[ KheResourceRepairPrepareMonitors(soln)\n");
  ercs = KheEventResourceClassSolverMake(soln);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( rt == NULL || KheEventResourceResourceType(er) == rt )
    {
      erc = KheEventResourceClassMake();
      MArrayAddLast(ercs->classes_by_event_resource, erc);
      KheEventResourceClassAddEventResource(erc, er, ercs);
    }
    else
      MArrayAddLast(ercs->classes_by_event_resource, NULL);
  }

  ** tentatively merge classes that might be time-equivalent **
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    if( rt == NULL || KheTaskResourceType(task) == rt )
    {
      if( !KheTaskIsCycleTask(task) && !KheTaskAssignIsFixed(task) )
      {
	first_er = NULL;
	KheMergeRelatedEventResources(task, &first_er, ercs);
      }
    }
  }

  ** build a separate array of all distinct classes **
  MArrayInit(ercs->distinct_classes);
  MArrayForEach(ercs->classes_by_event_resource, &erc, &i)
    if( erc != NULL &&
	i == KheEventResourceInstanceIndex(MArrayFirst(erc->event_resources)) )
      MArrayAddLast(ercs->distinct_classes, erc);
  if( DEBUG7 )
  {
    fprintf(stderr, "  distinct classes:\n");
    MArrayForEach(ercs->distinct_classes, &erc, &i)
      KheEventResourceClassDebug(erc, 2, stderr);
  }

  ** break up classes that don't work; sort the events of the final classes **
  MArrayForEach(ercs->distinct_classes, &erc, &i)
    KheEventResourceClassBreakUp(ercs, erc);
  MArrayForEach(ercs->final_classes, &erc, &i)
    MArraySort(erc->event_resources, &KheEventResourceCmp);
  if( DEBUG7 )
  {
    fprintf(stderr, "  final classes:\n");
    MArrayForEach(ercs->final_classes, &erc, &i)
      KheEventResourceClassDebug(erc, 2, stderr);
  }

  ** group assign resource and prefer resource monitors **
  MArrayForEach(ercs->final_classes, &erc, &i)
  {
    KheEventResourceDoAssignResourceGroup(ercs, erc);
    KheEventResourceDoPreferResourcesGroup(ercs, erc);
  }

  KheEventResourceClassSolverDelete(ercs);
  if( DEBUG7 )
    fprintf(stderr, "] KheResourceRepairPrepareMonitors returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceRepairUnPrepareMonitors(KHE_SOLN soln,                   */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Undo primary grouping.                                                   */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheResourceRepairUnPrepareMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  KHE_INSTANCE ins;  int i, j;  KHE_EVENT_RESOURCE er;  KHE_MONITOR m;
  KHE_GROUP_MONITOR gm;
  if( DEBUG7 )
    fprintf(stderr, "[ KheResourceRepairUnPrepareMonitors(soln)\n");
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( rt == NULL || KheEventResourceResourceType(er) == rt )
      for( j = 0;  j < KheSolnEventResourceMonitorCount(soln, er);  j++ )
      {
	m = KheSolnEventResourceMonitor(soln, er, j);
	switch( KheMonitorTag(m) )
	{
	  case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

	    if( KheMonitorHasParent(m, KHE_SUBTAG_ASSIGN_RESOURCE, &gm) )
	      KheGroupMonitorBypassAndDelete(gm);
	    break;

	  case KHE_PREFER_RESOURCES_MONITOR_TAG:

	    if( KheMonitorHasParent(m, KHE_SUBTAG_PREFER_RESOURCES, &gm) )
	      KheGroupMonitorBypassAndDelete(gm);
	    break;

	  default:

	    ** not interested **
	    break;
	}
      }
  }
  if( DEBUG7 )
    fprintf(stderr, "] KheResourceRepairUnPrepareMonitors returning\n");
}
*** */
