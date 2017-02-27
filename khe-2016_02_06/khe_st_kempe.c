
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
/*  FILE:         khe_kempe_move.c                                           */
/*  DESCRIPTION:  Kempe and ejecting meet moves                              */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define KHE_KEMPE_WITH_STATS 1

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_FRAME - a frame (meet, offset, duration) in which meets move   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_kempe_frame_rec {
  KHE_MEET		target_meet;
  int			start_offset;	/* the first legal offset in target  */
  int			stop_offset;	/* just after the last legal offset  */
} KHE_KEMPE_FRAME;


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_NODE - a node, along with information about its irregularity   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_kempe_node_rec {
  KHE_NODE		node;			/* the node in question      */
  int			init_irreg;		/* initial irregularity      */
} KHE_KEMPE_NODE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_STATS - a statistical record of a set of Kempe meet moves      */
/*                                                                           */
/*****************************************************************************/

struct khe_kempe_stats_rec {
  ARRAY_INT			step_count_histo;
  ARRAY_INT			phase_count_histo;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe demand secondary grouping"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheKempeDemandGroupMonitorMake(KHE_SOLN soln)          */
/*                                                                           */
/*  Make a Kempe demand group monitor with the appropriate children:         */
/*  one for each preassigned demand node.  There is no primary grouping      */
/*  of preassigned demand nodes, so we don't search for one.                 */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheKempeDemandGroupMonitorMake(KHE_SOLN soln)
{
  int i, j, k;  KHE_MEET meet;  KHE_TASK task;  KHE_GROUP_MONITOR res;
  KHE_MONITOR m;
  if( DEBUG2 )
    fprintf(stderr, "[ KheKempeDemandGroupMonitorMake(soln)\n");
  res = KheGroupMonitorMake(soln, KHE_SUBTAG_KEMPE_DEMAND,
    KheSubTagLabel(KHE_SUBTAG_KEMPE_DEMAND));
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet = KheSolnMeet(soln, i);
    for( j = 0;  j < KheMeetTaskCount(meet);  j++ )
    {
      task = KheMeetTask(meet, j);
      if( KheTaskIsPreassigned(task, NULL) )
	for( k = 0;  k < KheTaskDemandMonitorCount(task);  k++ )
	{
	  m = (KHE_MONITOR) KheTaskDemandMonitor(task, k);
	  KheGroupMonitorAddChildMonitor(res, m);
	}
    }
  }
  if( DEBUG2 )
  {
    KheMonitorDebug((KHE_MONITOR) res, 2, 2, stderr);
    fprintf(stderr, "] KheKempeDemandGroupMonitorMake returning\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe frame functions"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool IntervalsOverlap(int offset1, int durn1, int offset2, int durn2)    */
/*                                                                           */
/*  Return true if these intervals overlap.                                  */
/*                                                                           */
/*****************************************************************************/

bool IntervalsOverlap(int start_offset1, int stop_offset1,
  int start_offset2, int stop_offset2)
{
  return start_offset2 < stop_offset1 && start_offset1 < stop_offset2;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeFramesOverlap(KHE_KEMPE_FRAME f1, KHE_KEMPE_FRAME f2)       */
/*                                                                           */
/*  Return true if f1 and f2 overlap.                                        */
/*                                                                           */
/*****************************************************************************/

bool KheKempeFramesOverlap(KHE_KEMPE_FRAME f1, KHE_KEMPE_FRAME f2)
{
  return f1.target_meet == f2.target_meet &&
    IntervalsOverlap(f1.start_offset, f1.stop_offset,
      f2.start_offset, f2.stop_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeFrameContainsMeet(KHE_KEMPE_FRAME frame, KHE_MEET meet)     */
/*                                                                           */
/*  Return true if frame contains meet.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeFrameContainsMeet(KHE_KEMPE_FRAME frame, KHE_MEET meet)
{
  int start_offset, stop_offset;
  start_offset = KheMeetAsstOffset(meet);
  stop_offset = start_offset + KheMeetDuration(meet);
  return frame.target_meet == KheMeetAsst(meet) &&
    start_offset >= frame.start_offset && stop_offset <= frame.stop_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeFrameDuration(KHE_KEMPE_FRAME frame)                         */
/*                                                                           */
/*  Return the duration of *frame.                                           */
/*                                                                           */
/*****************************************************************************/

static int KheKempeFrameDuration(KHE_KEMPE_FRAME frame)
{
  return frame.stop_offset - frame.start_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_FRAME KheKempeFrame(KHE_MEET target_meet, int start_offset,    */
/*    int stop_offset)                                                       */
/*                                                                           */
/*  Return a Kempe frame with these attributes.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_KEMPE_FRAME KheKempeFrame(KHE_MEET target_meet, int start_offset,
  int stop_offset)
{
  KHE_KEMPE_FRAME res;
  res.target_meet = target_meet;
  res.start_offset = start_offset;
  res.stop_offset = stop_offset;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeFrameIsLegal(KHE_KEMPE_FRAME frame)                         */
/*                                                                           */
/*  Return true if frame is legal:  if its offsets make sense relative to    */
/*  each other and to its target meet.                                       */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeFrameIsLegal(KHE_KEMPE_FRAME frame)
{
  return 0 <= frame.start_offset && frame.start_offset < frame.stop_offset
    && frame.stop_offset <= KheMeetDuration(frame.target_meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheKempeFrameEnlargeForMeet(KHE_KEMPE_FRAME *frame, KHE_MEET meet)  */
/*                                                                           */
/*  Enlarge *frame so that it holds meet.                                    */
/*                                                                           */
/*****************************************************************************/

static void KheKempeFrameEnlargeForMeet(KHE_KEMPE_FRAME *frame, KHE_MEET meet)
{
  int start_offset, stop_offset;
  MAssert(KheMeetAsst(meet) == frame->target_meet,
    "KheKempeFrameEnlargeForMeet internal error");
  start_offset = KheMeetAsstOffset(meet);
  stop_offset = start_offset + KheMeetDuration(meet);
  if( start_offset < frame->start_offset )
    frame->start_offset = start_offset;
  if( stop_offset > frame->stop_offset )
    frame->stop_offset = stop_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_FRAME KheKempeFrameEnclosingMeets(ARRAY_KHE_MEET *meets,       */
/*    int start, int stop)                                                   */
/*                                                                           */
/*  Return a new frame that exactly encloses meets[start, stop-1].  There    */
/*  must be at least one meet, and they all must have the same target meet.  */
/*                                                                           */
/*****************************************************************************/

static KHE_KEMPE_FRAME KheKempeFrameEnclosingMeets(ARRAY_KHE_MEET *meets,
  int start, int stop)
{
  int offset, i;  KHE_MEET meet;  KHE_KEMPE_FRAME res;

  /* initialize res from first meet */
  MAssert(start < stop, "KheKempeFrameEnclosingMeets internal error 1");
  meet = MArrayGet(*meets, start);
  offset = KheMeetAsstOffset(meet);
  res = KheKempeFrame(KheMeetAsst(meet), offset, offset+KheMeetDuration(meet));

  /* visit subsequent meets and enlarge res as required */
  for( i = start + 1;  i < stop;  i++ )
  {
    meet = MArrayGet(*meets, i);
    KheKempeFrameEnlargeForMeet(&res, meet);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheKempeFrameDebug(KHE_KEMPE_FRAME frame, FILE *fp)                 */
/*                                                                           */
/*  Debug print of frame onto fp.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheKempeFrameDebug(KHE_KEMPE_FRAME frame, FILE *fp)
{
  fprintf(fp, "[%s", KheKempeFrameIsLegal(frame) ? "" : " !! ");
  KheMeetDebug(frame.target_meet, 1, -1, fp);
  fprintf(fp, " %d:%d]", frame.start_offset, frame.stop_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe node functions"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheKempeNodeInit(KHE_KEMPE_NODE *kn, bool preserve_regularity,      */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Initialize kn based on the node containing meet.                         */
/*                                                                           */
/*****************************************************************************/

static void KheKempeNodeInit(KHE_KEMPE_NODE *kn, bool preserve_regularity,
  KHE_MEET meet)
{
  kn->node = preserve_regularity ? KheMeetNode(meet) : NULL;
  kn->init_irreg = (kn->node == NULL ? 0 : KheNodeIrregularity(kn->node));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeNodeMoreIrregular(KHE_KEMPE_NODE *kn1, KHE_KEMPE_NODE *kn2) */
/*                                                                           */
/*  Return true if kn1 and kn2 are more irregular at the end than at the     */
/*  beginning.                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeNodeMoreIrregular(KHE_KEMPE_NODE *kn1, KHE_KEMPE_NODE *kn2)
{
  int kn1_irreg, kn2_irreg;
  kn1_irreg = (kn1->node == NULL ? 0 : KheNodeIrregularity(kn1->node));
  kn2_irreg = (kn2->node == NULL ? 0 : KheNodeIrregularity(kn2->node));
  return kn1_irreg + kn2_irreg > kn1->init_irreg + kn2->init_irreg;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main function (KheKempeMeetMove)"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)            */
/*                                                                           */
/*  If meet, or any meet assigned to meet directly or indirectly, contains   */
/*  a preassigned task, then set *task to one such task and return true,     */
/*  otherwise return false.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)
{
  int i;

  /* try meet's own tasks */
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    *task = KheMeetTask(meet, i);
    if( KheTaskIsPreassigned(*task, NULL) )
      return true;
  }

  /* try meets assigned to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
    if( KheMeetHasPreassignedTask(KheMeetAssignedTo(meet, i), task) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeFail(ARRAY_KHE_MEET *meets, KHE_TRACE t)                    */
/*                                                                           */
/*  Convenience function for KheKempeMeetMove to call when failing.          */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeFail(ARRAY_KHE_MEET *meets, KHE_TRACE t,
  char *message, KHE_MEET meet)
{
  MArrayFree(*meets);
  KheTraceDelete(t);
  if( DEBUG1 )
  {
    fprintf(stderr, "  ] KheKempeMeetMove returning false (%s", message);
    if( meet != NULL )
    {
      fprintf(stderr, " ");
      KheMeetDebug(meet, 1, -1, stderr);
    }
    fprintf(stderr, ")\n");
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorMonitorsPreassignedTask(KHE_MONITOR m, KHE_TASK *task)    */
/*                                                                           */
/*  If m is an ordinary demand monitor and the task it monitors is           */
/*  preassigned, set *task to that task and return true, else return false.  */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorMonitorsPreassignedTask(KHE_MONITOR m, KHE_TASK *task)
{
  if( KheMonitorTag(m) != KHE_ORDINARY_DEMAND_MONITOR_TAG )
    return false;
  *task = KheOrdinaryDemandMonitorTask((KHE_ORDINARY_DEMAND_MONITOR) m);
  return KheTaskIsPreassigned(*task, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheFindCompetitorMeets(KHE_TRACE t, ARRAY_KHE_MEET *meets,          */
/*    int *curr_step_start, KHE_MEET expected_target_meet,                   */
/*    bool *basic, char **err)                                               */
/*                                                                           */
/*  Use t to find the competitor meets of the meets just moved, and add      */
/*  them to *meets.  Return true if all successful, or false (with *err      */
/*  set to a brief explanation) otherwise.                                   */
/*                                                                           */
/*  Parameter *curr_step_start is the index in *meets of the first meet      */
/*  that moved on the current step.  Before a successful return, increase    */
/*  *curr_step_start to the initial value of MArraySize(*meets).             */
/*                                                                           */
/*  Each competitor meet m that this function discovers is classified into   */
/*  one of four classes.  Care is needed because meets that have just moved  */
/*  are among the competitors.  Let next_step_start be MArraySize(*meets)    */
/*  when this function is called.  The four classes are:                     */
/*                                                                           */
/*    Meets m not currently present in *meets.  These are newly discovered   */
/*    competitors of the meets that have just been moved, so they need to    */
/*    be added to the end of *meets for moving on the next step.             */
/*                                                                           */
/*    Meets m with pos < *curr_step_start.  These moved on previous steps    */
/*    and finding them again causes failure, since no meet may move twice.   */
/*                                                                           */
/*    Meets m with *curr_step_start <= pos < next_step_start.  These moved   */
/*    on the current step (the step whose competitors we are finding now).   */
/*    They may appear among the competitors but should be ignored.           */
/*                                                                           */
/*    Meets m with next_step_start <= pos.  These are meets found by this    */
/*    call of this function to require moving on the next step.  They can    */
/*    be discovered more than once but should be added to *meets only once.  */
/*                                                                           */
/*  The code below does not distinguish between the last two cases, because  */
/*  their action is the same (ignore the meet).                              */
/*                                                                           */
/*****************************************************************************/

static bool KheFindCompetitorMeets(KHE_TRACE t, ARRAY_KHE_MEET *meets,
  int *curr_step_start, KHE_MEET expected_target_meet,
  bool *basic, char **err)
{
  int i, j, pos, next_step_start;  KHE_MONITOR m, c, om;
  KHE_MEET meet;  KHE_TASK task;  KHE_SOLN soln;
  KHE_WORKLOAD_DEMAND_MONITOR wdm;  KHE_ORDINARY_DEMAND_MONITOR odm;
  soln = KheMeetSoln(expected_target_meet);
  next_step_start = MArraySize(*meets);
  for( i = 0;  i < KheTraceMonitorCount(t);  i++ )
  {
    m = KheTraceMonitor(t, i);
    MAssert(KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
      "KheFindCompetitorMeets internal error 1");
    if( KheMonitorCost(m) > KheTraceMonitorInitCost(t, i) )
    {
      KheSolnMatchingSetCompetitors(soln, m);
      for( j = 0;  j < KheSolnMatchingCompetitorCount(soln);  j++ )
      {
	c = KheSolnMatchingCompetitor(soln, j);
	switch( KheMonitorTag(c) )
	{
	  case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	    wdm = (KHE_WORKLOAD_DEMAND_MONITOR) c;
	    om = KheWorkloadDemandMonitorOriginatingMonitor(wdm);
	    if( KheMonitorTag(om) == KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG )
	    {
	      *err = KheResourceId(KheWorkloadDemandMonitorResource(wdm));
	      return false;
	    }
	    break;

	  case KHE_ORDINARY_DEMAND_MONITOR_TAG:

	    odm = (KHE_ORDINARY_DEMAND_MONITOR) c;
	    task = KheOrdinaryDemandMonitorTask(odm);
	    if( KheTaskIsPreassigned(task, NULL) )
	    {
	      /* find ancestor of meet directly under expected_target_meet */
	      *basic = false;
	      meet = KheTaskMeet(task);
	      MAssert(meet != NULL, "KheFindCompetitorMeets internal error 2");
	      while( KheMeetAsst(meet) != NULL &&
		  KheMeetAsst(meet) != expected_target_meet )
		meet = KheMeetAsst(meet);
	      if( KheMeetAsst(meet) != expected_target_meet )
	      {
		*err = "target_meet";
		return false;
	      }

	      /* ensure meet is enqueued for the next step, or fail */
	      if( !MArrayContains(*meets, meet, &pos) )
	      {
		/* meet not seen before, enqueue it for the next step */
		if( DEBUG3 )
		{
		  fprintf(stderr, "      adding meet ");
		  KheMeetDebug(meet, 1, -1, stderr);
		  fprintf(stderr, "\n");
		}
		MArrayAddLast(*meets, meet);
	      }
	      else if( pos < *curr_step_start )
	      {
		/* meet moved previously, can't move it again, so fail */
		*err = "re-move";
		return false;
	      }
	      else
	      {
		/* nothing to do; meet either moved on the current step, */
		/* or has already been enqueued for the next */
	      }
	    }
	    break;

	  default:

	    MAssert(false, "KheFindCompetitorMeets internal error 3");
	}
      }
    }
  }
  *curr_step_start = next_step_start;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheKempeDebugOneMove(int py, KHE_MEET meet, KHE_MEET target_meet,   */
/*    int offset)                                                            */
/*                                                                           */
/*  Debug print of one meet move during the Kempe meet move.                 */
/*                                                                           */
/*****************************************************************************/

static void KheKempeDebugOneTarget(KHE_MEET target_meet, int offset)
{
  fprintf(stderr, "<");
  KheMeetDebug(target_meet, 1, -1, stderr);
  fprintf(stderr, ", %d>", offset);
}

static void KheKempeDebugOneMove(int py, KHE_MEET meet, KHE_MEET target_meet,
  int offset)
{
  fprintf(stderr, "    %s KheMeetMove(", py == 1 ? "->" : "<-");
  KheMeetDebug(meet, 1, -1, stderr);
  fprintf(stderr, ", ");
  /* fprintf(stderr, " d%d, ", KheMeetDuration(meet)); */
  KheKempeDebugOneTarget(KheMeetAsst(meet), KheMeetAsstOffset(meet));
  fprintf(stderr, " to ");
  KheKempeDebugOneTarget(target_meet, offset);
  fprintf(stderr, ")\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,               */
/*    int offset, bool preserve_regularity, int *demand, bool *basic,        */
/*    KHE_KEMPE_STATS kempe_stats)                                           */
/*                                                                           */
/*  Make a Kempe meet move of meet from wherever it is now to target_meet    */
/*  at offset, and set *demand to the total demand of the meets that were    */
/*  moved.  Set *basic to true if the operation does not attempt to move     */
/*  any meets back the other way (successfully or not).                      */
/*                                                                           */
/*  If preserve_regularity is true, fail if irregularity increases.          */
/*                                                                           */
/*  Implementation note.  It would not be correct to group the demand        */
/*  monitors used to find competitors in any way, because at different       */
/*  offsets along a task there may be different competitors.                 */
/*                                                                           */
/*****************************************************************************/
static void KheKempeStatsOneEntry(KHE_KEMPE_STATS kempe_stats,
  int step_count, int phase_count);

bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand, bool *basic,
  KHE_KEMPE_STATS kempe_stats)
{
  KHE_KEMPE_NODE from_kn, to_kn;
  KHE_KEMPE_FRAME from_frame[2], to_frame[2], init_clash_frame, orig_meet_frame;
  int py, curr_step_start, offs, i, orig_start_offset;
  ARRAY_KHE_MEET meets;  KHE_MEET orig_target;  KHE_TRACE t;  char *err;
  KHE_MONITOR m;  KHE_TASK task;  KHE_GROUP_MONITOR kempe_gm;
  int step_count, phase_count;

  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheKempeMeetMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, ...)\n", offset);
  }

  /* meet must have a current assignment */
  orig_target = KheMeetAsst(meet);
  MAssert(orig_target != NULL, "KheKempeMeetMove: meet not assigned");
  orig_start_offset = KheMeetAsstOffset(meet);
  orig_meet_frame = KheKempeFrame(orig_target, orig_start_offset,
    orig_start_offset + KheMeetDuration(meet));
  *basic = true;
  *demand = 0;

  /* return false if the move takes us nowhere */
  if( orig_target == target_meet && orig_start_offset == offset )
  {
    if( DEBUG1 )
      fprintf(stderr, "  ] KheKempeMeetMove returning false (goes nowhere)\n");
    return false;
  }

  /* find initial node regularity, if required */
  KheKempeNodeInit(&from_kn, preserve_regularity, meet);
  KheKempeNodeInit(&to_kn, preserve_regularity, target_meet);

  /* if there are no preassigned tasks, do an ordinary move */
  if( !KheMeetHasPreassignedTask(meet, &task) )
  {
    /* make the move */
    if( !KheMeetMove(meet, target_meet, offset) )
    {
      if( DEBUG1 )
	fprintf(stderr, "  ] KheKempeMeetMove returning false (trivial)\n");
      return false;
    }
    *demand += KheMeetDemand(meet);

    /* check irregularity, if required */
    if( preserve_regularity && KheKempeNodeMoreIrregular(&from_kn, &to_kn) )
    {
      if( DEBUG1 )
	fprintf(stderr, "  ] KheKempeMeetMove returning false (trivial)\n");
      return false;
    }

    KheKempeStatsOneEntry(kempe_stats, 1, 1);
    if( DEBUG1 )
      fprintf(stderr, "  ] KheKempeMeetMove returning true (trivial)\n");
    return true;
  }

  /* find the Kempe demand group monitor (abort if can't) and trace it */
  m = (KHE_MONITOR) KheTaskDemandMonitor(task, 0);
  if( !KheMonitorHasParent(m, KHE_SUBTAG_KEMPE_DEMAND, &kempe_gm) )
    MAssert(false, "KheKempeMeetMove: cannot find Kempe demand group monitor");
  t = KheTraceMake(kempe_gm);

  /* build the initial set of meets to move (just meet) */
  MArrayInit(meets);
  MArrayAddLast(meets, meet);

  /* first phase: move meet and fail if it refuses to move */
  KheTraceBegin(t);
  if( DEBUG1 )
    KheKempeDebugOneMove(1, meet, target_meet, offset);
  if( !KheMeetMove(meet, target_meet, offset) )
    return KheKempeFail(&meets, t, "move", meet);
  *demand += KheMeetDemand(meet);
  step_count = phase_count = 1;
  KheTraceEnd(t);

  /* first step: use trace to find competitor meets and add them to meets */
  curr_step_start = 0;
  if( !KheFindCompetitorMeets(t, &meets, &curr_step_start,
	target_meet, basic, &err) )
    return KheKempeFail(&meets, t, err, NULL);

  /* frames, and second and later phases (only if there are clashing meets) */
  if( MArraySize(meets) > 1 )
  {
    /* construct init_clash_frame */
    init_clash_frame = KheKempeFrameEnclosingMeets(&meets, 1,MArraySize(meets));

    /* construct from_frame[0] and from_frame[1] for separate case */
    from_frame[0] = init_clash_frame;
    KheKempeFrameEnlargeForMeet(&from_frame[0], meet);
    from_frame[1] = KheKempeFrame(orig_target,
      from_frame[0].start_offset + orig_start_offset - offset,
      from_frame[0].stop_offset  + orig_start_offset - offset);

    if( KheKempeFrameIsLegal(from_frame[1]) && 
	!KheKempeFramesOverlap(from_frame[0], from_frame[1]) )
    {
      /* separate case applies */
      to_frame[1] = from_frame[0];
      to_frame[0] = from_frame[1];
      if( DEBUG1 )
      {
	fprintf(stderr, "    separate: ");
        KheKempeFrameDebug(from_frame[0], stderr);
        KheKempeFrameDebug(from_frame[1], stderr);
	fprintf(stderr, "\n");
      }
    }
    else if( orig_target != target_meet ||
	     KheKempeFramesOverlap(init_clash_frame, orig_meet_frame) )
      return KheKempeFail(&meets, t, "frames", NULL);
    else
    {
      /* combined case applies */
      from_frame[0] = init_clash_frame;
      if( orig_meet_frame.stop_offset <= init_clash_frame.start_offset )
      {
	/* combined block is from_frame[1]from_frame[0] */
	from_frame[1] = orig_meet_frame;
        from_frame[1].stop_offset = from_frame[0].start_offset;
	to_frame[1] = KheKempeFrame(orig_target,
	  from_frame[0].stop_offset - KheKempeFrameDuration(from_frame[1]),
          from_frame[0].stop_offset);
	to_frame[0] = KheKempeFrame(orig_target, from_frame[1].start_offset,
	  from_frame[1].start_offset + KheKempeFrameDuration(from_frame[0]));
	if( DEBUG1 )
	{
	  fprintf(stderr, "    combined 10: ");
	  KheKempeFrameDebug(from_frame[1], stderr);
	  KheKempeFrameDebug(from_frame[0], stderr);
	  fprintf(stderr, "\n");
	}
      }
      else
      {
	/* combined block is from_frame[0]from_frame[1] */
	from_frame[1] = orig_meet_frame;
        from_frame[1].start_offset = from_frame[0].stop_offset;
	to_frame[1] = KheKempeFrame(orig_target, from_frame[0].start_offset,
	  from_frame[0].start_offset + KheKempeFrameDuration(from_frame[1]));
	to_frame[0] = KheKempeFrame(orig_target,
	  from_frame[1].stop_offset - KheKempeFrameDuration(from_frame[0]),
          from_frame[1].stop_offset);
	if( DEBUG1 )
	{
	  fprintf(stderr, "    combined 01: ");
	  KheKempeFrameDebug(from_frame[0], stderr);
	  KheKempeFrameDebug(from_frame[1], stderr);
	  fprintf(stderr, "\n");
	}
      }
    }

    /* carry out the second and subsequent phases, until no more meets */
    for( py = 0;  curr_step_start < MArraySize(meets);  py = 1 - py )
    {
      /* move this step's meets and fail if any don't fit or don't move */
      KheTraceBegin(t);
      for( i = curr_step_start;  i < MArraySize(meets);  i++ )
      {
	meet = MArrayGet(meets, i);
	if( !KheKempeFrameContainsMeet(from_frame[py], meet) )
	  return KheKempeFail(&meets, t, "protruding", meet);
	offs = to_frame[py].start_offset + KheMeetAsstOffset(meet) -
	  from_frame[py].start_offset;
	if( DEBUG1 )
          KheKempeDebugOneMove(py, meet, to_frame[py].target_meet, offs);
	if( !KheMeetMove(meet, to_frame[py].target_meet, offs) )
	  return KheKempeFail(&meets, t, "move", meet);
	*demand += KheMeetDemand(meet);
	step_count++;
      }
      KheTraceEnd(t);
      phase_count++;

      /* use the trace to find competitor meets and add them to meets */
      if( !KheFindCompetitorMeets(t, &meets, &curr_step_start,
	    to_frame[py].target_meet, basic, &err) )
	return KheKempeFail(&meets, t, err, NULL);
    }
  }

  /* check irregularity, if required */
  if( preserve_regularity && KheKempeNodeMoreIrregular(&from_kn, &to_kn) )
    return KheKempeFail(&meets, t, "regularity", NULL);

  /* everything has moved and no more problems, so success */
  MArrayFree(meets);
  KheTraceDelete(t);
  KheKempeStatsOneEntry(kempe_stats, step_count, phase_count);
  if( DEBUG1 )
    fprintf(stderr, "  ] KheKempeMeetMove returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t,                     */
/*    bool preserve_regularity, int *demand, bool *basic,                    */
/*    KHE_KEMPE_STATS kempe_stats)                                           */
/*                                                                           */
/*  Make a Kempe meet move of meet to the cycle meet and offset that         */
/*  represent t.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand, bool *basic,
  KHE_KEMPE_STATS kempe_stats)
{
  KHE_SOLN soln;  KHE_MEET target_meet;  int offset;
  soln = KheMeetSoln(meet);
  target_meet = KheSolnTimeCycleMeet(soln, t);
  offset = KheSolnTimeCycleMeetOffset(soln, t);
  return KheKempeMeetMove(meet, target_meet, offset,
    preserve_regularity, demand, basic, kempe_stats);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "statistics"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_STATS KheKempeStatsMake(void)                                  */
/*                                                                           */
/*  Make a new KHE_KEMPE_STATS object.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_KEMPE_STATS KheKempeStatsMake(void)
{
#if KHE_KEMPE_WITH_STATS
  KHE_KEMPE_STATS res;
  MMake(res);
  MArrayInit(res->step_count_histo);
  MArrayInit(res->phase_count_histo);
  return res;
#else
  return NULL;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheKempeStatsDelete(KHE_KEMPE_STATS kempe_stats)                    */
/*                                                                           */
/*  Delete kempe_stats.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheKempeStatsDelete(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  MArrayFree(kempe_stats->step_count_histo);
  MArrayFree(kempe_stats->phase_count_histo);
  MFree(kempe_stats);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheKempeStatsOneEntry(KHE_KEMPE_STATS kempe_stats,                  */
/*    int step_count, int phase_count)                                       */
/*                                                                           */
/*  Add one entry to kempe_stats.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheKempeStatsOneEntry(KHE_KEMPE_STATS kempe_stats,
  int step_count, int phase_count)
{
#if KHE_KEMPE_WITH_STATS
  if( kempe_stats != NULL )
  {
    /* add one step_count */
    while( MArraySize(kempe_stats->step_count_histo) < step_count )
      MArrayAddLast(kempe_stats->step_count_histo, 0);
    MArrayPreInc(kempe_stats->step_count_histo, step_count - 1);

    /* add one phase_count */
    while( MArraySize(kempe_stats->phase_count_histo) < phase_count )
      MArrayAddLast(kempe_stats->phase_count_histo, 0);
    MArrayPreInc(kempe_stats->phase_count_histo, phase_count - 1);
  }
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsStepHistoMax(KHE_KEMPE_STATS kempe_stats)               */
/*                                                                           */
/*  Return the maximum number of steps in any successful Kempe meet move,    */
/*  or 0 if there have been no successful Kempe meet moves.                  */
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsStepHistoMax(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  return MArraySize(kempe_stats->step_count_histo);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsStepHistoFrequency(KHE_KEMPE_STATS kempe_stats,         */
/*    int step_count)                                                        */
/*                                                                           */
/*  Return the number of successful Kempe meet moves with step_count steps.  */
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsStepHistoFrequency(KHE_KEMPE_STATS kempe_stats,
  int step_count)
{
#if KHE_KEMPE_WITH_STATS
  MAssert(step_count >= 1, "KheKempeStatsStepHistoFrequency: step_count < 1");
  MAssert(step_count <= MArraySize(kempe_stats->step_count_histo),
    "KheKempeStatsStepHistoFrequency: "
    "step_count > KheKempeStatsStepHistoMax(ej)");
  return MArrayGet(kempe_stats->step_count_histo, step_count - 1);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsStepHistoTotal(KHE_KEMPE_STATS kempe_stats)             */
/*                                                                           */
/*  Return the total number of steps in successful Kempe meet moves.         */
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsStepHistoTotal(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  int no_of_steps, i, num;
  no_of_steps = 0;
  MArrayForEach(kempe_stats->step_count_histo, &num, &i)
    no_of_steps += num;
  return no_of_steps;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheKempeStatsStepHistoAverage(KHE_KEMPE_STATS kempe_stats)         */
/*                                                                           */
/*  Return the average number of steps of successful Kempe meet moves.       */
/*                                                                           */
/*****************************************************************************/

float KheKempeStatsStepHistoAverage(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  int no_of_steps, total_size_of_steps, i, num;
  no_of_steps = 0; total_size_of_steps = 0;
  MArrayForEach(kempe_stats->step_count_histo, &num, &i)
  {
    no_of_steps += num;
    total_size_of_steps += num * (i + 1);
  }
  MAssert(no_of_steps > 0, "KheKempeStatsStepHistoAverage: no steps");
  return (float) total_size_of_steps / (float) no_of_steps;
#else
  return 0.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsPhaseHistoMax(KHE_KEMPE_STATS kempe_stats)              */
/*                                                                           */
/*  Return the maximum number of phases in any successful Kempe meet move,   */
/*  or 0 if there have been no successful Kempe meet moves.                  */
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsPhaseHistoMax(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  return MArraySize(kempe_stats->phase_count_histo);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsPhaseHistoFrequency(KHE_KEMPE_STATS kempe_stats,        */
/*    int phase_count)                                                       */
/*                                                                           */
/*  Return the number of successful Kempe meet moves with phase_count phases.*/
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsPhaseHistoFrequency(KHE_KEMPE_STATS kempe_stats,
  int phase_count)
{
#if KHE_KEMPE_WITH_STATS
  MAssert(phase_count>=1, "KheKempeStatsPhaseHistoFrequency: phase_count < 1");
  MAssert(phase_count <= MArraySize(kempe_stats->phase_count_histo),
    "KheKempeStatsPhaseHistoFrequency: "
    "phase_count > KheKempeStatsPhaseHistoMax(ej)");
  return MArrayGet(kempe_stats->phase_count_histo, phase_count - 1);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeStatsPhaseHistoTotal(KHE_KEMPE_STATS kempe_stats)            */
/*                                                                           */
/*  Return the total number of phases in successful Kempe meet moves.        */
/*                                                                           */
/*****************************************************************************/

int KheKempeStatsPhaseHistoTotal(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  int no_of_phases, i, num;
  no_of_phases = 0;
  MArrayForEach(kempe_stats->phase_count_histo, &num, &i)
    no_of_phases += num;
  return no_of_phases;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheKempeStatsPhaseHistoAverage(KHE_KEMPE_STATS kempe_stats)        */
/*                                                                           */
/*  Return the average number of phases of successful Kempe meet moves.      */
/*                                                                           */
/*****************************************************************************/

float KheKempeStatsPhaseHistoAverage(KHE_KEMPE_STATS kempe_stats)
{
#if KHE_KEMPE_WITH_STATS
  int no_of_phases, total_size_of_phases, i, num;
  no_of_phases = 0; total_size_of_phases = 0;
  MArrayForEach(kempe_stats->phase_count_histo, &num, &i)
  {
    no_of_phases += num;
    total_size_of_phases += num * (i + 1);
  }
  MAssert(no_of_phases > 0, "KheKempeStatsPhaseHistoAverage: no phases");
  return (float) total_size_of_phases / (float) no_of_phases;
#else
  return 0.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejecting meet moves"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingFail(ARRAY_KHE_MEET *meets, KHE_TRACE t,                 */
/*    char *message, KHE_MEET meet)                                          */
/*                                                                           */
/*  Convenience function for KheEjectingMeetMove to call when failing.       */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectingFail(ARRAY_KHE_MEET *meets, KHE_TRACE t,
  char *message, KHE_MEET meet)
{
  MArrayFree(*meets);
  KheTraceDelete(t);
  if( DEBUG4 )
  {
    fprintf(stderr, "  ] KheEjectingMeetMove returning false (%s", message);
    if( meet != NULL )
    {
      fprintf(stderr, " ");
      KheMeetDebug(meet, 1, -1, stderr);
    }
    fprintf(stderr, ")\n");
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingMeetMove(KHE_MEET meet, KHE_MEET target_meet,            */
/*    int offset, bool preserve_regularity, int *demand, bool *basic)        */
/*                                                                           */
/*  Make an ejecting meet move of meet from wherever it is now (possibly     */
/*  unassigned) to target_meet at offset, and set *demand to the total       */
/*  demand of the nodes of the meets that were moved, and *basic to true     */
/*  if no meets are unassigned.                                              */
/*                                                                           */
/*  If preserve_regularity is true, fail if irregularity increases.          */
/*                                                                           */
/*  Implementation note.  This function uses the fact that when a change     */
/*  is made, any new unmatched nodes in the matching lie in the part that    */
/*  changed, not in the part that didn't.                                    */
/*                                                                           */
/*  Implementation note.  It would not be correct to group the demand        */
/*  monitors used to find competitors in any way, because at different       */
/*  offsets along a task there may be different competitors.                 */
/*                                                                           */
/*****************************************************************************/

bool KheEjectingMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand, bool *basic)
{
  KHE_NODE node[2];  int init_irreg[2], final_irreg[2];  int i, j, pos;
  ARRAY_KHE_MEET meets;  KHE_TRACE t;  bool res;  KHE_SOLN soln;
  KHE_MONITOR m, c;  KHE_TASK task;  KHE_GROUP_MONITOR kempe_gm;

  if( DEBUG4 )
  {
    fprintf(stderr, "  [ KheEjectingMeetMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s, &d)\n", offset,
      preserve_regularity ? "true" : "false");
  }

  /* return false if the move takes us nowhere */
  *demand = 0;
  *basic = true;
  if( KheMeetAsst(meet) == target_meet && KheMeetAsstOffset(meet) == offset )
  {
    if( DEBUG4 )
      fprintf(stderr, "  ] KheEjectingMeetMove returning false (no change)\n");
    return false;
  }

  /* if there are no preassigned tasks, do an ordinary move */
  if( !KheMeetHasPreassignedTask(meet, &task) )
  {
    res = KheMeetMove(meet, target_meet, offset);
    *demand += KheMeetDemand(meet);
    if( DEBUG4 )
      fprintf(stderr, "  ] KheEjectingMeetMove returning %s (basic move)\n",
	res ? "true" : "false");
    return res;
  }

  /* find the Kempe demand group monitor (abort if can't) and trace it */
  m = (KHE_MONITOR) KheTaskDemandMonitor(task, 0);
  if( !KheMonitorHasParent(m, KHE_SUBTAG_KEMPE_DEMAND, &kempe_gm) )
    MAssert(false, "KheEjectingMeetMove: missing Kempe demand group monitor");
  t = KheTraceMake(kempe_gm);

  /* initialize frame */
  if( preserve_regularity )
  {
    node[1] = KheMeetNode(meet);
    init_irreg[1] = (node[1] == NULL ? 0 : KheNodeIrregularity(node[1]));
    node[0] = KheMeetNode(target_meet);
    init_irreg[0] = (node[0] == NULL ? 0 : KheNodeIrregularity(node[0]));
  }
  else
  {
    node[1] = NULL;
    init_irreg[1] = 0;
    node[0] = NULL;
    init_irreg[0] = 0;
  }

  /* move meet */
  MArrayInit(meets);
  KheTraceBegin(t);
  if( !KheMeetMove(meet, target_meet, offset) )
    return KheEjectingFail(&meets, t, "move", meet);
  *demand += KheMeetDemand(meet);
  if( DEBUG4 )
  {
    fprintf(stderr, "    <- KheMeetMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, " d%d, ", KheMeetDuration(meet));
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d) (%d trace monitors)\n",
      offset, KheTraceMonitorCount(t));
  }
  KheTraceEnd(t);

  /* handle all preassigned demand monitors that increased in cost */
  soln = KheMeetSoln(meet);
  for( i = 0;  i < KheTraceMonitorCount(t);  i++ )
  {
    m = KheTraceMonitor(t, i);
    MAssert(KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
      "KheEjectingMeetMove internal error 3");
    if( KheMonitorCost(m) > KheTraceMonitorInitCost(t, i) )
    {
      /* ***
      for( c = KheMonitorFirstCompetitor(m);  c != NULL; 
	   c = KheMonitorNextCompetitor(m) )
      *** */
      KheSolnMatchingSetCompetitors(soln, m);
      for( j = 1;  j < KheSolnMatchingCompetitorCount(soln);  j++ )
      {
	c = KheSolnMatchingCompetitor(soln, j);
	if( KheMonitorTag(c) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
	  return KheEjectingFail(&meets, t, "unavail", NULL);
	MAssert(KheMonitorTag(c) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
	  "KheEjectingMeetMove internal error 4");
	if( KheMonitorMonitorsPreassignedTask(c, &task) )
	{
	  /* find ancestor of meet directly under to_target, or fail */
	  *basic = false;
	  meet = KheTaskMeet(task);
	  MAssert(meet != NULL, "KheEjectingMeetMove internal error 5");
	  while( KheMeetAsst(meet) != NULL && KheMeetAsst(meet) != target_meet )
	    meet = KheMeetAsst(meet);
	  if( KheMeetAsst(meet) != target_meet )
	    return KheEjectingFail(&meets, t, "scope", meet);

	  /* ensure meet is enqueued for unassigning */
	  if( !MArrayContains(meets, meet, &pos) )
	  {
	    /* meet not seen before, enqueue it for the next step */
	    if( DEBUG4 )
	    {
	      fprintf(stderr, "      adding meet ");
	      KheMeetDebug(meet, 1, -1, stderr);
	      fprintf(stderr, "\n");
	    }
	    MArrayAddLast(meets, meet);
	  }
	}
      }
    }
  }

  /* check irregularity, if required */
  if( preserve_regularity )
  {
    final_irreg[1] = (node[1] == NULL ? 0 : KheNodeIrregularity(node[1]));
    final_irreg[0] = (node[0] == NULL ? 0 : KheNodeIrregularity(node[0]));
    if( init_irreg[0] + init_irreg[1] < final_irreg[0] + final_irreg[1] )
      return KheEjectingFail(&meets, t, "regularity", NULL);
  }

  /* eject all these meets */
  MArrayForEach(meets, &meet, &i)
    if( !KheMeetUnAssign(meet) )
      return KheEjectingFail(&meets, t, "unassign", NULL);
  MArrayFree(meets);
  KheTraceDelete(t);
  if( DEBUG4 )
    fprintf(stderr, "  ] KheEjectingMeetMove returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectingMeetMoveTime(KHE_MEET meet, KHE_TIME t,                  */
/*    bool preserve_regularity, int *demand, bool *basic)                    */
/*                                                                           */
/*  Make an ejecting meet move of meet to the cycle meet and offset that     */
/*  represent t.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheEjectingMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand, bool *basic)
{
  KHE_SOLN soln;  KHE_MEET target_meet;  int offset;
  soln = KheMeetSoln(meet);
  target_meet = KheSolnTimeCycleMeet(soln, t);
  offset = KheSolnTimeCycleMeetOffset(soln, t);
  return KheEjectingMeetMove(meet, target_meet, offset,
    preserve_regularity, demand, basic);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "basic meet moves"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheBasicMeetMove(KHE_MEET meet, KHE_MEET target_meet,               */
/*    int offset, bool preserve_regularity, int *demand, bool *basic)        */
/*                                                                           */
/*  Make an ejecting meet move of meet from wherever it is now (possibly     */
/*  unassigned) to target_meet at offset, and set *demand to the total       */
/*  demand of the nodes of the meets that were moved, and *basic to true     */
/*  if no meets are unassigned.                                              */
/*                                                                           */
/*  If preserve_regularity is true, fail if irregularity increases.          */
/*                                                                           */
/*  Implementation note.  This function uses the fact that when a change     */
/*  is made, any new unmatched nodes in the matching lie in the part that    */
/*  changed, not in the part that didn't.                                    */
/*                                                                           */
/*  Implementation note.  It would not be correct to group the demand        */
/*  monitors used to find competitors in any way, because at different       */
/*  offsets along a task there may be different competitors.                 */
/*                                                                           */
/*****************************************************************************/

bool KheBasicMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand)
{
  KHE_NODE node[2];  int init_irreg[2], final_irreg[2];

  if( DEBUG4 )
  {
    fprintf(stderr, "  [ KheBasicMeetMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, %s, &d)\n", offset,
      preserve_regularity ? "true" : "false");
  }

  /* return false if the move takes us nowhere */
  *demand = 0;
  if( KheMeetAsst(meet) == target_meet && KheMeetAsstOffset(meet) == offset )
  {
    if( DEBUG4 )
      fprintf(stderr, "  ] KheBasicMeetMove returning false (no change)\n");
    return false;
  }

  /* initialize frame */
  if( preserve_regularity )
  {
    node[1] = KheMeetNode(meet);
    init_irreg[1] = (node[1] == NULL ? 0 : KheNodeIrregularity(node[1]));
    node[0] = KheMeetNode(target_meet);
    init_irreg[0] = (node[0] == NULL ? 0 : KheNodeIrregularity(node[0]));
  }
  else
  {
    node[1] = NULL;
    init_irreg[1] = 0;
    node[0] = NULL;
    init_irreg[0] = 0;
  }

  /* move meet */
  if( !KheMeetMove(meet, target_meet, offset) )
  {
    if( DEBUG4 )
      fprintf(stderr, "  ] KheBasicMeetMove returning false (move)");
    return false;
  }
  *demand += KheMeetDemand(meet);
  if( DEBUG4 )
  {
    fprintf(stderr, "    %s KheMeetMove(", "->");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, " d%d, ", KheMeetDuration(meet));
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", offset);
  }

  /* check irregularity, if required */
  if( preserve_regularity )
  {
    final_irreg[1] = (node[1] == NULL ? 0 : KheNodeIrregularity(node[1]));
    final_irreg[0] = (node[0] == NULL ? 0 : KheNodeIrregularity(node[0]));
    if( init_irreg[0] + init_irreg[1] < final_irreg[0] + final_irreg[1] )
    {
      if( DEBUG4 )
	fprintf(stderr, "  ] KheBasicMeetMove returning false (regularity)");
      return false;
    }
  }

  if( DEBUG4 )
    fprintf(stderr, "  ] KheBasicMeetMove returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheBasicMeetMoveTime(KHE_MEET meet, KHE_TIME t,                     */
/*    bool preserve_regularity, int *demand)                                 */
/*                                                                           */
/*  Make an ejecting meet move of meet to the cycle meet and offset that     */
/*  represent t.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheBasicMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand)
{
  KHE_SOLN soln;  KHE_MEET target_meet;  int offset;
  soln = KheMeetSoln(meet);
  target_meet = KheSolnTimeCycleMeet(soln, t);
  offset = KheSolnTimeCycleMeetOffset(soln, t);
  return KheBasicMeetMove(meet, target_meet, offset,
    preserve_regularity, demand);
}
