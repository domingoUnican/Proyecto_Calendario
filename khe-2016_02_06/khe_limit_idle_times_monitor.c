
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
/*  FILE:         khe_limit_idle_times_monitor.c                             */
/*  DESCRIPTION:  A limit idle times monitor                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR - monitors idle times                       */
/*                                                                           */
/*****************************************************************************/

struct khe_limit_idle_times_monitor_rec {
  INHERIT_MONITOR
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  ARRAY_KHE_TIME_GROUP_MONITOR	time_group_monitors;	/* time group mons   */
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT constraint;		/* monitoring this   */
  int				minimum;		/* from constraint   */
  int				maximum;		/* from constraint   */
  int				total_idle_count;	/* total idle times  */
  int				new_total_idle_count;	/* new total idle    */
  KHE_LIMIT_IDLE_TIMES_MONITOR	copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(               */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)            */
/*                                                                           */
/*  Make a new limit idle times monitor for rs.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LIMIT_IDLE_TIMES_MONITOR_TAG);
  res->resource_in_soln = rs;
  MArrayInit(res->time_group_monitors);
  res->constraint = c;
  res->minimum = KheLimitIdleTimesConstraintMinimum(c);
  res->maximum = KheLimitIdleTimesConstraintMaximum(c);
  res->total_idle_count = 0;
  res->new_total_idle_count = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(         */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR copy;  KHE_TIME_GROUP_MONITOR tgm;  int i;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFieldsPhase1((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->resource_in_soln =
      KheResourceInSolnCopyPhase1(m->resource_in_soln);
    MArrayInit(copy->time_group_monitors);
    MArrayForEach(m->time_group_monitors, &tgm, &i)
      MArrayAddLast(copy->time_group_monitors,
	KheTimeGroupMonitorCopyPhase1(tgm));
    copy->constraint = m->constraint;
    copy->minimum = m->minimum;
    copy->maximum = m->maximum;
    copy->total_idle_count = m->total_idle_count;
    copy->new_total_idle_count = m->new_total_idle_count;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m)  */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  int i;
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheMonitorCopyCommonFieldsPhase2((KHE_MONITOR) m);
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
    MArrayForEach(m->time_group_monitors, &tgm, &i)
      KheTimeGroupMonitorCopyPhase2(tgm);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m)      */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  if( m->attached )
    KheLimitIdleTimesMonitorDetachFromSoln(m);
  KheMonitorDeleteAllParentMonitors((KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource monitor holding m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return m->resource_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(      */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheLimitIdleTimesMonitorResource(                           */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLimitIdleTimesMonitorResource(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return KheResourceInSolnResource(m->resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time group monitors"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAddTimeGroupMonitor(                        */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm)            */
/*                                                                           */
/*  Add tgm to m.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAddTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm)
{
  MArrayAddLast(m->time_group_monitors, tgm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDeleteTimeGroupMonitor(                     */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm)            */
/*                                                                           */
/*  Delete tgm from m.  It must be present.                                  */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDeleteTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm)
{
  int pos;
  if( !MArrayContains(m->time_group_monitors, tgm, &pos) )
    MAssert(false,
      "KheLimitIdleTimesMonitorDeleteTimeGroupMonitor internal error");
  MArrayRemove(m->time_group_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitIdleTimesMonitorTimeGroupMonitorCount(                       */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the number of time group monitors monitored by m.                 */
/*                                                                           */
/*****************************************************************************/

int KheLimitIdleTimesMonitorTimeGroupMonitorCount(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return MArraySize(m->time_group_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_MONITOR KheLimitIdleTimesMonitorTimeGroupMonitor(         */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return the i'th time group monitor monitored by m.                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_MONITOR KheLimitIdleTimesMonitorTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)
{
  return MArrayGet(m->time_group_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAttachToSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)*/
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAttachToSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDetachFromSoln(                             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDetachFromSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAttachCheck(KHE_LIMIT_IDLE_TIMES_MONITOR m) */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheLimitIdleTimesMonitorAttachCheck(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitIdleTimesMonitorDev(KHE_LIMIT_IDLE_TIMES_MONITOR m,          */
/*      int idle_count)                                                      */
/*                                                                           */
/*  Work out the deviations caused by this many idle times.                  */
/*                                                                           */
/*****************************************************************************/

static int KheLimitIdleTimesMonitorDev(KHE_LIMIT_IDLE_TIMES_MONITOR m,
    int idle_count)
{
  if( idle_count < m->minimum )
    return m->minimum - idle_count;
  else if( idle_count > m->maximum )
    return idle_count - m->maximum;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorFlush(KHE_LIMIT_IDLE_TIMES_MONITOR m)       */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheLimitIdleTimesMonitorFlush(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  int old_devs, new_devs;
  if( m->new_total_idle_count != m->total_idle_count )
  {
    old_devs = KheLimitIdleTimesMonitorDev(m, m->total_idle_count);
    new_devs = KheLimitIdleTimesMonitorDev(m, m->new_total_idle_count);
    if( old_devs != new_devs )
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, new_devs));
    m->total_idle_count = m->new_total_idle_count;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAddBusyAndIdle(                             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,            */
/*    int busy_count, int idle_count)                                        */
/*                                                                           */
/*  Add an idle value for one monitored time group.                          */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count)
{
  if( idle_count != 0 )
  {
    m->new_total_idle_count += idle_count;
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDeleteBusyAndIdle(                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,            */
/*    int busy_count, int idle_count)                                        */
/*                                                                           */
/*  Remove the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count)
{
  if( idle_count != 0 )
  {
    m->new_total_idle_count -= idle_count;
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorChangeBusyAndIdle(                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,            */
/*    int old_busy_count, int new_busy_count,                                */
/*    int old_idle_count, int new_idle_count)                                */
/*                                                                           */
/*  Change the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count,
  int old_idle_count, int new_idle_count)
{
  if( new_idle_count != old_idle_count )
  {
    m->new_total_idle_count += (new_idle_count - old_idle_count);
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m)    */
/*                                                                           */
/*  Return the deviation of m.                                               */
/*                                                                           */
/*****************************************************************************/

int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return KheLimitIdleTimesMonitorDev(m, m->total_idle_count);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitIdleTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return a description of the deviation of m in heap memory.               */
/*                                                                           */
/*****************************************************************************/

char *KheLimitIdleTimesMonitorDeviationDescription(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  ARRAY_CHAR ac;  int i, count, idle;  char *name;  KHE_TIME_GROUP_MONITOR tgm;
  MStringInit(ac);
  if( KheLimitIdleTimesMonitorDeviation(m) == 0 )
    MStringAddString(ac, "0");
  else
  {
    MStringPrintf(ac, 100, "%d%s in ", KheLimitIdleTimesMonitorDeviation(m),
      m->total_idle_count < m->minimum ? " too few" :
      m->maximum > 0 ? " too many" : "");
    count = 0;
    MArrayForEach(m->time_group_monitors, &tgm, &i)
    {
      idle = KheTimeGroupMonitorIdleCount(tgm);
      if( idle > 0 )
      {
	if( count > 0 )
          MStringAddString(ac, ", ");
	name = KheTimeGroupName(KheTimeGroupMonitorTimeGroup(tgm));
	MStringPrintf(ac, 100, "%s", name == NULL ? "?" : name);
	count++;
      }
    }
  }
  return MStringVal(ac);
}


/*****************************************************************************/
/*                                                                           */
/* int KheLimitIdleTimesMonitorDeviationCount(KHE_LIMIT_IDLE_TIMES_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitIdleTimesMonitorDeviationCount(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return 1;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m,    */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0, "KheLimitIdleTimesMonitorDeviation: i out of range");
  return KheLimitIdleTimesMonitorDev(m, m->total_idle_count);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitIdleTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

/* ***
char *KheLimitIdleTimesMonitorDeviationDescription(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheLimitIdleTimesMonitorDeviationDescription: i out of range");
  return NULL;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,       */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp at the given indent.                            */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  KHE_RESOURCE r;
  if( verbosity >= 1 )
  {
    r = KheLimitIdleTimesMonitorResource(m);
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    if( m->minimum > 0 )
      fprintf(fp, " %s (min %d, max %d, idle %d)",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r),
	  m->minimum, m->maximum, m->total_idle_count);
    else
      fprintf(fp, " %s (max %d, idle %d)",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r),
	  m->maximum, m->total_idle_count);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
