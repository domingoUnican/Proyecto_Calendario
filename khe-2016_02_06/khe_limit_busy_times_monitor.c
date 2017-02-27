
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
/*  FILE:         khe_limit_busy_times_monitor.c                             */
/*  DESCRIPTION:  A time group monitor                                       */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG1_RESOURCE "T07"
#define DEBUG1_R(rs) ( DEBUG1 &&					\
  strcmp(KheResourceId(KheResourceInSolnResource(rs)), DEBUG1_RESOURCE)==0 )


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR - monitors busy times                       */
/*                                                                           */
/*****************************************************************************/

struct khe_limit_busy_times_monitor_rec {
  INHERIT_MONITOR
  int				deviation;		/* total dev         */
  int				new_deviation;		/* total dev         */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT constraint;		/* monitoring this   */
  int				minimum;		/* from constraint   */
  int				maximum;		/* from constraint   */
  int				ceiling;		/* user-defined      */
  /* bool			maximum_attached; */	/* maximum attached  */
  /* bool			separate; */		/* separate          */
  /* KHE_DEV_MONITOR		separate_dev_monitor;*/	/* separate dev      */
  ARRAY_KHE_TIME_GROUP		defective_time_groups;	/* defective tg's    */
  ARRAY_INT			defective_busy_counts;	/* defective counts  */
  KHE_LIMIT_BUSY_TIMES_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDebugCost(KHE_LIMIT_BUSY_TIMES_MONITOR m,   */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print the cost of m onto fp.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorDebugCost(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  FILE *fp)
{
  fprintf(fp, "cost (");
  /* ***
  if( m->separate )
    KheDevMonitorDebug(&m->separate_dev_monitor, fp);
  else
  *** */
  fprintf(fp, "%d", m->deviation);
  fprintf(fp, ") %.5f\n", KheCostShow(m->cost));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorSetLowerBound(                              */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Set the lower bound of m.                                                */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorSetLowerBound(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KHE_RESOURCE r;  int events_durn, min_in_domain, max_in_domain;
  KHE_TIME_GROUP domain;  KHE_INSTANCE ins;  KHE_CONSTRAINT mc;

  /* if r does not have a highly weighted avoid clashes constraint, quit */
  r = KheLimitBusyTimesMonitorResource(m);
  mc = (KHE_CONSTRAINT) m->constraint;
  if( !KheResourceHasAvoidClashesConstraint(r, KheConstraintCombinedWeight(mc)))
    return;  /* no highly weighted avoid clashes constraint */

  /* find the duration of highly weighted preassigned events */
  events_durn = KheResourcePreassignedEventsDuration(r,
    KheConstraintCombinedWeight(mc));

  /* min_in_domain = the min number of domain times occupied by workload */
  ins = KheResourceInstance(r);
  domain = KheLimitBusyTimesConstraintDomain(m->constraint);
  min_in_domain = events_durn -
    (KheInstanceTimeCount(ins) - KheTimeGroupTimeCount(domain));

  /* max_in_domain = the max number of cost-free occupied domain times */
  max_in_domain = m->maximum *
    KheLimitBusyTimesConstraintTimeGroupCount(m->constraint);
  if( min_in_domain > max_in_domain )
  {
    m->lower_bound = KheConstraintCost((KHE_CONSTRAINT) m->constraint,
      min_in_domain - max_in_domain);
    if( DEBUG2 )
      fprintf(stderr, "  setting limit busy times lower bound of %s to %.5f\n",
	KheResourceId(r), KheCostShow(m->lower_bound));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(               */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)            */
/*                                                                           */
/*  Make a new limit busy times monitor for rs.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  KHE_LIMIT_BUSY_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  if( DEBUG1_R(rs) )
    fprintf(stderr, "[ KheLimitBusyTimesMonitorMake(%s, %s)\n",
      KheResourceId(KheResourceInSolnResource(rs)),
      KheConstraintId( (KHE_CONSTRAINT) c));
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LIMIT_BUSY_TIMES_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->constraint = c;
  res->minimum = KheLimitBusyTimesConstraintMinimum(c);
  res->maximum = KheLimitBusyTimesConstraintMaximum(c);
  res->ceiling = INT_MAX;
  /* res->maximum_attached = true; */
  /* ***
  res->separate =
    (KheConstraintCostFunction((KHE_CONSTRAINT) c) != KHE_SUM_COST_FUNCTION);
  KheDevMonitorInit(&res->separate_dev_monitor);
  *** */
  res->deviation = 0;
  res->new_deviation = 0;
  MArrayInit(res->defective_time_groups);
  MArrayInit(res->defective_busy_counts);
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  KheLimitBusyTimesMonitorSetLowerBound(res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  if( DEBUG1_R(rs) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorMake returning, ");
    /* ***
    fprintf(stderr, "] KheLimitBusyTimesMonitorMake returning (%sseparate), ",
      res->separate ? "" : "!");
    *** */
    KheLimitBusyTimesMonitorDebugCost(res, stderr);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(         */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KHE_LIMIT_BUSY_TIMES_MONITOR copy;  KHE_TIME_GROUP tg;  int i, busy_count;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFieldsPhase1((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->resource_in_soln =
      KheResourceInSolnCopyPhase1(m->resource_in_soln);
    copy->constraint = m->constraint;
    copy->minimum = m->minimum;
    copy->maximum = m->maximum;
    copy->ceiling = m->ceiling;
    /* copy->maximum_attached = m->maximum_attached; */
    /* ***
    copy->separate = m->separate;
    KheDevMonitorCopy(&copy->separate_dev_monitor, &m->separate_dev_monitor);
    *** */
    copy->deviation = m->deviation;
    copy->new_deviation = m->new_deviation;
    MArrayInit(copy->defective_time_groups);
    MArrayForEach(m->defective_time_groups, &tg, &i)
      MArrayAddLast(copy->defective_time_groups, tg);
    MArrayInit(copy->defective_busy_counts);
    MArrayForEach(m->defective_busy_counts, &busy_count, &i)
      MArrayAddLast(copy->defective_busy_counts, busy_count);
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/* void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m)   */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheMonitorCopyCommonFieldsPhase2((KHE_MONITOR) m);
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m)      */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorDelete(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  if( m->attached )
    KheLimitBusyTimesMonitorDetachFromSoln(m);
  KheMonitorDeleteAllParentMonitors((KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  /* KheDevMonitorFree(&m->separate_dev_monitor); */
  MArrayFree(m->defective_time_groups);
  MArrayFree(m->defective_busy_counts);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource monitor holding m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->resource_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheLimitBusyTimesMonitorResource(                           */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLimitBusyTimesMonitorResource(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return KheResourceInSolnResource(m->resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorAttachToSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)*/
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*  Note: for cost 0, the number of busy times in each time group has        */
/*  to be either 0 or lie between m->minimum and m->maximum.  The first      */
/*  part of this condition means that the initial cost is always 0.          */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorAttachToSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorAttachToSoln(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDetachFromSoln(                             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDetachFromSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorDetachFromSoln(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorAttachCheck(KHE_LIMIT_BUSY_TIMES_MONITOR m) */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheLimitBusyTimesMonitorAttachCheck(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ceiling"                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorSetCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m,  */
/*    int ceiling)                                                           */
/*                                                                           */
/*  Set m's ceiling attribute.                                               */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorSetCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int ceiling)
{
  MAssert(ceiling >= 0, "KheLimitBusyTimesMonitorSetCeiling: negative ceiling",
    ceiling);
  if( m->attached )
  {
    KheLimitBusyTimesMonitorDetachFromSoln(m);
    m->ceiling = ceiling;
    KheLimitBusyTimesMonitorAttachToSoln(m);
  }
  else
    m->ceiling = ceiling;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m)      */
/*                                                                           */
/*  Return m's ceiling attribute.                                            */
/*                                                                           */
/*****************************************************************************/

int KheLimitBusyTimesMonitorCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->ceiling;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "detaching and attaching the maximum"                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorMaximumDetachFromSoln(                      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Detach maximum-monitoring.                                               */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete 
void KheLimitBusyTimesMonitorMaximumDetachFromSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  MAssert(m->maximum_attached,
    "KheLimitBusyTimesMonitorMaximumDetachFromSoln: already detached");
  if( m->attached )
  {
    KheLimitBusyTimesMonitorDetachFromSoln(m);
    m->maximum_attached = false;
    KheLimitBusyTimesMonitorAttachToSoln(m);
  }
  else
    m->maximum_attached = false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorMaximumAttachToSoln(                        */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Attach maximum-monitoring.                                               */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete 
void KheLimitBusyTimesMonitorMaximumAttachToSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  MAssert(!m->maximum_attached,
    "KheLimitBusyTimesMonitorMaximumDetachFromSoln: already attached");
  if( m->attached )
  {
    KheLimitBusyTimesMonitorDetachFromSoln(m);
    m->maximum_attached = true;
    KheLimitBusyTimesMonitorAttachToSoln(m);
  }
  else
    m->maximum_attached = true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitBusyTimesMonitorMaximumAttachedToSoln(                      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return true if maximum-monitoring is attached.                           */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete 
bool KheLimitBusyTimesMonitorMaximumAttachedToSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->maximum_attached;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorDev(                                         */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count)                        */
/*                                                                           */
/*  Work out the deviations caused by this busy_count.                       */
/*                                                                           */
/*****************************************************************************/

static int KheLimitBusyTimesMonitorDev(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count)
{
  if( busy_count == 0 || busy_count > m->ceiling )
    return 0;
  else if( busy_count < m->minimum )
    return m->minimum - busy_count;
  else if( busy_count > m->maximum )
    return busy_count - m->maximum;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorFlush(KHE_LIMIT_BUSY_TIMES_MONITOR m)       */
/*                                                                           */
/*  Flush m, assuming that there has been a change in deviation.             */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorFlush(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorFlush(m), init ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->new_deviation));
  m->deviation = m->new_deviation;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorFlush(m), final ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}

/* ***
static void KheLimitBusyTimesMonitorFlush(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorFlush(m), init ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  if( m->separate )
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
        KheDevMonitorDevs(&m->separate_dev_monitor)));
    KheDevMonitorFlush(&m->separate_dev_monitor);
  }
  else
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->new_deviation));
    m->deviation = m->new_deviation;
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorFlush(m), final ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorAddBusyAndIdle(                             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,            */
/*    int busy_count, int idle_count)                                        */
/*                                                                           */
/*  Add a busy value for one monitored time group.                           */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count)
{
  int dev, pos;  KHE_TIME_GROUP tg;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorAddBusyAndIdle(m, %d, %d), ",
      busy_count, idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  dev = KheLimitBusyTimesMonitorDev(m, busy_count);
  if( dev != 0 )
  {
    tg = KheTimeGroupMonitorTimeGroup(tgm);
    /* ***
    if( m->separate )
      KheDevMonitorAttach(&m->separate_dev_monitor, dev);
    else
    *** */
    m->new_deviation += dev;
    MAssert(!MArrayContains(m->defective_time_groups, tg, &pos),
      "KheLimitBusyTimesMonitorAddBusyAndIdle internal error");
    MArrayAddLast(m->defective_time_groups, tg);
    MArrayAddLast(m->defective_busy_counts, busy_count);
    KheLimitBusyTimesMonitorFlush(m);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorAddBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*   void KheLimitBusyTimesMonitorDeleteBusyAndIdle(                         */
/*     KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,           */
/*     int busy_count, int idle_count)                                       */
/*                                                                           */
/*  Remove the busy value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count)
{
  int dev, pos;  KHE_TIME_GROUP tg;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorDeleteBusyAndIdle(m, %d, %d) ",
      busy_count, idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  dev = KheLimitBusyTimesMonitorDev(m, busy_count);
  if( dev != 0 )
  {
    tg = KheTimeGroupMonitorTimeGroup(tgm);
    /* ***
    if( m->separate )
      KheDevMonitorDetach(&m->separate_dev_monitor, dev);
    else
    *** */
    m->new_deviation -= dev;
    KheLimitBusyTimesMonitorFlush(m);
    MAssert(MArrayContains(m->defective_time_groups, tg, &pos),
      "KheLimitBusyTimesMonitorDeleteBusyAndIdle internal error");
    MArrayRemove(m->defective_time_groups, pos);
    MArrayRemove(m->defective_busy_counts, pos);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorDeleteBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorChangeBusyAndIdle(                          */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,            */
/*    int old_busy_count, int new_busy_count, int old_idle_count,            */
/*    int new_idle_count)                                                    */
/*                                                                           */
/*  Change the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count, int old_idle_count,
  int new_idle_count)
{
  int old_dev, new_dev, pos;  KHE_TIME_GROUP tg;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ %s(m, %d, %d, %d, %d), ",
      "KheLimitBusyTimesMonitorChangeBusyAndIdle", old_busy_count,
      new_busy_count, old_idle_count, new_idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  old_dev = KheLimitBusyTimesMonitorDev(m, old_busy_count);
  new_dev = KheLimitBusyTimesMonitorDev(m, new_busy_count);
  if( old_dev != new_dev )
  {
    tg = KheTimeGroupMonitorTimeGroup(tgm);
    /* ***
    if( m->separate )
    {
      if( old_dev == 0 )
	KheDevMonitorAttach(&m->separate_dev_monitor, new_dev);
      else if( new_dev == 0 )
	KheDevMonitorDetach(&m->separate_dev_monitor, old_dev);
      else
	KheDevMonitorUpdate(&m->separate_dev_monitor, old_dev, new_dev);
    }
    else
    *** */
    m->new_deviation += (new_dev - old_dev);
    if( old_dev == 0 )
    {
      MAssert(!MArrayContains(m->defective_time_groups, tg, &pos),
	"KheLimitBusyTimesMonitorChangeBusyAndIdle internal error 1");
      MArrayAddLast(m->defective_time_groups, tg);
      MArrayAddLast(m->defective_busy_counts, new_busy_count);
    }
    else if( new_dev == 0 )
    {
      MAssert(MArrayContains(m->defective_time_groups, tg, &pos),
	"KheLimitBusyTimesMonitorChangeBusyAndIdle internal error");
      MArrayRemove(m->defective_time_groups, pos);
      MArrayRemove(m->defective_busy_counts, pos);
    }
    else
    {
      MAssert(MArrayContains(m->defective_time_groups, tg, &pos),
	"KheLimitBusyTimesMonitorChangeBusyAndIdle internal error");
      MArrayPut(m->defective_busy_counts, pos, new_busy_count);
    }
    KheLimitBusyTimesMonitorFlush(m);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorChangeBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorDefectiveTimeGroupCount(                     */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the number of defective time groups.                              */
/*                                                                           */
/*****************************************************************************/

int KheLimitBusyTimesMonitorDefectiveTimeGroupCount(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return MArraySize(m->defective_time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDefectiveTimeGroup(                         */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i, KHE_TIME_GROUP *tg,             */
/*    int *busy_count, int *minimum, int *maximum)                           */
/*                                                                           */
/*  Return the i'th defective time group.                                    */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDefectiveTimeGroup(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i, KHE_TIME_GROUP *tg,
  int *busy_count, int *minimum, int *maximum)
{
  *tg = MArrayGet(m->defective_time_groups, i);
  *busy_count = MArrayGet(m->defective_busy_counts, i);
  *minimum = m->minimum;
  *maximum = m->maximum;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorDeviation(KHE_LIMIT_BUSY_TIMES_MONITOR m)    */
/*                                                                           */
/*  Return the deviation of m.                                               */
/*                                                                           */
/*****************************************************************************/

int KheLimitBusyTimesMonitorDeviation(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDeviationShowOneTimeGroup(                  */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i, ARRAY_CHAR *ac)                 */
/*                                                                           */
/*  Show one time group in the deviation description of m.                   */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorDeviationShowOneTimeGroup(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i, ARRAY_CHAR *ac)
{
  int busy_count, minimum, maximum;  KHE_TIME_GROUP tg;  char *name;
  KheLimitBusyTimesMonitorDefectiveTimeGroup(m, i, &tg, &busy_count,
    &minimum, &maximum);
  name = KheTimeGroupName(tg);
  if( busy_count < minimum )
    MStringPrintf(*ac, 100, "%d too few in %s", minimum - busy_count,
      name == NULL ? "?" : name);
  else if( busy_count > maximum )
    MStringPrintf(*ac, 100, "%d too many in %s", busy_count - maximum,
      name == NULL ? "?" : name);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitBusyTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return a description of the deviation of m in heap memory.               */
/*                                                                           */
/*****************************************************************************/

char *KheLimitBusyTimesMonitorDeviationDescription(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  ARRAY_CHAR ac;  int i;  MStringInit(ac);
  if( m->deviation == 0 )
    MStringAddString(ac, "0");
  else if( KheLimitBusyTimesMonitorDefectiveTimeGroupCount(m) == 1 )
    KheLimitBusyTimesMonitorDeviationShowOneTimeGroup(m, 0, &ac);
  else
  {
    MStringPrintf(ac, 100, "%d: ", m->deviation);
    for( i = 0;  i < KheLimitBusyTimesMonitorDefectiveTimeGroupCount(m);  i++ )
    {
      if( i > 0 )
	MStringAddString(ac, "; ");
      KheLimitBusyTimesMonitorDeviationShowOneTimeGroup(m, i, &ac);
    }
  }
  return MStringVal(ac);
}


/*****************************************************************************/
/*                                                                           */
/* int KheLimitBusyTimesMonitorDeviationCount(KHE_LIMIT_BUSY_TIMES_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitBusyTimesMonitorDeviationCount(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return KheLimitBusyTimesConstraintTimeGroupCount(m->constraint);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorDeviation(                                   */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitBusyTimesMonitorDeviation(KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)
{
  KHE_TIMETABLE_MONITOR tt;  KHE_RESOURCE r;  KHE_TIME_GROUP tg;
  KHE_TIME_GROUP_MONITOR tgm;  int busy_count;
  r = KheResourceInSolnResource(m->resource_in_soln);
  tt = KheResourceTimetableMonitor(m->soln, r);
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tt) )
    KheMonitorAttachToSoln((KHE_MONITOR) tt);
  tg = KheLimitBusyTimesConstraintTimeGroup(m->constraint, i);
  if( !KheTimetableMonitorContainsTimeGroupMonitor(tt, tg, &tgm) )
    MAssert(false, "KheLimitBusyTimesMonitorDeviation internal error");
  busy_count = KheTimeGroupMonitorBusyCount(tgm);
  return KheLimitBusyTimesMonitorDev(m, busy_count);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitBusyTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

/* ***
char *KheLimitBusyTimesMonitorDeviationDescription(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)
{
  KHE_TIME_GROUP tg;
  tg = KheLimitBusyTimesConstraintTimeGroup(m->constraint, i);
  return KheTimeGroupName(tg);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,       */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  char buff[20];
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    if( m->ceiling < INT_MAX )
      snprintf(buff, 20, ", ceil %d", m->ceiling);
    else
      snprintf(buff, 20, "%s", "");
    fprintf(fp, " %s (min %d, max %d%s) ",
      KheResourceInSolnId(m->resource_in_soln),
      m->minimum, m->maximum, buff);
    fprintf(fp, "%s%d", m->deviation != m->new_deviation ? "*" : "",
      m->deviation);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}

/* ***
void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  char buff[20];
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    if( m->ceiling < INT_MAX )
      snprintf(buff, 20, ", ceil %d", m->ceiling);
    else
      snprintf(buff, 20, "%s", "");
    fprintf(fp, " %s (min %d, max %d%s)%s ",
      KheResourceInSolnId(m->resource_in_soln),
      m->minimum, m->maximum, buff, m->separate ? " sep" : "");
    if( m->separate )
      KheDevMonitorDebug(&m->separate_dev_monitor, fp);
    else
      fprintf(fp, "%s%d", m->deviation != m->new_deviation ? "*" : "",
	m->deviation);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
*** */
