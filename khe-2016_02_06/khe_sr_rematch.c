
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
/*  FILE:         khe_sr_rematch.c                                           */
/*  DESCRIPTION:  Resource rematching                                        */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_wmatch.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_TIMESET                                                      */
/*                                                                           */
/*  A set of times.                                                          */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_TIME) ARRAY_KHE_TIME;

typedef struct khe_rematch_timeset_rec {
  ARRAY_KHE_TIME		times;		/* the times                 */
} *KHE_REMATCH_TIMESET;

typedef MARRAY(KHE_REMATCH_TIMESET) ARRAY_KHE_REMATCH_TIMESET;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_DEMAND_NODE                                                  */
/*                                                                           */
/*  One demand node of the rematch bipartite graph, representing a set of    */
/*  tasks.                                                                   */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;

typedef struct khe_rematch_demand_node_rec {
  bool			leftover;		/* true when no asst to task */
  ARRAY_KHE_TASK	tasks;			/* the tasks                 */
  int			preassigned_count;	/* preassigned tasks         */
  KHE_WMATCH_NODE	wmatch_node;		/* demand node in wmatch     */
} *KHE_REMATCH_DEMAND_NODE;

typedef MARRAY(KHE_REMATCH_DEMAND_NODE) ARRAY_KHE_REMATCH_DEMAND_NODE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_SUPPLY_NODE                                                  */
/*                                                                           */
/*  One supply node of the rematch bipartite graph, representing a resource. */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_rematch_supply_node_rec {
  KHE_RESOURCE			resource;	/* the resource              */
  KHE_REMATCH_DEMAND_NODE	demand_node;	/* corr. demand node         */
  KHE_WMATCH_NODE		wmatch_node;	/* supply node in wmatch     */
} *KHE_REMATCH_SUPPLY_NODE;

typedef MARRAY(KHE_REMATCH_SUPPLY_NODE) ARRAY_KHE_REMATCH_SUPPLY_NODE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_SOLVER                                                       */
/*                                                                           */
/*  A rematch solver.                                                        */
/*                                                                           */
/*  Implementation note.  The arrays of supply and demand nodes are          */
/*  organized as follows.                                                    */
/*                                                                           */
/*  (1) For each resource in the instance there is one place in the          */
/*      supply and demand node arrays.  If the resource has the wrong        */
/*      type, these places are both NULL.  If it has the right type,         */
/*      at that place there is a supply node holding the resource and a      */
/*      demand node holding the tasks initially assigned that resource.      */
/*                                                                           */
/*  (2) Beyond MArraySize(rs->supply_nodes) there may be additional          */
/*      demand nodes.  The next unused one is at rs->used_demand_count.      */
/*      These nodes are used for additional tasks not assigned a resource.   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_rematch_solver_rec {
  KHE_TASKING			tasking;		/* tasking to solve  */
  bool				resource_invariant;	/* preserve invt     */
  ARRAY_KHE_REMATCH_TIMESET	time_sets;		/* all time sets     */
  ARRAY_KHE_REMATCH_SUPPLY_NODE	supply_nodes;		/* supply nodes      */
  ARRAY_KHE_REMATCH_DEMAND_NODE	demand_nodes;		/* demand nodes      */
  int				used_demand_count;	/* next free demand  */
  /* KHE_TRANSACTION		unassign_t; */		/* transaction       */
  /* KHE_TRANSACTION		assign_t; */		/* transaction       */
} *KHE_REMATCH_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times and time sets"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheRematchTimeSetAddTimesRecursive(KHE_REMATCH_TIMESET rts,         */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Add to rts the times that task and the tasks assigned to task, directly  */
/*  or indirectly, are running.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchTimeSetAddTimesRecursive(KHE_REMATCH_TIMESET rts,
  KHE_TASK task)
{
  KHE_MEET meet;  KHE_TASK sub_task;  int i;  KHE_TIME starting_time;

  /* add the times of task */
  meet = KheTaskMeet(task);
  if( meet != NULL )
  {
    starting_time = KheMeetAsstTime(meet);
    if( starting_time != NULL )
    {
      for( i = 0;  i < KheMeetDuration(meet);  i++ )
      {
	MAssert(KheTimeHasNeighbour(starting_time, i),
	  "KheRematchTimeSetAddTimesRecursive: time off end");
	MArrayAddLast(rts->times, KheTimeNeighbour(starting_time, i));
      }
    }
  }

  /* recursively add the times of tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    sub_task = KheTaskAssignedTo(task, i);
    KheRematchTimeSetAddTimesRecursive(rts, sub_task);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeSetOverlapsTask(KHE_REMATCH_TIMESET rts, KHE_TASK task)      */
/*                                                                           */
/*  Return true if rts overlaps the assigned times of task.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeSetOverlapsTask(KHE_REMATCH_TIMESET rts, KHE_TASK task)
{
  KHE_MEET meet;  KHE_TASK sub_task;  int i, j;  KHE_TIME starting_time;

  /* test the times of task */
  meet = KheTaskMeet(task);
  if( meet != NULL )
  {
    starting_time = KheMeetAsstTime(meet);
    if( starting_time != NULL )
    {
      for( i = 0;  i < KheMeetDuration(meet);  i++ )
      {
	MAssert(KheTimeHasNeighbour(starting_time, i),
	  "KheRematchTimeSetAddTimesRecursive: time off end");
	if( MArrayContains(rts->times, KheTimeNeighbour(starting_time, i), &j) )
	  return true;
      }
    }
  }

  /* recursively test the times of tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    sub_task = KheTaskAssignedTo(task, i);
    if( KheTimeSetOverlapsTask(rts, sub_task) )
      return true;
  }

  /* no luck */
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeIncreasingIndexCmp(const void *p1, const void *p2)            */
/*                                                                           */
/*  Comparison function for sorting an array of times into increasing        */
/*  index order.                                                             */
/*                                                                           */
/*****************************************************************************/

static int KheTimeIncreasingIndexCmp(const void *p1, const void *p2)
{
  KHE_TIME time1 = * (KHE_TIME *) p1;
  KHE_TIME time2 = * (KHE_TIME *) p2;
  return KheTimeIndex(time1) - KheTimeIndex(time2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheRematchTimeSetCmp(const void *p1, const void *p2)                 */
/*                                                                           */
/*  Comparison function for sorting an array of time sets.  It doesn't       */
/*  matter, really, what the order is; it's just for uniqueifying.           */
/*                                                                           */
/*****************************************************************************/

static int KheRematchTimeSetCmp(const void *p1, const void *p2)
{
  KHE_REMATCH_TIMESET rts1 = * (KHE_REMATCH_TIMESET *) p1;
  KHE_REMATCH_TIMESET rts2 = * (KHE_REMATCH_TIMESET *) p2;
  KHE_TIME time1, time2;  int i;
  if( MArraySize(rts1->times) != MArraySize(rts2->times) )
    return MArraySize(rts1->times) - MArraySize(rts2->times);
  for( i = 0;  i < MArraySize(rts1->times);  i++ )
  {
    time1 = MArrayGet(rts1->times, i);
    time2 = MArrayGet(rts2->times, i);
    if( time1 != time2 )
      return KheTimeIndex(time1) - KheTimeIndex(time2);
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_TIMESET KheRematchTimeSetMake(KHE_TASK task)                 */
/*                                                                           */
/*  Make a new rematch time set containing the times that task is running.   */
/*                                                                           */
/*****************************************************************************/

static KHE_REMATCH_TIMESET KheRematchTimeSetMake(KHE_TASK task)
{
  KHE_REMATCH_TIMESET res;

  /* initialize the object */
  MMake(res);
  MArrayInit(res->times);

  /* add the times of task and sort them */
  KheRematchTimeSetAddTimesRecursive(res, task);
  MArraySortUnique(res->times, &KheTimeIncreasingIndexCmp);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchTimeSetDelete(KHE_REMATCH_TIMESET rts)                    */
/*                                                                           */
/*  Delete rts.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchTimeSetDelete(KHE_REMATCH_TIMESET rts)
{
  MArrayFree(rts->times);
  MFree(rts);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchTimeSetDebug(KHE_REMATCH_TIMESET rts, FILE *fp)           */
/*                                                                           */
/*  Debug print of rts onto fp.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchTimeSetDebug(KHE_REMATCH_TIMESET rts, FILE *fp)
{
  KHE_TIME time;  int i;
  fprintf(fp, "[");
  MArrayForEach(rts->times, &time, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    fprintf(fp, "%s", KheTimeName(time));
  }
  fprintf(fp, "]");
}


/*****************************************************************************/
/*                                                                           */
/*  bool TaskAssignmentIsDefective(KHE_TASK task)                            */
/*                                                                           */
/*  Return true if task or any task assigned to task is defective.           */
/*                                                                           */
/*****************************************************************************/

static bool TaskAssignmentIsDefective(KHE_TASK task)
{
  KHE_TASK sub_task;  int i;  KHE_ORDINARY_DEMAND_MONITOR odm;
  KHE_SOLN soln;  KHE_MONITOR m;  KHE_EVENT_RESOURCE er;

  /* check whether this task has a defective demand monitor (a clash) */
  for( i = 0;  i < KheTaskDemandMonitorCount(task);  i++ )
  {
    odm = KheTaskDemandMonitor(task, i);
    if( KheMonitorCost((KHE_MONITOR) odm) > 0 )
      return true;
  }

  /* check whether this task has a defective event resource monitor */
  er = KheTaskEventResource(task);
  soln = KheTaskSoln(task);
  for( i = 0;  i < KheSolnEventResourceMonitorCount(soln, er);  i++ )
  {
    m = KheSolnEventResourceMonitor(soln, er, i);
    if( KheMonitorCost(m) > 0 )
      return true;
  }

  /* check for defective sub-tasks */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    sub_task = KheTaskAssignedTo(task, i);
    if( TaskAssignmentIsDefective(sub_task) )  /* bug fix here; was task */
      return true;
  }

  /* no sign of any defects */
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchBuildTimeSets(KHE_TASKING tasking,                        */
/*    ARRAY_KHE_REMATCH_TIMESET *time_sets)                                  */
/*                                                                           */
/*  Add time sets for the defective tasks of tasking to time_sets, and       */
/*  and uniquefy the time sets.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchBuildTimeSets(KHE_TASKING tasking,
  ARRAY_KHE_REMATCH_TIMESET *time_sets)
{
  KHE_TASK task;  int i;

  /* add one time set for each defective task */
  if( DEBUG2 )
    fprintf(stderr, "[ KheRematchBuildTimeSets(soln, rt, *time_sets)\n");
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( TaskAssignmentIsDefective(task) )
      MArrayAddLast(*time_sets, KheRematchTimeSetMake(task));
  }

  /* sort and uniqueify the time sets */
  /* there is a memory leak here when we drop a time set */
  MArraySortUnique(*time_sets,  &KheRematchTimeSetCmp);
  if( DEBUG2 )
    fprintf(stderr, "] KheRematchBuildTimeSets returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand nodes"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_DEMAND_NODE KheRematchDemandNodeMake(bool leftover)          */
/*                                                                           */
/*  Make a new demand node with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

static KHE_REMATCH_DEMAND_NODE KheRematchDemandNodeMake(bool leftover)
{
  KHE_REMATCH_DEMAND_NODE res;
  MMake(res);
  res->leftover = leftover;
  MArrayInit(res->tasks);
  res->preassigned_count = 0;
  res->wmatch_node = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRematchDemandNodeParticipates(KHE_REMATCH_DEMAND_NODE rdn)       */
/*                                                                           */
/*  Return true if rdn participates in the matching.  This will be the       */
/*  case when it is not preassigned and contains at least one task.          */
/*                                                                           */
/*****************************************************************************/

static bool KheRematchDemandNodeParticipates(KHE_REMATCH_DEMAND_NODE rdn)
{
  return rdn->preassigned_count == 0 && MArraySize(rdn->tasks) > 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeClear(KHE_REMATCH_DEMAND_NODE rdn)              */
/*                                                                           */
/*  Clear rdn.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeClear(KHE_REMATCH_DEMAND_NODE rdn)
{
  MArrayClear(rdn->tasks);
  rdn->preassigned_count = 0;
  rdn->wmatch_node = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeAddTask(KHE_REMATCH_DEMAND_NODE rdn,            */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Add task to rdn.                                                         */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeAddTask(KHE_REMATCH_DEMAND_NODE rdn,
  KHE_TASK task)
{
  MArrayAddLast(rdn->tasks, task);
  if( KheTaskAssignIsFixed(task) || KheTaskIsPreassigned(task, NULL) )
  {
    MAssert(!rdn->leftover, "KheRematchDemandNodeAddTask internal error");
    rdn->preassigned_count++;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeUnAssignTasks(KHE_REMATCH_DEMAND_NODE rdn)      */
/*                                                                           */
/*  Unassign the tasks of rdn.  They are not fixed, so this must succeed.    */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeUnAssignTasks(KHE_REMATCH_DEMAND_NODE rdn)
{
  KHE_TASK task;  int i;
  MAssert(rdn->preassigned_count == 0,
    "KheRematchDemandNodeUnAssignTasks internal error 1");
  MArrayForEachReverse(rdn->tasks, &task, &i)
    if( !KheTaskUnAssignResource(task) )
      MAssert(false, "KheRematchDemandNodeUnAssignTasks internal error 2");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRematchDemandNodeAssignTasks(KHE_REMATCH_DEMAND_NODE rdn,        */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  If possible, assign r to all the tasks of rdn and return true.  Else     */
/*  assign none of them and return false.                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheRematchDemandNodeAssignTasks(KHE_REMATCH_DEMAND_NODE rdn,
  KHE_RESOURCE r)
{
  KHE_TASK task;  int i;
  MArrayForEach(rdn->tasks, &task, &i)
    if( !KheTaskAssignResource(task, r) )
    {
      for( i = i - 1;  i >= 0;  i-- )
      {
	task = MArrayGet(rdn->tasks, i);
	if( !KheTaskUnAssignResource(task) )
	  MAssert(false, "KheRematchDemandNodeAssignTasks internal error");
      }
      return false;
    }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeAddWMatchNode(KHE_REMATCH_DEMAND_NODE rdn,      */
/*    KHE_WMATCH m)                                                          */
/*                                                                           */
/*  Add a wmatch node corresponding to rdn to m.                             */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeAddWMatchNode(KHE_REMATCH_DEMAND_NODE rdn,
  KHE_WMATCH m)
{
  rdn->wmatch_node = KheWMatchDemandNodeMake(m, (void *) rdn, NULL, 0);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeDelete(KHE_REMATCH_DEMAND_NODE rdn)             */
/*                                                                           */
/*  Delete rdn.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeDelete(KHE_REMATCH_DEMAND_NODE rdn)
{
  MArrayFree(rdn->tasks);
  MFree(rdn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchDemandNodeDebug(KHE_REMATCH_DEMAND_NODE rdn,              */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print of rdn onto fp.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchDemandNodeDebug(KHE_REMATCH_DEMAND_NODE rdn,
  FILE *fp)
{
  int i;  KHE_TASK task;
  fprintf(fp, "%s%s%c{", rdn->leftover ? "--leftover-- " : "",
    rdn->preassigned_count > 0 ? "--preass-- " : "",
    rdn->wmatch_node != NULL ? '*' : ' ');
  MArrayForEach(rdn->tasks, &task, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    KheTaskDebug(task, 1, -1, fp);
  }
  fprintf(fp, "}");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "supply nodes"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_SUPPLY_NODE KheRematchSupplyNodeMake(KHE_RESOURCE r,         */
/*    KHE_REMATCH_DEMAND_NODE rdn)                                           */
/*                                                                           */
/*  Make a new supply node representing r, not initially in any wmatch;      */
/*  rdn is the corresponding demand node.                                    */
/*                                                                           */
/*****************************************************************************/

static KHE_REMATCH_SUPPLY_NODE KheRematchSupplyNodeMake(KHE_RESOURCE r,
  KHE_REMATCH_DEMAND_NODE rdn)
{
  KHE_REMATCH_SUPPLY_NODE res;
  MMake(res);
  res->resource = r;
  res->demand_node = rdn;
  res->wmatch_node = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSupplyNodeClear(KHE_REMATCH_SUPPLY_NODE rsn)              */
/*                                                                           */
/*  Clear rsn.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSupplyNodeClear(KHE_REMATCH_SUPPLY_NODE rsn)
{
  rsn->wmatch_node = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRematchSupplyNodeParticipates(KHE_REMATCH_SUPPLY_NODE rsn)       */
/*                                                                           */
/*  Return true if rsn participates in the match, which will be the case     */
/*  when the corresponding demand node is not preassigned.                   */
/*                                                                           */
/*****************************************************************************/

static bool KheRematchSupplyNodeParticipates(KHE_REMATCH_SUPPLY_NODE rsn)
{
  return rsn->demand_node->preassigned_count == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSupplyNodeAddWMatchNode(KHE_REMATCH_SUPPLY_NODE rsn,      */
/*    KHE_WMATCH m)                                                          */
/*                                                                           */
/*  Add a wmatch node corresponding to rsn to m.                             */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSupplyNodeAddWMatchNode(KHE_REMATCH_SUPPLY_NODE rsn,
  KHE_WMATCH m)
{
  rsn->wmatch_node = KheWMatchSupplyNodeMake(m, (void *) rsn, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSupplyNodeDelete(KHE_REMATCH_SUPPLY_NODE rsn)             */
/*                                                                           */
/*  Delete rdn.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSupplyNodeDelete(KHE_REMATCH_SUPPLY_NODE rsn)
{
  MFree(rsn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSupplyNodeDebug(KHE_REMATCH_SUPPLY_NODE rsn,              */
/*    int col_width, FILE *fp)                                               */
/*                                                                           */
/*  Debug print of rsn onto fp in the given column width.                    */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSupplyNodeDebug(KHE_REMATCH_SUPPLY_NODE rsn,
  int col_width, FILE *fp)
{
  fprintf(fp, "%c%-*s", rsn->wmatch_node != NULL ? '*' : ' ',
    col_width, KheResourceName(rsn->resource));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "rematch solver"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_REMATCH_SOLVER KheRematchSolverMake(KHE_TASKING tasking,             */
/*    bool resource_invariant)                                               */
/*                                                                           */
/*  Make a rematch solver with these attributes.                             */
/*                                                                           */
/*****************************************************************************/

static KHE_REMATCH_SOLVER KheRematchSolverMake(KHE_TASKING tasking,
  bool resource_invariant)
{
  KHE_REMATCH_SOLVER res;  int i;  KHE_INSTANCE ins;  KHE_RESOURCE r;
  KHE_RESOURCE_TYPE rt;  KHE_REMATCH_SUPPLY_NODE supply_node;
  KHE_REMATCH_DEMAND_NODE demand_node;

  MMake(res);
  res->tasking = tasking;
  res->resource_invariant = resource_invariant;
  MArrayInit(res->time_sets);
  KheRematchBuildTimeSets(tasking, &res->time_sets);
  MArrayInit(res->supply_nodes);
  MArrayInit(res->demand_nodes);
  if( MArraySize(res->time_sets) > 0 )
  {
    ins = KheSolnInstance(KheTaskingSoln(tasking));
    rt = KheTaskingResourceType(tasking);
    for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
    {
      r = KheInstanceResource(ins, i);
      if( rt == NULL || KheResourceResourceType(r) == rt )
      {
	demand_node = KheRematchDemandNodeMake(false);
	supply_node = KheRematchSupplyNodeMake(r, demand_node);
      }
      else
      {
	demand_node = NULL;
	supply_node = NULL;
      }
      MArrayAddLast(res->supply_nodes, supply_node);
      MArrayAddLast(res->demand_nodes, demand_node);
    }
  }
  res->used_demand_count = MArraySize(res->supply_nodes);
  /* ***
  res->unassign_t = KheTransactionMake(KheTaskingSoln(tasking));
  res->assign_t = KheTransactionMake(KheTaskingSoln(tasking));
  *** */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSolverDelete(KHE_REMATCH_SOLVER rs)                       */
/*                                                                           */
/*  Delete rs.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSolverDelete(KHE_REMATCH_SOLVER rs)
{
  KHE_REMATCH_TIMESET rts;  KHE_REMATCH_SUPPLY_NODE rsn;  int i;
  KHE_REMATCH_DEMAND_NODE rdn;
  MArrayForEach(rs->time_sets, &rts, &i)
    KheRematchTimeSetDelete(rts);
  MArrayFree(rs->time_sets);
  MArrayForEach(rs->supply_nodes, &rsn, &i)
    if( rsn != NULL )
      KheRematchSupplyNodeDelete(rsn);
  MArrayFree(rs->supply_nodes);
  MArrayForEach(rs->demand_nodes, &rdn, &i)
    if( rdn != NULL )
      KheRematchDemandNodeDelete(rdn);
  MArrayFree(rs->demand_nodes);
  /* ***
  KheTransactionDelete(rs->unassign_t);
  KheTransactionDelete(rs->assign_t);
  *** */
  MFree(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchCostDebug(KHE_COST cost, FILE *fp)                        */
/*                                                                           */
/*  Debug function for displaying cost.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheRematchCostDebug(KHE_COST cost, FILE *fp)
{
  fprintf(fp, "%.5f", KheCostShow(cost));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceRematchEdgeFn(void *demand_back, void *supply_back,      */
/*    int64_t *cost)                                                         */
/*                                                                           */
/*  Edge function for wmatch.                                                */
/*                                                                           */
/*****************************************************************************/

bool KheResourceRematchEdgeFn(void *demand_back, void *supply_back,
  int64_t *cost)
{
  KHE_REMATCH_DEMAND_NODE rdn;  KHE_REMATCH_SUPPLY_NODE rsn;
  KHE_SOLN soln;  KHE_COST init_cost;

  /* boilerplate */
  MAssert(cost != NULL, "KheResourceRematchEdgeFn internal error 1");
  rdn = (KHE_REMATCH_DEMAND_NODE) demand_back;
  rsn = (KHE_REMATCH_SUPPLY_NODE) supply_back;
  MAssert(MArraySize(rdn->tasks) > 0,
    "KheResourceRematchEdgeFn internal error 2");
  soln = KheTaskSoln(MArrayFirst(rdn->tasks));
  init_cost = KheSolnCost(soln);

  if( DEBUG4 )
  {
    fprintf(stderr, "  edge ");
    KheRematchSupplyNodeDebug(rsn, 20, stderr);
    fprintf(stderr, " --> ");
    KheRematchDemandNodeDebug(rdn, stderr);
  }

  /* try assigning each task in turn, return false if any fail */
  if( !KheRematchDemandNodeAssignTasks(rdn, rsn->resource) )
  {
    MAssert(init_cost == KheSolnCost(soln),
      "KheResourceRematchEdgeFn internal error 4");
    if( DEBUG4 )
      fprintf(stderr, " false\n");
    return false;
  }

  /* success, so set *cost, unassign, and return true */
  *cost = KheSolnCost(soln);
  KheRematchDemandNodeUnAssignTasks(rdn);
  MAssert(init_cost == KheSolnCost(soln),
    "KheResourceRematchEdgeFn internal error 6");
  if( DEBUG4 )
    fprintf(stderr, " true %.5f\n", KheCostShow(*cost));
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSolverSolveTimeSet(KHE_REMATCH_SOLVER rs,                 */
/*    KHE_REMATCH_TIMESET rts)                                               */
/*                                                                           */
/*  Carry out a rematch of the tasks of rs->tasking that intersect with rts. */
/*                                                                           */
/*****************************************************************************/
static void KheRematchSolverDebug(KHE_REMATCH_SOLVER rs,
  int verbosity, int indent, FILE *fp);

static void KheRematchSolverSolveTimeSet(KHE_REMATCH_SOLVER rs,
  KHE_REMATCH_TIMESET rts)
{
  KHE_SOLN soln;  int i;  KHE_REMATCH_SUPPLY_NODE rsn;  KHE_RESOURCE r;
  KHE_REMATCH_DEMAND_NODE rdn;  KHE_TASK task;  bool something_to_do;
  KHE_COST init_cost, c;  KHE_WMATCH m;  KHE_MARK init_mark;

  soln = KheTaskingSoln(rs->tasking);
  init_cost = KheSolnCost(soln);
  if( DEBUG3 )
  {
    fprintf(stderr, "  [ KheRematchSolverSolveTimeSet(rs, ");
    KheRematchTimeSetDebug(rts, stderr);
    fprintf(stderr, "): init cost %.5f\n", KheCostShow(init_cost));
  }

  /* clear out any old stuff */
  m = NULL;
  MArrayForEach(rs->supply_nodes, &rsn, &i)
    if( rsn != NULL )
      KheRematchSupplyNodeClear(rsn);
  MArrayForEach(rs->demand_nodes, &rdn, &i)
    if( rdn != NULL )
      KheRematchDemandNodeClear(rdn);
  rs->used_demand_count = MArraySize(rs->supply_nodes);

  /* add the participating tasks to the demand nodes */
  for( i = 0;  i < KheTaskingTaskCount(rs->tasking);  i++ )
  {
    task = KheTaskingTask(rs->tasking, i);
    if( KheTimeSetOverlapsTask(rts, task) )
    {
      /* we're adding task, either across from resource or leftover */
      r = KheTaskAsstResource(task);
      if( r != NULL )
      {
	/* use an existing demand node corresponding to r */
	rdn = MArrayGet(rs->demand_nodes, KheResourceInstanceIndex(r));
      }
      else
      {
	/* use an extra demand node holding just this one task */
	if( rs->used_demand_count >= MArraySize(rs->demand_nodes) )
	{
	  rdn = KheRematchDemandNodeMake(true);
	  MArrayAddLast(rs->demand_nodes, rdn);
	}
	MAssert(rs->used_demand_count < MArraySize(rs->demand_nodes),
	  "KheRematchSolverSolveTimeSet: internal error 2");
	rdn = MArrayGet(rs->demand_nodes, rs->used_demand_count);
	rs->used_demand_count++;
      }
      KheRematchDemandNodeAddTask(rdn, task);
    }
  }

  /* unassign the participating demand nodes, when assigned */
  something_to_do = false;
  /* KheTransactionBegin(rs->unassign_t); */
  init_mark = KheMarkBegin(soln);
  MArrayForEach(rs->demand_nodes, &rdn, &i)
    if( rdn != NULL && KheRematchDemandNodeParticipates(rdn) )
    {
      something_to_do = true;
      if( !rdn->leftover )
	KheRematchDemandNodeUnAssignTasks(rdn);
    }
  /* KheTransactionEnd(rs->unassign_t); */

  /* reassign them using weighted bipartite matching */
  if( something_to_do )
  {
    /* build the wmatch and add supply and demand nodes */
    m = KheWMatchMake(rs, NULL, NULL, NULL, &KheResourceRematchEdgeFn,
      &KheRematchCostDebug, 0);
    MArrayForEach(rs->supply_nodes, &rsn, &i)
      if( rsn != NULL && KheRematchSupplyNodeParticipates(rsn) )
	KheRematchSupplyNodeAddWMatchNode(rsn, m);
    MArrayForEach(rs->demand_nodes, &rdn, &i)
      if( rdn != NULL && KheRematchDemandNodeParticipates(rdn) )
	KheRematchDemandNodeAddWMatchNode(rdn, m);
    if( DEBUG3 )
      KheRematchSolverDebug(rs, 1, 4, stderr);

    /* extract the edges and assign them */
    /* KheTransactionBegin(rs->assign_t); */
    MArrayForEach(rs->demand_nodes, &rdn, &i)
      if( rdn != NULL && KheRematchDemandNodeParticipates(rdn) )
      {
	rsn = KheWMatchDemandNodeAssignedTo(rdn->wmatch_node, &c);
	if( rsn != NULL )
	{
	  if( !KheRematchDemandNodeAssignTasks(rdn, rsn->resource) )
	    MAssert(false, "KheResourceRematchTimeSet internal error");
	}
      }
    /* KheTransactionEnd(rs->assign_t); */

    /* if new solution is not better, return to original solution */
    KheMarkEnd(init_mark, KheSolnCost(soln) >= init_cost);
    /* ***
    if( KheSolnCost(soln) >= init_cost )
    {
      KheTransactionUndo(rs->assign_t);
      KheTransactionUndo(rs->unassign_t);
    }
    *** */

    /* delete the wmatch */
    KheWMatchDelete(m);
  }

  MAssert(KheSolnCost(soln) <= init_cost,
    "KheResourceRematchTimeSet internal error 2");
  if( DEBUG3 )
  {
    if( KheSolnCost(soln) < init_cost )
      fprintf(stderr,
	"  ] KheResourceRematchTimeSet returning success: %.5f -> %.5f\n",
	KheCostShow(init_cost), KheCostShow(KheSolnCost(soln)));
    else
      fprintf(stderr,
	"  ] KheResourceRematchTimeSet returning fail: %.5f\n",
	KheCostShow(KheSolnCost(soln)));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSolverOneLineDebug(KHE_REMATCH_SUPPLY_NODE rsn,           */
/*    KHE_REMATCH_DEMAND_NODE rdn, int indent, FILE *fp)                     */
/*                                                                           */
/*  Debug print of one line of the resource solver onto fp with the given    */
/*  indent.  Either or both of rsn and rdn may be NULL.                      */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSolverOneLineDebug(KHE_REMATCH_SUPPLY_NODE rsn,
  KHE_REMATCH_DEMAND_NODE rdn, int indent, FILE *fp)
{
  int col_width = 20;
  if( rsn != NULL || rdn != NULL )
  {
    fprintf(fp, "%*s", indent, "");
    if( rsn != NULL )
      KheRematchSupplyNodeDebug(rsn, col_width, fp);
    else
      fprintf(fp, "%*s", col_width, "");
    if( rdn != NULL )
      KheRematchDemandNodeDebug(rdn, fp);
    fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRematchSolverDebug(KHE_REMATCH_SOLVER rs,                        */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Generate a debug print of rs onto fp with the given verbosity and        */
/*  indent.  Given the size of the print, we must have indent >= 0.          */
/*                                                                           */
/*****************************************************************************/

static void KheRematchSolverDebug(KHE_REMATCH_SOLVER rs,
  int verbosity, int indent, FILE *fp)
{
  KHE_RESOURCE_TYPE rt;  int i;  KHE_REMATCH_DEMAND_NODE rdn;
  KHE_REMATCH_SUPPLY_NODE rsn;
  MAssert(indent >= 0, "KheRematchSolverDebug: negative indent");
  rt = KheTaskingResourceType(rs->tasking);
  fprintf(stderr, "%*s[ RematchSolver(%s, %s)\n", indent, "",
    rt == NULL ? "NULL" : KheResourceTypeName(rt) == NULL ? "?" :
    KheResourceTypeName(rt), rs->resource_invariant ? "true" : "false");
  MArrayForEach(rs->demand_nodes, &rdn, &i)
  {
    if( i < MArraySize(rs->supply_nodes) )
      rsn = MArrayGet(rs->supply_nodes, i);
    else
      rsn = NULL;
    KheRematchSolverOneLineDebug(rsn, rdn, indent + 2, fp);
  }
  fprintf(stderr, "%*s]\n", indent, "");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main function"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceRematch(KHE_TASKING tasking, KHE_OPTIONS options)        */
/*                                                                           */
/*  Carry out resource rematching on the defective tasks of tasking.         */
/*                                                                           */
/*****************************************************************************/

bool KheResourceRematch(KHE_TASKING tasking, KHE_OPTIONS options)
{
  KHE_REMATCH_TIMESET rts;  int i;  KHE_SOLN soln;  KHE_REMATCH_SOLVER rs;
  bool resource_invariant = KheOptionsResourceInvariant(options);

  soln = KheTaskingSoln(tasking);
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceRematch(tasking, %s) init soln cost %.5f\n",
      resource_invariant ? "true" : "false", KheCostShow(KheSolnCost(soln)));

  /* build a solver */
  rs = KheRematchSolverMake(tasking, resource_invariant);

  /* solve for each time set */
  MArrayForEach(rs->time_sets, &rts, &i)
    KheRematchSolverSolveTimeSet(rs, rts);

  /* free the solver */
  KheRematchSolverDelete(rs);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceRematch returning (final soln cost %.5f)\n",
      KheCostShow(KheSolnCost(soln)));
  return true;
}
