
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
/*  FILE:         khe_limit_workload_monitor.c                               */
/*  DESCRIPTION:  A limit workload monitor                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DN "Welfare07"
#define DO_DEBUG1(m) (DEBUG1 && 0 ==					\
 strcmp(KheResourceName(KheResourceInSolnResource((m)->resource_in_soln)),DN))


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_WORKLOAD_MONITOR                                               */
/*                                                                           */
/*****************************************************************************/

struct khe_limit_workload_monitor_rec {
  INHERIT_MONITOR
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_LIMIT_WORKLOAD_CONSTRAINT	constraint;		/* monitoring this   */
  int				minimum;		/* from constraint   */
  int				maximum;		/* from constraint   */
  int				ceiling;		/* user-defined      */
  /* bool			maximum_attached; */	/* maximum attached  */
  float				workload;		/* workload          */
  int				deviation;		/* deviation         */
  KHE_LIMIT_WORKLOAD_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorSetLowerBound(KHE_LIMIT_WORKLOAD_MONITOR m)  */
/*                                                                           */
/*  If m's lower bound is greater than 0, set m->lower_bound to its value.   */
/*                                                                           */
/*****************************************************************************/

static void KheLimitWorkloadMonitorSetLowerBound(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  KHE_RESOURCE r;  KHE_EVENT_RESOURCE er;  int i, workload;

  /* find the total workload of preassigned event resources */
  workload = 0;
  r = KheLimitWorkloadMonitorResource(m);
  for( i = 0;  i < KheResourcePreassignedEventResourceCount(r);  i++ )
  {
    er = KheResourcePreassignedEventResource(r, i);
    workload += KheEventResourceWorkload(er);
  }

  /* set the lower bound if non-zero */
  if( workload > m->maximum )
    m->lower_bound = KheConstraintCost((KHE_CONSTRAINT) m->constraint,
      workload - m->maximum);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorMake(                  */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_WORKLOAD_CONSTRAINT c)              */
/*                                                                           */
/*  Make a new limit workload monitor object with these attributes.          */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_WORKLOAD_CONSTRAINT c)
{
  KHE_LIMIT_WORKLOAD_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LIMIT_WORKLOAD_MONITOR_TAG);
  MAssert(rs != NULL, "KheLimitWorkloadMonitorMake internal error");
  res->resource_in_soln = rs;
  res->constraint = c;
  res->minimum = KheLimitWorkloadConstraintMinimum(c);
  res->maximum = KheLimitWorkloadConstraintMaximum(c);
  res->ceiling = INT_MAX;
  /* res->maximum_attached = true; */
  res->workload = 0.0;
  res->deviation = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  KheLimitWorkloadMonitorSetLowerBound(res);
  if( DO_DEBUG1(res) )
  {
    fprintf(stderr, "  ");
    KheLimitWorkloadMonitorDebug(res, 1, -1, stderr);
    fprintf(stderr, "  after make\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorCopyPhase1(            */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorCopyPhase1(
  KHE_LIMIT_WORKLOAD_MONITOR m)
{
  KHE_LIMIT_WORKLOAD_MONITOR copy;
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
    copy->workload = m->workload;
    copy->deviation = m->deviation;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorCopyPhase2(KHE_LIMIT_WORKLOAD_MONITOR m)     */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorCopyPhase2(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheMonitorCopyCommonFieldsPhase2((KHE_MONITOR) m);
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
    if( DO_DEBUG1(m) )
    {
      fprintf(stderr, "  ");
      KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
      fprintf(stderr, "  after copy\n");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorDelete(KHE_LIMIT_WORKLOAD_MONITOR m)         */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorDelete(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  if( m->attached )
    KheLimitWorkloadMonitorDetachFromSoln(m);
  if( DO_DEBUG1(m) )
  {
    fprintf(stderr, "  ");
    KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
    fprintf(stderr, "  before delete\n");
  }
  KheMonitorDeleteAllParentMonitors((KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_WORKLOAD_CONSTRAINT KheLimitWorkloadMonitorConstraint(         */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Return the constraint monitored by m.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_WORKLOAD_CONSTRAINT KheLimitWorkloadMonitorConstraint(
  KHE_LIMIT_WORKLOAD_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/* KHE_RESOURCE KheLimitWorkloadMonitorResource(KHE_LIMIT_WORKLOAD_MONITOR m)*/
/*                                                                           */
/*  Return the resource monitored by m.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLimitWorkloadMonitorResource(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  return KheResourceInSolnResource(m->resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  float KheLimitWorkloadMonitorWorkload(KHE_LIMIT_WORKLOAD_MONITOR m)      */
/*                                                                           */
/*  Return the workload of the resource monitored by m.                      */
/*                                                                           */
/*****************************************************************************/

float KheLimitWorkloadMonitorWorkload(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  return m->workload;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorWorkloadAndLimits(                           */
/*    KHE_LIMIT_WORKLOAD_MONITOR m, float *workload,                         */
/*    int *minimum, int *maximum)                                            */
/*                                                                           */
/*  Return the workload, minimum, and maximum.                               */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorWorkloadAndLimits(
  KHE_LIMIT_WORKLOAD_MONITOR m, float *workload,
  int *minimum, int *maximum)
{
  *workload = m->workload;
  *minimum = KheLimitWorkloadConstraintMinimum(m->constraint);
  *maximum = KheLimitWorkloadConstraintMaximum(m->constraint);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorAttachToSoln(KHE_LIMIT_WORKLOAD_MONITOR m)   */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorAttachToSoln(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  if( DO_DEBUG1(m) )
  {
    fprintf(stderr, "  ");
    KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
    fprintf(stderr, "  after attach\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorDetachFromSoln(KHE_LIMIT_WORKLOAD_MONITOR m) */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorDetachFromSoln(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
  if( DO_DEBUG1(m) )
  {
    fprintf(stderr, "  ");
    KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
    fprintf(stderr, "  after detach\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorAttachCheck(KHE_LIMIT_WORKLOAD_MONITOR m)    */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheLimitWorkloadMonitorAttachCheck(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ceiling attribute"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorSetCeiling(KHE_LIMIT_WORKLOAD_MONITOR m,     */
/*    int ceiling)                                                           */
/*                                                                           */
/*  Set m's ceiling attribute.                                               */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorSetCeiling(KHE_LIMIT_WORKLOAD_MONITOR m,
  int ceiling)
{
  MAssert(ceiling >= 0, "KheLimitWorkloadMonitorSetCeiling: negative ceiling",
    ceiling);
  if( m->attached )
  {
    KheLimitWorkloadMonitorDetachFromSoln(m);
    m->ceiling = ceiling;
    KheLimitWorkloadMonitorAttachToSoln(m);
  }
  else
    m->ceiling = ceiling;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitWorkloadMonitorCeiling(KHE_LIMIT_WORKLOAD_MONITOR m)         */
/*                                                                           */
/*  Return m's ceiling attribute.                                            */
/*                                                                           */
/*****************************************************************************/

int KheLimitWorkloadMonitorCeiling(KHE_LIMIT_WORKLOAD_MONITOR m)
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
/*  void KheLimitWorkloadMonitorMaximumDetachFromSoln(                       */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Detach maximum-monitoring.                                               */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete
void KheLimitWorkloadMonitorMaximumDetachFromSoln(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  MAssert(m->maximum_attached,
    "KheLimitWorkloadMonitorMaximumDetachFromSoln: already detached");
  if( m->attached )
  {
    KheLimitWorkloadMonitorDetachFromSoln(m);
    m->maximum_attached = false;
    KheLimitWorkloadMonitorAttachToSoln(m);
  }
  else
    m->maximum_attached = false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorMaximumAttachToSoln(                         */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Attach maximum-monitoring.                                               */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete
void KheLimitWorkloadMonitorMaximumAttachToSoln(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  MAssert(!m->maximum_attached,
    "KheLimitWorkloadMonitorMaximumDetachFromSoln: already attached");
  if( m->attached )
  {
    KheLimitWorkloadMonitorDetachFromSoln(m);
    m->maximum_attached = true;
    KheLimitWorkloadMonitorAttachToSoln(m);
  }
  else
    m->maximum_attached = true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitWorkloadMonitorMaximumAttachedToSoln(                       */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Return true if maximum-monitoring is attached.                           */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete
bool KheLimitWorkloadMonitorMaximumAttachedToSoln(KHE_LIMIT_WORKLOAD_MONITOR m)
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
/*  int KheLimitWorkloadMonitorDev(KHE_LIMIT_WORKLOAD_MONITOR m,             */
/*    float workload)                                                        */
/*                                                                           */
/*  Work out the deviation caused by this much workload.                     */
/*                                                                           */
/*****************************************************************************/

static int KheLimitWorkloadMonitorDev(KHE_LIMIT_WORKLOAD_MONITOR m,
  float workload)
{
  if( workload > m->ceiling + 0.001 )
    return 0;
  else if( workload < m->minimum - 0.001 )
    return (int) ceil(m->minimum - 0.001 - workload);
  else if( workload > m->maximum + 0.001 )
    return (int) ceil(workload - m->maximum - 0.001);
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorAssignResource(                              */
/*    KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r)           */ 
/*                                                                           */
/*  Inform m that its resource r is being assigned to task.                  */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorAssignResource(KHE_LIMIT_WORKLOAD_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  float workload, new_workload, new_deviation;
  workload = KheTaskWorkload(task);
  if( workload != 0.0 )
  {
    new_workload = m->workload + workload;
    new_deviation = KheLimitWorkloadMonitorDev(m, new_workload);
    if( new_deviation != m->deviation )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, new_deviation));
      m->deviation = new_deviation;
    }
    m->workload = new_workload;
    if( DO_DEBUG1(m) )
    {
      fprintf(stderr, "  ");
      KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
      fprintf(stderr, "  after assign ");
      KheTaskDebug(task, 1, -1, stderr);
      fprintf(stderr, "\n");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadMonitorUnAssignResource(                            */
/*    KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r)           */
/*                                                                           */
/*  Inform m that its resource r is being unassigned from task.              */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorUnAssignResource(KHE_LIMIT_WORKLOAD_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  float workload, new_workload, new_deviation;
  workload = KheTaskWorkload(task);
  if( workload != 0.0 )
  {
    new_workload = m->workload - workload;
    /* *** have observed this being triggered on 0.000016!
    MAssert(new_workload >= -0.00001,
      "KheLimitWorkloadMonitorDeleteTask internal error (workload %.6f)",
      new_workload);
    *** */
    new_deviation = KheLimitWorkloadMonitorDev(m, new_workload);
    if( new_deviation != m->deviation )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, new_deviation));
      m->deviation = new_deviation;
    }
    m->workload = new_workload;
    if( DO_DEBUG1(m) )
    {
      fprintf(stderr, "  ");
      KheLimitWorkloadMonitorDebug(m, 1, -1, stderr);
      fprintf(stderr, "  after unassign ");
      KheTaskDebug(task, 1, -1, stderr);
      fprintf(stderr, "\n");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLimitWorkloadMonitorDeviation(KHE_LIMIT_WORKLOAD_MONITOR m)       */
/*                                                                           */
/*  Return the deviation of m.                                               */
/*                                                                           */
/*****************************************************************************/

int KheLimitWorkloadMonitorDeviation(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitWorkloadMonitorDeviationDescription(                       */
/*    KHE_LIMIT_WORKLOAD_MONITOR m)                                          */
/*                                                                           */
/*  Return a description of the deviation of m in heap memory.               */
/*                                                                           */
/*****************************************************************************/

char *KheLimitWorkloadMonitorDeviationDescription(
  KHE_LIMIT_WORKLOAD_MONITOR m)
{
  ARRAY_CHAR ac;
  MStringInit(ac);
  if( m->deviation == 0 )
    MStringAddString(ac, "0");
  else if( m->workload < m->minimum )
    MStringPrintf(ac, 100, "%d too little", m->deviation);
  else if( m->workload > m->maximum )
    MStringPrintf(ac, 100, "%d too much", m->deviation);
  /* ***
  else if( m->workload < m->minimum )
    MStringPrintf(ac, 100, "%.2f below min %d", m->workload, m->minimum);
  else if( m->workload > m->maximum )
    MStringPrintf(ac, 100, "%.2f above max %d", m->workload, m->minimum);
  *** */
  return MStringVal(ac);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitWorkloadMonitorDeviationCount(KHE_LIMIT_WORKLOAD_MONITOR m)  */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitWorkloadMonitorDeviationCount(KHE_LIMIT_WORKLOAD_MONITOR m)
{
  return 1;
}
*** */


/*****************************************************************************/
/*                                                                           */
/* int KheLimitWorkloadMonitorDeviation(KHE_LIMIT_WORKLOAD_MONITOR m, int i) */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLimitWorkloadMonitorDeviation(KHE_LIMIT_WORKLOAD_MONITOR m, int i)
{
  MAssert(i == 0, "KheLimitWorkloadMonitorDeviation: i out of range");
  return m->deviation;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitWorkloadMonitorDeviationDescription(                       */
/*    KHE_LIMIT_WORKLOAD_MONITOR m, int i)                                   */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

/* ***
char *KheLimitWorkloadMonitorDeviationDescription(
  KHE_LIMIT_WORKLOAD_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheLimitWorkloadMonitorDeviationDescription: i out of range");
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
/*  void KheLimitWorkloadMonitorDebug(KHE_LIMIT_WORKLOAD_MONITOR m,          */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheLimitWorkloadMonitorDebug(KHE_LIMIT_WORKLOAD_MONITOR m,
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
    fprintf(fp, " (wk %.6f, lim %d..%d%s, dev %d)", m->workload, m->minimum,
      m->maximum, buff, m->deviation);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
