
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
/*  FILE:         khe_st_combined.c                                          */
/*  DESCRIPTION:  Combined time solvers                                      */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG3 0


/*****************************************************************************/
/*                                                                           */
/*  Submodule "KheCycleNodeAssignTimes"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheCycleNodeAssignTimes(KHE_NODE cycle_node, KHE_OPTIONS options)   */
/*                                                                           */
/*  Assign times to the meets of all the proper descendants of cycle_node,   */
/*  assumed to be the cycle node, using the best available algorithm.        */
/*                                                                           */
/*****************************************************************************/

bool KheCycleNodeAssignTimes(KHE_NODE cycle_node, KHE_OPTIONS options)
{
  int i;  KHE_SOLN soln;  bool res;
  KHE_MEET_BOUND_GROUP cluster_mbg;  KHE_TASK_BOUND_GROUP tighten_tbg;
  bool time_cluster_meet_domains = KheOptionsTimeClusterMeetDomains(options);
  bool time_tighten_domains = KheOptionsTimeTightenDomains(options);
  bool time_node_repair = KheOptionsTimeNodeRepair(options);
  /* int save_ejector_limit_defects; */
  /* bool cluster_and_limit = true; */
  /* bool save_ejector_use_fuzzy_moves; */

  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheCycleNodeAssignTimes(Node %d)\n",
      KheNodeSolnIndex(cycle_node));
    KheNodeDebug(cycle_node, 4, 2, stderr);
  }

  /* assign preassigned meets */
  soln = KheNodeSoln(cycle_node);
  KheNodePreassignedAssignTimes(cycle_node, options);

  /* tighten resource domains */
  if( time_tighten_domains )
  {
    tighten_tbg = KheTaskBoundGroupMake();
    for( i = 0;  i < KheSolnTaskingCount(soln);  i++ )
      KheTaskingTightenToPartition(KheSolnTasking(soln, i),
	tighten_tbg, options);
  }
  else
    tighten_tbg = NULL;  /* keep compiler happy */

  /* cluster meet domains */
  if( time_cluster_meet_domains )
  {
    cluster_mbg = KheMeetBoundGroupMake();
    KheSolnAddUnavailableBounds(soln, KheCost(0, 0), cluster_mbg);
    KheSolnClusterAndLimitMeetDomains(soln, 0, KheCost(1, 0), 1.8,
      cluster_mbg, options);
    /* ***
    if( cluster_and_limit )
      KheSolnClusterAndLimitMeetDomains(soln, 0, 0, 0.8, cluster_mbg);
    else
      KheSolnClusterMeetDomains(soln, KheCost(0, 0), cluster_mbg);
    *** */
  }
  else
    cluster_mbg = NULL;  /* keep compiler happy */

  /* time assignment and repair */
  if( DEBUG3 )
    fprintf(stderr,
      "  KheCycleNodeAssignTimes calling KheNodeLayeredAssignTimes\n");
  /* ***
  save_ejector_limit_defects = KheOptionsEjectorLimitDefects(options);
  if( save_ejector_limit_defects > 50 )
    KheOptionsSetEjectorLimitDefects(options, 50);
  KheOptionsSetEjectorLimitDefects(options, save_ejector_limit_defects);
  *** */
  KheNodeLayeredAssignTimes(cycle_node, options /* was false, true */);
  if( time_node_repair )
  {
    if( DEBUG3 )
      fprintf(stderr,
	"  KheCycleNodeAssignTimes calling KheEjectionChainNodeRepairTimes\n");
    KheEjectionChainNodeRepairTimes(cycle_node, options);
  }
  KheNodeFlatten(cycle_node);
  KheNodeDeleteZones(cycle_node);
  if( time_cluster_meet_domains )
    KheMeetBoundGroupDelete(cluster_mbg);
  if( time_node_repair )
  {
    KheEjectionChainNodeRepairTimes(cycle_node, options);
    /* ***
    save_ejector_use_fuzzy_moves = KheOptionsEjectorUseFuzzyMoves(options);
    KheOptionsSetEjectorUseFuzzyMoves(options, true);
    KheEjectionChainNodeRepairTimes(cycle_node, options);
    KheOptionsSetEjectorUseFuzzyMoves(options, save_ejector_use_fuzzy_moves);
    *** */
  }

  /* undo resource domain tightening */
  if( time_tighten_domains )
  {
    if( DEBUG3 )
      fprintf(stderr, "  demand defects before untightening: %d\n",
	KheSolnMatchingDefectCount(soln));
    KheTaskBoundGroupDelete(tighten_tbg);
    if( DEBUG3 )
      fprintf(stderr, "  demand defects after untightening: %d\n",
	KheSolnMatchingDefectCount(soln));
  }
  
  /* all done */
  res = KheNodeAllChildMeetsAssigned(cycle_node);
  if( DEBUG3 )
    fprintf(stderr, "] KheCycleNodeAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}
