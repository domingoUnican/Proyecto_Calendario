
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
/*  FILE:         khe_task.c                                                 */
/*  DESCRIPTION:  A task                                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK - a task                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_task_rec {
  void				*back;			/* back pointer      */
  KHE_SOLN			soln;			/* enclosing soln    */
  int				soln_index;		/* index in soln     */
  int				meet_index;		/* index in meet     */
  KHE_MEET			meet;			/* optional meet     */
  ARRAY_KHE_TASK_BOUND		task_bounds;		/* task bounds       */
  KHE_RESOURCE_GROUP		domain;			/* resource domain   */
  KHE_TASKING			tasking;		/* optional tasking  */
  int				tasking_index;		/* index in tasking  */
  bool				target_fixed;		/* target fixed      */
  KHE_TASK			target_task;		/* task assigned to  */
  ARRAY_KHE_TASK		assigned_tasks;		/* assigned to this  */
  int				visit_num;		/* visit number      */
  int				reference_count;	/* reference count   */
  ARRAY_KHE_ORDINARY_DEMAND_MONITOR all_monitors;	/* all demand mon's  */
  ARRAY_KHE_ORDINARY_DEMAND_MONITOR attached_monitors;	/* attached monitors */
  KHE_RESOURCE_IN_SOLN		assigned_rs;		/* assigned resource */
  KHE_EVENT_RESOURCE_IN_SOLN	event_resource_in_soln;	/* optional er in s  */
  KHE_TASK			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "back pointers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelSetBack(KHE_TASK task, void *back)                     */
/*                                                                           */
/*  Set the back pointer of task to back, assuming all is well.              */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelSetBack(KHE_TASK task, void *back)
{
  KheSolnOpTaskSetBack(task->soln, task, task->back, back);
  task->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelSetBackUndo(KHE_TASK task, void *old_back,             */
/*    void *new_back)                                                        */
/*                                                                           */
/*  Undo KheTaskKernelSetBack.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelSetBackUndo(KHE_TASK task, void *old_back, void *new_back)
{
  task->back = old_back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetBack(KHE_TASK task, void *back)                           */
/*                                                                           */
/*  Set the back pointer of task.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetBack(KHE_TASK task, void *back)
{
  KheTaskKernelSetBack(task, back);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheTaskBack(KHE_TASK task)                                         */
/*                                                                           */
/*  Return the back pointer of task.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheTaskBack(KHE_TASK task)
{
  return task->back;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetVisitNum(KHE_TASK task, int num)                          */
/*                                                                           */
/*  Set the visit number of task.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetVisitNum(KHE_TASK task, int num)
{
  task->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskVisitNum(KHE_TASK task)                                       */
/*                                                                           */
/*  Return the visit number of task.                                         */
/*                                                                           */
/*****************************************************************************/

int KheTaskVisitNum(KHE_TASK task)
{
  return task->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskVisited(KHE_TASK task, int slack)                            */
/*                                                                           */
/*  Return true if task has been visited recently.                           */
/*                                                                           */
/*****************************************************************************/

bool KheTaskVisited(KHE_TASK task, int slack)
{
  return KheSolnGlobalVisitNum(KheTaskSoln(task)) - task->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskVisit(KHE_TASK task)                                         */
/*                                                                           */
/*  Visit task.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheTaskVisit(KHE_TASK task)
{
  task->visit_num = KheSolnGlobalVisitNum(KheTaskSoln(task));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnVisit(KHE_TASK task)                                       */
/*                                                                           */
/*  Unvisit task.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnVisit(KHE_TASK task)
{
  task->visit_num = KheSolnGlobalVisitNum(KheTaskSoln(task)) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "other simple attributes"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheTaskMeet(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the meet attribute of task.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheTaskMeet(KHE_TASK task)
{
  return task->meet;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetMeet(KHE_TASK task, KHE_MEET meet)                        */
/*                                                                           */
/*  Set the meet attribute of task.                                          */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetMeet(KHE_TASK task, KHE_MEET meet)
{
  task->meet = meet;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskMeetIndex(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the index number of task in its meet.                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskMeetIndex(KHE_TASK task)
{
  return task->meet_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetMeetIndex(KHE_TASK task, int meet_index)                  */
/*                                                                           */
/*  Set the index number of task in its meet to meet_index.                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetMeetIndex(KHE_TASK task, int meet_index)
{
  task->meet_index = meet_index;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskDuration(KHE_TASK task)                                       */
/*                                                                           */
/*  Return the duration of the meet containing task, or 0 if none.           */
/*                                                                           */
/*****************************************************************************/

int KheTaskDuration(KHE_TASK task)
{
  return task->meet == NULL ? 0 : KheMeetDuration(task->meet);
}


/*****************************************************************************/
/*                                                                           */
/*  float KheTaskWorkload(KHE_TASK task)                                     */
/*                                                                           */
/*  Return the workload of task, according to the formula                    */
/*                                                                           */
/*    Workload(task) = Duration(meet) * Workload(er) / Duration(e)           */
/*                                                                           */
/*  where meet is its meet, er is its event resource, and e is the           */
/*  enclosing event.                                                         */
/*                                                                           */
/*****************************************************************************/

float KheTaskWorkload(KHE_TASK task)
{
  KHE_EVENT_RESOURCE er;
  if( task->event_resource_in_soln == NULL )
    return 0.0;
  else
  {
    MAssert(task->meet != NULL, "KheTaskWorkload internal error");
    er = KheEventResourceInSolnEventResource(task->event_resource_in_soln);
    return (float) KheMeetDuration(task->meet) * KheEventResourceWorkload(er) /
      (float) KheEventDuration(KheEventResourceEvent(er));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheTaskSoln(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the soln attribute of task.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheTaskSoln(KHE_TASK task)
{
  return task->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskSolnIndex(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the index number of task in its soln.                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskSolnIndex(KHE_TASK task)
{
  return task->soln_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetSolnIndex(KHE_TASK task, int num)                         */
/*                                                                           */
/*  Set the index number of task in its soln to num.                         */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetSolnIndex(KHE_TASK task, int soln_index)
{
  task->soln_index = soln_index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task)                     */
/*                                                                           */
/*  Return the resource type attribute of task.                              */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task)
{
  return KheResourceGroupResourceType(task->domain);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task)                   */
/*                                                                           */
/*  Return the optional event resource of task.                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task)
{
  return task->event_resource_in_soln == NULL ? NULL :
    KheEventResourceInSolnEventResource(task->event_resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsPreassigned(KHE_TASK task, KHE_RESOURCE *r)                */
/*                                                                           */
/*  Return true if task is derived from a preassigned event resource,        */
/*  setting *r to that resource in that case.                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsPreassigned(KHE_TASK task, KHE_RESOURCE *r)
{
  KHE_EVENT_RESOURCE er;  KHE_RESOURCE res;
  er = KheTaskEventResource(task);
  if( er == NULL )
  {
    if( r != NULL )
      *r = NULL;
    return false;
  }
  res = KheEventResourcePreassignedResource(er);
  if( res == NULL )
  {
    if( r != NULL )
      *r = NULL;
    return false;
  }
  if( r != NULL )
    *r = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "creation and deletion"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskDoMake(void)                                             */
/*                                                                           */
/*  Obtain a new task from the memory allocator; initialize its arrays.      */
/*                                                                           */
/*****************************************************************************/

static KHE_TASK KheTaskDoMake(void)
{
  KHE_TASK res;
  MMake(res);
  MArrayInit(res->task_bounds);
  MArrayInit(res->assigned_tasks);
  MArrayInit(res->all_monitors);
  MArrayInit(res->attached_monitors);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnMake(KHE_TASK task)                                        */
/*                                                                           */
/*  Undo KheTaskDoMake, returning task's memory to the memory allocator.     */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnMake(KHE_TASK task)
{
  MArrayFree(task->task_bounds);
  MArrayFree(task->assigned_tasks);
  MArrayFree(task->all_monitors);
  MArrayFree(task->attached_monitors);
  MFree(task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskDoGet(KHE_SOLN soln)                                     */
/*                                                                           */
/*  Get a task object, either from soln's free list or allocated.            */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskDoGet(KHE_SOLN soln)
{
  KHE_TASK res;
  res = KheSolnGetTaskFromFreeList(soln);
  if( res == NULL )
    res = KheTaskDoMake();
  res->reference_count = 0;
  if( DEBUG6 )
    fprintf(stderr, "KheTaskDoGet %p\n", (void *) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnGet(KHE_TASK task)                                         */
/*                                                                           */
/*  Undo KheTaskDoGet, adding task to the free list and clearing its arrays. */
/*                                                                           */
/*****************************************************************************/

static void KheTaskUnGet(KHE_TASK task)
{
  if( DEBUG6 )
    fprintf(stderr, "KheTaskUnGet %p\n", (void *) task);
  KheSolnAddTaskToFreeList(task->soln, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskReferenceCountIncrement(KHE_TASK task)                       */
/*                                                                           */
/*  Increment task's reference count.                                        */
/*                                                                           */
/*****************************************************************************/

void KheTaskReferenceCountIncrement(KHE_TASK task)
{
  task->reference_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskReferenceCountDecrement(KHE_TASK task)                       */
/*                                                                           */
/*  Decrement task's reference count, and possibly add it to the free list.  */
/*                                                                           */
/*****************************************************************************/

void KheTaskReferenceCountDecrement(KHE_TASK task)
{
  MAssert(task->reference_count >= 1,
    "KheTaskReferenceCountDecrement internal error");
  if( --task->reference_count == 0 )
    KheTaskUnGet(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAdd(KHE_TASK task, KHE_SOLN soln, KHE_RESOURCE_TYPE rt,    */
/*    KHE_MEET meet, KHE_EVENT_RESOURCE er)                                  */
/*                                                                           */
/*  Initialize task (assuming its arrays are initialized, but possibly not   */
/*  empty, and its reference_count is initialized) and add it to soln.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskDoAdd(KHE_TASK task, KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  KHE_MEET meet, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  int i;  
  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_ORDINARY_DEMAND_MONITOR m; 

  task->back = NULL;
  task->soln = soln;
  KheSolnAddTask(soln, task);  /* sets soln->task_index too */
  if( meet != NULL )
    KheMeetAddTask(meet, task, true);
  else
  {
    task->meet = NULL;
    task->meet_index = -1;
  }
  if( er != NULL && KheEventResourcePreassignedResource(er) != NULL )
    task->domain = KheResourceSingletonResourceGroup(
      KheEventResourcePreassignedResource(er));
  else
    task->domain = KheResourceTypeFullResourceGroup(rt);
  task->tasking = NULL;
  task->tasking_index = -1;
  /* task->domain_fixed = false; */
  task->target_fixed = false;
  task->target_task = NULL;
  MArrayClear(task->assigned_tasks);
  task->visit_num = KheSolnGlobalVisitNum(soln);
  KheTaskReferenceCountIncrement(task);
  MArrayClear(task->all_monitors);
  MArrayClear(task->attached_monitors);
  if( task->meet != NULL && KheSolnMatching(task->soln) != NULL )
    for( i = 0;  i < KheMeetDuration(task->meet);  i++ )
    {
      dc = KheMeetDemandChunk(task->meet, i);
      m = KheOrdinaryDemandMonitorMake(task->soln, dc, task, i);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) task->soln,
	(KHE_MONITOR) m);
    }
  task->assigned_rs = NULL;
  if( er != NULL )
  {
    es = KheSolnEventInSoln(soln, KheEventIndex(KheEventResourceEvent(er)));
    task->event_resource_in_soln =
      KheEventInSolnEventResourceInSoln(es, KheEventResourceEventIndex(er));
    KheEventResourceInSolnAddTask(task->event_resource_in_soln, task);
  }
  else
    task->event_resource_in_soln = NULL;
  task->copy = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnAdd(KHE_TASK task)                                         */
/*                                                                           */
/*  Undo KheTaskDoAdd, leaving task unlinked from the solution.              */
/*                                                                           */
/*****************************************************************************/

static void KheTaskUnAdd(KHE_TASK task)
{
  /* inform task's event resource in soln */
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnDeleteTask(task->event_resource_in_soln, task);

  /* inform task's meet */
  if( task->meet != NULL )
    KheMeetDeleteTask(task->meet, task->meet_index);

  /* remove task's demand monitors */
  while( MArraySize(task->all_monitors) > 0 )
    KheOrdinaryDemandMonitorDelete(MArrayLast(task->all_monitors));

  /* delete from soln */
  KheSolnDeleteTask(task->soln, task);

  /* task is now not referenced from solution (this call may free task) */
  KheTaskReferenceCountDecrement(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAdd(KHE_TASK task, KHE_SOLN soln,                      */
/*    KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)            */
/*                                                                           */
/*  Kernel operation which adds task to soln (but does not make it).         */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAdd(KHE_TASK task, KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)
{
  KheTaskDoAdd(task, soln, rt, meet, er);
  KheSolnOpTaskAdd(soln, task, rt, meet, er);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAddUndo(KHE_TASK task)                                 */
/*                                                                           */
/*  Undo KheTaskKernelAdd.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAddUndo(KHE_TASK task)
{
  KheTaskUnAdd(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelDelete(KHE_TASK task)                                  */
/*                                                                           */
/*  Kernel operation which deletes task (but does not free it).              */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelDelete(KHE_TASK task)
{
  KheSolnOpTaskDelete(task->soln, task, KheTaskResourceType(task),
    task->meet, KheTaskEventResource(task));
  KheTaskUnAdd(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelDeleteUndo(KHE_TASK task)                              */
/*                                                                           */
/*  Undo KheTaskKernelDelete.                                                */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelDeleteUndo(KHE_TASK task, KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er)
{
  KheTaskDoAdd(task, soln, rt, meet, er);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt, KHE_MEET meet, */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Make and return a new task with these attributes.  The first two are     */
/*  compulsory.  If meet is absent, then er must be absent too.              */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt, KHE_MEET meet,
  KHE_EVENT_RESOURCE er)
{
  KHE_TASK res;  KHE_RESOURCE_GROUP rg;  KHE_TASK_BOUND tb;

  /* ensure parameters are legal */
  if( meet == NULL )
    MAssert(er == NULL, "KheTaskMake: meet == NULL && er != NULL");
  if( er != NULL )
    MAssert(KheMeetEvent(meet) == KheEventResourceEvent(er),
      "KheTaskMake: KheMeetEvent(meet) != KheEventResourceEvent(er)");

  /* make and initialize a new task object from scratch */
  res = KheTaskDoGet(soln);

  /* add it to the soln */
  KheTaskKernelAdd(res, soln, rt, meet, er);

  /* if its event resource is preassigned, add a singleton task bound */
  if( er != NULL && KheEventResourcePreassignedResource(er) != NULL )
  {
    rg = KheResourceSingletonResourceGroup(
      KheEventResourcePreassignedResource(er));
    tb = KheTaskBoundMake(soln, rg);
    if( !KheTaskAddTaskBound(res, tb) )
      MAssert(false, "KheTaskMake: cannot add preassignment task bound");
  }

  /* return it */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r)                 */
/*                                                                           */
/*  Make and return a cycle task for resource r.                             */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_TASK res;
  res = KheTaskMake(soln, KheResourceResourceType(r), NULL, NULL);
  res->assigned_rs = KheSolnResourceInSoln(soln, KheResourceInstanceIndex(r));
  res->domain = KheResourceSingletonResourceGroup(r);
  KheTaskAssignFix(res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDelete(KHE_TASK task)                                        */
/*                                                                           */
/*  Delete task.  If freed, its memory goes on soln's task free list.        */
/*                                                                           */
/*****************************************************************************/

void KheTaskDelete(KHE_TASK task)
{
  KHE_TASK t2;

  /* remove task from its tasking, if any */
  if( task->tasking != NULL )
    KheTaskingDeleteTask(task->tasking, task);

  /* unassign task, if assigned */
  if( task->target_task != NULL )
  {
    KheTaskAssignUnFix(task);
    KheTaskUnAssign(task);
  }

  /* unassign its child tasks */
  while( MArraySize(task->assigned_tasks) > 0 )
  {
    t2 = MArrayLast(task->assigned_tasks);
    KheTaskAssignUnFix(t2);
    KheTaskUnAssign(t2);
  }

  /* delete the task bounds of task */
  while( MArraySize(task->task_bounds) > 0 )
    KheTaskBoundDelete(MArrayLast(task->task_bounds));

  /* carry out the kernel delete operation (may free task) */
  KheTaskKernelDelete(task);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "copy"                                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskCopyPhase1(KHE_TASK task)                                */
/*                                                                           */
/*  Carry out Phase 1 of the operation of copying task.                      */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskCopyPhase1(KHE_TASK task)
{
  KHE_TASK copy, child_task;  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  KHE_TASK_BOUND tb;
  if( task->copy == NULL )
  {
    MMake(copy);
    task->copy = copy;
    copy->back = task->back;
    copy->soln = KheSolnCopyPhase1(task->soln);
    copy->soln_index = task->soln_index;
    copy->meet = task->meet == NULL ? NULL : KheMeetCopyPhase1(task->meet);
    copy->meet_index = task->meet_index;
    copy->target_fixed = task->target_fixed;
    MArrayInit(copy->task_bounds);
    MArrayForEach(task->task_bounds, &tb, &i)
      MArrayAddLast(copy->task_bounds, KheTaskBoundCopyPhase1(tb));
    copy->domain = task->domain;
    copy->tasking = task->tasking == NULL ? NULL :
      KheTaskingCopyPhase1(task->tasking);
    copy->tasking_index = task->tasking_index;
    copy->target_task = task->target_task == NULL ? NULL :
      KheTaskCopyPhase1(task->target_task);
    MArrayInit(copy->assigned_tasks);
    MArrayForEach(task->assigned_tasks, &child_task, &i)
      MArrayAddLast(copy->assigned_tasks, KheTaskCopyPhase1(child_task));
    copy->visit_num = task->visit_num;
    copy->reference_count = 1;  /* there are no paths, and copy is linked in */
    MArrayInit(copy->all_monitors);
    MArrayForEach(task->all_monitors, &m, &i)
      MArrayAddLast(copy->all_monitors, KheOrdinaryDemandMonitorCopyPhase1(m));
    MArrayInit(copy->attached_monitors);
    MArrayForEach(task->attached_monitors, &m, &i)
      MArrayAddLast(copy->attached_monitors,
	KheOrdinaryDemandMonitorCopyPhase1(m));
    copy->assigned_rs = task->assigned_rs == NULL ? NULL :
      KheResourceInSolnCopyPhase1(task->assigned_rs);
    copy->event_resource_in_soln = task->event_resource_in_soln == NULL ? NULL :
      KheEventResourceInSolnCopyPhase1(task->event_resource_in_soln);
    copy->copy = NULL;
  }
  return task->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskCopyPhase2(KHE_TASK task)                                    */
/*                                                                           */
/*  Carry out Phase 2 of copying task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTaskCopyPhase2(KHE_TASK task)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;  KHE_TASK_BOUND tb;
  if( task->copy != NULL )
  {
    task->copy = NULL;
    MArrayForEach(task->task_bounds, &tb, &i)
      KheTaskBoundCopyPhase2(tb);
    MArrayForEach(task->all_monitors, &m, &i)
      KheOrdinaryDemandMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "domain calculations"                                          */
/*                                                                           */
/*  This private submodule collects together all the helper functions for    */
/*  calculating domains required at various places throughout this file.     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskAddBoundDomain(KHE_TASK task,                  */
/*    KHE_TASK_BOUND tb)                                                     */
/*                                                                           */
/*  Return the domain of task after adding tb.  This works whether or not    */
/*  tb has already been added to task->task_bounds.                          */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_GROUP KheTaskAddBoundDomain(KHE_TASK task,
  KHE_TASK_BOUND tb)
{
  KHE_RESOURCE_GROUP res;
  if( MArraySize(task->task_bounds) == 0 )
    res = KheTaskBoundResourceGroup(tb);
  else if( MArraySize(task->task_bounds) == 1 &&
	   MArrayFirst(task->task_bounds) == tb )
    res = KheTaskBoundResourceGroup(tb);
  else
  {
    KheSolnResourceGroupBegin(task->soln, KheTaskResourceType(task));
    KheSolnResourceGroupUnion(task->soln, task->domain);
    KheSolnResourceGroupIntersect(task->soln, KheTaskBoundResourceGroup(tb));
    res = KheSolnResourceGroupEnd(task->soln);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskDeleteBoundDomain(KHE_TASK task,               */
/*    KHE_TASK_BOUND tb)                                                     */
/*                                                                           */
/*  Return the domain of task after deleting tb.  This works whether or not  */
/*  tb has already been deleted from task->task_bounds.  It also allows tb   */
/*  to be NULL, in which case it returns task's domain (assumed non-NULL)    */
/*  based on all its task bounds.                                            */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_GROUP KheTaskDeleteBoundDomain(KHE_TASK task,
  KHE_TASK_BOUND tb)
{
  enum { KHE_STATE_NONE, KHE_STATE_ONE, KHE_STATE_MANY } state;
  KHE_TASK_BOUND tb2;  int i;  KHE_RESOURCE_GROUP res;
  state = KHE_STATE_NONE;
  res = NULL;  /* keep compiler happy */
  MArrayForEach(task->task_bounds, &tb2, &i) if( tb2 != tb )
  {
    switch( state )
    {
      case KHE_STATE_NONE:

	res = KheTaskBoundResourceGroup(tb2);
	state = KHE_STATE_ONE;
	break;

      case KHE_STATE_ONE:

	KheSolnResourceGroupBegin(task->soln, KheTaskResourceType(task));
	KheSolnResourceGroupUnion(task->soln, res);
	KheSolnResourceGroupIntersect(task->soln,
	  KheTaskBoundResourceGroup(tb2));
	state = KHE_STATE_MANY;
	break;

      case KHE_STATE_MANY:

	KheSolnResourceGroupIntersect(task->soln,
	  KheTaskBoundResourceGroup(tb2));
	break;
    }
  }
  if( state == KHE_STATE_NONE )
    res = KheResourceTypeFullResourceGroup(KheTaskResourceType(task));
  else if( state == KHE_STATE_MANY )
    res = KheSolnResourceGroupEnd(task->soln);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "relation with enclosing tasking"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheTaskTasking(KHE_TASK task)                                */
/*                                                                           */
/*  Return the tasking containing task, or NULL if none.                     */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheTaskTasking(KHE_TASK task)
{
  return task->tasking;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetTasking(KHE_TASK task, KHE_TASKING tasking)               */
/*                                                                           */
/*  Set the tasking attribute of task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetTasking(KHE_TASK task, KHE_TASKING tasking)
{
  task->tasking = tasking;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskTaskingIndex(KHE_TASK task)                                   */
/*                                                                           */
/*  Return task's index in the enclosing tasking, or -1 if none.             */
/*                                                                           */
/*****************************************************************************/

int KheTaskTaskingIndex(KHE_TASK task)
{
  return task->tasking_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetTaskingIndex(KHE_TASK task, int tasking_index)            */
/*                                                                           */
/*  Set the tasking_index attributes of task.                                */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetTaskingIndex(KHE_TASK task, int tasking_index)
{
  task->tasking_index = tasking_index;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand monitor domains"                                       */
/*                                                                           */
/*  Implementation note.  The domains of the ordinary demand monitors of     */
/*  a task depend on the matching type, the domain of the task, the chain    */
/*  of assignments out of the task (defining its root task), and the         */
/*  domain of the root task.                                                 */
/*                                                                           */
/*  The precise dependence is given by KheTaskMatchingDomain immediately     */
/*  below.  The other functions in this submodule implement the changes      */
/*  required when one of the things that the domains depend on changes.      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task)                  */
/*                                                                           */
/*  Return a suitable domain for the matching demand nodes of task.          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task)
{
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      return KheTaskDomain(task);

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      return KheTaskDomain(KheTaskRoot(task));

    default:

      MAssert(false, "KheTaskMatchingDomain internal error");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingReset(KHE_TASK task)                                 */
/*                                                                           */
/*  Reset the matching within task.                                          */
/*                                                                           */
/*  This function makes no attempt to move gracefully from one state to      */
/*  another; rather, it starts again from scratch.  This is useful for       */
/*  initializing, and also when the matching type changes, since in those    */
/*  cases a reset is the sensible way forward.                               */
/*                                                                           */
/*  This function is called separately for every task, so there is no need   */
/*  to worry about making recursive calls.                                   */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingReset(KHE_TASK task)
{
  KHE_RESOURCE_GROUP rg;  int i;  KHE_ORDINARY_DEMAND_MONITOR m;
  if( MArraySize(task->attached_monitors) > 0 )
  {
    rg = KheTaskMatchingDomain(task);
    MArrayForEach(task->attached_monitors, &m, &i)
      KheOrdinaryDemandMonitorSetDomain(m, rg,
	KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,    */
/*    KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)                           */
/*                                                                           */
/*  Recursively set the domains of the demand monitors of task and its       */
/*  descendants to rg, with change_type saying what kind of change this is.  */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  KHE_TASK child_task;  int i;

  /* change the domains within task itself */
  MArrayForEach(task->attached_monitors, &m, &i)
    KheOrdinaryDemandMonitorSetDomain(m, rg, change_type);

  /* change the domains within the followers */
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskMatchingDoSetDomain(child_task, rg, change_type);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DOMAIN_CHANGE_TYPE KheTaskChangeType(                       */
/*    KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)                  */
/*                                                                           */
/*  Return the type of change when moving from old_rg to new_rg.             */
/*                                                                           */
/*****************************************************************************/

static KHE_MATCHING_DOMAIN_CHANGE_TYPE KheTaskChangeType(
  KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)
{
  if( DEBUG2 )
  {
    fprintf(stderr, "calling KheTaskChangeType(");
    KheResourceGroupDebug(old_rg, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceGroupDebug(new_rg, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  if( KheResourceGroupSubset(new_rg, old_rg) )
    return KHE_MATCHING_DOMAIN_CHANGE_TO_SUBSET;
  else if( KheResourceGroupSubset(old_rg, new_rg) )
    return KHE_MATCHING_DOMAIN_CHANGE_TO_SUPERSET;
  else
    return KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)      */
/*                                                                           */
/*  Given that the domain of task is about to change to rg, update the       */
/*  matching in task appropriately, including updating the descendants       */
/*  of task if appropriate.                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)
{
  int i;  KHE_ORDINARY_DEMAND_MONITOR m;
  KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* set the domains to rg within task only */
      if( MArraySize(task->attached_monitors) > 0 )
      {
	change_type = KheTaskChangeType(task->domain, rg);
	MArrayForEach(task->attached_monitors, &m, &i)
	  KheOrdinaryDemandMonitorSetDomain(m, rg, change_type);
      }
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* set the domains to rg, but only if unassigned root task; */
      /* and in that case, set them in all the followers too      */
      if( task->target_task == NULL )
      {
	change_type = KheTaskChangeType(task->domain, rg);
	KheTaskMatchingDoSetDomain(task, rg, change_type);
      }
      break;

    default:

      MAssert(false, "KheTaskSetDomain internal error 4");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingAssign(KHE_TASK task, KHE_TASK target_task)          */
/*                                                                           */
/*  Given that task is about to be assigned to target_task, update the       */
/*  matching appropriately, including the descendants if appropriate.        */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingAssign(KHE_TASK task, KHE_TASK target_task)
{
  KHE_RESOURCE_GROUP new_rg;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* assignments don't affect matchings of these types */
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* domain is tightening from task->domain to new root's domain */
      new_rg = KheTaskDomain(KheTaskRoot(target_task));
      if( !KheResourceGroupEqual(task->domain, new_rg) )
	KheTaskMatchingDoSetDomain(task, new_rg,
	  KHE_MATCHING_DOMAIN_CHANGE_TO_SUBSET);
      break;

    default:

      MAssert(false, "KheTaskMatchingAssign internal error");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingUnAssign(KHE_TASK task)                              */
/*                                                                           */
/*  Given that task is about to be unassigned, update the matching           */
/*  appropriately, including the descendants if appropriate.                 */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingUnAssign(KHE_TASK task)
{
  KHE_RESOURCE_GROUP old_rg;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* assignments don't affect matchings of these types */
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* domain is loosening from old_rg to task->domain */
      old_rg = KheTaskDomain(KheTaskRoot(task));
      if( !KheResourceGroupEqual(task->domain, old_rg) )
	KheTaskMatchingDoSetDomain(task, task->domain,
	  KHE_MATCHING_DOMAIN_CHANGE_TO_SUPERSET);
      break;

    default:

      MAssert(false, "KheTaskMatchingUnAssign internal error");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time assignment"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAssignTime(KHE_TASK task, int assigned_time_index)           */
/*                                                                           */
/*  Inform task that the enclosing meet has been assigned this time.         */
/*                                                                           */
/*****************************************************************************/

void KheTaskAssignTime(KHE_TASK task, int assigned_time_index)
{
  KHE_EVENNESS_HANDLER eh;
  if( task->assigned_rs != NULL )
    KheResourceInSolnAssignTime(task->assigned_rs, task, assigned_time_index);
  eh = KheSolnEvennessHandler(task->soln);
  if( eh != NULL )
    KheEvennessHandlerAddTask(eh, task, assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index)         */
/*                                                                           */
/*  Inform task that the enclosing meet has been unassigned this time.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index)
{
  KHE_EVENNESS_HANDLER eh;
  if( DEBUG5 )
  {
    fprintf(stderr, "[ KheTaskUnAssignTime(");
    KheTaskDebug(task, 2, -1, stderr);
    fprintf(stderr, ", %d)\n", assigned_time_index);
  }
  if( task->assigned_rs != NULL )
    KheResourceInSolnUnAssignTime(task->assigned_rs, task, assigned_time_index);
  eh = KheSolnEvennessHandler(task->soln);
  if( eh != NULL )
    KheEvennessHandlerDeleteTask(eh, task, assigned_time_index);
  if( DEBUG5 )
    fprintf(stderr, "] KheTaskUnAssignTime returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "split and merge"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoSplit(KHE_TASK task1, KHE_TASK task2, int durn1,           */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Split task at durn1 into task2, including adding task2 to meet2.         */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoSplit(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2)
{
  int i, duration2;  KHE_ORDINARY_DEMAND_MONITOR m;

  /* back, soln, and enclosing meet */
  task2->back = task1->back;
  task2->soln = task1->soln;
  KheSolnAddTask(task1->soln, task2);  /* will set task2->soln_index */
  MAssert(meet2 != NULL, "KheTaskSplit internal error");
  task2->meet = meet2;
  KheMeetAddTask(meet2, task2, false);  /* will set task2->meet_index */

  /* set domain, and tasking temporarily */
  task2->domain = task1->domain;
  task2->tasking = NULL;
  task2->tasking_index = -1;

  /* same fixed values */
  task2->target_fixed = task1->target_fixed;

  /* same target task as task */
  task2->target_task = task1->target_task;
  if( task2->target_task != NULL )
    MArrayAddLast(task1->target_task->assigned_tasks, task2);

  /* assigned tasks stay with task */
  task2->visit_num = task1->visit_num;
  KheTaskReferenceCountIncrement(task2);
  task2->copy = NULL;

  /* split the monitors between task1 and task2 */
  if( MArraySize(task1->all_monitors) > 0 )
  {
    duration2 = MArraySize(task1->all_monitors) - durn1;
    MAssert(duration2 >= 1, "KheTaskSplit internal error");
    for( i = durn1;  i < MArraySize(task1->all_monitors);  i++ )
    {
      m = MArrayGet(task1->all_monitors, i);
      KheOrdinaryDemandMonitorSetTaskAndOffset(m, task2, i - durn1);
      MArrayAddLast(task2->all_monitors, m);
    }
    MArrayDropFromEnd(task1->all_monitors, duration2);
    MArrayForEach(task1->attached_monitors, &m, &i)
      if( KheOrdinaryDemandMonitorTask(m) == task2 )
      {
	MArrayRemove(task1->attached_monitors, i);
	MArrayAddLast(task2->attached_monitors, m);
	i--;
      }
  }

  /* initialize assigned_rs and inform it of what is happening */
  task2->assigned_rs = task1->assigned_rs;
  if( task1->assigned_rs != NULL )
    KheResourceInSolnSplitTask(task1->assigned_rs, task1, task2);

  /* initialize event_resource_in_soln and inform it of what is happening */
  task2->event_resource_in_soln = task1->event_resource_in_soln;
  if( task2->event_resource_in_soln != NULL )
    KheEventResourceInSolnSplitTask(task2->event_resource_in_soln, task1,task2);

  /* add to tasking, if any (non-kernel, but worry about that later) */
  if( task1->tasking != NULL )
    KheTaskingAddTask(task1->tasking, task2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoMerge(KHE_TASK task1, KHE_TASK task2)                      */
/*                                                                           */
/*  Merge task2 into task1, leaving it unlinked but not freed.               */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoMerge(KHE_TASK task1, KHE_TASK task2)
{
  int i, pos;  KHE_ORDINARY_DEMAND_MONITOR m;  KHE_TASK child_task;

  /* remove task2 from its tasking, if any */
  if( task2->tasking != NULL )
    KheTaskingDeleteTask(task2->tasking, task2);

  /* remove task2 from its target task, if any */
  if( task1->target_task != NULL )
  {
    if( !MArrayContains(task1->target_task->assigned_tasks, task2, &pos) )
      MAssert(false, "KheTaskMerge internal error");
    MArrayRemove(task1->target_task->assigned_tasks, pos);
  }

  /* move task2's assigned tasks to task1 */
  while( MArraySize(task2->assigned_tasks) > 0 )
  {
    child_task = MArrayRemoveLast(task2->assigned_tasks);
    child_task->target_task = task1;
    MArrayAddLast(task1->assigned_tasks, child_task);
  }

  /* move the demand monitors */
  if( MArraySize(task2->all_monitors) > 0 )
  {
    MArrayForEach(task2->all_monitors, &m, &i)
    {
      KheOrdinaryDemandMonitorSetTaskAndOffset(m, task1,
	MArraySize(task1->all_monitors));
      MArrayAddLast(task1->all_monitors, m);
    }
    MArrayClear(task2->all_monitors);
    MArrayAppend(task1->attached_monitors, task2->attached_monitors, i);
    MArrayClear(task2->attached_monitors);
  }

  /* inform the assigned_rs, if any */
  if( task1->assigned_rs != NULL )
    KheResourceInSolnMergeTask(task1->assigned_rs, task1, task2);

  /* inform the event resource in soln, if any */
  if( task1->event_resource_in_soln != NULL )
    KheEventResourceInSolnMergeTask(task1->event_resource_in_soln,
      task1, task2);

  /* delete the task from soln and optionally add it to the free list */
  KheSolnDeleteTask(task1->soln, task2);
  task2->meet = NULL;
  KheTaskReferenceCountDecrement(task2);  /* may delete task */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelSplit(KHE_TASK task1, KHE_TASK task2, int durn1,       */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Kernel operation for splitting task1 into task2 at durn1.                */
/*                                                                           */
/*  This operation does nothing about adding task bounds to task2.           */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelSplit(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2)
{
  KheTaskDoSplit(task1, task2, durn1, meet2);
  KheSolnOpTaskSplit(task1->soln, task1, task2, durn1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelSplitUndo(KHE_TASK task1, KHE_TASK task2, int durn1,   */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Kernel operation for undoing a split of task1 into task2 at durn1.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelSplitUndo(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2)
{
  KheTaskDoMerge(task1, task2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelMerge(KHE_TASK task1, KHE_TASK task2, int durn1,       */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Kernel operation for merging task1 into task2.                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelMerge(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2)
{
  KheSolnOpTaskMerge(task1->soln, task1, task2, durn1, meet2);
  KheTaskDoMerge(task1, task2);  /* must *follow* KheSolnOpTaskMerge!! */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelMergeUndo(KHE_TASK task1, KHE_TASK task2, int durn1,   */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Kernel operation for undoing a merge of task1 into task2 at durn1.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelMergeUndo(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2)
{
  KheTaskDoSplit(task1, task2, durn1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2)                   */
/*                                                                           */
/*  Check that it is safe to merge these two tasks.                          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2)
{
  return task1 != task2 &&
    task1->tasking == task2->tasking &&
    task1->target_task == task2->target_task &&
    task1->assigned_rs == task2->assigned_rs &&
    task1->event_resource_in_soln == task2->event_resource_in_soln &&
    KheResourceGroupEqual(task1->domain, task2->domain);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assignment (basic functions)"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAssignResource(KHE_TASK task, KHE_RESOURCE r)              */
/*                                                                           */
/*  Task has just been assigned to a task that has a resource, so inform     */
/*  it and its descendants of that fact.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK child_task;  int i;
  task->assigned_rs = task->target_task->assigned_rs;
  KheResourceInSolnAssignResource(task->assigned_rs, task);
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnAssignResource(task->event_resource_in_soln, task, r);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskDoAssignResource(child_task, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoUnAssignResource(KHE_TASK task, KHE_RESOURCE r)            */
/*                                                                           */
/*  Task has just been unassigned from a task that has a resource, so        */
/*  inform it and its descendants of that fact.                              */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoUnAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK child_task;  int i;
  KheResourceInSolnUnAssignResource(task->assigned_rs, task);
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnUnAssignResource(task->event_resource_in_soln,
      task, r);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskDoUnAssignResource(child_task, r);
  task->assigned_rs = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAssign(KHE_TASK task, KHE_TASK target_task)                */
/*                                                                           */
/*  Carry out the task assignment operation, without the initial check.      */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAssign(KHE_TASK task, KHE_TASK target_task)
{
  /* update the matching */
  if( MArraySize(task->all_monitors) > 0 )
    KheTaskMatchingAssign(task, target_task);

  /* record the assignment */
  task->target_task = target_task;
  MArrayAddLast(target_task->assigned_tasks, task);

  /* if assigning a resource, inform descendants of task */
  if( target_task->assigned_rs != NULL )
    KheTaskDoAssignResource(task,
      KheResourceInSolnResource(target_task->assigned_rs));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoUnAssign(KHE_TASK task)                                    */
/*                                                                           */
/*  Carry out the task unassign operation, without the initial check.        */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoUnAssign(KHE_TASK task)
{
  int pos;

  /* update the matching */
  if( MArraySize(task->all_monitors) > 0 )
    KheTaskMatchingUnAssign(task);

  /* if unassigning a resource, inform descendants of task */
  if( task->target_task->assigned_rs != NULL )
    KheTaskDoUnAssignResource(task,
      KheResourceInSolnResource(task->target_task->assigned_rs));

  /* record the unassignment */
  if( !MArrayContains(task->target_task->assigned_tasks, task, &pos) )
    MAssert(false, "KheTaskUnAssign internal error");
  MArrayRemove(task->target_task->assigned_tasks, pos);
  task->target_task = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoMove(KHE_TASK task, KHE_TASK target_task)                  */
/*                                                                           */
/*  Carry out a move, without reporting it to the solution path.             */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoMove(KHE_TASK task, KHE_TASK target_task)
{
  if( task->target_task != NULL )
    KheTaskDoUnAssign(task);
  if( target_task != NULL )
    KheTaskDoAssign(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelMove(KHE_TASK task, KHE_TASK target_task)              */
/*                                                                           */
/*  Move task to target_task, assuming all is well.                          */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelMove(KHE_TASK task, KHE_TASK target_task)
{
  KheSolnOpTaskMove(task->soln, task, task->target_task, target_task);
  KheTaskDoMove(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelMoveUndo(KHE_TASK task,                                */
/*    KHE_TASK old_target_task, KHE_TASK new_target_task)                    */
/*                                                                           */
/*  Undo KheTaskKernelMove.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelMoveUndo(KHE_TASK task, KHE_TASK old_target_task,
  KHE_TASK new_target_task)
{
  KheTaskDoMove(task, old_target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task)               */
/*                                                                           */
/*  Check whether task can be moved to target_task.                          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task)
{
  /* the current assignment must not be fixed */
  if( task->target_fixed )
  {
    if( DEBUG3 )
      fprintf(stderr, "  KheTaskMoveCheck returning false (fixed)\n");
    return false;
  }

  /* the task must not be a cycle task */
  if( KheTaskIsCycleTask(task) )
  {
    if( DEBUG3 )
      fprintf(stderr, "  KheTaskMoveCheck returning false (cycle task)\n");
    return false;
  }

  /* the move must actually change something */
  if( task->target_task == target_task )
  {
    if( DEBUG3 )
      fprintf(stderr, "  KheTaskMoveCheck returning false (nochange)\n");
    return false;
  }

  /* if target_task != NULL, resource domains must match */
  if( target_task != NULL &&
      !KheResourceGroupSubset(target_task->domain, task->domain) )
  {
    if( DEBUG3 )
    {
      fprintf(stderr, "  KheTaskMoveCheck returning false (subset ");
      KheResourceGroupDebug(target_task->domain, 1, -1, stderr);
      fprintf(stderr, ", ");
      KheResourceGroupDebug(task->domain, 1, -1, stderr);
      fprintf(stderr, ")\n");
    }
    return false;
  }

  /* no problems, move is allowed */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMove(KHE_TASK task, KHE_TASK target_task)                    */
/*                                                                           */
/*  Move task to target_task.                                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMove(KHE_TASK task, KHE_TASK target_task)
{
  if( !KheTaskMoveCheck(task, target_task) )
    return false;
  KheTaskKernelMove(task, target_task);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assignment (helper functions)"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task)             */
/*                                                                           */
/*  Return true if task can be assigned to target_task.                      */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task)
{
  return task->target_task == NULL && KheTaskMoveCheck(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task)                  */
/*                                                                           */
/*  Assign task to target_task.                                              */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task)
{
  return task->target_task == NULL && KheTaskMove(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssignCheck(KHE_TASK task)                                 */
/*                                                                           */
/*  Return true if task may be unassigned.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheTaskUnAssignCheck(KHE_TASK task)
{
  return KheTaskMoveCheck(task, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssign(KHE_TASK task)                                      */
/*                                                                           */
/*  Unassign task.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheTaskUnAssign(KHE_TASK task)
{
  return KheTaskMove(task, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2)                    */
/*                                                                           */
/*  Check whether swapping task1 and task2 would succeed.                    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2)
{
  return KheTaskMoveCheck(task1, task2->target_task) &&
    KheTaskMoveCheck(task2, task1->target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2)                         */
/*                                                                           */
/*  Swap task1 and task2.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2)
{
  KHE_TASK task1_target_task;
  if( !KheTaskSwapCheck(task1, task2) )
    return false;
  task1_target_task = task1->target_task;
  KheTaskMove(task1, task2->target_task);
  KheTaskMove(task2, task1_target_task);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assignment (queries)"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskAsst(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the task that task is currently assigned to, or NULL if none.     */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskAsst(KHE_TASK task)
{
  return task->target_task;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskAssignedToCount(KHE_TASK target_task)                         */
/*                                                                           */
/*  Return the number of tasks assigned to target_task.                      */
/*                                                                           */
/*****************************************************************************/

int KheTaskAssignedToCount(KHE_TASK target_task)
{
  return MArraySize(target_task->assigned_tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i)                  */
/*                                                                           */
/*  Return the i'th task assigned to target_task.                            */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i)
{
  return MArrayGet(target_task->assigned_tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskTotalDuration(KHE_TASK task)                                  */
/*                                                                           */
/*  Return the total duration of task and the tasks that are assigned to     */
/*  it, directly and indirectly.                                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskTotalDuration(KHE_TASK task)
{
  int res, i;  KHE_TASK child_task;
  res = KheTaskDuration(task);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    res += KheTaskTotalDuration(child_task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  float KheTaskTotalWorkload(KHE_TASK task)                                */
/*                                                                           */
/*  Return the total workload of task and the tasks that are assigned to     */
/*  it, directly and indirectly.                                             */
/*                                                                           */
/*****************************************************************************/

float KheTaskTotalWorkload(KHE_TASK task)
{
  float res;  int i;  KHE_TASK child_task;
  res = KheTaskWorkload(task);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    res += KheTaskTotalWorkload(child_task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskRoot(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the root of the task's assignment chain.                          */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskRoot(KHE_TASK task)
{
  while( task->target_task != NULL )
    task = task->target_task;
  return task;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assignment (fixing and unfixing)"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAssignFix(KHE_TASK task)                                   */
/*                                                                           */
/*  Fix the assignment of task.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAssignFixRecursive(KHE_TASK task)
{
  KHE_TASK sub_task;  int i;
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnTaskAssignFix(task->event_resource_in_soln);
  MArrayForEach(task->assigned_tasks, &sub_task, &i)
    if( sub_task->target_fixed )
      KheTaskDoAssignFixRecursive(sub_task);
}

static void KheTaskDoAssignFix(KHE_TASK task)
{
  task->target_fixed = true;
  KheTaskDoAssignFixRecursive(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAssignUnFix(KHE_TASK task)                                 */
/*                                                                           */
/*  Unfix the assignment of task.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAssignUnFixRecursive(KHE_TASK task)
{
  KHE_TASK sub_task;  int i;
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnTaskAssignUnFix(task->event_resource_in_soln);
  MArrayForEach(task->assigned_tasks, &sub_task, &i)
    if( sub_task->target_fixed )
      KheTaskDoAssignUnFixRecursive(sub_task);
}

static void KheTaskDoAssignUnFix(KHE_TASK task)
{
  task->target_fixed = false;
  KheTaskDoAssignUnFixRecursive(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAssignFix(KHE_TASK task)                               */
/*                                                                           */
/*  The kernel operation which fixes the assignment of task.                 */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAssignFix(KHE_TASK task)
{
  KheTaskDoAssignFix(task);
  KheSolnOpTaskAssignFix(task->soln, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAssignFixUndo(KHE_TASK task)                           */
/*                                                                           */
/*  Undo KheTaskKernelAssignFix.                                             */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAssignFixUndo(KHE_TASK task)
{
  KheTaskDoAssignUnFix(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAssignUnFix(KHE_TASK task)                             */
/*                                                                           */
/*  The kernel operation which unfixes the assignment of task.               */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAssignUnFix(KHE_TASK task)
{
  KheTaskDoAssignUnFix(task);
  KheSolnOpTaskAssignUnFix(task->soln, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAssignUnFixUndo(KHE_TASK task)                         */
/*                                                                           */
/*  Undo KheTaskKernelAssignFix.                                             */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAssignUnFixUndo(KHE_TASK task)
{
  KheTaskDoAssignFix(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAssignFix(KHE_TASK task)                                     */
/*                                                                           */
/*  Prevent the assignment of task from changing.                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskAssignFix(KHE_TASK task)
{
  if( !task->target_fixed )
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "[ KheTaskAssignFix(");
      KheTaskDebug(task, 1, -1, stderr);
      fprintf(stderr, ")\n");
    }
    KheTaskKernelAssignFix(task);
    if( DEBUG1 )
      fprintf(stderr, "]\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAssignUnFix(KHE_TASK task)                                   */
/*                                                                           */
/*  Allow the assignment of task to change.                                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskAssignUnFix(KHE_TASK task)
{
  if( task->target_fixed )
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "[ KheTaskAssignUnFix(");
      KheTaskDebug(task, 1, -1, stderr);
      fprintf(stderr, ")\n");
    }
    KheTaskKernelAssignUnFix(task);
    if( DEBUG1 )
      fprintf(stderr, "]\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignIsFixed(KHE_TASK task)                                 */
/*                                                                           */
/*  Return true if the current assignment of task is fixed.                  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignIsFixed(KHE_TASK task)
{
  return task->target_fixed;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskFirstUnFixed(KHE_TASK task)                              */
/*                                                                           */
/*  Return the first unfixed task on the chain of assignments leading out    */
/*  of task, or NULL if none.                                                */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskFirstUnFixed(KHE_TASK task)
{
  while( task != NULL && task->target_fixed )
    task = task->target_task;
  return task;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cycle tasks and resource assignment"                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsCycleTask(KHE_TASK task)                                   */
/*                                                                           */
/*  Return true if task is a cycle task.                                     */
/*                                                                           */
/*  Implementation note.  To save a boolean field, we don't store this       */
/*  condition explicitly in task.  Instead, we use the fact that the only    */
/*  tasks that have an assigned resource without being assigned to another   */
/*  task are cycle tasks.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsCycleTask(KHE_TASK task)
{
  return task->target_task == NULL && task->assigned_rs != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheSolnResourceCycleTask(KHE_SOLN soln, KHE_RESOURCE r)         */
/*                                                                           */
/*  Return the cycle task for r in soln.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheSolnResourceCycleTask(KHE_SOLN soln, KHE_RESOURCE r)
{
  return KheSolnTask(soln, KheResourceInstanceIndex(r));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r)             */
/*                                                                           */
/*  Check whether it is possible to move task from wherever it is now to r.  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnTask(task->soln, KheResourceInstanceIndex(r));
  return KheTaskMoveCheck(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r)                  */
/*                                                                           */
/*  Move task from wherever it is now to r.                                  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnTask(task->soln, KheResourceInstanceIndex(r));
  return KheTaskMove(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r)           */
/*                                                                           */
/*  Return true if r can be assigned to task.                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnResourceCycleTask(task->soln, r);
  return KheTaskAssignCheck(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r)                */
/*                                                                           */
/*  Assign r to task if possible.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnResourceCycleTask(task->soln, r);
  return KheTaskAssign(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssignResourceCheck(KHE_TASK task)                         */
/*                                                                           */
/*  Return true if task may be unassigned.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheTaskUnAssignResourceCheck(KHE_TASK task)
{
  return KheTaskUnAssignCheck(task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssignResource(KHE_TASK task)                              */
/*                                                                           */
/*  Unassign task.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheTaskUnAssignResource(KHE_TASK task)
{
  return KheTaskUnAssign(task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheTaskAsstResource(KHE_TASK task)                          */
/*                                                                           */
/*  Return the resource that task is assigned to, or NULL if none.           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheTaskAsstResource(KHE_TASK task)
{
  return task->assigned_rs == NULL ? NULL :
    KheResourceInSolnResource(task->assigned_rs);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task domains and bounds"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task)                          */
/*                                                                           */
/*  Return the domain of task.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task)
{
  return task->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)            */
/*                                                                           */
/*  Set the domain of task to rg, assuming that this is safe to do.          */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)
{
  int assigned_time_index;  KHE_EVENNESS_HANDLER eh;
  if( MArraySize(task->all_monitors) > 0 )
    KheTaskMatchingSetDomain(task, rg);
  assigned_time_index = KheMeetAssignedTimeIndex(task->meet);
  if( assigned_time_index != NO_TIME_INDEX )
  {
    eh = KheSolnEvennessHandler(task->soln);
    if( eh != NULL )
    {
      KheEvennessHandlerDeleteTask(eh, task, assigned_time_index);
      task->domain = rg;
      KheEvennessHandlerAddTask(eh, task, assigned_time_index);
    }
    else
      task->domain = rg;
  }
  else
    task->domain = rg;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)             */
/*                                                                           */
/*  Add tb to task.                                                          */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)
{
  /* update task and tb */
  MArrayAddLast(task->task_bounds, tb);
  KheTaskBoundAddTask(tb, task);

  /* change task's domain */
  KheTaskDoSetDomain(task, KheTaskAddBoundDomain(task, tb));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb,          */
/*    int pos, KHE_RESOURCE_GROUP new_domain)                                */
/*                                                                           */
/*  Delete tb from task.  Here pos is tb's position in task, and new_domain  */
/*  is the value of task's domain after the deletion.                        */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb,
  int pos, KHE_RESOURCE_GROUP new_domain)
{
  /* update task and tb */
  MAssert(MArrayGet(task->task_bounds, pos) == tb,
    "KheTaskDoDeleteTaskBound internal error");
  MArrayDropAndPlug(task->task_bounds, pos);
  KheTaskBoundDeleteTask(tb, task);

  /* change task's domain */
  KheTaskDoSetDomain(task, new_domain);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)         */
/*                                                                           */
/*  Kernel operation which adds tb to task.                                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)
{
  KheSolnOpTaskAddTaskBound(task->soln, task, tb);
  KheTaskDoAddTaskBound(task, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelAddTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb)     */
/*                                                                           */
/*  Undo KheTaskKernelAddTaskBound.                                          */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelAddTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb)
{
  int pos;  KHE_RESOURCE_GROUP new_domain;
  if( !MArrayContains(task->task_bounds, tb, &pos) )
    MAssert(false, "KheTaskKernelAddTaskBoundUndo internal error");
  new_domain = KheTaskDeleteBoundDomain(task, tb);
  KheTaskDoDeleteTaskBound(task, tb, pos, new_domain);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)      */
/*                                                                           */
/*  Delete tb from task.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)
{
  int pos;  KHE_RESOURCE_GROUP new_domain;
  if( !MArrayContains(task->task_bounds, tb, &pos) )
    MAssert(false, "KheTaskKernelAddTaskBoundUndo internal error");
  new_domain = KheTaskDeleteBoundDomain(task, tb);
  KheTaskDoDeleteTaskBound(task, tb, pos, new_domain);
  KheSolnOpTaskDeleteTaskBound(task->soln, task, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskKernelDeleteTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb)  */
/*                                                                           */
/*  Undo KheTaskKernelDeleteTaskBound.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTaskKernelDeleteTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb)
{
  KheTaskDoAddTaskBound(task, tb);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAddTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb)          */
/*                                                                           */
/*  Return true if it is safe to add tb to task.                             */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAddTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb)
{
  KHE_RESOURCE_GROUP rg;

  /* rg must have the right type */
  rg = KheTaskBoundResourceGroup(tb);
  MAssert(KheResourceGroupResourceType(task->domain) ==
    KheResourceGroupResourceType(rg),
    "KheTaskAddTaskBoundCheck: rg has wrong type");

  /* task may not be a cycle task */
  if( KheTaskIsCycleTask(task) )
    return false;

  /* rg must be a superset of any target task's domain */
  if( task->target_task != NULL &&
      !KheResourceGroupSubset(task->target_task->domain, rg) )
    return false;

  /* all OK */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)               */
/*                                                                           */
/*  Add tb to task.  This has already been checked for safety by a call to   */
/*  KheTaskTaskBoundMakeCheck above.                                         */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)
{
  if( !KheTaskAddTaskBoundCheck(task, tb) )
    return false;

  /* carry out the kernel add operation */
  KheTaskKernelAddTaskBound(task, tb);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskDoDeleteTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb,     */
/*    int *pos, KHE_RESOURCE_GROUP *new_domain)                              */
/*                                                                           */
/*  Check that it is safe to delete tb from task.                            */
/*                                                                           */
/*****************************************************************************/

bool KheTaskDoDeleteTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb,
  int *pos, KHE_RESOURCE_GROUP *new_domain)
{
  KHE_TASK child_task;  int i;

  /* abort if tb is not present in task */
  if( !MArrayContains(task->task_bounds, tb, pos) )
    MAssert(false, "KheTaskDeleteTaskBoundCheck:  task does not contain tb");

  /* task may not be a cycle task */
  if( KheTaskIsCycleTask(task) )
    return false;

  /* check compatibility with descendant domains, if any */
  *new_domain = KheTaskDeleteBoundDomain(task, tb);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    if( !KheResourceGroupSubset(*new_domain, child_task->domain) )
      return false;

  /* all in order */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskDeleteTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb)       */
/*                                                                           */
/*  Check that it is safe to delete tb from task.                            */
/*                                                                           */
/*****************************************************************************/

bool KheTaskDeleteTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb)
{
  int pos;  KHE_RESOURCE_GROUP new_domain;
  return KheTaskDoDeleteTaskBoundCheck(task, tb, &pos, &new_domain);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)            */
/*                                                                           */
/*  Delete tb from task.                                                     */
/*                                                                           */
/*****************************************************************************/

bool KheTaskDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb)
{
  int pos;  KHE_RESOURCE_GROUP new_domain;
  /* KHE_TASK_BOUND tb2;  int index; */

  /* make sure that the operation can proceed */
  if( !KheTaskDoDeleteTaskBoundCheck(task, tb, &pos, &new_domain) )
    return false;

  /* carry out the kernel delete operation */
  /* the body of KheTaskKernelDeleteTaskBound, minus things already done */
  KheTaskDoDeleteTaskBound(task, tb, pos, new_domain);
  KheSolnOpTaskDeleteTaskBound(task->soln, task, tb);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskTaskBoundCount(KHE_TASK task)                                 */
/*                                                                           */
/*  Return the number of task bounds of task.                                */
/*                                                                           */
/*****************************************************************************/

int KheTaskTaskBoundCount(KHE_TASK task)
{
  return MArraySize(task->task_bounds);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_BOUND KheTaskTaskBound(KHE_TASK task, int i)                    */
/*                                                                           */
/*  Return the i'th task bound of task.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_TASK_BOUND KheTaskTaskBound(KHE_TASK task, int i)
{
  return MArrayGet(task->task_bounds, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand monitors"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingBegin(KHE_TASK task)                                 */
/*                                                                           */
/*  Begin the matching in task.                                              */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingBegin(KHE_TASK task)
{
  int i;  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_ORDINARY_DEMAND_MONITOR m; 
  if( task->meet != NULL )
    for( i = 0;  i < KheMeetDuration(task->meet);  i++ )
    {
      dc = KheMeetDemandChunk(task->meet, i);
      m = KheOrdinaryDemandMonitorMake(task->soln, dc, task, i);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) task->soln,
	(KHE_MONITOR) m);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingEnd(KHE_TASK task)                                   */
/*                                                                           */
/*  End the matching in task.                                                */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingEnd(KHE_TASK task)
{
  while( MArraySize(task->all_monitors) > 0 )
    KheOrdinaryDemandMonitorDelete(MArrayLast(task->all_monitors));
  MAssert(MArraySize(task->attached_monitors) == 0,
    "KheTaskMatchingEnd internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of task are attached.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  MArrayForEach(task->all_monitors, &m, &i)
    if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorAttachToSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of task are detached.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task)
{
  while( MArraySize(task->attached_monitors) > 0 )
    KheMonitorDetachFromSoln((KHE_MONITOR) MArrayLast(task->attached_monitors));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight)        */
/*                                                                           */
/*  Change the weight of all the attached monitors of task.                  */
/*  NB it would actually be wrong to do this for all monitors.               */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  MArrayForEach(task->attached_monitors, &m, &i)
    KheOrdinaryDemandMonitorSetWeight(m, new_weight);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskDemandMonitorCount(KHE_TASK task)                             */
/*                                                                           */
/*  Return the number of demand monitors of task.                            */
/*                                                                           */
/*****************************************************************************/

int KheTaskDemandMonitorCount(KHE_TASK task)
{
  return MArraySize(task->all_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i)   */
/*                                                                           */
/*  Return the i'th demand monitor of task.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i)
{
  return MArrayGet(task->all_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/* void KheTaskAddDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)*/
/*                                                                           */
/*  Add m to task.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskAddDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  MArrayAddLast(task->all_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDeleteDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Remove m from task.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTaskDeleteDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  int pos;
  if( !MArrayContains(task->all_monitors, m, &pos) )
    MAssert(false, "KheTaskDeleteDemandMonitor internal error");
  MArrayRemove(task->all_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAttachDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Attach m to task.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheTaskAttachDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  MArrayAddLast(task->attached_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDetachDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Detach m from task.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTaskDetachDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  int pos;
  if( !MArrayContains(task->attached_monitors, m, &pos) )
    MAssert(false, "KheTaskDetachDemandMonitor internal error");
  MArrayRemove(task->attached_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet, KML_ERROR *ke)  */
/*                                                                           */
/*  Make a task based on task_elt and add it to meet.                        */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet, KML_ERROR *ke)
{
  char *role, *ref;  KHE_TASK task;  KHE_INSTANCE ins;
  KHE_EVENT_RESOURCE er;  KHE_EVENT e;
  KHE_RESOURCE resource, preassigned_resource;

  /* check task_elt */
  ins = KheSolnInstance(KheMeetSoln(meet));
  if( !KmlCheck(task_elt, "Reference : $Role", ke) &&
      !KmlCheck(task_elt, "R : $Role", ke) )
    return false;
	  
  /* make sure the role makes sense; find the event resource */
  role = KmlExtractText(KmlChild(task_elt, 0));
  e = KheMeetEvent(meet);
  MAssert(e != NULL, "KheTaskMakeFromKml: no event (internal error)");
  if( !KheEventRetrieveEventResource(e, role, &er) )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Role> \"%s\" unknown in <Event> \"%s\"", role, KheEventId(e));

  /* make sure that meet does not already have a task for this */
  if( KheMeetRetrieveTask(meet, role, &task) )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Role> \"%s\" already assigned in <Event> \"%s\"", role, KheEventId(e));

  /* make sure the reference is to a legal resource */
  ref = KmlAttributeValue(task_elt, 0);
  if( !KheInstanceRetrieveResource(ins, ref, &resource) )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> Reference \"%s\" unknown", ref);

  /* check that resource is compatible with preassignment */
  preassigned_resource = KheEventResourcePreassignedResource(er);
  if( preassigned_resource != NULL && preassigned_resource != resource )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> \"%s\" conflicts with preassigned resource \"%s\"",
      ref, KheResourceId(preassigned_resource));

  /* check that resource is compatible with ResourceType */
  if( KheResourceResourceType(resource) != KheEventResourceResourceType(er) )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> of type \"%s\" where type \"%s\" expected",
      KheResourceTypeId(KheResourceResourceType(resource)),
      KheResourceTypeId(KheEventResourceResourceType(er)));

  /* make a task, link it to meet, and assign resource to it */
  task = KheTaskMake(KheMeetSoln(meet), KheEventResourceResourceType(er),
    meet, er);
  if( !KheTaskAssignResource(task, resource) )
    return KmlError(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> \"%s\" unassignable here", ref);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskCheckForWriting(KHE_TASK task)                               */
/*                                                                           */
/*  Check that task can be written safely; abort with error message if not.  */
/*                                                                           */
/*****************************************************************************/

void KheTaskCheckForWriting(KHE_TASK task)
{
  KHE_RESOURCE ass_r, pre_r;  KHE_EVENT e;  char *role;  KHE_EVENT_RESOURCE er;

  /* only interested in tasks derived from event resources */
  er = KheTaskEventResource(task);
  if( er == NULL )
    return;

  /* get assigned and preassigned resources and check that they have Ids */
  role = KheEventResourceRole(er);
  e = KheEventResourceEvent(er);
  ass_r = KheTaskAsstResource(task);
  pre_r = KheEventResourcePreassignedResource(er);
  MAssert(ass_r == NULL || KheResourceId(ass_r) != NULL,
    "KheArchiveWrite: resource without Id assigned to event %s (role %s)",
    KheEventId(e), role == NULL ? "(none)" : role);
  MAssert(pre_r == NULL || KheResourceId(pre_r) != NULL,
    "KheArchiveWrite: resource without Id preassigned to event %s (role %s)",
    KheEventId(e), role == NULL ? "(none)" : role);

  if( pre_r != NULL )
  {
    /* if preassigned, check that the assigned resource is equal to it */
    MAssert(ass_r != NULL,
     "KheArchiveWrite: in event %s, event resource with preassigned resource %s"
      " has task with missing resource assignment",
      KheEventId(e), KheResourceId(pre_r));
    MAssert(ass_r == pre_r,
     "KheArchiveWrite: in event %s, event resource with preassigned resource %s"
      " has task with inconsistent resource assignment %s",
      KheEventId(e), KheResourceId(pre_r), KheResourceId(ass_r));
  }
  else
  {
    /* if unpreassigned, must have a role */
    MAssert(role != NULL, "KheArchiveWrite: in event %s, unpreassigned event"
      "resource has no role", KheEventId(e));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMustWrite(KHE_TASK task, KHE_EVENT_RESOURCE er)              */
/*                                                                           */
/*  Return true if it is necessary to write task, because it is derived      */
/*  from an event resource and assigned a value which is not preassigned.    */
/*  This function assumes that KheTaskCheckForWriting returned successfully. */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMustWrite(KHE_TASK task)
{
  KHE_RESOURCE ass_r, pre_r;  KHE_EVENT_RESOURCE er;
  er = KheTaskEventResource(task);
  ass_r = KheTaskAsstResource(task);
  pre_r = er == NULL ? NULL : KheEventResourcePreassignedResource(er);
  return er != NULL && ass_r != NULL && ass_r != pre_r;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskWrite(KHE_TASK task, KML_FILE kf)                            */
/*                                                                           */
/*  Write task to kf.                                                        */
/*                                                                           */
/*  This function assumes that KheTaskCheckForWriting and KheTaskMustWrite   */
/*  returned successfully, and that the event from which the meet            */
/*  containing this task is derived has an Id.                               */
/*                                                                           */
/*****************************************************************************/

void KheTaskWrite(KHE_TASK task, KML_FILE kf)
{
  KHE_RESOURCE ass_r;  KHE_EVENT_RESOURCE er;

  /* print assigned resource (must be present, different from preassigned) */
  ass_r = KheTaskAsstResource(task);
  er = KheTaskEventResource(task);
  KmlEltAttributeEltPlainText(kf, "Resource", "Reference",
    KheResourceId(ass_r), "Role", KheEventResourceRole(er));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDebug(KHE_TASK task, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of task onto fp with the given verbosity and indent.         */
/*                                                                           */
/*****************************************************************************/

void KheTaskDebug(KHE_TASK task, int verbosity, int indent, FILE *fp)
{
  KHE_RESOURCE r;  KHE_TIME t;  KHE_TASK child_task;  int i, j, pos;
  ARRAY_KHE_EVENT_RESOURCE event_resources;  KHE_EVENT_RESOURCE er;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( verbosity > 1 )
      fprintf(fp, "[ ");
    if( KheTaskIsCycleTask(task) )
    {
      r = KheResourceInSolnResource(task->assigned_rs);
      fprintf(fp, "/%s/", KheResourceId(r) != NULL ? KheResourceId(r) : "-");
    }
    else
    {
      if( task->meet != NULL )
      {
	KheMeetDebug(task->meet, 1, -1, fp);
	fprintf(fp, ".%d", task->meet_index);
	t = KheMeetAsstTime(task->meet);
	if( t != NULL )
	{
	  fprintf(stderr, "$%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	  if( KheTaskDuration(task) > 1 )
	  {
	    i = KheTimeIndex(t) + KheTaskDuration(task) - 1;
	    t = KheInstanceTime(KheSolnInstance(KheTaskSoln(task)), i);
	    fprintf(stderr, "-%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	  }
	}
      }
      if( MArraySize(task->assigned_tasks) > 0 )
      {
	MArrayInit(event_resources);
	MArrayForEach(task->assigned_tasks, &child_task, &i)
	{
	  er = KheTaskEventResource(child_task);
	  if( !MArrayContains(event_resources, er, &pos) )
	    MArrayAddLast(event_resources, er); /* may add NULL */
	}
	fprintf(fp, "{");
	MArrayForEach(event_resources, &er, &i)
	{
	  if( i > 0 )
	    fprintf(fp, ", ");
	  fprintf(fp, "%s", er == NULL ? "??" :
	    KheEventId(KheEventResourceEvent(er)) == NULL ?  "-" :
	    KheEventId(KheEventResourceEvent(er)));
	  MArrayForEach(task->assigned_tasks, &child_task, &j)
	    if( KheTaskEventResource(child_task) == er )
	    {
	      if( child_task->meet == NULL )
		fprintf(fp, "+??");
	      else if( KheMeetAsstTime(child_task->meet) == NULL )
		fprintf(fp, "+__");
	      else
	      {
		t = KheMeetAsstTime(child_task->meet);
		fprintf(fp, "+%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	      }
	    }
	}
	fprintf(fp, "}");
	MArrayFree(event_resources);
      }
    }
    if( verbosity >= 2 )
    {
      fprintf(fp, ": ");
      KheResourceGroupDebug(task->domain, 1, -1, fp);
      if( task->assigned_rs != NULL )
      {
	fprintf(fp, " := ");
	KheResourceDebug(KheResourceInSolnResource(task->assigned_rs),
	  1, -1, fp);
      }
    }
    if( verbosity > 1 )
      fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
