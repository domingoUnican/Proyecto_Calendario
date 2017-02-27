
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
/*  FILE:         khe_sm_ejector.c                                           */
/*  DESCRIPTION:  Ejector objects                                            */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define MAX_GROUP_AUGMENT 30
#define MAX_REPAIRS_SAVE 30
#define MAX_REPAIRS_TRY 20
#define MAX_AUGMENT_COUNT 120

#define DEBUG1 0	/* KheEjectorMainAugment */
#define DEBUG2 0	/* phases and new successes */
#define DEBUG3 0	/* various */
#define DEBUG4 0	/* KheEjectorAugment */
#define DEBUG5 0	/* KheEjectorSolveEnd */
#define DEBUG6 0	/* KheAugmentHandleRepair */
#define DEBUG7 0	/* histograms */
#define DEBUG8 0	/* drops */

#define KHE_EJECTOR_WITH_STATS 1


/*****************************************************************************/
/*                                                                           */
/*  Type declarations (construction)                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MINOR_SCHEDULE                                               */
/*                                                                           */
/*****************************************************************************/

struct khe_ejector_minor_schedule_rec {
  /* KHE_EJECTOR_SOLVE_TYPE	solve_type; */	/* solve type                */
  int				max_depth;	/* max depth                 */
  /* int			max_d isruption; */ /* max d isruption       */
  bool				may_revisit;	/* allow revisiting          */
};

typedef MARRAY(KHE_EJECTOR_MINOR_SCHEDULE) ARRAY_KHE_EJECTOR_MINOR_SCHEDULE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MAJOR_SCHEDULE                                               */
/*                                                                           */
/*****************************************************************************/

struct khe_ejector_major_schedule_rec {
  ARRAY_KHE_EJECTOR_MINOR_SCHEDULE	minor_schedules;
};

typedef MARRAY(KHE_EJECTOR_MAJOR_SCHEDULE) ARRAY_KHE_EJECTOR_MAJOR_SCHEDULE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_AUGMENT_FN - one augment function, stored with its type              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_augment_fn_rec {
  KHE_EJECTOR_AUGMENT_FN	fn;
  int				type;
} KHE_AUGMENT_FN;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REPAIR_INFO - constant information about one repair type             */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ejector_repair_info_rec {
  int				repair_type;		/* integer value     */
  char				*repair_label;		/* label             */
} *KHE_REPAIR_INFO;

typedef MARRAY(KHE_REPAIR_INFO) ARRAY_KHE_REPAIR_INFO;


/*****************************************************************************/
/*                                                                           */
/*  KHE_AUGMENT_INFO - constant information about one augment type           */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ejector_augment_info_rec {
  int				augment_type;		/* integer value     */
  char				*augment_label;		/* label             */
} *KHE_AUGMENT_INFO;

typedef MARRAY(KHE_AUGMENT_INFO) ARRAY_KHE_AUGMENT_INFO;


/*****************************************************************************/
/*                                                                           */
/*  Type declarations (statistics)                                           */
/*                                                                           */
/*****************************************************************************/

#if KHE_EJECTOR_WITH_STATS
/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_IMPROVEMENT_STATS - statistics about one improvement         */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ejector_improvement_stats_rec {
  int				repair_count;		/* no. of repairs    */
  float				time;			/* time to here      */
  KHE_COST			cost;			/* cost after        */
  int				defects;		/* defects           */
} KHE_EJECTOR_IMPROVEMENT_STATS;

typedef MARRAY(KHE_EJECTOR_IMPROVEMENT_STATS)
  ARRAY_KHE_EJECTOR_IMPROVEMENT_STATS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_REPAIR_STATS - statistics about repairs of one augment_type          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_pair_rec {
  int				total;
  int				successful;
} KHE_PAIR;

typedef MARRAY(KHE_PAIR) ARRAY_KHE_PAIR;

typedef struct khe_ejector_repair_stats_rec {
  KHE_PAIR			overall;
  ARRAY_KHE_PAIR		by_type;
} KHE_REPAIR_STATS;

typedef MARRAY(KHE_REPAIR_STATS) ARRAY_KHE_REPAIR_STATS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_STATS - statistics about the ejector generally               */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ejector_stats_rec {

  /* statistics about the current call to KheEjectorSolve */
  ARRAY_KHE_EJECTOR_IMPROVEMENT_STATS	improvement_stats;
  KHE_STATS_TIMER		timer;
  KHE_COST			init_cost;
  int				init_defects;

  /* statistics about the full history */
  ARRAY_INT			repair_count_histo;
  ARRAY_INT			augment_count_histo;
  ARRAY_KHE_REPAIR_STATS	repair_stats;
} KHE_EJECTOR_STATS;
#endif


/*****************************************************************************/
/*                                                                           */
/*  Type declarations (solving)                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SAVED_REPAIR - one repair operation, only when it needs to be saved  */
/*                                                                           */
/*****************************************************************************/
typedef struct khe_augment_rec *KHE_AUGMENT;

typedef struct khe_saved_repair_rec {
  KHE_AUGMENT		augment;		/* enclosing augment         */
  int			repair_type;		/* repair's repair type      */
  int			max_sub_chains;		/* max number of sub-chains  */
  KHE_PATH		path;			/* holds the repair          */
  void (*on_success_fn)(void *on_success_val);	/* on_success function       */
  void			*on_success_val;	/* on_success value          */
} *KHE_SAVED_REPAIR;

typedef MARRAY(KHE_SAVED_REPAIR) ARRAY_KHE_SAVED_REPAIR;


/*****************************************************************************/
/*                                                                           */
/*   KHE_AUGMENT - one call on an augment function                           */
/*                                                                           */
/*****************************************************************************/
typedef MARRAY(KHE_MONITOR) ARRAY_KHE_MONITOR;

struct khe_augment_rec {
  KHE_EJECTOR		ejector;		/* enclosing ejector         */
  KHE_MARK		mark;			/* marks soln at start       */
  KHE_COST		target_cost;		/* augment aims to beat this */
  ARRAY_KHE_SAVED_REPAIR saved_repairs;		/* saved repairs             */
  KHE_TRACE		trace;			/* for tracing each repair   */
  ARRAY_KHE_MONITOR	open_monitors;		/* open monitors in trace    */
  int			augment_type;		/* augment type              */
  bool			test_limits;		/* inlude limits in success  */
  bool			success;		/* if successful repair      */
  int			repair_count;		/* no of repairs, if success */
};

typedef MARRAY(KHE_AUGMENT) ARRAY_KHE_AUGMENT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_STATE - the state of an ejector                              */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_EJECTOR_MAKE_SETTING,		/* setting up to make                */
  KHE_EJECTOR_SOLVE_IDLE,		/* idle, ready to solve              */
  KHE_EJECTOR_SOLVE_SETTING,		/* setting up to solve               */
  KHE_EJECTOR_SOLVE_RUN,		/* running                           */
  KHE_EJECTOR_SOLVE_RUN_REPAIR,		/* running, expecting RepairEnd      */
} KHE_EJECTOR_STATE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST_LIMIT - a limit on the cost of a given monitor                  */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_cost_limit_rec {
  KHE_MONITOR			monitor;		/* the monitor       */
  KHE_COST			cost;			/* its cost limit    */
  bool				reducing;		/* reducing limit    */
} KHE_COST_LIMIT;

typedef MARRAY(KHE_COST_LIMIT) ARRAY_KHE_COST_LIMIT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR                                                              */
/*                                                                           */
/*  Note on depth.  The depth of an augment is the number of calls on        */
/*  the chain it ends, including itself.  So the depth of an augment         */
/*  called immediately from the main loop is 1, the depth of an augment      */
/*  below it is 2, and so on.                                                */
/*                                                                           */
/*****************************************************************************/

struct khe_ejector_rec
{
  /* defined after setting up the ejector */
  KHE_EJECTOR_STATE		state;			/* ejector state     */
  ARRAY_KHE_AUGMENT_INFO	augment_info_array;	/* augment info      */
  ARRAY_KHE_REPAIR_INFO		repair_info_array;	/* repair info       */
  ARRAY_KHE_EJECTOR_MAJOR_SCHEDULE major_schedules;	/* major schedules   */
  KHE_EJECTOR_MAJOR_SCHEDULE	new_major_schedule;	/* when constructing */
  KHE_AUGMENT_FN		nongroup_augment[KHE_MONITOR_TAG_COUNT];
  KHE_AUGMENT_FN		group_augment[MAX_GROUP_AUGMENT];
  ARRAY_KHE_AUGMENT		free_augments;		/* free list         */
  ARRAY_KHE_SAVED_REPAIR	free_repairs;		/* free list         */
#if KHE_EJECTOR_WITH_STATS
  KHE_EJECTOR_STATS		stats;			/* statistics        */
#endif

  /* defined after setting up for solving */
  KHE_GROUP_MONITOR		start_gm;		/* main loop gm      */
  KHE_GROUP_MONITOR		continue_gm;		/* augment fn        */
  KHE_OPTIONS			options;		/* solve options     */
  ARRAY_KHE_COST_LIMIT		cost_limits;		/* cost limits       */
  ARRAY_KHE_AUGMENT		augment_stack;		/* augment stack     */
  int				augment_count;		/* no. of augments   */

  /* defined only temporarily during solving (which is what "curr_" means) */
  int				curr_augment_count;	/* current augments  */
  int				curr_successful_visit;	/* visit num         */
  ARRAY_KHE_MONITOR		curr_defects;		/* current defects   */
  /* KHE_COST			curr_target_cost; */	/* current target    */
  KHE_EJECTOR_MAJOR_SCHEDULE	curr_major_schedule;	/* current schedule  */
  KHE_EJECTOR_MINOR_SCHEDULE	curr_minor_schedule;	/* current schedule  */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector construction (general)"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR KheEjectorMakeBegin(void)                                    */
/*                                                                           */
/*  Start off the construction of a new ejector.                             */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorInitAllStats(KHE_EJECTOR ej);

KHE_EJECTOR KheEjectorMakeBegin(void)
{
  KHE_EJECTOR res;  int i;

  /* defined after setting up the ejector */
  MMake(res);
  res->state = KHE_EJECTOR_MAKE_SETTING;
  MArrayInit(res->augment_info_array);
  MArrayInit(res->repair_info_array);
  MArrayInit(res->major_schedules);
  res->new_major_schedule = NULL;
  for( i = 0;  i < KHE_MONITOR_TAG_COUNT;  i++ )
  {
    res->nongroup_augment[i].fn = NULL;
    res->nongroup_augment[i].type = 0;
  }
  for( i = 0;  i < MAX_GROUP_AUGMENT;  i++ )
  {
    res->group_augment[i].fn = NULL;
    res->group_augment[i].type = 0;
  }
  MArrayInit(res->free_augments);
  MArrayInit(res->free_repairs);
  KheEjectorInitAllStats(res);

  /* defined after setting up for solving */
  res->start_gm = NULL;
  res->continue_gm = NULL;
  res->options = NULL;
  MArrayInit(res->cost_limits);
  MArrayInit(res->augment_stack);
  res->augment_count = 0;

  /* defined only temporarily during solving */
  res->curr_augment_count = 0;
  res->curr_successful_visit = 0;
  MArrayInit(res->curr_defects);
  /* res->curr_target_cost = -1; */
  res->curr_major_schedule = NULL;
  res->curr_minor_schedule = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMakeEnd(KHE_EJECTOR ej)                                   */
/*                                                                           */
/*  Finish off the construction of the new ejector.                          */
/*                                                                           */
/*****************************************************************************/

void KheEjectorMakeEnd(KHE_EJECTOR ej)
{
  MAssert(ej->state == KHE_EJECTOR_MAKE_SETTING,
    "KheEjectorMakeEnd called out of order");
  ej->state = KHE_EJECTOR_SOLVE_IDLE;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR KheEjectorCopy(KHE_EJECTOR ej)                               */
/*                                                                           */
/*  Make a copy of ej.                                                       */
/*                                                                           */
/*****************************************************************************/
static KHE_EJECTOR_MAJOR_SCHEDULE KheMajorScheduleCopy(
  KHE_EJECTOR_MAJOR_SCHEDULE mas);

KHE_EJECTOR KheEjectorCopy(KHE_EJECTOR ej)
{
  KHE_EJECTOR res;  int i;  KHE_EJECTOR_MAJOR_SCHEDULE mas;
  res = KheEjectorMakeBegin();
  MArrayForEach(ej->major_schedules, &mas, &i)
    MArrayAddLast(res->major_schedules, KheMajorScheduleCopy(mas));
  for( i = 0;  i < KHE_MONITOR_TAG_COUNT;  i++ )
    res->nongroup_augment[i] = ej->nongroup_augment[i];
  for( i = 0;  i < MAX_GROUP_AUGMENT;  i++ )
    res->group_augment[i] = ej->group_augment[i];
  KheEjectorMakeEnd(ej);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorDelete(KHE_EJECTOR ej)                                    */
/*                                                                           */
/*  Delete ej.                                                               */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorMajorScheduleDelete(KHE_EJECTOR_MAJOR_SCHEDULE ejs);
static void KheAugmentInfoDelete(KHE_AUGMENT_INFO ai);
static void KheRepairInfoDelete(KHE_REPAIR_INFO ri);
static void KheEjectorStatsDelete(KHE_EJECTOR ej);
static void KheAugmentFree(KHE_AUGMENT ea);
static void KheSavedRepairFree(KHE_SAVED_REPAIR er);

void KheEjectorDelete(KHE_EJECTOR ej)
{
  /* defined after setting up the ejector */
  while( MArraySize(ej->augment_info_array) > 0 )
    KheAugmentInfoDelete(MArrayRemoveLast(ej->augment_info_array));
  MArrayFree(ej->augment_info_array);
  while( MArraySize(ej->repair_info_array) > 0 )
    KheRepairInfoDelete(MArrayRemoveLast(ej->repair_info_array));
  MArrayFree(ej->repair_info_array);
  while( MArraySize(ej->major_schedules) > 0 )
    KheEjectorMajorScheduleDelete(MArrayRemoveLast(ej->major_schedules));
  MArrayFree(ej->major_schedules);
  while( MArraySize(ej->free_augments) > 0 )
    KheAugmentFree(MArrayRemoveLast(ej->free_augments));
  MArrayFree(ej->free_augments);
  while( MArraySize(ej->free_repairs) > 0 )
    KheSavedRepairFree(MArrayRemoveLast(ej->free_repairs));
  MArrayFree(ej->free_repairs);
  KheEjectorStatsDelete(ej);

  /* defined after setting up for solving */
  MArrayFree(ej->cost_limits);
  MAssert(MArraySize(ej->augment_stack) == 0,
    "KheEjectorDelete called during solving");
  MArrayFree(ej->augment_stack);

  /* defined only temporarily during solving */
  MArrayFree(ej->curr_defects);
  MFree(ej);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector construction (major schedules)"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorMajorScheduleMake(void)             */
/*                                                                           */
/*  Make a major schedule with this solve type and no minor schedules to     */
/*  begin with.                                                              */
/*                                                                           */
/*****************************************************************************/

static KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorMajorScheduleMake(void)
{
  KHE_EJECTOR_MAJOR_SCHEDULE res;
  MMake(res);
  MArrayInit(res->minor_schedules);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMajorScheduleDelete(KHE_EJECTOR_MAJOR_SCHEDULE ejs)       */
/*                                                                           */
/*  Free major schedule ejms.                                                */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorMinorScheduleDelete(KHE_EJECTOR_MINOR_SCHEDULE ejms);

static void KheEjectorMajorScheduleDelete(KHE_EJECTOR_MAJOR_SCHEDULE ejs)
{
  while( MArraySize(ejs->minor_schedules) > 0 )
    KheEjectorMinorScheduleDelete(MArrayRemoveLast(ejs->minor_schedules));
  MArrayFree(ejs->minor_schedules);
  MFree(ejs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MAJOR_SCHEDULE KheMajorScheduleCopy(                         */
/*    KHE_EJECTOR_MAJOR_SCHEDULE mas)                                        */
/*                                                                           */
/*  Return a fresh copy of mas.                                              */
/*                                                                           */
/*****************************************************************************/
static KHE_EJECTOR_MINOR_SCHEDULE KheMinorScheduleCopy(
  KHE_EJECTOR_MINOR_SCHEDULE mis);

static KHE_EJECTOR_MAJOR_SCHEDULE KheMajorScheduleCopy(
  KHE_EJECTOR_MAJOR_SCHEDULE mas)
{
  KHE_EJECTOR_MAJOR_SCHEDULE res;  KHE_EJECTOR_MINOR_SCHEDULE mis;  int i;
  res = KheEjectorMajorScheduleMake();
  MArrayForEach(mas->minor_schedules, &mis, &i)
    MArrayAddLast(res->minor_schedules, KheMinorScheduleCopy(mis));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMajorScheduleCount(KHE_EJECTOR ej)                         */
/*                                                                           */
/*  Return the number of major schedules of ej.                              */
/*                                                                           */
/*****************************************************************************/

int KheEjectorMajorScheduleCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->major_schedules);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorMajorSchedule(KHE_EJECTOR ej, int i)*/
/*                                                                           */
/*  Return the i'th major schedule of ej.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorMajorSchedule(KHE_EJECTOR ej, int i)
{
  return MArrayGet(ej->major_schedules, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMajorScheduleBegin(KHE_EJECTOR ej)                        */
/*                                                                           */
/*  Begin the construction of a major schedule.                              */
/*                                                                           */
/*****************************************************************************/

void KheEjectorMajorScheduleBegin(KHE_EJECTOR ej)
{
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule==NULL,
    "KheEjectorMajorScheduleBegin called out of order");
  ej->new_major_schedule = KheEjectorMajorScheduleMake();
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMajorScheduleEnd(KHE_EJECTOR ej)                          */
/*                                                                           */
/*  End the construction of a major schedule.                                */
/*                                                                           */
/*****************************************************************************/

void KheEjectorMajorScheduleEnd(KHE_EJECTOR ej)
{
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule!=NULL,
    "KheEjectorMajorScheduleEnd called out of order");
  MArrayAddLast(ej->major_schedules, ej->new_major_schedule);
  ej->new_major_schedule = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMajorScheduleMinorScheduleCount(                           */
/*    KHE_EJECTOR_MAJOR_SCHEDULE ejm)                                        */
/*                                                                           */
/*  Return the number of minor schedules in ejm.                             */
/*                                                                           */
/*****************************************************************************/

int KheEjectorMajorScheduleMinorScheduleCount(KHE_EJECTOR_MAJOR_SCHEDULE ejm)
{
  return MArraySize(ejm->minor_schedules);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MINOR_SCHEDULE KheEjectorMajorScheduleMinorSchedule(         */
/*    KHE_EJECTOR_MAJOR_SCHEDULE ejm, int i)                                 */
/*                                                                           */
/*  Return the i'th minor schedule of ejm.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_MINOR_SCHEDULE KheEjectorMajorScheduleMinorSchedule(
  KHE_EJECTOR_MAJOR_SCHEDULE ejm, int i)
{
  return MArrayGet(ejm->minor_schedules, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector construction (minor schedules)"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MINOR_SCHEDULE KheEjectorMinorScheduleMake(                  */
/*    int max_depth, bool may_revisit)                                       */
/*                                                                           */
/*  Make a new ejector schedule with these attributes.                       */
/*                                                                           */
/*****************************************************************************/

static KHE_EJECTOR_MINOR_SCHEDULE KheEjectorMinorScheduleMake(
  int max_depth, bool may_revisit)
{
  KHE_EJECTOR_MINOR_SCHEDULE res;
  MAssert(max_depth > 0,
    "KheEjectorMinorScheduleMake: max_depth (%d) out of range", max_depth);
  /* ***
  MAssert(max_d isruption >= 0,
    "KheEjectorMinorScheduleMake: max_d isruption (%d) out of range",
    max_d isruption);
  *** */
  MMake(res);
  /* res->solve_type = solve_type; */
  res->max_depth = max_depth;
  /* res->max_d isruption = max_d isruption; */
  res->may_revisit = may_revisit;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMinorScheduleDelete(KHE_EJECTOR_MINOR_SCHEDULE ejms)      */
/*                                                                           */
/*  Free minor schedule ejms.                                                */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorMinorScheduleDelete(KHE_EJECTOR_MINOR_SCHEDULE ejms)
{
  MFree(ejms);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MINOR_SCHEDULE KheMinorScheduleCopy(                         */
/*    KHE_EJECTOR_MINOR_SCHEDULE mis)                                        */
/*                                                                           */
/*  Return a fresh copy of mis.                                              */
/*                                                                           */
/*****************************************************************************/

static KHE_EJECTOR_MINOR_SCHEDULE KheMinorScheduleCopy(
  KHE_EJECTOR_MINOR_SCHEDULE mis)
{
  return KheEjectorMinorScheduleMake(mis->max_depth, mis->may_revisit);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMinorScheduleAdd(KHE_EJECTOR ej,                          */
/*    int max_depth, bool may_revisit)                                       */
/*                                                                           */
/*  Add a minor schedule with these attributes to the current major schedule.*/
/*                                                                           */
/*****************************************************************************/

void KheEjectorMinorScheduleAdd(KHE_EJECTOR ej,
  int max_depth, bool may_revisit)
{
  KHE_EJECTOR_MINOR_SCHEDULE ms;
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule!=NULL,
    "KheEjectorMinorScheduleAdd called out of order");
  ms = KheEjectorMinorScheduleMake(max_depth, may_revisit);
  MArrayAddLast(ej->new_major_schedule->minor_schedules, ms);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SOLVE_TYPE KheEjectorMinorScheduleSolveType(                 */
/*    KHE_EJECTOR_MINOR_SCHEDULE ejms)                                       */
/*                                                                           */
/*  Return the solve type of ejms.                                           */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_EJECTOR_SOLVE_TYPE KheEjectorMinorScheduleSolveType(
  KHE_EJECTOR_MINOR_SCHEDULE ejms)
{
  return ejms->solve_type;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMinorScheduleMaxDepth(KHE_EJECTOR_MINOR_SCHEDULE ejms)     */
/*                                                                           */
/*  Return the max_depth attribute of ejms.                                  */
/*                                                                           */
/*****************************************************************************/

int KheEjectorMinorScheduleMaxDepth(KHE_EJECTOR_MINOR_SCHEDULE ejms)
{
  return ejms->max_depth;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMinorScheduleMaxDisruption(KHE_EJECTOR_SCHEDULE ejms)      */
/*                                                                           */
/*  Return the max_d isruption attribute of ejms.                            */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheEjectorMinorScheduleMaxDisruption(KHE_EJECTOR_MINOR_SCHEDULE ejms)
{
  return ejms->max_d isruption;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorScheduleMayRevisit(KHE_EJECTOR_MINOR_SCHEDULE ejms)       */
/*                                                                           */
/*  Return the may_revisit attribute of ejms.                                */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorScheduleMayRevisit(KHE_EJECTOR_MINOR_SCHEDULE ejms)
{
  return ejms->may_revisit;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector construction (other schedules)"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddDefaultSchedules(KHE_EJECTOR ej)                       */
/*                                                                           */
/*  Add the default schedules to ej.                                         */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddDefaultSchedules(KHE_EJECTOR ej)
{
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule==NULL,
    "KheEjectorAddDefaultSchedules called out of order");
  KheEjectorMajorScheduleBegin(ej);
  KheEjectorMinorScheduleAdd(ej, 1, true);
  KheEjectorMajorScheduleEnd(ej);
  /* ***
  KheEjectorMajorScheduleBegin(ej);
  KheEjectorMinorScheduleAdd(ej, 2, false);
  KheEjectorMajorScheduleEnd(ej);
  KheEjectorMajorScheduleBegin(ej);
  KheEjectorMinorScheduleAdd(ej, 3, true);
  KheEjectorMajorScheduleEnd(ej);
  *** */
  KheEjectorMajorScheduleBegin(ej);
  KheEjectorMinorScheduleAdd(ej, INT_MAX, false);
  KheEjectorMajorScheduleEnd(ej);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEjectorReadMinorSchedule(KHE_EJECTOR ej, char *p)               */
/*                                                                           */
/*  Read one minor schedule, starting at *p, and install it.  Return a       */
/*  pointer to the first unread character.                                   */
/*                                                                           */
/*****************************************************************************/

char *KheEjectorReadMinorSchedule(KHE_EJECTOR ej, char *p)
{
  int max_depth;  bool may_revisit;  /* KHE_EJECTOR_SOLVE_TYPE solve_type; */

  /* read optional solve_type */
  /* *** obsolete
  switch( *p )
  {
    case 'f':
      solve_type = KHE_EJECTOR_FIRST;
      p++;
      break;

    case 'c':
      solve_type = KHE_EJECTOR_BEST_COST;
      p++;
      break;

    case 'd':
      solve_type = KHE_EJECTOR_BEST_DISRUPTION;
      p++;
      break;

    default:

      solve_type = KHE_EJECTOR_FIRST;
      break;
  }
  *** */

  /* read max_depth */
  switch( *p )
  {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':

      max_depth = *p - '0';
      break;

    case 'u':

      max_depth = INT_MAX;
      break;

    default:

      MAssert(false,
	"KheEjectorSetSchedulesFromString: '%c' in minor schedule", *p);
      max_depth = 0;  /* keep compiler happy */
      break;
  }
  p++;

  /* read may_revisit */
  switch( *p )
  {
    case '+':
      may_revisit = true;
      break;

    case '-':
      may_revisit = false;
      break;

    default:
      MAssert(false,
	"KheEjectorSetSchedulesFromString: minor schedule ends with '%c'", *p);
      may_revisit = false;  /* keep compiler happy */
      break;
  }
  p++;

  /* install the minor schedule and return */
  KheEjectorMinorScheduleAdd(ej, max_depth, may_revisit);
  return p;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEjectorReadMajorSchedule(KHE_EJECTOR ej, char *p)               */
/*                                                                           */
/*  Read one major schedule, starting at *p, and install it.  Return a       */
/*  pointer to the first unread character.                                   */
/*                                                                           */
/*****************************************************************************/

char *KheEjectorReadMajorSchedule(KHE_EJECTOR ej, char *p)
{
  KheEjectorMajorScheduleBegin(ej);
  p = KheEjectorReadMinorSchedule(ej, p);
  while( *p != ',' && *p != '\0' )
    p = KheEjectorReadMinorSchedule(ej, p);
  KheEjectorMajorScheduleEnd(ej);
  return p;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorSetSchedulesFromString(KHE_EJECTOR ej,                    */
/*    char *ejector_schedules_string)                                        */
/*                                                                           */
/*  Set ej's schedules using the instructions in ejector_schedules_string.   */
/*                                                                           */
/*****************************************************************************/

void KheEjectorSetSchedulesFromString(KHE_EJECTOR ej,
  char *ejector_schedules_string)
{
  char *p;
  MAssert(ej->state == KHE_EJECTOR_MAKE_SETTING,
    "KheEjectorSetSchedulesFromString called out of order");
  p = ejector_schedules_string;
  p = KheEjectorReadMajorSchedule(ej, p);
  while( *p == ',' )
  {
    p++;
    p = KheEjectorReadMajorSchedule(ej, p);
  }
  MAssert(*p == '\0', "KheEjectorSetSchedulesFromString: trailing character");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector construction (augment functions)"                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddAugment(KHE_EJECTOR ej, KHE_MONITOR_TAG tag,           */
/*    KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type)                   */
/*                                                                           */
/*  Set the augment function for non-group monitors with this tag.           */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddAugment(KHE_EJECTOR ej, KHE_MONITOR_TAG tag,
  KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type)
{
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule==NULL,
    "KheEjectorAddAugment called out of order");
  MAssert(tag >= 0 && tag < KHE_MONITOR_TAG_COUNT,
    "KheEjectorAddAugment: tag (%d) out of range", tag);
  MAssert(tag != KHE_GROUP_MONITOR_TAG,
    "KheEjectorAddAugment: tag is KHE_GROUP_MONITOR_TAG");
  ej->nongroup_augment[tag].fn = augment_fn;
  ej->nongroup_augment[tag].type = augment_type;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddGroupAugment(KHE_EJECTOR ej, int sub_tag,              */
/*    KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type)                   */
/*                                                                           */
/*  Set the augment function for group monitors with this sub-tag.           */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddGroupAugment(KHE_EJECTOR ej, int sub_tag,
  KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type)
{
  MAssert(ej->state==KHE_EJECTOR_MAKE_SETTING && ej->new_major_schedule==NULL,
    "KheEjectorAddGroupAugment called out of order");
  MAssert(sub_tag >= 0 && sub_tag < MAX_GROUP_AUGMENT,
    "KheEjectorAddGroupAugment: sub_tag (%d) out of range", sub_tag);
  ej->group_augment[sub_tag].fn = augment_fn;
  ej->group_augment[sub_tag].type = augment_type;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving (saved repair objects)"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SAVED_REPAIR KheSavedRepairMake(KHE_AUGMENT ea,                      */
/*    int repair_type, int max_sub_chains, KHE_PATH path,                    */
/*    void (*on_success_fn)(void *on_success_val), void *on_success_val)     */
/*                                                                           */
/*  Make a saved repair object with these attributes.   Get memory off the   */
/*  free list, or by an ordinary creation.                                   */
/*                                                                           */
/*****************************************************************************/

static KHE_SAVED_REPAIR KheSavedRepairMake(KHE_AUGMENT ea,
  int repair_type, int max_sub_chains, KHE_PATH path,
  void (*on_success_fn)(void *on_success_val), void *on_success_val)
{
  KHE_SAVED_REPAIR res;  KHE_EJECTOR ej;
  ej = ea->ejector;
  if( MArraySize(ej->free_repairs) > 0 )
    res = MArrayRemoveLast(ej->free_repairs);
  else
    MMake(res);
  res->augment = ea;
  res->repair_type = repair_type;
  res->max_sub_chains = max_sub_chains;
  res->path = path;
  res->on_success_fn = on_success_fn;
  res->on_success_val = on_success_val;
  return res;
}

/* ***
static KHE_SAVED_REPAIR KheSavedRepairMake(KHE_EJECTOR ej, int repair_type)
{
  KHE_SAVED_REPAIR res;
  if( MArraySize(ej->free_repairs) > 0 )
    res = MArrayRemoveLast(ej->free_repairs);
  else
    MMake(res);
  res->repair_type = repair_type;
  res->path = NULL;
  res->on_success_fn = NULL;
  res->on_success_val = NULL;
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSavedRepairFree(KHE_SAVED_REPAIR er)                             */
/*                                                                           */
/*  Free er - not by putting it on the free list, but altogether.            */
/*                                                                           */
/*****************************************************************************/

static void KheSavedRepairFree(KHE_SAVED_REPAIR er)
{
  MFree(er);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSavedRepairDelete(KHE_SAVED_REPAIR er)                           */
/*                                                                           */
/*  Delete er, by putting it on its ejector's the free list.                 */
/*                                                                           */
/*****************************************************************************/

static void KheSavedRepairDelete(KHE_SAVED_REPAIR er)
{
  KHE_EJECTOR ej;
  ej = er->augment->ejector;
  MArrayAddLast(ej->free_repairs, er);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSavedRepairCmp(const void *t1, const void *t2)                    */
/*                                                                           */
/*  Comparison function for sorting repairs by increasing solution cost.     */
/*                                                                           */
/*****************************************************************************/

static int KheSavedRepairCmp(const void *t1, const void *t2)
{
  KHE_SAVED_REPAIR er1 = * (KHE_SAVED_REPAIR *) t1;
  KHE_SAVED_REPAIR er2 = * (KHE_SAVED_REPAIR *) t2;
  return KheCostCmp(KhePathSolnCost(er1->path), KhePathSolnCost(er2->path));
}

/* ***
static int KheSavedRepairCmp(const void *t1, const void *t2)
{
  KHE_SAVED_REPAIR r1 = * (KHE_SAVED_REPAIR *) t1;
  KHE_SAVED_REPAIR r2 = * (KHE_SAVED_REPAIR *) t2;
  return KheCostCmp(r1->soln_cost, r2->soln_cost);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSavedRepairImmediateSuccess(KHE_SAVED_REPAIR er)                 */
/*                                                                           */
/*  Return true if er is immediately successful, that is, if the current     */
/*  solution cost is below the target and any active group monitor limits    */
/*  are satisfied.                                                           */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheSavedRepairImmediateSuccess(KHE_SAVED_REPAIR er)
{
  int i;  KHE_SOLN soln;  KHE_COST_LIMIT lim;  KHE_EJECTOR ej;
  ej = er->augment->ejector;
  soln = KheMonitorSoln((KHE_MONITOR) ej->start_gm);
  if( KheSolnCost(soln) >= er->augment->target_cost )
    return false;
  if( ej->test_limits )
    MArrayForEach(ej->cost_limits, &lim, &i)
      if( KheMonitorCost(lim.monitor) > lim.cost )
	return false;
  return true;
}
*** */

/* ***
static bool KheEjectorImmediateSuccess(KHE_EJECTOR ej)
{
  int i;  KHE_SOLN soln;  KHE_COST_LIMIT lim;
  soln = KheMonitorSoln((KHE_MONITOR) ej->start_gm);
  if( KheSolnCost(soln) >= ej->curr_target_cost )
    return false;
  if( ej->test_limits )
    MArrayForEach(ej->cost_limits, &lim, &i)
      if( KheMonitorCost(lim.monitor) > lim.cost )
	return false;
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSavedRepairRecursiveSuccess(KHE_SAVED_REPAIR er)                 */
/*                                                                           */
/*  Return true if repair er is successful recursively, that is, if          */
/*  repairing one of the defects on its trace makes a successful chain.      */
/*  Alternatively, if er->multi-repair is true, try repairing all of them.   */
/*                                                                           */
/*  This function assumes that KheEjectorNotAtLimit(ej) is true, that is,    */
/*  that the chain does not currently have its maximum depth.                */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorAugment(KHE_EJECTOR ej, KHE_COST target_cost,
  KHE_MONITOR d);
static bool KheAugmentDefectIsOpen(KHE_AUGMENT ea, int i, KHE_MONITOR *d);

static bool KheSavedRepairRecursiveSuccess(KHE_SAVED_REPAIR er)
{
  int i;  KHE_MONITOR m;  KHE_AUGMENT ea;
  ** KHE_MARK mark;  KHE_SOLN soln; **
  if( er->max_sub_chains )
  {
    ** multi-repair (ejection tree) case **
    ** *** complete redo here still to do
    if( DEBUG9 )
      fprintf(stderr, "[ max_sub_chains (depth %d):\n", KheEjectorCurrDepth(ej));
    soln = KheMonitorSoln((KHE_MONITOR) ej->continue_gm);
    mark = KheMarkBegin(soln);
    save_target_cost = ej->curr_target_cost;
    trace = MArrayLast(ej->augment_stack)->trace;
    for( i = 0;  i < KheTraceMonitorCount(trace);  i++ )
    {
      ej->curr_target_cost = KheSolnCost(soln);
      ej->test_limits = false;
      if( KheEjectorDefectIsOpen(ej, trace, i, &m) )
      {
	if( DEBUG9 )
	  KheMonitorDebug(m, 2, 2, stderr);
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	if( KheEjectorAugment(ej, m) )
	{
	  ej->curr_target_cost = save_target_cost;
	  ej->test_limits = true;
	  if( KheEjectorImmediateSuccess(ej) )
	  {
	    if( DEBUG9 )
	      fprintf(stderr, "] max_sub_chains returning true\n");
	    KheMarkEnd(mark, false);
	    return true;
	  }
	}
      }
    }
    KheMarkEnd(mark, true);
    ej->curr_target_cost = save_target_cost;
    ej->test_limits = true;
    if( DEBUG9 )
      fprintf(stderr, "] max_sub_chains returning false\n");
    *** **
  }
  else
  {
    ** single repair (ejection chain) case - try to repair one monitor **
    ea = er->augment;
    for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
      if( KheAugmentDefectIsOpen(ea, i, &m) &&
	  KheEjectorAugment(ea->ejector, ea->target_cost, m) )
	return true;
  }
  return false;
}
*** */


/* ***
static bool KheEjectorRecursiveSuccess(KHE_EJECTOR ej, int max_sub_chains)
{
  int i;  KHE_MONITOR m;  KHE_MARK mark;
  KHE_SOLN soln;  KHE_TRACE trace;
  if( max_sub_chains )
  {
    ** multi-repair (ejection tree) case - try to repair all costly monitors **
    if( DEBUG9 )
      fprintf(stderr, "[ max_sub_chains (depth %d):\n", KheEjectorCurrDepth(ej));
    soln = KheMonitorSoln((KHE_MONITOR) ej->continue_gm);
    mark = KheMarkBegin(soln);
    save_target_cost = ej->curr_target_cost;
    trace = MArrayLast(ej->augment_stack)->trace;
    for( i = 0;  i < KheTraceMonitorCount(trace);  i++ )
    {
      ej->curr_target_cost = KheSolnCost(soln);
      ej->test_limits = false;
      if( KheEjectorDefectIsOpen(ej, trace, i, &m) )
      {
	if( DEBUG9 )
	  KheMonitorDebug(m, 2, 2, stderr);
	if( KheEjectorCurrDepth(ej) == 1 )
	  KheSolnNewGlobalVisit(soln);
	if( KheEjectorAugment(ej, m) )
	{
	  ej->curr_target_cost = save_target_cost;
	  ej->test_limits = true;
	  if( KheEjectorImmediateSuccess(ej) )
	  {
	    if( DEBUG9 )
	      fprintf(stderr, "] max_sub_chains returning true\n");
	    KheMarkEnd(mark, false);
	    return true;
	  }
	}
      }
    }
    KheMarkEnd(mark, true);
    ej->curr_target_cost = save_target_cost;
    ej->test_limits = true;
    if( DEBUG9 )
      fprintf(stderr, "] max_sub_chains returning false\n");
  }
  else
  {
    ** single repair (ejection chain) case - try to repair one monitor **
    trace = MArrayLast(ej->augment_stack)->trace;
    for( i = 0;  i < KheTraceMonitorCount(trace);  i++ )
      if( KheEjectorDefectIsOpen(ej, trace, i, &m) && KheEjectorAugment(ej, m) )
	return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheSavedRepairMayContinue(KHE_SAVED_REPAIR er)                      */
/*                                                                           */
/*  Repair er has just ended, and the current trace contains a trace of it.  */
/*  Return true if this repair is worth continuing with, either because the  */
/*  cost and monitor limits show that it has produced a better solution, or  */
/*  because there is a monitor that could be recursed on.                    */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorNotAtLimit(KHE_EJECTOR ej);
static bool KheAugmentSuccess(KHE_AUGMENT ea);
static bool KheAugmentDefectIsOpen(KHE_AUGMENT ea, int i, KHE_MONITOR *d);

static bool KheSavedRepairMayContinue(KHE_SAVED_REPAIR er)
{
  int i;  KHE_MONITOR m;  KHE_AUGMENT ea;
  ea = er->augment;
  if( KheAugmentSuccess(ea) )
    return true;
  if( KheEjectorNotAtLimit(ea->ejector) )
  {
    for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
      if( KheAugmentDefectIsOpen(ea, i, &m) )
	return true;
  }
  return false;
  ** ***
  if( DEBUG8 )
  {
    fprintf(stderr, "%*strace ", 4 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(m, 1, 0, stderr);
  }
  *** **
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving (augment objects)"                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_AUGMENT KheAugmentMake(KHE_EJECTOR ej, KHE_COST target_cost,         */
/*    bool test_limits, int augment_type)                                    */
/*                                                                           */
/*  Make a new augment object with these attributes.                         */
/*                                                                           */
/*****************************************************************************/

static KHE_AUGMENT KheAugmentMake(KHE_EJECTOR ej, KHE_COST target_cost,
  bool test_limits, int augment_type)
{
  KHE_AUGMENT res;
  if( MArraySize(ej->free_augments) > 0 )
  {
    res = MArrayRemoveLast(ej->free_augments);
    MArrayClear(res->saved_repairs);
    MArrayClear(res->open_monitors);
  }
  else
  {
    MMake(res);
    MArrayInit(res->saved_repairs);
    MArrayInit(res->open_monitors);
  }
  res->ejector = ej;
  res->mark = KheMarkBegin(KheEjectorSoln(ej));
  res->target_cost = target_cost;
  res->trace = KheTraceMake(ej->continue_gm);
  res->augment_type = augment_type;
  res->test_limits = test_limits;
  res->success = false;
  res->repair_count = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentFree(KHE_AUGMENT ea)                                      */
/*                                                                           */
/*  Free ea altogether.                                                      */
/*                                                                           */
/*****************************************************************************/

static void KheAugmentFree(KHE_AUGMENT ea)
{
  MAssert(MArraySize(ea->saved_repairs) == 0,
    "KheAugmentFree internal error");
  MArrayFree(ea->saved_repairs);
  MArrayFree(ea->open_monitors);
  MFree(ea);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentDelete(KHE_AUGMENT ea)                                    */
/*                                                                           */
/*  Delete ea (put it on the free list).  Also put its repairs on their      */
/*  free list.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheAugmentDelete(KHE_AUGMENT ea)
{
  while( MArraySize(ea->saved_repairs) > 0 )
    KheSavedRepairDelete(MArrayRemoveLast(ea->saved_repairs));
  KheTraceDelete(ea->trace);
  MArrayAddLast(ea->ejector->free_augments, ea);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "augment and repair algorithm (private)"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MARK KheAugmentBegin(KHE_EJECTOR ej, int augment_type)               */
/*                                                                           */
/*  Increase depth and return a mark for that depth.                         */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_MARK KheAugmentBegin(KHE_EJECTOR ej, int augment_type)
{
  KHE_MARK res;
  res = KheMarkBegin(KheMonitorSoln((KHE_MONITOR) ej->continue_gm));
  KheMarkSetInfo(res, augment_type);
  MArrayAddLast(ej->mark_stack, res);
  ej->augment_count++;
  return res;
}
*** */

/* ***
static KHE_AUGMENT KheAugmentBegin(KHE_EJECTOR ej,
  int augment_type)
{
  KHE_AUGMENT res;

  ** get an augment object, from the free list or freshly created **
  if( MArraySize(ej->free_augments) > 0 )
    res = MArrayRemoveLast(ej->free_augments);
  else
    res = KheAugmentMake(ej);

  ** initialize it, add it to the augment stack, and return it **
  res->augment_type = augment_type;
  MAssert(MArraySize(res->repairs)==0, "KheAugmentBegin internal error");
  MArrayAddLast(ej->augment_stack, res);
  ej->augment_count++;
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentEnd(KHE_EJECTOR ej)                                       */
/*                                                                           */
/*  Remove the top element from the augment stack and free it.               */
/*                                                                           */
/*****************************************************************************/

/* this is basically KheMarkEnd, need to look more closely though */
/* ***
static void KheAugmentEnd(KHE_EJECTOR ej)
{
  KHE_MARK mark;
  mark = MArrayRemoveLast(ej->mark_stack);
}
*** */

/* ***
static void KheAugmentEnd(KHE_EJECTOR ej)
{
  KHE_AUGMENT ea;  int i;
  ea = MArrayRemoveLast(ej->augment_stack);
  MArrayAppend(ej->free_repairs, ea->repairs, i);
  MArrayClear(ea->repairs);
  MArrayAddLast(ej->free_augments, ea);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentMonitorIsOpen(KHE_AUGMENT ea, KHE_MONITOR m)              */
/*                                                                           */
/*  Return true if removing defect m, as far as possible, would succeed.     */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheAugmentMonitorIsOpen(KHE_AUGMENT ea, KHE_MONITOR m)
{
  KHE_COST max_improv, cost_now;
  max_improv = KheMonitorCost(m) - KheMonitorLowerBound(m);
  cost_now = KheSolnCost(KheMonitorSoln(m));
  return max_improv > 0 && cost_now - max_improv < ea->target_cost;
}
*** */

/* ***
static bool KheEjectorMonitorIsOpen(KHE_EJECTOR ej, KHE_MONITOR m)
{
  KHE_COST max_improv, cost_now;
  max_improv = KheMonitorCost(m) - KheMonitorLowerBound(m);
  cost_now = KheSolnCost(KheMonitorSoln(m));
  return max_improv > 0 && cost_now - max_improv < ej->curr_target_cost;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentDefectIsOpen(KHE_AUGMENT ea, int i, KHE_MONITOR *d)       */
/*                                                                           */
/*  If the i'th monitor on trace increased in cost and may be repaired in    */
/*  a way that leads to success, return true and set *d to that monitor.     */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheAugmentDefectIsOpen(KHE_AUGMENT ea, int i, KHE_MONITOR *d)
{
  KHE_COST cost;
  *d = KheTraceMonitor(ea->trace, i);
  cost = KheTraceMonitorInitCost(ea->trace, i);
  return KheMonitorCost(*d) > cost && KheAugmentMonitorIsOpen(ea, *d);
}
*** */

/* ***
static bool KheEjectorDefectIsOpen(KHE_EJECTOR ej, KHE_TRACE trace,
  int i, KHE_MONITOR *d)
{
  KHE_COST cost;
  *d = KheTraceMonitor(trace, i);
  cost = KheTraceMonitorInitCost(trace, i);
  return KheMonitorCost(*d) > cost && KheEjectorMonitorIsOpen(ej, *d);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheMonitorPotential(KHE_MONITOR m)                              */
/*                                                                           */
/*  Return the potential of m:  its cost minus its lower bound.              */
/*                                                                           */
/*****************************************************************************/

static KHE_COST KheMonitorPotential(KHE_MONITOR m)
{
  return KheMonitorCost(m) - KheMonitorLowerBound(m);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorDecreasingPotentialCmp(const void *t1, const void *t2)     */
/*                                                                           */
/*  Comparison function for sorting an array of monitors by decreasing       */
/*  potential.                                                               */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorDecreasingPotentialCmp(const void *t1, const void *t2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) t1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) t2;
  return KheCostCmp(KheMonitorPotential(m2), KheMonitorPotential(m1));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentSelectOpenDefects(KHE_AUGMENT ea, int max_sub_chains)     */
/*                                                                           */
/*  Set ea->open_monitors to the best max_sub_chains open monitors, or all   */
/*  open monitors if fewer than that are available.                          */
/*                                                                           */
/*****************************************************************************/

static void KheAugmentSelectOpenDefects(KHE_AUGMENT ea, int max_sub_chains)
{
  int i, excess;  KHE_MONITOR m;

  /* accumulate the open monitors in ea->open_monitors */
  MArrayClear(ea->open_monitors);
  for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
  {
    m = KheTraceMonitor(ea->trace, i);
    if( KheMonitorCost(m) > KheTraceMonitorInitCost(ea->trace, i) )
      MArrayAddLast(ea->open_monitors, m);
  }

  /* sort the open monitors by decreasing potential */
  MArraySort(ea->open_monitors, &KheMonitorDecreasingPotentialCmp);

  /* reduce to the best max_sub_chains open monitors */
  excess = MArraySize(ea->open_monitors) - max_sub_chains;
  if( excess > 0 )
    MArrayDropFromEnd(ea->open_monitors, excess);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheAugmentSubTargetCost(KHE_AUGMENT ea, int start_pos)          */
/*                                                                           */
/*  Calculate the sub_target_cost (sub_c in the docs) from start_pos.        */
/*                                                                           */
/*****************************************************************************/

static KHE_COST KheAugmentSubTargetCost(KHE_AUGMENT ea, int start_pos)
{
  KHE_COST res;  int i;  KHE_MONITOR m;
  res = ea->target_cost;
  for( i = start_pos;  i < MArraySize(ea->open_monitors);  i++ )
  {
    m = MArrayGet(ea->open_monitors, i);
    res += KheMonitorPotential(m);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentSuccess(KHE_AUGMENT ea, KHE_COST target_cost,             */
/*    bool test_limits)                                                      */
/*                                                                           */
/*  Return true if ea is successful for target_cost and test_limits.         */
/*                                                                           */
/*****************************************************************************/

static bool KheAugmentSuccess(KHE_AUGMENT ea, KHE_COST target_cost,
  bool test_limits)
{
  int i;  KHE_COST_LIMIT lim;  KHE_EJECTOR ej;
  ej = ea->ejector;
  if( KheSolnCost(KheEjectorSoln(ej)) >= target_cost )
    return false;
  if( test_limits )
    MArrayForEach(ej->cost_limits, &lim, &i)
      if( KheMonitorCost(lim.monitor) > lim.cost )
	return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentRecursiveSuccess(KHE_AUGMENT ea, int max_sub_chains)      */
/*                                                                           */
/*  Return true if ea is a success recursively.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorNotAtLimit(KHE_EJECTOR ej);
static bool KheEjectorAugment(KHE_EJECTOR ej, KHE_COST target_cost,
  bool test_limits, KHE_MONITOR d);

static bool KheAugmentRecursiveSuccess(KHE_AUGMENT ea, int max_sub_chains)
{
  int i;  KHE_MONITOR m;  KHE_SOLN soln;  bool sub_test_limits;
  KHE_EJECTOR ej;
  ej = ea->ejector;
  soln = KheEjectorSoln(ej);
  MArrayForEach(ea->open_monitors, &m, &i)
  {
    ** quit early if target reached **
    if( KheSolnCost(soln) < ea->target_cost )
      break;

    ** *** rewrite needed here (still to do)
    ea->sub_target_cost -= KheMonitorPotential(m);
    sub_test_limits = ea->test_limits &&
      i == MArraySize(ea->open_monitors) - 1;
    if( !KheEjectorAugment(ej, ea->sub_target_cost, sub_test_limits, m) )
      return false;
    *** **
  }
  return i >= MArraySize(ea->open_monitors);
}
*** */

/* ***
static bool KheAugmentRecursiveSuccess(KHE_AUGMENT ea, int max_sub_chains)
{
  int i;  KHE_MONITOR m;
  ** KHE_MARK mark;  KHE_SOLN soln; **
  if( KheEjectorNotAtLimit(ea->ejector) )
  {
    if( max_sub_chains )
    {
      ** multi-repair (ejection tree) case **
      ** *** complete redo here still to do
      if( DEBUG9 )
	fprintf(stderr,"[ max_sub_chains (depth %d):\n", KheEjectorCurrDepth(ej));
      soln = KheMonitorSoln((KHE_MONITOR) ej->continue_gm);
      mark = KheMarkBegin(soln);
      save_target_cost = ej->curr_target_cost;
      trace = MArrayLast(ej->augment_stack)->trace;
      for( i = 0;  i < KheTraceMonitorCount(trace);  i++ )
      {
	ej->curr_target_cost = KheSolnCost(soln);
	ej->test_limits = false;
	if( KheEjectorDefectIsOpen(ej, trace, i, &m) )
	{
	  if( DEBUG9 )
	    KheMonitorDebug(m, 2, 2, stderr);
	  if( KheEjectorCurrDepth(ej) == 1 )
	    KheSolnNewGlobalVisit(soln);
	  if( KheEjectorAugment(ej, m) )
	  {
	    ej->curr_target_cost = save_target_cost;
	    ej->test_limits = true;
	    if( KheEjectorImmediateSuccess(ej) )
	    {
	      if( DEBUG9 )
		fprintf(stderr, "] max_sub_chains returning true\n");
	      KheMarkEnd(mark, false);
	      return true;
	    }
	  }
	}
      }
      KheMarkEnd(mark, true);
      ej->curr_target_cost = save_target_cost;
      ej->test_limits = true;
      if( DEBUG9 )
	fprintf(stderr, "] max_sub_chains returning false\n");
      *** **
    }
    else
    {
      ** single repair (ejection chain) case - try to repair one monitor **
      for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
	if( KheAugmentDefectIsOpen(ea, i, &m) &&
	  KheEjectorAugment(ea->ejector, ea->target_cost, ea->test_limits, m))
	  return true;
    }
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentSuccessfulEnd(KHE_AUGMENT ea)                             */
/*                                                                           */
/*  Do what's appropriate at the moment when a chain ends successfully.      */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheAugmentSuccessfulEnd(KHE_AUGMENT ea)
{
  int i;  KHE_MONITOR m;  KHE_COST_LIMIT lim;  KHE_EJECTOR ej;

  if( ea->test_limits )
  {
    ** reduce cost limits **
    ej = ea->ejector;
    MArrayForEach(ej->cost_limits, &lim, &i)
    {
      lim.cost = KheMonitorCost(lim.monitor);
      MArrayPut(ej->cost_limits, i, lim);
    }

    ** record statistics **
    KheEjectorStatsAddImprovement(ej);

    ** promote defects **
    if( KheOptionsEjectorPromoteDefects(ej->options) &&
	ej->start_gm != ej->continue_gm )
    {
      for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
      {
	m = KheTraceMonitor(ea->trace, i);
	if( KheMonitorCost(m) > KheTraceMonitorInitCost(ea->trace, i) &&
	    !KheGroupMonitorHasChildMonitor(ej->start_gm, m) )
	  KheGroupMonitorAddChildMonitor(ej->start_gm, m);
      }
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentMayContinue(KHE_AUGMENT ea)                               */
/*                                                                           */
/*  A repair of ea has just ended; the current trace contains a trace of it. */
/*  Return true if this repair is worth continuing with, either because the  */
/*  cost and monitor limits show that it has produced a better solution, or  */
/*  because there is a monitor that could be recursed on.                    */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheAugmentMayContinue(KHE_AUGMENT ea)
{
  int i;  KHE_MONITOR m;
  if( KheAugmentSuccess(ea, ea->target_cost, ea->test_limits) )
    return true;
  if( KheEjectorNotAtLimit(ea->ejector) )
  {
    for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
      if( KheAugmentDefectIsOpen(ea, i, &m) )
	return true;
  }
  return false;
  ** ***
  if( DEBUG8 )
  {
    fprintf(stderr, "%*strace ", 4 * KheEjectorCurrDepth(ej) + 2, "");
    KheMonitorDebug(m, 1, 0, stderr);
  }
  *** **
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorNotAtLimit(KHE_EJECTOR ej)                                */
/*                                                                           */
/*  Return true if the current chain is not at the maximum length.           */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorNotAtLimit(KHE_EJECTOR ej)
{
  return MArraySize(ej->augment_stack) < ej->curr_minor_schedule->max_depth
    && ej->curr_augment_count < MAX_AUGMENT_COUNT;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentSucceed(KHE_AUGMENT ea, int repair_type,                  */
/*    void (*on_success_fn)(void *on_success_val), void *on_success_val,     */
/*    int repair_count)                                                      */
/*                                                                           */
/*  Do what has to be done after a successful augment, then return true.     */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorStatsOneRepair(KHE_EJECTOR ej, int augment_type,
  int repair_type, bool success);

static bool KheAugmentSucceed(KHE_AUGMENT ea, int repair_type,
  void (*on_success_fn)(void *on_success_val), void *on_success_val,
  int repair_count)
{
  int i;  KHE_MONITOR m;  KHE_EJECTOR ej;
  if( on_success_fn != NULL )
    on_success_fn(on_success_val);
  if( ea->test_limits )
  {
    /* reduce cost limits */
    /* ***
    MArrayForEach(ej->cost_limits, &lim, &i)
    {
      lim.cost = KheMonitorCost(lim.monitor);
      MArrayPut(ej->cost_limits, i, lim);
    }
    *** */

    /* record statistics */
    /* KheEjectorStatsAddImprovement(ej, MArraySize(ej->augment_stack)); */

    /* promote defects */
    ej = ea->ejector;
    if( KheOptionsEjectorPromoteDefects(ej->options) &&
	ej->start_gm != ej->continue_gm )
    {
      for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
      {
	m = KheTraceMonitor(ea->trace, i);
	if( KheMonitorCost(m) > KheTraceMonitorInitCost(ea->trace, i) &&
	    !KheGroupMonitorHasChildMonitor(ej->start_gm, m) )
	  KheGroupMonitorAddChildMonitor(ej->start_gm, m);
      }
    }
  }
  KheEjectorStatsOneRepair(ea->ejector, ea->augment_type, repair_type, true);
  ea->success = true;
  ea->repair_count = repair_count;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentHandleRepair(KHE_AUGMENT ea, bool success,                */
/*    bool save_and_sort, int repair_type, int max_sub_chains,               */
/*    void (*on_success_fn)(void *on_success_val), void *on_success_val)     */
/*                                                                           */
/*  Do most of what needs to be done after a repair of ea ends.  Return      */
/*  true if the user's augment function should now stop generating repairs.  */
/*                                                                           */
/*****************************************************************************/
static bool KheEjectorAugment(KHE_EJECTOR ej, KHE_COST target_cost,
  bool test_limits, KHE_MONITOR d, int *repair_count);

static bool KheAugmentHandleRepair(KHE_AUGMENT ea, bool success,
  bool save_and_sort, int repair_type, int max_sub_chains,
  void (*on_success_fn)(void *on_success_val), void *on_success_val)
{
  KHE_MONITOR d;  int i, rcount, repair_count;  KHE_COST sub_target_cost;
  bool sub_test_limits;  KHE_SOLN soln;

  /* if repair was unsuccessful, forget the whole thing */
  if( !success )
  {
    KheMarkUndo(ea->mark);
    if( DEBUG6 )
      fprintf(stderr, "%*srepair %d not success\n",
	2*KheEjectorCurrDepth(ea->ejector), "", repair_type);
    return false;
  }

  /* if save_and_sort, save repair it if has some hope of succeeding */
  /* signal time to stop generating repairs if we have enough */
  soln = KheEjectorSoln(ea->ejector);
  if( save_and_sort )
  {
    /* repair was successful; if it may continue, save it for sorting later */
    if( KheAugmentSuccess(ea, ea->target_cost, ea->test_limits) ||
	(KheEjectorNotAtLimit(ea->ejector) &&
	KheSolnCost(soln) < KheAugmentSubTargetCost(ea, 0)) )
    {
      if( DEBUG6 )
	fprintf(stderr, "%*srepair %d saved\n",
	  2*KheEjectorCurrDepth(ea->ejector), "", repair_type);
      MArrayAddLast(ea->saved_repairs,
	KheSavedRepairMake(ea, repair_type, max_sub_chains,
	  KheMarkAddPath(ea->mark), on_success_fn, on_success_val));
    }
    else
    {
      if( DEBUG6 )
	fprintf(stderr, "%*srepair %d not saved\n",
	  2*KheEjectorCurrDepth(ea->ejector), "", repair_type);
    }
    KheMarkUndo(ea->mark);
    return MArraySize(ea->saved_repairs) >= MAX_REPAIRS_SAVE;
  }

  /* the remainder here is the main Augment function after applying repair */
  if( KheAugmentSuccess(ea, ea->target_cost, ea->test_limits) )
  {
    if( DEBUG6 )
      fprintf(stderr, "%*srepair %d immediate success\n",
	2*KheEjectorCurrDepth(ea->ejector), "", repair_type);
    return KheAugmentSucceed(ea, repair_type, on_success_fn, on_success_val, 1);
  }

  /* select max_sub_chains open defects and augment recursively */
  if( KheEjectorNotAtLimit(ea->ejector) )
  {
    KheAugmentSelectOpenDefects(ea, max_sub_chains);
    if( DEBUG6 )
    {
      if( max_sub_chains == INT_MAX )
	fprintf(stderr, "%*srepair (max_sub_chains INT_MAX, selected %d)\n", 
	  2*KheEjectorCurrDepth(ea->ejector), "",
	  MArraySize(ea->open_monitors));
      else
	fprintf(stderr, "%*srepair (max_sub_chains %d, selected %d)\n", 
	  2*KheEjectorCurrDepth(ea->ejector), "", max_sub_chains,
	  MArraySize(ea->open_monitors));
    }
    repair_count = 1;
    MArrayForEach(ea->open_monitors, &d, &i)
    {
      sub_target_cost = KheAugmentSubTargetCost(ea, i + 1);
      sub_test_limits =
	(i < MArraySize(ea->open_monitors) - 1 ? false : ea->test_limits);
      if( KheAugmentSuccess(ea, sub_target_cost, sub_test_limits) )
	continue;
      if( KheSolnCost(soln) - KheMonitorPotential(d) >= sub_target_cost )
	break;
      if( KheMonitorPotential(d) > 0 && !KheEjectorAugment(ea->ejector,
	    sub_target_cost, sub_test_limits, d, &rcount) )
	break;
      repair_count += rcount;
      if( KheAugmentSuccess(ea, ea->target_cost, ea->test_limits) )
        return KheAugmentSucceed(ea, repair_type, on_success_fn,
	  on_success_val, repair_count);
    }
  }
  else
  {
    if( DEBUG6 )
      fprintf(stderr, "%*srepair %d at depth limit %d\n",
	2*KheEjectorCurrDepth(ea->ejector), "", repair_type,
	KheEjectorCurrDepth(ea->ejector));
  }
  KheEjectorStatsOneRepair(ea->ejector, ea->augment_type, repair_type, false);
  KheMarkUndo(ea->mark);
  return false;
    
  /* ***
    if( KheAugmentSuccess(ea, ea->target_cost, ea->test_limits) )
  {
    ** entire chain has just ended successfully **
    if( on_success_fn != NULL )
      on_success_fn(on_success_val);
    KheAugmentSuccessfulEnd(ea);
    KheEjectorStatsOneRepair(ea->ejector, ea->augment_type, repair_type, true);
    ea->success = true;
    return true;
  }
  else if( KheAugmentRecursiveSuccess(ea, max_sub_chains))
  {
    ** entire chain is successful after recursion **
    ** ea = MArrayLast(ej->augment_stack);  ** array may have reallocated! **
    if( on_success_fn != NULL )
      on_success_fn(on_success_val);
    KheEjectorStatsOneRepair(ea->ejector, ea->augment_type, repair_type, true);
    ea->success = true;
    return true;
  }
  else
  {
    ** chain is unsuccessful after recursion **
    ** ea = MArrayLast(ej->augment_stack);  ** array may have reallocated! **
    KheEjectorStatsOneRepair(ea->ejector, ea->augment_type, repair_type, false);
    KheMarkUndo(ea->mark);
    return false;
  }
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorHandleSuccess(KHE_EJECTOR ej, int augment_type,           */
/*    int repair_type, bool at_end)                                          */
/*                                                                           */
/*  Do what's appropriate when returning up a successful chain.              */
/*  If at_end, we are right at the end of the chain.                         */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheEjectorHandleSuccess(KHE_EJECTOR ej, int augment_type,
  int repair_type, bool at_end)
{
  int i;  KHE_MONITOR m;  KHE_COST_LIMIT lim;  KHE_TRACE trace;
  if( at_end )
  {
    ** reduce cost limits **
    if( ej->test_limits )
      MArrayForEach(ej->cost_limits, &lim, &i)
      {
	lim.cost = KheMonitorCost(lim.monitor);
	MArrayPut(ej->cost_limits, i, lim);
      }

    ** record statistics **
    KheEjectorStatsAddImprovement(ej);

    ** promote defects **
    if( KheOptionsEjectorPromoteDefects(ej->options) &&
	ej->start_gm != ej->continue_gm )
    {
      trace = MArrayLast(ej->augment_stack)->trace;
      for( i = 0;  i < KheTraceMonitorCount(trace);  i++ )
      {
	m = KheTraceMonitor(trace, i);
	if( KheMonitorCost(m) > KheTraceMonitorInitCost(trace, i) &&
	    !KheGroupMonitorHasChildMonitor(ej->start_gm, m) )
	  KheGroupMonitorAddChildMonitor(ej->start_gm, m);
      }
    }
  }
  KheEjectorStatsOneRepair(ej, augment_type, repair_type, true);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorAugment(KHE_EJECTOR ej, KHE_COST target_cost,             */
/*    bool test_limits, KHE_MONITOR d, int *repair_count)                    */
/*                                                                           */
/*  Carry out one augment, including growing and shrinking the augment       */
/*  stack, and dispatching one call to the user's augment function.          */
/*  Return true if the chain or tree is successful; and in that case, set    */
/*  *repair_count to the number of repairs in the successful chain or tree.  */
/*  Success means reducing the solution cost to below target_cost, and if    */
/*  test_limits is true, doing so without violating any monitor limits.      */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorAugment(KHE_EJECTOR ej, KHE_COST target_cost,
  bool test_limits, KHE_MONITOR d, int *repair_count)
{
  KHE_EJECTOR_AUGMENT_FN augment_fn;  int sub_tag, augment_type, i;
  KHE_SAVED_REPAIR er;  KHE_SOLN soln;  KHE_AUGMENT ea;  bool success;

  /* find the user's augment function, or NULL if none */
  MAssert(KheMonitorPotential(d) > 0, "KheEjectorAugment internal error 1");
  if( KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG )
  {
    sub_tag = KheGroupMonitorSubTag((KHE_GROUP_MONITOR) d);
    MAssert(sub_tag >= 0 && sub_tag < MAX_GROUP_AUGMENT,
      "KheEjectorAugment: sub_tag (%d) out of range", sub_tag);
    augment_fn = ej->group_augment[sub_tag].fn;
    augment_type = ej->group_augment[sub_tag].type;
  }
  else
  {
    augment_fn = ej->nongroup_augment[KheMonitorTag(d)].fn;
    augment_type = ej->nongroup_augment[KheMonitorTag(d)].type;
  }

  /* quit now if there is no augment function */
  if( augment_fn == NULL )
    return false;

  /* boilerplate */
  soln = KheMonitorSoln(d);
  if( DEBUG4 )
    fprintf(stderr,
      "%*s[ KheEjectorAugment (ej, %.5f, %s, %s:%.5f) soln %.5f depth %d%s\n",
      2*KheEjectorCurrDepth(ej), "", KheCostShow(target_cost),
      test_limits ? "true" : "false", 
      KheMonitorLabel(d), KheCostShow(KheMonitorCost(d)),
      KheCostShow(KheSolnCost(soln)), KheEjectorCurrDepth(ej),
      KheEjectorCurrDepth(ej) == ej->curr_minor_schedule->max_depth - 1 ?
      " (limit)" : "");

  /* push a new element onto the augment stack */
  /* mark = KheAugmentBegin(ej, augment_type); */
  ea = KheAugmentMake(ej, target_cost, test_limits, augment_type);
  /* ***
  ea->augment_type = augment_type;
  ea->mark = KheMarkBegin(soln);
  ea_rec.mark = KheMarkBegin(soln);
  KheMarkSetInfo(ea_rec.mark, augment_type);
  ea_rec.trace = KheTraceMake(ej->continue_gm);
  ea_rec.success = false;
  *** */
  MArrayAddLast(ej->augment_stack, ea);

  /* call the user's augment function */
  ej->augment_count++;
  ej->curr_augment_count++;
  augment_fn(ej, d);
  MAssert(ej->state == KHE_EJECTOR_SOLVE_RUN,
    "KheEjectorRepairBegin called without matching KheEjectorRepairEnd");
  /* ea = MArrayLast(ej->augment_stack);  ** array may have reallocated! */

  /* try save_and_sort repairs, if no success yet and there are some */
  if( !ea->success && MArraySize(ea->saved_repairs) > 0 )
  {
    /* KheMarkPathSort(ea->mark, &KhePathIncreasingSolnCostCmp); */
    MArraySort(ea->saved_repairs, &KheSavedRepairCmp);
    for( i = 0; i < MAX_REPAIRS_TRY && i < MArraySize(ea->saved_repairs); i++ )
    {
      er = MArrayGet(ea->saved_repairs, i);
      KheTraceBegin(ea->trace);
      /* mark = KheMarkBegin(KheMonitorSoln(d)); */
      KhePathRedo(er->path);
      /* KheTransactionRedo(er->transaction); */
      KheTraceEnd(ea->trace);
      if( KheAugmentHandleRepair(ea, true, false, er->repair_type,
	    er->max_sub_chains, er->on_success_fn, er->on_success_val) )
	break;
      /* ***
      if( KheAugmentSuccess(ea) )
      {
	** successful as is **
	if( er->on_success_fn != NULL )
	  er->on_success_fn(er->on_success_val);
	KheAugmentSuccessfulEnd(ea);
	KheEjectorStatsOneRepair(ej, ea->augment_type, er->repair_type, true);
	ea->success = true;
	break;
      }
      else if( KheAugmentRecursiveSuccess(ea, er->max_sub_chains) )
      {
	** successful after recursion **
	if( er->on_success_fn != NULL )
	  er->on_success_fn(er->on_success_val);
        KheEjectorStatsOneRepair(ej, ea->augment_type, er->repair_type, true);
	ea->success = true;
	break;
      }
      else
      {
	** chain is unsuccessful after recursion **
	KheEjectorStatsOneRepair(ej, ea->augment_type, er->repair_type, false);
	KheMarkUndo(ea->mark);
	** KheTransactionUndo(er->transaction); **
	** KheMarkEnd(mark, true); **
      }
      *** */
    }
  }

  /* pop augment stack and exit */
  /* KheAugmentEnd(ej); */
  KheMarkEnd(ea->mark, !ea->success);
  success = ea->success;
  *repair_count = ea->repair_count;
  KheAugmentDelete(MArrayRemoveLast(ej->augment_stack));
  if( DEBUG4 )
    fprintf(stderr, "%*s] KheEjectorAugment returning %s\n",
      2*KheEjectorCurrDepth(ej), "", success ? "true" : "false");
  return success;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorMainAugment(KHE_EJECTOR ej, KHE_MONITOR d,                */
/*    int *repair_count)                                                     */
/*                                                                           */
/*  Augment from d, trying each minor schedule in turn.  This function is    */
/*  called from the main loop, so it has to initialize for finding one       */
/*  chain as well as start the chain.                                        */
/*                                                                           */
/*  When this function returns true, *repair_count is set to the number      */
/*  of repairs in the successful chain or tree.                              */
/*                                                                           */
/*  Implementation note.  It is possible for the cost of d to change to      */
/*  0 even when the algorithm does not succeed, when d is a resource         */
/*  demand monitor.  Hence the test at the start of each iteration.          */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorMainAugment(KHE_EJECTOR ej, KHE_MONITOR d,
  int *repair_count)
{
  KHE_COST save_cost;  int i, msc_count;  KHE_EJECTOR_MAJOR_SCHEDULE ejs;
  KHE_SOLN soln;
  MAssert(KheMonitorCost(d) > 0, "KheEjectorMainAugment internal error 1");

  /* initialize solve fields of ej */
  soln = KheMonitorSoln(d);
  ejs = ej->curr_major_schedule;
  /* ej->curr_target_cost = KheSolnCost(soln); */

  /* run each minor schedule */
  save_cost = KheSolnCost(soln);
  msc_count = MArraySize(ejs->minor_schedules);
  for( i = 0;  KheMonitorPotential(d) > 0 && i < msc_count;  i++ )
  {
    ej->curr_minor_schedule = MArrayGet(ejs->minor_schedules, i);
    KheSolnNewGlobalVisit(soln);
    MAssert(MArraySize(ej->augment_stack) == 0,
      "KheEjectorMainAugment internal error 2");
    ej->curr_augment_count = 0;
    if( KheEjectorAugment(ej, KheSolnCost(soln), true, d, repair_count) )
      return true;
    if( DEBUG1 && KheSolnCost(soln) != save_cost )
      KheSolnCostByTypeDebug(soln, 2, 2, stderr);
    MAssert(KheSolnCost(soln) == save_cost,
      "KheEjectorMainAugment internal error 3: %.5f != %.5f",
      KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving (main part)"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorSolve(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,         */
/*    KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options)                    */
/*                                                                           */
/*  Run a solve using ej.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorSolve(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,
  KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options)
{
  MAssert(ej->state == KHE_EJECTOR_SOLVE_IDLE,
    "KheEjectorSolve called out of order");
  KheEjectorSolveBegin(ej, start_gm, continue_gm, options);
  return KheEjectorSolveEnd(ej);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorSolveBegin(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,    */
/*    KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options)                    */
/*                                                                           */
/*  Begin setting up for a solve.                                            */
/*                                                                           */
/*****************************************************************************/

void KheEjectorSolveBegin(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,
  KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options)
{
  MAssert(ej->state == KHE_EJECTOR_SOLVE_IDLE,
    "KheEjectorSolveBegin called out of order");
  MAssert(KheMonitorSoln((KHE_MONITOR) start_gm) ==
    KheMonitorSoln((KHE_MONITOR) continue_gm),
    "KheEjectorSolveBegin: start_gm and continue_gm monitor unequal solutions");
  ej->start_gm = start_gm;
  ej->continue_gm = continue_gm;
  ej->options = options;
  ej->state = KHE_EJECTOR_SOLVE_SETTING;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorDecreasingCostCmp(const void *t1, const void *t2)          */
/*  int KheMonitorDecreasingCostDiversifyCmp(const void *t1, const void *t2) */
/*                                                                           */
/*  Comparison functions for sorting an array of defects, first by           */
/*  increasing visit number, then by decreasing cost, then by index number.  */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorDecreasingCostCmp(const void *t1, const void *t2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) t1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) t2;
  int cmp;
  /* ***
  if( KheMonitorVisitNum(m1) != KheMonitorVisitNum(m2) )
    return KheMonitorVisitNum(m1) - KheMonitorVisitNum(m2);
  *** */
  cmp = KheCostCmp(KheMonitorCost(m1), KheMonitorCost(m2));
  if( cmp != 0 )
    return cmp;
  return KheMonitorSolnIndex(m1) - KheMonitorSolnIndex(m2);
}

static int KheMonitorDecreasingCostDiversifyCmp(const void *t1,
  const void *t2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) t1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) t2;
  int cmp;
  cmp = KheCostCmp(KheMonitorCost(m1), KheMonitorCost(m2));
  /* ***
  if( KheMonitorVisitNum(m1) != KheMonitorVisitNum(m2) )
    return KheMonitorVisitNum(m1) - KheMonitorVisitNum(m2);
  *** */
  if( cmp != 0 )
    return cmp;
  else if( KheSolnDiversifierChoose(KheMonitorSoln(m1), 2) == 0 )
    return KheMonitorSolnIndex(m1) - KheMonitorSolnIndex(m2);
  else
    return KheMonitorSolnIndex(m2) - KheMonitorSolnIndex(m1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorCopyAndSortDefects(KHE_EJECTOR ej)                        */
/*                                                                           */
/*  Copy and sort the defects of ej's start monitor.  Also drop some from    */
/*  the end of the sorted list, if there are too many.                       */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorCopyAndSortDefects(KHE_EJECTOR ej)
{
  KHE_MONITOR d;  int i, ejector_limit_defects, drop;

  /* make sure soln's cost is up to date (IMPORTANT - nasty bug fix!) */
  /* KheSolnCost(KheEjectorSoln(ej)); see KheGroupMonitorDefectCount for fix */

  /* copy the defects into ej->curr_defects */
  MArrayClear(ej->curr_defects);
  for( i = 0;  i < KheGroupMonitorDefectCount(ej->start_gm);  i++ )
  {
    d = KheGroupMonitorDefect(ej->start_gm, i);
    MArrayAddLast(ej->curr_defects, d);
  }

  /* sort ej->curr_defects */
  MArraySort(ej->curr_defects, KheOptionsDiversify(ej->options) ?
    KheMonitorDecreasingCostDiversifyCmp : KheMonitorDecreasingCostCmp);

  /* ensure that there are at most ejector_limit_defects defects on list */
  ejector_limit_defects = KheOptionsEjectorLimitDefects(ej->options);
  drop = MArraySize(ej->curr_defects) - ejector_limit_defects;
  if( drop > 0 )
  {
    if( DEBUG8 )
    {
      fprintf(stderr,
	"  KheEjectorCopyAndSortDefects: cost %.5f, %d defects, dropping %d:\n",
	KheCostShow(KheSolnCost(KheEjectorSoln(ej))),
	MArraySize(ej->curr_defects), drop);
      for( i = 0;  i < drop;  i++ )
      {
	d = MArrayGet(ej->curr_defects, MArraySize(ej->curr_defects) - drop+i);
	KheMonitorDebug(d, 1, 4, stderr);
      }
    }
    MArrayDropFromEnd(ej->curr_defects, drop);
  }
  else if( DEBUG8 )
    fprintf(stderr, "  KheEjectorCopyAndSortDefects: cost %.5f, %d defects\n",
      KheCostShow(KheSolnCost(KheEjectorSoln(ej))),
      MArraySize(ej->curr_defects));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorBriefDebugDefects(KHE_EJECTOR ej, int indent, FILE *fp)   */
/*                                                                           */
/*  Produce a brief debug print of the current defects.                      */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheEjectorBriefDebugDefects(KHE_EJECTOR ej, int indent, FILE *fp)
{
  KHE_MONITOR_TAG tag;  int count, i;  KHE_MONITOR m;  KHE_GROUP_MONITOR gm;
  for(tag = KHE_ASSIGN_RESOURCE_MONITOR_TAG; tag < KHE_MONITOR_TAG_COUNT; tag++)
  {
    count = 0;
    MArrayForEach(ej->curr_defects, &m, &i)
    {
      if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
      {
	gm = (KHE_GROUP_MONITOR) m;
	if( KheGroupMonitorDefectCount(gm) > 0 )
	  m = KheGroupMonitorDefect(gm, 0);
      }
      if( KheMonitorTag(m) == tag )
	count++;
    }
    if( count > 0 )
      fprintf(stderr, "%*s%3d %s\n", indent, "", count, KheMonitorTagShow(tag));
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorDebugDefects(KHE_GROUP_MONITOR gm, char *label,           */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of the defects of gm.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorDebugDefects(KHE_GROUP_MONITOR gm, char *label,
  int indent, FILE *fp)
{
  int i;  KHE_MONITOR d;
  for( i = 0;  i < KheGroupMonitorDefectCount(gm);  i++ )
  {
    d = KheGroupMonitorDefect(gm, i);
    fprintf(stderr, "%*s%s ", indent, "", label);
    KheMonitorDebug(d, 2, 2, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorDebugDefects2(KHE_EJECTOR ej, int indent, FILE *fp)       */
/*                                                                           */
/*  Debug print of the defects of ej.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorDebugDefects2(KHE_EJECTOR ej, int indent, FILE *fp)
{
  int i;  KHE_MONITOR d;
  MArrayForEach(ej->curr_defects, &d, &i)
    KheMonitorDebug(d, 1, 2, stderr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSetFailures(KHE_MONITOR m, int f)                         */
/*  int KheMonitorFailures(KHE_MONITOR m)                                    */
/*                                                                           */
/*  Set and get the number of failures since the last success.               */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheMonitorSetFailures(KHE_MONITOR m, int f)
{
  KheMonitorSetVisitNum(m, f);
}

static int KheMonitorFailures(KHE_MONITOR m)
{
  return KheMonitorVisitNum(m);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorDoOnePhase(KHE_EJECTOR ej)                                */
/*                                                                           */
/*  Carry out one phase of the solve, returning true if progress was made.   */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorStatsAddImprovement(KHE_EJECTOR ej, int repair_count);

static bool KheEjectorDoOnePhase(KHE_EJECTOR ej)
{
  /* bool ejector_suppress_recent; */
  int i, j, /* f, phase_visit_num, */ repair_count;
  KHE_MONITOR d, successful_d;  KHE_SOLN soln;  KHE_COST_LIMIT lim;
  soln = KheEjectorSoln(ej);
  /* phase_visit_num = KheSolnGlobalVisitNum(soln); */
  /* ejector_suppress_recent = KheOptionsEjectorSuppressRecent(ej->options); */
  successful_d = NULL;
  KheEjectorCopyAndSortDefects(ej);
  if( DEBUG2 || DEBUG3 )
  {
    fprintf(stderr, "  ejector starting phase (%d defects):\n",
      MArraySize(ej->curr_defects));
    /* KheEjectorBriefDebugDefects(ej, 2, stderr); */
    KheEjectorDebugDefects2(ej, 2, stderr);
  }
  MArrayForEach(ej->curr_defects, &d, &i)
  {
    /* quit early if progressing and d has a larger visit num */
    /* ***
    if( successful_d != NULL &&
	KheMonitorVisitNum(d) > KheMonitorVisitNum(successful_d) )
      break;
    *** */

    /* handle d in one of several ways */
    if( DEBUG3 )
    {
      fprintf(stderr, "  augment ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
    /* f = KheMonitorFailures(d); */
    if( KheMonitorPotential(d) <= 0 )
    {
      if( DEBUG2 || DEBUG3 )
      {
	fprintf(stderr, "  drop ");
	KheMonitorDebug(d, 1, 0, stderr);
      }
    }
    else if( KheMonitorVisitNum(d) > ej->curr_successful_visit )
    {
      if( DEBUG2 || DEBUG3 )
      {
	fprintf(stderr, "  skip ");
	KheMonitorDebug(d, 1, 0, stderr);
      }
    }
    else if( KheEjectorMainAugment(ej, d, &repair_count) )
    {
      if( DEBUG2 || DEBUG3 )
      {
        if( KheMonitorVisitNum(d) > ej->curr_successful_visit )
	  fprintf(stderr, "  xsuc ");
	else
	  fprintf(stderr, "  succ ");
	KheMonitorDebug(d, 1, 0, stderr);
      }

      /* update visit num */
      KheMonitorSetVisitNum(d, KheSolnGlobalVisitNum(soln));

      /* add to improvement stats */
      KheEjectorStatsAddImprovement(ej, repair_count);

      /* update reducing cost limits */
      MArrayForEach(ej->cost_limits, &lim, &j)
	if( lim.reducing )
	{
	  lim.cost = KheMonitorCost(lim.monitor);
	  MArrayPut(ej->cost_limits, j, lim);
	}

      successful_d = d;
      ej->curr_successful_visit = KheSolnGlobalVisitNum(soln);
      /* ***
      KheMonitorSetFailures(d, 0);
      if( DEBUG2 || DEBUG3 )
	fprintf(stderr,
	  "  success on %s (%d of %d), cost now %.5f\n", KheMonitorLabel(d),
	  i, MArraySize(ej->curr_defects), KheCostShow(KheSolnCost(soln)));
      *** */
    }
    else
    {
      if( DEBUG2 || DEBUG3 )
      {
	fprintf(stderr, "  fail ");
	KheMonitorDebug(d, 1, 0, stderr);
      }

      /* update visit num */
      KheMonitorSetVisitNum(d, KheSolnGlobalVisitNum(soln));

      /* ***
      KheMonitorSetFailures(d, f + 1);
      if( ejector_suppress_recent )
	KheMonitorSetVisitNum(d, phase_visit_num);
      *** */
    }
  }
  return successful_d != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorSolveEnd(KHE_EJECTOR ej)                                  */
/*                                                                           */
/*  End setting up for a solve, including doing the actual solving.          */
/*                                                                           */
/*****************************************************************************/
static void KheEjectorResetSolveStats(KHE_EJECTOR ej, KHE_SOLN soln);

bool KheEjectorSolveEnd(KHE_EJECTOR ej)
{
  int i;  KHE_SOLN soln;  KHE_COST initial_cost;

  /* set up for solving */
  MAssert(ej->state == KHE_EJECTOR_SOLVE_SETTING,
    "KheEjectorSolveEnd called out of order");
  ej->state = KHE_EJECTOR_SOLVE_RUN;
  soln = KheEjectorSoln(ej);
  initial_cost = KheSolnCost(soln);
  MArrayClear(ej->augment_stack);
  KheEjectorResetSolveStats(ej, soln);
  if( DEBUG5 )
    fprintf(stderr, "[ KheEjectorSolveEnd(initial cost %.5f)\n",
      KheCostShow(initial_cost));
  if( DEBUG3 )
    KheEjectorDebugDefects(ej->start_gm, "init", 2, stderr);

  /* for each major schedule, repeatedly carry out one phase */
  MArrayForEach(ej->major_schedules, &ej->curr_major_schedule, &i)
  {
    ej->curr_successful_visit = KheSolnGlobalVisitNum(soln); /* schedule */
    while( KheEjectorDoOnePhase(ej) && !KheSolnTimeLimitReached(soln) );
    /* one more for luck; the augment functions are nondeterministic now */
    /* KheEjectorDoOnePhase(ej);  didn't help */
  }

  /* clean up and exit */
  if( DEBUG3 )
    KheEjectorDebugDefects(ej->start_gm, "final", 2, stderr);
  MAssert(KheSolnCost(soln) <= initial_cost,
    "KheEjectorSolve: final cost (%.5f) exceeds initial cost (%.5f)\n",
     KheCostShow(KheSolnCost(soln)), KheCostShow(initial_cost));
  ej->state = KHE_EJECTOR_SOLVE_IDLE;
  ej->start_gm = NULL;
  ej->continue_gm = NULL;
  ej->options = NULL;
  MArrayClear(ej->cost_limits);
  MAssert(MArraySize(ej->augment_stack) == 0,
    "KheEjectorSolveEnd internal error");
  ej->augment_count = 0;
  MArrayClear(ej->curr_defects);

  if( DEBUG5 )
    fprintf(stderr, "] KheEjectorSolveEnd(final cost %.5f)\n",
      KheCostShow(KheSolnCost(soln)));
  return KheSolnCost(soln) < initial_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving - cost limits"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorDoAddMonitorCostLimit(KHE_EJECTOR ej,                     */
/*    KHE_MONITOR m, KHE_COST cost_limit, bool reducing)                     */
/*                                                                           */
/*  Add a cost limit to ej.                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorDoAddMonitorCostLimit(KHE_EJECTOR ej,
  KHE_MONITOR m, KHE_COST cost_limit, bool reducing)
{
  KHE_COST_LIMIT lim;
  lim.monitor = m;
  lim.cost = cost_limit;
  lim.reducing = reducing; 
  MArrayAddLast(ej->cost_limits, lim);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej, KHE_MONITOR m,        */
/*    KHE_COST cost_limit)                                                   */
/*                                                                           */
/*  Add a cost limit to ej.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej, KHE_MONITOR m,
  KHE_COST cost_limit)
{
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectorAddMonitorCostLimit(ej, m, %.5f)\n",
      KheCostShow(cost_limit));
    if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
      KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) m, 2, 2, stderr);
    else
      KheMonitorDebug(m, 2, 2, stderr);
    fprintf(stderr, "]\n");
  }
  MAssert(ej->state == KHE_EJECTOR_SOLVE_SETTING,
    "KheEjectorAddMonitorCostLimit called out of order");
  KheEjectorDoAddMonitorCostLimit(ej, m, cost_limit, false);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddMonitorCostLimitReducing(KHE_EJECTOR ej,               */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Add a reducing cost limit to ej.                                         */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddMonitorCostLimitReducing(KHE_EJECTOR ej,
  KHE_MONITOR m)
{
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectorAddMonitorCostLimitReducing(ej, m (%.5f))\n",
      KheCostShow(KheMonitorCost(m)));
    if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
      KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) m, 2, 2, stderr);
    else
      KheMonitorDebug(m, 2, 2, stderr);
    fprintf(stderr, "]\n");
  }
  MAssert(ej->state == KHE_EJECTOR_SOLVE_SETTING,
    "KheEjectorAddMonitorCostLimitReducing called out of order");
  KheEjectorDoAddMonitorCostLimit(ej, m, KheMonitorCost(m), true);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej)                      */
/*                                                                           */
/*  Return the number of cost limits in ej.                                  */
/*                                                                           */
/*****************************************************************************/

int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->cost_limits);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,                   */
/*    KHE_MONITOR *m, KHE_COST *cost_limit, bool *reducing)                  */
/*                                                                           */
/*  Return the i'th cost limit of ej.                                        */
/*                                                                           */
/*****************************************************************************/

void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,
  KHE_MONITOR *m, KHE_COST *cost_limit, bool *reducing)
{
  KHE_COST_LIMIT lim;
  lim = MArrayGet(ej->cost_limits, i);
  *m = lim.monitor;
  *cost_limit = lim.cost;
  *reducing = lim.reducing;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving - queries"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheEjectorStartGroupMonitor(KHE_EJECTOR ej)            */
/*                                                                           */
/*  Return the group monitor whose defects ej's main loop repairs.           */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheEjectorStartGroupMonitor(KHE_EJECTOR ej)
{
  return ej->start_gm;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheEjectorContinueGroupMonitor(KHE_EJECTOR ej)         */
/*                                                                           */
/*  Return the group monitor whose defects ej's augment functions repair.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheEjectorContinueGroupMonitor(KHE_EJECTOR ej)
{
  return ej->continue_gm;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS KheEjectorOptions(KHE_EJECTOR ej)                            */
/*                                                                           */
/*  Return the options object passed to KheEjectorSolve.                     */
/*                                                                           */
/*****************************************************************************/

KHE_OPTIONS KheEjectorOptions(KHE_EJECTOR ej)
{
  return ej->options;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej)                                  */
/*                                                                           */
/*  Return start_gm's (and also continue_gm)'s solution.                     */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej)
{
  return KheMonitorSoln((KHE_MONITOR) ej->start_gm);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej)                            */
/*                                                                           */
/*  Return the target cost of ej.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej)
{
  KHE_AUGMENT ea;
  MAssert(MArraySize(ej->augment_stack) > 0,
    "KheEjectorTargetCost: not currently augmenting");
  ea = MArrayLast(ej->augment_stack);
  return ea->target_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorCurrMajorSchedule(KHE_EJECTOR ej)   */
/*                                                                           */
/*  Return the major schedule currently in use.                              */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorCurrMajorSchedule(KHE_EJECTOR ej)
{
  return ej->curr_major_schedule;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_MINOR_SCHEDULE KheEjectorCurrMinorSchedule(KHE_EJECTOR ej)   */
/*                                                                           */
/*  Return the minor schedule currently in use.                              */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_MINOR_SCHEDULE KheEjectorCurrMinorSchedule(KHE_EJECTOR ej)
{
  return ej->curr_minor_schedule;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorCurrMayRevisit(KHE_EJECTOR ej)                            */
/*                                                                           */
/*  Return true if the current minor schedule allows revisiting.             */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorCurrMayRevisit(KHE_EJECTOR ej)
{
  return ej->curr_minor_schedule->may_revisit;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorCurrDepth(KHE_EJECTOR ej)                                  */
/*                                                                           */
/*  Return the depth of the current chain.                                   */
/*                                                                           */
/*****************************************************************************/

int KheEjectorCurrDepth(KHE_EJECTOR ej)
{
  return MArraySize(ej->augment_stack);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorCurrDisruption(KHE_EJECTOR ej)                             */
/*                                                                           */
/*  Return the disrupt ion of the current chain.                              */
/*                                                                           */
/*****************************************************************************/

/* *** obsolete
int KheEjectorCurrDisruption(KHE_EJECTOR ej)
{
  return ej->curr_d isruption;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorCurrAugmentCount(KHE_EJECTOR ej)                           */
/*                                                                           */
/*  Return the number of augments on ej since the beginning of this solve.   */
/*                                                                           */
/*****************************************************************************/

int KheEjectorCurrAugmentCount(KHE_EJECTOR ej)
{
  return ej->augment_count;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector solving - repairs"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorRepairBegin(KHE_EJECTOR ej)                               */
/*                                                                           */
/*  Begin a repair.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheEjectorRepairBegin(KHE_EJECTOR ej)
{
  KHE_AUGMENT ea;

  /* check and change state */
  MAssert(ej->state == KHE_EJECTOR_SOLVE_RUN,
    "KheEjectorRepairBegin called out of order");
  ej->state = KHE_EJECTOR_SOLVE_RUN_REPAIR;

  /* fresh visit number if requested and depth is 1 */
  if( KheOptionsEjectorFreshVisits(ej->options) && KheEjectorCurrDepth(ej)==1 )
    KheSolnNewGlobalVisit(KheEjectorSoln(ej));

  /* start tracing */
  ea = MArrayLast(ej->augment_stack);
  MAssert(!ea->success,
    "KheEjectorRepairBegin called after last KheEjectorRepairEnd");
  KheTraceBegin(ea->trace);

  /* make a repair object, add it to ea's repair list, and start tracing */
  /* *** not making a repair object now
  ea = MArrayLast(ej->augment_stack);
  ea->curr_repair = KheSavedRepairMake(ea, repair_type);
  ** MArrayAddLast(ea->repairs, er); **
  KheTraceBegin(ea->trace);
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorRepairOnSuccess(KHE_EJECTOR ej,                           */
/*    void (*on_success_fn)(void *on_success_val), void *on_success_val)     */
/*                                                                           */
/*  Set these attributes of the current repair object.                       */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheEjectorRepairOnSuccess(KHE_EJECTOR ej,
  void (*on_success_fn)(void *on_success_val), void *on_success_val)
{
  KHE_SAVED_REPAIR er;  KHE_AUGMENT ea;
  MAssert(ej->state == KHE_EJECTOR_SOLVE_RUN_REPAIR,
    "KheEjectorRepairOnSuccess called out of order");
  ea = MArrayLast(ej->augment_stack);
  ** er = MArrayLast(ea->repairs); **
  er = ea->curr_repair;
  er->on_success_fn = on_success_fn;
  er->on_success_val = on_success_val;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorRepairEndLong(KHE_EJECTOR ej, int repair_type,            */
/*    bool success, int max_sub_chains, bool save_and_sort,                  */
/*    void (*on_success_fn)(void *on_success_val), void *on_success_val)     */
/*                                                                           */
/*  End a repair, return true if augment function should end early.          */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorRepairEndLong(KHE_EJECTOR ej, int repair_type,
  bool success, int max_sub_chains, bool save_and_sort,
  void (*on_success_fn)(void *on_success_val), void *on_success_val)
{
  KHE_AUGMENT ea;
  MAssert(max_sub_chains > 0,
    "KheEjectorRepairEndLong: max_subchains (%d) out of range", max_sub_chains);
  MAssert(ej->state == KHE_EJECTOR_SOLVE_RUN_REPAIR,
    "KheEjectorRepairEnd called out of order");
  ej->state = KHE_EJECTOR_SOLVE_RUN;
  ea = MArrayLast(ej->augment_stack);
  KheTraceEnd(ea->trace);
  return KheAugmentHandleRepair(ea, success, save_and_sort, repair_type,
    max_sub_chains, on_success_fn, on_success_val);
  /* ***
  else if( KheAugmentSuccess(ea) )
  {
    ** entire chain has just ended successfully **
    if( on_success_fn != NULL )
      on_success_fn(on_success_val);
    KheAugmentSuccessfulEnd(ea);
    KheEjectorStatsOneRepair(ej, ea->augment_type, repair_type, true);
    ea->success = true;
    return true;
  }
  else if( KheAugmentRecursiveSuccess(ea, max_sub_chains))
  {
    ** entire chain is successful after recursion **
    ** ea = MArrayLast(ej->augment_stack);  ** array may have reallocated! **
    if( on_success_fn != NULL )
      on_success_fn(on_success_val);
    KheEjectorStatsOneRepair(ej, ea->augment_type, repair_type, true);
    ea->success = true;
    return true;
  }
  else
  {
    ** chain is unsuccessful after recursion **
    ** ea = MArrayLast(ej->augment_stack);  ** array may have reallocated! **
    KheEjectorStatsOneRepair(ej, ea->augment_type, repair_type, false);
    KheMarkUndo(ea->mark);
    return false;
  }
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorRepairEnd(KHE_EJECTOR ej, int repair_type, bool success)  */
/*                                                                           */
/*  Abbreviated form of KheEjectorRepairEndLong.                             */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorRepairEnd(KHE_EJECTOR ej, int repair_type, bool success)
{
  return KheEjectorRepairEndLong(ej, repair_type, success, 1, false,NULL,NULL);
}


/* *** old version equivalent to old KheEjectorSuccess
bool KheEjectorRepairEnd(KHE_EJECTOR ej, bool success, int disr uption)
{
  KHE_AUGMENT ea, ea2;  KHE_SAVED_REPAIR er;
  KHE_MONITOR m;  int i;  KHE_SOLN soln;  KHE_COST max_improvement;

  ** retrieve the repair object and end its transaction and trace **
  ea = MArrayGet(ej->augment_stack, ej->curr_depth - 1);
  er = MArrayLast(ea->repairs);
  KheTransactionEnd(er->transaction);
  KheTraceEnd(ea->trace);

  ** if user says repair was unsuccessful, remove it and return false **
  if( !success )
  {
    KheTransaction Undo(er->transaction);
    MArrayAddLast(ej->free_repairs, MArrayRemoveLast(ea->repairs));
    return false;
  }

  ** disrup tion has increased **
  ej->curr_disrupt ion += disr uption;

  ** if the current solution is successful, handle it and return **
  soln = KheMonitorSoln((KHE_MONITOR) ej->start_gm);
  if( KheEjectorImmediateSuccess(ej, soln) )
  {
    ** new best **
    if( DEBUG5 )
      fprintf(stderr, "  KheEjectorSolve new best (cost %.5f, depth %d)\n",
	KheCostShow(KheSolnCost(soln)), KheEjectorCurrDepth(ej));
    if( DEBUG4 )
      fprintf(stderr, "%*ssucceeding with soln cost %.5f (target was %.5f)\n",
	4 * KheEjectorCurrDepth(ej) + 2, "",
	KheCostShow(KheSolnCost(soln)), KheCostShow(ej->curr_target_cost));

    ** record success right up the chain **
    for( i = 0;  i < ej->curr_depth;  i++ )
    {
      ea2 = MArrayGet(ej->augment_stack, i);
      ea2->success = true;
    }

    ** reduce limits **
    MArrayForEach(ej->cost_limit_monitors, &m, &i)
      MArrayPut(ej->cost_limit_costs, i, KheMonitorCost(m));

    ** record statistics **
    KheEjectorStatsAddImprovement(ej, ea->augment_type, er->repair_type,
      ej->curr_depth);
    KheEjectorStatsOneRepair(ej, ea->augment_type, er->repair_type, true);

    ** return, or whatever is appropriate for the current solve type **
    switch( ej->curr_major_schedule->solve_type )
    {
      case KHE_EJECTOR_FIRST:

	** ordinary ejection chain, return true immediately **
	ej->curr_di sruption -= di sruption;

	** but first, promote defects if requested and appropriate **
	if( KheOptionsEjectorPromoteDefects(ej->options) &&
	    ej->start_gm != ej->continue_gm )
	{
	  for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
	  {
	    m = KheTraceMonitor(ea->trace, i);
	    if( KheMonitorCost(m) > KheTraceMonitorInitCost(ea->trace, i) &&
		!KheGroupMonitorHasChildMonitor(ej->start_gm, m) )
	      KheGroupMonitorAddChildMonitor(ej->start_gm, m);
	  }
	}
	return true;

      case KHE_EJECTOR_BEST_COST:

	** minimize cost, leave best_d isruption alone at INT_MAX **
	if( KheSolnCost(soln) < ej->best_cost )
	{
	  KheTransaction Copy(ej->curr_transaction, ej->best_transaction);
	  ej->best_cost = KheSolnCost(soln);
	  MAssert(ej->best_d isruption == INT_MAX,
	    "KheEjectorRepairEnd internal error");
	}
	ej->curr_d isruption -= d isruption;
	KheTransaction Undo(er->transaction);
	return false;

      case KHE_EJECTOR_BEST_DISRUPTION:

	** minimize (d isruption, cost) **
	if( ej->curr_d isruption < ej->best_d isruption ||
	    (ej->curr_d isruption == ej->best_d isruption &&
	     KheSolnCost(soln) < ej->best_cost) )
	{
	  ** new best, record it and keep going **
	  if( DEBUG2 )
	  {
	    fprintf(stderr, "  KheEjectorRepairEnd new best (%d, %.5f):\n",
	      ej->curr_d isruption, KheCostShow(KheSolnCost(soln)));
	    KheTransactionDebug(ej->curr_transaction, 2, 2, stderr);
	  }
	  KheTransaction Copy(ej->curr_transaction, ej->best_transaction);
	  ej->best_d isruption = ej->curr_d isruption;
	  ej->best_cost = KheSolnCost(soln);
	}
	ej->curr_d isruption -= d isruption;
	KheTransaction Undo(er->transaction);
	return false;

      default:

	MAssert(false, "KheEjectorRepairEnd internal error (solve type)");
	break;
    }
  }

  ** if we are not up against a depth or d isruption limit, recurse **
  if( ej->curr_depth < ej->curr_minor_schedule->max_depth &&
      ej->curr_d isruption <= ej->curr_minor_schedule->max_d isruption &&
      ej->curr_d isruption <= ej->best_d isruption )
  {
    for( i = 0;  i < KheTraceMonitorCount(ea->trace);  i++ )
    {
      m = KheTraceMonitor(ea->trace, i);
      if( KheMonitorCost(m) > KheTraceMonitorInitCost(ea->trace, i) )
      {
	if( DEBUG8 )
	{
	  fprintf(stderr, "%*strace ", 4 * KheEjectorCurrDepth(ej) + 2, "");
	  KheMonitorDebug(m, 1, 0, stderr);
	}
        max_improvement = KheMonitorCost(m) - KheMonitorLowerBound(m);
	if( KheSolnCost(soln) - max_improvement < ej->curr_target_cost &&
	    KheEjectorAugment(ej, m) )
	{
	  ej->curr_d isruption -= d isruption;
	  KheEjectorStatsOneRepair(ej, ea->augment_type,er->repair_type,true);
	  return true;
	}
      }
    }
  }
  KheEjectorStatsOneRepair(ej, ea->augment_type, er->repair_type, false);
  if( DEBUG6 )
    fprintf(stderr, "%*s-ejector (%s, %d trace monitors)\n",
      2*KheEjectorCurrDepth(ej) + 2, "",
      ej->curr_depth >= ej->curr_minor_schedule->max_depth ? "depth" :
      ej->curr_d isruption > ej->curr_minor_schedule->max_d isruption ? "disr" :
      ej->curr_d isruption > ej->best_d isruption ? "best_disr" : "-",
      KheTraceMonitorCount(ea->trace));
  if( DEBUG8 )
    fprintf(stderr, "%*sfailing %son cost %.5f not less than %.5f\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      ej->curr_depth >= ej->curr_minor_schedule->max_depth ? "(max depth)" : "",
      KheCostShow(KheSolnCost(soln)), KheCostShow(ej->curr_target_cost));
  KheTransaction Undo(er->transaction);
  ej->curr_d isruption -= d isruption;
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "measuring performance (general)"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorInitAllStats(KHE_EJECTOR ej)                              */
/*                                                                           */
/*  Initialize all the statistics of ej.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorInitAllStats(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  MArrayInit(ej->stats.improvement_stats);
  ej->stats.timer = KheStatsTimerMake();
  ej->stats.init_cost = 0;
  ej->stats.init_defects = 0;
  if( DEBUG7 )
    fprintf(stderr, "  %p histo init\n", (void *) ej);
  MArrayInit(ej->stats.repair_count_histo);
  MArrayInit(ej->stats.augment_count_histo);
  MArrayInit(ej->stats.repair_stats);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorStatsDelete(KHE_EJECTOR ej)                               */
/*                                                                           */
/*  Free the memory consumed by ej's stats.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorStatsDelete(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  KHE_REPAIR_STATS sr;  int i;
  MArrayFree(ej->stats.improvement_stats);
  MArrayFree(ej->stats.repair_count_histo);
  MArrayFree(ej->stats.augment_count_histo);
  MArrayForEach(ej->stats.repair_stats, &sr, &i)
    MArrayFree(sr.by_type);
  MArrayFree(ej->stats.repair_stats);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorResetSolveStats(KHE_EJECTOR ej)                           */
/*                                                                           */
/*  Reset those statistics of ej concerned with an individual solve.         */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorResetSolveStats(KHE_EJECTOR ej, KHE_SOLN soln)
{
#if KHE_EJECTOR_WITH_STATS
  MArrayClear(ej->stats.improvement_stats);
  KheStatsTimerReset(ej->stats.timer);
  ej->stats.init_cost = KheSolnCost(soln);
  ej->stats.init_defects = KheGroupMonitorDefectCount(ej->start_gm);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorStatsOneRepair(KHE_EJECTOR ej, int augment_type,          */
/*    int repair_type, bool success)                                         */
/*                                                                           */
/*  This function should be called each time a repair is made (each time     */
/*  KheEjectorRepairEnd is called), with success == true if the repair       */
/*  causes the enclosing call to KheEjectorAugment to return true.           */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorStatsOneRepair(KHE_EJECTOR ej, int augment_type,
  int repair_type, bool success)
{
#if KHE_EJECTOR_WITH_STATS
  KHE_REPAIR_STATS repair_stats, *rs;  KHE_PAIR pair, *ps;

  /* make sure there is an entry in repairs for augment_type */
  while( MArraySize(ej->stats.repair_stats) <= augment_type )
  {
    repair_stats.overall.total = 0;
    repair_stats.overall.successful = 0;
    MArrayInit(repair_stats.by_type);
    MArrayAddLast(ej->stats.repair_stats, repair_stats);
  }

  /* update the repair record for augment_type */
  rs = &MArrayGet(ej->stats.repair_stats, augment_type);
  rs->overall.total++;
  if( success )
    rs->overall.successful++;

  /* make sure there is an entry in rs for repair_type */
  while( MArraySize(rs->by_type) <= repair_type )
  {
    pair.total = 0;
    pair.successful = 0;
    MArrayAddLast(rs->by_type, pair);
  }

  /* update the repair record for repair_type */
  ps = &MArrayGet(rs->by_type, repair_type);
  ps->total++;
  if( success )
    ps->successful++;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorStatsAddImprovement(KHE_EJECTOR ej, int repair_count)     */
/*                                                                           */
/*  This function should be called each time an overall improvement is       */
/*  found.  Here repair_count is the number of repairs in the improvement.   */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorStatsAddImprovement(KHE_EJECTOR ej, int repair_count)
{
#if KHE_EJECTOR_WITH_STATS
  KHE_EJECTOR_IMPROVEMENT_STATS improvement_stats;  /* int chain_length; */
  /* chain_length = MArraySize(ej->augment_stack); */
  MAssert(repair_count >= 1, "KheEjectorStatsSuccessfulEnd internal error 1");

  /* store sequence info about this chain */
  improvement_stats.repair_count = repair_count;
  improvement_stats.time = KheStatsTimerNow(ej->stats.timer);
  improvement_stats.cost = KheSolnCost(KheEjectorSoln(ej));
  improvement_stats.defects = KheGroupMonitorDefectCount(ej->start_gm);
  MArrayAddLast(ej->stats.improvement_stats, improvement_stats);

  /* update repair_count_histo */
  while( MArraySize(ej->stats.repair_count_histo) < repair_count )
    MArrayAddLast(ej->stats.repair_count_histo, 0);
  if( DEBUG7 )
    fprintf(stderr, "  %p histo[%d]++\n", (void *) ej, repair_count - 1);
  MArrayPreInc(ej->stats.repair_count_histo, repair_count - 1);

  /* update augment_count_histo */
  while( MArraySize(ej->stats.augment_count_histo) < ej->curr_augment_count )
    MArrayAddLast(ej->stats.augment_count_histo, 0);
  MArrayPreInc(ej->stats.augment_count_histo, ej->curr_augment_count - 1);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementCount(KHE_EJECTOR ej)                           */
/*                                                                           */
/*  Return the number of improvements.                                       */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementCount(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  return MArraySize(ej->stats.improvement_stats);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementNumberOfRepairs(KHE_EJECTOR ej, int i)          */
/*                                                                           */
/*  Return the number of repairs of the i'th improvement.                    */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementNumberOfRepairs(KHE_EJECTOR ej, int i)
{
#if KHE_EJECTOR_WITH_STATS
  return MArrayGet(ej->stats.improvement_stats, i).repair_count;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheEjectorImprovementTime(KHE_EJECTOR ej, int i)                   */
/*                                                                           */
/*  Return the elapsed time between when KheEjectorSolve began and when the  */
/*  i'th improvement was applied.                                            */
/*                                                                           */
/*****************************************************************************/

float KheEjectorImprovementTime(KHE_EJECTOR ej, int i)
{
#if KHE_EJECTOR_WITH_STATS
  return MArrayGet(ej->stats.improvement_stats, i).time;
#else
  return 0.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEjectorImprovementCost(KHE_EJECTOR ej, int i)                */
/*                                                                           */
/*  Return the solution cost after the i'th improvement.                     */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEjectorImprovementCost(KHE_EJECTOR ej, int i)
{
#if KHE_EJECTOR_WITH_STATS
  return MArrayGet(ej->stats.improvement_stats, i).cost;
#else
  return KheCost(0, 0);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementDefects(KHE_EJECTOR ej, int i)                  */
/*                                                                           */
/*  Return the number of main loop defects after the i'th improvement.       */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementDefects(KHE_EJECTOR ej, int i)
{
#if KHE_EJECTOR_WITH_STATS
  return MArrayGet(ej->stats.improvement_stats, i).defects;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEjectorInitCost(KHE_EJECTOR ej)                              */
/*                                                                           */
/*  Return the initial cost.                                                 */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEjectorInitCost(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  return ej->stats.init_cost;
#else
  return KheCost(0, 0);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorInitDefects(KHE_EJECTOR ej)                                */
/*                                                                           */
/*  Return the initial number of defects.                                    */
/*                                                                           */
/*****************************************************************************/

int KheEjectorInitDefects(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  return ej->stats.init_defects;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementRepairHistoMax(KHE_EJECTOR ej)                  */
/*                                                                           */
/*  Return the maximum number of repairs in any improvement, or 0 if         */
/*  there have been no improvements.                                         */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementRepairHistoMax(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  if( DEBUG7 )
    fprintf(stderr, "  %p histo max %d\n", (void *) ej,
      MArraySize(ej->stats.repair_count_histo));
  return MArraySize(ej->stats.repair_count_histo);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementRepairHistoFrequency(KHE_EJECTOR ej,            */
/*    int repair_count)                                                      */
/*                                                                           */
/*  Return the number of improvements with the given number of repairs.      */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementRepairHistoFrequency(KHE_EJECTOR ej, int repair_count)
{
#if KHE_EJECTOR_WITH_STATS
  MAssert(repair_count >= 1,
    "KheEjectorImprovementRepairHistoFrequency: repair_count < 1");
  MAssert(repair_count <= MArraySize(ej->stats.repair_count_histo),
    "KheEjectorImprovementRepairHistoFrequency: "
    "repair_count > KheEjectorImprovementRepairHistoMax(ej)");
  return MArrayGet(ej->stats.repair_count_histo, repair_count - 1);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementRepairHistoTotal(KHE_EJECTOR ej)                */
/*                                                                           */
/*  Return the total number of improvements.                                 */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementRepairHistoTotal(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  int no_of_chains, i, num;
  no_of_chains = 0;
  MArrayForEach(ej->stats.repair_count_histo, &num, &i)
    no_of_chains += num;
  return no_of_chains;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheEjectorImprovementRepairHistoAverage(KHE_EJECTOR ej)            */
/*                                                                           */
/*  Return the average number of repairs of successful improvements.         */
/*                                                                           */
/*****************************************************************************/

float KheEjectorImprovementRepairHistoAverage(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  int no_of_chains, total_length_of_chains, i, num;
  no_of_chains = 0; total_length_of_chains = 0;
  MArrayForEach(ej->stats.repair_count_histo, &num, &i)
  {
    no_of_chains += num;
    total_length_of_chains += num * (i + 1);
  }
  MAssert(no_of_chains > 0, "KheEjectorChainHistoAverage: no chains");
  return (float) total_length_of_chains / (float) no_of_chains;
#else
  return 0.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementAugmentHistoMax(KHE_EJECTOR ej)                 */
/*                                                                           */
/*  Return the maximum number of augments in any improvement, or 0 if        */
/*  there have been no improvements.                                         */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementAugmentHistoMax(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  return MArraySize(ej->stats.augment_count_histo);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementAugmentHistoFrequency(KHE_EJECTOR ej,           */
/*    int repair_count)                                                      */
/*                                                                           */
/*  Return the number of improvements with the given number of augments.     */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementAugmentHistoFrequency(KHE_EJECTOR ej,
  int augment_count)
{
#if KHE_EJECTOR_WITH_STATS
  MAssert(augment_count >= 1,
    "KheEjectorImprovementAugmentHistoFrequency: repair_count < 1");
  MAssert(augment_count <= MArraySize(ej->stats.augment_count_histo),
    "KheEjectorImprovementAugmentHistoFrequency: "
    "augment_count > KheEjectorImprovementAugmentHistoMax(ej)");
  return MArrayGet(ej->stats.augment_count_histo, augment_count - 1);
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorImprovementAugmentHistoTotal(KHE_EJECTOR ej)               */
/*                                                                           */
/*  Return the total number of improvements.                                 */
/*                                                                           */
/*****************************************************************************/

int KheEjectorImprovementAugmentHistoTotal(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  int no_of_chains, i, num;
  no_of_chains = 0;
  MArrayForEach(ej->stats.augment_count_histo, &num, &i)
    no_of_chains += num;
  return no_of_chains;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheEjectorImprovementAugmentHistoAverage(KHE_EJECTOR ej)           */
/*                                                                           */
/*  Return the average number of augments of successful improvements.        */
/*                                                                           */
/*****************************************************************************/

float KheEjectorImprovementAugmentHistoAverage(KHE_EJECTOR ej)
{
#if KHE_EJECTOR_WITH_STATS
  int no_of_chains, total_length_of_chains, i, num;
  no_of_chains = 0; total_length_of_chains = 0;
  MArrayForEach(ej->stats.augment_count_histo, &num, &i)
  {
    no_of_chains += num;
    total_length_of_chains += num * (i + 1);
  }
  MAssert(no_of_chains > 0, "KheEjectorChainAugmentHistoAverage: no chains");
  return (float) total_length_of_chains / (float) no_of_chains;
#else
  return 0.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorTotalRepairs(KHE_EJECTOR ej, int augment_type)             */
/*                                                                           */
/*  Return the total number of augment_type repairs.                         */
/*                                                                           */
/*****************************************************************************/

int KheEjectorTotalRepairs(KHE_EJECTOR ej, int augment_type)
{
#if KHE_EJECTOR_WITH_STATS
  return MArraySize(ej->stats.repair_stats) <= augment_type ? 0 :
    MArrayGet(ej->stats.repair_stats, augment_type).overall.total;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorSuccessfulRepairs(KHE_EJECTOR ej, int augment_type)        */
/*                                                                           */
/*  Return the number of successful augment_type repairs.                    */
/*                                                                           */
/*****************************************************************************/

int KheEjectorSuccessfulRepairs(KHE_EJECTOR ej, int augment_type)
{
#if KHE_EJECTOR_WITH_STATS
  return MArraySize(ej->stats.repair_stats) <= augment_type ? 0 :
    MArrayGet(ej->stats.repair_stats, augment_type).overall.successful;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorTotalRepairsByType(KHE_EJECTOR ej, int augment_type,       */
/*    int repair_type)                                                       */
/*                                                                           */
/*  Return the number of (augment_type, repair_type) pairs.                  */
/*                                                                           */
/*****************************************************************************/

int KheEjectorTotalRepairsByType(KHE_EJECTOR ej, int augment_type,
  int repair_type)
{
#if KHE_EJECTOR_WITH_STATS
  KHE_REPAIR_STATS rs;
  if( MArraySize(ej->stats.repair_stats) <= augment_type )
    return 0;
  rs = MArrayGet(ej->stats.repair_stats, augment_type);
  if( MArraySize(rs.by_type) <= repair_type )
    return 0;
  return MArrayGet(rs.by_type, repair_type).total;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorSuccessfulRepairsByType(KHE_EJECTOR ej, int augment_type,  */
/*    int repair_type)                                                       */
/*                                                                           */
/*  Return the number of successful (augment_type, repair_type) pairs.       */
/*                                                                           */
/*****************************************************************************/

int KheEjectorSuccessfulRepairsByType(KHE_EJECTOR ej, int augment_type,
  int repair_type)
{
#if KHE_EJECTOR_WITH_STATS
  KHE_REPAIR_STATS rs;
  if( MArraySize(ej->stats.repair_stats) <= augment_type )
    return 0;
  rs = MArrayGet(ej->stats.repair_stats, augment_type);
  if( MArraySize(rs.by_type) <= repair_type )
    return 0;
  return MArrayGet(rs.by_type, repair_type).successful;
#else
  return 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "measuring performance (augment info and augment types)"       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_AUGMENT_INFO KheAugmentInfoMake(int augment_type,                    */
/*    char *augment_label)                                                   */
/*                                                                           */
/*  Make a new augment_info object with these attributes.                    */
/*                                                                           */
/*****************************************************************************/

static KHE_AUGMENT_INFO KheAugmentInfoMake(int augment_type,
  char *augment_label)
{
  KHE_AUGMENT_INFO res;
  MAssert(augment_label != NULL,
    "KheAugmentInfoMake: augment_label == NULL");
  MMake(res);
  res->augment_type = augment_type;
  res->augment_label = MStringCopy(augment_label);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAugmentInfoDelete(KHE_AUGMENT_INFO ai)                           */
/*                                                                           */
/*  Delete ai, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheAugmentInfoDelete(KHE_AUGMENT_INFO ai)
{
  MFree(ai->augment_label);
  MFree(ai);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAugmentInfoFind(ARRAY_KHE_AUGMENT_INFO *aai,                     */
/*    int augment_type, KHE_AUGMENT_INFO *ai)                                */
/*                                                                           */
/*  If *aai contains a repair info object with the given repair_type,        */
/*  return true and set *ai to that object.  Otherwise return false.         */
/*                                                                           */
/*****************************************************************************/

static bool KheAugmentInfoFind(ARRAY_KHE_AUGMENT_INFO *aai,
  int augment_type, KHE_AUGMENT_INFO *ai)
{
  KHE_AUGMENT_INFO ai2;  int i;
  MArrayForEach(*aai, &ai2, &i)
    if( ai2->augment_type == augment_type )
    {
      *ai = ai2;
      return true;
    }
  *ai = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddAugmentType(KHE_EJECTOR ej, int augment_type,          */
/*    char *augment_label)                                                   */
/*                                                                           */
/*  Declare an augment type.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddAugmentType(KHE_EJECTOR ej, int augment_type,
  char *augment_label)
{
  KHE_AUGMENT_INFO res;
  MAssert(ej->state == KHE_EJECTOR_MAKE_SETTING,
    "KheEjectorAddAugmentType called out of order");
  MAssert( !KheAugmentInfoFind(&ej->augment_info_array, augment_type,
    &res), "KheEjectorAddAugmentType: augment_type already declared");
  res = KheAugmentInfoMake(augment_type, augment_label);
  MArrayAddLast(ej->augment_info_array, res);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorAugmentTypeCount(KHE_EJECTOR ej)                           */
/*                                                                           */
/*  Return the number of declared augment types.                             */
/*                                                                           */
/*****************************************************************************/

int KheEjectorAugmentTypeCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->augment_info_array);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorAugmentType(KHE_EJECTOR ej, int i)                         */
/*                                                                           */
/*  Return the i'th declared augment type.                                   */
/*                                                                           */
/*****************************************************************************/

int KheEjectorAugmentType(KHE_EJECTOR ej, int i)
{
  KHE_AUGMENT_INFO ai;
  ai = MArrayGet(ej->augment_info_array, i);
  return ai->augment_type;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEjectorAugmentTypeLabel(KHE_EJECTOR ej, int augment_type)       */
/*                                                                           */
/*  Return the label of augment_type.                                        */
/*                                                                           */
/*****************************************************************************/

char *KheEjectorAugmentTypeLabel(KHE_EJECTOR ej, int augment_type)
{
  KHE_AUGMENT_INFO ai;
  if( !KheAugmentInfoFind(&ej->augment_info_array, augment_type, &ai) )
    MAssert(false, "KheAugmentTypeLabel: augment_type not declared");
  return ai->augment_label;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "measuring performance (repair info and repair types)"         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_REPAIR_INFO KheRepairInfoMake(int repair_type, char *repair_label)   */
/*                                                                           */
/*  Make a new repair_info object with these attributes.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_REPAIR_INFO KheRepairInfoMake(int repair_type, char *repair_label)
{
  KHE_REPAIR_INFO res;
  MAssert(repair_label != NULL,
    "KheRepairInfoMake: repair_label == NULL");
  MMake(res);
  res->repair_type = repair_type;
  res->repair_label = MStringCopy(repair_label);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRepairInfoDelete(KHE_REPAIR_INFO ri)                             */
/*                                                                           */
/*  Delete ri, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheRepairInfoDelete(KHE_REPAIR_INFO ri)
{
  MFree(ri->repair_label);
  MFree(ri);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRepairInfoFind(ARRAY_KHE_REPAIR_INFO *ari, int repair_type,      */
/*    KHE_REPAIR_INFO *ri)                                                   */
/*                                                                           */
/*  If *ari contains a repair info object with the given repair_type,        */
/*  return true and set *ri to that object.  Otherwise return false.         */
/*                                                                           */
/*****************************************************************************/

static bool KheRepairInfoFind(ARRAY_KHE_REPAIR_INFO *ari, int repair_type,
  KHE_REPAIR_INFO *ri)
{
  KHE_REPAIR_INFO ri2;  int i;
  MArrayForEach(*ari, &ri2, &i)
    if( ri2->repair_type == repair_type )
    {
      *ri = ri2;
      return true;
    }
  *ri = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddRepairType(KHE_EJECTOR ej, int repair_type,            */
/*    char *repair_label)                                                    */
/*                                                                           */
/*  Declare a repair type.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddRepairType(KHE_EJECTOR ej, int repair_type,
  char *repair_label)
{
  KHE_REPAIR_INFO res;
  MAssert(ej->state == KHE_EJECTOR_MAKE_SETTING,
    "KheEjectorAddRepairType called out of order");
  MAssert( !KheRepairInfoFind(&ej->repair_info_array, repair_type, &res),
    "KheEjectorAddRepairType: repair_type already declared");
  res = KheRepairInfoMake(repair_type, repair_label);
  MArrayAddLast(ej->repair_info_array, res);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorRepairTypeCount(KHE_EJECTOR ej)                            */
/*                                                                           */
/*  Return the number of declared repair types.                              */
/*                                                                           */
/*****************************************************************************/

int KheEjectorRepairTypeCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->repair_info_array);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorRepairType(KHE_EJECTOR ej, int i)                          */
/*                                                                           */
/*  Return the i'th declared repair type.                                    */
/*                                                                           */
/*****************************************************************************/

int KheEjectorRepairType(KHE_EJECTOR ej, int i)
{
  KHE_REPAIR_INFO ri;
  ri = MArrayGet(ej->repair_info_array, i);
  return ri->repair_type;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEjectorRepairTypeLabel(KHE_EJECTOR ej, int repair_type)         */
/*                                                                           */
/*  Return the label of repair_type.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheEjectorRepairTypeLabel(KHE_EJECTOR ej, int repair_type)
{
  KHE_REPAIR_INFO ri;
  if( !KheRepairInfoFind(&ej->repair_info_array, repair_type, &ri) )
    MAssert(false, "KheEjectorRepairTypeLabel: repair_type not declared");
  return ri->repair_label;
}
