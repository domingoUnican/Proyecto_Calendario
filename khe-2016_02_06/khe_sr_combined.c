
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
/*  FILE:         khe_sr_combined.c                                          */
/*  DESCRIPTION:  Combined resource solvers                                  */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_priqueue.h"
#include <limits.h>

#define DEBUG1 0	/* entry and exit of each main call */
#define DEBUG2 0	/* stages within each main call     */
#define DEBUG9 0

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;



/*****************************************************************************/
/*                                                                           */
/*  Submodule "trying unassignments"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnTryTaskUnAssignments(KHE_SOLN soln)                          */
/*                                                                           */
/*  Try unassigning each task of soln, to see if that improves the cost.     */
/*  Return true if any unassignments were kept.                              */
/*                                                                           */
/*****************************************************************************/

bool KheSolnTryTaskUnAssignments(KHE_SOLN soln)
{
  KHE_TASK task, target_task;  int i;  KHE_COST cost;  bool res;
  if( DEBUG9 )
    fprintf(stderr, "[ KheSolnTryTaskUnAssignments(soln)\n");
  res = false;
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    target_task = KheTaskAsst(task);
    cost = KheSolnCost(soln);
    if( target_task != NULL && !KheTaskIsPreassigned(task, NULL) &&
	KheTaskUnAssign(task) )
    {
      if( KheSolnCost(soln) < cost )
      {
	res = true;
	if( DEBUG9 )
	{
	  fprintf(stderr, "  %.5f -> %.5f for unassigning task ",
	    KheCostShow(cost), KheCostShow(KheSolnCost(soln)));
	  KheTaskDebug(task, 1, 0, stderr);
	}
      }
      else
	KheTaskAssign(task, target_task);
    }
  }
  if( DEBUG9 )
    fprintf(stderr, "] KheSolnTryTaskUnAssignments returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "putting it all together"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  char *KheTaskingLabel(KHE_TASKING tasking)                               */
/*                                                                           */
/*  Return a string description of the scope of tasking.                     */
/*                                                                           */
/*****************************************************************************/

static char *KheTaskingLabel(KHE_TASKING tasking)
{
  return KheTaskingResourceType(tasking) == NULL ? "All Resources" :
    KheResourceTypeId(KheTaskingResourceType(tasking)) == NULL ? "?" :
    KheResourceTypeId(KheTaskingResourceType(tasking));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDebugStage(KHE_TASKING tasking, char *stage, int verbosity)      */
/*                                                                           */
/*  Print a debug message showing the stage, and the solution cost.          */
/*                                                                           */
/*****************************************************************************/

static void KheDebugStage(KHE_TASKING tasking, char *stage, int verbosity)
{
  fprintf(stderr, "  %s:\n", stage);
  KheSolnDebug(KheTaskingSoln(tasking), verbosity, 2, stderr);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskingAssignResourcesStage1(KHE_TASKING tasking,                */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Assign resources to the leader tasks of tasking (Stage 1):  assign       */
/*  most tasks preserving the resource assignment invariant.                 */
/*                                                                           */
/*****************************************************************************/

bool KheTaskingAssignResourcesStage1(KHE_TASKING tasking, KHE_OPTIONS options)
{
  KHE_TASK_JOB_TYPE tjt;  KHE_RESOURCE_TYPE rt;
  if( DEBUG1 )
    fprintf(stderr, "[ KheTaskingAssignResourcesStage1(%s)\n",
      KheTaskingLabel(tasking));

  /* prohibit violations of avoid split asst and prefer resources constraints*/
  KheOptionsSetResourceInvariant(options, true);
  tjt = KHE_TASK_JOB_HARD_PRC | KHE_TASK_JOB_SOFT_PRC |
        KHE_TASK_JOB_HARD_ASAC | KHE_TASK_JOB_SOFT_ASAC;
  KheTaskingMakeTaskTree(tasking, tjt, NULL, options);

  /* resource packing or constructive heuristic */
  rt = KheTaskingResourceType(tasking);
  if( rt == NULL || KheResourceTypeAvoidSplitAssignmentsCount(rt) > 0 )
    KheResourcePackAssignResources(tasking, options);
  else
    KheMostConstrainedFirstAssignResources(tasking, options);

  /* ejection chains repair */
  if( DEBUG2 )
    KheDebugStage(tasking, "in Stage1 before repair", 2);
  KheEjectionChainRepairResources(tasking, options);
  if( DEBUG1 )
  {
    KheDebugStage(tasking, "at end of Stage1", 2);
    fprintf(stderr, "] KheTaskingAssignResourcesStage1 returning\n");
  }

  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskingAssignResourcesStage2(KHE_TASKING tasking,                */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Assign resources to the leader tasks of tasking (Stage 2):  find         */
/*  split assignments, still preserving the resource assignment invariant.   */
/*                                                                           */
/*****************************************************************************/

bool KheTaskingAssignResourcesStage2(KHE_TASKING tasking, KHE_OPTIONS options)
{
  KHE_RESOURCE_TYPE rt;

  rt = KheTaskingResourceType(tasking);
  if( KheResourceTypeAvoidSplitAssignmentsCount(rt) > 0 )
  {
    if( DEBUG1 )
      fprintf(stderr, "[ KheTaskingAssignResourcesStage2(%s)\n",
	KheTaskingLabel(tasking));
    KheFindSplitResourceAssignments(tasking, options);
    /* the following is not redundant; it splits everything not already split */
    KheTaskingAllowSplitAssignments(tasking, false);

    /* ejection chains repair */
    if( DEBUG2 )
      KheDebugStage(tasking, "in Stage2 before repair", 2);
    KheEjectionChainRepairResources(tasking, options);
    if( KheOptionsResourceRematch(options) )
      KheResourceRematch(tasking, options);
    KheResourcePairRepair(tasking, options);
    if( DEBUG1 )
    {
      KheDebugStage(tasking, "at end of Stage2", 2);
      fprintf(stderr, "] KheTaskingAssignResourcesStage2 returning\n");
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskingAssignResourcesStage3(KHE_TASKING tasking,                */
/*    KHE_OPTIONS options)                                                   */
/*                                                                           */
/*  Assign resources to the leader tasks of tasking (Stage 2):  last-ditch   */
/*  assignments, without the resource assignment invariant.                  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskingAssignResourcesStage3(KHE_TASKING tasking, KHE_OPTIONS options)
{
  /* allow violations of prefer resources constraints */
  if( DEBUG1 )
    fprintf(stderr, "[ KheTaskingAssignResourcesStage3(%s)\n",
      KheTaskingLabel(tasking));
  KheOptionsSetResourceInvariant(options, false);
  KheTaskingEnlargeDomains(tasking, true);

  /* ejection chains repair */
  KheEjectionChainRepairResources(tasking, options);
  if( DEBUG1 )
  {
    KheDebugStage(tasking, "at end of Stage3", 2);
    fprintf(stderr, "] KheTaskingAssignResourcesStage3 returning\n");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskingAssignResources(KHE_TASKING tasking, KHE_OPTIONS options) */
/*                                                                           */
/*  Assign resources to the leader tasks of tasking.                         */
/*                                                                           */
/*****************************************************************************/

bool KheTaskingAssignResources(KHE_TASKING tasking, KHE_OPTIONS options)
{
  KheTaskingAssignResourcesStage1(tasking, options);
  KheTaskingAssignResourcesStage2(tasking, options);
  KheTaskingAssignResourcesStage3(tasking, options);
  return true;
}
