
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
/*  FILE:         khe_soln.c                                                 */
/*  DESCRIPTION:  A solution                                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG10 0
#define DEBUG11 0

#define KHE_PLACEHOLDER -1

/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN - a solution                                                    */
/*                                                                           */
/*  Attributes time_groups, time_nhoods, resource_groups, event_groups, and  */
/*  free_supply_chunks are private and must remain so, because they denote   */
/*  things that are created specifically within this solution and are not    */
/*  to be copied.                                                            */
/*                                                                           */
/*****************************************************************************/

struct khe_soln_rec {
  INHERIT_GROUP_MONITOR
  bool				placeholder;		/* t if placeholder  */
  KHE_INSTANCE			instance;		/* instance solved   */
  ARRAY_KHE_SOLN_GROUP		soln_groups;		/* soln groups       */
  KML_ERROR			invalid_error;		/* only if invalid   */
  char				*description;		/* description       */
  float				running_time;		/* running_time      */
  KHE_EVENNESS_HANDLER		evenness_handler;	/* evenness handler  */
  ARRAY_KHE_MONITOR_LINK	free_monitor_links;	/* free list         */
  ARRAY_KHE_TRACE		free_traces;		/* free list         */
  KHE_PATH			main_path;		/* the main path     */
  ARRAY_KHE_MARK		marks;			/* all marks         */
  ARRAY_KHE_MARK		free_marks;		/* free list of marks*/
  ARRAY_KHE_PATH		free_paths;		/* free list of paths*/
  bool				time_lset_building;	/* when building tl  */
  LSET				time_lset;		/* time lset         */
  LSET_TABLE			time_lset_table;	/* table of lsets    */
  KHE_RESOURCE_TYPE		resource_lset_rt;	/* when building rl  */
  LSET				resource_lset;		/* resource lset     */
  LSET_TABLE			resource_lset_table;	/* table of lsets    */
  KHE_EVENT_GROUP		curr_event_group;	/* temp variable     */
  ARRAY_KHE_TIME_GROUP		time_groups;		/* all created ones  */
  ARRAY_KHE_TIME_GROUP_NHOOD	time_nhoods;		/* all created ones  */
  ARRAY_KHE_RESOURCE_GROUP	resource_groups;	/* all created ones  */
  ARRAY_KHE_EVENT_GROUP		event_groups;		/* all created ones  */
  ARRAY_KHE_MEET_BOUND		meet_bounds;		/* all meet bounds   */
  ARRAY_KHE_TASK_BOUND		task_bounds;		/* all meet bounds   */
  ARRAY_KHE_RESOURCE_IN_SOLN	resources_in_soln;	/* res. monitors     */
  ARRAY_KHE_EVENT_IN_SOLN	events_in_soln;		/* event monitors    */
  ARRAY_KHE_MONITOR		monitors;		/* all monitors      */
  ARRAY_KHE_MEET		meets;			/* meets             */
  ARRAY_KHE_MEET		free_meets;		/* free meets        */
  ARRAY_KHE_MEET_BOUND		free_meet_bounds;	/* free meet bounds  */
  ARRAY_KHE_MEET		time_to_cycle_meet;	/* maps time to c.m. */
  ARRAY_INT			time_to_cycle_offset;	/* maps time to c.o. */
  ARRAY_KHE_TIME_GROUP		packing_time_groups;	/* packing time grps */
  ARRAY_KHE_NODE		free_nodes;		/* free nodes        */
  ARRAY_KHE_NODE		nodes;			/* layer tree nodes  */
  ARRAY_KHE_LAYER		free_layers;		/* free layers       */
  ARRAY_KHE_ZONE		free_zones;		/* free zones        */
  ARRAY_KHE_TASK		tasks;			/* tasks             */
  ARRAY_KHE_TASK		free_tasks;		/* free tasks        */
  ARRAY_KHE_TASK_BOUND		free_task_bounds;	/* free meet bounds  */
  ARRAY_KHE_TASKING		taskings;		/* taskings          */
  ARRAY_KHE_TASKING		free_taskings;		/* free taskings     */
  KHE_MATCHING_TYPE		matching_type;		/* matching type     */
  KHE_COST			matching_weight;	/* weight of matching*/
  KHE_MATCHING			matching;		/* the matching      */
  ARRAY_KHE_MATCHING_SUPPLY_CHUNK matching_free_supply_chunks;	/* free list */
  ARRAY_SHORT			matching_zero_domain;	/* domain { 0 }      */
  int				diversifier;		/* diversifier       */
  int				global_visit_num;	/* global visit num  */
  KHE_STATS_TIMER		timer;			/* timer             */
  float				time_limit;		/* time limit        */
  KHE_SOLN			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddInitialCycleMeet(KHE_SOLN soln)                           */
/*                                                                           */
/*  Add the initial cycle meet to soln, that is, if there is at least one    */
/*  time in the instance.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheSolnAddInitialCycleMeet(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  KHE_TIME t;  KHE_MEET meet;  int i;
  ins = KheSolnInstance(soln);
  if( KheInstanceTimeCount(ins) > 0 )
  {
    /* add the meet, setting various fields appropriately for a cycle meet */
    meet = KheMeetMake(soln, KheInstanceTimeCount(ins), NULL);
    t = KheInstanceTime(ins, 0);
    KheMeetSetAssignedTimeIndexAndDomain(meet, t);
    KheMeetAssignFix(meet);

    /* initialize time_to_cycle_meet and time_to_cycle_offset */
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
    {
      MArrayAddLast(soln->time_to_cycle_meet, meet);
      MArrayAddLast(soln->time_to_cycle_offset, i);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddCycleTasks(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Add the cycle tasks to the soln.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheSolnAddCycleTasks(KHE_SOLN soln)
{
  int i;  KHE_RESOURCE r;
  for( i = 0;  i < KheInstanceResourceCount(soln->instance);  i++ )
  {
    r = KheInstanceResource(soln->instance, i);
    KheCycleTaskMake(soln, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAssignResourceConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_ASSIGN_RESOURCE_CONSTRAINT c)                       */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAssignResourceConstraintMonitors(
  KHE_SOLN soln, KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  int i;  KHE_EVENT_RESOURCE er;  KHE_EVENT_IN_SOLN es;
  KHE_EVENT_RESOURCE_IN_SOLN ers;  KHE_ASSIGN_RESOURCE_MONITOR m;
  for( i = 0;  i < KheAssignResourceConstraintEventResourceCount(c);  i++ )
  {
    er = KheAssignResourceConstraintEventResource(c, i);
    es = MArrayGet(soln->events_in_soln,
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
    m = KheAssignResourceMonitorMake(ers, c);
    if( DEBUG5 )
      fprintf(stderr, "new KheAssignResourceMonitor %p (attached %d)\n",
	(void *) m, (int) KheMonitorAttachedToSoln((KHE_MONITOR) m));
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAssignTimeConstraintMonitors(                   */
/*    KHE_SOLN soln, KHE_ASSIGN_TIME_CONSTRAINT c)                           */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAssignTimeConstraintMonitors(
  KHE_SOLN soln, KHE_ASSIGN_TIME_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_ASSIGN_TIME_MONITOR m;
  for( i = 0;  i < KheAssignTimeConstraintEventCount(c);  i++ )
  {
    e = KheAssignTimeConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheAssignTimeMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheAssignTimeConstraintEventGroupCount(c);  i++ )
  {
    eg = KheAssignTimeConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheAssignTimeMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachSplitEventsConstraintMonitors(                  */
/*    KHE_SOLN soln, KHE_SPLIT_EVENTS_CONSTRAINT c)                          */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachSplitEventsConstraintMonitors(
  KHE_SOLN soln, KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_SPLIT_EVENTS_MONITOR m;
  if( DEBUG6 )
    fprintf(stderr, "[ KheSolnMakeAndAttachSplitEventsConstraintMonitors()\n");
  for( i = 0;  i < KheSplitEventsConstraintEventCount(c);  i++ )
  {
    e = KheSplitEventsConstraintEvent(c, i);
    if( DEBUG6 )
    {
      fprintf(stderr, "  event %d ", KheEventIndex(e));
      KheEventDebug(e, 1, 0, stderr);
    }
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheSplitEventsMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheSplitEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheSplitEventsConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      if( DEBUG6 )
      {
	fprintf(stderr, "    event group event %d ", KheEventIndex(e));
	KheEventDebug(e, 1, 0, stderr);
      }
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheSplitEventsMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  if( DEBUG6 )
    fprintf(stderr, "] KheSolnMakeAndAttachSplitEventsConstraintMonitors()\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(
  KHE_SOLN soln, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m;
  for( i = 0;  i < KheDistributeSplitEventsConstraintEventCount(c);  i++ )
  {
    e = KheDistributeSplitEventsConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheDistributeSplitEventsMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheDistributeSplitEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheDistributeSplitEventsConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheDistributeSplitEventsMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachPreferResourcesConstraintMonitors(              */
/*    KHE_SOLN soln, KHE_PREFER_RESOURCES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachPreferResourcesConstraintMonitors(
  KHE_SOLN soln, KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  int i;  KHE_EVENT_RESOURCE er;
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_PREFER_RESOURCES_MONITOR m;
  for( i = 0;  i < KhePreferResourcesConstraintEventResourceCount(c);  i++ )
  {
    er = KhePreferResourcesConstraintEventResource(c, i);
    es = MArrayGet(soln->events_in_soln,
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
    m = KhePreferResourcesMonitorMake(ers, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachPreferTimesConstraintMonitors(                  */
/*    KHE_SOLN soln, KHE_PREFER_TIMES_CONSTRAINT c)                          */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachPreferTimesConstraintMonitors(
  KHE_SOLN soln, KHE_PREFER_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_PREFER_TIMES_MONITOR m;
  for( i = 0;  i < KhePreferTimesConstraintEventCount(c);  i++ )
  {
    e = KhePreferTimesConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KhePreferTimesMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KhePreferTimesConstraintEventGroupCount(c);  i++ )
  {
    eg = KhePreferTimesConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KhePreferTimesMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  int i;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m;
  for( i = 0;  i < KheAvoidSplitAssignmentsConstraintEventGroupCount(c);  i++ )
  {
    m = KheAvoidSplitAssignmentsMonitorMake(soln, c, i);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachSpreadEventsConstraintMonitors(                 */
/*    KHE_SOLN soln, KHE_SPREAD_EVENTS_CONSTRAINT c)                         */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachSpreadEventsConstraintMonitors(
  KHE_SOLN soln, KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  int i;  KHE_EVENT_GROUP eg;  KHE_SPREAD_EVENTS_MONITOR m;
  for( i = 0;  i < KheSpreadEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheSpreadEventsConstraintEventGroup(c, i);
    m = KheSpreadEventsMonitorMake(soln, c, eg);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLinkEventsConstraintMonitors(                   */
/*    KHE_SOLN soln, KHE_LINK_EVENTS_CONSTRAINT c)                           */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLinkEventsConstraintMonitors(
  KHE_SOLN soln, KHE_LINK_EVENTS_CONSTRAINT c)
{
  int i;  KHE_EVENT_GROUP eg;  KHE_LINK_EVENTS_MONITOR m;
  for( i = 0;  i < KheLinkEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheLinkEventsConstraintEventGroup(c, i);
    m = KheLinkEventsMonitorMake(soln, c, eg);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachOrderEventsConstraintMonitors(                  */
/*    KHE_SOLN soln, KHE_ORDER_EVENTS_CONSTRAINT c)                          */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachOrderEventsConstraintMonitors(
  KHE_SOLN soln, KHE_ORDER_EVENTS_CONSTRAINT c)
{
  int i, min_sep, max_sep;  KHE_EVENT e1, e2;  KHE_ORDER_EVENTS_MONITOR m;
  for( i = 0;  i < KheOrderEventsConstraintEventPairCount(c);  i++ )
  {
    e1 = KheOrderEventsConstraintFirstEvent(c, i);
    e2 = KheOrderEventsConstraintSecondEvent(c, i);
    min_sep = KheOrderEventsConstraintMinSeparation(c, i);
    max_sep = KheOrderEventsConstraintMaxSeparation(c, i);
    m = KheOrderEventsMonitorMake(soln, c, e1, e2, min_sep, max_sep);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidClashesConstraintMonitors(                 */
/*    KHE_SOLN soln, KHE_AVOID_CLASHES_CONSTRAINT c)                         */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidClashesConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_CLASHES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_AVOID_CLASHES_MONITOR m;
  for( i = 0;  i < KheAvoidClashesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheAvoidClashesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheAvoidClashesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheAvoidClashesConstraintResourceCount(c);  i++ )
  {
    r = KheAvoidClashesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheAvoidClashesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m;
  for( i=0; i < KheAvoidUnavailableTimesConstraintResourceGroupCount(c); i++ )
  {
    rg = KheAvoidUnavailableTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheAvoidUnavailableTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheAvoidUnavailableTimesConstraintResourceCount(c);  i++ )
  {
    r = KheAvoidUnavailableTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheAvoidUnavailableTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_IDLE_TIMES_MONITOR m;
  for( i = 0;  i < KheLimitIdleTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitIdleTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheLimitIdleTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitIdleTimesConstraintResourceCount(c);  i++ )
  {
    r = KheLimitIdleTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheLimitIdleTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(             */
/*    KHE_SOLN soln, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                    */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(
  KHE_SOLN soln, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_CLUSTER_BUSY_TIMES_MONITOR m;
  for( i = 0;  i < KheClusterBusyTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheClusterBusyTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheClusterBusyTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheClusterBusyTimesConstraintResourceCount(c);  i++ )
  {
    r = KheClusterBusyTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheClusterBusyTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_BUSY_TIMES_MONITOR m;
  for( i = 0;  i < KheLimitBusyTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitBusyTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheLimitBusyTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitBusyTimesConstraintResourceCount(c);  i++ )
  {
    r = KheLimitBusyTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheLimitBusyTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(                */
/*    KHE_SOLN soln, KHE_LIMIT_WORKLOAD_CONSTRAINT c)                        */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_WORKLOAD_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_WORKLOAD_MONITOR m;
  for( i = 0;  i < KheLimitWorkloadConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitWorkloadConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
      m = KheLimitWorkloadMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitWorkloadConstraintResourceCount(c);  i++ )
  {
    r = KheLimitWorkloadConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
    m = KheLimitWorkloadMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachConstraintMonitors(KHE_SOLN soln)               */
/*                                                                           */
/*  Make and attach the constraint monitors of soln.                         */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachConstraintMonitors(KHE_SOLN soln)
{
  int i;  KHE_CONSTRAINT c;  KHE_INSTANCE ins;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    switch( KheConstraintTag(c) )
    {
      case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAssignResourceConstraintMonitors(soln,
	  (KHE_ASSIGN_RESOURCE_CONSTRAINT) c);
	break;

      case KHE_ASSIGN_TIME_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAssignTimeConstraintMonitors(soln,
	  (KHE_ASSIGN_TIME_CONSTRAINT) c);
	break;

      case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachSplitEventsConstraintMonitors(soln,
	  (KHE_SPLIT_EVENTS_CONSTRAINT) c);
	break;

      case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(soln,
	  (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c);
	break;

      case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachPreferResourcesConstraintMonitors(soln,
	  (KHE_PREFER_RESOURCES_CONSTRAINT) c);
	break;

      case KHE_PREFER_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachPreferTimesConstraintMonitors(soln,
	  (KHE_PREFER_TIMES_CONSTRAINT) c);
	break;

      case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(soln,
	  (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c);
	break;

      case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachSpreadEventsConstraintMonitors(soln,
	  (KHE_SPREAD_EVENTS_CONSTRAINT) c);
	break;

      case KHE_LINK_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLinkEventsConstraintMonitors(soln,
	  (KHE_LINK_EVENTS_CONSTRAINT) c);
	break;

      case KHE_ORDER_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachOrderEventsConstraintMonitors(soln,
	  (KHE_ORDER_EVENTS_CONSTRAINT) c);
	break;

      case KHE_AVOID_CLASHES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidClashesConstraintMonitors(soln,
	  (KHE_AVOID_CLASHES_CONSTRAINT) c);
	break;

      case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(soln,
	  (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(soln,
	  (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c);
	break;

      case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(soln,
	  (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(soln,
	  (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(soln,
	  (KHE_LIMIT_WORKLOAD_CONSTRAINT) c);
	break;

      default:

	MAssert(false, 
	  "KheSolnMakeAndAttachConstraintMonitors illegal constraint tag");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)       */
/*                                                                           */
/*  Add soln_group to soln; soln's instance is in soln_group's archive.      */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)
{
  MArrayAddLast(soln->soln_groups, soln_group);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)    */
/*                                                                           */
/*  Delete soln_group from soln's list of soln_groups.  It must be present.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)
{
  int pos;
  if( !MArrayContains(soln->soln_groups, soln_group, &pos) )
    MAssert(false, "KheSolnDeleteSolnGroup internal error");
  MArrayRemove(soln->soln_groups, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnMake(KHE_INSTANCE ins)                                   */
/*                                                                           */
/*  Make and return a new soln of instance, with no meets, no nodes, and     */
/*  no layers.                                                               */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnMake(KHE_INSTANCE ins)
{
  KHE_SOLN res;  int i;  /* KHE_EVENT e; */  KHE_RESOURCE r;
  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;

  if( DEBUG1 )
    fprintf(stderr, "[ KheSolnMake(%s)\n",
      KheInstanceId(ins) != NULL ? KheInstanceId(ins) : "-");
  MAssert(KheInstanceComplete(ins),
    "KheSolnMake called before KheInstanceMakeEnd");

  /* attributes inherited from KHE_GROUP_MONITOR */
  MMake(res);
  MArrayInit(res->monitors); /* must put this early! */
  KheMonitorInitCommonFields((KHE_MONITOR) res, res, KHE_GROUP_MONITOR_TAG);
  MArrayInit(res->child_links);
  MArrayInit(res->defect_links);
  MArrayInit(res->traces);
  res->sub_tag = -1;
  res->sub_tag_label = "Soln";

  /* placeholder, instance, solution group, invalid_error, description, time */
  res->placeholder = false;
  res->instance = ins;
  MArrayInit(res->soln_groups);
  res->invalid_error = NULL;
  res->description = NULL;
  res->running_time = -1.0; /* negative means absent */

  /* evenness handler */
  res->evenness_handler = NULL;

  /* traces and transactions */
  MArrayInit(res->free_monitor_links);
  MArrayInit(res->free_traces);

  /* marks and paths */
  res->main_path = KhePathMake(res);
  MArrayInit(res->marks);
  MArrayInit(res->free_marks);
  MArrayInit(res->free_paths);

  /* group and domain construction variables */
  res->time_lset_building = false;
  res->time_lset = LSetNew();
  res->time_lset_table = LSetTableMake();
  res->resource_lset_rt = NULL;
  res->resource_lset = LSetNew();
  res->resource_lset_table = LSetTableMake();
  res->curr_event_group = NULL;
  MArrayInit(res->time_groups);
  MArrayInit(res->time_nhoods);
  MArrayInit(res->resource_groups);
  MArrayInit(res->event_groups);
  MArrayInit(res->meet_bounds);
  MArrayInit(res->task_bounds);

  /* resource in soln objects, one per resource */
  MArrayInit(res->resources_in_soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    rs = KheResourceInSolnMake(res, r);
    MArrayAddLast(res->resources_in_soln, rs);
  }

  /* event in soln objects, one per instance event */
  MArrayInit(res->events_in_soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    /* e = KheInstanceEvent(ins, i); */
    es = KheEventInSolnMake(res, KheInstanceEvent(ins, i));
    MArrayAddLast(res->events_in_soln, es);
  }

  /* meets */
  MArrayInit(res->free_meets);
  MArrayInit(res->meets);

  /* meet bounds */
  MArrayInit(res->free_meet_bounds);

  /* cycle meet stuff */
  MArrayInit(res->time_to_cycle_meet);
  MArrayInit(res->time_to_cycle_offset);
  MArrayInit(res->packing_time_groups);

  /* nodes */
  MArrayInit(res->free_nodes);
  MArrayInit(res->nodes);

  /* layers */
  MArrayInit(res->free_layers);

  /* zones */
  MArrayInit(res->free_zones);

  /* tasks and taskings */
  MArrayInit(res->tasks);
  MArrayInit(res->free_tasks);
  MArrayInit(res->taskings);
  MArrayInit(res->free_taskings);

  /* task bounds */
  MArrayInit(res->free_task_bounds);

  /* matching */
  res->matching_type = KHE_MATCHING_TYPE_SOLVE;
  res->matching_weight = 0;
  res->matching = NULL;  /* no matching initially */
  MArrayInit(res->matching_free_supply_chunks);
  MArrayInit(res->matching_zero_domain);
  MArrayAddLast(res->matching_zero_domain, 0);

  /* cycle meets and tasks */
  KheSolnAddInitialCycleMeet(res);
  KheSolnAddCycleTasks(res);

  /* diversifier, visit_num and copy */
  res->diversifier = 0;
  res->global_visit_num = 0;
  res->timer = KheStatsTimerMake();
  res->time_limit = -1.0;
  res->copy = NULL;

  /* make and attach constraint monitors */
  KheSolnMakeAndAttachConstraintMonitors(res);

  if( DEBUG1 )
    fprintf(stderr, "] KheSolnMake returning\n");
  MAssert(res->instance != NULL, "KheSolnMake internal error");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDoDelete(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Carry out that part of the deletion of soln which is common to both      */
/*  KheSolnDelete and KheSolnReduceToPlaceholder.  This is everything        */
/*  except deletion from the solution group and freeing the object.          */
/*                                                                           */
/*  Implementation note.  Deletion requires a plan.  We must ensure that     */
/*  every object to be deleted is deleted exactly once, and we must do it    */
/*  in a way that re-uses user-accessible operations for deleting parts of   */
/*  a solution.  This plan shows which deletion functions delete what.       */
/*                                                                           */
/*    KheSolnDoDelete (public)                                               */
/*      KheLayerDelete (public, deletes only the layer)                      */
/*      KheMeetDelete (public)                                               */
/*        KheSolnResourceDelete (public)                                     */
/*          KheOrdinaryDemandMonitorDelete (private)                         */
/*        KheMatchingDemandChunkDelete (private, only when no nodes)         */
/*      KheNodeDelete (public, deletes only the node)                        */
/*      KheTimeDomainCacheDelete (private)                                   */
/*      KheEventInSolnDelete (private, does not free any monitors)           */
/*        KheEventResourceInSolnDelete (private, doesn't free mons/chunks)   */
/*      KheResourceInSolnDelete (private, doesn't free monitors or chunks)   */
/*        KheResourceInSolnWorkloadRequirementDelete (private)               */
/*      KheTimeDomainFree (private, doesn't free any elements)               */
/*      KheEventGroupDelete (private, doesn't free any elements)             */
/*      KheResourceGroupDelete (private, doesn't free any elements)          */
/*      KheTimeGroupDelete (private, doesn't free any elements)              */
/*      KheMonitorDelete (private) + its many redefs in child classes        */
/*      KheOrdinaryDemandMonitorDelete (private)                             */
/*      KheWorkloadDemandMonitorDelete (private)                             */
/*      KheTransactionFree (deletes free transactions, user deletes others)  */
/*      KheGroupMonitorDelete (public, deletes just the one monitor)         */
/*                                                                           */
/*  NB the following functions are not deletions in the present sense,       */
/*  because they move their objects to free lists rather than freeing them.  */
/*                                                                           */
/*      KheTransactionDelete (public)                                        */
/*      KheSolnMatchingAddOrdinarySupplyChunkToFreeList                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnDoDelete(KHE_SOLN soln)
{
  KHE_MONITOR m;  KHE_TASKING tasking;  KHE_TASK task;  KHE_MEET meet;
  KHE_NODE node;

  /***********************************************************************/
  /*                                                                     */
  /*  First phase, in which the normal solution invariant is maintained  */
  /*                                                                     */
  /***********************************************************************/

  /* delete taskings and tasks */
  while( MArraySize(soln->taskings) > 0 )
  {
    tasking = MArrayLast(soln->taskings);
    MAssert(KheTaskingSoln(tasking) == soln, "KheSolnDelete internal error 1");
    KheTaskingDelete(tasking);
  }
  while( MArraySize(soln->tasks) > 0 )
  {
    task = MArrayLast(soln->tasks);
    MAssert(KheTaskSoln(task) == soln, "KheSolnDelete internal error 2");
    KheTaskDelete(task);
  }

  /* delete meets */
  while( MArraySize(soln->meets) > 0 )
  {
    meet = MArrayLast(soln->meets);
    MAssert(KheMeetSoln(meet) == soln, "KheSolnDelete internal error 3");
    KheMeetDelete(meet);
  }

  /* delete task bounds */
  while( MArraySize(soln->task_bounds) > 0 )
    KheTaskBoundDelete(MArrayLast(soln->task_bounds));

  /* delete meet bounds */
  while( MArraySize(soln->meet_bounds) > 0 )
    KheMeetBoundDelete(MArrayLast(soln->meet_bounds));

  /* delete nodes (NB meets must be deleted before nodes, else nodes fail) */
  while( MArraySize(soln->nodes) > 0 )
  {
    node = MArrayLast(soln->nodes);
    MAssert(KheNodeSoln(node) == soln, "KheSolnDelete internal error 4");
    if( !KheNodeDelete(node) )
      MAssert(false, "KheSolnDelete internal error 5");
  }

  /* delete monitors (these will remove themselves from the array) */
  while( MArraySize(soln->monitors) > 1 )  /* 1 for soln itself */
  {
    m = MArrayLast(soln->monitors);
    MAssert(KheMonitorSoln(m) == soln, "KheSolnDelete internal error 6");
    if( m != (KHE_MONITOR) soln )
      KheMonitorDelete(m);
  }


  /***********************************************************************/
  /*                                                                     */
  /*  Second phase, in which the solution invariant is not maintained    */
  /*  (working up the object's list of attributes)                       */
  /*                                                                     */
  /***********************************************************************/

  /* free meets, tasks, and taskings */
  while( MArraySize(soln->free_taskings) > 0 )
    KheTaskingFree(MArrayRemoveLast(soln->free_taskings));
  MArrayFree(soln->free_taskings);
  MArrayFree(soln->taskings);
  while( MArraySize(soln->free_meets) > 0 )
    KheMeetUnMake(MArrayRemoveLast(soln->free_meets));
  MArrayFree(soln->free_meets);
  while( MArraySize(soln->free_nodes) > 0 )
    KheNodeUnMake(MArrayRemoveLast(soln->free_nodes));
  MArrayFree(soln->free_nodes);
  while( MArraySize(soln->free_layers) > 0 )
    KheLayerUnMake(MArrayRemoveLast(soln->free_layers));
  MArrayFree(soln->free_layers);
  while( MArraySize(soln->free_zones) > 0 )
    KheZoneUnMake(MArrayRemoveLast(soln->free_zones));
  MArrayFree(soln->free_zones);
  while( MArraySize(soln->free_task_bounds) > 0 )
    KheTaskBoundUnMake(MArrayRemoveLast(soln->free_task_bounds));
  while( MArraySize(soln->free_meet_bounds) > 0 )
    KheMeetBoundUnMake(MArrayRemoveLast(soln->free_meet_bounds));
  while( MArraySize(soln->free_meet_bounds) > 0 )
    KheMeetBoundUnMake(MArrayRemoveLast(soln->free_meet_bounds));
  MArrayFree(soln->free_meet_bounds);
  while( MArraySize(soln->free_tasks) > 0 )
    KheTaskUnMake(MArrayRemoveLast(soln->free_tasks));
  MArrayFree(soln->free_tasks);
  MArrayFree(soln->tasks);

  /* free various arrays whose elements are freed separately */
  MArrayFree(soln->nodes);
  MArrayFree(soln->meets);
  MArrayFree(soln->time_to_cycle_meet);
  MArrayFree(soln->time_to_cycle_offset);
  MArrayFree(soln->packing_time_groups);
  MArrayFree(soln->monitors);
  MArrayFree(soln->meet_bounds);
  MArrayFree(soln->task_bounds);

  /* delete event in soln objects */
  while( MArraySize(soln->events_in_soln) > 0 )
    KheEventInSolnDelete(MArrayRemoveLast(soln->events_in_soln));
  MArrayFree(soln->events_in_soln);

  /* delete resource in soln objects */
  while( MArraySize(soln->resources_in_soln) > 0 )
    KheResourceInSolnDelete(MArrayRemoveLast(soln->resources_in_soln));
  MArrayFree(soln->resources_in_soln);

  /* delete event groups */
  while( MArraySize(soln->event_groups) > 0 )
    KheEventGroupDelete(MArrayRemoveLast(soln->event_groups));
  MArrayFree(soln->event_groups);

  /* delete resource groups */
  while( MArraySize(soln->resource_groups) > 0 )
    KheResourceGroupDelete(MArrayRemoveLast(soln->resource_groups));
  MArrayFree(soln->resource_groups);

  /* delete time groups */
  while( MArraySize(soln->time_groups) > 0 )
    KheTimeGroupDelete(MArrayRemoveLast(soln->time_groups));
  MArrayFree(soln->time_groups);

  /* delete time nhoods */
  while( MArraySize(soln->time_nhoods) > 0 )
    KheTimeGroupNHoodDelete(MArrayRemoveLast(soln->time_nhoods));
  MArrayFree(soln->time_nhoods);

  /* delete free monitor links */
  while( MArraySize(soln->free_monitor_links) > 0 )
    MFree(MArrayRemoveLast(soln->free_monitor_links));
  MArrayFree(soln->free_monitor_links);

  /* delete free traces (user must delete others) */
  while( MArraySize(soln->free_traces) > 0 )
    KheTraceFree(MArrayRemoveLast(soln->free_traces));
  MArrayFree(soln->free_traces);

  /* delete all marks and paths */
  KhePathFree(soln->main_path);
  while( MArraySize(soln->marks) > 0 )
    KheMarkFree(MArrayRemoveLast(soln->marks));
  while( MArraySize(soln->free_marks) > 0 )
    KheMarkFree(MArrayRemoveLast(soln->free_marks));
  while( MArraySize(soln->free_paths) > 0 )
    KhePathFree(MArrayRemoveLast(soln->free_paths));

  /* delete monitors */
  if( DEBUG8 && MArraySize(soln->child_links) > 0 )
  {
    int i;  KHE_MONITOR_LINK link;
    MArrayForEach(soln->child_links, &link, &i)
      KheMonitorDebug(link->child, 1, 2, stderr);
  }
  MAssert(MArraySize(soln->child_links) == 0,
    "KheSolnDoDelete internal error 7 (%d child monitors)",
    MArraySize(soln->child_links));
  MArrayFree(soln->child_links);
  MAssert(MArraySize(soln->defect_links) == 0,
    "KheSolnDoDelete internal error 8 (%d defects)",
    MArraySize(soln->defect_links));
  MArrayFree(soln->defect_links);

  /* check traces */
  MAssert(MArraySize(soln->traces) == 0,
    "KheSolnDoDelete:  soln is currently being traced");
  MArrayFree(soln->traces);

  /* delete the matching (should be no demand nodes or chunks left by now) */
  if( soln->matching != NULL )
  {
    KheMatchingDelete(soln->matching);
    soln->matching = NULL;
  }
  MArrayFree(soln->matching_free_supply_chunks);

  /* delete evenness handler */
  if( soln->evenness_handler != NULL )
    KheEvennessHandlerDelete(soln->evenness_handler);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDelete(KHE_SOLN soln)                                        */
/*                                                                           */
/*  Delete soln.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnDelete(KHE_SOLN soln)
{
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheSolnDelete(");
    KheSolnDebug(soln, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  while( MArraySize(soln->soln_groups) > 0 )
    KheSolnGroupDeleteSoln(MArrayLast(soln->soln_groups), soln);
  KheSolnDoDelete(soln);
  KheStatsTimerDelete(soln->timer);
  MFree(soln);
  if( DEBUG4 )
    fprintf(stderr, "] KheSolnDelete returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnReduceToPlaceholder(KHE_SOLN soln)                           */
/*                                                                           */
/*  Reduce soln to a placeholder.                                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnReduceToPlaceholder(KHE_SOLN soln)
{
  KHE_COST cost;
  MAssert(!KheSolnIsPlaceholder(soln), 
    "KheSolnReduceToPlaceholder:  soln is already a placeholder");
  cost = soln->cost;
  KheSolnDoDelete(soln);
  soln->cost = cost;
  soln->placeholder = true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnIsPlaceholder(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Return true if soln is a placeholder solution.                           */
/*                                                                           */
/*****************************************************************************/

bool KheSolnIsPlaceholder(KHE_SOLN soln)
{
  return soln->placeholder;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnReduceToInvalid(KHE_SOLN soln, KML_ERROR ke)                 */
/*                                                                           */
/*  Make soln invalid.  It may already be a placeholder, but it may not      */
/*  already be invalid.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnReduceToInvalid(KHE_SOLN soln, KML_ERROR ke)
{
  MAssert(!KheSolnIsInvalid(soln), 
    "KheSolnReduceToInvalid:  soln is already invalid");
  MAssert(ke != NULL, "KheSolnReduceToInvalid:  ke == NULL");
  if( !KheSolnIsPlaceholder(soln) )
    KheSolnReduceToPlaceholder(soln);
  soln->invalid_error = ke;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnIsInvalid(KHE_SOLN soln)                                     */
/*                                                                           */
/*  Return true if soln is invalid.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheSolnIsInvalid(KHE_SOLN soln)
{
  return soln->invalid_error != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ERROR KheSolnInvalidError(KHE_SOLN soln)                             */
/*                                                                           */
/*  Return the error that rendered soln invalid, or NULL if not invalid.     */
/*                                                                           */
/*****************************************************************************/

KML_ERROR KheSolnInvalidError(KHE_SOLN soln)
{
  return soln->invalid_error;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetDescription(KHE_SOLN soln, char *description)             */
/*                                                                           */
/*  Set or reset the Description attribute of soln.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetDescription(KHE_SOLN soln, char *description)
{
  soln->description = description;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnDescription(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Return the Description attribute of soln.                                */
/*                                                                           */
/*****************************************************************************/

char *KheSolnDescription(KHE_SOLN soln)
{
  return soln->description;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetRunningTime(KHE_SOLN soln, float running_time)            */
/*                                                                           */
/*  Set or reset the RunningTime attribute of soln.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetRunningTime(KHE_SOLN soln, float running_time)
{
  soln->running_time = running_time;
}


/*****************************************************************************/
/*                                                                           */
/*  float KheSolnRunningTime(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Return the RunningTime attribute of soln.                                */
/*                                                                           */
/*****************************************************************************/

float KheSolnRunningTime(KHE_SOLN soln)
{
  return soln->running_time;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetBack(KHE_SOLN soln, void *back)                           */
/*                                                                           */
/*  Set the back pointer of soln.                                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetBack(KHE_SOLN soln, void *back)
{
  soln->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheSolnBack(KHE_SOLN soln)                                         */
/*                                                                           */
/*  Return the back pointer of soln.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheSolnBack(KHE_SOLN soln)
{
  return soln->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln)                                */
/*                                                                           */
/*  Carry out Phase 1 of the copying of soln.                                */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln)
{
  int i;  KHE_SOLN copy;  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;
  KHE_MEET meet;  KHE_NODE node;  KHE_MONITOR m;  KHE_MONITOR_LINK link;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_TASK task;  KHE_TASKING tasking;
  KHE_MEET_BOUND mb;  KHE_TASK_BOUND tb;
  if( soln->copy == NULL )
  {
    /* inherited from KHE_GROUP_MONITOR */
    MAssert(MArraySize(soln->traces) == 0,
      "KheSolnCopy cannot copy:  soln is currently being traced");
    MAssert(!KheSolnIsInvalid(soln),
      "KheSolnCopy cannot copy:  soln is invalid");
    MAssert(!KheSolnIsPlaceholder(soln),
      "KheSolnCopy cannot copy:  soln is a placeholder");
    MMake(copy);
    if( DEBUG7 )
      fprintf(stderr, "[ %p = KheSolnCopyPhase1(%p)\n", (void *) copy,
	(void *) soln);
    soln->copy = copy;
    KheMonitorCopyCommonFieldsPhase1((KHE_MONITOR) copy, (KHE_MONITOR) soln);
    MArrayInit(copy->child_links);
    MArrayForEach(soln->child_links, &link, &i)
      MArrayAddLast(copy->child_links, KheMonitorLinkCopyPhase1(link));
    MArrayInit(copy->defect_links);
    MArrayForEach(soln->defect_links, &link, &i)
      MArrayAddLast(copy->defect_links, KheMonitorLinkCopyPhase1(link));
    MArrayInit(copy->traces);
    copy->sub_tag = soln->sub_tag;
    copy->sub_tag_label = soln->sub_tag_label;

    /* specific to KHE_SOLN */
    copy->placeholder = soln->placeholder;
    copy->instance = soln->instance;
    MArrayInit(copy->soln_groups);
    copy->invalid_error = NULL;
    copy->evenness_handler = soln->evenness_handler == NULL ? NULL :
      KheEvennessHandlerCopyPhase1(soln->evenness_handler);
    MArrayInit(copy->free_monitor_links);
    MArrayInit(copy->free_traces);
    copy->main_path = KhePathMake(copy);
    MArrayInit(copy->marks);
    MArrayInit(copy->free_marks);
    MArrayInit(copy->free_paths);
    copy->time_lset_building = false;
    copy->time_lset = LSetNew();
    copy->time_lset_table = LSetTableMake();
    copy->resource_lset_rt = NULL;
    copy->resource_lset = LSetNew();
    copy->resource_lset_table = LSetTableMake();
    copy->curr_event_group = NULL;
    MArrayInit(copy->time_groups);      /* don't copy these ones! */
    MArrayInit(copy->time_nhoods);      /* don't copy these ones! */
    MArrayInit(copy->resource_groups);  /* don't copy these ones! */
    MArrayInit(copy->event_groups);     /* don't copy these ones! */
    MArrayInit(copy->resources_in_soln);
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      MArrayAddLast(copy->resources_in_soln, KheResourceInSolnCopyPhase1(rs));
    MArrayInit(copy->events_in_soln);
    MArrayForEach(soln->events_in_soln, &es, &i)
      MArrayAddLast(copy->events_in_soln, KheEventInSolnCopyPhase1(es));
    MArrayInit(copy->monitors);
    MArrayForEach(soln->monitors, &m, &i)
      MArrayAddLast(copy->monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->meet_bounds);
    MArrayForEach(soln->meet_bounds, &mb, &i)
      MArrayAddLast(copy->meet_bounds, KheMeetBoundCopyPhase1(mb));
    MArrayInit(copy->task_bounds);
    MArrayForEach(soln->task_bounds, &tb, &i)
      MArrayAddLast(copy->task_bounds, KheTaskBoundCopyPhase1(tb));
    MArrayInit(copy->meets);
    MArrayForEach(soln->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->time_to_cycle_meet);
    MArrayForEach(soln->time_to_cycle_meet, &meet, &i)
      MArrayAddLast(copy->time_to_cycle_meet, KheMeetCopyPhase1(meet));
    MArrayInit(copy->time_to_cycle_offset);
    MArrayAppend(copy->time_to_cycle_offset, soln->time_to_cycle_offset, i);
    MArrayInit(copy->packing_time_groups);
    MArrayAppend(copy->packing_time_groups, soln->packing_time_groups, i);
    MArrayInit(copy->nodes);
    MArrayForEach(soln->nodes, &node, &i)
      MArrayAddLast(copy->nodes, KheNodeCopyPhase1(node));
    MArrayInit(copy->tasks);
    MArrayForEach(soln->tasks, &task, &i)
      MArrayAddLast(copy->tasks, KheTaskCopyPhase1(task));
    MArrayInit(copy->free_meets);
    MArrayInit(copy->free_nodes);
    MArrayInit(copy->free_layers);
    MArrayInit(copy->free_zones);
    MArrayInit(copy->free_meet_bounds);
    MArrayInit(copy->free_tasks);
    MArrayInit(copy->free_task_bounds);
    MArrayInit(copy->taskings);
    MArrayForEach(soln->taskings, &tasking, &i)
      MArrayAddLast(copy->taskings, KheTaskingCopyPhase1(tasking));
    MArrayInit(copy->free_taskings);
    copy->matching_type = soln->matching_type;
    copy->matching_weight = soln->matching_weight;
    copy->matching = soln->matching == NULL ? NULL :
      KheMatchingCopyPhase1(soln->matching);
    MArrayInit(copy->matching_free_supply_chunks);
    MArrayForEach(soln->matching_free_supply_chunks, &sc, &i)
      MArrayAddLast(copy->matching_free_supply_chunks,
	KheMatchingSupplyChunkCopyPhase1(sc));
    MArrayInit(copy->matching_zero_domain);
    MArrayAddLast(copy->matching_zero_domain, 0);
    copy->diversifier = soln->diversifier;
    copy->global_visit_num = soln->global_visit_num;
    copy->timer = KheStatsTimerCopy(soln->timer);
    copy->running_time = soln->running_time;
    copy->time_limit = soln->time_limit;
    copy->copy = NULL;
    if( DEBUG7 )
      fprintf(stderr, "] KheSolnCopyPhase1 returning\n");
  }
  return soln->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCopyPhase2(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Carry out Phase 2 of the copying of soln.                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnCopyPhase2(KHE_SOLN soln)
{
  int i;  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;  KHE_MEET meet;
  KHE_NODE node;  KHE_MONITOR m;  KHE_TASK task;  KHE_TASKING tasking;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_MONITOR_LINK link;
  if( soln->copy != NULL )
  {
    soln->copy = NULL;
    KheMonitorCopyCommonFieldsPhase2((KHE_MONITOR) soln);
    MArrayForEach(soln->child_links, &link, &i)
      KheMonitorLinkCopyPhase2(link);
    MArrayForEach(soln->defect_links, &link, &i)
      KheMonitorLinkCopyPhase2(link);
    if( soln->evenness_handler != NULL )
      KheEvennessHandlerCopyPhase2(soln->evenness_handler);
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnCopyPhase2(rs);
    MArrayForEach(soln->events_in_soln, &es, &i)
      KheEventInSolnCopyPhase2(es);
    MArrayForEach(soln->monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetCopyPhase2(meet);
    MArrayForEach(soln->nodes, &node, &i)
      KheNodeCopyPhase2(node);
    MArrayForEach(soln->tasks, &task, &i)
      KheTaskCopyPhase2(task);
    MArrayForEach(soln->taskings, &tasking, &i)
      KheTaskingCopyPhase2(tasking);
    MArrayForEach(soln->matching_free_supply_chunks, &sc, &i)
      KheMatchingSupplyChunkCopyPhase2(sc);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnCopy(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Make a deep copy of soln.                                                */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnCopy(KHE_SOLN soln)
{
  KHE_SOLN copy;

  /* probabilistic check for re-entrant call */
  MAssert(soln->copy == NULL, "re-entrant call on KheSolnCopy");

  /* check temporary group construction variables */
  MAssert(!soln->time_lset_building,
    "KheSolnCopy called while time group under construction");
  MAssert(soln->resource_lset_rt == NULL,
    "KheSolnCopy called while resource group under construction");
  MAssert(soln->curr_event_group == NULL,
    "KheSolnCopy called while event group under construction");
  MAssert(MArraySize(soln->marks) == 0,
    "KheSolnCopy called after unmatched KheMarkBegin");

  KheSolnMatchingUpdate(soln);
  copy = KheSolnCopyPhase1(soln);
  KheSolnCopyPhase2(soln);
  return copy;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheSolnInstance(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the instance of soln.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheSolnInstance(KHE_SOLN soln)
{
  return soln->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnSolnGroupCount(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Return the number of solution groups containing soln.                    */
/*                                                                           */
/*****************************************************************************/

int KheSolnSolnGroupCount(KHE_SOLN soln)
{
  return MArraySize(soln->soln_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln, int i)                    */
/*                                                                           */
/*  Return the i'th solution group containing soln.                          */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->soln_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "evenness handling"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln)               */
/*                                                                           */
/*  Return soln's evenness handler, or NULL if none.                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln)
{
  return soln->evenness_handler;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEvennessBegin(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Begin evenness handling.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnEvennessBegin(KHE_SOLN soln)
{
  MAssert(soln->evenness_handler == NULL,
    "KheSolnEvennessBegin: evenness monitoring already on");
  soln->evenness_handler = KheEvennessHandlerMake(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEvennessEnd(KHE_SOLN soln)                                   */
/*                                                                           */
/*  End evenness handling.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnEvennessEnd(KHE_SOLN soln)
{
  MAssert(soln->evenness_handler != NULL,
    "KheSolnEvennessEnd: evenness monitoring not on");
  KheEvennessHandlerDelete(soln->evenness_handler);
  soln->evenness_handler = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnHasEvenness(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return true when evenness handling is on in soln.                        */
/*                                                                           */
/*****************************************************************************/

bool KheSolnHasEvenness(KHE_SOLN soln)
{
  return soln->evenness_handler != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Ensure that all evenness monitors are attached.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln)
{
  MAssert(soln->evenness_handler != NULL,
    "KheSolnAttachAllEvennessMonitors: evenness monitoring not on");
  KheEvennessHandlerAttachAllEvennessMonitors(soln->evenness_handler);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Ensure that all evenness monitors are detached.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln)
{
  MAssert(soln->evenness_handler != NULL,
    "KheSolnDetachAllEvennessMonitors: evenness monitoring not on");
  KheEvennessHandlerDetachAllEvennessMonitors(soln->evenness_handler);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight) */
/*                                                                           */
/*  Set the weight of all evenness monitors.                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight)
{
  MAssert(soln->evenness_handler != NULL,
    "KheSolnSetAllEvennessMonitorWeights: evenness monitoring not on");
  KheEvennessHandleSetAllEvennessMonitorWeights(soln->evenness_handler, weight);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cost and monitors"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheCost(int hard_cost, int soft_cost)                           */
/*                                                                           */
/*  Combine hard_cost and soft_cost into a single cost.                      */
/*                                                                           */
/*****************************************************************************/
#define KHE_HARD_COST_WEIGHT ((KHE_COST) 1 << (KHE_COST) 32)

KHE_COST KheCost(int hard_cost, int soft_cost)
{
  return (KHE_COST) hard_cost * KHE_HARD_COST_WEIGHT + (KHE_COST) soft_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheHardCost(KHE_COST combined_cost)                                  */
/*                                                                           */
/*  Return the hard cost component of combined_cost.                         */
/*                                                                           */
/*****************************************************************************/

int KheHardCost(KHE_COST combined_cost)
{
  return (int) (combined_cost / KHE_HARD_COST_WEIGHT);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSoftCost(KHE_COST combined_cost)                                  */
/*                                                                           */
/*  Return the soft cost component of combined_cost.                         */
/*                                                                           */
/*****************************************************************************/

int KheSoftCost(KHE_COST combined_cost)
{
  return (int) (combined_cost % KHE_HARD_COST_WEIGHT);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheCostCmp(KHE_COST cost1, KHE_COST cost2)                           */
/*                                                                           */
/*  Return an int which is less than, equal to, or greater than zero if      */
/*  the first argument is respectively less than, equal to, or greater       */
/*  than the second.                                                         */
/*                                                                           */
/*****************************************************************************/

int KheCostCmp(KHE_COST cost1, KHE_COST cost2)
{
  if( KheHardCost(cost1) != KheHardCost(cost2) )
    return KheHardCost(cost1) - KheHardCost(cost2);
  else
    return KheSoftCost(cost1) - KheSoftCost(cost2);
}


/*****************************************************************************/
/*                                                                           */
/*  double KheCostShow(KHE_COST combined_cost)                               */
/*                                                                           */
/*  Return a floating point value suitable for displaying combined_cost.     */
/*                                                                           */
/*****************************************************************************/
#define KHE_COST_SHOW_DIGITS 99999

double KheCostShow(KHE_COST combined_cost)
{
  int soft_cost;
  soft_cost = KheSoftCost(combined_cost);
  if( soft_cost > KHE_COST_SHOW_DIGITS )
    soft_cost = KHE_COST_SHOW_DIGITS;
  return (double) KheHardCost(combined_cost) +
    (double) soft_cost / (KHE_COST_SHOW_DIGITS + 1);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnCost(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the total cost of soln.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnCost(KHE_SOLN soln)
{
  if( soln->matching != NULL )
    KheMatchingUnmatchedDemandNodeCount(soln->matching);
  return soln->cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnChildMonitorCount(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the number of child monitors of soln.                             */
/*                                                                           */
/*****************************************************************************/

int KheSolnChildMonitorCount(KHE_SOLN soln)
{
  return KheGroupMonitorChildMonitorCount((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i)                    */
/*                                                                           */
/*  Return the i'th child monitor of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i)
{
  return KheGroupMonitorChildMonitor((KHE_GROUP_MONITOR) soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDefectCount(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the number of defects (child monitors of non-zero cost) of soln.  */
/*                                                                           */
/*****************************************************************************/

int KheSolnDefectCount(KHE_SOLN soln)
{
  return KheGroupMonitorDefectCount((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i)                          */
/*                                                                           */
/*  Return the i'th defect (child monitor of non-zero cost) of soln.         */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i)
{
  return KheGroupMonitorDefect((KHE_GROUP_MONITOR) soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitors and cost"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSolnEventMonitorCount(KHE_SOLN soln, KHE_EVENT e)                 */
/*                                                                           */
/*  Return the number of event monitors of e in soln.                        */
/*                                                                           */
/*****************************************************************************/

int KheSolnEventMonitorCount(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitorCount(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i)       */
/*                                                                           */
/*  Return the i'th event monitor of e in soln.                              */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitor(es, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnEventCost(KHE_SOLN soln, KHE_EVENT e)                    */
/*                                                                           */
/*  Return the total cost of monitors applicable to e.                       */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnEventCost(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnCost(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnEventMonitorCost(KHE_SOLN soln, KHE_EVENT e,             */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of monitors of type tag applicable to e.           */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnEventMonitorCost(KHE_SOLN soln, KHE_EVENT e, KHE_MONITOR_TAG tag)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitorCost(es, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln,KHE_EVENT e)*/
/*                                                                           */
/*  Return the timetable of e in soln.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnTimetableMonitor(es);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource monitors and cost"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSolnEventResourceMonitorCount(KHE_SOLN soln,                      */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Return the number of event resource monitors of er in soln.              */
/*                                                                           */
/*****************************************************************************/

int KheSolnEventResourceMonitorCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnMonitorCount(ers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnEventResourceMonitor(KHE_SOLN soln,                   */
/*    KHE_EVENT_RESOURCE er, int i)                                          */
/*                                                                           */
/*  Return the i'th event resource monitor of er in soln.                    */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnEventResourceMonitor(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, int i)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnMonitor(ers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er)  */
/*                                                                           */
/*  Return the total cost of monitors applicable to er.                      */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnCost(ers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnEventResourceMonitorCost(KHE_SOLN soln,                  */
/*    KHE_EVENT_RESOURCE er, KHE_MONITOR_TAG tag)                            */
/*                                                                           */
/*  Return the total cost of monitors of type tag applicable to er.          */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnEventResourceMonitorCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er,
  KHE_MONITOR_TAG tag)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnMonitorCost(ers, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks assigned to resources"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r)          */
/*                                                                           */
/*  Return the number of tasks assigned r in soln.                           */
/*                                                                           */
/*****************************************************************************/

int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnAssignedTaskCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i)   */
/*                                                                           */
/*  Return the i'th task assigned to r in soln.                              */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnAssignedTask(rs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource monitors and cost"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSolnResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r)           */
/*                                                                           */
/*  Return the number of resource monitors of r in soln.                     */
/*                                                                           */
/*****************************************************************************/

int KheSolnResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnMonitorCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i) */
/*                                                                           */
/*  Return the i'th resource monitor of r in soln.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnMonitor(rs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnResourceCost(KHE_SOLN soln, KHE_RESOURCE r)              */
/*                                                                           */
/*  Return the total cost of monitors applicable to r.                       */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnResourceCost(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnCost(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,       */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of constraints of type tag applicable to r.        */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,
  KHE_MONITOR_TAG tag)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnMonitorCost(rs, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln,         */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the timetable of r in soln.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnTimetableMonitor(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i)         */
/*                                                                           */
/*  Return the i'th resource monitor of soln.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->resources_in_soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m, int *index_in_soln) */
/*                                                                           */
/*  Add m to soln, including setting m's soln_index.                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m)
{
  KheMonitorSetSolnIndex(m, MArraySize(soln->monitors));
  MArrayAddLast(soln->monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m)                  */
/*                                                                           */
/*  Delete m from soln.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m)
{
  KHE_MONITOR tmp;
  tmp = MArrayRemoveAndPlug(soln->monitors, KheMonitorSolnIndex(m));
  KheMonitorSetSolnIndex(tmp, KheMonitorSolnIndex(m));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMonitorCount(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return the number of monitors of soln.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSolnMonitorCount(KHE_SOLN soln)
{
  return MArraySize(soln->monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i)                         */
/*                                                                           */
/*  Return the i'th monitor of soln.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnBypassAndDeleteAllGroupMonitors(KHE_SOLN soln)               */
/*                                                                           */
/*  Bypass and delete every group monitor of soln.                           */
/*                                                                           */
/*****************************************************************************/

void KheSolnBypassAndDeleteAllGroupMonitors(KHE_SOLN soln)
{
  KHE_MONITOR m;  int i;
  MArrayForEach(soln->monitors, &m, &i)
    if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
    {
      KheGroupMonitorBypassAndDelete((KHE_GROUP_MONITOR) m);
      i--;
    }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheSolnGetMeetFromFreeList(KHE_SOLN soln)                       */
/*                                                                           */
/*  Return a new meet from soln's free list, or NULL if none.                */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheSolnGetMeetFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_meets) > 0 )
    return MArrayRemoveLast(soln->free_meets);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMeetToFreeList(KHE_SOLN soln, KHE_MEET meet)              */
/*                                                                           */
/*  Add meet to soln's free list of meets.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMeetToFreeList(KHE_SOLN soln, KHE_MEET meet)
{
  MArrayAddLast(soln->free_meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet)                        */
/*                                                                           */
/*  Add meet to soln, and set its soln and soln_index fields.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet)                       
{
  KheMeetSetSoln(meet, soln);
  KheMeetSetSolnIndex(meet, MArraySize(soln->meets));
  MArrayAddLast(soln->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet)                     */
/*                                                                           */
/*  Delete meet from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet)
{
  KHE_MEET tmp;

  /* remove from meets */
  tmp = MArrayRemoveAndPlug(soln->meets, KheMeetSolnIndex(meet));
  KheMeetSetSolnIndex(tmp, KheMeetSolnIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,                    */
/*    KHE_EVENT *problem_event)                                              */
/*                                                                           */
/*  Ensure that soln is a complete representation of its instance, by        */
/*  adding one meet to each meet set whose total duration is less than       */
/*  the duration of its event, and adding tasks to all meets as required.    */
/*                                                                           */
/*  Return true if successful, or, if prevented by the presence of an        */
/*  event whose meets' durations are already too great, set *problem_event   */
/*  to the first such event and return false.                                */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,
  KHE_EVENT *problem_event)
{
  int i;  KHE_EVENT_IN_SOLN es;
  MArrayForEach(soln->events_in_soln, &es, &i)
    if( !KheEventInSolnMakeCompleteRepresentation(es, problem_event) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAssignPreassignedTimes(KHE_SOLN soln)                        */
/*                                                                           */
/*  Assign preassigned times to all the meets of soln that have them and     */
/*  are not already assigned.                                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnAssignPreassignedTimes(KHE_SOLN soln)
{
  KHE_MEET meet;  int i;  KHE_TIME time;
  if( DEBUG9 )
    fprintf(stderr, "[ KheSolnAssignPreassignedTimes(soln)\n");
  MArrayForEach(soln->meets, &meet, &i)
    if( KheMeetAsst(meet) == NULL && KheMeetIsPreassigned(meet, &time) )
    {
      if( !KheMeetAssignTime(meet, time) )
	MAssert(false, "KheSolnAssignPreassignedTimes failed to assign");
    }
  if( DEBUG9 )
    fprintf(stderr, "] KheSolnAssignPreassignedTimes returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMeetCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of meets in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnMeetCount(KHE_SOLN soln)
{
  return MArraySize(soln->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheSolnMeet(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th meet of soln.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheSolnMeet(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i)               */
/*                                                                           */
/*  Return the i'th event in soln of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->events_in_soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e)                        */
/*                                                                           */
/*  Return the number of meets for e in soln.                                */
/*                                                                           */
/*****************************************************************************/

int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMeetCount(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i)                 */
/*                                                                           */
/*  Return the i'th meet of event e in soln.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMeet(es, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cycle meets"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t)                 */
/*                                                                           */
/*  Return the cycle meet covering t.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t)
{
  return MArrayGet(soln->time_to_cycle_meet, KheTimeIndex(t));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t)                */
/*                                                                           */
/*  Return the offset of t in its cycle meet.                                */
/*                                                                           */
/*****************************************************************************/

int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t)
{
  return MArrayGet(soln->time_to_cycle_offset, KheTimeIndex(t));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnTimeMayBeginBlock(KHE_SOLN soln, KHE_TIME t, int durn)       */
/*                                                                           */
/*  Return true if a meet of this durn may start at time t.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheSolnTimeMayBeginBlock(KHE_SOLN soln, KHE_TIME t, int durn)
{
  KHE_MEET cycle_meet;  int cycle_offset;
  cycle_meet = KheSolnTimeCycleMeet(soln, t);
  cycle_offset = KheSolnTimeCycleMeetOffset(soln, t);
  return cycle_offset + durn <= KheMeetDuration(cycle_meet);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KhePackingTimeGroupMake(KHE_SOLN soln, int duration)      */
/*                                                                           */
/*  Make the packing time group for meets of soln of this duration.          */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KhePackingTimeGroupMake(KHE_SOLN soln, int duration)
{
  KHE_TIME t;  int i;
  KheSolnTimeGroupBegin(soln);
  for( i = 0;  i < KheInstanceTimeCount(soln->instance);  i++ )
  {
    t = KheInstanceTime(soln->instance, i);
    if( KheSolnTimeMayBeginBlock(soln, t, duration) )
      KheSolnTimeGroupAddTime(soln, t);
  }
  return KheSolnTimeGroupEnd(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration)      */
/*                                                                           */
/*  Return the time group suitable for use as the domain of meets of         */
/*  the given duration.                                                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration)
{
  MArrayFill(soln->packing_time_groups, duration, NULL);
  if( MArrayGet(soln->packing_time_groups, duration - 1) == NULL )
    MArrayPut(soln->packing_time_groups, duration - 1,
      KhePackingTimeGroupMake(soln, duration));
  return MArrayGet(soln->packing_time_groups, duration - 1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)*/
/*                                                                           */
/*  Record the fact that a cycle meet has just split into meet1 and meet2.   */
/*                                                                           */
/*****************************************************************************/

void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)
{
  int i, ti;

  /* packing time groups are now out of date */
  MArrayClear(soln->packing_time_groups);

  /* reset time_to_cycle_meet and time_to_cycle_offset */
  for( i = 0;  i < KheMeetDuration(meet2);  i++ )
  {
    ti = i + KheMeetAssignedTimeIndex(meet2);  
    MArrayPut(soln->time_to_cycle_meet, ti, meet2);
    MArrayPut(soln->time_to_cycle_offset, ti, i);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)*/
/*                                                                           */
/*  Record the fact that these cycle meets are merging.                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)
{
  int i, ti;

  /* packing time groups are now out of date */
  MArrayClear(soln->packing_time_groups);

  /* reset time_to_cycle_meet and time_to_cycle_offset */
  for( i = 0;  i < KheMeetDuration(meet2);  i++ )
  {
    ti = i + KheMeetAssignedTimeIndex(meet2);  
    MArrayPut(soln->time_to_cycle_meet, ti, meet1);
    MArrayPut(soln->time_to_cycle_offset, ti, i + KheMeetDuration(meet1));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet bounds"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb)               */
/*                                                                           */
/*  Add mb to soln.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb)
{
  KheMeetBoundSetSoln(mb, soln);
  KheMeetBoundSetSolnIndex(mb, MArraySize(soln->meet_bounds));
  MArrayAddLast(soln->meet_bounds, mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb)            */
/*                                                                           */
/*  Delete mb from soln.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb)
{
  KHE_MEET_BOUND tmp;  int soln_index;
  tmp = MArrayRemoveLast(soln->meet_bounds);
  if( tmp != mb )
  {
    soln_index = KheMeetBoundSolnIndex(mb);
    KheMeetBoundSetSolnIndex(tmp, soln_index);
    MArrayPut(soln->meet_bounds, soln_index, tmp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_BOUND KheSolnGetMeetBoundFromFreeList(KHE_SOLN soln)            */
/*                                                                           */
/*  Return a meet bound object from soln's free list, or NULL if none.       */
/*                                                                           */
/*****************************************************************************/

KHE_MEET_BOUND KheSolnGetMeetBoundFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_meet_bounds) > 0 )
    return MArrayRemoveLast(soln->free_meet_bounds);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMeetBoundToFreeList(KHE_SOLN soln, KHE_MEET_BOUND mb)     */
/*                                                                           */
/*  Add mb to soln's free list.                                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMeetBoundToFreeList(KHE_SOLN soln, KHE_MEET_BOUND mb)
{
  MArrayAddLast(soln->free_meet_bounds, mb);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)      */
/*                                                                           */
/*  Return the number of soln resources in soln corresponding to er.         */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnTaskCount(ers);
}


/*****************************************************************************/
/*                                                                           */
/* KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er, int i)*/
/*                                                                           */
/*  Return the i'th solution resource corresponding to er in soln.           */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er, int i)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
  return KheEventResourceInSolnTask(ers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAssignPreassignedResources(KHE_SOLN soln,                    */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Assign preassigned resources to those tasks that are not already         */
/*  assigned and have them.  If rt != NULL, this applies only to tasks       */
/*  of resource type rt.                                                     */
/*                                                                           */
/*  If as_in_event_resource is true, a task is considered to have a          */
/*  preassigned resource if it is derived from an event resource with        */
/*  a preassigned resource.  If as_in_event_resource is false, a task        */
/*  is considered to have a preassigned resource if its domain contains      */
/*  exactly one element.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnAssignPreassignedResources(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetAssignPreassignedResources(meet, rt);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task bounds"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb)               */
/*                                                                           */
/*  Add tb to soln.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb)
{
  KheTaskBoundSetSoln(tb, soln);
  KheTaskBoundSetSolnIndex(tb, MArraySize(soln->task_bounds));
  MArrayAddLast(soln->task_bounds, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb)            */
/*                                                                           */
/*  Delete tb from soln.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb)
{
  KHE_TASK_BOUND tmp;  int soln_index;
  tmp = MArrayRemoveLast(soln->task_bounds);
  if( tmp != tb )
  {
    soln_index = KheTaskBoundSolnIndex(tb);
    KheTaskBoundSetSolnIndex(tmp, soln_index);
    MArrayPut(soln->task_bounds, soln_index, tmp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_BOUND KheSolnGetTaskBoundFromFreeList(KHE_SOLN soln)            */
/*                                                                           */
/*  Return a task bound object from soln's free list, or NULL if none.       */
/*                                                                           */
/*****************************************************************************/

KHE_TASK_BOUND KheSolnGetTaskBoundFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_task_bounds) > 0 )
    return MArrayRemoveLast(soln->free_task_bounds);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskBoundToFreeList(KHE_SOLN soln, KHE_TASK_BOUND tb)     */
/*                                                                           */
/*  Add tb to soln's free list.                                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskBoundToFreeList(KHE_SOLN soln, KHE_TASK_BOUND tb)
{
  MArrayAddLast(soln->free_task_bounds, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "nodes"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheSolnGetNodeFromFreeList(KHE_SOLN soln)                       */
/*                                                                           */
/*  Return a new node from soln's free list, or NULL if none.                */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheSolnGetNodeFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_nodes) > 0 )
    return MArrayRemoveLast(soln->free_nodes);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddNodeToFreeList(KHE_SOLN soln, KHE_NODE node)              */
/*                                                                           */
/*  Add node to soln's free list of nodes.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddNodeToFreeList(KHE_SOLN soln, KHE_NODE node)
{
  MArrayAddLast(soln->free_nodes, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node)                        */
/*                                                                           */
/*  Add node to soln, and set its soln and soln_index fields.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node)                       
{
  KheNodeSetSoln(node, soln);
  KheNodeSetSolnIndex(node, MArraySize(soln->nodes));
  MArrayAddLast(soln->nodes, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node)                     */
/*                                                                           */
/*  Delete node from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node)
{
  KHE_NODE tmp;

  /* remove from nodes */
  tmp = MArrayRemoveAndPlug(soln->nodes, KheNodeSolnIndex(node));
  KheNodeSetSolnIndex(tmp, KheNodeSolnIndex(node));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnNodeCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of nodes in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnNodeCount(KHE_SOLN soln)
{
  return MArraySize(soln->nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheSolnNode(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th solution node of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheSolnNode(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layers"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheSolnGetLayerFromFreeList(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return a new layer from soln's free list, or NULL if none.               */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheSolnGetLayerFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_layers) > 0 )
    return MArrayRemoveLast(soln->free_layers);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddLayerToFreeList(KHE_SOLN soln, KHE_LAYER layer)           */
/*                                                                           */
/*  Add layer to soln's free list of layers.                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddLayerToFreeList(KHE_SOLN soln, KHE_LAYER layer)
{
  MArrayAddLast(soln->free_layers, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "zones"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheSolnGetZoneFromFreeList(KHE_SOLN soln)                       */
/*                                                                           */
/*  Return a new zone from soln's free list, or NULL if none.                */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheSolnGetZoneFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_zones) > 0 )
    return MArrayRemoveLast(soln->free_zones);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddZoneToFreeList(KHE_SOLN soln, KHE_ZONE zone)              */
/*                                                                           */
/*  Add zone to soln's free list of zones.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddZoneToFreeList(KHE_SOLN soln, KHE_ZONE zone)
{
  MArrayAddLast(soln->free_zones, zone);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln)                       */
/*                                                                           */
/*  Return a new task from soln's free list, or NULL if none.                */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_tasks) > 0 )
    return MArrayRemoveLast(soln->free_tasks);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task)              */
/*                                                                           */
/*  Add task to soln's free list of tasks.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task)
{
  MArrayAddLast(soln->free_tasks, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task)                        */
/*                                                                           */
/*  Add task to soln, including setting its soln_index.                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task)
{
  KheTaskSetSolnIndex(task, MArraySize(soln->tasks));
  MArrayAddLast(soln->tasks, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task)                     */
/*                                                                           */
/*  Delete task from soln.  Yes, this code works even when task is at the    */
/*  end of the array, and even when it is the last element of the array.     */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task)
{
  KHE_TASK tmp;
  tmp = MArrayRemoveAndPlug(soln->tasks, KheTaskSolnIndex(task));
  KheTaskSetSolnIndex(tmp, KheTaskSolnIndex(task));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTaskCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of tasks in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnTaskCount(KHE_SOLN soln)
{
  return MArraySize(soln->tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheSolnTask(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th task of soln.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheSolnTask(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "taskings"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln)                 */
/*                                                                           */
/*  Return a new tasking from soln's free list, or NULL if none.             */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_taskings) > 0 )
    return MArrayRemoveLast(soln->free_taskings);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking)     */
/*                                                                           */
/*  Add tasking to soln's free list of taskings.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking)
{
  MArrayAddLast(soln->free_taskings, tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking,               */
/*    int *index_in_soln)                                                    */
/*                                                                           */
/*  Add tasking to soln, returning its index.                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking)
{
  KheTaskingSetSolnIndex(tasking, MArraySize(soln->taskings));
  MArrayAddLast(soln->taskings, tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking)            */
/*                                                                           */
/*  Delete tasking from soln.                                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking)
{
  KHE_TASKING tmp;
  tmp = MArrayRemoveAndPlug(soln->taskings, KheTaskingSolnIndex(tasking));
  KheTaskingSetSolnIndex(tmp, KheTaskingSolnIndex(tasking));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTaskingCount(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return the number of taskings in soln.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSolnTaskingCount(KHE_SOLN soln)
{
  return MArraySize(soln->taskings);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i)                         */
/*                                                                           */
/*  Return the i'th tasking of soln.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->taskings, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution time groups and neighbourhoods"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupBegin(KHE_SOLN soln)                                */
/*                                                                           */
/*  Begin the construction of a soln time group.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupBegin(KHE_SOLN soln)
{
  MAssert(!soln->time_lset_building,
    "KheSolnTimeGroupBegin: time group already under construction");
  LSetClear(soln->time_lset);
  soln->time_lset_building = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME time)               */
/*                                                                           */
/*  Add a time to the time group currently being constructed.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME time)
{
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupAddTime: time group not under construction");
  LSetInsert(&soln->time_lset, KheTimeIndex(time));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME time)               */
/*                                                                           */
/*  Take away time from the time group currently being constructed.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME time)
{
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupSubTime: time group not under construction");
  LSetDelete(soln->time_lset, KheTimeIndex(time));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2)            */
/*                                                                           */
/*  Union a time group to the time group currently being constructed.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupUnionTimeGroup: time group not under construction");
  LSetUnion(&soln->time_lset, KheTimeGroupTimeSet(tg2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupIntersect(KHE_SOLN soln,KHE_TIME_GROUP tg2)         */
/*                                                                           */
/*  Intersect a time group to the time group currently being constructed.    */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupIntersect(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupIntersect: time group not under construction");
  LSetIntersection(soln->time_lset, KheTimeGroupTimeSet(tg2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupDifference(KHE_SOLN soln, KHE_TIME_GROUP tg2)       */
/*                                                                           */
/*  Take away tg2 from the time group currently being constructed.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupDifference(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupDifference: time group not under construction");
  LSetDifference(soln->time_lset, KheTimeGroupTimeSet(tg2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTimeGroup(KHE_SOLN soln, KHE_TIME_GROUP tg)               */
/*                                                                           */
/*  Add tg to soln.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTimeGroup(KHE_SOLN soln, KHE_TIME_GROUP tg)
{
  MArrayAddLast(soln->time_groups, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln)                        */
/*                                                                           */
/*  End the construction of a time group and return it.                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln)
{
  KHE_TIME_GROUP res;  LSET final_lset;
  MAssert(soln->time_lset_building,
    "KheSolnTimeGroupEnd: time group not under construction");
  soln->time_lset_building = false;
  if( LSetTableRetrieve(soln->time_lset_table, soln->time_lset, (void **)&res) )
    return res;
  final_lset = LSetCopy(soln->time_lset);
  res = KheTimeGroupMakeInternal(soln->instance, KHE_TIME_GROUP_TYPE_SOLN,
    soln, KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, final_lset);
  KheTimeGroupFinalize(res, soln, NULL, -1);
  LSetTableInsert(soln->time_lset_table, final_lset, (void *) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTimeNHood(KHE_SOLN soln, KHE_TIME_GROUP_NHOOD nhood)      */
/*                                                                           */
/*  Add nhood to soln.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTimeNHood(KHE_SOLN soln, KHE_TIME_GROUP_NHOOD nhood)
{
  MArrayAddLast(soln->time_nhoods, nhood);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution resource groups"                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,      */
/*    void *impl)                                                            */
/*                                                                           */
/*  Begin the construction of a soln resource group.                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  if( DEBUG10 )
    fprintf(stderr, "[ KheSolnResourceGroupBegin(soln, rt)\n");
  MAssert(soln->resource_lset_rt == NULL,
    "KheSolnResourceGroupBegin: resource group already under construction");
  MAssert(rt != NULL, "KheSolnResourceGroupBegin: rt is NULL");
  LSetClear(soln->resource_lset);
  soln->resource_lset_rt = rt;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r)      */
/*                                                                           */
/*  Add a r to the resource group currently being constructed.               */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r)
{
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupAddResource: resource group not under construction");
  MAssert(KheResourceResourceType(r) == soln->resource_lset_rt,
    "KheSolnResourceGroupAddResource: resource has wrong resource type");
  LSetInsert(&soln->resource_lset, KheResourceInstanceIndex(r));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r)      */
/*                                                                           */
/*  Take away r from the resource group currently being constructed.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r)
{
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupSubResource: resource group not under construction");
  MAssert(KheResourceResourceType(r) == soln->resource_lset_rt,
    "KheSolnResourceGroupSubResource: resource has wrong resource type");
  LSetDelete(soln->resource_lset, KheResourceInstanceIndex(r));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)    */
/*                                                                           */
/*  Add a resource group to the resource group currently being constructed.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupUnionResourceGroup: "
    "resource group not under construction");
  MAssert(KheResourceGroupResourceType(rg2) == soln->resource_lset_rt,
    "KheSolnResourceGroupUnion: resource group has wrong resource type");
  LSetUnion(&soln->resource_lset, KheResourceGroupResourceSet(rg2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupIntersect(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Intersect a resource group to the resource group currently being         */
/*  constructed.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupIntersect(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupIntersect: resource group not under construction");
  MAssert(KheResourceGroupResourceType(rg2) == soln->resource_lset_rt,
    "KheSolnResourceGroupIntersect: resource group has wrong resource type");
  LSetIntersection(soln->resource_lset, KheResourceGroupResourceSet(rg2));
}


/*****************************************************************************/
/*                                                                           */
/* void KheSolnResourceGroupDifference(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Take away rg2 from the resource group currently being constructed.       */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupDifference(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupDifference: resource group not under construction");
  MAssert(KheResourceGroupResourceType(rg2) == soln->resource_lset_rt,
    "KheSolnResourceGroupDifference: resource group has wrong resource type");
  LSetDifference(soln->resource_lset, KheResourceGroupResourceSet(rg2));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddResourceGroup(KHE_SOLN soln, KHE_RESOURCE_GROUP rg)       */
/*                                                                           */
/*  Add rg to soln.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddResourceGroup(KHE_SOLN soln, KHE_RESOURCE_GROUP rg)
{
  MArrayAddLast(soln->resource_groups, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln)                */
/*                                                                           */
/*  End the construction of a resource group and return it.                  */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln)
{
  KHE_RESOURCE_GROUP res;  LSET final_lset;  KHE_RESOURCE_TYPE rt;
  MAssert(soln->resource_lset_rt != NULL,
    "KheSolnResourceGroupEnd: resource group not under construction");
  rt = soln->resource_lset_rt;
  soln->resource_lset_rt = NULL;
  if( LSetTableRetrieve(soln->resource_lset_table, soln->resource_lset,
	(void **) &res) )
    return res;
  final_lset = LSetCopy(soln->resource_lset);
  res = KheResourceGroupMakeInternal(rt, KHE_RESOURCE_GROUP_TYPE_SOLN,
    soln, NULL, NULL, final_lset);
  KheResourceGroupSetResourcesArrayInternal(res);
  LSetTableInsert(soln->resource_lset_table, final_lset, (void *) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution event groups"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupBegin(KHE_SOLN soln)                               */
/*                                                                           */
/*  Begin the construction of a soln event group.                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupBegin(KHE_SOLN soln)
{
  MAssert(soln->curr_event_group == NULL,
    "KheSolnEventGroupBegin: event group already under construction");
  soln->curr_event_group = KheEventGroupMakeInternal(soln->instance,
    KHE_EVENT_GROUP_TYPE_SOLN, soln, KHE_EVENT_GROUP_KIND_ORDINARY, NULL, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e)               */
/*                                                                           */
/*  Add a r to the event group currently being constructed.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupAddEvent: no event group under construction");
  KheEventGroupUnionInternal(soln->curr_event_group,
    KheEventSingletonEventGroup(e));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e)               */
/*                                                                           */
/*  Take away r from the event group currently being constructed.            */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupSubEvent: no event group under construction");
  KheEventGroupDifferenceInternal(soln->curr_event_group,
    KheEventSingletonEventGroup(e));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2)          */
/*                                                                           */
/*  Add a event group to the event group currently being constructed.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupUnion: no event group under construction");
  KheEventGroupUnionInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2)      */
/*                                                                           */
/*  Intersect a event group to the event group currently being constructed.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupIntersect: no event group under construction");
  KheEventGroupIntersectInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2)     */
/*                                                                           */
/*  Take away eg2 from the event group currently being constructed.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
   "KheSolnEventGroupDifference: no event group under construction");
  KheEventGroupDifferenceInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddEventGroup(KHE_SOLN soln, KHE_EVENT_GROUP eg)             */
/*                                                                           */
/*  Add eg to soln.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddEventGroup(KHE_SOLN soln, KHE_EVENT_GROUP eg)
{
  MArrayAddLast(soln->event_groups, eg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln)                      */
/*                                                                           */
/*  End the construction of a event group and return it.                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln)
{
  KHE_EVENT_GROUP res;
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupSubEvent: no event group under construction");
  res = soln->curr_event_group;
  KheEventGroupSetEventsArrayInternal(res);
  soln->curr_event_group = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitor links"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR_LINK KheSolnGetMonitorLinkFromFreeList(KHE_SOLN soln)        */
/*                                                                           */
/*  Return a new monitor link object from soln's free list, or NULL if none. */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR_LINK KheSolnGetMonitorLinkFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_monitor_links) > 0 )
    return MArrayRemoveLast(soln->free_monitor_links);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMonitorLinkToFreeList(KHE_SOLN soln, KHE_MONITOR_LINK ml) */
/*                                                                           */
/*  Add t to soln's free list of monitor links.                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMonitorLinkToFreeList(KHE_SOLN soln, KHE_MONITOR_LINK ml)
{
  MArrayAddLast(soln->free_monitor_links, ml);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "traces"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return a new trace from soln's free list, or NULL if none.               */
/*                                                                           */
/*****************************************************************************/

KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_traces) > 0 )
    return MArrayRemoveLast(soln->free_traces);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t)               */
/*                                                                           */
/*  Add t to soln's free list of traces.                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t)
{
  MArrayAddLast(soln->free_traces, t);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "marks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMarkToFreeList(KHE_SOLN soln, KHE_MARK mark)              */
/*                                                                           */
/*  Add mark to the free list.                                               */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMarkToFreeList(KHE_SOLN soln, KHE_MARK mark)
{
  MArrayAddLast(soln->free_marks, mark);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MARK KheSolnTakeMarkFromFreeList(KHE_SOLN soln)                      */
/*                                                                           */
/*  Take a mark from the free list, or return NULL if the list is empty.     */
/*                                                                           */
/*****************************************************************************/

KHE_MARK KheSolnTakeMarkFromFreeList(KHE_SOLN soln)
{
  return MArraySize(soln->free_marks) == 0 ? NULL :
    MArrayRemoveLast(soln->free_marks);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMarkBegin(KHE_SOLN soln, KHE_MARK mark,                      */
/*    int *index, int *start_pos)                                            */
/*                                                                           */
/*  This function informs soln that mark is beginning.  It adds mark to      */
/*  the end of its mark stack, and sets *index and *start_pos to the         */
/*  correct values for the new mark.                                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMarkBegin(KHE_SOLN soln, KHE_MARK mark,
  int *index, int *start_pos)
{
  /* set *index and *start_pos */
  *index = MArraySize(soln->marks);
  *start_pos = KhePathCount(soln->main_path);

  /* add mark to the mark stack */
  MArrayAddLast(soln->marks, mark);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMarkEnd(KHE_SOLN soln, KHE_MARK mark)                        */
/*                                                                           */
/*  This function informs soln that a mark is ending.  It checks that this   */
/*  mark is on top of the stack and removes it from the mark stack.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnMarkEnd(KHE_SOLN soln, KHE_MARK mark)
{
  KHE_MARK mark2;  int pos;
  MAssert(MArraySize(soln->marks) > 0, "KheMarkEnd: mark already ended");
  mark2 = MArrayRemoveLast(soln->marks);
  MAssert(mark2 == mark, MArrayContains(soln->marks, mark, &pos) ?
    "KheMarkEnd: wrong mark has ended" : "KheMarkEnd: mark already ended");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMarkOnTop(KHE_SOLN soln, KHE_MARK mark)                      */
/*                                                                           */
/*  Return true if mark is on top of soln stack.                             */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMarkOnTop(KHE_SOLN soln, KHE_MARK mark)
{
  return MArraySize(soln->marks) > 0 && MArrayLast(soln->marks) == mark;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "paths"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PATH KheSolnMainPath(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Return the main path.                                                    */
/*                                                                           */
/*****************************************************************************/

KHE_PATH KheSolnMainPath(KHE_SOLN soln)
{
  return soln->main_path;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddPathToFreeList(KHE_SOLN soln, KHE_PATH path)              */
/*                                                                           */
/*  Add path to soln's free list.                                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddPathToFreeList(KHE_SOLN soln, KHE_PATH path)
{
  MArrayAddLast(soln->free_paths, path);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PATH KheSolnTakePathFromFreeList(KHE_SOLN soln)                      */
/*                                                                           */
/*  Return a path from soln's free list, or NULL if none.                    */
/*                                                                           */
/*****************************************************************************/

KHE_PATH KheSolnTakePathFromFreeList(KHE_SOLN soln)
{
  return MArraySize(soln->free_paths) == 0 ? NULL :
    MArrayRemoveLast(soln->free_paths);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - meets"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetSetBack(KHE_SOLN soln, KHE_MEET meet,                  */
/*    void *old_back, void *new_back)                                        */
/*                                                                           */
/*  Inform soln that a call to KheMeetSetBack has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetSetBack(KHE_SOLN soln, KHE_MEET meet,
  void *old_back, void *new_back)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetSetBack(soln->main_path, meet, old_back, new_back);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetAdd(KHE_SOLN soln, KHE_MEET meet, int duration,        */
/*    KHE_EVENT e)                                                           */
/*                                                                           */
/*  Inform soln that a call to KheMeetKernelAdd has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetAdd(KHE_SOLN soln, KHE_MEET meet, int duration,
  KHE_EVENT e)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetAdd(soln->main_path, meet, duration, e);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetDelete(KHE_SOLN soln, KHE_MEET meet, int duration,     */
/*    KHE_EVENT e)                                                           */
/*                                                                           */
/*  Inform soln that a call to KheMeetKernelDelete has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetDelete(KHE_SOLN soln, KHE_MEET meet, int duration,
  KHE_EVENT e)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetDelete(soln->main_path, meet, duration, e);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,   */
/*    int durn1)                                                             */
/*                                                                           */
/*  Inform soln that a call to KheMeetSplit has occurred.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,
  int durn1)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetSplit(soln->main_path, meet1, meet2, durn1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,   */
/*    int durn1)                                                             */
/*                                                                           */
/*  Inform soln that a call to KheMeetMerge has occurred.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,
  int durn1)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetMerge(soln->main_path, meet1, meet2, durn1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetMove(KHE_SOLN soln, KHE_MEET meet,                     */
/*    KHE_MEET old_target_meet, int old_target_offset,                       */
/*    KHE_MEET new_target_meet, int new_target_offset)                       */
/*                                                                           */
/*  Inform soln that a call to KheMeetMove has occurred.                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetMove(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET old_target_meet, int old_target_offset,
  KHE_MEET new_target_meet, int new_target_offset)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetMove(soln->main_path, meet, old_target_meet,
      old_target_offset, new_target_meet, new_target_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetSetAutoDomain(KHE_SOLN soln, KHE_MEET meet,            */
/*    bool automatic)                                                        */
/*                                                                           */
/*  Inform soln that a call to KheMeetSetAutoDomain has occurred.            */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetSetAutoDomain(KHE_SOLN soln, KHE_MEET meet, bool automatic)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetSetAutoDomain(soln->main_path, meet, automatic);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetAssignFix(KHE_SOLN soln, KHE_MEET meet)                */
/*                                                                           */
/*  Inform soln that a call to KheMeetAssignFix has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetAssignFix(KHE_SOLN soln, KHE_MEET meet)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetAssignFix(soln->main_path, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetAssignUnFix(KHE_SOLN soln, KHE_MEET meet)              */
/*                                                                           */
/*  Inform soln that a call to KheMeetAssignUnFix has occurred.              */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetAssignUnFix(KHE_SOLN soln, KHE_MEET meet)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetAssignUnFix(soln->main_path, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetAddMeetBound(KHE_SOLN soln,                            */
/*    KHE_MEET meet, KHE_MEET_BOUND mb)                                      */
/*                                                                           */
/*  Inform soln that a call to KheMeetAddMeetBound has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetAddMeetBound(KHE_SOLN soln,
  KHE_MEET meet, KHE_MEET_BOUND mb)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetAddMeetBound(soln->main_path, meet, mb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetDeleteMeetBound(KHE_SOLN soln,                         */
/*    KHE_MEET meet, KHE_MEET_BOUND mb)                                      */
/*                                                                           */
/*  Inform soln that a call to KheMeetDeleteMeetBound has occurred.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetDeleteMeetBound(KHE_SOLN soln,
  KHE_MEET meet, KHE_MEET_BOUND mb)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetDeleteMeetBound(soln->main_path, meet, mb);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - meet bounds"                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetBoundAdd(KHE_SOLN soln, KHE_MEET_BOUND mb,             */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Inform soln that a call to KheMeetBoundMake has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetBoundAdd(KHE_SOLN soln, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetBoundAdd(soln->main_path, mb, occupancy, dft_tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetBoundDelete(KHE_SOLN soln, KHE_MEET_BOUND mb,          */
/*    bool occupancy, KHE_TIME_GROUP dft_tg)                                 */
/*                                                                           */
/*  Inform soln that a call to KheMeetBoundDelete has occurred.              */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetBoundDelete(KHE_SOLN soln, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetBoundDelete(soln->main_path, mb, occupancy, dft_tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetBoundAddTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb,    */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Inform soln that a call to KheMeetBoundAddTimeGroup has occurred.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetBoundAddTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetBoundAddTimeGroup(soln->main_path, mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetBoundDeleteTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb, */
/*    int duration, KHE_TIME_GROUP tg)                                       */
/*                                                                           */
/*  Inform soln that a call to KheMeetBoundDeleteTimeGroup has occurred.     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetBoundDeleteTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpMeetBoundDeleteTimeGroup(soln->main_path, mb, duration, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - tasks"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskSetBack(KHE_SOLN soln, KHE_TASK task,                  */
/*    void *old_back, void *new_back)                                        */
/*                                                                           */
/*  Inform soln that a call to KheTaskSetBack has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskSetBack(KHE_SOLN soln, KHE_TASK task,
  void *old_back, void *new_back)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskSetBack(soln->main_path, task, old_back, new_back);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskAdd(KHE_SOLN soln, KHE_TASK task,                      */
/*    KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)            */
/*                                                                           */
/*  Inform soln that a call to KheTaskAdd has occurred.                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskAdd(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskAdd(soln->main_path, task, rt, meet, er);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskDelete(KHE_SOLN soln, KHE_TASK task,                   */
/*    KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)            */
/*                                                                           */
/*  Inform soln that a call to KheTaskDelete has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskDelete(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskDelete(soln->main_path, task, rt, meet, er);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskSplit(KHE_SOLN soln, KHE_TASK task1,                   */
/*    KHE_TASK task2, int durn1, KHE_MEET meet2)                             */
/*                                                                           */
/*  Inform soln that a task split with these attributes is occurring.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskSplit(KHE_SOLN soln, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskSplit(soln->main_path, task1, task2, durn1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskMerge(KHE_SOLN soln, KHE_TASK task1,                   */
/*    KHE_TASK task2, int durn1, KHE_MEET meet2)                             */
/*                                                                           */
/*  Inform soln that a task merge with these attributes is occurring.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskMerge(KHE_SOLN soln, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskMerge(soln->main_path, task1, task2, durn1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskMove(KHE_SOLN soln, KHE_TASK task,                     */
/*    KHE_TASK old_target_task, KHE_TASK new_target_task)                    */
/*                                                                           */
/*  Inform soln that a call to KheTaskMove has occurred.                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskMove(KHE_SOLN soln, KHE_TASK task,
  KHE_TASK old_target_task, KHE_TASK new_target_task)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskMove(soln->main_path, task, old_target_task, new_target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskAssignFix(KHE_SOLN soln, KHE_TASK task)                */
/*                                                                           */
/*  Inform soln that a call to KheTaskAssignFix has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskAssignFix(KHE_SOLN soln, KHE_TASK task)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskAssignFix(soln->main_path, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskAssignUnFix(KHE_SOLN soln, KHE_TASK task)              */
/*                                                                           */
/*  Inform soln that a call to KheTaskAssignUnFix has occurred.              */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskAssignUnFix(KHE_SOLN soln, KHE_TASK task)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskAssignUnFix(soln->main_path, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskAddTaskBound(KHE_SOLN soln,                            */
/*    KHE_TASK task, KHE_TASK_BOUND tb)                                      */
/*                                                                           */
/*  Inform soln that a call to KheTaskAddTaskBound has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskAddTaskBound(KHE_SOLN soln,
  KHE_TASK task, KHE_TASK_BOUND tb)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskAddTaskBound(soln->main_path, task, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskDeleteTaskBound(KHE_SOLN soln,                         */
/*    KHE_TASK task, KHE_TASK_BOUND tb)                                      */
/*                                                                           */
/*  Inform soln that a call to KheTaskDeleteTaskBound has occurred.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskDeleteTaskBound(KHE_SOLN soln,
  KHE_TASK task, KHE_TASK_BOUND tb)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskDeleteTaskBound(soln->main_path, task, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - task bounds"                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskBoundAdd(KHE_SOLN soln, KHE_TASK_BOUND tb,             */
/*    KHE_TASK_BOUND_GROUP tbg, KHE_TASK task, KHE_RESOURCE_GROUP rg)        */
/*                                                                           */
/*  Inform soln that a call to KheTaskBoundAdd has occurred.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskBoundAdd(KHE_SOLN soln, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskBoundAdd(soln->main_path, tb, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskBoundDelete(KHE_SOLN soln, KHE_TASK_BOUND tb,          */
/*    KHE_TASK_BOUND_GROUP tbg, KHE_TASK task, KHE_RESOURCE_GROUP rg)        */
/*                                                                           */
/*  Inform soln that a call to KheTaskBoundDelete has occurred.              */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskBoundDelete(KHE_SOLN soln, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpTaskBoundDelete(soln->main_path, tb, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - nodes"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeSetBack(KHE_SOLN soln, KHE_NODE node,                  */
/*    void *old_back, void *new_back)                                        */
/*                                                                           */
/*  Inform soln that a call to KheNodeSetBack has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeSetBack(KHE_SOLN soln, KHE_NODE node,
  void *old_back, void *new_back)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeSetBack(soln->main_path, node, old_back, new_back);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeAdd(KHE_SOLN soln, KHE_NODE node)                      */
/*                                                                           */
/*  Inform soln that a call to KheNodeAdd has occurred.                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeAdd(KHE_SOLN soln, KHE_NODE node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeAdd(soln->main_path, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeDelete(KHE_SOLN soln, KHE_NODE node)                   */
/*                                                                           */
/*  Inform soln that a call to KheNodeDelete has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeDelete(KHE_SOLN soln, KHE_NODE node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeDelete(soln->main_path, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeAddParent(KHE_SOLN soln,                               */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Inform soln that a call to KheNodeAddParent has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeAddParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeAddParent(soln->main_path, child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeDeleteParent(KHE_SOLN soln,                            */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Inform soln that a call to KheNodeDeleteParent has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeDeleteParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeDeleteParent(soln->main_path, child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeSwapChildNodesAndLayers(KHE_SOLN soln,                 */
/*    KHE_NODE node1, KHE_NODE node2)                                        */
/*                                                                           */
/*  Inform soln that a call to KheNodeSwapChildNodesAndLayers has occurred.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeSwapChildNodesAndLayers(KHE_SOLN soln,
  KHE_NODE node1, KHE_NODE node2)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeSwapChildNodesAndLayers(soln->main_path, node1, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeAddMeet(KHE_SOLN soln, KHE_NODE node, KHE_MEET meet)   */
/*                                                                           */
/*  Inform soln that a call to KheNodeAddMeet has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeAddMeet(KHE_SOLN soln, KHE_NODE node, KHE_MEET meet)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeAddMeet(soln->main_path, node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeDeleteMeet(KHE_SOLN soln, KHE_NODE node, KHE_MEET meet)*/
/*                                                                           */
/*  Inform soln that a call to KheNodeDeleteMeet has occurred.               */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeDeleteMeet(KHE_SOLN soln, KHE_NODE node, KHE_MEET meet)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpNodeDeleteMeet(soln->main_path, node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - layers"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerSetBack(KHE_SOLN soln, KHE_LAYER layer,               */
/*    void *old_back, void *new_back)                                        */
/*                                                                           */
/*  Inform soln that a call to KheLayerSetBack has occurred.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerSetBack(KHE_SOLN soln, KHE_LAYER layer,
  void *old_back, void *new_back)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerSetBack(soln->main_path, layer, old_back, new_back);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerAdd(KHE_SOLN soln, KHE_LAYER layer,                   */
/*    KHE_NODE parent_node)                                                  */
/*                                                                           */
/*  Inform soln that a call to KheLayerMake has occurred.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerAdd(KHE_SOLN soln, KHE_LAYER layer, KHE_NODE parent_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerAdd(soln->main_path, layer, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerDelete(KHE_SOLN soln, KHE_LAYER layer,                */
/*    KHE_NODE parent_node)                                                  */
/*                                                                           */
/*  Inform soln that a call to KheLayerDelete has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerDelete(KHE_SOLN soln, KHE_LAYER layer, KHE_NODE parent_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerDelete(soln->main_path, layer, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerAddChildNode(KHE_SOLN soln, KHE_LAYER layer,          */
/*    KHE_NODE child_node)                                                   */
/*                                                                           */
/*  Inform soln that a call to KheLayerAddChildNode has occurred.            */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerAddChildNode(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE child_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerAddChildNode(soln->main_path, layer, child_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerDeleteChildNode(KHE_SOLN soln, KHE_LAYER layer,       */
/*    KHE_NODE child_node)                                                   */
/*                                                                           */
/*  Inform soln that a call to KheLayerDeleteChildNode has occurred.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerDeleteChildNode(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE child_node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerDeleteChildNode(soln->main_path, layer, child_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerAddResource(KHE_SOLN soln, KHE_LAYER layer,           */
/*    KHE_RESOURCE resource)                                                 */
/*                                                                           */
/*  Inform soln that a call to KheLayerAddResource has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerAddResource(KHE_SOLN soln, KHE_LAYER layer,
  KHE_RESOURCE resource)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerAddResource(soln->main_path, layer, resource);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpLayerDeleteResource(KHE_SOLN soln, KHE_LAYER layer,        */
/*    KHE_RESOURCE resource)                                                 */
/*                                                                           */
/*  Inform soln that a call to KheLayerDeleteResource has occurred.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpLayerDeleteResource(KHE_SOLN soln, KHE_LAYER layer,
  KHE_RESOURCE resource)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpLayerDeleteResource(soln->main_path, layer, resource);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "path operation loading - zones"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpZoneSetBack(KHE_SOLN soln, KHE_ZONE zone,                  */
/*    void *old_back, void *new_back)                                        */
/*                                                                           */
/*  Inform soln that a call to KheZoneSetBack has occurred.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpZoneSetBack(KHE_SOLN soln, KHE_ZONE zone,
  void *old_back, void *new_back)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpZoneSetBack(soln->main_path, zone, old_back, new_back);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpZoneAdd(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node)       */
/*                                                                           */
/*  Inform soln that a call to KheZoneMake has occurred.                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpZoneAdd(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpZoneAdd(soln->main_path, zone, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpZoneDelete(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node)    */
/*                                                                           */
/*  Inform soln that a call to KheZoneDelete has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpZoneDelete(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpZoneDelete(soln->main_path, zone, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpZoneAddMeetOffset(KHE_SOLN soln,                           */
/*    KHE_ZONE zone, KHE_MEET meet, int offset)                              */
/*                                                                           */
/*  Inform soln that a call to KheZoneAddMeetOffset has occurred.            */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpZoneAddMeetOffset(KHE_SOLN soln,
  KHE_ZONE zone, KHE_MEET meet, int offset)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpZoneAddMeetOffset(soln->main_path, zone, meet, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpZoneDeleteMeetOffset(KHE_SOLN soln,                        */
/*    KHE_ZONE zone, KHE_MEET meet, int offset)                              */
/*                                                                           */
/*  Inform soln that a call to KheZoneDeleteMeetOffset has occurred.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpZoneDeleteMeetOffset(KHE_SOLN soln,
  KHE_ZONE zone, KHE_MEET meet, int offset)
{
  if( MArraySize(soln->marks) > 0 )
    KhePathOpZoneDeleteMeetOffset(soln->main_path, zone, meet, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "diversifiers"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetDiversifier(KHE_SOLN soln, int val)                       */
/*                                                                           */
/*  Set the diversifier of soln to val.                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetDiversifier(KHE_SOLN soln, int val)
{
  soln->diversifier = val;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDiversifier(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the diversifier of soln.                                          */
/*                                                                           */
/*****************************************************************************/

int KheSolnDiversifier(KHE_SOLN soln)
{
  return soln->diversifier;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDiversifierChoose(KHE_SOLN soln, int c)                       */
/*                                                                           */
/*  Choose an integer i in the range 0 <= i < c using soln's diversifier.    */
/*                                                                           */
/*****************************************************************************/

int KheSolnDiversifierChoose(KHE_SOLN soln, int c)
{
  int i, c1f;
  c1f = 1;
  for( i = 1;  i < c && c1f <= soln->diversifier;  i++ )
    c1f *= i;
  return ((soln->diversifier / c1f) + (soln->diversifier % c1f)) % c;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "global visit number"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetGlobalVisitNum(KHE_SOLN soln, int num)                    */
/*                                                                           */
/*  Set soln's visit number to num.                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetGlobalVisitNum(KHE_SOLN soln, int num)
{
  soln->global_visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnGlobalVisitNum(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Return soln's visit number.                                              */
/*                                                                           */
/*****************************************************************************/

int KheSolnGlobalVisitNum(KHE_SOLN soln)
{
  return soln->global_visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnNewGlobalVisit(KHE_SOLN soln)                                */
/*                                                                           */
/*  Increment soln's visit number.                                           */
/*                                                                           */
/*****************************************************************************/

void KheSolnNewGlobalVisit(KHE_SOLN soln)
{
  soln->global_visit_num++;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "soft time limits"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  float KheSolnTimeNow(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the wall clock time since soln was created.                       */
/*                                                                           */
/*****************************************************************************/

float KheSolnTimeNow(KHE_SOLN soln)
{
  return KheStatsTimerNow(soln->timer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetTimeLimit(KHE_SOLN soln, float limit_in_secs)             */
/*                                                                           */
/*  Set the time limit to limit_in_secs.                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetTimeLimit(KHE_SOLN soln, float limit_in_secs)
{
  soln->time_limit = limit_in_secs;
}


/*****************************************************************************/
/*                                                                           */
/*  float KheSolnTimeLimit(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the time limit, or -1.0 if none.                                  */
/*                                                                           */
/*****************************************************************************/

float KheSolnTimeLimit(KHE_SOLN soln)
{
  return soln->time_limit;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnTimeLimitReached(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return true if there is a time limit and it has been reached.            */
/*                                                                           */
/*****************************************************************************/

bool KheSolnTimeLimitReached(KHE_SOLN soln)
{
  float now;
  if( soln->time_limit == -1 )
  {
    if( DEBUG11 )
      fprintf(stderr, "KheSolnTimeLimitReached = false (no time limit)\n");
    return false;
  }
  now = KheSolnTimeNow(soln);
  if( now == -1 )
  {
    if( DEBUG11 )
      fprintf(stderr, "KheSolnTimeLimitReached = false (time not known)\n");
    return false;
  }
  if( DEBUG11 )
    fprintf(stderr,
      "KheSolnTimeLimitReached = %s (now %.2f mins, limit %.2f mins)\n",
      now >= soln->time_limit ? "true" : "false", now / 60.0,
      soln->time_limit / 60.0);
  return now >= soln->time_limit;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool Error(KHE_SOLN soln, bool allow_invalid_solns, KML_ERROR *ke)       */
/*                                                                           */
/*  Helper function for KheSolnMakeFromKml below.  An error has been         */
/*  found and assigned to *ke, so it is time to return.  But, depending on   */
/*  allow_invalid_solns, we either return false immediately, or make         */
/*  soln invalid and return true.                                            */
/*                                                                           */
/*****************************************************************************/

static bool Error(KHE_SOLN soln, bool allow_invalid_solns, KML_ERROR *ke)
{
  if( allow_invalid_solns )
  {
    KheSolnReduceToInvalid(soln, *ke);
    *ke = NULL;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_ARCHIVE archive,           */
/*    bool allow_invalid_solns, KHE_SOLN *soln, KML_ERROR *ke)               */
/*                                                                           */
/*  Make a solution based on soln_elt.  Its instance must come from archive. */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_ARCHIVE archive,
  bool allow_invalid_solns, KHE_SOLN *soln, KML_ERROR *ke)
{
  KML_ELT events_elt, event_elt, descr_elt, rt_elt;  char *ref, *text;
  KHE_EVENT e;  KHE_INSTANCE ins;  int i;  float running_time;

  /* check soln_elt, find ins, and make res, the result object */
  if( DEBUG2 )
    fprintf(stderr, "[ KheSolnMakeFromKml(soln_elt)\n");
  if( !KmlCheck(soln_elt,
	"Reference : +$Description +%RunningTime +Events +Report", ke) )
    return false;
  ref = KmlAttributeValue(soln_elt, 0);
  if( !KheArchiveRetrieveInstance(archive, ref, &ins) )
    return KmlError(ke, KmlLineNum(soln_elt), KmlColNum(soln_elt),
      "<Solution> Reference \"%s\" unknown", ref);
  *soln = KheSolnMake(ins);

  /* Description */
  if( KmlContainsChild(soln_elt, "Description", &descr_elt) )
    KheSolnSetDescription(*soln, KmlExtractText(descr_elt));

  /* RunningTime */
  if( KmlContainsChild(soln_elt, "RunningTime", &rt_elt) )
  {
    text = KmlExtractText(rt_elt);
    sscanf(text, "%f", &running_time);
    KheSolnSetRunningTime(*soln, running_time);
  }

  /* Events */
  if( KmlContainsChild(soln_elt, "Events", &events_elt) )
  {
    if( !KmlCheck(events_elt, ": *Event", ke) )
      return Error(*soln, allow_invalid_solns, ke);
    for( i = 0;  i < KmlChildCount(events_elt);  i++ )
    {
      event_elt = KmlChild(events_elt, i);
      if( !KheMeetMakeFromKml(event_elt, *soln, ke) )
	return Error(*soln, allow_invalid_solns, ke);
    }
  }

  /* make sure event durations are correct, then add any missing ones */
  if( !KheSolnMakeCompleteRepresentation(*soln, &e) )
  {
    KmlError(ke, KmlLineNum(soln_elt), KmlColNum(soln_elt),
      "<Solution> invalid total duration of meets of event \"%s\"",
      KheEventId(e));
    return Error(*soln, allow_invalid_solns, ke);
  }

  /* convert preassignments into assignments */
  KheSolnAssignPreassignedTimes(*soln);
  KheSolnAssignPreassignedResources(*soln, NULL);

  if( DEBUG2 )
  {
    KheSolnDebug(*soln, 2, 2, stderr);
    fprintf(stderr, "] KheSolnMakeFromKml returning\n");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourcesReportWrite(KHE_SOLN soln, KML_FILE kf)             */
/*                                                                           */
/*  Write the Resources part of the report on soln.                          */
/*                                                                           */
/*****************************************************************************/

static void KheSolnResourcesReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started, resource_started;  KHE_CONSTRAINT c;
  KHE_INSTANCE ins;  KHE_RESOURCE r;  int i, j, cost;  KHE_MONITOR m;
  section_started = false;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    resource_started = false;
    for( j = 0;  j < KheSolnResourceMonitorCount(soln, r);  j++ )
    {
      m = KheSolnResourceMonitor(soln, r, j);
      c = KheMonitorConstraint(m);
      if( c != NULL && KheMonitorCost(m) > 0 )
      {
	if( !section_started )
	{
	  KmlBegin(kf, "Resources");
	  section_started = true;
	}
	if( !resource_started )
	{
	  KmlBegin(kf, "Resource");
	  KmlAttribute(kf, "Reference", KheResourceId(r));
	  resource_started = true;
	}
        KmlBegin(kf, "Constraint");
	KmlAttribute(kf, "Reference", KheConstraintId(c));
	cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
	  KheSoftCost(KheMonitorCost(m));
	KmlEltFmtText(kf, "Cost", "%d", cost);
	/* no Description at present */
        KmlEnd(kf, "Constraint");
      }
    }
    if( resource_started )
      KmlEnd(kf, "Resource");
  }
  if( section_started )
    KmlEnd(kf, "Resources");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorReportWithEvent(KHE_MONITOR_TAG tag)                      */
/*                                                                           */
/*  Return true if tag tags a monitor that is reported in the Events         */
/*  section of a report.                                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorReportWithEvent(KHE_MONITOR_TAG tag)
{
  return tag == KHE_ASSIGN_RESOURCE_MONITOR_TAG ||
    tag == KHE_ASSIGN_TIME_MONITOR_TAG ||
    tag == KHE_SPLIT_EVENTS_MONITOR_TAG ||
    tag == KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG ||
    tag == KHE_PREFER_RESOURCES_MONITOR_TAG ||
    tag == KHE_PREFER_TIMES_MONITOR_TAG;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheHandleEventMonitor(KHE_SOLN soln, KML_FILE kf, KHE_MONITOR m,    */
/*    KHE_EVENT e, bool *section_started, bool *event_started)               */
/*                                                                           */
/*  Handle the reporting of monitor m, if relevant with non-zero cost.       */
/*                                                                           */
/*****************************************************************************/

static void KheHandleEventMonitor(KHE_SOLN soln, KML_FILE kf, KHE_MONITOR m,
  KHE_EVENT e, bool *section_started, bool *event_started)
{
  KHE_CONSTRAINT c;  int cost;
  if( KheMonitorReportWithEvent(KheMonitorTag(m)) && KheMonitorCost(m) > 0 )
  {
    c = KheMonitorConstraint(m);
    if( !*section_started )
    {
      KmlBegin(kf, "Events");
      *section_started = true;
    }
    if( !*event_started )
    {
      KmlBegin(kf, "Event");
      KmlAttribute(kf, "Reference", KheEventId(e));
      *event_started = true;
    }
    KmlBegin(kf, "Constraint");
    KmlAttribute(kf, "Reference", KheConstraintId(c));
    cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
      KheSoftCost(KheMonitorCost(m));
    KmlEltFmtText(kf, "Cost", "%d", cost);
    /* no Description at present */
    KmlEnd(kf, "Constraint");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventsReportWrite(KHE_SOLN soln, KML_FILE kf)                    */
/*                                                                           */
/*  Write the Events part of the report on soln.                             */
/*                                                                           */
/*****************************************************************************/

static void KheEventsReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started, event_started;  KHE_EVENT_RESOURCE er;
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j, k;  KHE_MONITOR m;
  section_started = false;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    event_started = false;
    for( j = 0;  j < KheSolnEventMonitorCount(soln, e);  j++ )
    {
      m = KheSolnEventMonitor(soln, e, j);
      KheHandleEventMonitor(soln, kf, m, e, &section_started, &event_started);
    }
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      for( k = 0;  k < KheSolnEventResourceMonitorCount(soln, er);  k++ )
      {
	m = KheSolnEventResourceMonitor(soln, er, k);
	KheHandleEventMonitor(soln, kf, m, e, &section_started, &event_started);
      }
    }
    if( event_started )
      KmlEnd(kf, "Event");
  }
  if( section_started )
    KmlEnd(kf, "Events");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorReportWithEventGroup(KHE_MONITOR m, KHE_EVENT_GROUP eg)   */
/*                                                                           */
/*  Return true if m is to be reported in eg's section of the report.        */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorReportWithEventGroup(KHE_MONITOR m, KHE_EVENT_GROUP eg)
{
  KHE_SPREAD_EVENTS_MONITOR sem;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int i;
  switch( KheMonitorTag(m) )
  {
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m;
      c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
      i = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
      return KheAvoidSplitAssignmentsConstraintEventGroup(c, i) == eg;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      sem = (KHE_SPREAD_EVENTS_MONITOR) m;
      return KheSpreadEventsMonitorEventGroup(sem) == eg;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return KheLinkEventsMonitorEventGroup((KHE_LINK_EVENTS_MONITOR) m) == eg;

    default:

      return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupsReportWrite(KHE_SOLN soln, KML_FILE kf)               */
/*                                                                           */
/*  Write the EventGroups part of the report on soln to ktf.                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventGroupsReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started;  KHE_EVENT_GROUP eg;  KHE_CONSTRAINT c;
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j, k, n, pos, cost;
  ARRAY_KHE_MONITOR eg_monitors;  KHE_MONITOR m;  KHE_EVENT_RESOURCE er;
  section_started = false;
  ins = KheSolnInstance(soln);
  MArrayInit(eg_monitors);
  for( i = 0;  i < KheInstanceEventGroupCount(ins);  i++ )
  {
    eg = KheInstanceEventGroup(ins, i);

    /* find the monitors of event group eg */
    MArrayClear(eg_monitors);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      for( k = 0;  k < KheSolnEventMonitorCount(soln, e);  k++ )
      {
	m = KheSolnEventMonitor(soln, e, k);
	if( KheMonitorReportWithEventGroup(m, eg) &&
	    KheMonitorCost(m) > 0 && !MArrayContains(eg_monitors, m, &pos) )
	  MArrayAddLast(eg_monitors, m);
      }
      for( k = 0;  k < KheEventResourceCount(e);  k++ )
      {
	er = KheEventResource(e, k);
	for( n = 0;  n < KheSolnEventResourceMonitorCount(soln, er);  n++ )
	{
	  m = KheSolnEventResourceMonitor(soln, er, n);
	  if( KheMonitorReportWithEventGroup(m, eg) &&
	      KheMonitorCost(m) > 0 && !MArrayContains(eg_monitors, m, &pos) )
	    MArrayAddLast(eg_monitors, m);
	}
      }
    }

    /* print the report for event group eg, if any relevant monitors */
    if( MArraySize(eg_monitors) > 0 )
    {
      if( !section_started )
      {
	KmlBegin(kf, "EventGroups");
	section_started = true;
      }
      KmlBegin(kf, "EventGroup");
      KmlAttribute(kf, "Reference", KheEventGroupId(eg));
      MArrayForEach(eg_monitors, &m, &k)
      {
	c = KheMonitorConstraint(m);
	KmlBegin(kf, "Constraint");
	KmlAttribute(kf, "Reference", KheConstraintId(c));
	cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
	  KheSoftCost(KheMonitorCost(m));
	KmlEltFmtText(kf, "Cost", "%d", cost);
	/* no Description at present */
	KmlEnd(kf, "Constraint");
      }
      KmlEnd(kf, "EventGroup");
    }
  }
  if( section_started )
    KmlEnd(kf, "EventGroups");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf)         */
/*                                                                           */
/*  Write soln to kf, with a report if with_reports is true.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf)
{
  KHE_EVENT_IN_SOLN es;  int i;  bool event_written;
  KmlBegin(kf, "Solution");
  KmlAttribute(kf, "Reference", KheInstanceId(soln->instance));
  if( soln->description != NULL )
  {
    KmlBegin(kf, "Description");
    KmlPlainText(kf, soln->description);
    KmlEnd(kf, "Description");
  }
  if( soln->running_time >= 0.0 )
    KmlEltFmtText(kf, "RunningTime", "%.1f", soln->running_time);
    /* ***
    KmlBegin(kf, "RunningTime");
    KmlFmtText(kf, "%.1f", soln->running_time);
    KmlEnd(kf, "RunningTime");
    *** */
  event_written = false;
  MArrayForEach(soln->events_in_soln, &es, &i)
    KheEventInSolnWrite(es, kf, &event_written);
    /* ***
    if( KheEventInSolnMeetCount(es) > 0 )
    {
      if( !event_written )
      {
	KmlBegin(kf, "Events");
	event_written = true;
      }
      KheEventInSolnWrite(es, kf, &event_written);
    }
    *** */
  if( event_written )
    KmlEnd(kf, "Events");
  if( with_reports )
  {
    KmlBegin(kf, "Report");
    KmlEltFmtText(kf, "InfeasibilityValue","%d",KheHardCost(KheSolnCost(soln)));
    KmlEltFmtText(kf, "ObjectiveValue", "%d", KheSoftCost(KheSolnCost(soln)));
    KheSolnResourcesReportWrite(soln, kf);
    KheEventsReportWrite(soln, kf);
    KheEventGroupsReportWrite(soln, kf);
    KmlEnd(kf, "Report");
  }
  KmlEnd(kf, "Solution");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - zero domain"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return a domain containing just 0.                                       */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln)
{
  return soln->matching_zero_domain;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - free supply chunks"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(        */
/*    KHE_SOLN soln, KHE_MEET meet)                                          */
/*                                                                           */
/*  Return a fresh ordinary supply chunk, with one node for each resource    */
/*  of the instance, either taken from a free list or made from scratch.     */
/*  Set the impl field of the supply chunk to meet.                          */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(
  KHE_SOLN soln, KHE_MEET meet)
{
  KHE_MATCHING_SUPPLY_CHUNK res;  KHE_INSTANCE ins;  int i;
  if( MArraySize(soln->matching_free_supply_chunks) > 0 )
    res = MArrayRemoveLast(soln->matching_free_supply_chunks);
  else
  {
    ins = KheSolnInstance(soln);
    res = KheMatchingSupplyChunkMake(soln->matching, NULL);
    for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
      KheMatchingSupplyNodeMake(res, (void *) KheInstanceResource(ins, i));
  }
  KheMatchingSupplyChunkSetImpl(res, meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,      */
/*    KHE_MATCHING_SUPPLY_CHUNK sc)                                          */
/*                                                                           */
/*  Save sc for reuse later.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,
  KHE_MATCHING_SUPPLY_CHUNK sc)
{
  MArrayAddLast(soln->matching_free_supply_chunks, sc);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - setting up"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheSolnMatching(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the matching held by soln, or NULL if no matching.                */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheSolnMatching(KHE_SOLN soln)
{
  return soln->matching;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingUpdate(KHE_SOLN soln)                                */
/*                                                                           */
/*  Bring soln's matching up to date, or do nothing if there is no matching. */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingUpdate(KHE_SOLN soln)
{
  if( soln->matching != NULL )
    KheMatchingUnmatchedDemandNodeCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingBegin(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Begin matching.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingBegin(KHE_SOLN soln)
{
  KHE_MEET meet;  KHE_TASK task;  int i;
  MAssert(soln->matching == NULL,
    "KheSolnMatchingBegin: soln already has a matching");
  soln->matching = KheMatchingMake(soln);
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingBegin(meet);
  MArrayForEach(soln->tasks, &task, &i)
    KheTaskMatchingBegin(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingEnd(KHE_SOLN soln)                                   */
/*                                                                           */
/*  End matching.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingEnd(KHE_SOLN soln)
{
  KHE_MEET meet;  KHE_TASK task;  int i;
  MAssert(soln->matching != NULL, "KheSolnMatchingEnd: no matching");
  MArrayForEach(soln->tasks, &task, &i)
    KheTaskMatchingEnd(task);
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingEnd(meet);
  KheMatchingDelete(soln->matching);
  soln->matching = NULL;
  MArrayFree(soln->matching_free_supply_chunks);
  MArrayInit(soln->matching_free_supply_chunks);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnHasMatching(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return true if soln has a matching.                                      */
/*                                                                           */
/*****************************************************************************/

bool KheSolnHasMatching(KHE_SOLN soln)
{
  return soln->matching != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST matching_weight)   */
/*                                                                           */
/*  Set the weight that the matching is to have in the total cost.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST matching_weight)
{
  KHE_RESOURCE_IN_SOLN rs;  KHE_MEET meet;  int i;
  MAssert(soln->matching != NULL, "KheSolnMatchingSetWeight: no matching");
  if( matching_weight != soln->matching_weight )
  {
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnMatchingSetWeight(rs, matching_weight);
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetMatchingSetWeight(meet, matching_weight);
    soln->matching_weight = matching_weight;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnMatchingWeight(KHE_SOLN soln)                            */
/*                                                                           */
/*  Return the weight of matching in soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnMatchingWeight(KHE_SOLN soln)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingWeight: no matching");
  return soln->matching_weight;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln)                         */
/*                                                                           */
/*  Return a suitable value for the matching_weight field of soln, being     */
/*  the minimum of all the following quantities:                             */
/*                                                                           */
/*    * for each resource r, the sum of the combined weights of the avoid    */
/*      clashes constraints applicable to r;                                 */
/*                                                                           */
/*    * for each event resource er, the sum of the combined weights of the   */
/*      assign resource constraints applicable to er.                        */
/*                                                                           */
/*  If there are no such quantities, the value returned is 0.                */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln)
{
  int i, j, k;  KHE_COST weight, res;  KHE_EVENT e;  KHE_EVENT_RESOURCE er;
  KHE_RESOURCE r;  KHE_INSTANCE ins;  KHE_CONSTRAINT c;

  /* avoid clashes constraints */
  ins = KheSolnInstance(soln);
  res = KheCostMax;
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    weight = 0;
    for( j = 0;  j < KheResourceConstraintCount(r);  j++ )
    {
      c = KheResourceConstraint(r, j);
      if( KheConstraintTag(c) == KHE_AVOID_CLASHES_CONSTRAINT_TAG )
	weight += KheConstraintCombinedWeight(c);
    }
    if( weight < res )
      res = weight;
  }

  /* assign resource constraints */
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      weight = 0;
      for( k = 0;  k < KheEventResourceConstraintCount(er);  k++ )
      {
	c = KheEventResourceConstraint(er, k);
	if( KheConstraintTag(c) == KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG )
	  weight += KheConstraintCombinedWeight(c);
      }
      if( weight < res )
	res = weight;
    }
  }

  /* final value is res, or 0 if no cases */
  return res == KheCostMax ? 0 : res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return the type of soln's matching.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingType: no matching");
  return soln->matching_type;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt)         */
/*                                                                           */
/*  Set the type of soln's matching.                                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt)
{
  /* KHE_RESOURCE_IN_SOLN rs; */  KHE_MEET meet;  int i;  KHE_TASK task;
  MAssert(soln->matching != NULL, "KheSolnMatchingSetType: no matching");
  if( mt != soln->matching_type )
  {
    /* *** workload demand monitors are unaffected by type
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnMatchingSetType(rs, mt);
    *** */
    soln->matching_type = mt;  /* must come first */
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetMatchingReset(meet);
    MArrayForEach(soln->tasks, &task, &i)
      KheTaskMatchingReset(task);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingMarkBegin(KHE_SOLN soln)                             */
/*                                                                           */
/*  Begin a bracketed section of code that might return the matching to      */
/*  its initial state.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingMarkBegin(KHE_SOLN soln)
{
  if( soln->matching != NULL )
    KheMatchingMarkBegin(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo)                    */
/*                                                                           */
/*  End a bracketed section of code that might return the matching to        */
/*  its state at the start of the bracketing.  Parameter undo should be      */
/*  true if indeed it did return the matching to that state.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo)
{
  if( soln->matching != NULL )
    KheMatchingMarkEnd(soln->matching, undo);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnSupplyNodeIsOrdinary(KHE_MATCHING_SUPPLY_NODE sn,            */
/*    KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)                     */
/*                                                                           */
/*  If sn is an ordinary supply node (which at present it always is), set    */
/*  *meet to the meet it lies in, *meet_offset to its offset in that meet,   */
/*  and *r to the resource it represents, and return true.                   */
/*                                                                           */
/*****************************************************************************/

static bool KheSolnSupplyNodeIsOrdinary(KHE_MATCHING_SUPPLY_NODE sn,
  KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)
{
  *meet = (KHE_MEET)
    KheMatchingSupplyChunkImpl(KheMatchingSupplyNodeChunk(sn));
  *meet_offset = KheMeetSupplyNodeOffset(*meet, sn);
  *r = (KHE_RESOURCE) KheMatchingSupplyNodeImpl(sn);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeDebug(KHE_MATCHING_SUPPLY_NODE sn, int verbosity,      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Show supply node sn onto fp.                                             */
/*                                                                           */
/*****************************************************************************/

void KheSupplyNodeDebug(KHE_MATCHING_SUPPLY_NODE sn, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET meet;  int meet_offset;  KHE_RESOURCE r;  KHE_TIME t;
  KHE_INSTANCE ins;  KHE_EVENT e;
  if( !KheSolnSupplyNodeIsOrdinary(sn, &meet, &meet_offset, &r) )
    MAssert(false, "KheSupplyNodeDebug internal error");
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ ");
    ins = KheSolnInstance(KheMeetSoln(meet));
    if( KheMeetIsCycleMeet(meet) )
    {
      t = KheInstanceTime(ins, KheMeetAssignedTimeIndex(meet) + meet_offset);
      fprintf(fp, "%s:%s", KheResourceId(r) != NULL ? KheResourceId(r) : "-",
	KheTimeId(t) != NULL ? KheTimeId(t) : "-");
    }
    else
    {
      e = KheMeetEvent(meet);
      fprintf(fp, "%s:%s+%d", KheResourceId(r) != NULL ? KheResourceId(r) : "-",
	e==NULL ? "~" : KheEventId(e)==NULL ? "-" : KheEventId(e), meet_offset);
    }
    fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of soln's matching onto fp with the given verbosity and      */
/*  indent.                                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeDebug(KHE_MATCHING_DEMAND_NODE dn, int verbosity,
  int indent, FILE *fp)
{
  KheMonitorDebug((KHE_MONITOR) dn, verbosity, indent, fp);
}

void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  if( soln->matching != NULL )
  {
    KheSolnMatchingUpdate(soln);
    KheMatchingDebug(soln->matching, &KheSupplyNodeDebug, &KheDemandNodeDebug,
      verbosity, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - ordinary supply and demand nodes"                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln)       */
/*                                                                           */
/*  Make sure all the ordinary demand monitors of soln are attached.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingAttachAllOrdinaryDemandMonitors(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln)       */
/*                                                                           */
/*  Make sure all the ordinary demand monitors of soln are detached.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingDetachAllOrdinaryDemandMonitors(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - workload demand nodes"                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln)            */
/*                                                                           */
/*  Add all workload requirements for soln.                                  */
/*                                                                           */
/*****************************************************************************/

/* see khe_workload.c for the implementation of this function */


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln,               */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the number of workload requirements associated with r in soln.    */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  return KheResourceInSolnWorkloadRequirementCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,                   */
/*    KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg)                   */
/*                                                                           */
/*  Return the i'th workload requirement associated with r in soln.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg, KHE_MONITOR *m)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  KheResourceInSolnWorkloadRequirement(rs, i, num, tg, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln,             */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Begin a new set of workload requirements for r in soln.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  KheResourceInSolnBeginWorkloadRequirements(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,                */
/*    KHE_RESOURCE r, int num, KHE_TIME_GROUP tg, KHE_MONITOR m)             */
/*                                                                           */
/*  Add one workload requirement for r in soln.                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int num, KHE_TIME_GROUP tg, KHE_MONITOR m)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  KheResourceInSolnAddWorkloadRequirement(rs, num, tg, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln,               */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  End a new set of workload requirements for r in soln.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  KheResourceInSolnEndWorkloadRequirements(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - diagnosing failure to match"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingDefectCount(KHE_SOLN soln)                            */
/*                                                                           */
/*  Return the number of unmatched demand nodes of soln.                     */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingDefectCount(KHE_SOLN soln)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingDefectCount: no matching");
  return KheMatchingUnmatchedDemandNodeCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i)                  */
/*                                                                           */
/*  Return the i'th unmatched demand node of soln.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingDefect: no matching");
  return (KHE_MONITOR) KheMatchingUnmatchedDemandNode(soln->matching, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetCount(KHE_SOLN soln)                           */
/*                                                                           */
/*  Return the number of Hall sets.                                          */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetCount(KHE_SOLN soln)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingHallSetCount: no matching");
  return KheMatchingHallSetCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i)          */
/*                                                                           */
/*  Return the number of supply nodes in the i'th Hall set.                  */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i)
{
  KHE_MATCHING_HALL_SET hs;
  hs = KheMatchingHallSet(soln->matching, i);
  return KheMatchingHallSetSupplyNodeCount(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i)          */
/*                                                                           */
/*  Return the number of demand nodes in the i'th Hall set.                  */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i)
{
  KHE_MATCHING_HALL_SET hs;
  hs = KheMatchingHallSet(soln->matching, i);
  return KheMatchingHallSetDemandNodeCount(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,           */
/*    int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)       */
/*                                                                           */
/*  Extract information about the j'th supply node of the i'th Hall set.     */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,
  int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)
{
  KHE_MATCHING_HALL_SET hs;  KHE_MATCHING_SUPPLY_NODE sn;
  hs = KheMatchingHallSet(soln->matching, i);
  sn = KheMatchingHallSetSupplyNode(hs, j);
  return KheSolnSupplyNodeIsOrdinary(sn, meet, meet_offset, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingHallSetsDebug(KHE_SOLN soln, int verbosity,          */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of the Hall sets of soln onto fp with the given verbosity    */
/*  and indent.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingHallSetsDebug(KHE_SOLN soln, int verbosity,
  int indent, FILE *fp)
{
  KHE_MATCHING_HALL_SET hs;  int i;
  MAssert(soln->matching != NULL, "KheSolnMatchingHallSetsDebug: no matching");
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ %d Hall Sets:\n", indent, "",
      KheSolnMatchingHallSetCount(soln));
    for( i = 0;  i < KheSolnMatchingHallSetCount(soln);  i++ )
    {
      hs = KheMatchingHallSet(soln->matching, i);
      KheMatchingHallSetDebug(hs, &KheSupplyNodeDebug, &KheDemandNodeDebug,
	verbosity, indent + 2, fp);
    }
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingSetCompetitors(KHE_SOLN soln, KHE_MONITOR m)         */
/*                                                                           */
/*  Set the competitors of m.                                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingSetCompetitors(KHE_SOLN soln, KHE_MONITOR m)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingSetCompetitors: no matching");
  MAssert(KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
    KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG,
    "KheSolnMatchingSetCompetitors: m is not a demand monitor");
  MAssert(KheMonitorAttachedToSoln(m),
    "KheSolnMatchingSetCompetitors: m is not attached");
  KheMatchingSetCompetitors(soln->matching, (KHE_MATCHING_DEMAND_NODE) m);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingCompetitorCount(KHE_SOLN soln)                        */
/*                                                                           */
/*  Return the number of competitors in the current set.                     */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingCompetitorCount(KHE_SOLN soln)
{
  MAssert(soln->matching!=NULL, "KheSolnMatchingCompetitorCount: no matching");
  return KheMatchingCompetitorCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnMatchingCompetitor(KHE_SOLN soln, int i)              */
/*                                                                           */
/*  Return the i'th competitor in the current set.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnMatchingCompetitor(KHE_SOLN soln, int i)
{
  MAssert(soln->matching != NULL, "KheSolnMatchingCompetitor: no matching");
  return (KHE_MONITOR) KheMatchingCompetitor(soln->matching, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,           */
/*    int *defect_count)                                                     */
/*                                                                           */
/*  Return the cost and number of defects of monitors below soln that        */
/*  have the given tag.                                                      */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,
  int *defect_count)
{
  return KheGroupMonitorCostByType((KHE_GROUP_MONITOR) soln, tag,
    defect_count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCostByTypeDebug(KHE_SOLN soln, int verbosity,                */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of the cost of soln onto fp with the given indent.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnCostByTypeDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  KheGroupMonitorCostByTypeDebug((KHE_GROUP_MONITOR) soln,
    verbosity, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of soln onto fp with the given verbosity and indent.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  int i;  KHE_EVENT_IN_SOLN es;  KHE_MEET meet;  KHE_RESOURCE_IN_SOLN rs;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Soln (instance \"%s\", diversifier %d, cost %.5f)",
      KheInstanceId(soln->instance)!=NULL ? KheInstanceId(soln->instance):"-",
      soln->diversifier, KheCostShow(KheSolnCost(soln)));
    if( indent >= 0 )
    {
      fprintf(fp, "\n");
      if( verbosity >= 2 )
      {
	if( verbosity >= 4 )
	{
	  MArrayForEach(soln->meets, &meet, &i)
	    KheMeetDebug(meet, verbosity, indent + 2, fp);
	  MArrayForEach(soln->events_in_soln, &es, &i)
	    KheEventInSolnDebug(es, verbosity, indent + 2, fp);
	  MArrayForEach(soln->resources_in_soln, &rs, &i)
	    KheResourceInSolnDebug(rs, verbosity, indent + 2, fp);
	}
	if( verbosity >= 3 )
	{
	  fprintf(fp, "%*s  defects:\n", indent, "");
	  KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) soln, 2, indent+2, fp);
	}
	KheSolnCostByTypeDebug(soln, 2, indent + 2, fp);
      }
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
      fprintf(fp, " ]");
  }
}
