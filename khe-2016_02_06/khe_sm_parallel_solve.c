
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
/*  FILE:         khe_sm_parallel_solve.c                                    */
/*  DESCRIPTION:  KheParallelSolve().  If you can't compile this file,       */
/*                see the makefile for a workaround.                         */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>
#include <math.h>
#if KHE_USE_PTHREAD
#include <pthread.h>
#endif

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_PARALLEL_SOLN - one solution, to be solved in parallel.              */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef struct khe_parallel_soln_rec {
  KHE_SOLN		soln;			** solution                  **
#if KHE_USE_PTHREAD
  pthread_t		thread;			** its thread                **
#endif
} *KHE_PARALLEL_SOLN;

typedef MARRAY(KHE_PARALLEL_SOLN) ARRAY_KHE_PARALLEL_SOLN;
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARALLEL_SOLN KheParallelSolnMake(KHE_SOLN soln, int diversifier)    */
/*                                                                           */
/*  Make one parallel solution with these attributes.                        */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_PARALLEL_SOLN KheParallelSolnMake(KHE_SOLN soln, int diversifier)
{
  KHE_PARALLEL_SOLN res;
  MMake(res);
  res->soln = soln;
  KheSolnSetDiversifier(soln, diversifier);
  ** res->thread is undefined here **
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnStart(KHE_PARALLEL_SOLN ps,                          */
/*    KHE_GENERAL_SOLVER solver)                                             */
/*                                                                           */
/*  Start ps on solver, preferably in parallel.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheParallelSolnStart(KHE_PARALLEL_SOLN ps,
  KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)
{
  MAssert(ps->soln != NULL && KheSolnInstance(ps->soln) != NULL,
    "KheParallelSolnStart internal error");
#if KHE_USE_PTHREAD
  pthread_create(&ps->thread, NULL, (void * (*)(void *)) solver, ps->soln);
#else
  ps->soln = solver(ps->soln);
#endif
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnStop(KHE_PARALLEL_SOLN ps)                           */
/*                                                                           */
/*  Stop ps on solver (or rather, wait until it stops), preferably in        */
/*  parallel.                                                                */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheParallelSolnStop(KHE_PARALLEL_SOLN ps)
{
#if KHE_USE_PTHREAD
  pthread_join(ps->thread, (void **) &ps->soln);
#endif
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnDelete(KHE_PARALLEL_SOLN ps)                         */
/*                                                                           */
/*  Delete ps, including its soln if non-NULL.                               */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheParallelSolnDelete(KHE_PARALLEL_SOLN ps)
{
  if( ps->soln != NULL )
    KheSolnDelete(ps->soln);
  MFree(ps);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheAverageCostByType(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,   */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the average cost of *parallel_solns by tag.                       */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_COST KheAverageCostByType(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  KHE_MONITOR_TAG tag, float *defect_count)
{
  int i, num, defects;  KHE_PARALLEL_SOLN ps;  KHE_COST cost;
  int hard_cost, soft_cost;
  defects = hard_cost = soft_cost = 0;
  MArrayForEach(*parallel_solns, &ps, &i)
  {
    cost = KheSolnCostByType(ps->soln, tag, &num);
    hard_cost += KheHardCost(cost);
    soft_cost += KheSoftCost(cost);
    defects += num;
  }
  *defect_count = (float) defects / MArraySize(*parallel_solns);
  return KheCost(hard_cost / MArraySize(*parallel_solns),
    soft_cost / MArraySize(*parallel_solns));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelDebugAverages(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,   */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of the average cost of *parallel_solns, by type.             */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheParallelDebugAverages(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  int verbosity, int indent, FILE *fp)
{
  int tag;  KHE_COST cost, total_cost;  char buff[30];
  float defect_count, total_defect_count;
  MAssert(MArraySize(*parallel_solns) >= 1, "KheParallelDebugAverages!");
  sprintf(buff, "Average of %d solutions", MArraySize(*parallel_solns));
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s%-31s %9s %13s\n", indent, "", buff, "Defects", "Cost");
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    total_cost = 0;  total_defect_count = 0;
    for( tag = 0;  tag < KHE_MONITOR_TAG_COUNT;  tag++ )
    {
      cost = KheAverageCostByType(parallel_solns, tag, &defect_count);
      if( cost != 0 || defect_count != 0 )
	fprintf(fp, "%*s%-34s %6.1f %13.5f\n", indent, "",
	  KheMonitorTagShow(tag), defect_count, KheCostShow(cost));
      total_cost += cost;
      total_defect_count += defect_count;
    }
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    fprintf(fp, "%*s%-34s %6.1f %13.5f\n", indent, "", "Total",
      total_defect_count, KheCostShow(total_cost));
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelDebugHistogram(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,  */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of histogram of solution costs.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheParallelDebugHistogram(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  int verbosity, int indent, FILE *fp)
{
  float min_cost, max_cost, cost;
  int min_index, max_index, index, freq, total_freq, i, j, increment, med;
  KHE_PARALLEL_SOLN ps;  ARRAY_INT frequencies;

  if( verbosity >= 1 )
  {
    ** find min and max costs **
    ps = MArrayFirst(*parallel_solns);
    min_cost = max_cost = KheCostShow(KheSolnCost(ps->soln));
    for( i = 1;  i < MArraySize(*parallel_solns);  i++ )
    {
      ps = MArrayGet(*parallel_solns, i);
      cost = KheCostShow(KheSolnCost(ps->soln));
      if( cost < min_cost )
	min_cost = cost;
      else if( cost > max_cost )
	max_cost = cost;
    }

    ** find min and max indexes (rounding the costs down and up) **
    min_index = (int) floor(min_cost);
    max_index = (int) ceil(max_cost);

    ** find increment **
    if( max_index - min_index <= 20 )
      increment = 1;
    else if( max_index - min_index <= 50 )
      increment = 2;
    else if( max_index - min_index <= 100 )
      increment = 5;
    else if( max_index - min_index <= 200 )
      increment = 10;
    else if( max_index - min_index <= 500 )
      increment = 20;
    else if( max_index - min_index <= 1000 )
      increment = 50;
    else
      increment = 200;

    ** round min_index down, and max_index up, to multiples of increment **
    min_index = increment * (min_index / increment);
    max_index = increment * ((max_index + increment - 1) / increment);

    ** initialize one array index for each index, max_index exclusive **
    MArrayInit(frequencies);
    MArrayFill(frequencies, (max_index - min_index) / increment, 0);

    ** fill the array **
    MArrayForEach(*parallel_solns, &ps, &i)
    {
      cost = KheCostShow(KheSolnCost(ps->soln));
      index = ((int) floor(cost) - min_index) / increment;
      MArrayInc(frequencies, index);
    }

    ** print it **
    total_freq = 0;
    med = MArraySize(*parallel_solns) / 2;
    MArrayForEachReverse(frequencies, &freq, &i)
    {
      fprintf(fp, "%*s%7.5f %c ", indent, "", (float) min_index + i*increment,
	total_freq <= med && total_freq + freq >= med ? 'M' : '|');
      for( j = 0;  j < freq;  j++ )
	fprintf(fp, "*");
      fprintf(fp, "\n");
      total_freq += freq;
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheParallelSolve(KHE_SOLN soln, int thread_count,               */
/*    KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)                        */
/*                                                                           */
/*  Make thread_count - 1 copies of soln, solve the resulting thread_count   */
/*  solutions in parallel, return the best solution, and delete the others.  */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_SOLN KheParallelSolve(KHE_SOLN soln, int thread_count,
  KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)
{
  ARRAY_KHE_PARALLEL_SOLN parallel_solns;  KHE_PARALLEL_SOLN ps, best_ps;
  int diversifier, i;  KHE_COST best_cost;  KHE_SOLN res;

  if( DEBUG1 )
    fprintf(stderr, "[ KheParallelSolve(soln, %d, solver) %s\n", thread_count,
      KHE_USE_PTHREAD ? "with threads" : "no threads, so sequential");

  ** make a total of thread_count diversified solutions **
  MAssert(thread_count >= 1, "KheParallelSolve: thread_count (%d) out of range",
    thread_count);
  MArrayInit(parallel_solns);
  diversifier = KheSolnDiver sifier(soln);
  for( i = 0;  i < thread_count;  i++ )
  {
    ps = KheParallelSolnMake(i==0 ? soln : KheSo lnCopy(soln), diversifier + i);
    MAssert(ps->soln != NULL && KheSolnInstance(ps->soln) != NULL,
      "KheParallelSolve internal error (%d)", i);
    MArrayAddLast(parallel_solns, ps);
  }

  ** start the solve threads on solver **
  MArrayForEach(parallel_solns, &ps, &i)
    KheParallelSolnStart(ps, solver, options);

  ** stop the solve threads (or rather, wait for them to end) **
  MArrayForEach(parallel_solns, &ps, &i)
    KheParallelSolnStop(ps);

  ** find the best solution **
  best_ps = NULL;
  best_cost = KheCostMax;
  MArrayForEach(parallel_solns, &ps, &i)
  {
    if( DEBUG1 )
      fprintf(stderr, "  soln %d has cost %.5f%s\n",
	i, KheCostShow(KheSolnCost(ps->soln)),
        KheSolnCost(ps->soln) < best_cost ? " (new best)" : "");
    if( KheSolnCost(ps->soln) < best_cost )
    {
      best_ps = ps;
      best_cost = KheSolnCost(ps->soln);
    }
  }

  ** print a histogram of solutions **
  if( DEBUG1 )
  {
    KheParallelDebugHistogram(&parallel_solns, 2, 4, stderr);
    fprintf(stderr, "\n");
    KheParallelDebugAverages(&parallel_solns, 2, 4, stderr);
    fprintf(stderr, "\n");
  }

  ** free everything except the best solution **
  MAssert(best_ps != NULL, "KheParallelSolve internal error");
  res = best_ps->soln;
  best_ps->soln = NULL;
  while( MArraySize(parallel_solns) > 0 )
    KheParallelSolnDelete(MArrayRemoveLast(parallel_solns));
  MArrayFree(parallel_solns);

  ** return the best soln **
  if( DEBUG1 )
  {
    KheSolnDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheParallelSolve returning (res has cost %.5f)\n",
      KheCostShow(KheSolnCost(res)));
  }
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Sobmodule "solving archive in parallel"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  PINST - an instance to be solved in parallel                             */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_SOLN) ARRAY_KHE_SOLN;
typedef MARRAY(KHE_COST) ARRAY_KHE_COST;

typedef struct pinst_rec {
  KHE_INSTANCE			instance;
  ARRAY_KHE_SOLN		best_solns;
  int				tried_solns;
  ARRAY_KHE_COST		all_costs;
} *PINST;

typedef MARRAY(PINST) ARRAY_PINST;
#if KHE_USE_PTHREAD
typedef MARRAY(pthread_t) ARRAY_PTHREAD;
#endif


/*****************************************************************************/
/*                                                                           */
/*  PSOLVE - a parallel solve                                                */
/*                                                                           */
/*****************************************************************************/

typedef struct psolve_rec {
#if KHE_USE_PTHREAD
  pthread_mutex_t	mutex;			/* locks entire record       */
  pthread_t		leader_thread;		/* leader thread             */
  ARRAY_PTHREAD		other_threads;		/* other threads             */
#endif
  KHE_ARCHIVE		archive;		/* archive parameter         */
  int			thread_count;		/* thread count parameter    */
  int			make_solns;		/* make_solns parameter      */
  int			keep_solns;		/* keep_solns parameter      */
  KHE_GENERAL_SOLVER	solver;			/* solver parameter          */
  KHE_OPTIONS		options;		/* solver options            */
  int			curr_instance;		/* currently on this         */
  int			curr_soln;		/* currently on this         */
  ARRAY_PINST		instances;		/* instances being solved    */
} *PSOLVE;


/*****************************************************************************/
/*                                                                           */
/*  PINST PInstMake(KHE_INSTANCE ins)                                        */
/*                                                                           */
/*  Make a new pinst object for ins.                                         */
/*                                                                           */
/*****************************************************************************/

static PINST PInstMake(KHE_INSTANCE ins)
{
  PINST res;
  MMake(res);
  res->instance = ins;
  MArrayInit(res->best_solns);
  res->tried_solns = 0;
  MArrayInit(res->all_costs);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void PInstFree(PINST pinst)                                              */
/*                                                                           */
/*  Free pinst.                                                              */
/*                                                                           */
/*****************************************************************************/

static void PInstFree(PINST pinst)
{
  MArrayFree(pinst->best_solns);
  MFree(pinst);
}


/*****************************************************************************/
/*                                                                           */
/*  PSOLVE PSolveMake(KHE_ARCHIVE archive, KHE_INSTANCE ins,                 */
/*    int thread_count, int make_solns, int keep_solns,                      */
/*    KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)                        */
/*                                                                           */
/*  Make a new psolve object with these attributes.                          */
/*                                                                           */
/*****************************************************************************/

static PSOLVE PSolveMake(KHE_ARCHIVE archive, KHE_INSTANCE ins,
  int thread_count, int make_solns, int keep_solns,
  KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)
{
  PSOLVE res;  int i;
  MMake(res);
#if KHE_USE_PTHREAD
  pthread_mutex_init(&res->mutex, NULL);
  res->leader_thread = pthread_self();
  MArrayInit(res->other_threads);
#endif
  res->archive = archive;
  res->thread_count = thread_count;
  res->make_solns = make_solns;
  res->keep_solns = keep_solns;
  res->solver = solver;
  res->options = options;
  /*  res->options = (options == NULL ? NULL : KheOptionsCopy(options)); */
  res->curr_instance = 0;
  MArrayInit(res->instances);
  if( archive != NULL )
    for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
    {
      ins = KheArchiveInstance(archive, i);
      MArrayAddLast(res->instances, PInstMake(ins));
    }
  else
    MArrayAddLast(res->instances, PInstMake(ins));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void PSolveFree(PSOLVE psolve)                                           */
/*                                                                           */
/*  Free psolve.                                                             */
/*                                                                           */
/*****************************************************************************/

static void PSolveFree(PSOLVE psolve)
{
  while( MArraySize(psolve->instances) > 0 )
    PInstFree(MArrayRemoveLast(psolve->instances));
  MFree(psolve);
}


/*****************************************************************************/
/*                                                                           */
/*  bool PSolveMoreToDo(PSOLVE psolve, PINST *res, int *diversifier)         */
/*                                                                           */
/*  If there is still work to do, set *res to the pinst needing solving      */
/*  and *diversifier to a suitable diversifier and return true.  Otherwise   */
/*  return false.  Also update psolve.                                       */
/*                                                                           */
/*****************************************************************************/

static bool PSolveMoreToDo(PSOLVE psolve, PINST *res, int *diversifier)
{
  PINST pinst;
#if KHE_USE_PTHREAD
  pthread_mutex_lock(&psolve->mutex);
#endif
  while( psolve->curr_instance < MArraySize(psolve->instances) )
  {
    pinst = MArrayGet(psolve->instances, psolve->curr_instance);
    if( pinst->tried_solns < psolve->make_solns )
    {
      *res = pinst;
      *diversifier = pinst->tried_solns;
      pinst->tried_solns++;
#if KHE_USE_PTHREAD
      pthread_mutex_unlock(&psolve->mutex);
#endif
      if( DEBUG3 )
	fprintf(stderr, "  PSolveMoreToDo returning true (%s %d)\n",
	  KheInstanceId(pinst->instance), pinst->tried_solns);
      return true;
    }
    psolve->curr_instance++;
  }
#if KHE_USE_PTHREAD
  pthread_mutex_unlock(&psolve->mutex);
#endif
  if( DEBUG3 )
    fprintf(stderr, "  PSolveMoreToDo returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void PSolveAddSoln(PSOLVE psolve, PINST pinst, KHE_SOLN soln)            */
/*                                                                           */
/*  Add soln to pinst of psolve, and possibly remove and delete the worst.   */
/*                                                                           */
/*****************************************************************************/

static void PSolveAddSoln(PSOLVE psolve, PINST pinst, KHE_SOLN soln)
{
  int i;  KHE_SOLN best_soln;
#if KHE_USE_PTHREAD
  pthread_mutex_lock(&psolve->mutex);
#endif
  MArrayAddLast(pinst->all_costs, KheSolnCost(soln));
  for( i = 0;  i < MArraySize(pinst->best_solns);  i++ )
  {
    best_soln = MArrayGet(pinst->best_solns, i);
    if( KheSolnCost(soln) < KheSolnCost(best_soln) )
    {
      MArrayInsert(pinst->best_solns, i, soln);
      break;
    }
  }
  if( i >= MArraySize(pinst->best_solns) )
    MArrayAddLast(pinst->best_solns, soln);
  if( MArraySize(pinst->best_solns) > psolve->keep_solns )
    KheSolnDelete(MArrayRemoveLast(pinst->best_solns));
#if KHE_USE_PTHREAD
    pthread_mutex_unlock(&psolve->mutex);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void *PSolveRun(void *arg)                                               */
/*                                                                           */
/*  Thread execution function.                                               */
/*                                                                           */
/*****************************************************************************/

static void *PSolveRun(void *arg)
{
  PSOLVE psolve;  PINST pinst;  KHE_SOLN soln;  int diversifier;
  psolve = (PSOLVE) arg;
  while( PSolveMoreToDo(psolve, &pinst, &diversifier) )
  {
    soln = KheSolnMake(pinst->instance);
    KheSolnSetDiversifier(soln, diversifier);
    soln = psolve->solver(soln, psolve->options == NULL ? NULL :
      KheOptionsCopy(psolve->options));
    PSolveAddSoln(psolve, pinst, soln);
  }
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheCostIncreasingCmp(const void *t1, const void *t2)                 */
/*                                                                           */
/*  Comparison function for sorting an array of costs into increasing order. */ 
/*                                                                           */
/*****************************************************************************/

static int KheCostIncreasingCmp(const void *t1, const void *t2)
{
  KHE_COST cost1 = * (KHE_COST *) t1;
  KHE_COST cost2 = * (KHE_COST *) t2;
  return KheCostCmp(cost1, cost2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveParallelSolve(KHE_ARCHIVE archive, int thread_count,      */
/*    int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options,        */
/*    int keep_solns, KHE_SOLN_GROUP soln_group)                             */
/*                                                                           */
/*  Solve the instances of archive in parallel, using a pool of thread_count */
/*  threads, including the thread that called KheArchiveParallelSolve.       */
/*                                                                           */
/*  If soln_group is non-NULL, keep the best keep_solns out of the           */
/*  make_solns diversified solutions made for each instance, and add them    */
/*  to soln_group, deleting the others.  Otherwise deletes all solutions.    */
/*                                                                           */
/*****************************************************************************/

void KheArchiveParallelSolve(KHE_ARCHIVE archive, int thread_count,
  int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options,
  int keep_solns, KHE_SOLN_GROUP soln_group)
{
  PSOLVE psolve;  int i, j;  PINST pinst;  KHE_SOLN soln;  KHE_COST cost;
  KHE_STATS_TIMER st;  float secs;
#if KHE_USE_PTHREAD
  pthread_t thread;
#endif
  MAssert(thread_count >= 1, "KheArchiveParallelSolve: thread_count < 1");
  MAssert(make_solns >= keep_solns,
    "KheArchiveParallelSolve: make_solns < keep_solns");
  if( DEBUG2 )
    fprintf(stderr, "[ KheArchiveParallelSolve(threads %d, make %d, keep %d)\n",
      thread_count, make_solns, keep_solns);
  st = KheStatsTimerMake();

  /* create a psolve object */
  psolve = PSolveMake(archive, NULL, thread_count, make_solns, keep_solns,
    solver, options);

  /* start the other threads */
#if KHE_USE_PTHREAD
  pthread_mutex_lock(&psolve->mutex);
  for( i = 1;  i < thread_count;  i++ )
  {
    pthread_create(&thread, NULL, &PSolveRun, psolve);
    MArrayAddLast(psolve->other_threads, thread);
  }
  pthread_mutex_unlock(&psolve->mutex);
#endif

  /* run this thread */
  PSolveRun((void *) psolve);

  /* wait for the other threads to terminate */
#if KHE_USE_PTHREAD
  MArrayForEach(psolve->other_threads, &thread, &i)
    pthread_join(thread, NULL);
#endif

  /* add the surviving solutions to soln_group, if any */
  if( soln_group != NULL )
    MArrayForEach(psolve->instances, &pinst, &i)
      MArrayForEach(pinst->best_solns, &soln, &j)
	KheSolnGroupAddSoln(soln_group, soln);
  if( DEBUG2 )
  {
    MArrayForEach(psolve->instances, &pinst, &i)
    {
      if( MArraySize(pinst->all_costs) == 1 )
	fprintf(stderr, "  sole soln of \"%s\" has cost %.5f\n",
	  KheInstanceId(pinst->instance),
	  KheCostShow(MArrayFirst(pinst->all_costs)));
      else
      {
	fprintf(stderr, "  all solns of \"%s\":",
	  KheInstanceId(pinst->instance));
	MArrayForEach(pinst->all_costs, &cost, &j)
	  fprintf(stderr, "%s%.5f", j % 8 == 0 ? "\n  " : " ",
	    KheCostShow(cost));
	fprintf(stderr, "\n");
	MArraySortUnique(pinst->all_costs, &KheCostIncreasingCmp);
	fprintf(stderr, "  %d unique costs of \"%s\":",
	  MArraySize(pinst->all_costs), KheInstanceId(pinst->instance));
	MArrayForEach(pinst->all_costs, &cost, &j)
	  fprintf(stderr, "%s%.5f", j % 8 == 0 ? "\n  " : " ",
	    KheCostShow(cost));
	fprintf(stderr, "\n");
      }
    }
    if( soln_group != NULL )
    {
      fprintf(stderr, "  best solutions:\n");
      for( i = 0;  i < KheSolnGroupSolnCount(soln_group);  i++ )
      {
	soln = KheSolnGroupSoln(soln_group, i);
	KheSolnDebug(soln, 2, 2, stderr);
      }
      /* ***
      for( i = 0;  i < KheSolnGroupSolnCount(soln_group);  i++ )
      {
	soln = KheSolnGroupSoln(soln_group, i);
	fprintf(stderr, "%s%.5f", i == 0 ? "(" : ", ",
	  KheCostShow(KheSolnCost(soln)));
      }
      fprintf(stderr, ")\n");
      *** */
    }
    secs = KheStatsTimerNow(st);
    if( secs > 300.0 )
      fprintf(stderr,
	"] KheArchiveParallelSolve returning (%.2f mins elapsed)\n",
	secs / 60.0);
    else
      fprintf(stderr,
	"] KheArchiveParallelSolve returning (%.2f secs elapsed)\n", secs);
  }
  KheStatsTimerDelete(st);
  PSolveFree(psolve);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheInstanceParallelSolve(KHE_INSTANCE ins, int thread_count,    */
/*    int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)        */
/*                                                                           */
/*  Like KheArchiveParallelSolve except that it solves only one instance     */
/*  and saves (and returns) any one best solution.                           */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheInstanceParallelSolve(KHE_INSTANCE ins, int thread_count,
  int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options)
{
  PSOLVE psolve;  int i;  PINST pinst;  KHE_SOLN soln;
#if KHE_USE_PTHREAD
  pthread_t thread;
#endif
  MAssert(thread_count >= 1, "KheInstanceParallelSolve: thread_count < 1");
  MAssert(make_solns >= 1, "KheArchiveParallelSolve: make_solns < 1");
  if( DEBUG2 )
    fprintf(stderr, "[ KheInstanceParallelSolve(threads %d, make %d)\n",
      thread_count, make_solns);

  /* create a psolve object */
  psolve = PSolveMake(NULL, ins, thread_count, make_solns, 1, solver, options);

  /* start the other threads */
#if KHE_USE_PTHREAD
  pthread_mutex_lock(&psolve->mutex);
  for( i = 1;  i < thread_count;  i++ )
  {
    pthread_create(&thread, NULL, &PSolveRun, psolve);
    MArrayAddLast(psolve->other_threads, thread);
  }
  pthread_mutex_unlock(&psolve->mutex);
#endif

  /* run this thread */
  PSolveRun((void *) psolve);

  /* wait for the other threads to terminate */
#if KHE_USE_PTHREAD
  MArrayForEach(psolve->other_threads, &thread, &i)
    pthread_join(thread, NULL);
#endif

  /* extract the sole surviving solution */
  MAssert(MArraySize(psolve->instances) == 1,
    "KheInstanceParallelSolve internal error 1");
  pinst = MArrayFirst(psolve->instances);
  MAssert(MArraySize(pinst->best_solns) == 1,
    "KheInstanceParallelSolve internal error 2");
  soln = MArrayFirst(pinst->best_solns);
  PSolveFree(psolve);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final solution:\n");
    KheSolnDebug(soln, 2, 2, stderr);
    fprintf(stderr, "] KheInstanceParallelSolve returning\n");
  }
  return soln;
}
