
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
/*  FILE:         khe_se_solvers.c                                           */
/*  DESCRIPTION:  Ejection chain solvers                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0	/* main calls */
#define DEBUG2 0	/* initial and final defects */
#define DEBUG3 0

#define DEBUG4 0	/* augment calls */
#define DEBUG5 0	/* repairs tried */
#define DEBUG6 0	/* demand repairs */
#define DEBUG7 0	/* cluster repairs */
#define DEBUG8 0	/* complex idle times repairs */
#define DEBUG9 0	/* complex idle times recursion */
#define DEBUG10 0	/* split repairs */

#define DEBUG11 0
#define DEBUG12 0
#define DEBUG13 0
#define DEBUG14 0
#define DEBUG15 0	/* avoid split assignments repairs */
#define DEBUG16 0	/* limit idle meet bound repairs */
#define DEBUG17 0	/* extra limit busy times repairs */

#define MAX_LIT_REPAIRS 1000	/* max number of complex limit idle repairs */

#define WITH_DEMAND_NODE_SWAPS true
#define WITH_SPREAD_EVENTS_NODE_SWAPS false
#define WITH_PREFER_TIMES_NODE_SWAPS false
#define WITH_OLD_AVOID_SPLIT_REPAIR 0
#define WITH_OLD_SPREAD_REPAIR 0

#define WITH_OLD_LIMIT_IDLE_REPAIR 1
#define WITH_NEW_LIMIT_IDLE_REPAIR_OVERLAP 0
#define WITH_NEW_LIMIT_IDLE_REPAIR_EXACT 0

#define WITH_EXTRA_LIMIT_BUSY_TIMES_REPAIRS 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_TIME_GROUP) ARRAY_KHE_TIME_GROUP;


/*****************************************************************************/
/*                                                                           */
/*  KHE_AUGMENT_TYPE - augment types (i.e. defect types)                     */
/*                                                                           */
/*  The order here follows the order of monitor types used in the Guide.     */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_AUGMENT_ORDINARY_DEMAND,
  KHE_AUGMENT_WORKLOAD_DEMAND,
  KHE_AUGMENT_SPLIT_EVENTS,
  KHE_AUGMENT_ASSIGN_TIME,
  KHE_AUGMENT_PREFER_TIMES,
  KHE_AUGMENT_SPREAD_EVENTS,
  KHE_AUGMENT_LINK_EVENTS,
  KHE_AUGMENT_ORDER_EVENTS,
  KHE_AUGMENT_ASSIGN_RESOURCE,
  KHE_AUGMENT_PREFER_RESOURCES,
  KHE_AUGMENT_AVOID_SPLIT_ASSIGNMENTS,
  KHE_AUGMENT_AVOID_CLASHES,
  KHE_AUGMENT_AVOID_UNAVAILABLE_TIMES,
  KHE_AUGMENT_LIMIT_IDLE_TIMES,
  KHE_AUGMENT_CLUSTER_BUSY_TIMES,
  KHE_AUGMENT_LIMIT_BUSY_TIMES,
  KHE_AUGMENT_LIMIT_WORKLOAD 
} KHE_AUGMENT_TYPE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REPAIR_TYPE - repair types (i.e. repair operations)                  */
/*                                                                           */
/*  There doesn't seem to be any natural order for repair types.  These      */
/*  ones appear in the order they appear in the text of my PATAT14 paper.    */
/*                                                                           */
/*****************************************************************************/

typedef enum {

  /* a subset of the 7 operations, applied to meets, in popularity order */
  KHE_REPAIR_KEMPE_MEET_MOVE,
  KHE_REPAIR_EJECTING_MEET_MOVE,
  KHE_REPAIR_BASIC_MEET_MOVE,
  KHE_REPAIR_FUZZY_MEET_MOVE,
  KHE_REPAIR_EJECTING_MEET_ASSIGN,
  KHE_REPAIR_BASIC_MEET_ASSIGN,

  /* a subset of the 7 operations, applied to tasks */
  KHE_REPAIR_EJECTING_TASK_MOVE,
  KHE_REPAIR_EJECTING_TASK_ASSIGN,

  /* combined operations */
  KHE_REPAIR_NODE_SWAP,
  KHE_REPAIR_MEET_SPLIT,
  KHE_REPAIR_SPLIT_MOVE,
  KHE_REPAIR_MERGE_MOVE,
  /* KHE_REPAIR_EJECTING_TASK_SET_MOVE, */
  KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_ASSIGN,
  KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_MOVE,
  KHE_REPAIR_SPLIT_TASKS_UNASSIGN,
  KHE_REPAIR_COMPLEX_IDLE_MOVE,
  KHE_REPAIR_CLUSTER_MEETS_UNASSIGN,
  KHE_REPAIR_LIMIT_BUSY_MEETS_UNASSIGN,
  KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN
} KHE_REPAIR_TYPE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TRY_MEET_MOVE_FN                                                     */
/*                                                                           */
/*  Functions of this type are called to decide whether to try meet moves    */
/*  of meet (or of an ancestor of meet) which give meet this starting time.  */
/*                                                                           */
/*  Parameter impl gives access to other information whose type is known     */
/*  to the function, which it can use in making its decision.                */
/*                                                                           */
/*****************************************************************************/

typedef bool (*KHE_TRY_MEET_MOVE_FN)(KHE_MEET meet, KHE_TIME time, void *impl);


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Control options"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorRepairTimes(KHE_EJECTOR ej)                               */
/*  KHE_NODE KheEjectorLimitNode(KHE_EJECTOR ej)                             */
/*  bool KheEjectorRepairResources(KHE_EJECTOR ej)                           */
/*  bool KheEjectorNodesBeforeMeets(KHE_EJECTOR ej)                          */
/*  bool KheEjectorUseKempeMoves(KHE_EJECTOR ej)                             */
/*  bool KheEjectorEjectingNotBasic(KHE_EJECTOR ej)                          */
/*  bool KheEjectorUseFuzzyMoves(KHE_EJECTOR ej)                             */
/*  bool KheEjectorUseSplitMoves(KHE_EJECTOR ej)                             */
/*                                                                           */
/*  Return the indicated control options.                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorRepairTimes(KHE_EJECTOR ej)
{
  return KheOptionsEjectorRepairTimes(KheEjectorOptions(ej));
}

static KHE_NODE KheEjectorLimitNode(KHE_EJECTOR ej)
{
  return KheOptionsEjectorLimitNode(KheEjectorOptions(ej));
}

static bool KheEjectorRepairResources(KHE_EJECTOR ej)
{
  return KheOptionsEjectorRepairResources(KheEjectorOptions(ej));
}

static bool KheEjectorNodesBeforeMeets(KHE_EJECTOR ej)
{
  return KheOptionsEjectorNodesBeforeMeets(KheEjectorOptions(ej));
}

static KHE_OPTIONS_KEMPE KheEjectorUseKempeMoves(KHE_EJECTOR ej)
{
  return KheOptionsEjectorUseKempeMoves(KheEjectorOptions(ej));
}

static bool KheEjectorEjectingNotBasic(KHE_EJECTOR ej)
{
  return KheOptionsEjectorEjectingNotBasic(KheEjectorOptions(ej));
}

static bool KheEjectorUseFuzzyMoves(KHE_EJECTOR ej)
{
  return KheOptionsEjectorUseFuzzyMoves(KheEjectorOptions(ej));
}

static bool KheEjectorUseSplitMoves(KHE_EJECTOR ej)
{
  return KheOptionsEjectorUseSplitMoves(KheEjectorOptions(ej));
}

static KHE_KEMPE_STATS KheEjectorKempeStats(KHE_EJECTOR ej)
{
  return KheOptionsTimeKempeStats(KheEjectorOptions(ej));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Repair operations"                                            */
/*                                                                           */
/*  The functions in this submodule implement all the repair operations,     */
/*  in the order they are declared in KHE_REPAIR_TYPE.  Each function        */
/*  begins with KheEjectorRepairBegin and ends with (and returns the value   */
/*  returned by) KheEjectorRepairEnd or KheEjectorRepairEndLong.             */
/*                                                                           */
/*  Exception:  KHE_REPAIR_COMPLEX_IDLE_MOVE was too hard to disentangle.    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheBasicMeetAssignOrMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,       */
/*    KHE_MEET target_meet, int offset, bool preserve_regularity)            */
/*                                                                           */
/*  Try a basic meet assignment (KHE_REPAIR_BASIC_MEET_ASSIGN) or a basic    */
/*  meet move (KHE_REPAIR_BASIC_MEET_MOVE) of meet to target_meet at offset. */
/*                                                                           */
/*****************************************************************************/

static bool KheBasicMeetAssignOrMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset, bool preserve_regularity)
{
  bool success, move;  int d;
  /* ***
  MAssert(KheMeetAsst(meet) == NULL,
    "KheBasicMeetAssignRepair internal error");
  *** */
  move = (KheMeetAsst(meet) != NULL);
  KheEjectorRepairBegin(ej);
  success = KheBasicMeetMove(meet, target_meet, offset, preserve_regularity,&d);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheBasicMeetAssignOrMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s)\n", offset,
      preserve_regularity ? "true" : "false");
  }
  return KheEjectorRepairEnd(ej, move ? KHE_REPAIR_BASIC_MEET_MOVE :
    KHE_REPAIR_BASIC_MEET_ASSIGN, success);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheBasicMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,               */
/*    KHE_MEET target_meet, int offset, bool preserve_regularity)            */
/*                                                                           */
/*  Try a basic move (KHE_REPAIR_BASIC_MEET_MOVE) of meet to target_meet     */
/*  at offset.                                                               */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheBasicMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset, bool preserve_regularity)
{
  bool success;  int d;
  MAssert(KheMeetAsst(meet) != NULL, "KheBasicMeetMoveRepair internal error");
  KheEjectorRepairBegin(ej);
  success = KheBasicMeetMove(meet, target_meet, offset, preserve_regularity,&d);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheBasicMeetMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s)\n", offset,
      preserve_regularity ? "true" : "false");
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_BASIC_MEET_MOVE, success);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingMeetAssignOrMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,    */
/*    KHE_MEET target_meet, int offset, bool preserve_regularity,bool *basic)*/
/*                                                                           */
/*  Try an ejecting meet assignment (KHE_REPAIR_EJECTING_MEET_ASSIGN) or     */
/*  an ejecting meet move (KHE_REPAIR_EJECTING_MEET_MOVE) of meet to         */
/*  target_meet at offset.                                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectingMeetAssignOrMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset, bool preserve_regularity, bool *basic)
{
  bool success, move;  int d;
  move = (KheMeetAsst(meet) != NULL);
  KheEjectorRepairBegin(ej);
  success = KheEjectingMeetMove(meet, target_meet, offset, preserve_regularity,
    &d, basic);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheEjectingMeetAssignOrMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s)\n", offset,
      preserve_regularity ? "true" : "false");
  }
  return KheEjectorRepairEnd(ej, move ? KHE_REPAIR_EJECTING_MEET_MOVE :
    KHE_REPAIR_EJECTING_MEET_ASSIGN, success);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,            */
/*    KHE_MEET target_meet, int offset, bool preserve_regularity, bool*basic)*/
/*                                                                           */
/*  Try an ejecting move (KHE_REPAIR_EJECTING_MEET_MOVE) of meet to          */
/*  target_meet at offset.                                                   */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectingMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset, bool preserve_regularity, bool *basic)
{
  bool success;  int d;
  KheEjectorRepairBegin(ej);
  success = KheEjectingMeetMove(meet, target_meet, offset, preserve_regularity,
    &d, basic);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheEjectingMeetMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s)\n", offset,
      preserve_regularity ? "true" : "false");
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_EJECTING_MEET_MOVE, success);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,               */
/*    KHE_MEET target_meet, int offset, bool preserve_regularity,            */
/*    bool *basic, KHE_KEMPE_STATS kempe_stats)                              */
/*                                                                           */
/*  Try a Kempe meet move (KHE_REPAIR_KEMPE_MEET_MOVE) of meet to            */
/*  target_meet at offset.                                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset, bool preserve_regularity,
  bool *basic, KHE_KEMPE_STATS kempe_stats)
{
  bool success;  int d;
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet, target_meet, offset,
    preserve_regularity, &d, basic, kempe_stats);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheKempeMeetMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s)\n", offset,
      preserve_regularity ? "true" : "false");
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_KEMPE_MEET_MOVE, success);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheFuzzyMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,               */
/*    KHE_MEET target_meet, int offset)                                      */
/*                                                                           */
/*  Try a fuzzy move (KHE_REPAIR_FUZZY_MEET_MOVE) of meet to target_meet     */
/*  at offset.                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheFuzzyMeetMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset)
{
  bool success;
  MAssert(KheMeetAsst(meet) != NULL, "KheFuzzyMeetMoveRepair internal error");
  KheEjectorRepairBegin(ej);
  success = KheFuzzyMeetMove(meet, target_meet, offset, 4, 2, 12);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheFuzzyMeetMove(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", offset);
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_FUZZY_MEET_MOVE, success);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingTaskAssignRepair(KHE_EJECTOR ej, KHE_TASK task,          */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Try an ejecting task assignment (KHE_REPAIR_EJECTING_TASK_ASSIGN)        */
/*  of task to r.                                                            */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectingTaskAssignRepair(KHE_EJECTOR ej, KHE_TASK task,
  KHE_RESOURCE r)
{
  bool success;
  MAssert(KheTaskAsst(task) == NULL,
    "KheEjectingTaskAssignRepair internal error");
  KheEjectorRepairBegin(ej);
  success = KheTaskEjectingMoveResource(task, r);  ** can also Assign **
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s%crepair EjectingTaskAssignRepair(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_EJECTING_TASK_ASSIGN, success);
    ** KheTaskDuration(task) **
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingTaskAssignOrMoveRepair(KHE_EJECTOR ej, KHE_TASK task,    */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Try an ejecting task assignment (KHE_REPAIR_EJECTING_TASK_ASSIGN) or an  */
/*  ejecting task move (KHE_REPAIR_EJECTING_TASK_MOVE) of task to r.         */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectingTaskAssignOrMoveRepair(KHE_EJECTOR ej, KHE_TASK task,
  KHE_RESOURCE r)
{
  bool success, move;
  /* allowing both assignments and moves now
  MAssert(KheTaskAsstResource(task) != NULL,
    "KheEjectingTaskMoveRepair internal error");
  *** */
  move = (KheTaskAsstResource(task) != NULL);
  KheEjectorRepairBegin(ej);
  success = KheTaskEjectingMoveResource(task, r);  /* can Assign or Move */
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s%crepair KheEjectingTaskAssignOrMoveRepair(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  return KheEjectorRepairEnd(ej, move ? KHE_REPAIR_EJECTING_TASK_MOVE :
    KHE_REPAIR_EJECTING_TASK_ASSIGN, success);
    /* KheTaskDuration(task) */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingTaskSetMoveRepair(KHE_EJECTOR ej,                        */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE from_r1,        */
/*    KHE_RESOURCE from_r2, KHE_RESOURCE r)                                  */
/*                                                                           */
/*  Try an ejecting task-set move (KHE_REPAIR_EJECTING_TASK_SET_MOVE) to     */
/*  r of those tasks monitored by asam which are currently assigned to       */
/*  from_r1 and from_r2.                                                     */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static bool KheEjectingTaskSetMoveRepair(KHE_EJECTOR ej,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE from_r1,
  KHE_RESOURCE from_r2, KHE_RESOURCE r)
{
  KHE_RESOURCE ar;  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;
  KHE_TASK task;  int egi, count, j, k, durn;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  bool success;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheEjectorSoln(ej);
  KheEjectorRepairBegin(ej);
  success = true;
  durn = 0;
  for( j = 0;  success && j < count;  j++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, j);
    for( k = 0;  success && k < KheEventResourceTaskCount(soln, er);  k++ )
    {
      task = KheEventResourceTask(soln, er, k);
      ar = KheTaskAsstResource(task);
      if( (ar == from_r1 || ar == from_r2) && ar != r )
      {
	success = success && KheTaskEjectingMoveResource(task, r);
	durn += KheTaskDuration(task);
      }
    }
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_EJECTING_TASK_SET_MOVE, success);
    /* durn */
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSwapRepair(KHE_EJECTOR ej, KHE_NODE node1, KHE_NODE node2)   */
/*                                                                           */
/*  Try a node swap (repair KHE_REPAIR_NODE_SWAP) of node1 and node nd2.     */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeSwapRepair(KHE_EJECTOR ej, KHE_NODE node1, KHE_NODE node2)
{
  bool success;
  KheEjectorRepairBegin(ej);
  success = KheNodeMeetSwap(node1, node2);
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s%crepair KheNodeMeetSwap(",
      2 * KheEjectorCurrDepth(ej) + 2, "", success ? '+' : '-');
    KheNodeDebug(node1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheNodeDebug(node2, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  return KheEjectorRepairEnd(ej, KHE_REPAIR_NODE_SWAP, success);
    /* KheNodeDemand(node1) + KheNodeDemand(node2) */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSplitRepair(KHE_EJECTOR ej, KHE_MEET meet, int duration1)    */
/*                                                                           */
/*  Try a meet split (KHE_REPAIR_MEET_SPLIT) of meet.                        */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetSplitRepair(KHE_EJECTOR ej, KHE_MEET meet, int duration1)
{
  bool success;  KHE_MEET meet2;
  KheEjectorRepairBegin(ej);
  success = KheMeetSplit(meet, duration1, true, &meet, &meet2);
  return KheEjectorRepairEnd(ej, KHE_REPAIR_MEET_SPLIT, success);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSplitMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,                   */
/*    KHE_MEET target_meet, int offset)                                      */
/*                                                                           */
/*  Try two split moves (KHE_REPAIR_SPLIT_MOVE) of meet to target_meet       */
/*  at offset.                                                               */
/*                                                                           */
/*  Breaking this into two repairs is still to do.                           */
/*                                                                           */
/*****************************************************************************/

static bool KheSplitMoveRepair(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int offset)
{
  bool success, split_success, basic;  int d, durn1;  KHE_MEET meet2;
  MAssert(KheMeetAsst(meet) != NULL, "KheSplitMoveRepair internal error");
  if( KheMeetDuration(meet) >= 2 )
  {
    /* try moving the first half of the split */
    durn1 = 1 + KheEjectorCurrAugmentCount(ej) % (KheMeetDuration(meet)-1);
    if( DEBUG11 )
    {
      fprintf(stderr, "[ KheSplitMoveRepair splitting ");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, " at %d\n", durn1);
    }
    KheEjectorRepairBegin(ej);
    /* KheMeetDomainUnFix(meet); */
    /* KheMeetSplitUnFix(meet); */
    split_success = KheMeetSplit(meet, durn1, true, &meet, &meet2);
    success = split_success &&
      KheKempeMeetMove(meet, target_meet, offset, false, &d, &basic, NULL);
    if( DEBUG11 )
      fprintf(stderr, "  1 split_success: %s, success %s\n",
	split_success ? "true" : "false", success ? "true" : "false");
    if( KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_MOVE, success) )
    {
      /* KheMeetSplitFix(meet); */
      /* KheMeetDomainFix(meet); */
      if( DEBUG11 )
	fprintf(stderr, "] KheSplitMoveRepair returning true\n");
      return true;
    }

    /* try moving the second half of the split */
    KheEjectorRepairBegin(ej);
    /* KheMeetDomainUnFix(meet); */
    /* KheMeetSplitUnFix(meet); */
    split_success = KheMeetSplit(meet, durn1, true, &meet, &meet2);
    success = split_success &&
      KheKempeMeetMove(meet2, target_meet, offset, false, &d, &basic, NULL);
    if( DEBUG11 )
      fprintf(stderr, "  2 split_success: %s, success %s\n",
	split_success ? "true" : "false", success ? "true" : "false");
    if( KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_MOVE, success) )
    {
      /* KheMeetSplitFix(meet); */
      /* KheMeetDomainFix(meet); */
      if( DEBUG11 )
	fprintf(stderr, "] KheSplitMoveRepair returning true\n");
      return true;
    }
    if( DEBUG11 )
      fprintf(stderr, "] KheSplitMoveRepair returning false\n");
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMergeMoveRepair(KHE_EJECTOR ej, KHE_MEET meet1, KHE_MEET meet2,  */
/*    bool after)                                                            */
/*                                                                           */
/*  Try one merge move (KHE_REPAIR_MERGE_MOVE) which Kempe-moves meet1 to    */
/*  before or after meet2, depending on the after parameter.  Both meets     */
/*  are currently assigned.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheMergeMoveRepair(KHE_EJECTOR ej, KHE_MEET meet1, KHE_MEET meet2,
  bool after)
{
  bool success, b;  int d;
  if( DEBUG10 )
  {
    fprintf(stderr, "[ KheMergeMoveRepair(ej, ");
    KheMeetDebug(meet1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(meet2, 1, -1, stderr);
    fprintf(stderr, ", %s)\n", after ? "after" : "before");
  }
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet1, KheMeetAsst(meet2),
    (after ? KheMeetAsstOffset(meet2) + KheMeetDuration(meet2) :
    KheMeetAsstOffset(meet2) - KheMeetDuration(meet1)), false, &d, &b, NULL)
    && KheMeetMerge(meet1, meet2, true, &meet1);
  success = KheEjectorRepairEnd(ej, KHE_REPAIR_MERGE_MOVE, success);
  if( DEBUG10 )
    fprintf(stderr, "] KheMergeMoveRepair returning %s\n",
      success ? "true" : "false");
  return success;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMergeMoveRepair(KHE_EJECTOR ej, KHE_MEET meet1, KHE_MEET meet2)  */
/*                                                                           */
/*  Try merge moves (KHE_REPAIR_MERGE_MOVE) of meet1 and meet2.  Both        */
/*  meets are currently assigned.                                            */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMergeMoveRepair(KHE_EJECTOR ej, KHE_MEET meet1, KHE_MEET meet2)
{
  bool success, b;  int d;

  ** try the first merge **
  ** durn1 = 1 + KheEjectorCurrAugmentCount(ej) % (KheMeetDuration(meet)-1); **
  if( DEBUG10 )
  {
    fprintf(stderr, "[ KheMergeMoveRepair(ej, ");
    KheMeetDebug(meet1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(meet2, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  ** try a repair that moves meet1 to directly before meet2 and merges them **
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet1, KheMeetAsst(meet2),
    KheMeetAsstOffset(meet2) - KheMeetDuration(meet1), false, &d, &b) &&
    KheMeetMerge(meet1, meet2, true, &meet1);
  if( KheEjectorRepairEnd(ej, KHE_REPAIR_MERGE_MOVE, success) )
  {
    if( DEBUG10 )
      fprintf(stderr, "] KheMergeMoveRepair returning true (a)\n");
    return true;
  }

  ** try a repair that moves meet1 to directly after meet2 and merges them **
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet1, KheMeetAsst(meet2),
    KheMeetAsstOffset(meet2) + KheMeetDuration(meet2), false, &d, &b) &&
    KheMeetMerge(meet1, meet2, true, &meet1);
  if( KheEjectorRepairEnd(ej, KHE_REPAIR_MERGE_MOVE, success) )
  {
    if( DEBUG10 )
      fprintf(stderr, "] KheMergeMoveRepair returning true (b)\n");
    return true;
  }

  ** try a repair that moves meet2 to directly before meet1 and merges them **
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet2, KheMeetAsst(meet1),
    KheMeetAsstOffset(meet1) - KheMeetDuration(meet2), false, &d, &b) &&
    KheMeetMerge(meet2, meet1, true, &meet2);
  if( KheEjectorRepairEnd(ej, KHE_REPAIR_MERGE_MOVE, success) )
  {
    if( DEBUG10 )
      fprintf(stderr, "] KheMergeMoveRepair returning true (c)\n");
    return true;
  }

  ** try a repair that moves meet2 to directly after meet1 and merges them **
  KheEjectorRepairBegin(ej);
  success = KheKempeMeetMove(meet2, KheMeetAsst(meet1),
    KheMeetAsstOffset(meet1) + KheMeetDuration(meet1), false, &d, &b) &&
    KheMeetMerge(meet2, meet1, true, &meet2);
  if( KheEjectorRepairEnd(ej, KHE_REPAIR_MERGE_MOVE, success) )
  {
    if( DEBUG10 )
      fprintf(stderr, "] KheMergeMoveRepair returning true (d)\n");
    return true;
  }

  if( DEBUG10 )
    fprintf(stderr, "] KheMergeMoveRepair returning false\n");
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair(KHE_EJECTOR ej,   */
/*    KHE_MEET meet, KHE_MEET target_meet, int offset, KHE_TASK task,        */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Carry out a combined Kempe meet move and task assign/move augment.       */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair(KHE_EJECTOR ej,
  KHE_MEET meet, KHE_MEET target_meet, int offset, KHE_TASK task,
  KHE_RESOURCE r)
{
  bool success, basic, move;  int d;
  MAssert(KheMeetAsst(meet) != NULL,
    "KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair internal error 1");
  /* allowing assignments now
  MAssert(KheTaskAsst(task) != NULL,
    "KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair internal error 2");
  *** */
  KheEjectorRepairBegin(ej);
  move = (KheTaskAsst(task) != NULL);
  success = (move ? KheTaskUnAssign(task) : true) &&
    KheKempeMeetMove(meet, target_meet, offset, false, &d, &basic, NULL) &&
    KheTaskEjectingMoveResource(task, r);  /* can also Assign */
    /* KheTaskAssignResource(task, r); */
  if( DEBUG5 )
  {
    fprintf(stderr,
      "%*s%crepair KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair(",
      2 * KheEjectorCurrDepth(ej) + 4, "", success ? '+' : '-');
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, ", offset);
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", %s)\n", KheResourceId(r));
  }
  return KheEjectorRepairEnd(ej,
    move ? KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_MOVE :
    KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_ASSIGN, success);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheSplitTasksUnassignResourceGroup(                   */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE r)              */
/*                                                                           */
/*  Return a resource group consisting of the resources currently assigned   */
/*  to the tasks monitored by asam, excluding r.                             */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
#else
static KHE_RESOURCE_GROUP KheSplitTasksUnassignResourceGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE r)
{
  KHE_SOLN soln;  int i;  KHE_RESOURCE r2;
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  KheSolnResourceGroupBegin(soln, KheResourceResourceType(r));
  for( i = 0;  i < KheAvoidSplitAssignmentsMonitorResourceCount(asam);  i++ )
  {
    r2 = KheAvoidSplitAssignmentsMonitorResource(asam, i);
    if( r2 != r )
      KheSolnResourceGroupAddResource(soln, r2);
  }
  return KheSolnResourceGroupEnd(soln);
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitTasksUnassignOnSuccess(void *on_success_val)                */
/*                                                                           */
/*  On-success function used when repairing avoid split assignments defects. */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
#else
static void KheSplitTasksUnassignOnSuccess(void *on_success_val)
{
  KHE_TASK_BOUND tb;  bool success;
  if( DEBUG13 )
    fprintf(stderr, "[ KheSplitTasksUnassignOnSuccess\n");
  tb = (KHE_TASK_BOUND) on_success_val;
  success = KheTaskBoundDelete(tb);
  if( DEBUG13 )
    fprintf(stderr, "] KheSplitTasksUnassignOnSuccess(success = %s)\n",
      success ? "true" : "false");
}
#endif

/* ***
static void KheSplitTasksUnassignOnSuccess(void *on_success_val)
{
  KHE_TASK_BOUND_GROUP tbg;  bool success;
  if( DEBUG13 )
    fprintf(stderr, "[ KheSplitTasksUnassignOnSuccess\n");
  tbg = (KHE_TASK_BOUND_GROUP) on_success_val;
  success = KheTaskBoundGroupDelete(tbg);
  if( DEBUG13 )
    fprintf(stderr, "] KheSplitTasksUnassignOnSuccess(success = %s)\n",
      success ? "true" : "false");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitTasksUnassignRepair(KHE_EJECTOR ej,                         */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE r)              */
/*                                                                           */
/*  Repair which tries unassigning all tasks monitored by asam that are      */
/*  assigned r, and reducing the domains of all tasks monitored by asam      */
/*  to exclude r.                                                            */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
#else
static bool KheSplitTasksUnassignRepair(KHE_EJECTOR ej,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE r)
{
  KHE_SOLN soln;  KHE_RESOURCE_GROUP avoid_splits_rg;  KHE_TASK task;
  int egi, count, i, j;  KHE_TASK_BOUND tb;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  KHE_EVENT_RESOURCE er;
  soln = KheEjectorSoln(ej);
  avoid_splits_rg = KheSplitTasksUnassignResourceGroup(asam, r);
  KheEjectorRepairBegin(ej);
  tb = KheTaskBoundMake(soln, avoid_splits_rg);
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheEventResourceTask(soln, er, j);
      if( KheTaskAsstResource(task) == r && !KheTaskUnAssign(task) )
	return KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, false);
      /* ***
      if( !KheTaskBoundMake(tbg, task, avoid_splits_rg, &tb) )
	return KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, false);
      *** */
      if( !KheTaskAddTaskBound(task, tb) )
	return KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, false);
    }
  }
  return KheEjectorRepairEndLong(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, true,
    INT_MAX, false, &KheSplitTasksUnassignOnSuccess, (void *) tb);
}
#endif

/* ***
static bool KheSplitTasksUnassignRepair(KHE_EJECTOR ej,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam, KHE_RESOURCE r)
{
  KHE_SOLN soln;  KHE_RESOURCE_GROUP avoid_splits_rg;  KHE_TASK task;
  int egi, count, i, j;  KHE_TASK_BOUND_GROUP tbg;  KHE_TASK_BOUND tb;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  KHE_EVENT_RESOURCE er;
  avoid_splits_rg = KheSplitTasksUnassignResourceGroup(asam, r);
  KheEjectorRepairBegin(ej);
  soln = KheEjectorSoln(ej);
  tbg = KheTaskBound GroupMake(soln);
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheEventResourceTask(soln, er, j);
      if( KheTaskAsstResource(task) == r && !KheTaskUnAssign(task) )
	return KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, false);
      if( !KheTaskBoundMake(tbg, task, avoid_splits_rg, &tb) )
	return KheEjectorRepairEnd(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, false);
    }
  }
  return KheEjectorRepairEndLong(ej, KHE_REPAIR_SPLIT_TASKS_UNASSIGN, true,
    INT_MAX, false, &KheSplitTasksUnassignOnSuccess, (void *) tbg);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheClusterMeetsUnassignTimeGroup(                         */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm, KHE_TIME_GROUP tg)                */
/*                                                                           */
/*  Return a time group that is the cycle minus every time group of cbtm     */
/*  that has no meets, and minus tg as well.                                 */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KheClusterMeetsUnassignTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm, KHE_TIME_GROUP tg)
{
  KHE_SOLN soln;  int i, tgm_count;  KHE_TIME_GROUP_MONITOR tgm;
  soln = KheMonitorSoln((KHE_MONITOR) cbtm);
  KheSolnTimeGroupBegin(soln);
  KheSolnTimeGroupUnion(soln, KheInstanceFullTimeGroup(KheSolnInstance(soln)));
  tgm_count = KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
  for( i = 0;  i < tgm_count;  i++ )
  {
    tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, i);
    if( KheTimeGroupMonitorBusyCount(tgm) == 0 )
      KheSolnTimeGroupDifference(soln, KheTimeGroupMonitorTimeGroup(tgm));
  }
  KheSolnTimeGroupDifference(soln, tg);
  return KheSolnTimeGroupEnd(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnComplementTimeGroup(KHE_SOLN soln,                 */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Return a time group that is the cycle minus tg.                          */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KheSolnComplementTimeGroup(KHE_SOLN soln,
  KHE_TIME_GROUP tg)
{
  KheSolnTimeGroupBegin(soln);
  KheSolnTimeGroupUnion(soln, KheInstanceFullTimeGroup(KheSolnInstance(soln)));
  KheSolnTimeGroupDifference(soln, tg);
  return KheSolnTimeGroupEnd(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterMeetsUnassignOnSuccess(void *on_success_val)              */
/*                                                                           */
/*  On-success function used when repairing cluster busy times defects.      */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheClusterMeetsUnassignOnSuccess(void *on_success_val)
{
  KHE_MEET_BOUND_GROUP mbg;  bool success;
  if( DEBUG13 )
    fprintf(stderr, "[ KheClusterMeetsUnassignOnSuccess\n");
  mbg = (KHE_MEET_BOUND_GROUP) on_success_val;
  success = KheMeetBoundGroupDelete(mbg);
  if( DEBUG13 )
    fprintf(stderr, "] KheClusterMeetsUnassignOnSuccess(success = %s)\n",
      success ? "true" : "false");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetBoundOnSuccess(void *on_success_val)                         */
/*                                                                           */
/*  On-success function used by meet bound repairs.                          */
/*                                                                           */
/*****************************************************************************/

static void KheMeetBoundOnSuccess(void *on_success_val)
{
  KHE_MEET_BOUND mb;  bool success;
  if( DEBUG13 )
    fprintf(stderr, "[ KheMeetBoundOnSuccess\n");
  mb = (KHE_MEET_BOUND) on_success_val;
  success = KheMeetBoundDelete(mb);
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBoundOnSuccess(success = %s)\n",
      success ? "true" : "false");
}

/* ***
static void KheMeetBoundOnSuccess(void *on_success_val)
{
  KHE_MEET_BOUND_GROUP mbg;  bool success;
  if( DEBUG13 )
    fprintf(stderr, "[ KheMeetBoundOnSuccess\n");
  mbg = (KHE_MEET_BOUND_GROUP) on_success_val;
  success = KheMeetBoundGroupDelete(mbg);
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBoundOnSuccess(success = %s)\n",
      success ? "true" : "false");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBoundRepair(KHE_EJECTOR ej, KHE_RESOURCE r,                  */
/*    KHE_TIME_GROUP include_tg, int repair_type)                            */
/*                                                                           */
/*  Reduce the domains of the meets currently assigned r, to contain a       */
/*  subset of the times of include_tg.  Meets currently assigned outside     */
/*  include_tg are unassigned first.  Return true if all successful.         */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetBoundRepair(KHE_EJECTOR ej, KHE_RESOURCE r,
  KHE_TIME_GROUP include_tg, int repair_type)
{
  int i, junk, durn;  KHE_TASK task;  KHE_TIME time;  KHE_MEET meet;
  KHE_MEET_BOUND mb;  KHE_SOLN soln;  bool success;
  if( DEBUG13 )
  {
    fprintf(stderr, "[ KheMeetBoundRepair(ej, %s, ", KheResourceId(r));
    KheTimeGroupDebug(include_tg, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", repair_type);
  }

  /* unassign r's meets outside include_tg; add time bounds to all r's meets */
  soln = KheEjectorSoln(ej);
  KheEjectorRepairBegin(ej);
  mb = KheMeetBoundMake(soln, true, include_tg);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    meet = KheMeetFirstMovable(KheTaskMeet(task), &junk);
    if( meet == NULL )
    {
      if( DEBUG13 )
	fprintf(stderr, "] KheMeetBoundRepair returning false (no unfixed)");
      return KheEjectorRepairEnd(ej, repair_type, false);
    }
    durn = KheMeetDuration(meet);
    time = KheMeetAsstTime(meet);
    if( time != NULL && KheTimeGroupOverlap(include_tg, time, durn) != durn )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, " unassigning ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      if( !KheMeetUnAssign(meet) )
      {
	if( DEBUG13 )
	{
	  fprintf(stderr, "] KheMeetBoundRepair returning false");
	  fprintf(stderr, " cannot unassign ");
	  KheMeetDebug(meet, 1, 0, stderr);
	}
	return KheEjectorRepairEnd(ej, repair_type, false);
      }
    }
    /* if( !KheMeetBoundMake(mbg, meet, KHE_ANY_DURATION, include_tg, &mb) ) */
    if( !KheMeetAddMeetBound(meet, mb) )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, "] KheMeetBoundRepair returning false");
	fprintf(stderr, " cannot bound ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      return KheEjectorRepairEnd(ej, repair_type, false);
    }
  }
  success = KheEjectorRepairEndLong(ej, repair_type, true, INT_MAX, false,
    &KheMeetBoundOnSuccess, (void *) mb);
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBoundRepair returning %s\n",
      success ? "true" : "False");
  return success;
}

/* ***
static bool KheMeetBoundRepair(KHE_EJECTOR ej, KHE_RESOURCE r,
  KHE_TIME_GROUP include_tg, int repair_type)
{
  int i, junk;  KHE_TASK task;  KHE_TIME time;  KHE_MEET meet;
  KHE_MEET_BOUND_GROUP mbg;  KHE_MEET_BOUND mb;  KHE_SOLN soln;
  if( DEBUG13 )
    fprintf(stderr, "[ KheMeetBoundRepair(ej, %s, %s, %d)\n",
      KheResourceId(r), KheTimeGroupId(include_tg), repair_type);

  ** unassign r's meets outside include_tg; add time bounds to all r's meets **
  soln = KheEjectorSoln(ej);
  KheEjectorRepairBegin(ej);
  mbg = KheMeetBoundGroupMake(soln);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    meet = KheMeetFi rstUnFixed(KheTaskMeet(task), &junk);
    time = KheMeetAsstTime(meet);
    if( time != NULL && !KheTimeGroupContains(include_tg, time) )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, " unassigning ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      if( !KheMeetUnAssign(meet) )
      {
	if( DEBUG13 )
	{
	  fprintf(stderr, "] KheMeetBoundRepair returning false");
	  fprintf(stderr, " cannot unassign ");
	  KheMeetDebug(meet, 1, 0, stderr);
	}
	return KheEjectorRepairEnd(ej, repair_type, false);
      }
    }
    if( !KheMeetBoundMake(mbg, meet, KHE_ANY_DURATION, include_tg, &mb) )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, "] KheMeetBoundRepair returning false");
	fprintf(stderr, " cannot include ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      return KheEjectorRepairEnd(ej, repair_type, false);
    }
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBoundRepair returning true\n");
  return KheEjectorRepairEndLong(ej, repair_type, true, INT_MAX, false,
    &KheMeetBoundOnSuccess, (void *) mbg);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterMeetsUnassignRepair(KHE_EJECTOR ej,                       */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm, KHE_TIME_GROUP tg)                */
/*                                                                           */
/*  Reduce the domains of the meets currently assigned r, so that they       */
/*  contain none of the times of tg, or of other time groups of cbtm that    */
/*  have no meets.  Any meets currently assigned in tg are unassigned first. */
/*                                                                           */
/*****************************************************************************/

/* *** inlined now
static bool KheClusterMeetsUnassignRepair(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm, KHE_TIME_GROUP tg)
{
  KHE_TIME_GROUP include_tg;
  include_tg = KheClusterMeetsUnassignTimeGroup(cbtm, tg);
  return KheMeetBoundRepair(ej, KheClusterBusyTimesMonitorResource(cbtm),
    include_tg, KHE_REPAIR_CLUSTER_MEETS_UNASSIGN);
}
*** */

/* *** old version before KheMeetBoundRepair 
static bool KheClusterMeetsUnassignRepair(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm, KHE_TIME_GROUP tg)
{
  int i, junk;  KHE_TASK task;  KHE_TIME time;  KHE_TIME_GROUP cluster_tg;
  KHE_MEET meet;  KHE_MEET_BOUND_GROUP mbg;  KHE_MEET_BOUND mb;  KHE_SOLN soln;
  KHE_RESOURCE r;
  if( DEBUG13 )
    fprintf(stderr, "[ KheResourceMeetsReduceDomains(%s, %s, soln)\n",
      KheResourceId(r), KheTimeGroupId(tg));

  ** make a time group which excludes empty days and tg **
  r = KheClusterBusyTimesMonitorResource(cbtm);
  soln = KheEjectorSoln(ej);
  cluster_tg = KheClusterMeetsUnassignTimeGroup(cbtm, tg);
  if( DEBUG13 )
  {
    fprintf(stderr, "  cluster_tg = ");
    KheTimeGroupDebug(cluster_tg, 1, 0, stderr);
  }

  ** unassign r's meets in tg and add time bounds to all r's meets **
  KheEjectorRepairBegin(ej);
  mbg = KheMeetBoundGroupMake(soln);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    meet = KheMeetFir stUnFixed(KheTaskMeet(task), &junk);
    time = KheMeetAsstTime(meet);
    if( time != NULL && KheTimeGroupContains(tg, time) )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, " unassigning ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      if( !KheMeetUnAssign(meet) )
      {
	if( DEBUG13 )
	{
	  fprintf(stderr, "] KheResourceMeetsReduceDomains returning false");
	  fprintf(stderr, " cannot unassign ");
	  KheMeetDebug(meet, 1, 0, stderr);
	}
	return KheEjectorRepairEnd(ej, KHE_REPAIR_CLUSTER_MEETS_UNASSIGN,false);
      }
    }
    if( !KheMeetBoundMake(mbg, meet, KHE_ANY_DURATION, cluster_tg, &mb) )
    {
      if( DEBUG13 )
      {
	fprintf(stderr, "] KheResourceMeetsReduceDomains returning false");
	fprintf(stderr, " cannot exclude tg from ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      return KheEjectorRepairEnd(ej, KHE_REPAIR_CLUSTER_MEETS_UNASSIGN, false);
    }
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheResourceMeetsReduceDomains returning true\n");
  return KheEjectorRepairEndLong(ej, KHE_REPAIR_CLUSTER_MEETS_UNASSIGN, true,
    INT_MAX, false, &KheClusterMeetsUnassignOnSuccess, (void *) mbg);
}
*** */


/* *** old version from before time bounds
static bool KheResourceMeetsReduceDomains(KHE_RESOURCE r, KHE_TIME_GROUP tg,
  KHE_SOLN soln)
{
  int i, junk;  KHE_TASK task;  KHE_TIME time;  KHE_MEET meet;
  if( DEBUG13 )
    fprintf(stderr, "[ KheResourceMeetsReduceDomains(%s, %s, soln)\n",
      KheResourceId(r), KheTimeGroupId(tg));
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    meet = KheMeetFirs tUnFixed(KheTaskMeet(task), &junk);
    time = KheMeetAsstTime(meet);
    if( time != NULL && KheTimeGroupContains(tg, time) )
    {
      if( !KheMeetUnAssign(meet) )
      {
	if( DEBUG13 )
	{
	  fprintf(stderr, "] KheResourceMeetsReduceDomains returning false");
	  fprintf(stderr, " cannot unassign ");
	  KheMeetDebug(meet, 1, 0, stderr);
	}
	return false;
      }
    }
    if( !KheMeetExcludeDomain(meet, tg) )
    {
      fprintf(stderr, "] KheResourceMeetsReduceDomains returning false");
      fprintf(stderr, " cannot exclude tg from ");
      KheMeetDebug(meet, 1, 0, stderr);
      return false;
    }
  }
  fprintf(stderr, "] KheResourceMeetsReduceDomains returning true\n");
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "useful sequences of repair operations"                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSwapToSimilarNodeAugment(KHE_EJECTOR ej, KHE_NODE node)      */
/*                                                                           */
/*  Try swapping node with other nodes that it shares a layer with.          */
/*  The node is known to be not visited.                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeSwapToSimilarNodeAugment(KHE_EJECTOR ej, KHE_NODE node)
{
  KHE_LAYER layer;  KHE_NODE node2;  int i, index, extra;
  if( KheNodeParentLayerCount(node) > 0 )
  {
    layer = KheNodeParentLayer(node, 0);
    extra = KheEjectorCurrAugmentCount(ej);
    for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    {
      index = (extra + i) % KheLayerChildNodeCount(layer);
      node2 = KheLayerChildNode(layer, index);
      if( node2 != node && KheNodeSameParentLayers(node, node2) &&
	  KheNodeSwapRepair(ej, node, node2) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeIsVizier(KHE_NODE node)                                      */
/*                                                                           */
/*  Return true if node is a vizier node.  At present we are deciding        */
/*  this by seeing whether it has a parent and is its parent's sole child.   */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeIsVizier(KHE_NODE node)
{
  KHE_NODE parent_node;
  parent_node = KheNodeParent(node);
  return parent_node != NULL && KheNodeChildCount(parent_node) == 1;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeIsSuitable(KHE_NODE node, bool try_vizier,                   */
/*    KHE_NODE limit_node)                                                   */
/*                                                                           */
/*  Return true if it is acceptable to change the assignments of the         */
/*  meets of node.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeIsSuitable(KHE_NODE node, bool try_vizier,
  KHE_NODE limit_node)
{
  /* not acceptable if there is no parent node */
  if( KheNodeParent(node) == NULL )
    return false;

  /* not acceptable if not trying viziers and this is a vizier */
  if( !try_vizier && KheNodeIsVizier(node) )
    return false;

  /* not acceptable if there is a node limit and we have reached it */
  if( node == limit_node )
    return false;

  /* otherwise OK */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTryKempe(KHE_MEET meet, KHE_OPTIONS_KEMPE kempe)                 */
/*                                                                           */
/*  Decide whether to try Kempe moves on meet.                               */
/*                                                                           */
/*****************************************************************************/

static bool KheTryKempe(KHE_MEET meet, KHE_OPTIONS_KEMPE kempe)
{
  int i, layer_durn, max_layer_durn, cycle_durn;  KHE_NODE node;
  switch( kempe )
  {
    case KHE_OPTIONS_KEMPE_NO:

      return false;

    case KHE_OPTIONS_KEMPE_AUTO:

      node = KheMeetNode(meet);
      if( node != NULL && KheNodeParentLayerCount(node) > 0 )
      {
	max_layer_durn = 0;
	for( i = 0;  i < KheNodeParentLayerCount(node);  i++ )
	{
	  layer_durn = KheLayerDuration(KheNodeParentLayer(node, i));
	  if( layer_durn > max_layer_durn )
	    max_layer_durn = layer_durn;
	}
	cycle_durn = KheInstanceTimeCount(KheSolnInstance(KheMeetSoln(meet)));
	return 10 * max_layer_durn >= 8 * cycle_durn;
      }
      else
	return true;
      break;

    case KHE_OPTIONS_KEMPE_YES:

      return true;

    default:

      MAssert(false, "KheTryKempe internal error");
      return false;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeAugment(KHE_EJECTOR ej, KHE_MEET meet,                       */
/*    bool try_vizier, KHE_TRY_MEET_MOVE_FN try_meet_move_fn, void *impl,    */
/*    KHE_OPTIONS_KEMPE try_kempe, bool try_ejecting, bool try_basic,        */
/*    bool try_node_swaps)                                                   */
/*                                                                           */
/*  This wonderfully general time repair function tries to change the        */
/*  assignment of meet, or one of its ancestors, using meet assignments      */
/*  and moves of various kinds, and node swaps.  The other parameters        */
/*  offer detailed control over exactly what is tried, as follows:           */
/*                                                                           */
/*  try_vizier                                                               */
/*    If true, try assignments and moves of meets in vizier nodes as well    */
/*    as in other nodes.                                                     */
/*                                                                           */
/*  try_meet_move_fn                                                         */
/*    If this function returns true, try meet assignments and moves on       */
/*    meet, and on meet's proper ancestors if ancestors is true.   Exactly   */
/*    which meet assignments and moves depends on the next 3 parameters.     */
/*    If the function is NULL, no time is needed and we try everything.      */
/*                                                                           */
/*  try_kempe                                                                */
/*    If KHE_OPTIONS_KEMPE_NO, don't try Kempe moves.                        */
/*    If KHE_OPTIONS_KEMPE_AUTO, try Kempe moves when the meet lies in a     */
/*    layer of relatively large duration.                                    */
/*    If KHE_OPTIONS_KEMPE_YES, try Kempe moves.                             */
/*                                                                           */
/*  try_ejecting                                                             */
/*    If true, try ejecting assignments or moves (depending on whether meet  */
/*    is unassigned or assigned) when try_meet_move_fn returns true.         */
/*                                                                           */
/*  try_basic                                                                */
/*    If true, try basic assignments or moves (depending on whether meet     */
/*    is unassigned or assigned) when try_meet_move_fn returns true.         */
/*                                                                           */
/*  try_node_swaps                                                           */
/*    If true, try node swaps on meet's node (if any), and on meet's         */
/*    proper ancestors' nodes (if any) if ancestors is true.                 */
/*                                                                           */
/*  Kempe meet moves are always tried before basic and ejecting moves,       */
/*  and basic and ejecting moves are skipped if a corresponding Kempe        */
/*  move was tried and indicated (via its "basic" parameter) that it         */
/*  did what a basic or ejecting move would do.                              */
/*                                                                           */
/*  This function also consults the time_limit_node option to ensure         */
/*  that any meets whose assignments it changes lie within the limit.        */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeAugment(KHE_EJECTOR ej, KHE_MEET meet,
  bool try_vizier, KHE_TRY_MEET_MOVE_FN try_meet_move_fn, void *impl,
  KHE_OPTIONS_KEMPE try_kempe, bool try_ejecting, bool try_basic,
  bool try_node_swaps)
{
  int base, i, max_offset, offset, x_offset, index, extra;
  KHE_MEET anc_meet, target_meet;
  KHE_NODE node, limit_node, parent_node;  KHE_TIME time;
  bool basic, try_meet_moves;
  bool nodes_before_meets = KheEjectorNodesBeforeMeets(ej);
  bool try_fuzzy_moves = KheEjectorUseFuzzyMoves(ej);
  bool try_split_moves = KheEjectorUseSplitMoves(ej);
  KHE_KEMPE_STATS kempe_stats = KheEjectorKempeStats(ej);
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheTimeAugment(", 2 * KheEjectorCurrDepth(ej)+2, "");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* follow the chain upwards to the first ancestor lying in a node */
  base = 0;
  anc_meet = meet;
  while( anc_meet != NULL && KheMeetNode(anc_meet) == NULL )
  {
    base += KheMeetAsstOffset(anc_meet);
    anc_meet = KheMeetAsst(anc_meet);
  }
  if( anc_meet == NULL )
  {
    if( DEBUG4 )
      fprintf(stderr,"%*s] KheTimeAugment ret false (no movable ancestor)\n",
	2 * KheEjectorCurrDepth(ej) + 2, "");
    return false;
  }

  /* make sure anc_meet's node is in scope */
  node = KheMeetNode(anc_meet);
  limit_node = KheEjectorLimitNode(ej);
  if( limit_node != NULL && !KheNodeIsProperDescendant(node, limit_node) )
  {
    if( DEBUG4 )
      fprintf(stderr,"%*s] KheTimeAugment ret false (not in node scope)\n",
	2 * KheEjectorCurrDepth(ej) + 2, "");
    return false;
  }

  /* try repairs at each ancestor of meet lying in a suitable node */
  try_meet_moves = (try_kempe != KHE_OPTIONS_KEMPE_NO) ||
    try_ejecting || try_basic;
  while( anc_meet != NULL )
  {
    node = KheMeetNode(anc_meet);
    if( node != NULL && KheNodeIsSuitable(node, try_vizier, limit_node) )
    {
      /* if nodes before meets, try node swaps of anc_meet's node */
      if( nodes_before_meets && try_node_swaps && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KheTimeAugment ret true (node)\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return true;
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheNodeUnVisit(node);
      }

      /* try meet moves of anc_meet */
      if( try_meet_moves && !KheMeetVisited(anc_meet, 0) )
      {
	KheMeetVisit(anc_meet);
	parent_node = KheNodeParent(node);
	extra = KheEjectorCurrAugmentCount(ej) +
	  (KheOptionsDiversify(KheEjectorOptions(ej)) ?
	  5 * KheSolnDiversifier(KheEjectorSoln(ej)) : 0);
	for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
	{
	  index = (i + extra) % KheNodeMeetCount(parent_node);
	  target_meet = KheNodeMeet(parent_node, index);
	  time = KheMeetAsstTime(target_meet);
	  if( try_meet_move_fn == NULL || time != NULL )
	  {
	    max_offset = KheMeetDuration(target_meet)-KheMeetDuration(anc_meet);
	    for( offset = 0;  offset <= max_offset;  offset++ )
	    {
	      x_offset = (offset + extra) % (max_offset + 1);
	      if( try_meet_move_fn == NULL || try_meet_move_fn(meet,
		    KheTimeNeighbour(time, base + x_offset), impl) )
	      {
		if( KheMeetAsst(anc_meet) == NULL )
		{
		  /* meet assignments (not moves) are allowed; try ejecting */
		  basic = false;
		  if( try_ejecting && KheEjectingMeetAssignOrMoveRepair(ej,
		      anc_meet, target_meet, x_offset, true, &basic) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (ejecting asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  /* try basic, unless already tried by ejecting */
		  if( !basic && try_basic && KheBasicMeetAssignOrMoveRepair(ej,
		      anc_meet, target_meet, x_offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (basic asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
		else
		{
		  /* meet moves (not assignments) are allowed; try kempe */
		  basic = false;
		  if( KheTryKempe(anc_meet, try_kempe) &&
		      KheKempeMeetMoveRepair(ej, anc_meet, target_meet,
			x_offset, true, &basic, kempe_stats) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (kempe)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  /* try ejecting, unless already tried by kempe */
		  /* ***
		  if( KheEjectorCurrDepth(ej) == 1 )
		    KheSolnNewGlobalVisit(KheMeetSoln(meet));
		  *** */
		  if( !basic && try_ejecting &&
		      KheEjectingMeetAssignOrMoveRepair(ej, anc_meet,
			target_meet, x_offset, true, &basic) )
		  {
		    if( DEBUG4 )
		     fprintf(stderr,"%*s] KheTimeAugment ret true (ejecting)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  /* try basic, unless already tried by kempe or ejecting */
		  if( !basic && try_basic && KheBasicMeetAssignOrMoveRepair(ej,
		      anc_meet, target_meet, x_offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (basic)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  /* try fuzzy, if allowed and depth is 1 */
		  if( try_fuzzy_moves && KheEjectorCurrDepth(ej) == 1 &&
		      KheFuzzyMeetMoveRepair(ej, meet, target_meet, x_offset) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (fuzzy)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  /* try split, if allowed and depth is 1 */
		  if( try_split_moves && KheEjectorCurrDepth(ej) == 1 &&
		      KheSplitMoveRepair(ej, meet, target_meet, x_offset) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (split)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
	      }
	    }
	  }
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheMeetUnVisit(anc_meet);
      }

      /* if nodes after meets, try node swaps of anc_meet's node */
      if( !nodes_before_meets && try_node_swaps && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KheTimeAugment ret true (node)\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return true;
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheNodeUnVisit(node);
      }
    }
    base += KheMeetAsstOffset(anc_meet);
    anc_meet = KheMeetAsst(anc_meet);
  } 
  if( DEBUG4 )
    fprintf(stderr,"%*s] KheTimeAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeDoubleAugment(KHE_EJECTOR ej, KHE_MEET meet,                 */
/*    bool try_vizier, KHE_TRY_MEET_MOVE_FN try_meet_move_fn1,               */
/*    KHE_TRY_MEET_MOVE_FN try_meet_move_fn2, void *impl, bool try_kempe,    */
/*    bool try_ejecting, bool try_basic, bool try_node_swaps)                */
/*                                                                           */
/*  Like KheTimeAugment except that the list of possible target meets is     */
/*  traversed twice, the first them testing with try_meet_move_fn1, the      */
/*  second time with try_meet_move_fn2.                                      */
/*                                                                           */
/*  try_vizier                                                               */
/*    If true, try assignments and moves of meets in vizier nodes as well    */
/*    as in other nodes.                                                     */
/*                                                                           */
/*  try_meet_move_fn                                                         */
/*    If this function returns true, try meet assignments and moves on       */
/*    meet, and on meet's proper ancestors if ancestors is true.   Exactly   */
/*    which meet assignments and moves depends on the next 3 parameters.     */
/*    If the function is NULL, no time is needed and we try everything.      */
/*                                                                           */
/*  try_kempe                                                                */
/*    If true, try Kempe moves when try_meet_move_fn returns true.  The      */
/*    meet must be currently assigned (there are no Kempe assignments).      */
/*                                                                           */
/*  try_ejecting                                                             */
/*    If true, try ejecting assignments or moves (depending on whether meet  */
/*    is unassigned or assigned) when try_meet_move_fn returns true.         */
/*                                                                           */
/*  try_basic                                                                */
/*    If true, try basic assignments or moves (depending on whether meet     */
/*    is unassigned or assigned) when try_meet_move_fn returns true.         */
/*                                                                           */
/*  try_node_swaps                                                           */
/*    If true, try node swaps on meet's node (if any), and on meet's         */
/*    proper ancestors' nodes (if any) if ancestors is true.                 */
/*                                                                           */
/*  Kempe meet moves are always tried before basic and ejecting moves,       */
/*  and basic and ejecting moves are skipped if a corresponding Kempe        */
/*  move was tried and indicated (via its "basic" parameter) that it         */
/*  did what a basic or ejecting move would do.                              */
/*                                                                           */
/*  This function also consults the time_limit_node option to ensure         */
/*  that any meets whose assignments it changes lie within the limit.        */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheTimeDoubleAugment(KHE_EJECTOR ej, KHE_MEET meet,
  bool try_vizier, KHE_TRY_MEET_MOVE_FN try_meet_move_fn1,
  KHE_TRY_MEET_MOVE_FN try_meet_move_fn2, void *impl, bool try_kempe,
  bool try_ejecting, bool try_basic, bool try_node_swaps)
{
  int base, i, max_offset, offset, index;  KHE_MEET anc_meet, target_meet;
  KHE_NODE node, limit_node, parent_node;  KHE_TIME time;
  bool basic, try_meet_moves;
  bool nodes_before_meets =
    KheOptionsEjectorNodesBeforeMeets(KheEjectorOptions(ej));
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheTimeAugment(", 2 * KheEjectorCurrDepth(ej)+2, "");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  ** follow the chain upwards to the first ancestor lying in a node **
  base = 0;
  anc_meet = meet;
  while( anc_meet != NULL && KheMeetNode(anc_meet) == NULL )
  {
    base += KheMeetAsstOffset(anc_meet);
    anc_meet = KheMeetAsst(anc_meet);
  }
  if( anc_meet == NULL )
  {
    if( DEBUG4 )
      fprintf(stderr,"%*s] KheTimeAugment ret false (no movable ancestor)\n",
	2 * KheEjectorCurrDepth(ej) + 2, "");
    return false;
  }

  ** make sure anc_meet's node is in scope **
  node = KheMeetNode(anc_meet);
  limit_node = KheEjectorLimitNode(ej);
  if( limit_node != NULL && !KheNodeIsProperDescendant(node, limit_node) )
  {
    if( DEBUG4 )
      fprintf(stderr,"%*s] KheTimeAugment ret false (not in node scope)\n",
	2 * KheEjectorCurrDepth(ej) + 2, "");
    return false;
  }

  ** try repairs at each ancestor of meet lying in a suitable node **
  try_meet_moves = try_kempe || try_ejecting || try_basic;
  while( anc_meet != NULL )
  {
    node = KheMeetNode(anc_meet);
    if( node != NULL && KheNodeIsSuitable(node, try_vizier, limit_node) )
    {
      ** if nodes before meets, try node swaps of anc_meet's node **
      if( nodes_before_meets && try_node_swaps && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KheTimeAugment ret true (node)\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return true;
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheNodeUnVisit(node);
      }

      ** try meet moves of anc_meet **
      if( try_meet_moves && !KheMeetVisited(anc_meet, 0) )
      {
	KheMeetVisit(anc_meet);
	parent_node = KheNodeParent(node);
	for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
	{
	  index = (KheEjectorCurrAugmentCount(ej) + i) %
            KheNodeMeetCount(parent_node);
	  target_meet = KheNodeMeet(parent_node, index);
	  time = KheMeetAsstTime(target_meet);
	  if( try_meet_move_fn1 == NULL || time != NULL )
	  {
	    max_offset = KheMeetDuration(target_meet)-KheMeetDuration(anc_meet);
	    for( offset = 0;  offset <= max_offset;  offset++ )
	    {
	      if( try_meet_move_fn1 == NULL || try_meet_move_fn1(meet,
		    KheTimeNeighbour(time, base + offset), impl) )
	      {
		if( KheMeetAsst(anc_meet) == NULL )
		{
		  ** meet assignments (not moves) are allowed; try ejecting **
		  basic = false;
		  if( try_ejecting && KheEjectingMeetAssignRepair(ej,
		      anc_meet, target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (ejecting asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try basic, unless already tried by ejecting **
		  if( !basic && try_basic && KheBasicMeetAssignRepair(ej,
		      anc_meet, target_meet, offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (basic asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
		else
		{
		  ** meet moves (not assignments) are allowed; try kempe **
		  basic = false;
		  if( try_kempe && KheKempeMeetMoveRepair(ej, anc_meet,
			target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (kempe)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try ejecting, unless already tried by kempe **
		  if( !basic && try_ejecting && KheEjectingMeetMoveRepair(ej,
		      anc_meet, target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		     fprintf(stderr,"%*s] KheTimeAugment ret true (ejecting)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try basic, unless already tried by kempe or ejecting **
		  if( !basic && try_basic && KheBasicMeetMoveRepair(ej,
		      anc_meet, target_meet, offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (basic)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
	      }
	    }
	  }
	}
	for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
	{
	  index = (KheEjectorCurrAugmentCount(ej) + i) %
            KheNodeMeetCount(parent_node);
	  target_meet = KheNodeMeet(parent_node, index);
	  time = KheMeetAsstTime(target_meet);
	  if( try_meet_move_fn2 == NULL || time != NULL )
	  {
	    max_offset = KheMeetDuration(target_meet)-KheMeetDuration(anc_meet);
	    for( offset = 0;  offset <= max_offset;  offset++ )
	    {
	      if( try_meet_move_fn2 == NULL || try_meet_move_fn2(meet,
		    KheTimeNeighbour(time, base + offset), impl) )
	      {
		if( KheMeetAsst(anc_meet) == NULL )
		{
		  ** meet assignments (not moves) are allowed; try ejecting **
		  basic = false;
		  if( try_ejecting && KheEjectingMeetAssignRepair(ej,
		      anc_meet, target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (ejecting asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try basic, unless already tried by ejecting **
		  if( !basic && try_basic && KheBasicMeetAssignRepair(ej,
		      anc_meet, target_meet, offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,
			"%*s] KheTimeAugment ret true (basic asst)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
		else
		{
		  ** meet moves (not assignments) are allowed; try kempe **
		  basic = false;
		  if( try_kempe && KheKempeMeetMoveRepair(ej, anc_meet,
			target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (kempe)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try ejecting, unless already tried by kempe **
		  if( !basic && try_ejecting && KheEjectingMeetMoveRepair(ej,
		      anc_meet, target_meet, offset, true, &basic) )
		  {
		    if( DEBUG4 )
		     fprintf(stderr,"%*s] KheTimeAugment ret true (ejecting)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }

		  ** try basic, unless already tried by kempe or ejecting **
		  if( !basic && try_basic && KheBasicMeetMoveRepair(ej,
		      anc_meet, target_meet, offset, true) )
		  {
		    if( DEBUG4 )
		      fprintf(stderr,"%*s] KheTimeAugment ret true (basic)\n",
			2 * KheEjectorCurrDepth(ej) + 2, "");
		    return true;
		  }
		}
	      }
	    }
	  }
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheMeetUnVisit(anc_meet);
      }

      ** if nodes after meets, try node swaps of anc_meet's node **
      if( !nodes_before_meets && try_node_swaps && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KheTimeAugment ret true (node)\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return true;
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheNodeUnVisit(node);
      }
    }
    base += KheMeetAsstOffset(anc_meet);
    anc_meet = KheMeetAsst(anc_meet);
  } 
  if( DEBUG4 )
    fprintf(stderr,"%*s] KheTimeAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheInDomainMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)     */
/*                                                                           */
/*  Return true when assigning or moving meet to time would place its        */
/*  starting time in a given time group, passed in impl.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheInDomainMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  return KheTimeGroupContains((KHE_TIME_GROUP) impl, time);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNotInDomainMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)  */
/*                                                                           */
/*  Return true when assigning or moving meet to time would not place its    */
/*  starting time in a given time group, passed in impl.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheNotInDomainMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  return !KheTimeGroupContains((KHE_TIME_GROUP) impl, time);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheIncreaseOverlapMeetMoveFn(KHE_MEET meet,                         */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Return true when moving meet to time would increase the overlap with     */
/*  a time group, passed in impl.  This function assumes that meet is        */
/*  currently assigned a time.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheIncreaseOverlapMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  KHE_TIME_GROUP tg;  int curr_overlap, new_overlap, durn;
  tg = (KHE_TIME_GROUP) impl;
  durn = KheMeetDuration(meet);
  curr_overlap = KheTimeGroupOverlap(tg, KheMeetAsstTime(meet), durn);
  new_overlap = KheTimeGroupOverlap(tg, time, durn);
  return new_overlap > curr_overlap;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDecreaseOverlapMeetMoveFn(KHE_MEET meet,                         */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Return true when moving meet to time would decrease the overlap with     */
/*  a time group, passed in impl.  This function assumes that meet is        */
/*  currently assigned a time.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheDecreaseOverlapMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  KHE_TIME_GROUP tg;  int curr_overlap, new_overlap, durn;
  tg = (KHE_TIME_GROUP) impl;
  durn = KheMeetDuration(meet);
  curr_overlap = KheTimeGroupOverlap(tg, KheMeetAsstTime(meet), durn);
  new_overlap = KheTimeGroupOverlap(tg, time, durn);
  return new_overlap < curr_overlap;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVAIL_RESOURCE_MEET_MOVE                                             */
/*                                                                           */
/*  Data needed for meet move functions following.                           */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef struct khe_avail_resource_meet_move_rec {
  KHE_TIME_GROUP	time_group;
  KHE_RESOURCE		resource;
} *KHE_AVAIL_RESOURCE_MEET_MOVE;
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheOutsideTimeGroupResourceAvailMeetMoveFn(KHE_MEET meet,           */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Meet move function which allows meet to move to a time outside a         */
/*  given time group when a given resource is free.                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheOutsideTimeGroupResourceAvailMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  KHE_AVAIL_RESOURCE_MEET_MOVE armv;  KHE_TIMETABLE_MONITOR tm;
  armv = (KHE_AVAIL_RESOURCE_MEET_MOVE) impl;
  tm = KheResourceTimetableMonitor(KheMeetSoln(meet), armv->resource);
  return !KheTimeGroupContains(armv->time_group, time) &&
    KheTimetableMonitorTimeMeetCount(tm, time) == 0;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheOutsideTimeGroupResourceUnAvailMeetMoveFn(KHE_MEET meet,         */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Meet move function which allows meet to move to a time outside a         */
/*  given time group when a given resource is busy.                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheOutsideTimeGroupResourceUnAvailMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  KHE_AVAIL_RESOURCE_MEET_MOVE armv;  KHE_TIMETABLE_MONITOR tm;
  armv = (KHE_AVAIL_RESOURCE_MEET_MOVE) impl;
  tm = KheResourceTimetableMonitor(KheMeetSoln(meet), armv->resource);
  return !KheTimeGroupContains(armv->time_group, time) &&
    KheTimetableMonitorTimeMeetCount(tm, time) > 0;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(KHE_EJECTOR ej,  */
/*    KHE_TASK task, KHE_RESOURCE r)                                         */
/*                                                                           */
/*  Try a Kempe move of task's meet to somewhere else, and at the same       */
/*  time an ejecting move or assignment of task to r.  This function does    */
/*  not mark anything visited.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(KHE_EJECTOR ej,
  KHE_TASK task, KHE_RESOURCE r)
{
  /* KHE_SOLN soln; */  int max_offset, offset, base, i, junk, index, extra;
  KHE_MEET anc_meet, target_meet;  KHE_NODE node, parent_node;
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s[ KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", %s)\n", KheResourceId(r));
  }
  /* soln = KheEjectorSoln(ej); */
  anc_meet = KheMeetFirstMovable(KheTaskMeet(task), &junk);
  base = 0;
  while( anc_meet != NULL )
  {
    node = KheMeetNode(anc_meet);
    if( node != NULL && KheNodeParent(node) != NULL && !KheNodeIsVizier(node) )
    {
      parent_node = KheNodeParent(node);
      extra = KheEjectorCurrAugmentCount(ej);
      for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
      {
	index = (extra + i ) % KheNodeMeetCount(parent_node);
	target_meet = KheNodeMeet(parent_node, index);
	max_offset = KheMeetDuration(target_meet)-KheMeetDuration(anc_meet);
	for( offset = 0;  offset <= max_offset;  offset++ )
	{
	  if( KheKempeMeetMoveAndEjectingTaskAssignOrMoveRepair(ej,
		anc_meet, target_meet, offset, task, r) )
	  {
	    if( DEBUG5 )
	      fprintf(stderr, "%*s] %s ret true\n",
		2 * KheEjectorCurrDepth(ej) + 2, "",
		"KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment");
	    return true;
	  }
	}
      }
    }
    base += KheMeetAsstOffset(anc_meet);
    anc_meet = KheMeetAsst(anc_meet);
  }
  if( DEBUG5 )
    fprintf(stderr,
      "%*s] KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskEjectingAssignResourceGroup(KHE_EJECTOR ej, KHE_TASK task,   */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  Try an ejecting task assignment of task to each resource of rg, and      */
/*  if times may be repaired, also try a meet-and-task move.                 */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheTaskEjectingAssignResourceGroup(KHE_EJECTOR ej, KHE_TASK task,
  KHE_RESOURCE_GROUP rg)
{
  int i, index;  KHE_RESOURCE r;
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    index = (KheEjectorCurrAugmentCount(ej) + i) %
      KheResourceGroupResourceCount(rg);
    r = KheResourceGroupResource(rg, index);
    if( KheEjectingTaskAssignOrMoveRepair(ej, task, r) )
      return true;
    if( KheEjectorRepairTimes(ej) &&
	KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(ej, task, r) )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignOrMove(KHE_EJECTOR ej, KHE_TASK task,                  */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  Assign or move task to any element of rg except what it's assigned to    */
/*  now, possibly doing a Kempe meet move of the enclosing meet as well.     */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskAssignOrMove(KHE_EJECTOR ej, KHE_TASK task,
  KHE_RESOURCE_GROUP rg)
{
  int i, index, extra;  KHE_RESOURCE r, curr_r;
  curr_r = KheTaskAsstResource(task);
  extra = KheEjectorCurrAugmentCount(ej);
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    index = (extra + i) % KheResourceGroupResourceCount(rg);
    r = KheResourceGroupResource(rg, index);
    if( r != curr_r )
    {
      if( KheEjectingTaskAssignOrMoveRepair(ej, task, r) )
	return true;
      if( KheEjectorRepairTimes(ej) &&
          KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(ej, task, r) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingTaskMoveAugment(KHE_EJECTOR ej, KHE_TASK task)           */
/*                                                                           */
/*  Try all ejecting task moves of task.  Don't bother trying if preassigned.*/
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectingTaskMoveAugment(KHE_EJECTOR ej, KHE_TASK task)
{
  KHE_RESOURCE_GROUP rg;
  if( !KheTaskIsPreassigned(task, NULL) )
  {
    task = KheTaskFirstUnFixed(task);
    if( task != NULL && !KheTaskVisited(task, 0) )
    {
      KheTaskVisit(task);
      rg = KheTaskDomain(task);
      if( KheTaskAssignOrMove(ej, task, rg) )
	return true;
      if( KheEjectorCurrMayRevisit(ej) )
	KheTaskUnVisit(task);
    }
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetEjectingTaskMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,       */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Try all ejecting task moves of the task of meet currently assigned r.    */
/*  Don't bother trying if r is preassigned to the task involved.            */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMeetEjectingTaskMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_RESOURCE r)
{
  KHE_TASK task;
  if( !KheMeetContainsResourceAssignment(meet, r, &task) )
    MAssert(false, "KheMeetEjectingTaskMoveAugment internal error");
  return KheEjectingTaskMoveAugment(ej, task);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceOverloadAugment(KHE_EJECTOR ej, KHE_RESOURCE r,          */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Carry out an augment which tries to decrease the number of times that    */
/*  r is busy within time group tg.                                          */
/*                                                                           */
/*  This function is used to repair avoid unavailable times, limit busy      */
/*  times, and limit workload defects, and demand defects derived from them. */
/*                                                                           */
/*  Implementation note.  Testing tg_time_count against all_time_count       */
/*  will save time on limit workload defects.                                */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceOverloadAugment(KHE_EJECTOR ej, KHE_RESOURCE r,
  KHE_TIME_GROUP tg)
{
  KHE_MEET meet;  KHE_TIME time;  KHE_TASK task;  bool ejecting;
  int i, tg_time_count, all_time_count, index, durn, extra;  KHE_SOLN soln;
  soln = KheEjectorSoln(ej);
  ejecting = KheEjectorEjectingNotBasic(ej);
  tg_time_count = KheTimeGroupTimeCount(tg);
  all_time_count = KheInstanceTimeCount(KheSolnInstance(soln));

  /* if r is (possibly) unpreassigned, try ejecting task moves */
  if( KheEjectorRepairResources(ej) &&
      !KheResourceTypeDemandIsAllPreassigned(KheResourceResourceType(r)) )
  {
    extra = KheEjectorCurrAugmentCount(ej);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      index = (extra + i) % KheResourceAssignedTaskCount(soln, r);
      task = KheResourceAssignedTask(soln, r, index);
      if( !KheTaskIsPreassigned(task, NULL) )
      {
	meet = KheTaskMeet(task);
	durn = KheMeetDuration(meet);
	time = KheMeetAsstTime(meet);
	if( time != NULL && KheTimeGroupOverlap(tg, time, durn) > 0 )
	{
	  task = KheTaskFirstUnFixed(task);
	  if( task != NULL && !KheTaskVisited(task, 0) )
	  {
	    KheTaskVisit(task);
	    if( KheTaskAssignOrMove(ej, task, KheTaskDomain(task)) )
	      return true;
	    if( KheEjectorCurrMayRevisit(ej) )
	      KheTaskUnVisit(task);
	  }
	}
      }
    }
  }

  /* try ejecting meet moves away from tg */
  if( KheEjectorRepairTimes(ej) && tg_time_count < all_time_count )
  {
    extra = KheEjectorCurrAugmentCount(ej);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      index = (extra + i) % KheResourceAssignedTaskCount(soln, r);
      task = KheResourceAssignedTask(soln, r, index);
      meet = KheTaskMeet(task);
      time = KheMeetAsstTime(meet);
      durn = KheMeetDuration(meet);
      if( time != NULL && KheTimeGroupOverlap(tg, time, durn) > 0 )
      {
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	*** */
	if( KheTimeAugment(ej, meet, true, &KheDecreaseOverlapMeetMoveFn,
	  (void *) tg, KHE_OPTIONS_KEMPE_NO, ejecting, !ejecting, false) )
	  return true;
      }
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOneMeetOnly(KHE_TIMETABLE_MONITOR tm, KHE_TIME_GROUP tg,         */
/*    int busy_count)                                                        */
/*                                                                           */
/*  Return true if tm contains one meet only in tg; if present, that meet    */
/*  will have duration busy_count.                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheOneMeetOnly(KHE_TIMETABLE_MONITOR tm, KHE_TIME_GROUP tg,
  int busy_count)
{
  int i, j;  KHE_TIME t;  KHE_MEET meet;
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    t = KheTimeGroupTime(tg, i);
    for( j = 0;  j < KheTimetableMonitorTimeMeetCount(tm, t);  j++ )
    {
      meet = KheTimetableMonitorTimeMeet(tm, t, j);
      if( KheMeetAsstTime(meet) == t && KheMeetDuration(meet) == busy_count )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceUnderloadAugment(KHE_EJECTOR ej, KHE_RESOURCE r,         */
/*    KHE_TIME_GROUP tg, int busy_count)                                     */
/*                                                                           */
/*  Carry out an augment which tries to either increase or reduce to zero    */
/*  the number of times that r is busy within time group tg.  It is          */
/*  currently busy at busy_count times.                                      */
/*                                                                           */
/*  This function is used to repair cluster busy times and limit busy        */
/*  times underloads.                                                        */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceUnderloadAugment(KHE_EJECTOR ej, KHE_RESOURCE r,
  KHE_TIME_GROUP tg, int busy_count)
{
  KHE_MEET meet;  KHE_TIME time;  bool ejecting;
  int i, tg_time_count, all_time_count, index, durn, extra;  KHE_SOLN soln;
  soln = KheEjectorSoln(ej);
  ejecting = KheEjectorEjectingNotBasic(ej);
  tg_time_count = KheTimeGroupTimeCount(tg);
  all_time_count = KheInstanceTimeCount(KheSolnInstance(soln));

  if( KheEjectorRepairTimes(ej) && tg_time_count < all_time_count )
  {
    /* try ejecting meet moves into tg */
    extra = KheEjectorCurrAugmentCount(ej);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      index = (extra + i) % KheResourceAssignedTaskCount(soln, r);
      meet = KheTaskMeet(KheResourceAssignedTask(soln, r, index));
      time = KheMeetAsstTime(meet);
      durn = KheMeetDuration(meet);
      if( time != NULL && KheTimeGroupOverlap(tg, time, durn) < durn &&
	KheTimeAugment(ej, meet, false, &KheIncreaseOverlapMeetMoveFn,
	  (void *) tg, KHE_OPTIONS_KEMPE_YES, ejecting, !ejecting, false) )
	return true;
    }

    /* try clearing out tg altogether, using a cluster-like repair */
    if( busy_count > 0 && (KheEjectorCurrDepth(ej) == 1 ||
	KheOneMeetOnly(KheResourceTimetableMonitor(soln, r), tg, busy_count)) )
    {
      if( KheEjectorCurrDepth(ej) == 1 )
	KheSolnNewGlobalVisit(soln);
      if( KheMeetBoundRepair(ej, r, KheSolnComplementTimeGroup(soln, tg),
	    KHE_REPAIR_LIMIT_BUSY_MEETS_UNASSIGN) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "split events augment functions"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheSplitMonitorEvent(KHE_MONITOR d)                            */
/*                                                                           */
/*  Return the event monitored by split events or distribute split events    */
/*  monitor d.                                                               */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT KheSplitMonitorEvent(KHE_MONITOR d)
{
  switch( KheMonitorTag(d) )
  {
    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return KheSplitEventsMonitorEvent((KHE_SPLIT_EVENTS_MONITOR) d);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return KheDistributeSplitEventsMonitorEvent(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) d);

    default:

      MAssert(false, "KheSplitMonitorEvent internal error");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnEventMeetsVisited(KHE_SOLN soln, KHE_EVENT e, int slack)     */
/*                                                                           */
/*  Like KheMeetVisited(meet, slack) only applied to all the meets of soln   */
/*  derived from e; it returns true if any of them are visited.              */
/*                                                                           */
/*****************************************************************************/

static bool KheSolnEventMeetsVisited(KHE_SOLN soln, KHE_EVENT e, int slack)
{
  int i;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    if( KheMeetVisited(KheEventMeet(soln, e, i), slack) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventMeetsVisit(KHE_SOLN soln, KHE_EVENT e)                  */
/*                                                                           */
/*  KheMeetVisit(meet) applied to all the meets of soln derived from e.      */
/*                                                                           */
/*****************************************************************************/

static void KheSolnEventMeetsVisit(KHE_SOLN soln, KHE_EVENT e)
{
  int i;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    KheMeetVisit(KheEventMeet(soln, e, i));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventMeetsUnVisit(KHE_SOLN soln, KHE_EVENT e)                */
/*                                                                           */
/*  KheMeetUnVisit(meet) applied to all the meets of soln derived from e.    */
/*                                                                           */
/*****************************************************************************/

static void KheSolnEventMeetsUnVisit(KHE_SOLN soln, KHE_EVENT e)
{
  int i;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    KheMeetUnVisit(KheEventMeet(soln, e, i));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)                */
/*                                                                           */
/*  Augment function for individual split events and distribute split        */
/*  events defects.                                                          */
/*                                                                           */
/*****************************************************************************/

static void KheSplitEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SPLIT_ANALYSER sa;  int i, j, k, merged_durn, split1_durn, split2_durn;
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet1, meet2;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheSplitEventsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_SPLIT_EVENTS_MONITOR_TAG ||
    KheMonitorTag(d) == KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG,
    "KheSplitEventsAugment internal error 1");
  if( KheEjectorUseSplitMoves(ej) )
  {
    soln = KheMonitorSoln(d);
    e = KheSplitMonitorEvent(d);
    if( !KheSolnEventMeetsVisited(soln, e, 0) )
    {
      /* get split and merge suggestions */
      sa = KheOptionsStructuralSplitAnalyser(KheEjectorOptions(ej));
      KheSolnEventMeetsVisit(soln, e);
      KheSplitAnalyserAnalyse(sa, soln, e);
      if( DEBUG10 )
	KheSplitAnalyserDebug(sa, 2, 2 * KheEjectorCurrDepth(ej) + 2, stderr);

      /* carry out sa's split suggestions */
      for( i = 0;  i < KheSplitAnalyserSplitSuggestionCount(sa);  i++ )
      {
	KheSplitAnalyserSplitSuggestion(sa, i, &merged_durn, &split1_durn);
	for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
	{
	  meet1 = KheEventMeet(soln, e, j);
	  if( KheMeetAsst(meet1)!=NULL && KheMeetDuration(meet1)==merged_durn )
	  {
            if( KheMeetSplitRepair(ej, meet1, split1_durn) )
	    {
	      if( DEBUG4 )
		fprintf(stderr, "%*s] KheSplitEventsAugment ret true (a)\n",
		  2 * KheEjectorCurrDepth(ej) + 2, "");
	      return;
	    }
	  }
	}
      }

      /* carry out sa's merge suggestions */
      for( i = 0;  i < KheSplitAnalyserMergeSuggestionCount(sa);  i++ )
      {
	KheSplitAnalyserMergeSuggestion(sa, i, &split1_durn, &split2_durn);
	for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
	{
	  meet1 = KheEventMeet(soln, e, j);
	  if( KheMeetAsst(meet1)!=NULL && KheMeetDuration(meet1)==split1_durn )
	  {
	    for( k = j + 1;  k < KheEventMeetCount(soln, e);  k++ )
	    {
	      meet2 = KheEventMeet(soln, e, k);
	      if( KheMeetAsst(meet2) != NULL &&
		  KheMeetDuration(meet2) == split2_durn )
	      {
		if( KheMergeMoveRepair(ej, meet1, meet2, false) )
		{
		  if( DEBUG4 )
		    fprintf(stderr, "%*s] KheSplitEventsAugment ret true\n",
		      2 * KheEjectorCurrDepth(ej) + 2, "");
		  return;
		}
		if( KheMergeMoveRepair(ej, meet1, meet2, true) )
		{
		  if( DEBUG4 )
		    fprintf(stderr, "%*s] KheSplitEventsAugment ret true\n",
		      2 * KheEjectorCurrDepth(ej) + 2, "");
		  return;
		}
		if( KheMergeMoveRepair(ej, meet2, meet1, false) )
		{
		  if( DEBUG4 )
		    fprintf(stderr, "%*s] KheSplitEventsAugment ret true\n",
		      2 * KheEjectorCurrDepth(ej) + 2, "");
		  return;
		}
		if( KheMergeMoveRepair(ej, meet2, meet1, true) )
		{
		  if( DEBUG4 )
		    fprintf(stderr, "%*s] KheSplitEventsAugment ret true\n",
		      2 * KheEjectorCurrDepth(ej) + 2, "");
		  return;
		}
	      }
	    }
	  }
	}
      }
      if( KheEjectorCurrMayRevisit(ej) )
	KheSolnEventMeetsUnVisit(soln, e);
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheSplitEventsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)           */
/*                                                                           */
/*  Augment function for groups of split events and distribute split events  */
/*  defects.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheSplitEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheSplitEventsGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheSplitEventsGroupAugment internal error 2");
  KheSplitEventsAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assign time augment functions"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeAugment(KHE_EJECTOR ej, KHE_MONITOR d)                 */
/*                                                                           */
/*  Augment function for individual assign time defects.                     */
/*                                                                           */
/*  If KheEjectorRepairTimes(ej), then for each monitored unassigned meet,   */
/*  try all ejecting meet moves to a parent meet and offset that would       */
/*  assign the unassigned meet within its domain.                            */
/*                                                                           */
/*****************************************************************************/

static void KheAssignTimeAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;  int i;
  bool ejecting;  KHE_ASSIGN_TIME_MONITOR atm;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheAssignTimeAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_ASSIGN_TIME_MONITOR_TAG,
    "KheAssignTimeAugment internal error 1");
  if( KheEjectorRepairTimes(ej) )
  {
    atm = (KHE_ASSIGN_TIME_MONITOR) d;
    soln = KheEjectorSoln(ej);
    e = KheAssignTimeMonitorEvent(atm);
    ejecting = KheEjectorEjectingNotBasic(ej);
    for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    {
      meet = KheEventMeet(soln, e, i);
      if( KheMeetAsstTime(meet) == NULL )
      {
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	*** */
	if( KheTimeAugment(ej, meet, true, &KheInDomainMeetMoveFn,
	  (void *) KheMeetDomain(meet), KHE_OPTIONS_KEMPE_NO,
	  ejecting, !ejecting, false) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KheAssignTimeAugment ret true",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return;
	}
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheAssignTimeAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)            */
/*                                                                           */
/*  Augment function for groups of assign time defects.                      */
/*                                                                           */
/*****************************************************************************/

static void KheAssignTimeGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheAssignTimeGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheAssignTimeGroupAugment internal error 2");
  KheAssignTimeAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "prefer times augment functions"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)                */
/*                                                                           */
/*  Augment function for individual prefer times defects.                    */
/*                                                                           */
/*  If KheEjectorRepairTimes(ej), then for each monitored meet with an       */
/*  unpreferred assignment, try all Kempe/ejecting meet moves to a parent    */
/*  meet and offset that give the meet a preferred assignment.               */
/*                                                                           */
/*****************************************************************************/

static void KhePreferTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;  int i, durn;
  KHE_PREFER_TIMES_CONSTRAINT ptc;  KHE_TIME_GROUP domain;  KHE_TIME t;
  bool ejecting;  KHE_OPTIONS_KEMPE kempe;  KHE_PREFER_TIMES_MONITOR ptm;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KhePreferTimesAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_PREFER_TIMES_MONITOR_TAG,
    "KhePreferTimesAugment internal error 1");
  if( KheEjectorRepairTimes(ej) )
  {
    soln = KheEjectorSoln(ej);
    ptm = (KHE_PREFER_TIMES_MONITOR) d;
    e = KhePreferTimesMonitorEvent(ptm);
    ptc = KhePreferTimesMonitorConstraint(ptm);
    domain = KhePreferTimesConstraintDomain(ptc);
    durn = KhePreferTimesConstraintDuration(ptc);
    ejecting = KheEjectorEjectingNotBasic(ej);
    kempe = KheEjectorUseKempeMoves(ej);
    for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    {
      meet = KheEventMeet(soln, e, i);
      t = KheMeetAsstTime(meet);
      if( (durn == KHE_ANY_DURATION || KheMeetDuration(meet) == durn) &&
	  t != NULL && !KheTimeGroupContains(domain, t) )
      {
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	*** */
	if( KheTimeAugment(ej, meet, true, &KheInDomainMeetMoveFn,
	    (void *) domain, kempe, ejecting, !ejecting,
	    WITH_PREFER_TIMES_NODE_SWAPS) )
	{
	  if( DEBUG4 )
	    fprintf(stderr, "%*s] KhePreferTimesAugment ret true",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return;
	}
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KhePreferTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)           */
/*                                                                           */
/*  Augment function for groups of prefer times defects.                     */
/*                                                                           */
/*****************************************************************************/

static void KhePreferTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KhePreferTimesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KhePreferTimesGroupAugment internal error 2");
  KhePreferTimesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread events augment functions"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsHigh(KHE_SPREAD_EVENTS_MONITOR sem, int i,            */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is high, return true and set *tg to the    */
/*  time group, otherwise return false.                                      */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheTimeGroupIsHigh(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc > maximum;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsHighTimeGroup(                        */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a high time group.                           */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheSpreadEventsMonitorContainsHighTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    if( KheTimeGroupIsHigh(sem, i, &tg) )
      return true;
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,             */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is low, return true and set *tg to the     */
/*  time group, otherwise return false.                                      */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc < minimum;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsLowTimeGroup(                         */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a low time group.                            */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheSpreadEventsMonitorContainsLowTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    if( KheTimeGroupIsLow(sem, i, &tg) )
      return true;
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)         */
/*                                                                           */
/*  Return true if meet is a high meet (if it lies in a high time group).    */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;
  t = KheMeetAsstTime(meet);
  if( t != NULL )
    for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    {
      KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
      if( KheTimeGroupContains(tg, t) && inc > maximum )
	return true;
    }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)   */
/*                                                                           */
/*  Return true if meet is a very middle meet (if it lies in a middle time   */
/*  group which would remain middle if meet was removed from it).            */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;
  t = KheMeetAsstTime(meet);
  if( t != NULL )
    for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    {
      KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
      if( KheTimeGroupContains(tg, t) && inc <= maximum && inc > minimum )
	return true;
    }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_STUFF - pass this to try function                             */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
typedef struct khe_spread_stuff_rec {
  bool				to_middle;
  KHE_SPREAD_EVENTS_MONITOR	sem;
} *KHE_SPREAD_STUFF;
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)      */
/*                                                                           */
/*  This function allows a Kempe move of meet to time to be tried when it    */
/*  moves it to a low group, or (when to_middle is set) to a middle group    */
/*  which would not then become high.                                        */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheSpreadMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  KHE_SPREAD_STUFF ss = (KHE_SPREAD_STUFF) impl;
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(ss->sem);  i++ )
  {
    KheSpreadEventsMonitorTimeGroup(ss->sem, i, &tg, &minimum, &maximum, &inc);
    if( KheTimeGroupContains(tg, time) &&
	(inc < minimum || (ss->to_middle && inc < maximum)) )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,     */
/*    KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet)                          */
/*                                                                           */
/*  Try to augment by moving one of meet's ancestors so that meet lies in    */
/*  a low time group, or (but only if to_middle is true) a very middle time  */
/*  group.                                                                   */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,
  KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet)
{
  bool ejecting;  KHE_OPTIONS_KEMPE kempe;  struct khe_spread_stuff_rec ss_rec;
  ejecting = KheEjectorEjectingNotBasic(ej);
  kempe = KheEjectorUseKempeMoves(ej);
  ss_rec.to_middle = to_middle;
  ss_rec.sem = sem;
  return KheTimeAugment(ej, meet, true, &KheSpreadMeetMoveFn, (void *) &ss_rec,
    kempe, ejecting, !ejecting, WITH_SPREAD_EVENTS_NODE_SWAPS);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)               */
/*                                                                           */
/*  Augment function for individual spread events defects.                   */
/*                                                                           */
/*  If KheEjectorRepairTimes(ej), try all Kempe/ejecting meet moves of       */
/*  the relevant meets from outside each day to inside, or vice versa,       */
/*  depending on whether the problem is too few meets or too many.           */
/*                                                                           */
/*****************************************************************************/

static void KheSpreadEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT_GROUP eg;  KHE_EVENT e;  KHE_MEET meet;
  KHE_SPREAD_EVENTS_MONITOR sem;  KHE_TIME time;  KHE_TIME_GROUP tg;
  int i, j, k, minimum, maximum, inc, index, extra;  bool ejecting;
  KHE_OPTIONS_KEMPE kempe;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheDoSpreadEventsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_SPREAD_EVENTS_MONITOR_TAG,
    "KheSpreadEventsAugment internal error 1");
  if( KheEjectorRepairTimes(ej) )
  {
    ejecting = KheEjectorEjectingNotBasic(ej);
    kempe = KheEjectorUseKempeMoves(ej);
    soln = KheEjectorSoln(ej);
    sem = (KHE_SPREAD_EVENTS_MONITOR) d;
    eg = KheSpreadEventsMonitorEventGroup(sem);
    extra = KheEjectorCurrAugmentCount(ej) +
      (KheOptionsDiversify(KheEjectorOptions(ej)) ?
      5 * KheSolnDiversifier(KheEjectorSoln(ej)) : 0);
    for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    {
      KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
      if( inc < minimum )
      {
	/* try all meet moves from outside tg to inside tg */
	for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
	{
	  index = (j + extra) % KheEventGroupEventCount(eg);
	  e = KheEventGroupEvent(eg, index);
	  for( k = 0;  k < KheEventMeetCount(soln, e);  k++ )
	  {
	    index = (k + extra) % KheEventMeetCount(soln, e);
	    meet = KheEventMeet(soln, e, index);
	    time = KheMeetAsstTime(meet);
	    if( time != NULL && !KheTimeGroupContains(tg, time) &&
	        KheTimeAugment(ej, meet, true, &KheInDomainMeetMoveFn,
		  (void *) tg, kempe, ejecting, !ejecting,
		  WITH_SPREAD_EVENTS_NODE_SWAPS) )
	    {
	      if( DEBUG4 )
		fprintf(stderr, "%*s] KheSpreadEventsAugment ret true (a)\n",
		  2 * KheEjectorCurrDepth(ej) + 2, "");
	      return;
	    }
	  }
	}
      }
      else if( inc > maximum )
      {
	/* try all meet moves from inside tg to outside tg */
	for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
	{
	  index = (extra + j) % KheEventGroupEventCount(eg);
	  e = KheEventGroupEvent(eg, index);
	  for( k = 0;  k < KheEventMeetCount(soln, e);  k++ )
	  {
            index = (k + extra) % KheEventMeetCount(soln, e);
	    meet = KheEventMeet(soln, e, index);
	    time = KheMeetAsstTime(meet);
	    if( time != NULL && KheTimeGroupContains(tg, time) &&
	        KheTimeAugment(ej, meet, true, &KheNotInDomainMeetMoveFn,
		  (void *) tg, kempe, ejecting, !ejecting,
		  WITH_SPREAD_EVENTS_NODE_SWAPS) )
	    {
	      if( DEBUG4 )
		fprintf(stderr, "%*s] KheSpreadEventsAugment ret true (b)\n",
		  2 * KheEjectorCurrDepth(ej) + 2, "");
	      return;
	    }
	  }
	}
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheDoSpreadEventsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}

/* *** old version with a deprecated take on precision
static void KheSpreadEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT_GROUP eg;  KHE_EVENT e;  KHE_MEET meet;  int i, j;
  KHE_SPREAD_EVENTS_MONITOR sem;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheDoSpreadEventsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_SPREAD_EVENTS_MONITOR_TAG,
    "KheSpreadEventsAugment internal error 1");
  if( KheEjectorRepairTimes(ej) )
  {
    soln = KheEjectorSoln(ej);
    sem = (KHE_SPREAD_EVENTS_MONITOR) d;
    eg = KheSpreadEventsMonitorEventGroup(sem);

    ** try to move a high meet to a low or very middle time group **
    if( KheSpreadEventsMonitorContainsHighTimeGroup(sem) )
      for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
      {
	e = KheEventGroupEvent(eg, i);
	for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
	{
	  meet = KheEventMeet(soln, e, j);
	  if( KheMeetIsHigh(meet, sem) &&
	      KheMoveToLowOrVeryMiddleAugment(ej, true, sem, meet) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr, "%*s] KheDoSpreadEventsAugment ret true (high)\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }

    ** try to move a very middle meet to a low time group **
    if( KheSpreadEventsMonitorContainsLowTimeGroup(sem) )
      for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
      {
	e = KheEventGroupEvent(eg, i);
	for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
	{
	  meet = KheEventMeet(soln, e, j);
	  if( KheMeetIsVeryMiddle(meet, sem) &&
	      KheMoveToLowOrVeryMiddleAugment(ej, false, sem, meet) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr, "%*s] KheDoSpreadEventsAugment ret true (low)\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheDoSpreadEventsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)          */
/*                                                                           */
/*  Augment function for groups of spread events defects.                    */
/*                                                                           */
/*****************************************************************************/

static void KheSpreadEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheSpreadEventsGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheSpreadEventsGroupAugment internal error 2");
  KheSpreadEventsAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "order events augment functions"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheOrderEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)                */
/*                                                                           */
/*  Augment function for individual order events defects.                    */
/*                                                                           */
/*  Not implemented yet.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheOrderEventsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheOrderEventsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_ORDER_EVENTS_MONITOR_TAG,
    "KheOrderEventsAugment internal error 1");
  if( KheEjectorRepairTimes(ej) )
  {
    /* still to do */
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheOrderEventsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrderEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)           */
/*                                                                           */
/*  Augment function for groups of order events defects.                     */
/*                                                                           */
/*****************************************************************************/

static void KheOrderEventsGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheOrderEventsGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheOrderEventsGroupAugment internal error 2");
  KheOrderEventsAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assign resource defects"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)             */
/*                                                                           */
/*  Augment function for individual assign resource defects.                 */
/*                                                                           */
/*  If KheEjectorRepairResources(ej), then for each monitored unassigned     */
/*  task, try all ejecting task assignments to resources in its domain.      */
/*  If, in addition, KheEjectorRepairTimes(ej) is true, then also try        */
/*  combinations of those ejecting task assignments with meet moves.         */
/*                                                                           */
/*****************************************************************************/

static void KheAssignResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i;
  KHE_ASSIGN_RESOURCE_MONITOR arm;
  if( DEBUG3 )
  {
    fprintf(stderr, "%*s[ KheAssignResourceAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_ASSIGN_RESOURCE_MONITOR_TAG,
    "KheAssignResourceAugment internal error 1");
  if( KheEjectorRepairResources(ej) )
  {
    arm = (KHE_ASSIGN_RESOURCE_MONITOR) d;
    soln = KheEjectorSoln(ej);
    er = KheAssignResourceMonitorEventResource(arm);
    for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, i));
      if( task != NULL && KheTaskAsst(task) == NULL && !KheTaskVisited(task,0) )
      {
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	*** */
	KheTaskVisit(task);
        if( KheTaskAssignOrMove(ej, task, KheTaskDomain(task)) )
	{
	  if( DEBUG3 )
	    fprintf(stderr, "%*s] KheAssignResourceAugment ret true\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "");
	  return;
	}
	if( KheEjectorCurrMayRevisit(ej) )
	  KheTaskUnVisit(task);
      }
    }
  }
  if( DEBUG3 )
    fprintf(stderr, "%*s] KheAssignResourceAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)        */
/*                                                                           */
/*  Augment function for groups of assign resource defects.                  */
/*                                                                           */
/*****************************************************************************/

static void KheAssignResourceGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheAssignResourceGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheAssignResourceGroupAugment internal error 2");
  KheAssignResourceAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "prefer resources defects"                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesAugment(KHE_EJECTOR ej, KHE_MONITOR d)            */
/*                                                                           */
/*  Augment function for individual prefer resources defects.                */
/*                                                                           */
/*  If KheEjectorRepairResources(ej), then for each monitored task assigned  */
/*  an unpreferred resource, try all ejecting task moves to preferred ones.  */
/*                                                                           */
/*****************************************************************************/

static void KhePreferResourcesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE old_r;
  KHE_PREFER_RESOURCES_CONSTRAINT prc;
  KHE_PREFER_RESOURCES_MONITOR prm;
  if( DEBUG3 )
  {
    fprintf(stderr, "%*s[ KhePreferResourcesAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug((KHE_MONITOR) prm, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_PREFER_RESOURCES_MONITOR_TAG,
    "KhePreferResourcesAugment internal error 1");
  if( KheEjectorRepairResources(ej) )
  {
    soln = KheEjectorSoln(ej);
    prm = (KHE_PREFER_RESOURCES_MONITOR) d;
    er = KhePreferResourcesMonitorEventResource(prm);
    prc = KhePreferResourcesMonitorConstraint(prm);
    rg = KhePreferResourcesConstraintDomain(prc);
    for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, i));
      if( task != NULL && KheTaskAsst(task) != NULL && !KheTaskVisited(task,0) )
      {
	old_r = KheTaskAsstResource(task);
	if( old_r != NULL && !KheResourceGroupContains(rg, old_r) )
	{
	  /* ***
	  if( KheEjectorCurrDepth(ej) == 1 )
	    KheSolnNewGlobalVisit(soln);
	  *** */
	  KheTaskVisit(task);
	  if( KheTaskAssignOrMove(ej, task, rg) )
	  {
	    if( DEBUG3 )
	      fprintf(stderr, "%*s] KhePreferResourcesAugment ret true",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	  if( KheEjectorCurrMayRevisit(ej) )
	    KheTaskUnVisit(task);
	}
      }
    }
  }
  if( DEBUG3 )
    fprintf(stderr, "%*s] KhePreferResourcesAugment ret false",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)       */
/*                                                                           */
/*  Augment function for groups of prefer resources defects.                 */
/*                                                                           */
/*****************************************************************************/

static void KhePreferResourcesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KhePreferResourcesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KhePreferResourcesGroupAugment internal error 2");
  KhePreferResourcesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "avoid split assignments defects"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignedTasksVisited(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam,   */
/*    int slack)                                                             */
/*                                                                           */
/*  Like KheTaskVisited(task, slack) only applied to all the assigned        */
/*  tasks monitored by asam; it returns true if any of them are visited.     */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static bool KheAssignedTasksVisited(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam,
  int slack)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, j));
      if( KheTaskAsstResource(task) != NULL && KheTaskVisited(task, slack) )
	return true;
    }
  }
  return false;
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignedTasksVisit(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)     */
/*                                                                           */
/*  Like KheTaskVisit(task) only applied to all the assigned tasks           */
/*  monitored by asam.                                                       */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static void KheAssignedTasksVisit(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, j));
      if( KheTaskAsstResource(task) != NULL )
	KheTaskVisit(task);
    }
  }
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignedTasksUnVisit(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)   */
/*                                                                           */
/*  Like KheTaskUnVisit(task) only applied to all the assigned tasks         */
/*  monitored by asam.                                                       */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static void KheAssignedTasksUnVisit(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, j));
      if( KheTaskAsstResource(task) != NULL )
	KheTaskUnVisit(task);
    }
  }
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheAnyTaskAssignedToResource(KHE_RESOURCE r,                    */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)                              */
/*                                                                           */
/*  Return any task monitored by asam assigned r.  There must be one.        */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static KHE_TASK KheAnyTaskAssignedToResource(KHE_RESOURCE r,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskFirstUnFixed(KheEventResourceTask(soln, er, j));
      if( KheTaskAsstResource(task) == r )
	return task;
    }
  }
  MAssert(false, "KheAnyTaskAssignedToResource internal error");
  return NULL;  /* keep compiler happy */
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej, KHE_MONITOR d)      */
/*                                                                           */
/*  Augment function for individual avoid split assignments defects.         */
/*  There is no group version of this function.                              */
/*                                                                           */
/*  If KheEjectorRepairResources(ej), then for each pair of resources        */
/*  assigned to monitored tasks, and for each resource in the domain of      */
/*  one of those tasks, try a set of ejecting task moves of those tasks to   */
/*  that resource.                                                           */
/*                                                                           */
/*  Also, if KheEjectorRepairTimes(ej) as well, then for each resource       */
/*  assigned to exactly one of the monitored tasks, try moving that          */
/*  task's meet to a different time, and simultaneously moving the task      */
/*  to one of the other resources assigned to a monitored task.              */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_AVOID_SPLIT_REPAIR
static void KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;  KHE_TASK task;
  int i, j, k;  KHE_RESOURCE r1, r2, r;  KHE_RESOURCE_GROUP rg;
  if( DEBUG5 )
  {
    fprintf(stderr, "%*s[ KheAvoidSplitAssignmentsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheEjectorRepairResources(ej) )
  {
    asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d;
    if( !KheAssignedTasksVisited(asam, 0) )
    {
      KheAssignedTasksVisit(asam);

      /* try ejecting task-set moves */
      for( i = 0; i < KheAvoidSplitAssignmentsMonitorResourceCount(asam); i++ )
      {
	r1 = KheAvoidSplitAssignmentsMonitorResource(asam, i);
	rg = KheTaskDomain(KheAnyTaskAssignedToResource(r1, asam));
	for( j=i+1; j<KheAvoidSplitAssignmentsMonitorResourceCount(asam); j++ )
	{
	  r2 = KheAvoidSplitAssignmentsMonitorResource(asam, j);
	  for( k = 0;  k < KheResourceGroupResourceCount(rg);  k++ )
	  {
	    r = KheResourceGroupResource(rg, k);
	    if( DEBUG12 )
	      fprintf(stderr, "  trying %s\n",
		KheResourceId(r) == NULL ? "-" : KheResourceId(r));
	    if( KheEjectingTaskSetMoveRepair(ej, asam, r1, r2, r) )
	    {
	      if( DEBUG5 )
		fprintf(stderr, "] KheAvoidSplitAssignmentsAugment ret true\n");
	      return;
	    }
	  }
	}
      }

      /* try meet/task moves */
      if( KheEjectorRepairTimes(ej) )
      for( i = 0; i < KheAvoidSplitAssignmentsMonitorResourceCount(asam); i++ )
      {
	if( KheAvoidSplitAssignmentsMonitorResourceMultiplicity(asam, i) == 1 )
	{
	  /* r1 occurs in only one task, try moving it to any other resource */
	  r1 = KheAvoidSplitAssignmentsMonitorResource(asam, i);
	  task = KheAnyTaskAssignedToResource(r1, asam);
	  MAssert(task!=NULL, "KheAvoidSplitAssignmentsAugment internal error");
	  for( j=0; j<KheAvoidSplitAssignmentsMonitorResourceCount(asam); j++ )
	  {
	    r2 = KheAvoidSplitAssignmentsMonitorResource(asam, j);
	    if(r2!=r1 &&
	      KheKempeMeetMoveAndEjectingTaskAssignOrMoveAugment(ej, task, r2))
	    {
	      if( DEBUG5 )
		fprintf(stderr, "%*s] KheAvoidSplitAssignmentsAugment true\n",
		  2 * KheEjectorCurrDepth(ej) + 2, "");
	      return;
	    }
	  }
	}
      }

      if( KheEjectorCurrMayRevisit(ej) )
	KheAssignedTasksUnVisit(asam);
    }
  }
  if( DEBUG5 )
    fprintf(stderr, "%*s] KheAvoidSplitAssignmentsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
#else
static void KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;  int i;  KHE_RESOURCE r;
  /* bool save_ejector_repair_times; */
  if( DEBUG15 )
  {
    fprintf(stderr, "%*s[ KheAvoidSplitAssignmentsAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheEjectorRepairResources(ej) )
  {
    asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d;
    /* *** interesting idea, didn't help much
    save_ejector_repair_times = KheEjectorRepairTimes(ej);
    if( KheEjectorCurrDepth(ej) == 1 )
      KheOptionsSetEjectorRepairTimes(KheEjectorOptions(ej), true);
    *** */
    for( i = 0;  i < KheAvoidSplitAssignmentsMonitorResourceCount(asam);  i++ )
    {
      r = KheAvoidSplitAssignmentsMonitorResource(asam, i);
      if( KheEjectorCurrDepth(ej) == 1 ||
	  KheAvoidSplitAssignmentsMonitorResourceMultiplicity(asam, i) == 1 )
      {
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(KheEjectorSoln(ej));
	if( KheSplitTasksUnassignRepair(ej, asam, r) )
	{
	  if( DEBUG15 )
	  {
	    fprintf(stderr,
	      "%*s] KheAvoidSplitAssignmentsAugment ret true%s:\n",
	      2 * KheEjectorCurrDepth(ej) + 2, "",
	      KheEjectorCurrDepth(ej) == 1 ? " 1" : "");
	      KheMonitorDebug(d, 1, 2 * KheEjectorCurrDepth(ej) + 4, stderr);
	  }
	  /* ***
	  KheOptionsSetEjectorRepairTimes(KheEjectorOptions(ej),
	    save_ejector_repair_times);
	  *** */
	  return;
	}
      }
    }
    /* ***
    KheOptionsSetEjectorRepairTimes(KheEjectorOptions(ej),
      save_ejector_repair_times);
    *** */
  }
  if( DEBUG15 )
    fprintf(stderr, "%*s] KheAvoidSplitAssignmentsAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  Submodule "avoid clashes augments"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesAugment(KHE_EJECTOR ej, KHE_MONITOR d)               */
/*                                                                           */
/*  Augment function for individual avoid clashes defects.                   */
/*                                                                           */
/*  The current plan is to detach all avoid clashes monitors during          */
/*  repair, since their work is done (and done better) by demand monitors.   */
/*  So this function aborts if it is called.                                 */
/*                                                                           */
/*  Old specification                                                        */
/*  -----------------                                                        */
/*  For each clashing task, try moving it to any other resource in its       */
/*  domain.  This function expects to be called only during resource repair, */
/*  because during time repair avoid clashes monitors are usually detached.  */
/*  Even during resource repair it would probably be better to ignore these  */
/*  defects, since they are basically unrepairable then anyway.              */
/*                                                                           */
/*****************************************************************************/

static void KheAvoidClashesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  /* ***
  KHE_RESOURCE r;  KHE_TIMETABLE_MONITOR tm;
  KHE_SOLN soln;  KHE_TIME t;  KHE_MEET meet;  int i, j;
  KHE_AVOID_CLASHES_MONITOR acm;
  *** */
  if( DEBUG3 )
  {
    fprintf(stderr, "%*s[ KheAvoidClashesAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  MAssert(false, "KheAvoidClashesAugment: unexpected call");

  /* boilerplate */
  /* ***
  MAssert(KheMonitorTag(d) == KHE_AVOID_CLASHES_MONITOR_TAG,
    "KheAvoidClashesAugment internal error 1");
  soln = KheEjectorSoln(ej);
  acm = (KHE_AVOID_CLASHES_MONITOR) d;
  r = KheAvoidClashesMonitorResource(acm);
  tm = KheResourceTimetableMonitor(soln, r);

  ** find each clashing task and reassign it in all possible ways **
  for( i = 0;  i < KheTimetableMonitorClashingTimeCount(tm);  i++ )
  {
    t = KheTimetableMonitorClashingTime(tm, i);
    for( j = 0;  j < KheTimetableMonitorTimeMeetCount(tm, t);  j++ )
    {
      meet = KheTimetableMonitorTimeMeet(tm, t, j);
      if(KheMeetEjectingTaskMoveAugment(ej, meet, r) )
      {
	if( DEBUG3 )
	  fprintf(stderr,"%*s] KheAvoidClashesAugment ret true\n",
	    2 * KheEjectorCurrDepth(ej) + 2, "");
	return;
      }
    }
  }

  ** wrapup **
  if( DEBUG3 )
    fprintf(stderr, "%*s] KheAvoidClashesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)          */
/*                                                                           */
/*  Augment function for groups of avoid clashes defects.                    */
/*                                                                           */
/*****************************************************************************/

static void KheAvoidClashesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheAvoidClashesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheAvoidClashesGroupAugment internal error 2");
  KheAvoidClashesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "avoid unavailable times augments"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)      */
/*                                                                           */
/*  Augment function for individual avoid unavailable times defects.         */
/*                                                                           */
/*  Try repairs documented in the header of KheResourceOverloadAugment,      */
/*  which in this case amount to ejecting task moves away from d's resource  */
/*  and Kempe/ejecting meet moves away from unavailable times.               */
/*                                                                           */
/*****************************************************************************/

static void KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR autm;  KHE_TIME_GROUP domain;
  KHE_RESOURCE r; KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT autc;
  /* KHE_SOLN soln; */
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheAvoidUnavailableTimesAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  MAssert(KheMonitorTag(d) == KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG,
    "KheAvoidUnavailableTimesAugment internal error 1");
  autm = (KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) d;
  /* soln = KheEjectorSoln(ej); */
  autc = KheAvoidUnavailableTimesMonitorConstraint(autm);
  domain = KheAvoidUnavailableTimesConstraintUnavailableTimes(autc);
  r = KheAvoidUnavailableTimesMonitorResource(autm);
  if( KheResourceOverloadAugment(ej, r, domain) )
  {
    if( DEBUG4 )
      fprintf(stderr, "%*s] KheAvoidUnavailableTimesAugment ret true\n",
	2 * KheEjectorCurrDepth(ej) + 2, "");
    return;
  }

  if( DEBUG4 )
    fprintf(stderr, "%*s] KheAvoidUnavailableTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d) */
/*                                                                           */
/*  Augment function for groups of avoid unavailable times defects.          */
/*                                                                           */
/*****************************************************************************/

static void KheAvoidUnavailableTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheAvoidUnavailableTimesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheAvoidUnavailableTimesGroupAugment internal error 2");
  KheAvoidUnavailableTimesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "limit idle times augments (simple)"                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleTimeWillBeBusy(KHE_TIME time,                           */
/*    KHE_TIMETABLE_MONITOR tm, KHE_MEET meet, KHE_TIME new_time)            */
/*                                                                           */
/*  Return true if time will be busy after meet moves to new_time.           */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
static bool KheLimitIdleTimeWillBeBusy(KHE_TIME time,
  KHE_TIMETABLE_MONITOR tm, KHE_MEET meet, KHE_TIME new_time)
{
  int i;  KHE_MEET meet2;

  /* time will be busy if there is already some other meet at time */
  for( i = 0;  i < KheTimetableMonitorTimeMeetCount(tm, time);  i++ )
  {
    meet2 = KheTimetableMonitorTimeMeet(tm, time, i);
    if( meet2 != meet )
      return true;
  }

  /* alternatively, time will be busy if meet will overlap time */
  return KheTimeIntervalsOverlap(new_time, KheMeetDuration(meet), time, 1) > 0;
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitNewIdle(KHE_TIME_GROUP tg, KHE_TIMETABLE_MONITOR tm,         */
/*    KHE_MEET meet, KHE_TIME new_time)                                      */
/*                                                                           */
/*  Return the number of idle times there will be in tg after meet           */
/*  moves from where it is now to new_time.                                  */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
static int KheLimitNewIdle(KHE_TIME_GROUP tg, KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, KHE_TIME new_time)
{
  KHE_TIME time, first_busy, last_busy;  int i, busy_count;

  /* find the number of and first and last busy times after meet moves */
  first_busy = last_busy = NULL;
  busy_count = 0;
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    time = KheTimeGroupTime(tg, i);
    if( KheLimitIdleTimeWillBeBusy(time, tm, meet, new_time) )
    {
      busy_count++;
      if( first_busy == NULL )
	first_busy = time;
      last_busy = time;
    }
  }

  /* use the usual formula to calculate the number of idle times */
  return busy_count == 0 ? 0 :
    (KheTimeIndex(last_busy) - KheTimeIndex(first_busy) + 1) - busy_count;
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)    */
/*                                                                           */
/*  Return true if moving meet from where it is now to new_time would        */
/*  decrease the number of idle times, according to litm (passed in impl).   */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
static bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  int tg_durn, durn, i, delta;  KHE_LIMIT_IDLE_TIMES_MONITOR litm;
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME tg_time, old_time;
  litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) impl;
  tm = KheResourceTimetableMonitor(KheMeetSoln(meet),
    KheLimitIdleTimesMonitorResource(litm));
  durn = KheMeetDuration(meet);
  old_time = KheMeetAsstTime(meet);
  delta = 0;
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    tg = KheTimeGroupMonitorTimeGroup(tgm);
    tg_time = KheTimeGroupTime(tg, 0);
    tg_durn = KheTimeGroupTimeCount(tg);
    if( KheTimeIntervalsOverlap(old_time, durn, tg_time, tg_durn) ||
	KheTimeIntervalsOverlap(time, durn, tg_time, tg_durn) )
      delta += KheLimitNewIdle(tg, tm, meet, time) -
	KheTimeGroupMonitorIdleCount(tgm);
  }
  return delta < 0;
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_INFO - info needed by move fn                       */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef struct khe_limit_idle_times_info_rec {
  KHE_LIMIT_IDLE_TIMES_MONITOR	monitor;		** the defect        **
  bool				adjacent_to_idle;	** if adj. to idle   **
} *KHE_LIMIT_IDLE_TIMES_INFO;
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)    */
/*                                                                           */
/*  This function allows a move of meet to time to be tried when it reduces  */
/*  the number of idle times.                                                */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  KHE_TIME times[2];  int count, i, durn;  KHE_LIMIT_IDLE_TIMES_INFO liti;
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;

  ** boilerplate **
  liti = (KHE_LIMIT_IDLE_TIMES_INFO) impl;
  litm = liti->monitor;
  tm = KheResourceTimetableMonitor(KheMonitorSoln((KHE_MONITOR) litm),
    KheLimitIdleTimesMonitorResource(litm));

  ** if r is not free at the times required, return false **
  if( !KheTimetableMonitorTimeAvailable(tm, meet, time) )
    return false;

  ** if time reduces idle times, return true **
  durn = KheMeetDuration(meet);
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    tg = KheTimeGroupMonitorTimeGroup(tgm);
    if( KheTimeGroupContains(tg, time) )
    {
      ** a move to within tgm's existing busy window is always acceptable **
      KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
      if( count == 2 && KheTimeLT(times[0], 0, time, 0) &&
	  KheTimeLT(time, durn-1, times[1], 0) )
	return true;

      ** if meet is adjacent to an idle time, a move to adjacent to the **
      ** first or last time is also acceptable if meet is not running then **
      if( liti->adjacent_to_idle )
      {
	** moving to an empty day seems odd, but causes no new idle times **
	if( count == 0 )
	  return true;

	** before times[0] is fine unless meet starts at times[0] **
	if( KheTimeEQ(time, durn, times[0], 0) &&
	    KheTimeNE(KheMeetAsstTime(meet), 0, times[0], 0) )
	  return true;

	** after times[count-1] is fine unless meet ends at times[count-1] **
	if( KheTimeEQ(time, 0, times[count-1], 1) &&
	    KheTimeNE(KheMeetAsstTime(meet), durn-1, times[count-1], 0) )
	  return true;
      }
    }
  }

  ** otherwise return false **
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "limit idle times augments (complex)"                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LIT_SOLVER - solver for complex limit idle times repairs             */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_lit_solver_rec {
  KHE_EJECTOR			ejector;	/* the ejector               */
  KHE_LIMIT_IDLE_TIMES_MONITOR	litm;		/* the defect                */
  KHE_TIME_GROUP		tg;		/* defective time group      */
  KHE_RESOURCE			resource;	/* litm's resource           */
  KHE_SOLN			soln;		/* litm's soln               */
  ARRAY_KHE_MEET		meets;		/* the meets of tgm          */
  int				total_durn;	/* total duration of meets   */
  int				asst_count;	/* number of meets assigned  */
  int				asst_problems;	/* number of bad assignments */
  int				repair_count;	/* number repairs            */
  KHE_TIME			curr_start_time; /* start time, currently    */
} *KHE_LIT_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIT_SOLVER KheLitSolverMake(KHE_EJECTOR ej,                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)         */
/*                                                                           */
/*  Make and return a new lit solver with these attributes.                  */
/*                                                                           */
/*****************************************************************************/

static KHE_LIT_SOLVER KheLitSolverMake(KHE_EJECTOR ej,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_LIT_SOLVER res;  KHE_TIMETABLE_MONITOR tm;
  KHE_TIME time;  int i, j, pos;  KHE_MEET meet;

  /* make the basic object */
  MMake(res);
  res->ejector = ej;
  res->litm = litm;
  res->tg = KheTimeGroupMonitorTimeGroup(tgm);
  res->resource = KheLimitIdleTimesMonitorResource(litm);
  res->soln = KheEjectorSoln(ej);
  MArrayInit(res->meets);
  res->total_durn = 0;
  res->asst_count = 0;
  res->asst_problems = 0;
  res->repair_count = 0;
  res->curr_start_time = NULL;

  /* add the meets of tgm */
  tm = KheResourceTimetableMonitor(res->soln, res->resource);
  for( i = 0;  i < KheTimeGroupTimeCount(res->tg);  i++ )
  {
    time = KheTimeGroupTime(res->tg, i);
    for( j = 0;  j < KheTimetableMonitorTimeMeetCount(tm, time);  j++ )
    {
      meet = KheTimetableMonitorTimeMeet(tm, time, j);
      if( !MArrayContains(res->meets, meet, &pos) )
      {
	MArrayAddLast(res->meets, meet);
	res->total_durn += KheMeetDuration(meet);
      }
    }
  }

  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLitSolverDelete(KHE_LIT_SOLVER lits)                             */
/*                                                                           */
/*  Delete lits.                                                             */
/*                                                                           */
/*****************************************************************************/

static void KheLitSolverDelete(KHE_LIT_SOLVER lits)
{
  MArrayFree(lits->meets);
  MFree(lits);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLitSolverAsstIsOpen(KHE_LIT_SOLVER lits,                         */
/*    KHE_MEET meet, KHE_TIME time, int pos)                                 */
/*                                                                           */
/*  Return true if the assignment of meet to time is open.                   */
/*                                                                           */
/*****************************************************************************/
#define KheIndent 2*(KheEjectorCurrDepth(lits->ejector) + pos + 1)

/* *** this way doesn't work.  Will optimize later if successful
static bool KheLitSolverAsstIsOpen(KHE_LIT_SOLVER lits,
  KHE_MEET meet, KHE_TIME time, int pos)
{
  int unassigned_tixels;
  MAssert(lits->asst_problems <= 1, "KheLitSolverAsstIsOpen internal error");
  unassigned_tixels = KheSolnMatchingDefectCount(lits->soln);
  if( !KheMeetAssignTime(meet, time) )
  {
    ** not open because failed to assign **
    fprintf(stderr, "%*s  KheLitSolverAsstIsOpen returning false (asst)\n",
      KheIndent, "");
    return false;
  }
  else if( unassigned_tixels == KheSolnMatchingDefectCount(lits->soln) )
  {
    ** open since assigned and no increase in tixel problems **
    KheMeetUnAssignTime(meet);
    fprintf(stderr, "%*s  KheLitSolverAsstIsOpen returning true (no tix)\n",
      KheIndent, "");
    return true;
  }
  else
  {
    ** tixel problem; open only if we are not up to 2 tixel problems **
    KheMeetUnAssignTime(meet);
    lits->asst_problems++;
    fprintf(stderr, "%*s  KheLitSolverAsstIsOpen returning %s (tix)\n",
      KheIndent, "", lits->asst_problems <= 1 ? "true" : "false");
    return lits->asst_problems <= 1;
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLitSolverSolve(KHE_LIT_SOLVER lits, int pos, int prev_durn)      */
/*                                                                           */
/*  Solve lits from position pos onwards; prev_durn is the total duration    */
/*  previous to pos.                                                         */
/*                                                                           */
/*****************************************************************************/

static bool KheLitSolverSolve(KHE_LIT_SOLVER lits, int pos, int prev_durn)
{
  KHE_TIME time;  int i, save_problems;  KHE_MEET meet, tmp;  bool success;
  if( DEBUG9 )
    fprintf(stderr, "%*s[ KheLitSolverSolve(lits, pos %d, prev_durn %d)\n",
      KheIndent, "", pos, prev_durn);
  if( pos >= MArraySize(lits->meets) )
  {
    /* carry out the repair now; the ejector will undo it */
    lits->repair_count++;
    KheEjectorRepairBegin(lits->ejector);
    MArrayForEach(lits->meets, &meet, &i)
      KheMeetUnAssignTime(meet);
    time = lits->curr_start_time;
    success = true;
    MArrayForEach(lits->meets, &meet, &i)
    {
      MAssert(time != NULL, "KheLitSolverSolve internal error 1");
      success = KheMeetAssignTime(meet, time);
      if( !success )
	break;
      time = KheTimeHasNeighbour(time, KheMeetDuration(meet)) ?
	KheTimeNeighbour(time, KheMeetDuration(meet)) : NULL;
    }
    success = KheEjectorRepairEnd(lits->ejector, KHE_REPAIR_COMPLEX_IDLE_MOVE,
      success /* , lits->total_durn */);
    if( DEBUG9 )
      fprintf(stderr, "%*s] KheLitSolverSolve returning %s (repair)\n",
	KheIndent, "", success ? "true" : "false");
    return success;
  }
  else if( lits->repair_count < MAX_LIT_REPAIRS )
  {
    /* try every meet from pos onwards in position pos */
    for( i = pos;  i < MArraySize(lits->meets);  i++ )
    {
      MArraySwap(lits->meets, pos, i, tmp);
      meet = MArrayGet(lits->meets, pos);
      time = KheTimeNeighbour(lits->curr_start_time, prev_durn);
      save_problems = lits->asst_problems;
      if( /* KheLitSolverAsstIsOpen(lits, meet, time, pos) && */
	  KheLitSolverSolve(lits, pos+1, prev_durn + KheMeetDuration(meet)) )
      {
	if( DEBUG9 )
	  fprintf(stderr, "%*s] KheLitSolverSolve returning true\n",
	    KheIndent, "");
	return true;
      }
      lits->asst_problems = save_problems;
      MArraySwap(lits->meets, pos, i, tmp);
    }
    if( DEBUG9 )
      fprintf(stderr, "%*s] KheLitSolverSolve returning false\n",
	KheIndent, "");
    return false;
  }
  else
  {
    /* just try one repair, since limit is reached */
    meet = MArrayGet(lits->meets, pos);
    time = KheTimeNeighbour(lits->curr_start_time, prev_durn);
    save_problems = lits->asst_problems;
    if( /* KheLitSolverAsstIsOpen(lits, meet, time, pos) && */
	KheLitSolverSolve(lits, pos+1, prev_durn + KheMeetDuration(meet)) )
    {
      if( DEBUG9 )
	fprintf(stderr, "%*s] KheLitSolverSolve returning true (single)\n",
	  KheIndent, "");
      return true;
    }
    lits->asst_problems = save_problems;
    if( DEBUG9 )
      fprintf(stderr, "%*s] KheLitSolverSolve returning false (single)\n",
	KheIndent, "");
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleComplexAugment(KHE_EJECTOR ej,                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)         */
/*                                                                           */
/*  Augment function for limit idle times defects which tries complex        */
/*  reassignments of the meets assigned to the times of tgm, which is        */
/*  known to contain at least one idle time.                                 */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitIdleComplexAugment(KHE_EJECTOR ej,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_LIT_SOLVER lits;  int stop_index, i;
  lits = KheLitSolverMake(ej, litm, tgm);
  if( DEBUG8 )
    fprintf(stderr, "%*s[ KheLimitIdleComplexAugment(ej, %s, %s)\n",
      2*KheEjectorCurrDepth(ej), "", KheResourceId(lits->resource),
      KheTimeGroupId(lits->tg));
  stop_index = KheTimeGroupTimeCount(lits->tg) - lits->total_durn;
  for( i = 0;  i < stop_index;  i++ )
  {
    lits->curr_start_time = KheTimeGroupTime(lits->tg, i);
    if( DEBUG8 )
      fprintf(stderr, "%*s  starting time %s:\n", 2*KheEjectorCurrDepth(ej), "",
	KheTimeId(lits->curr_start_time));
    if( KheLitSolverSolve(lits, 0, 0) )
    {
      KheLitSolverDelete(lits);
      if( DEBUG8 )
	fprintf(stderr,
	  "%*s] KheLimitIdleComplexAugment returning true (%d repairs)\n",
	  2*KheEjectorCurrDepth(ej), "", lits->repair_count);
      return true;
    }
  }
  KheLitSolverDelete(lits);
  if( DEBUG8 )
    fprintf(stderr,
      "%*s] KheLimitIdleComplexAugment returning false (%d repairs)\n",
      2*KheEjectorCurrDepth(ej), "", lits->repair_count);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)    */
/*                                                                           */
/*  Return true if moving meet from where it is now to time would cause      */
/*  it to begin at an idle time of litm (passed in impl).                    */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
#else
#if WITH_NEW_LIMIT_IDLE_REPAIR_EXACT
static bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;  KHE_TIME_GROUP_MONITOR tgm;
  int durn, xdurn, i, count;  KHE_TIME times[2], xt;
  KHE_TIMETABLE_MONITOR tm;
  litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) impl;
  tm = KheResourceTimetableMonitor(KheMeetSoln(meet),
    KheLimitIdleTimesMonitorResource(litm));
  if( KheTimetableMonitorTimeMeetCount(tm, time) > 0 )
    return false;
  durn = KheMeetDuration(meet);
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    if( KheTimeGroupMonitorIdleCount(tgm) > 0 )
    {
      /* tgm has an idle time to overlap with */
      KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
      MAssert(count == 2, "KheLimitIdleMeetMoveFn internal error");
      xt = KheTimeNeighbour(times[0], 1);
      xdurn = KheTimeIndex(times[1]) - KheTimeIndex(times[0]) - 1;
      if( KheTimeIntervalsOverlap(time, 1, xt, xdurn) > 0 )
	return true;
    }
  }
  return false;
}
#endif
#endif


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)    */
/*                                                                           */
/*  Return true if moving meet from where it is now to time would cause      */
/*  it to overlap an idle time of litm (passed in impl).                     */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
#else
#if WITH_NEW_LIMIT_IDLE_REPAIR_OVERLAP
static bool KheLimitIdleMeetMoveFn(KHE_MEET meet, KHE_TIME time, void *impl)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;  KHE_TIME_GROUP_MONITOR tgm;
  int durn, xdurn, ydurn, i, count, ti;  KHE_TIME times[2], xt, yt, time2;
  KHE_TIMETABLE_MONITOR tm;
  litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) impl;
  tm = KheResourceTimetableMonitor(KheMeetSoln(meet),
    KheLimitIdleTimesMonitorResource(litm));
  durn = KheMeetDuration(meet);
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    if( KheTimeGroupMonitorIdleCount(tgm) > 0 )
    {
      /* tgm has an idle time to overlap with */
      KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
      MAssert(count == 2, "KheLimitIdleMeetMoveFn internal error");
      xt = KheTimeNeighbour(times[0], 1);
      xdurn = KheTimeIndex(times[1]) - KheTimeIndex(times[0]) - 1;
      if( KheTimeIntervalsOverlapInterval(time, durn, xt, xdurn, &yt, &ydurn) )
	for( ti = 0;  ti < ydurn;  ti++ )
	{
	  time2 = KheTimeNeighbour(yt, ti);
	  if( KheTimetableMonitorTimeMeetCount(tm, time2) == 0 )
	    return true;
	}
    }
  }
  return false;
}
#endif
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)             */
/*                                                                           */
/*  Augment function for individual limit idle times defects.                */
/*                                                                           */
/*  This function tries each ejecting meet of a meet assigned the monitor's  */
/*  resource to a time that causes it to overlap with an idle time.          */
/*                                                                           */
/*****************************************************************************/
#if WITH_OLD_LIMIT_IDLE_REPAIR
#else
static void KheLimitIdleTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;  KHE_MEET meet;  KHE_RESOURCE r;
  KHE_OPTIONS_KEMPE kempe;  bool ejecting;  int i, index, extra, junk;
  KHE_SOLN soln;  KHE_TASK task;  KHE_TIME_GROUP_MONITOR tgm;

  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheLimitIdleTimesAugment(\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheEjectorRepairTimes(ej) )
  {
    /* simple repairs */
    soln = KheEjectorSoln(ej);
    litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) d;
    ejecting = KheEjectorEjectingNotBasic(ej);
    kempe = KheEjectorUseKempeMoves(ej);
    extra = KheEjectorCurrAugmentCount(ej);
    r = KheLimitIdleTimesMonitorResource(litm);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      index = (extra + i) % KheResourceAssignedTaskCount(soln, r);
      task = KheResourceAssignedTask(soln, r, index);
      meet = KheMeetFirstMovable(KheTaskMeet(task), &junk);
      if( meet!=NULL && KheTimeAugment(ej, meet, true, &KheLimitIdleMeetMoveFn,
	  (void *) litm, kempe, ejecting, !ejecting, false) )
      {
	if( DEBUG4 )
	  fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true\n",
	    2 * KheEjectorCurrDepth(ej) + 2, "");
	return;
      }
    }

    /* complex repairs, only tried at depth 1 (currently not at all) */
    if( KheEjectorCurrDepth(ej) == 1 && false )
      for( i = 0; i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm); i++ )
      {
        index = (extra+i) % KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);
	tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, index);
	if( KheTimeGroupMonitorIdleCount(tgm) > 0 )
	{
	  /* ***
	  KheSolnNewGlobalVisit(KheMonitorSoln(d));
	  *** */
	  if( KheLimitIdleComplexAugment(ej, litm, tgm) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true (c)\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheLimitIdleTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleMeetBoundAugment(KHE_EJECTOR ej, KHE_MEET last_meet,    */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)         */
/*                                                                           */
/*  Try unassigning last_meet and reducing the domains of all litm's meets.  */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
static bool KheLimitIdleMeetBoundAugment(KHE_EJECTOR ej, KHE_MEET last_meet,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_SOLN soln;  int i, j, count, num, durn, junk;
  KHE_TIME_GROUP_MONITOR tgm2;  KHE_MEET meet;  KHE_TASK task;  KHE_RESOURCE r;
  KHE_TIME times[2];  KHE_TIME_GROUP busy_tg, starting_tg;  KHE_MEET_BOUND mb;
  bool success;
  r = KheLimitIdleTimesMonitorResource(litm);
  soln = KheEjectorSoln(ej);
  if( DEBUG16 )
  {
    fprintf(stderr, "[ KheLimitIdleMeetBoundAugment(ej, ");
    KheMeetDebug(last_meet, 1, -1, stderr);
    fprintf(stderr, ", litm(%s), tgm(", KheResourceId(r));
    KheTimeGroupDebug(KheTimeGroupMonitorTimeGroup(tgm), 1, -1, stderr);
    fprintf(stderr, "))\n");
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      task = KheResourceAssignedTask(soln, r, i);
      meet = KheMeetFirstMovable(KheTaskMeet(task), &junk);
      if( meet != NULL )
	KheMeetDebug(meet, 2, 2, stderr);
    }
  }

  /* build a time group containing all times not mentioned in litm */
  KheSolnTimeGroupBegin(soln);
  KheSolnTimeGroupUnion(soln, KheInstanceFullTimeGroup(KheSolnInstance(soln)));
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm2 = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    KheSolnTimeGroupDifference(soln, KheTimeGroupMonitorTimeGroup(tgm2));
  }

  /* add the busy spans of litm's time groups, except the last time of tgm */
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm2 = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    KheTimeGroupMonitorFirstAndLastBusyTimes(tgm2, times, &count);
    if( count >= 1 )
    {
      if( DEBUG16 )
      {
	fprintf(stderr, "  within ");
	KheTimeGroupDebug(KheTimeGroupMonitorTimeGroup(tgm2), 1, -1, stderr);
	fprintf(stderr, " busy span %s .. %s\n", KheTimeId(times[0]),
	  KheTimeId(times[count - 1]));
      }
      num = KheTimeIndex(times[count-1]) - KheTimeIndex(times[0]) + 1;
      if( tgm2 == tgm )
	num--;
      for( j = 0;  j < num;  j++ )
        KheSolnTimeGroupAddTime(soln, KheTimeNeighbour(times[0], j));
    }
  }
  busy_tg = KheSolnTimeGroupEnd(soln);
  if( DEBUG16 )
  {
    fprintf(stderr, "  busy_tg: ");
    KheTimeGroupDebug(busy_tg, 1, 0, stderr);
  }

  /* unassign last_meet */
  KheEjectorRepairBegin(ej);
  if( !KheMeetUnAssign(last_meet) )
  {
    if( DEBUG16 )
      fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret false (last_meet)\n");
    return KheEjectorRepairEnd(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN, false);
  }

  /* add a meet bound to each meet */
  mb = KheMeetBoundMake(soln, true, busy_tg);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    meet = KheMeetFirstMovable(KheTaskMeet(task), &junk);
    if( meet == NULL || !KheMeetAddMeetBound(meet, mb) )
    {
      if( DEBUG16 )
      {
	fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret false (");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, ", %d, ", durn);
	KheTimeGroupDebug(starting_tg, 1, -1, stderr);
	fprintf(stderr, ")\n");
      }
      return KheEjectorRepairEnd(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN,
	false);
    }
  }
  success = KheEjectorRepairEndLong(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN,
    true, 1, false, &KheMeetBoundOnSuccess, (void *) mb);
  if( DEBUG16 )
    fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret %s (end)\n",
      success ? "true" : "false");
  return success;
}

/* ***
static bool KheLimitIdleMeetBoundAugment(KHE_EJECTOR ej, KHE_MEET last_meet,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm, KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_SOLN soln;  int i, j, count, num, durn, junk;  KHE_MEET_BOUND_GROUP mbg;
  KHE_TIME_GROUP_MONITOR tgm2;  KHE_MEET meet;  KHE_TASK task;  KHE_RESOURCE r;
  KHE_TIME times[2];  KHE_TIME_GROUP busy_tg, starting_tg;  KHE_MEET_BOUND mb;
  bool success;
  r = KheLimitIdleTimesMonitorResource(litm);
  soln = KheEjectorSoln(ej);
  if( DEBUG16 )
  {
    fprintf(stderr, "[ KheLimitIdleMeetBoundAugment(ej, ");
    KheMeetDebug(last_meet, 1, -1, stderr);
    fprintf(stderr, ", litm(%s), tgm(", KheResourceId(r));
    KheTimeGroupDebug(KheTimeGroupMonitorTimeGroup(tgm), 1, -1, stderr);
    fprintf(stderr, "))\n");
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      task = KheResourceAssignedTask(soln, r, i);
      meet = KheMeetFir stUnFixed(KheTaskMeet(task), &junk);
      KheMeetDebug(meet, 2, 2, stderr);
    }
  }

  ** build a time group containing all times not mentioned in litm **
  KheSolnTimeGroupBegin(soln);
  KheSolnTimeGroupUnion(soln, KheInstanceFullTimeGroup(KheSolnInstance(soln)));
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm2 = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    KheSolnTimeGroupDifference(soln, KheTimeGroupMonitorTimeGroup(tgm2));
  }

  ** add the busy spans of litm's time groups, except the last time of tgm **
  for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
  {
    tgm2 = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, i);
    KheTimeGroupMonitorFirstAndLastBusyTimes(tgm2, times, &count);
    if( count >= 1 )
    {
      if( DEBUG16 )
      {
	fprintf(stderr, "  within ");
	KheTimeGroupDebug(KheTimeGroupMonitorTimeGroup(tgm2), 1, -1, stderr);
	fprintf(stderr, " busy span %s .. %s\n", KheTimeId(times[0]),
	  KheTimeId(times[count - 1]));
      }
      num = KheTimeIndex(times[count-1]) - KheTimeIndex(times[0]) + 1;
      if( tgm2 == tgm )
	num--;
      for( j = 0;  j < num;  j++ )
        KheSolnTimeGroupAddTime(soln, KheTimeNeighbour(times[0], j));
    }
  }
  busy_tg = KheSolnTimeGroupEnd(soln);
  if( DEBUG16 )
  {
    fprintf(stderr, "  busy_tg: ");
    KheTimeGroupDebug(busy_tg, 1, 0, stderr);
  }

  ** unassign last_meet **
  KheEjectorRepairBegin(ej);
  if( !KheMeetUnAssign(last_meet) )
  {
    if( DEBUG16 )
      fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret false (last_meet)\n");
    return KheEjectorRepairEnd(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN, false);
  }

  ** add meet bounds for each possible duration (currently just 1-3) **
  mbg = KheMeetBoundGroupMake(soln);
  for( durn = 1;  durn <= 3;  durn++ )
  {
    starting_tg = KheSolnStartingTimeGroup(soln, busy_tg, durn);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      task = KheResourceAssignedTask(soln, r, i);
      meet = KheMeetFir stUnFixed(KheTaskMeet(task), &junk);
      if( !KheMeetBoundMake(mbg, meet, durn, starting_tg, &mb) )
      {
	if( DEBUG16 )
	{
	  fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret false (");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, ", %d, ", durn);
	  KheTimeGroupDebug(starting_tg, 1, -1, stderr);
	  fprintf(stderr, ")\n");
	}
	return KheEjectorRepairEnd(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN,
	  false);
      }
    }
  }
  success = KheEjectorRepairEndLong(ej, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN,
    true, 1, false, KheMeetBoundOnSuccess, (void *) mbg);
  if( DEBUG16 )
    fprintf(stderr, "] KheLimitIdleMeetBoundAugment ret %s (end)\n",
      success ? "true" : "false");
  return success;
}
*** */
#endif


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupMonitorHasSoleLastMeet(KHE_TIME_GROUP_MONITOR tgm,      */
/*    KHE_MEET *last_meet)                                                   */
/*                                                                           */
/*  If the last busy time of tgm is occupied by a single meet, set           */
/*  *last_meet to that meet and return true.  Otherwise return false.        */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupMonitorHasSoleLastMeet(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MEET *last_meet)
{
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME times[2];  int count;
  KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
  MAssert(count >= 1, "KheTimeGroupMonitorHasSoleLastMeet internal error");
  tm = KheTimeGroupMonitorTimetableMonitor(tgm);
  if( KheTimetableMonitorTimeMeetCount(tm, times[count-1]) == 1 )
  {
    *last_meet = KheTimetableMonitorTimeMeet(tm, times[count-1], 0);
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d) old version */
/*                                                                           */
/*  Augment function for individual limit idle times defects.                */
/*                                                                           */
/*  This function tries each Kempe/ejecting meet move whose initial meet     */
/*  move moves a meet from the start or end of a day (actually, a time       */
/*  group of the monitor) in any way that reduces the number of idle times.  */
/*                                                                           */
/*****************************************************************************/

#if WITH_OLD_LIMIT_IDLE_REPAIR
static void KheLimitIdleTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;  KHE_MEET meet, last_meet;
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME_GROUP_MONITOR tgm;  bool ejecting;
  KHE_TIME time, times[2];  int count, i, j, k, index, extra, durn;

  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheLimitIdleTimesAugment(\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheEjectorRepairTimes(ej) )
  {
    /* simple repairs */
    litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) d;
    tm = KheResourceTimetableMonitor(KheEjectorSoln(ej),
      KheLimitIdleTimesMonitorResource(litm));
    ejecting = KheEjectorEjectingNotBasic(ej);
    extra = KheEjectorCurrAugmentCount(ej);
    for( durn = 1;  durn <= 4;  durn++ )
    {
      for( i = 0; i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm); i++ )
      {
	index = (extra+i) % KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);
	tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, index);
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(KheMonitorSoln(d));
	*** */
	KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
	for( j = 0;  j < count;  j++ )
	{
	  time = times[j];
	  for( k = 0;  k < KheTimetableMonitorTimeMeetCount(tm, time);  k++ )
	  {
	    meet = KheTimetableMonitorTimeMeet(tm, time, k);
	    if( KheMeetDuration(meet) == durn ||
		(durn == 4 && KheMeetDuration(meet) >= durn) )
	    {
	      if( KheTimeAugment(ej, meet, true, &KheLimitIdleMeetMoveFn,
		(void *)litm, KHE_OPTIONS_KEMPE_YES, ejecting,!ejecting,false) )
	      {
		if( DEBUG4 )
		  fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true\n",
		    2 * KheEjectorCurrDepth(ej) + 2, "");
		return;
	      }
	    }
	  }
	}
      }
    }

    /* meet bound repairs, only tried at depth 1 (currently not at all) */
    if( KheEjectorCurrDepth(ej) == 1 && false )
      for( i = 0; i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm); i++ )
      {
        index = (extra+i) % KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);
	tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, index);
	if( KheTimeGroupMonitorIdleCount(tgm) > 0 &&
	    KheTimeGroupMonitorHasSoleLastMeet(tgm, &last_meet) )
	{
	  KheSolnNewGlobalVisit(KheEjectorSoln(ej));
	  if( KheLimitIdleMeetBoundAugment(ej, last_meet, litm, tgm) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true (d)\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }

    /* complex repairs, only tried at depth 1 (currently not at all) */
    if( KheEjectorCurrDepth(ej) == 1 && false )
      for( i = 0; i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm); i++ )
      {
        index = (extra+i) % KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);
	tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, index);
	if( KheTimeGroupMonitorIdleCount(tgm) > 0 )
	{
	  /* ***
	  KheSolnNewGlobalVisit(KheEjectorSoln(ej));
	  *** */
	  if( KheLimitIdleComplexAugment(ej, litm, tgm) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true (c)\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheLimitIdleTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesAugment (old version)                              */
/*                                                                           */
/*  Augment function for individual limit idle times defects.                */
/*                                                                           */
/*  This function tries each ejecting meet move which moves a meet from the  */
/*  start or end of a day (formally, a time group of the monitor) in a way   */
/*  which reduces the number of idle times.                                  */
/*                                                                           */
/*  Take each meet assigned to d's resource which occurs at the beginning    */
/*  or end of one of its days, and try each ejecting move of that meet to a  */
/*  time that reduces the number of idle times and does not cause clashes.   */
/*  A move to any non-clashing time between the first and last busy times    */
/*  on any day is acceptable.  If the meet is adjacent to an idle time,      */
/*  then moving it far away will remove that idle time, so in that case it   */
/*  is also acceptable to move it to just before the first busy time of any  */
/*  day, and just after the last busy time of any day, provided that first   */
/*  or last busy time is not one when the meet itself is currently running.  */
/*                                                                           */
/*  This function assumes that the time groups of the limit idle times       */
/*  defect being repaired contain only consecutive times.  Fortunately,      */
/*  this "compactness" condition is now part of the official specification   */
/*  of the limit idle times constraint.                                      */
/*                                                                           */
/*****************************************************************************/

/* *** old version
static void KheLimitIdleTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR litm;  KHE_RESOURCE r;  KHE_MEET meet;
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME_GROUP_MONITOR tgm;
  KHE_TIME time, times[2];  int count, i, j, k, index;
  bool ejecting;  struct khe_limit_idle_times_info_rec ir;

  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheLimitIdleTimesAugment(\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheEjectorRepairTimes(ej) )
  {
    litm = (KHE_LIMIT_IDLE_TIMES_MONITOR) d;
    r = KheLimitIdleTimesMonitorResource(litm);
    tm = KheResourceTimetableMonitor(KheMonitorSoln(d), r);
    ejecting = KheEjectorEjectingNotBasic(ej);
    for( i = 0;  i < KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);  i++ )
    {
      index = (KheEjectorCurrAugmentCount(ej) + i) %
        KheLimitIdleTimesMonitorTimeGroupMonitorCount(litm);
      tgm = KheLimitIdleTimesMonitorTimeGroupMonitor(litm, index);
      KheTimeGroupMonitorFirstAndLastBusyTimes(tgm, times, &count);
      ** *** did some good, but probably only by chance
      if( KheEjectorCurrDepth(ej) <= 1 )
	KheSolnNewGlobalVisit(KheMonitorSoln(d));
      *** **
      for( j = 0;  j < count;  j++ )
      {
	time = times[j];
	for( k = 0;  k < KheTimetableMonitorTimeMeetCount(tm, time);  k++ )
	{
	  meet = KheTimetableMonitorTimeMeet(tm, time, k);

	  ** decide whether meet is adjacent to an idle time, and set ir **
	  ir.monitor = litm;
	  ir.adjacent_to_idle = false;
	  if( KheTimetableMonitorTimeMeetCount(tm, time) == 1 )
	  {
	    mtime = KheMeetAsstTime(meet);
	    mdurn = KheMeetDuration(meet);
	    if( j == 0 )
	    {
	      ** meet is sole meet at start of day **
	      if( KheTimeLT(mtime, mdurn, times[count-1], 0) )
	      {
		ntime = KheTimeNeighbour(mtime, mdurn);
		if( KheTimetableMonitorTimeMeetCount(tm, ntime) == 0 )
		  ir.adjacent_to_idle = true;
	      }
	    }
	    else
	    {
	      ** meet is sole meet at end of day **
	      if( KheTimeGT(mtime, 0, times[0], 0) )
	      {
		ntime = KheTimeNeighbour(mtime, -1);
		if( KheTimetableMonitorTimeMeetCount(tm, ntime) == 0 )
		  ir.adjacent_to_idle = true;
	      }
	    }
	  }

	  ** do the repair **
	  if( KheTimeAugment(ej, meet, true, &KheLimitIdleMeetMoveFn,
	      (void *) litm, true, ejecting, !ejecting, false) )
	  {
	    if( DEBUG4 )
	      fprintf(stderr,"%*s] KheLimitIdleTimesAugment ret true\n",
		2 * KheEjectorCurrDepth(ej) + 2, "");
	    return;
	  }
	}
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheLimitIdleTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)        */
/*                                                                           */
/*  Augment function for groups of limit idle times defects.                 */
/*                                                                           */
/*****************************************************************************/

static void KheLimitIdleTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheLimitIdleTimesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheLimitIdleTimesGroupAugment internal error 2");
  KheLimitIdleTimesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cluster busy times augments"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterToEmptyDayMeetMoveFn(KHE_MEET meet,                       */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Meet move function which allows the meet to move to any time on any      */
/*  empty day.  Here impl is a cluster busy times monitor.                   */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static bool KheClusterToEmptyDayMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  int i;  KHE_TIME_GROUP_MONITOR tgm;
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm;
  cbtm = (KHE_CLUSTER_BUSY_TIMES_MONITOR) impl;
  if( DEBUG7 )
  {
    fprintf(stderr, "[ KheClusterToEmptyDayMeetMoveFn(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, "(, %s)\n", KheTimeId(time));
  }
  for( i = 0;  i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);  i++)
  {
    tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, i);
    if( KheTimeGroupMonitorBusyCount(tgm) == 0 &&
	KheTimeGroupContains(KheTimeGroupMonitorTimeGroup(tgm), time) )
    {
      if( DEBUG7 )
	fprintf(stderr, "] KheClusterToEmptyDayMeetMoveFn ret true\n");
      return true;
    }
  }
  if( DEBUG7 )
    fprintf(stderr, "] KheClusterToEmptyDayMeetMoveFn ret false\n");
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMoveCouldCauseUnderload(KHE_MEET meet,                       */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Return true if moving meet could cause an underload in cbtm.             */
/*                                                                           */
/*****************************************************************************/

/* *** who cares?  try it anyway
static bool KheMeetMoveCouldCauseUnderload(KHE_MEET meet,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, busy_count, overlap;  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_TIME time;
  time = KheMeetAsstTime(meet);
  if( time != NULL )
    for( i = 0; i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm); i++)
    {
      tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, i);
      busy_count = KheTimeGroupMonitorBusyCount(tgm);
      tg = KheTimeGroupMonitorTimeGroup(tgm);
      overlap = KheMeetTimeGroupOverlap(meet, tg);
      if( overlap > 0 && (busy_count - overlap <= 0) )
      {
	if( DEBUG7 )
	  fprintf(stderr, "  overlap %d, busy_count %d\n", overlap, busy_count);
	return true;
      }
    }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetCouldFixClusterUnderload(KHE_MEET meet,                      */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Return true if a move of meet could fix a cluster underload in cbtm,     */
/*  because meet is assigned a time, and either meet overlaps one of the     */
/*  time groups of cbtm and is not the only meet to do so, or meet does      */
/*  not overlap any time group of cbtm.  The point of this condition is      */
/*  that meet can move without emptying out any time group of cbtm.          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMeetCouldFixClusterUnderload(KHE_MEET meet,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, overlap, busy_count;  KHE_TIME time;  KHE_TIME_GROUP_MONITOR tgm;
  KHE_TIME_GROUP tg;
  time = KheMeetAsstTime(meet);
  if( time == NULL )
    return false;
  for( i = 0;  i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);  i++ )
  {
    tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, i);
    busy_count = KheTimeGroupMonitorBusyCount(tgm);
    if( busy_count > 0 )
    {
      tg = KheTimeGroupMonitorTimeGroup(tgm);
      overlap = KheTimeGroupOverlap(tg, time, KheMeetDuration(meet));
      if( overlap > 0 )
	return busy_count > overlap;
    }
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterToEmptyDayMeetMoveFn(KHE_MEET meet, KHE_TIME time,        */
/*    void *impl)                                                            */
/*                                                                           */
/*  Callback function which permits a meet move to time when it would be     */
/*  the only meet on its day.   Here impl is a cluster busy times monitor.   */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheClusterToEmptyDayMeetMoveFn(KHE_MEET meet, KHE_TIME time,
  void *impl)
{
  int i;  KHE_TIME_GROUP_MONITOR tgm;  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm;
  KHE_TIME_GROUP tg;
  cbtm = (KHE_CLUSTER_BUSY_TIMES_MONITOR) impl;
  for( i = 0;  i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);  i++ )
  {
    tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, i);
    if( KheTimeGroupMonitorBusyCount(tgm) == 0 )
    {
      tg = KheTimeGroupMonitorTimeGroup(tgm);
      if( KheTimeGroupOverlap(tg, time, KheMeetDuration(meet)) > 0 )
	return true;
    }
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterUnderloadAugment(KHE_EJECTOR ej,                          */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Repair an underload defect in cbtm, by moving any meet which is not      */
/*  the only meet in its day (or not present on any day) to a day when       */
/*  it is the only meet in its day.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheClusterUnderloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, junk;  KHE_RESOURCE r;  KHE_SOLN soln;  bool ejecting;
  KHE_MEET meet;  KHE_TASK task;
  if( KheEjectorRepairTimes(ej) )
  {
    r = KheClusterBusyTimesMonitorResource(cbtm);
    if( DEBUG7 )
      fprintf(stderr, "%*s[ KheClusterUnderloadAugment(ej, %s)\n",
	2*KheEjectorCurrDepth(ej), "", KheResourceId(r));
    soln = KheEjectorSoln(ej);
    ejecting = KheEjectorEjectingNotBasic(ej);
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      task = KheResourceAssignedTask(soln, r, i);
      meet = KheMeetFirs tUnFixed(KheTaskMeet(task), &junk);
      if( KheMeetCouldFixClusterUnderload(meet, cbtm) )
      {
	if( KheTimeAugment(ej, meet, false, &KheClusterToEmptyDayMeetMoveFn,
	    (void *) cbtm, KHE_OPTIONS_KEMPE_NO, ejecting, !ejecting, false) )
	{
	  if( DEBUG7 )
	    fprintf(stderr, "%*s] KheClusterUnderloadAugment ret true\n",
	      2*KheEjectorCurrDepth(ej), "");
	  return;
	}
      }
    }
    if( DEBUG7 )
      fprintf(stderr, "%*s] KheClusterUnderloadAugment ret false\n",
	2*KheEjectorCurrDepth(ej), "");
  }
}
*** */

/* ***
static void KheClusterUnderloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, j, k, index, tgm_count;  KHE_RESOURCE r;  KHE_TIME_GROUP tg;
  KHE_TIME_GROUP_MONITOR tgm;  KHE_SOLN soln;  bool ejecting;  KHE_MEET meet;
  KHE_TIMETABLE_MONITOR tm;  KHE_TIME time;
  if( KheEjectorRepairTimes(ej) )
  {
    r = KheClusterBusyTimesMonitorResource(cbtm);
    soln = KheEjectorSoln(ej);
    tm = KheResourceTimetableMonitor(soln, r);
    ejecting = KheEjectorEjectingNotBasic(ej);
    if( DEBUG7 )
      fprintf(stderr, "%*s[ KheClusterUnderloadAugment(ej, %s)\n",
	2*KheEjectorCurrDepth(ej), "", KheResourceId(r));
    tgm_count = KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
    for( i = 0;  i < tgm_count;  i++ )
    {
      index = (KheEjectorCurrAugmentCount(ej) + i) % tgm_count;
      tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, index);
      if( KheTimeGroupMonitorBusyCount(tgm) > 0 )
      {
	tg = KheTimeGroupMonitorTimeGroup(tgm);
	for( j = 0;  j < KheTimeGroupTimeCount(tg);  j++ )
	{
	  time = KheTimeGroupTime(tg, j);
	  for( k = 0;  k < KheTimetableMonitorTimeMeetCount(tm, time);  k++ )
	  {
	    meet = KheTimetableMonitorTimeMeet(tm, time, k);
	    if( KheMeetDuration(meet) < KheTimeGroupMonitorBusyCount(tgm) &&
		KheMeetAsstTime(meet) == time )
	    {
	      ** meet is not the only meet on its day **
	      if( KheTimeAugment(ej, meet, false,
		  &KheClusterToEmptyDayMeetMoveFn,
		  (void *) cbtm, false, ejecting, !ejecting, false) )
	      {
		if( DEBUG7 )
		  fprintf(stderr, "%*s] KheClusterUnderloadAugment ret true\n",
		    2*KheEjectorCurrDepth(ej), "");
		return;
	      }
	    }
	  }
	}
      }
    }
  }
  if( DEBUG7 )
    fprintf(stderr, "%*s] KheClusterUnderloadAugment ret false\n",
      2*KheEjectorCurrDepth(ej), "");
}
*** */


static void KheClusterUnderloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, index, extra;  KHE_RESOURCE r;  KHE_TIME_GROUP tg;
  KHE_TIME_GROUP_MONITOR tgm;  /* KHE_SOLN soln;  bool ejecting; */
  if( KheEjectorRepairTimes(ej) )
  {
    r = KheClusterBusyTimesMonitorResource(cbtm);
    /* soln = KheEjectorSoln(ej); */
    /* ejecting = KheEjectorEjectingNotBasic(ej); */
    if( DEBUG7 )
      fprintf(stderr, "%*s[ KheClusterUnderloadAugment(ej, %s)\n",
	2 * KheEjectorCurrDepth(ej), "", KheResourceId(r));
    extra = KheEjectorCurrAugmentCount(ej);
    for( i=0; i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm); i++ )
    {
      index = (extra+i) % KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
      tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, index);
      if( KheTimeGroupMonitorBusyCount(tgm) == 0 )
      {
	/* ***
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	*** */
	tg = KheTimeGroupMonitorTimeGroup(tgm);
        if( KheResourceUnderloadAugment(ej, r, tg, 0) )
	{
	  if( DEBUG7 )
	    fprintf(stderr, "%*s] KheClusterUnderloadAugment ret true\n",
	      2*KheEjectorCurrDepth(ej), "");
	  return;
	}
      }
    }
  }
  if( DEBUG7 )
    fprintf(stderr, "%*s] KheClusterUnderloadAugment ret false\n",
      2*KheEjectorCurrDepth(ej), "");
}


/* *** old version that should have used KheResourceLoadDefectAugment but didn't
static void KheClusterUnderloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, j, busy_count;  KHE_RESOURCE r;  KHE_MEET meet;  KHE_TIME_GROUP tg;
  KHE_TIME_GROUP_MONITOR tgm;  KHE_SOLN soln;  KHE_TASK task;  bool ejecting;
  if( KheEjectorRepairTimes(ej) )
  {
    r = KheClusterBusyTimesMonitorResource(cbtm);
    soln = KheMonitorSoln((KHE_MONITOR) cbtm);
    ejecting = KheEjectorEjectingNotBasic(ej);
    if( DEBUG7 )
      fprintf(stderr, "%*s[ KheClusterUnderloadAugment(ej, %s)\n",
	2*KheEjectorCurrDepth(ej), "", KheResourceId(r));
    for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
    {
      ** find a meet that can be moved **
      task = KheResourceAssignedTask(soln, r, i);
      meet = KheTaskMeet(task);
      if( DEBUG7 )
      {
	fprintf(stderr, "%*s  trying to move ",
	  2*KheEjectorCurrDepth(ej), "");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, ":\n");
      }
      for(j=0; j < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm); j++)
      {
	tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, j);
	tg = KheTimeGroupMonitorTimeGroup(tgm);
	busy_count = KheTimeGroupMonitorBusyCount(tgm);
	if( busy_count == 0 )
	{
	  if( DEBUG7 )
	  {
	    fprintf(stderr, "%*s  trying to move ",
	      2*KheEjectorCurrDepth(ej), "");
	    KheMeetDebug(meet, 1, -1, stderr);
	    fprintf(stderr, " into %s:\n", KheTimeGroupId(tg) == NULL ? "-" :
	      KheTimeGroupId(tg));
	  }
	  if( KheTimeAugment(ej, meet, true, &KheIn creaseOverlapMeetMoveFn,
	      (void *) tg, true, ejecting, !ejecting, false) )
	  {
	    if( DEBUG7 )
	      fprintf(stderr, "%*s] KheClusterUnderloadAugment ret true\n",
		2*KheEjectorCurrDepth(ej), "");
	    return;
	  }
	}
      }
    }
  }
  if( DEBUG7 )
    fprintf(stderr, "%*s] KheClusterUnderloadAugment ret false\n",
      2*KheEjectorCurrDepth(ej), "");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterToNonEmptyDayMeetMoveFn(KHE_MEET meet,                    */
/*    KHE_TIME time, void *impl)                                             */
/*                                                                           */
/*  Meet move function which allows the meet to move to any time on any      */
/*  non-empty day except its current day.  Here impl is a cluster busy       */
/*  times monitor plus the time group monitor for the current day.           */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete
typedef struct khe_cluster_info_rec {
  KHE_CLUSTER_BUSY_TIMES_MONITOR	cbtm;		** cluster monitor   **
  KHE_TIME_GROUP_MONITOR		tgm;		** don't move here   **
} *KHE_CLUSTER_INFO;

static bool KheClusterToNonEmptyDayMeetMoveFn(KHE_MEET meet,
  KHE_TIME time, void *impl)
{
  int i;  KHE_TIME_GROUP_MONITOR tgm;  KHE_CLUSTER_INFO ci;
  ci = (KHE_CLUSTER_INFO) impl;
  for( i=0; i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(ci->cbtm); i++)
  {
    tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(ci->cbtm, i);
    if( tgm != ci->tgm && KheTimeGroupMonitorBusyCount(tgm) > 0 &&
	KheTimeGroupContains(KheTimeGroupMonitorTimeGroup(tgm), time) )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterOverloadAugment(KHE_EJECTOR ej,                           */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Repair an overload defect in cbtm, by moving all the meets on one day.   */
/*                                                                           */
/*****************************************************************************/

static void KheClusterOverloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, busy_count, index, tgm_count, extra;  KHE_TIME_GROUP_MONITOR tgm;
  KHE_TIME_GROUP tg, include_tg;  KHE_TIMETABLE_MONITOR tm;
  KHE_RESOURCE r;  KHE_SOLN soln;
  if( DEBUG14 )
    fprintf(stderr, "[ KheClusterOverloadAugment(repair_times %s, depth %d)\n",
      KheEjectorRepairTimes(ej) ? "true" : "false", KheEjectorCurrDepth(ej));
  if( KheEjectorRepairTimes(ej) )
  {
    soln = KheEjectorSoln(ej);
    r = KheClusterBusyTimesMonitorResource(cbtm);
    tm = KheResourceTimetableMonitor(soln, r);
    tgm_count = KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
    extra = KheEjectorCurrAugmentCount(ej);
    for( i = 0;  i < tgm_count;  i++ )
    {
      index = (extra + i) % tgm_count;
      tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, index);
      busy_count = KheTimeGroupMonitorBusyCount(tgm);
      if( busy_count > 0 )
      {
	tg = KheTimeGroupMonitorTimeGroup(tgm);
	if( KheEjectorCurrDepth(ej)==1 || KheOneMeetOnly(tm, tg, busy_count) )
	{
	  if( KheEjectorCurrDepth(ej) == 1 )
	    KheSolnNewGlobalVisit(soln);
	  include_tg = KheClusterMeetsUnassignTimeGroup(cbtm, tg);
	  if( KheMeetBoundRepair(ej, r, include_tg,
	      KHE_REPAIR_CLUSTER_MEETS_UNASSIGN) )
	  {
	    if( DEBUG14 )
	      fprintf(stderr, "] KheClusterOverloadAugment returning 1\n");
	    return;
	  }
	}
      }
    }
  }
  if( DEBUG14 )
    fprintf(stderr, "] KheClusterOverloadAugment returning 2\n");
}


/* *** obsolete
static void KheClusterOverloadAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  int i, j, k, busy_count, index;  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME t;
  KHE_TIME_GROUP tg, include_tg; KHE_TIMETABLE_MONITOR tm;  KHE_RESOURCE r;
  KHE_SOLN soln; KHE_MEET meet;  bool ejecting;
  struct khe_cluster_info_rec cir;
  if( DEBUG14 )
    fprintf(stderr, "[ KheClusterOverloadAugment(repair_times %s, depth %d)\n",
      KheEjectorRepairTimes(ej) ? "true" : "false", KheEjectorCurrDepth(ej));
  soln = KheEjectorSoln(ej);
  if( KheEjectorRepairTimes(ej) )
  {
    r = KheClusterBusyTimesMonitorResource(cbtm);
    tm = KheResourceTimetableMonitor(soln, r);

    ** try moving individual meets **
    ejecting = KheEjectorEjectingNotBasic(ej);
    for( i = 0; i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm); i++)
    {
      index = (KheEjectorCurrAugmentCount(ej) + i) %
        KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
      tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, index);
      busy_count = KheTimeGroupMonitorBusyCount(tgm);
      if( busy_count > 0 )
      {
	tg = KheTimeGroupMonitorTimeGroup(tgm);
	for( j = 0;  j < KheTimeGroupTimeCount(tg);  j++ )
	{
	  t = KheTimeGroupTime(tg, j);
	  for( k = 0;  k < KheTimetableMonitorTimeMeetCount(tm, t);  k++ )
	  {
	    meet = KheTimetableMonitorTimeMeet(tm, t, k);
	    if( KheMeetAsstTime(meet)==t && KheMeetDuration(meet)==busy_count )
	    {
	      ** meet starts at t and is the only meet on this day **
	      ** ***
	      if( KheEjectorCurrDepth(ej) == 1 )
		KheSolnNewGlobalVisit(KheMeetSoln(meet));
	      *** **
	      cir.cbtm = cbtm;
	      cir.tgm = tgm;
	      if( KheTimeAugment(ej, meet, true,
		  &KheClusterToNonEmptyDayMeetMoveFn, (void *) &cir, false,
		  ejecting, !ejecting, false) )
	      {
		if( DEBUG14 )
		  fprintf(stderr, "] KheClusterOverloadAugment returning 1\n");
		return;
	      }
	    }
	  }
	}
      }
    }

    ** try clearing out entire days **
    if( KheEjectorCurrDepth(ej) == 1 )
    {
      for( i=0; i < KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm); i++)
      {
	index = (KheEjectorCurrAugmentCount(ej) + i) %
	  KheClusterBusyTimesMonitorTimeGroupMonitorCount(cbtm);
	tgm = KheClusterBusyTimesMonitorTimeGroupMonitor(cbtm, index);
	busy_count = KheTimeGroupMonitorBusyCount(tgm);
	if( busy_count > 0 )
	{
	  ** try clearing out this day, by reducing the domains of all of **
	  ** r's meets so that they don't occupy this day, now or ever,   **
	  ** and unassigning meets that are not on this day               **
	  ** ***
	  if( KheEjectorCurrDepth(ej) == 1 )
	    KheSolnNewGlobalVisit(soln);
	  *** **
	  tg = KheTimeGroupMonitorTimeGroup(tgm);
	  include_tg = KheClusterMeetsUnassignTimeGroup(cbtm, tg);
	  if( KheMeetBoundRepair(ej, r, include_tg,
	      KHE_REPAIR_CLUSTER_MEETS_UNASSIGN) )
	  {
	    if( DEBUG14 )
	      fprintf(stderr, "] KheClusterOverloadAugment returning 2\n");
	    return;
	  }
	}
      }
    }
  }

  if( DEBUG14 )
    fprintf(stderr, "] KheClusterOverloadAugment returning 3\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)           */
/*                                                                           */
/*  Augment function for individual cluster busy times defects.              */
/*                                                                           */
/*  If there is an underload, try moving any meet from a day when there      */
/*  are two or more meets to a day when there are none.  If there is an      */
/*  overload, try all Kempe/ejecting moves of meets from days in which       */
/*  they are alone to days in which they are not alone, plus ejection trees. */
/*                                                                           */
/*****************************************************************************/

static void KheClusterBusyTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm;
  int count, minimum, maximum;
  MAssert(KheMonitorTag(d) == KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG,
    "KheClusterBusyTimesAugment internal error 1");
  cbtm = (KHE_CLUSTER_BUSY_TIMES_MONITOR) d;
  KheClusterBusyTimesMonitorBusyGroupCount(cbtm, &count, &minimum, &maximum);
  if( count < minimum )
    KheClusterUnderloadAugment(ej, cbtm);
  else if( count > maximum )
    KheClusterOverloadAugment(ej, cbtm);
  else
    MAssert(false, "KheClusterBusyTimesAugment internal error 2");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)      */
/*                                                                           */
/*  Augment function for groups of cluster busy times defects.               */
/*                                                                           */
/*****************************************************************************/

static void KheClusterBusyTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheClusterBusyTimesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheClusterBusyTimesGroupAugment internal error 2");
  KheClusterBusyTimesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "limit busy times augments"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)             */
/*                                                                           */
/*  Augment function for individual limit busy times defects.                */
/*                                                                           */
/*  Try repairs documented in the headers of KheResourceOverloadAugment      */
/*  and KheResourceUnderloadAugment.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_LIMIT_BUSY_TIMES_MONITOR lbtm;  KHE_TIME_GROUP defective_tg, tg, tg2;
  KHE_RESOURCE r;  bool first;
  int i, j, k, busy_count, minimum, maximum;  ARRAY_KHE_TIME_GROUP time_groups;
  KHE_MONITOR m;  KHE_SOLN soln;  KHE_LIMIT_BUSY_TIMES_CONSTRAINT lbtc;
  if( DEBUG4 )
  {
    fprintf(stderr, "%*s[ KheLimitBusyTimesAugment(\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_LIMIT_BUSY_TIMES_MONITOR_TAG,
    "KheLimitBusyTimesAugment internal error 1");
  lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) d;
  r = KheLimitBusyTimesMonitorResource(lbtm);
  first = true;
  for( i = 0;  i < KheLimitBusyTimesMonitorDefectiveTimeGroupCount(lbtm);  i++ )
  {
    KheLimitBusyTimesMonitorDefectiveTimeGroup(lbtm, i, &defective_tg,
      &busy_count, &minimum, &maximum);
    /* ***
    if( KheEjectorCurrDepth(ej) == 1 )
      KheSolnNewGlobalVisit(KheMonitorSoln(d));
    *** */
    if( busy_count > maximum )
    {
      if( KheResourceOverloadAugment(ej, r, defective_tg) )
      {
	if( DEBUG4 )
	  fprintf(stderr, "%*s] KheLimitBusyTimesAugment ret true",
	    2 * KheEjectorCurrDepth(ej) + 2, "");
	return;
      }
    }
    else
    {
      if( KheResourceUnderloadAugment(ej, r, defective_tg, busy_count) )
      {
	if( DEBUG4 )
	  fprintf(stderr, "%*s] KheLimitBusyTimesAugment ret true",
	    2 * KheEjectorCurrDepth(ej) + 2, "");
	return;
      }
      if( WITH_EXTRA_LIMIT_BUSY_TIMES_REPAIRS &&
	  first && KheEjectorRepairTimes(ej) && KheEjectorCurrDepth(ej) == 1 )
      {
	/* find all the time groups of all r's limit busy times monitors */
	if( DEBUG17 )
	  fprintf(stderr, "[ extra limit busy times repairs:\n");
	first = false;
	soln = KheEjectorSoln(ej);
	MArrayInit(time_groups);
	for( i = 0;  i < KheSolnResourceMonitorCount(soln, r);  i++ )
	{
	  m = KheSolnResourceMonitor(soln, r, i);
	  if( KheMonitorTag(m) == KHE_LIMIT_BUSY_TIMES_MONITOR_TAG )
	  {
	    /* find the time groups of all of r's limit busy times monitors */
            lbtm = (KHE_LIMIT_BUSY_TIMES_MONITOR) m;
            lbtc = KheLimitBusyTimesMonitorConstraint(lbtm);
	    for( j=0; j < KheLimitBusyTimesConstraintTimeGroupCount(lbtc); j++ )
	    {
	      tg = KheLimitBusyTimesConstraintTimeGroup(lbtc, j);
	      if( !KheTimeGroupEqual(tg, defective_tg) )
	      {
		MArrayForEach(time_groups, &tg2, &k)
		  if( KheTimeGroupEqual(tg, tg2) )
		    break;
		if( k >= MArraySize(time_groups) )
		  MArrayAddLast(time_groups, tg);
	      }
	    }
	  }
	}

	/* try clearing out each of these time groups */
	MArrayForEach(time_groups, &tg, &k)
	{
	  if( KheEjectorCurrDepth(ej) == 1 )
	    KheSolnNewGlobalVisit(soln);
	  if( DEBUG17 )
	  {
	    fprintf(stderr, "  extra limit busy times repair: ");
	    KheTimeGroupDebug(tg, 1, 0, stderr);
	  }
	  if( KheMeetBoundRepair(ej, r, KheSolnComplementTimeGroup(soln, tg),
		KHE_REPAIR_LIMIT_BUSY_MEETS_UNASSIGN) )
	  {
	    MArrayFree(time_groups);
	    if( DEBUG17 )
	      fprintf(stderr, "] extra limit busy times repairs ret true\n");
	    return;
	  }
	}

	/* cleanup and end */
	if( DEBUG17 )
	  fprintf(stderr, "] extra limit busy times repairs ret false\n");
	MArrayFree(time_groups);
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheLimitBusyTimesAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)        */
/*                                                                           */
/*  Augment function for groups of limit busy times defects.                 */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheLimitBusyTimesGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheLimitBusyTimesGroupAugment internal error 2");
  KheLimitBusyTimesAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "limit workload augments"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadAugment(KHE_EJECTOR ej, KHE_MONITOR d)              */
/*                                                                           */
/*  Augment function for individual limit workload defects.                  */
/*                                                                           */
/*  Try repairs documented in the header of KheResourceLoadDefectAugment,    */
/*  which in this case amount to just one kind:  ejecting task moves of      */
/*  tasks assigned the overloaded resource to other resources.  At present   */
/*  there are no repairs for underload defects.                              */
/*                                                                           */
/*****************************************************************************/

static void KheLimitWorkloadAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_LIMIT_WORKLOAD_MONITOR lwm;  KHE_RESOURCE r;
  KHE_TIME_GROUP tg;  float workload;  int minimum, maximum;
  if( DEBUG3 )
  {
    fprintf(stderr, "%*s[ KheResourceRepairLimitWorkloadAugment (",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  lwm = (KHE_LIMIT_WORKLOAD_MONITOR) d;
  r = KheLimitWorkloadMonitorResource(lwm);
  tg = KheInstanceFullTimeGroup(KheResourceInstance(r));
  KheLimitWorkloadMonitorWorkloadAndLimits(lwm, &workload, &minimum, &maximum);
  if( workload > maximum )
  {
    if( KheResourceOverloadAugment(ej, r, tg) )
    {
      if( DEBUG3 )
	fprintf(stderr,
	  "%*s] KheResourceRepairLimitWorkloadAugment ret true (overload)\n",
	  2 * KheEjectorCurrDepth(ej) + 2, "");
      return;
    }
  }
  else
  {
    /* this repair actually does nothing, because of tg */
    if( KheResourceUnderloadAugment(ej, r, tg, minimum - 1) )
    {
      if( DEBUG3 )
	fprintf(stderr,
	  "%*s] KheResourceRepairLimitWorkloadAugment ret true (overload)\n",
	  2 * KheEjectorCurrDepth(ej) + 2, "");
      return;
    }
  }
  if( DEBUG3 )
    fprintf(stderr, "%*s] KheResourceRepairLimitWorkloadAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitWorkloadGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)         */
/*                                                                           */
/*  Augment function for groups of limit workload defects.                   */
/*                                                                           */
/*****************************************************************************/

static void KheLimitWorkloadGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheLimitWorkloadGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheLimitWorkloadGroupAugment internal error 2");
  KheLimitWorkloadAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ordinary and workload demand augments"                        */
/*                                                                           */
/*  Implementation note.  Ordinary and workload demand augments are          */
/*  treated the same, because for them it is the set of competitors which    */
/*  is the true defect, and it can have demand monitors of both kinds.       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)           */
/*                                                                           */
/*  Comparison function for sorting meets by increasing demand.              */
/*                                                                           */
/*****************************************************************************/

/* ***
static int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)
{
  KHE_MEET meet1 = * (KHE_MEET *) t1;
  KHE_MEET meet2 = * (KHE_MEET *) t2;
  if( KheMeetDemand(meet1) != KheMeetDemand(meet2) )
    return KheMeetDemand(meet1) - KheMeetDemand(meet2);
  else
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheDemandDefectIsWorkloadOverload(KHE_MONITOR d, KHE_RESOURCE *r,   */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If defect d has a workload demand defect competitor, set *r to the       */
/*  originating monitor's resource and *tg to its time group, and return     */
/*  true.  Otherwise return false.                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheDemandDefectIsWorkloadOverload(KHE_MONITOR d, KHE_RESOURCE *r,
  KHE_TIME_GROUP *tg)
{
  int i;  KHE_MONITOR m;  KHE_WORKLOAD_DEMAND_MONITOR wdm;  KHE_SOLN soln;
  soln = KheMonitorSoln(d);
  KheSolnMatchingSetCompetitors(soln, d);
  for( i = 0;  i < KheSolnMatchingCompetitorCount(soln);  i++ )
  {
    m = KheSolnMatchingCompetitor(soln, i);
    if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    {
      wdm = (KHE_WORKLOAD_DEMAND_MONITOR) m;
      *r = KheWorkloadDemandMonitorResource(wdm);
      *tg = KheWorkloadDemandMonitorTimeGroup(wdm);
      return true;
    }
  }
  *r = NULL;
  *tg = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)                     */
/*                                                                           */
/*  Augment function for individual ordinary or workload demand defects.     */
/*                                                                           */
/*  If the defect has a workload demand defect competitor, try repairs       */
/*  documented in the header of KheResourceLoadDefectAugment, which in this  */
/*  case means meet moves out of the domain of the workload monitor's        */
/*  originating monitor.                                                     */
/*                                                                           */
/*  If the defect has no workload demand defect competitor, try moving       */
/*  each of clashing meets away.                                             */
/*                                                                           */
/*****************************************************************************/

static void KheDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_TIME_GROUP tg;  KHE_MONITOR m;  KHE_MEET meet;  bool ejecting;
  KHE_OPTIONS_KEMPE kempe;  int i;  KHE_SOLN soln;  KHE_RESOURCE r;
  ARRAY_KHE_MEET meets;  KHE_ORDINARY_DEMAND_MONITOR odm;

  /* boilerplate */
  if( DEBUG6 )
  {
    fprintf(stderr, "%*s[ KheDemandAugment(",
      2 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(d, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheMonitorTag(d) == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
          KheMonitorTag(d) == KHE_WORKLOAD_DEMAND_MONITOR_TAG,
    "KheDemandAugment internal error 1");
  soln = KheEjectorSoln(ej);
  ejecting = KheEjectorEjectingNotBasic(ej);
  kempe = KheEjectorUseKempeMoves(ej);

  if( KheDemandDefectIsWorkloadOverload(d, &r, &tg) )
  {
    /* defect is workload overload of r in tg, so handle appropriately */
    if( KheResourceOverloadAugment(ej, r, tg) )
    {
      if( DEBUG6 )
	fprintf(stderr, "%*s] KheDemandAugment ret true: %.5f\n",
	  2*KheEjectorCurrDepth(ej) + 2, "", KheCostShow(KheSolnCost(soln)));
      return;
    }
  }
  else if( KheEjectorRepairTimes(ej) )
  {
    /* defect is ordinary overload; get meets */
    /* KheDemandDefectIsWorkloadOverload calls KheSolnMatchingSetCompetitors */
    MArrayInit(meets);
    for( i = 0;  i < KheSolnMatchingCompetitorCount(soln);  i++ )
    {
      m = KheSolnMatchingCompetitor(soln, i);
      MAssert(KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
	"KheDemandAugment internal error 1");
      odm = (KHE_ORDINARY_DEMAND_MONITOR) m;
      meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
      MArrayAddLast(meets, meet);
    }
    if( DEBUG6 )
      fprintf(stderr, "  KheDemandAugment: %d meets\n", MArraySize(meets));

    /* move meets */
    /* MArraySort(meets, &KheMeetIncreasingDemandCmp); */
    MArrayForEach(meets, &meet, &i)
    {
      /* this call to KheTimeAugment excludes moves at vizier nodes */
      if( KheTimeAugment(ej, meet, false, NULL, NULL, kempe, ejecting,
	  !ejecting, WITH_DEMAND_NODE_SWAPS) )
      {
	if( DEBUG6 )
	  fprintf(stderr, "%*s] KheDemandAugment ret true: %.5f\n",
	    2*KheEjectorCurrDepth(ej) + 2, "", KheCostShow(KheSolnCost(soln)));
	MArrayFree(meets);
	return;
      }
    }
    MArrayFree(meets);
  }

  /* no luck */
  if( DEBUG6 )
    fprintf(stderr, "%*s] KheDemandAugment ret false\n",
      2 * KheEjectorCurrDepth(ej) + 2, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)                */
/*                                                                           */
/*  Augment function for groups of ordinary or workload demand defects.      */
/*                                                                           */
/*****************************************************************************/

static void KheDemandGroupAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR gm;
  MAssert(KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG,
    "KheDemandGroupAugment internal error 1");
  gm = (KHE_GROUP_MONITOR) d;
  MAssert(KheGroupMonitorDefectCount(gm) > 0,
    "KheDemandGroupAugment internal error 2");
  KheDemandAugment(ej, KheGroupMonitorDefect(gm, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main functions"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR KheEjectionChainEjectorMake(KHE_OPTIONS options)             */
/*                                                                           */
/*  Make an ejector object.                                                  */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR KheEjectionChainEjectorMake(KHE_OPTIONS options)
{
  KHE_EJECTOR res;

  /* ejector and schedules */
  res = KheEjectorMakeBegin();
  KheEjectorSetSchedulesFromString(res,
    KheOptionsEjectorSchedulesString(options));

  /* augment types */
  KheEjectorAddAugmentType(res, KHE_AUGMENT_ORDINARY_DEMAND,
    "Ordinary demand");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_WORKLOAD_DEMAND,
    "Workload demand");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_SPLIT_EVENTS,
    "Split events");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_ASSIGN_TIME,
    "Assign time");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_PREFER_TIMES,
    "Prefer times");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_SPREAD_EVENTS,
    "Spread events");
  /* no function for link events defects */
  KheEjectorAddAugmentType(res, KHE_AUGMENT_ORDER_EVENTS,
    "Order events");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_ASSIGN_RESOURCE,
    "Assign resource");
  KheEjectorAddAugmentType(res,KHE_AUGMENT_PREFER_RESOURCES,
    "Prefer resources");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_AVOID_SPLIT_ASSIGNMENTS,
    "Avoid split assts");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_AVOID_CLASHES,
    "Avoid clashes");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_AVOID_UNAVAILABLE_TIMES,
    "Avoid unavailable times");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_LIMIT_IDLE_TIMES,
    "Limit idle times");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_CLUSTER_BUSY_TIMES,
    "Cluster busy times");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_LIMIT_BUSY_TIMES,
    "Limit busy times");
  KheEjectorAddAugmentType(res, KHE_AUGMENT_LIMIT_WORKLOAD,
    "Limit workload");

  /* repair types - meet moves and assignments */
  KheEjectorAddRepairType(res, KHE_REPAIR_KEMPE_MEET_MOVE,
    "Kempe meet move");
  KheEjectorAddRepairType(res, KHE_REPAIR_EJECTING_MEET_MOVE,
    "Ejecting meet move");
  KheEjectorAddRepairType(res, KHE_REPAIR_BASIC_MEET_MOVE,
    "Basic meet move");
  KheEjectorAddRepairType(res, KHE_REPAIR_FUZZY_MEET_MOVE,
    "Fuzzy meet move");
  KheEjectorAddRepairType(res, KHE_REPAIR_EJECTING_MEET_ASSIGN,
    "Ejecting meet assignment");
  KheEjectorAddRepairType(res, KHE_REPAIR_BASIC_MEET_ASSIGN,
    "Basic meet assignment");

  /* repair types - task moves and assignments */
  KheEjectorAddRepairType(res, KHE_REPAIR_EJECTING_TASK_MOVE,
    "Ejecting task move");
  KheEjectorAddRepairType(res, KHE_REPAIR_EJECTING_TASK_ASSIGN,
    "Ejecting task assignment");

  /* repair types - combined operations */
  KheEjectorAddRepairType(res, KHE_REPAIR_NODE_SWAP,
    "Node swap");
  KheEjectorAddRepairType(res, KHE_REPAIR_MEET_SPLIT,
    "Meet split");
  KheEjectorAddRepairType(res, KHE_REPAIR_SPLIT_MOVE,
    "Split move");
  KheEjectorAddRepairType(res, KHE_REPAIR_MERGE_MOVE,
    "Merge move");
  /* ***
  KheEjectorAddRepairType(res, KHE_REPAIR_EJECTING_TASK_SET_MOVE,
    "Ejecting task-set move");
  *** */
  KheEjectorAddRepairType(res,
    KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_ASSIGN,
    "Kempe meet move + ejecting task assignment");
  KheEjectorAddRepairType(res,
    KHE_REPAIR_KEMPE_MEET_MOVE_AND_EJECTING_TASK_MOVE,
    "Kempe meet move + ejecting task move");
  KheEjectorAddRepairType(res, KHE_REPAIR_SPLIT_TASKS_UNASSIGN,
    "Split tasks unassign and reduce");
  KheEjectorAddRepairType(res, KHE_REPAIR_COMPLEX_IDLE_MOVE,
    "Complex idle move");
  KheEjectorAddRepairType(res, KHE_REPAIR_CLUSTER_MEETS_UNASSIGN,
    "Cluster unassign and reduce");
  KheEjectorAddRepairType(res, KHE_REPAIR_LIMIT_BUSY_MEETS_UNASSIGN,
    "Limit busy unassign and reduce");
  KheEjectorAddRepairType(res, KHE_REPAIR_LIMIT_IDLE_MEETS_UNASSIGN,
    "Limit idle unassign and reduce");

  /* augment functions - demand monitors (and groups thereof) */
  KheEjectorAddAugment(res, KHE_ORDINARY_DEMAND_MONITOR_TAG, 
    &KheDemandAugment, KHE_AUGMENT_ORDINARY_DEMAND);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_ORDINARY_DEMAND,
    &KheDemandGroupAugment, KHE_AUGMENT_ORDINARY_DEMAND);

  KheEjectorAddAugment(res, KHE_WORKLOAD_DEMAND_MONITOR_TAG, 
    &KheDemandAugment, KHE_AUGMENT_WORKLOAD_DEMAND);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_WORKLOAD_DEMAND,
    &KheDemandGroupAugment, KHE_AUGMENT_WORKLOAD_DEMAND);

  /* augment functions - event monitors (and groups thereof) */
  KheEjectorAddAugment(res, KHE_SPLIT_EVENTS_MONITOR_TAG,
    &KheSplitEventsAugment, KHE_AUGMENT_SPLIT_EVENTS);
  KheEjectorAddAugment(res, KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG,
    &KheSplitEventsAugment, KHE_AUGMENT_SPLIT_EVENTS);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_SPLIT_EVENTS, 
    &KheSplitEventsGroupAugment, KHE_AUGMENT_SPLIT_EVENTS);

  KheEjectorAddAugment(res, KHE_ASSIGN_TIME_MONITOR_TAG,
    &KheAssignTimeAugment, KHE_AUGMENT_ASSIGN_TIME);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_ASSIGN_TIME, 
    &KheAssignTimeGroupAugment, KHE_AUGMENT_ASSIGN_TIME);

  KheEjectorAddAugment(res, KHE_PREFER_TIMES_MONITOR_TAG,
    &KhePreferTimesAugment, KHE_AUGMENT_PREFER_TIMES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_PREFER_TIMES, 
    &KhePreferTimesGroupAugment, KHE_AUGMENT_PREFER_TIMES);

  KheEjectorAddAugment(res, KHE_SPREAD_EVENTS_MONITOR_TAG,
    &KheSpreadEventsAugment, KHE_AUGMENT_SPREAD_EVENTS);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_SPREAD_EVENTS, 
    &KheSpreadEventsGroupAugment, KHE_AUGMENT_SPREAD_EVENTS);

  KheEjectorAddAugment(res, KHE_ORDER_EVENTS_MONITOR_TAG,
    &KheOrderEventsAugment, KHE_AUGMENT_ORDER_EVENTS);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_ORDER_EVENTS,
    &KheOrderEventsGroupAugment, KHE_AUGMENT_ORDER_EVENTS);

  /* augment functions - event resource monitors (and groups thereof) */
  KheEjectorAddAugment(res, KHE_ASSIGN_RESOURCE_MONITOR_TAG,
    &KheAssignResourceAugment, KHE_AUGMENT_ASSIGN_RESOURCE);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_ASSIGN_RESOURCE,
    &KheAssignResourceGroupAugment, KHE_AUGMENT_ASSIGN_RESOURCE);

  KheEjectorAddAugment(res, KHE_PREFER_RESOURCES_MONITOR_TAG,
    &KhePreferResourcesAugment, KHE_AUGMENT_PREFER_RESOURCES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_PREFER_RESOURCES,
    &KhePreferResourcesGroupAugment, KHE_AUGMENT_PREFER_RESOURCES);

  KheEjectorAddAugment(res, KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG,
    &KheAvoidSplitAssignmentsAugment, KHE_AUGMENT_AVOID_SPLIT_ASSIGNMENTS);

  /* augment functions - resource monitors (and groups thereof) */
  KheEjectorAddAugment(res, KHE_AVOID_CLASHES_MONITOR_TAG,
    &KheAvoidClashesAugment, KHE_AUGMENT_AVOID_CLASHES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_AVOID_CLASHES,
    &KheAvoidClashesGroupAugment, KHE_AUGMENT_AVOID_CLASHES);

  KheEjectorAddAugment(res, KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG,
    &KheAvoidUnavailableTimesAugment, KHE_AUGMENT_AVOID_UNAVAILABLE_TIMES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_AVOID_UNAVAILABLE_TIMES,
    &KheAvoidUnavailableTimesGroupAugment, KHE_AUGMENT_AVOID_UNAVAILABLE_TIMES);

  KheEjectorAddAugment(res, KHE_LIMIT_IDLE_TIMES_MONITOR_TAG,
    &KheLimitIdleTimesAugment, KHE_AUGMENT_LIMIT_IDLE_TIMES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_LIMIT_IDLE_TIMES,
    &KheLimitIdleTimesGroupAugment, KHE_AUGMENT_LIMIT_IDLE_TIMES);

  KheEjectorAddAugment(res, KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG,
    &KheClusterBusyTimesAugment, KHE_AUGMENT_CLUSTER_BUSY_TIMES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_CLUSTER_BUSY_TIMES,
    &KheClusterBusyTimesGroupAugment, KHE_AUGMENT_CLUSTER_BUSY_TIMES);

  KheEjectorAddAugment(res, KHE_LIMIT_BUSY_TIMES_MONITOR_TAG,
    &KheLimitBusyTimesAugment, KHE_AUGMENT_LIMIT_BUSY_TIMES);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_LIMIT_BUSY_TIMES,
    &KheLimitBusyTimesGroupAugment, KHE_AUGMENT_LIMIT_BUSY_TIMES);

  KheEjectorAddAugment(res, KHE_LIMIT_WORKLOAD_MONITOR_TAG,
    &KheLimitWorkloadAugment, KHE_AUGMENT_LIMIT_WORKLOAD);
  KheEjectorAddGroupAugment(res, KHE_SUBTAG_LIMIT_WORKLOAD,
    &KheLimitWorkloadGroupAugment, KHE_AUGMENT_LIMIT_WORKLOAD);

  /* wrap up */
  KheEjectorMakeEnd(res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainNodeRepairTimes(KHE_NODE parent_node,               */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Use an ejection chain to repair the time assignments of the meets of     */
/*  the descendants of parent_node.  The repair operation are Kempe meet     */
/*  moves (required to preserve zones, where present) and node swaps if      */
/*  nodes lie in layers.  Return true if any progress was made.              */
/*                                                                           */
/*  The ejector used is KheOptionsEjector(options, 0) if non-NULL, else      */
/*  a newly made ejector (which will be deleted at the end).                 */
/*                                                                           */
/*  If the time_vizier_node option is true, insert and split a vizier        */
/*  node before starting work, and remove it afterwards.                     */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainNodeRepairTimes(KHE_NODE parent_node, KHE_OPTIONS options)
{
  KHE_SOLN soln;  KHE_GROUP_MONITOR kempe_gm, start_gm, limit_gm;
  KHE_EJECTOR ej;  KHE_NODE vizier_node;  bool res;
  bool time_vizier_node = KheOptionsEjectorVizierNode(options);
  bool time_node_regularity = KheOptionsTimeNodeRegularity(options);

  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheEjectionChainNodeRepairTimes(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* get an ejector */
  ej = KheOptionsEjector(options, 0);
  if( ej == NULL )
    ej = KheEjectionChainEjectorMake(options);

  /* set the control options */
  KheOptionsSetEjectorRepairTimes(options, true);
  KheOptionsSetEjectorLimitNode(options,
    KheNodeIsCycleNode(parent_node) ? NULL : parent_node);
  KheOptionsSetEjectorRepairResources(options, false);

  /* insert and split a vizier node, if required */
  if( time_vizier_node )
  {
    vizier_node = KheNodeVizierMake(parent_node);
    KheNodeMeetSplit(vizier_node, false);
  }

  /* build the required group monitors and solve */
  soln = KheNodeSoln(parent_node);
  KheEjectionChainPrepareMonitors(soln);
  kempe_gm = KheKempeDemandGroupMonitorMake(soln);
  start_gm = KheNodeTimeRepairStartGroupMonitorMake(parent_node);
  limit_gm = KheGroupEventMonitors(soln, KHE_ASSIGN_TIME_MONITOR_TAG,
    KHE_SUBTAG_ASSIGN_TIME);
  if( DEBUG2 )
  {
    fprintf(stderr, "  initial defects:\n");
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }
  KheEjectorSolveBegin(ej, start_gm, (KHE_GROUP_MONITOR) soln, options);
  KheEjectorAddMonitorCostLimitReducing(ej, (KHE_MONITOR) limit_gm);
  res = KheEjectorSolveEnd(ej);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final defects:\n");
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }

  /* remove any vizier node and make sure the parent is fully zoned */
  if( time_vizier_node )
  {
    KheNodeVizierDelete(parent_node);
    if( time_node_regularity )
      KheNodeExtendZones(parent_node);
  }

  /* clean up, including deleting the group monitors, and return */
  if( KheOptionsEjector(options, 0) == NULL )
    KheEjectorDelete(ej);
  KheGroupMonitorDelete(kempe_gm);
  KheGroupMonitorDelete(start_gm);
  KheGroupMonitorDelete(limit_gm);
  KheEjectionChainUnPrepareMonitors(soln);
  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainNodeRepairTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainLayerRepairTimes(KHE_LAYER layer,                   */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Use an ejection chain to repair the time assignments of the meets of     */
/*  layer.  In other respects the same as KheEjectionChainNoderRepairTimes.  */
/*                                                                           */
/*  If the time_vizier_node option is true, insert and split a vizier        */
/*  node before starting work, and remove it afterwards.                     */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainLayerRepairTimes(KHE_LAYER layer, KHE_OPTIONS options)
{
  KHE_GROUP_MONITOR kempe_gm, start_gm, limit_gm;  bool res;
  KHE_SOLN soln;  KHE_EJECTOR ej;  KHE_NODE parent_node, vizier_node;
  bool time_vizier_node = KheOptionsEjectorVizierNode(options);
  bool time_node_regularity = KheOptionsTimeNodeRegularity(options);

  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheEjectionChainLayerRepairTimes(");
    KheLayerDebug(layer, 1, -1, stderr);
    fprintf(stderr, ")\n");
    KheLayerDebug(layer, 2, 2, stderr);
  }

  /* get an ejector */
  ej = KheOptionsEjector(options, 1);
  if( ej == NULL )
    ej = KheEjectionChainEjectorMake(options);

  /* set the control options */
  parent_node = KheLayerParentNode(layer);
  KheOptionsSetEjectorRepairTimes(options, true);
  KheOptionsSetEjectorLimitNode(options,
    KheNodeIsCycleNode(parent_node) ? NULL : parent_node);
  KheOptionsSetEjectorRepairResources(options, false);

  /* insert and split a vizier node, if required */
  if( time_vizier_node )
  {
    vizier_node = KheNodeVizierMake(parent_node);
    KheNodeMeetSplit(vizier_node, false);
  }

  /* build the required group monitors and solve */
  soln = KheLayerSoln(layer);
  KheEjectionChainPrepareMonitors(soln);
  kempe_gm = KheKempeDemandGroupMonitorMake(soln);
  if( KheOptionsTimeLayerRepairLong(options) )
    start_gm = KheLayerTimeRepairLongStartGroupMonitorMake(layer);
  else
    start_gm = KheLayerTimeRepairStartGroupMonitorMake(layer);
  limit_gm = KheGroupEventMonitors(soln, KHE_ASSIGN_TIME_MONITOR_TAG,
    KHE_SUBTAG_ASSIGN_TIME);
  if( DEBUG2 )
  {
    fprintf(stderr, "  initial defects (soln cost %.5f):\n",
      KheCostShow(KheSolnCost(soln)));
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }
  KheEjectorSolveBegin(ej, start_gm, (KHE_GROUP_MONITOR) soln, options);
  KheEjectorAddMonitorCostLimitReducing(ej, (KHE_MONITOR) limit_gm);
  res = KheEjectorSolveEnd(ej);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final defects (soln cost %.5f):\n",
      KheCostShow(KheSolnCost(soln)));
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }

  /* remove any vizier node and make sure the parent is fully zoned */
  if( time_vizier_node )
  {
    KheNodeVizierDelete(parent_node);
    if( time_node_regularity )
      KheNodeExtendZones(parent_node);
  }

  /* clean up, including deleting the group monitors, and return */
  if( KheOptionsEjector(options, 1) == NULL )
    KheEjectorDelete(ej);
  KheGroupMonitorDelete(kempe_gm);
  KheGroupMonitorDelete(start_gm);
  KheGroupMonitorDelete(limit_gm);
  KheEjectionChainUnPrepareMonitors(soln);
  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainLayerRepairTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainRepairResources(KHE_TASKING tasking,                */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Ejection chain local search for improving resource assts.  If the        */
/*  resource_invariant options is true, preserve the resource assignment     */
/*  invariant.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainRepairResources(KHE_TASKING tasking, KHE_OPTIONS options)
{
  KHE_EJECTOR ej;  KHE_GROUP_MONITOR kempe_gm, start_gm, limit_gm;
  KHE_SOLN soln;  /* KHE_RESOURCE_TYPE rt; */  bool res;
  bool resource_invariant = KheOptionsResourceInvariant(options);

  if( DEBUG1 )
    fprintf(stderr, "[ KheEjectionChainRepairResources(tasking)\n");

  /* get an ejector */
  ej = KheOptionsEjector(options, 2);
  if( ej == NULL )
    ej = KheEjectionChainEjectorMake(options);

  /* set the control options */
  KheOptionsSetEjectorRepairTimes(options, false);
  KheOptionsSetEjectorLimitNode(options, NULL);
  KheOptionsSetEjectorRepairResources(options, true);

  /* build the required group monitors and solve */
  soln = KheTaskingSoln(tasking);
  KheEjectionChainPrepareMonitors(soln);
  kempe_gm = KheKempeDemandGroupMonitorMake(soln);
  start_gm = KheTaskingStartGroupMonitorMake(tasking);
  if( DEBUG2 )
  {
    fprintf(stderr, "  initial defects:\n");
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }
  KheEjectorSolveBegin(ej, start_gm, (KHE_GROUP_MONITOR) soln, options);
  limit_gm = NULL;
  if( resource_invariant )
  {
    /* rt = KheTaskingResourceType(tasking); */
    limit_gm = KheAllDemandGroupMonitorMake(soln);
    KheEjectorAddMonitorCostLimitReducing(ej, (KHE_MONITOR) limit_gm);
  }
  res = KheEjectorSolveEnd(ej);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final defects:\n");
    KheGroupMonitorDefectDebug(start_gm, 2, 4, stderr);
  }

  /* clean up, including deleting the group monitors, and return */
  if( KheOptionsEjector(options, 2) == NULL )
    KheEjectorDelete(ej);
  KheGroupMonitorDelete(kempe_gm);
  if( resource_invariant )
    KheGroupMonitorDelete(limit_gm);
  KheGroupMonitorDelete(start_gm);
  KheEjectionChainUnPrepareMonitors(soln);
  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainRepairResources returning %s\n",
      res ? "true" : "false");
  return res;
}
