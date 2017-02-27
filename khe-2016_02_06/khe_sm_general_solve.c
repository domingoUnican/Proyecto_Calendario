
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
/*  FILE:         khe_sm_general_solve.c                                     */
/*  DESCRIPTION:  KheGeneralSolve2014().                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0

/*****************************************************************************/
/*                                                                           */
/*  void KheDebugStage(KHE_SOLN soln, char *stage, int verbosity)            */
/*                                                                           */
/*  Print a debug message showing the stage, how much time has been spent    */
/*  so far, and the solution cost.                                           */
/*                                                                           */
/*****************************************************************************/

static void KheDebugStage(KHE_SOLN soln, char *stage, int verbosity)
{
  float secs;
  MAssert(soln != NULL, "KheDebugStage internal error");
  secs = KheSolnTimeNow(soln);
  if( secs > 300.0 )
    fprintf(stderr, "  KheGeneralSolve2014 %s (%.2f mins elapsed)%s\n",
      stage, secs / 60.0, soln != NULL ? ":" : "");
  else
    fprintf(stderr, "  KheGeneralSolve2014 %s (%.2f secs elapsed)%s\n",
      stage, secs, soln != NULL ? ":" : "");
  /* ***
  fprintf(stderr, "  KheGeneralSolve2014 %s (%.2f secs so far)%s\n",
    stage, KheTestTimeOfDayEnd(tv), soln != NULL ? ":" : "");
  *** */
  KheSolnDebug(soln, verbosity, 2, stderr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDebugLostMeet(KHE_SOLN soln, char *lost_meet_id)                 */
/*                                                                           */
/*  Debug print of lost meet.                                                */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheDebugLostMeet(KHE_SOLN soln, char *lost_meet_id)
{
  KHE_INSTANCE ins;  KHE_MEET lost_meet;  KHE_EVENT e;  int i;
  ins = KheSolnInstance(soln);
  if( KheInstanceRetrieveEvent(ins, lost_meet_id, &e) )
  {
    fprintf(stderr, "  lost meet %s:\n", lost_meet_id);
    for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    {
      lost_meet = KheEventMeet(soln, e, i);
      KheMeetDebug(lost_meet, 2, 2, stderr);
      if( KheMeetNode(lost_meet) != NULL )
      {
	fprintf(stderr, "    in node ");
	KheNodeDebug(KheMeetNode(lost_meet), 2, 2, stderr);
      }
    }
  }
  else
    fprintf(stderr, "  did not retrieve lost meet %s:\n", lost_meet_id);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceContainsSoftSplitConstraint(KHE_INSTANCE ins)            */
/*                                                                           */
/*  Return true if ins contains at least one soft split events or            */
/*  distribute split events constraint.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheInstanceContainsSoftSplitConstraint(KHE_INSTANCE ins)
{
  int i;  KHE_CONSTRAINT c;
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    if( (KheConstraintTag(c) == KHE_SPLIT_EVENTS_CONSTRAINT_TAG ||
        KheConstraintTag(c) == KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG)
	&& !KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheGeneralSolve2014(KHE_SOLN soln, KHE_OPTIONS options)         */
/*                                                                           */
/*  Solve soln, assuming that it has just emerged from KheSolnMake.          */
/*                                                                           */
/*****************************************************************************/
/* extern void KheEjectionChainNodeGroupForTimingTest(KHE_NODE parent_node); */

KHE_SOLN KheGeneralSolve2014(KHE_SOLN soln, KHE_OPTIONS options)
{
  int i;  KHE_EVENT junk;  KHE_NODE cycle_node;  KHE_TASKING tasking;
  char buff[200];
  KHE_INSTANCE ins = KheSolnInstance(soln);
  bool monitor_evenness = KheOptionsMonitorEvenness(options);
  bool time_assignment_only = KheOptionsTimeAssignmentOnly(options);
  KHE_TIME_EQUIV time_equiv = KheOptionsStructuralTimeEquiv(options);
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheGeneralSolve2014(%p %s) div %d\n", (void *) soln,
      KheInstanceId(ins) == NULL ? "-" : KheInstanceId(ins),
      KheSolnDiversifier(soln));
    KheDebugStage(soln, "at start", 2);
  }


  /***************************************************************************/
  /*                                                                         */
  /*  structural phase                                                       */
  /*                                                                         */
  /***************************************************************************/

  /* set soft time limit to 5 minutes */
  KheSolnSetTimeLimit(soln, 5 * 60.0);

  /* split initial cycle meet and make complete representation */
  KheSolnSplitCycleMeet(soln);
  if( !KheSolnMakeCompleteRepresentation(soln, &junk) )
    MAssert(false, "KheGeneralSolve2014:  !KheSolnMakeCompleteRepresentation");

  /* build layer tree */
  cycle_node = KheLayerTreeMake(soln);
  if( DEBUG2 )
  {
    fprintf(stderr, "  cycle node after KheLayerTreeMake:\n");
    KheNodeDebug(cycle_node, 4, 2, stderr);
  }
  KheOptionsSetEjectorUseSplitMoves(options,
    KheInstanceContainsSoftSplitConstraint(ins));

  /* attach matching and (if with_evenness) evenness monitors */
  KheSolnMatchingBegin(soln);
  KheSolnMatchingSetWeight(soln, KheCost(1, 0));
  KheSolnMatchingAddAllWorkloadRequirements(soln);
  KheSolnMatchingAttachAllOrdinaryDemandMonitors(soln);
  if( monitor_evenness )
  {
    KheSolnEvennessBegin(soln);
    KheSolnSetAllEvennessMonitorWeights(soln, KheCost(0, 5));
    KheSolnAttachAllEvennessMonitors(soln);
  }
  if( DEBUG1 )
  {
    KheDebugStage(soln, "after installing the matching", 2);
    if( KheSolnMatchingDefectCount(soln) > 0 )
      KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) soln, 2, 2, stderr);
  }

  /* build task tree, including assigning preassigned resources */
  KheOptionsSetResourceInvariant(options, true);
  KheTaskTreeMake(soln, KHE_TASK_JOB_HARD_PRC | KHE_TASK_JOB_HARD_ASAC |
    KHE_TASK_JOB_SOFT_ASAC, options);

  /* find time-equivalent events and resources */
  KheTimeEquivSolve(time_equiv, soln);

  /* coordinate layers and build runarounds */
  KheCoordinateLayers(cycle_node, true);
  if( DEBUG2 )
  {
    fprintf(stderr, "  cycle node after KheCoordinateLayers:\n");
    KheNodeDebug(cycle_node, 4, 2, stderr);
  }
  KheBuildRunarounds(cycle_node, &KheNodeSimpleAssignTimes, options,
    &KheRunaroundNodeAssignTimes, options);
  if( DEBUG2 )
  {
    fprintf(stderr, "  cycle node after KheBuildRunarounds:\n");
    KheNodeDebug(cycle_node, 4, 2, stderr);
  }


  /***************************************************************************/
  /*                                                                         */
  /*  time assignment phase                                                  */
  /*                                                                         */
  /***************************************************************************/

  /* assign to nodes below the cycle node */
  for( i = 0;  i < KheNodeChildCount(cycle_node);  i++ )
    KheNodeRecursiveAssignTimes(KheNodeChild(cycle_node, i),
      &KheRunaroundNodeAssignTimes, options);

  /* ***
  if( DEBUG1 )
    KheDebugStage(soln, "before grouping timing test", 2);
  for( i = 0;  i < 10000;  i++ )
    KheEjectionChainNodeGroupForTimingTest(cycle_node);
  if( DEBUG1 )
    KheDebugStage(soln, "after grouping timing test", 2);
  *** */

  /* assign to the cycle node */
  if( DEBUG1 )
    KheDebugStage(soln, "before time assignment", 2);
  KheCycleNodeAssignTimes(cycle_node, options);
  if( DEBUG1 )
    KheDebugStage(soln, "after time assignment", 2);
  if( time_assignment_only )
  {
    KheSolnSetRunningTime(soln, KheSolnTimeNow(soln));
    if( DEBUG1 )
      fprintf(stderr, "] KheGeneralSolve2014 returning after time asst\n");
    return soln;
  }


  /***************************************************************************/
  /*                                                                         */
  /*  resource assignment phases                                             */
  /*                                                                         */
  /***************************************************************************/

  /* end the inclusion of demand cost in the solution cost */
  KheDisconnectAllDemandMonitors(soln, NULL);

  /* assign taskings (parts 1 and 2) */
  for( i = 0;  i < KheSolnTaskingCount(soln);  i++ )
  {
    tasking = KheSolnTasking(soln, i);
    KheTaskingAssignResourcesStage1(tasking, options);
    KheTaskingAssignResourcesStage2(tasking, options);
    if( DEBUG1 )
    {
      sprintf(buff, "after %s assignment (parts 1 and 2)",
	KheResourceTypeName(KheTaskingResourceType(tasking)));
      KheDebugStage(soln, buff, 2);
    }
  }

  /* assign taskings (part 3) */
  for( i = 0;  i < KheSolnTaskingCount(soln);  i++ )
  {
    tasking = KheSolnTasking(soln, i);
    KheTaskingAssignResourcesStage3(tasking, options);
    if( DEBUG1 )
    {
      sprintf(buff, "after %s assignment (part 3)",
	KheResourceTypeName(KheTaskingResourceType(tasking)));
      KheDebugStage(soln, buff, 2);
    }
  }

  /* try a tree search wherever there are still problems */
  /* ***
  KheTreeSearchRepairTimes(soln, NULL, true);
  *** */


  /***************************************************************************/
  /*                                                                         */
  /*  cleanup phase                                                          */
  /*                                                                         */
  /***************************************************************************/

  /* ensure that the solution cost is the official cost */
  KheSolnEnsureOfficialCost(soln);

  /* merge meets (requires split events monitors, hence this placement) */
  if( DEBUG1 )
    KheDebugStage(soln, "before KheMergeMeets", 2);
  KheMergeMeets(soln);
  if( DEBUG1 )
    KheDebugStage(soln, "after KheMergeMeets", 2);

  /* try task and meet unassignments */
  KheSolnTryTaskUnAssignments(soln);
  KheSolnTryMeetUnAssignments(soln);

  /* wrapup */
  if( DEBUG1 )
  {
    KheDebugStage(soln, "at end", 2);
    fprintf(stderr, "] KheGeneralSolve2014 returning\n");
  }
  /* KheStatsTimerDelete(st); */
  KheSolnSetRunningTime(soln, KheSolnTimeNow(soln));
  return soln;
}
