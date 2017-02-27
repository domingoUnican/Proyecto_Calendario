
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
/*  FILE:         khe_sr_kempe.c                                             */
/*  DESCRIPTION:  Kempe and ejecting task moves                              */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_priqueue.h"
#include <limits.h>

#define DEBUG5 0


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignedTimesIntersect(KHE_TASK task, KHE_TIME_GROUP tg)     */
/*                                                                           */
/*  Return true if task is running at any of the times of tg.                */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskAssignedTimesIntersect(KHE_TASK task, KHE_TIME_GROUP tg)
{
  int i;  KHE_TASK child_task;  KHE_MEET meet;  KHE_TIME t;

  /* check task itself */
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAsstTime(meet) != NULL )
  {
    t = KheMeetAsstTime(meet);
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
      if( KheTimeGroupContains(tg, KheTimeNeighbour(t, i)) )
	return true;
  }

  /* check the tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    child_task = KheTaskAssignedTo(task, i);
    if( KheTaskAssignedTimesIntersect(child_task, tg) )
      return true;
  }

  /* return false if found nothing */
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssignClashing(KHE_TASK task, KHE_RESOURCE r)              */
/*                                                                           */
/*  Unassign every task in r's timetable that would clash with task, and     */
/*  return true; or if one of those tasks is preassigned r, return false.    */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskUnAssignClashing(KHE_TASK task, KHE_RESOURCE r)
{
  int i;  KHE_TASK child_task, task2;  KHE_MEET meet, meet2;
  KHE_TIME t, time;  KHE_TIMETABLE_MONITOR tm;

  /* find r's timetable monitor and make sure it is attached */
  tm = KheResourceTimetableMonitor(KheTaskSoln(task), r);
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tm) )
    KheMonitorAttachToSoln((KHE_MONITOR) tm);

  /* unassign tasks that clash with task */
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAsstTime(meet) != NULL )
  {
    time = KheMeetAsstTime(meet);
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
    {
      t = KheTimeNeighbour(time, i);
      while( KheTimetableMonitorTimeMeetCount(tm, t) > 0 )
      {
	meet2 = KheTimetableMonitorTimeMeet(tm, t, 0);
	if( !KheMeetContainsResourceAssignment(meet2, r, &task2) )
	  MAssert(false, "KheTaskKempeStepAssignResource internal error");
	task2 = KheTaskFirstUnFixed(task2);
	if( task2 == NULL || KheTaskIsPreassigned(task2, NULL) ||
	    !KheTaskUnAssign(task2) )
	  return false;
      }
    }
  }

  /* unassign tasks that clash with task's child tasks */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    child_task = KheTaskAssignedTo(task, i);
    if( !KheTaskUnAssignClashing(child_task, r) )
      return false;
  }

  /* return true since no problems arose */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskEjectingMoveResource(KHE_TASK task, KHE_RESOURCE r)          */
/*                                                                           */
/*  Carry out an ejecting move of task to r.  This version does not worry    */
/*  about the matching, it merely fails on preassigned times and unassigns   */
/*  clashing tasks.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskEjectingMoveResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TIME_GROUP unavail_times;  bool res;
  if( DEBUG5 )
  {
    fprintf(stderr, "[ KheTaskEjectingMoveResource(");
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* fail if r is currently assigned to task */
  if( KheTaskAsstResource(task) == r )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheTaskEjectingMoveResource ret. false (nochange)\n");
    return false;
  }

  /* fail if r is unavailable at any of task's assigned times */
  unavail_times = KheResourceHardUnavailableTimeGroup(r);
  if( KheTaskAssignedTimesIntersect(task, unavail_times) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheTaskEjectingMoveResource ret. false (unavail)\n");
    return false;
  }

  /* unassign everything that will clash with task */
  if( !KheTaskUnAssignClashing(task, r) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheTaskEjectingMoveResource ret. false (preass)\n");
    return false;
  }

  /* make the assignment and fail if it fails */
  res = KheTaskMoveResource(task, r);
  if( DEBUG5 )
    fprintf(stderr, "] KheTaskEjectingMoveResource returning %s\n",
      res ? "true" : "false");
  return res;
}
