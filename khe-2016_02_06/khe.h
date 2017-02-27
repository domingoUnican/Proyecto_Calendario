
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
/*  FILE:         khe.h                                                      */
/*  DESCRIPTION:  Include this file whenever you use KHE                     */
/*                                                                           */
/*  This file has two parts:  type declarations and function declarations.   */
/*  Each part is organized to parallel the User's Guide exactly:             */
/*                                                                           */
/*                    Part A:  The Platform                                  */
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*    Chapter 2.   Archives and Solution Groups                              */
/*    Chapter 3.   Instances                                                 */
/*    Chapter 4.   Solutions                                                 */
/*    Chapter 5.   Extra Types for Solving                                   */
/*    Chapter 6.   Solution Monitoring                                       */
/*    Chapter 7.   Matchings and Evenness                                    */
/*                                                                           */
/*                    Part B:  Solving                                       */
/*                                                                           */
/*    Chapter 8.   Introducing Solving                                       */
/*    Chapter 9.   Structural Solvers                                        */
/*    Chapter 10.  Time Solvers                                              */
/*    Chapter 11.  Resource Solvers                                          */
/*    Chapter 12.  Ejection Chains                                           */
/*                                                                           */
/*    Appendix A.  Modules Packaged with KHE                                 */
/*    Appendix B.  Implementation Notes                                      */
/*                                                                           */
/*  This makes it easy to verify that KHE offers what the User's Guide says  */
/*  it offers.  See files m.h, khe_lset.h, khe_priqueue.h, and kml.h for     */
/*  the functions documented in Appendix A.                                  */
/*                                                                           */
/*****************************************************************************/
#ifndef KHE_HEADER_FILE
#define KHE_HEADER_FILE

#define	KHE_VERSION   "2016_02_06"
#define KHE_USE_PTHREAD 1
#define KHE_USE_TIMING 1
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "kml.h"

#define KHE_ANY_DURATION 0
#define KHE_COST_SHOW_DIGITS 99999


/*****************************************************************************/
/*                                                                           */
/*                        TYPE DECLARATIONS                                  */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*                    Part A: The Platform                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*    Chapter 2.   Archives and Solution Groups                              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_archive_rec *KHE_ARCHIVE;
typedef struct khe_archive_metadata_rec *KHE_ARCHIVE_METADATA;
typedef struct khe_soln_group_rec *KHE_SOLN_GROUP;
typedef struct khe_soln_group_metadata_rec *KHE_SOLN_GROUP_METADATA;
typedef struct khe_soln_rec *KHE_SOLN;
typedef void (*KHE_ARCHIVE_FN)(KHE_ARCHIVE archive, void *impl);
typedef void (*KHE_SOLN_GROUP_FN)(KHE_SOLN_GROUP soln_group, void *impl);
typedef void (*KHE_SOLN_FN)(KHE_SOLN soln, void *impl);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 3.   Instances                                                 */
/*                                                                           */
/*****************************************************************************/

/* 3.1 Creating instances */
typedef struct khe_instance_rec *KHE_INSTANCE;
typedef struct khe_instance_metadata_rec *KHE_INSTANCE_METADATA;

/* 3.4 Times */
typedef enum {
  KHE_TIME_GROUP_KIND_ORDINARY,
  KHE_TIME_GROUP_KIND_WEEK,
  KHE_TIME_GROUP_KIND_DAY,
  /* KHE_TIME_GROUP_KIND_PREDEFINED */
} KHE_TIME_GROUP_KIND;

typedef struct khe_time_group_rec *KHE_TIME_GROUP;
typedef struct khe_time_rec *KHE_TIME;

/* 3.5 Resources */
typedef struct khe_resource_type_rec *KHE_RESOURCE_TYPE;
typedef struct khe_resource_group_rec *KHE_RESOURCE_GROUP;
typedef struct khe_resource_rec *KHE_RESOURCE;

/* 3.6 Events */
typedef enum {
  KHE_EVENT_GROUP_KIND_COURSE,
  KHE_EVENT_GROUP_KIND_ORDINARY
} KHE_EVENT_GROUP_KIND;

typedef struct khe_event_group_rec *KHE_EVENT_GROUP;
typedef struct khe_event_rec *KHE_EVENT;
typedef struct khe_event_resource_rec *KHE_EVENT_RESOURCE;
typedef struct khe_event_resource_group_rec *KHE_EVENT_RESOURCE_GROUP;

/* 3.7 Constraints */
typedef struct khe_constraint_rec *KHE_CONSTRAINT;

typedef enum {
  /* KHE_SUM_STEPS_COST_FUNCTION, */
  /* KHE_STEP_SUM_COST_FUNCTION, */
  KHE_STEP_COST_FUNCTION,
  /* KHE_SUM_COST_FUNCTION, */
  KHE_LINEAR_COST_FUNCTION,
  /* KHE_SUM_SQUARES_COST_FUNCTION, */
  /* KHE_SQUARE_SUM_COST_FUNCTION, */
  KHE_QUADRATIC_COST_FUNCTION
} KHE_COST_FUNCTION;

typedef enum {
  KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG,
  KHE_ASSIGN_TIME_CONSTRAINT_TAG,
  KHE_SPLIT_EVENTS_CONSTRAINT_TAG,
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG,
  KHE_PREFER_RESOURCES_CONSTRAINT_TAG,
  KHE_PREFER_TIMES_CONSTRAINT_TAG,
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG,
  KHE_SPREAD_EVENTS_CONSTRAINT_TAG,
  KHE_LINK_EVENTS_CONSTRAINT_TAG,
  KHE_ORDER_EVENTS_CONSTRAINT_TAG,
  KHE_AVOID_CLASHES_CONSTRAINT_TAG,
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG,
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG,
  KHE_CONSTRAINT_TAG_COUNT
} KHE_CONSTRAINT_TAG;

typedef struct khe_limited_time_group_rec *KHE_LIMITED_TIME_GROUP;
typedef struct khe_time_spread_rec *KHE_TIME_SPREAD;

typedef struct khe_assign_resource_constraint_rec
  *KHE_ASSIGN_RESOURCE_CONSTRAINT;
typedef struct khe_assign_time_constraint_rec
  *KHE_ASSIGN_TIME_CONSTRAINT;
typedef struct khe_split_events_constraint_rec
  *KHE_SPLIT_EVENTS_CONSTRAINT;
typedef struct khe_distribute_split_events_constraint_rec
  *KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT;
typedef struct khe_prefer_resources_constraint_rec
  *KHE_PREFER_RESOURCES_CONSTRAINT;
typedef struct khe_prefer_times_constraint_rec
  *KHE_PREFER_TIMES_CONSTRAINT;
typedef struct khe_avoid_split_assignments_constraint_rec
  *KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT;
typedef struct khe_spread_events_constraint_rec
  *KHE_SPREAD_EVENTS_CONSTRAINT;
typedef struct khe_link_events_constraint_rec
  *KHE_LINK_EVENTS_CONSTRAINT;
typedef struct khe_order_events_constraint_rec
  *KHE_ORDER_EVENTS_CONSTRAINT;
typedef struct khe_avoid_clashes_constraint_rec
  *KHE_AVOID_CLASHES_CONSTRAINT;
typedef struct khe_avoid_unavailable_times_constraint_rec
  *KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT;
typedef struct khe_limit_idle_times_constraint_rec
  *KHE_LIMIT_IDLE_TIMES_CONSTRAINT;
typedef struct khe_cluster_busy_times_constraint_rec
  *KHE_CLUSTER_BUSY_TIMES_CONSTRAINT;
typedef struct khe_limit_busy_times_constraint_rec
  *KHE_LIMIT_BUSY_TIMES_CONSTRAINT;
typedef struct khe_limit_workload_constraint_rec
  *KHE_LIMIT_WORKLOAD_CONSTRAINT;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 4.   Solutions                                                 */
/*                                                                           */
/*****************************************************************************/

typedef int64_t KHE_COST;
#define KheCostMax INT64_MAX
typedef struct khe_meet_rec *KHE_MEET;
/* typedef struct khe_meet_bound_group_rec *KHE_MEET_BOUND_GROUP; */
typedef struct khe_meet_bound_rec *KHE_MEET_BOUND;
typedef struct khe_task_rec *KHE_TASK;
typedef struct khe_task_bound_rec *KHE_TASK_BOUND;
typedef struct khe_mark_rec *KHE_MARK;
typedef struct khe_path_rec *KHE_PATH;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 5.   Extra Types for Solving                                   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_rec *KHE_NODE;
typedef struct khe_layer_rec *KHE_LAYER;
typedef struct khe_zone_rec *KHE_ZONE;
typedef struct khe_tasking_rec *KHE_TASKING;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 6.   Solution Monitoring                                       */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_ASSIGN_RESOURCE_MONITOR_TAG,
  KHE_ASSIGN_TIME_MONITOR_TAG,
  KHE_SPLIT_EVENTS_MONITOR_TAG,
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG,
  KHE_PREFER_RESOURCES_MONITOR_TAG,
  KHE_PREFER_TIMES_MONITOR_TAG,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG,
  KHE_SPREAD_EVENTS_MONITOR_TAG,
  KHE_LINK_EVENTS_MONITOR_TAG,
  KHE_ORDER_EVENTS_MONITOR_TAG,
  KHE_AVOID_CLASHES_MONITOR_TAG,
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG,
  KHE_LIMIT_IDLE_TIMES_MONITOR_TAG,
  KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG,
  KHE_LIMIT_BUSY_TIMES_MONITOR_TAG,
  KHE_LIMIT_WORKLOAD_MONITOR_TAG,
  KHE_TIMETABLE_MONITOR_TAG,
  KHE_TIME_GROUP_MONITOR_TAG,
  KHE_ORDINARY_DEMAND_MONITOR_TAG,
  KHE_WORKLOAD_DEMAND_MONITOR_TAG,
  KHE_EVENNESS_MONITOR_TAG,
  KHE_GROUP_MONITOR_TAG,
  KHE_MONITOR_TAG_COUNT
} KHE_MONITOR_TAG;

typedef struct khe_monitor_rec *KHE_MONITOR;

typedef struct khe_assign_resource_monitor_rec
  *KHE_ASSIGN_RESOURCE_MONITOR;
typedef struct khe_assign_time_monitor_rec
  *KHE_ASSIGN_TIME_MONITOR;
typedef struct khe_split_events_monitor_rec
  *KHE_SPLIT_EVENTS_MONITOR;
typedef struct khe_distribute_split_events_monitor_rec
  *KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR;
typedef struct khe_prefer_resources_monitor_rec
  *KHE_PREFER_RESOURCES_MONITOR;
typedef struct khe_prefer_times_monitor_rec
  *KHE_PREFER_TIMES_MONITOR;
typedef struct khe_avoid_split_assignments_monitor_rec
  *KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR;
typedef struct khe_spread_events_monitor_rec
  *KHE_SPREAD_EVENTS_MONITOR;
typedef struct khe_link_events_monitor_rec
  *KHE_LINK_EVENTS_MONITOR;
typedef struct khe_order_events_monitor_rec
  *KHE_ORDER_EVENTS_MONITOR;
typedef struct khe_avoid_clashes_monitor_rec
  *KHE_AVOID_CLASHES_MONITOR;
typedef struct khe_avoid_unavailable_times_monitor_rec
  *KHE_AVOID_UNAVAILABLE_TIMES_MONITOR;
typedef struct khe_limit_idle_times_monitor_rec
  *KHE_LIMIT_IDLE_TIMES_MONITOR;
typedef struct khe_cluster_busy_times_monitor_rec
  *KHE_CLUSTER_BUSY_TIMES_MONITOR;
typedef struct khe_limit_busy_times_monitor_rec
  *KHE_LIMIT_BUSY_TIMES_MONITOR;
typedef struct khe_limit_workload_monitor_rec
  *KHE_LIMIT_WORKLOAD_MONITOR;
typedef struct khe_timetable_monitor_rec
  *KHE_TIMETABLE_MONITOR;
typedef struct khe_time_group_monitor_rec
  *KHE_TIME_GROUP_MONITOR;
typedef struct khe_group_monitor_rec
  *KHE_GROUP_MONITOR;

typedef struct khe_trace_rec *KHE_TRACE;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 7.   Matchings and Evenness                                    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ordinary_demand_monitor_rec
  *KHE_ORDINARY_DEMAND_MONITOR;

typedef struct khe_workload_demand_monitor_rec
  *KHE_WORKLOAD_DEMAND_MONITOR;

typedef enum {
  KHE_MATCHING_TYPE_EVAL_INITIAL,
  KHE_MATCHING_TYPE_EVAL_TIMES,
  KHE_MATCHING_TYPE_EVAL_RESOURCES,
  KHE_MATCHING_TYPE_SOLVE
} KHE_MATCHING_TYPE;

typedef struct khe_evenness_monitor_rec *KHE_EVENNESS_MONITOR;


/*****************************************************************************/
/*                                                                           */
/*                    Part B:  Solving                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 8.  Introducing Solving                                        */
/*                                                                           */
/*****************************************************************************/
 
typedef struct khe_options_rec *KHE_OPTIONS;

typedef enum {
  KHE_OPTIONS_KEMPE_NO,
  KHE_OPTIONS_KEMPE_AUTO,
  KHE_OPTIONS_KEMPE_YES
} KHE_OPTIONS_KEMPE;

typedef enum {
  KHE_OPTIONS_RESOURCE_PAIR_NONE,
  KHE_OPTIONS_RESOURCE_PAIR_SPLITS,
  KHE_OPTIONS_RESOURCE_PAIR_PARTITIONS,
  KHE_OPTIONS_RESOURCE_PAIR_ALL
} KHE_OPTIONS_RESOURCE_PAIR;

typedef KHE_SOLN (*KHE_GENERAL_SOLVER)(KHE_SOLN soln, KHE_OPTIONS options);

typedef enum {
  KHE_STATS_TABLE_PLAIN,
  KHE_STATS_TABLE_LOUT,
  KHE_STATS_TABLE_LATEX
} KHE_STATS_TABLE_TYPE;

typedef enum {
  KHE_STATS_DATASET_HISTO
} KHE_STATS_DATASET_TYPE;

typedef struct khe_stats_timer_rec *KHE_STATS_TIMER;

typedef enum {
  KHE_BACKOFF_NONE,
  KHE_BACKOFF_EXPONENTIAL,
} KHE_BACKOFF_TYPE;

typedef struct khe_backoff_rec *KHE_BACKOFF;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 9.   Structural Solvers                                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_time_equiv_rec *KHE_TIME_EQUIV;

typedef enum {
  KHE_SUBTAG_SPLIT_EVENTS,	      /* "SplitEventsGroupMonitor"           */
  KHE_SUBTAG_DISTRIBUTE_SPLIT_EVENTS, /* "DistributeSplitEventsGroupMonitor" */
  KHE_SUBTAG_ASSIGN_TIME,	      /* "AssignTimeGroupMonitor"            */
  KHE_SUBTAG_PREFER_TIMES,	      /* "PreferTimesGroupMonitor"           */
  KHE_SUBTAG_SPREAD_EVENTS,	      /* "SpreadEventsGroupMonitor"          */
  KHE_SUBTAG_LINK_EVENTS,	      /* "LinkEventsGroupMonitor"            */
  KHE_SUBTAG_ORDER_EVENTS,	      /* "OrderEventsGroupMonitor"           */
  KHE_SUBTAG_ASSIGN_RESOURCE,	      /* "AssignResourceGroupMonitor"        */
  KHE_SUBTAG_PREFER_RESOURCES,	      /* "PreferResourcesGroupMonitor"       */
  KHE_SUBTAG_AVOID_SPLIT_ASSIGNMENTS, /* "AvoidSplitAssignmentsGroupMonitor" */
  KHE_SUBTAG_AVOID_CLASHES,	      /* "AvoidClashesGroupMonitor"          */
  KHE_SUBTAG_AVOID_UNAVAILABLE_TIMES, /* "AvoidUnavailableTimesGroupMonitor" */
  KHE_SUBTAG_LIMIT_IDLE_TIMES,	      /* "LimitIdleTimesGroupMonitor"        */
  KHE_SUBTAG_CLUSTER_BUSY_TIMES,      /* "ClusterBusyTimesGroupMonitor"      */
  KHE_SUBTAG_LIMIT_BUSY_TIMES,	      /* "LimitBusyTimesGroupMonitor"        */
  KHE_SUBTAG_LIMIT_WORKLOAD,	      /* "LimitWorkloadGroupMonitor"         */
  KHE_SUBTAG_ORDINARY_DEMAND,	      /* "OrdinaryDemandGroupMonitor"        */
  KHE_SUBTAG_WORKLOAD_DEMAND,	      /* "WorkloadDemandGroupMonitor"        */
  KHE_SUBTAG_KEMPE_DEMAND,	      /* "KempeDemandGroupMonitor"           */
  KHE_SUBTAG_NODE_TIME_REPAIR,	      /* "NodeTimeRepairGroupMonitor"        */
  KHE_SUBTAG_LAYER_TIME_REPAIR,	      /* "LayerTimeRepairGroupMonitor"       */
  KHE_SUBTAG_TASKING,		      /* "TaskingGroupMonitor"               */
  KHE_SUBTAG_ALL_DEMAND		      /* "AllDemandGroupMonitor"             */
} KHE_SUBTAG_STANDARD_TYPE;

typedef struct khe_split_analyser_rec *KHE_SPLIT_ANALYSER;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 10.   Time Solvers                                             */
/*                                                                           */
/*****************************************************************************/

typedef bool (*KHE_NODE_TIME_SOLVER)(KHE_NODE parent_node, KHE_OPTIONS options);
typedef bool (*KHE_LAYER_TIME_SOLVER)(KHE_LAYER layer, KHE_OPTIONS options);

typedef struct khe_kempe_stats_rec *KHE_KEMPE_STATS;
typedef struct khe_meet_bound_group_rec *KHE_MEET_BOUND_GROUP;

/* typedef struct khe_layer_match_rec *KHE_LAYER_MATCH; */
typedef struct khe_layer_asst_rec *KHE_LAYER_ASST;

typedef struct khe_meet_set_solver_rec *KHE_MEET_SET_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 11.  Resource Solvers                                          */
/*                                                                           */
/*****************************************************************************/

typedef bool (*KHE_TASKING_SOLVER)(KHE_TASKING tasking, KHE_OPTIONS options);

typedef struct khe_task_bound_group_rec *KHE_TASK_BOUND_GROUP;

typedef enum {
  KHE_TASK_JOB_HARD_PRC = 1,
  KHE_TASK_JOB_SOFT_PRC = 2,
  KHE_TASK_JOB_HARD_ASAC = 4,
  KHE_TASK_JOB_SOFT_ASAC = 8,
  KHE_TASK_JOB_PARTITION = 16
} KHE_TASK_JOB_TYPE;

typedef struct khe_task_group_rec *KHE_TASK_GROUP;
typedef struct khe_task_groups_rec *KHE_TASK_GROUPS;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 12.  Ejection Chains                                           */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ejector_rec *KHE_EJECTOR;

/* ***
typedef enum {
  KHE_EJECTOR_FIRST,
  KHE_EJECTOR_BEST_COST,
  KHE_EJECTOR_BEST_DISRUPTION
} KHE_EJECTOR_SOLVE_TYPE;
*** */

typedef struct khe_ejector_major_schedule_rec *KHE_EJECTOR_MAJOR_SCHEDULE;
typedef struct khe_ejector_minor_schedule_rec *KHE_EJECTOR_MINOR_SCHEDULE;

typedef void (*KHE_EJECTOR_AUGMENT_FN)(KHE_EJECTOR ej, KHE_MONITOR d);


/*****************************************************************************/
/*                                                                           */
/*                     FUNCTION DECLARATIONS                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                    Part A: The Platform                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 2.   Archives and Solution Groups                              */
/*                                                                           */
/*****************************************************************************/

/* 2.1 Archives */
extern KHE_ARCHIVE KheArchiveMake(char *id, KHE_ARCHIVE_METADATA md);
extern char *KheArchiveId(KHE_ARCHIVE archive);
extern KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive);

extern void KheArchiveSetBack(KHE_ARCHIVE archive, void *back);
extern void *KheArchiveBack(KHE_ARCHIVE archive);

extern KHE_ARCHIVE_METADATA KheArchiveMetaDataMake(char *name,
  char *contributor, char *date, char *description, char *remarks);
extern char *KheArchiveMetaDataName(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataContributor(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataDate(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataDescription(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataRemarks(KHE_ARCHIVE_METADATA md);

extern bool KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins);
extern void KheArchiveDeleteInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins);

extern int KheArchiveInstanceCount(KHE_ARCHIVE archive);
extern KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i);
extern bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,
  KHE_INSTANCE *ins);

extern int KheArchiveSolnGroupCount(KHE_ARCHIVE archive);
extern KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i);
extern bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP *soln_group);

/* 2.2 Solution groups */
extern bool KheSolnGroupMake(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP_METADATA md, KHE_SOLN_GROUP *soln_group);
extern void KheSolnGroupSetBack(KHE_SOLN_GROUP soln_group, void *back);
extern void *KheSolnGroupBack(KHE_SOLN_GROUP soln_group);

extern KHE_ARCHIVE KheSolnGroupArchive(KHE_SOLN_GROUP soln_group);
extern char *KheSolnGroupId(KHE_SOLN_GROUP soln_group);
extern KHE_SOLN_GROUP_METADATA KheSolnGroupMetaData(KHE_SOLN_GROUP soln_group);

extern KHE_SOLN_GROUP_METADATA KheSolnGroupMetaDataMake(char *contributor,
  char *date, char *description, char *publication, char *remarks);
extern char *KheSolnGroupMetaDataContributor(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataDate(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataDescription(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataPublication(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataRemarks(KHE_SOLN_GROUP_METADATA md);

extern void KheSolnGroupAddSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln);
extern void KheSolnGroupDeleteSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln);

extern int KheSolnGroupSolnCount(KHE_SOLN_GROUP soln_group);
extern KHE_SOLN KheSolnGroupSoln(KHE_SOLN_GROUP soln_group, int i);

/* 2.3 Reading archives */
extern bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive, KML_ERROR *ke,
  bool infer_resource_partitions, bool allow_invalid_solns,
  char **leftover, int *leftover_len, FILE *echo_fp);
extern bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,
  KML_ERROR *ke, bool infer_resource_partitions, bool allow_invalid_solns);

/* 2.4 Reading archives incrementally */
extern bool KheArchiveReadIncremental(FILE *fp, KHE_ARCHIVE *archive,
  KML_ERROR *ke, bool infer_resource_partitions, bool allow_invalid_solns,
  char **leftover, int *leftover_len, FILE *echo_fp,
  KHE_ARCHIVE_FN archive_begin_fn, KHE_ARCHIVE_FN archive_end_fn,
  KHE_SOLN_GROUP_FN soln_group_begin_fn,
  KHE_SOLN_GROUP_FN soln_group_end_fn, KHE_SOLN_FN soln_fn, void *impl);

/* 2.5 Writing archives */
extern void KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp);
extern void KheArchiveWriteSolnGroup(KHE_ARCHIVE archive,
  KHE_SOLN_GROUP soln_group, bool with_reports, FILE *fp);
extern void KheArchiveWriteWithoutSolnGroups(KHE_ARCHIVE archive, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 3.   Instances                                                 */
/*                                                                           */
/*****************************************************************************/

/* 3.1 Creating instances - general */
extern KHE_INSTANCE KheInstanceMakeBegin(char *id, KHE_INSTANCE_METADATA md);

extern char *KheInstanceId(KHE_INSTANCE ins);
extern char *KheInstanceName(KHE_INSTANCE ins);
extern KHE_INSTANCE_METADATA KheInstanceMetaData(KHE_INSTANCE ins);

extern int KheInstanceArchiveCount(KHE_INSTANCE ins);
extern KHE_ARCHIVE KheInstanceArchive(KHE_INSTANCE ins, int i);

extern void KheInstanceSetBack(KHE_INSTANCE ins, void *back);
extern void *KheInstanceBack(KHE_INSTANCE ins);

extern void KheInstanceMakeEnd(KHE_INSTANCE ins,
  bool infer_resource_partitions);

/* 3.1 Creating instances - metadata */
extern KHE_INSTANCE_METADATA KheInstanceMetaDataMake(char *name,
  char *contributor, char *date, char *country, char *description,
  char *remarks);
extern char *KheInstanceMetaDataName(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataContributor(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataDate(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataCountry(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataDescription(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataRemarks(KHE_INSTANCE_METADATA md);

/* 3.2 Visiting and retrieving the components of instances - time groups */
extern int KheInstanceTimeGroupCount(KHE_INSTANCE ins);
extern KHE_TIME_GROUP KheInstanceTimeGroup(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveTimeGroup(KHE_INSTANCE ins, char *id,
  KHE_TIME_GROUP *tg);

extern KHE_TIME_GROUP KheInstanceFullTimeGroup(KHE_INSTANCE ins);
extern KHE_TIME_GROUP KheInstanceEmptyTimeGroup(KHE_INSTANCE ins);

/* 3.2 Visiting and retrieving the components of instances - times */
extern int KheInstanceTimeCount(KHE_INSTANCE ins);
extern KHE_TIME KheInstanceTime(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveTime(KHE_INSTANCE ins, char *id,
  KHE_TIME *t);

/* 3.2 Visiting and retrieving the components of instances - resource types */
extern int KheInstanceResourceTypeCount(KHE_INSTANCE ins);
extern KHE_RESOURCE_TYPE KheInstanceResourceType(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveResourceType(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_TYPE *rt);

/* 3.2 Visiting and retrieving the components of instances - resources */
extern bool KheInstanceRetrieveResourceGroup(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_GROUP *rg);
extern bool KheInstanceRetrieveResource(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE *r);
extern int KheInstanceResourceCount(KHE_INSTANCE ins);
extern KHE_RESOURCE KheInstanceResource(KHE_INSTANCE ins, int i);

/* 3.2 Visiting and retrieving the components of instances - event groups */
extern int KheInstanceEventGroupCount(KHE_INSTANCE ins);
extern KHE_EVENT_GROUP KheInstanceEventGroup(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveEventGroup(KHE_INSTANCE ins, char *id,
  KHE_EVENT_GROUP *eg);
extern KHE_EVENT_GROUP KheInstanceFullEventGroup(KHE_INSTANCE ins);
extern KHE_EVENT_GROUP KheInstanceEmptyEventGroup(KHE_INSTANCE ins);

/* 3.2 Visiting and retrieving the components of instances - events */
extern int KheInstanceEventCount(KHE_INSTANCE ins);
extern KHE_EVENT KheInstanceEvent(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveEvent(KHE_INSTANCE ins, char *id,
  KHE_EVENT *e);

/* 3.2 Visiting and retrieving the components of instances - event resources */
extern int KheInstanceEventResourceCount(KHE_INSTANCE ins);
extern KHE_EVENT_RESOURCE KheInstanceEventResource(KHE_INSTANCE ins, int i);

/* 3.2 Visiting and retrieving the components of instances - constraints */
extern int KheInstanceConstraintCount(KHE_INSTANCE ins);
extern KHE_CONSTRAINT KheInstanceConstraint(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveConstraint(KHE_INSTANCE ins, char *id,
  KHE_CONSTRAINT *c);

/* 3.3 Constraint density */
extern int KheInstanceConstraintDensityCount(KHE_INSTANCE ins,
  KHE_CONSTRAINT_TAG constraint_tag);
extern int KheInstanceConstraintDensityTotal(KHE_INSTANCE ins,
  KHE_CONSTRAINT_TAG constraint_tag);

/* 3.4.1 Time groups */
extern bool KheTimeGroupMake(KHE_INSTANCE ins, KHE_TIME_GROUP_KIND kind,
  char *id, char *name, KHE_TIME_GROUP *tg);
extern void KheTimeGroupSetBack(KHE_TIME_GROUP tg, void *back);
extern void *KheTimeGroupBack(KHE_TIME_GROUP tg);

extern KHE_INSTANCE KheTimeGroupInstance(KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP_KIND KheTimeGroupKind(KHE_TIME_GROUP tg);
extern char *KheTimeGroupId(KHE_TIME_GROUP tg);
extern char *KheTimeGroupName(KHE_TIME_GROUP tg);

extern void KheTimeGroupAddTime(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupSubTime(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupUnion(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupIntersect(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupDifference(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);

extern int KheTimeGroupTimeCount(KHE_TIME_GROUP tg);
extern KHE_TIME KheTimeGroupTime(KHE_TIME_GROUP tg, int i);

extern bool KheTimeGroupContains(KHE_TIME_GROUP tg, KHE_TIME t);
extern bool KheTimeGroupEqual(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);
extern bool KheTimeGroupSubset(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);
extern bool KheTimeGroupDisjoint(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);

extern bool KheTimeGroupIsCompact(KHE_TIME_GROUP tg);
extern int KheTimeGroupOverlap(KHE_TIME_GROUP tg, KHE_TIME time, int durn);

extern KHE_TIME_GROUP KheTimeGroupNeighbour(KHE_TIME_GROUP tg, int delta);

extern void KheTimeGroupDebug(KHE_TIME_GROUP tg, int verbosity,
  int indent, FILE *fp);

/* 3.4.2 Times */
extern bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,
  bool break_after, KHE_TIME *t);
extern void KheTimeSetBack(KHE_TIME t, void *back);
extern void *KheTimeBack(KHE_TIME t);

extern KHE_INSTANCE KheTimeInstance(KHE_TIME t);
extern char *KheTimeId(KHE_TIME t);
extern char *KheTimeName(KHE_TIME t);
extern bool KheTimeBreakAfter(KHE_TIME t);
extern int KheTimeIndex(KHE_TIME t);

extern bool KheTimeHasNeighbour(KHE_TIME t, int delta);
extern KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta);

extern bool KheTimeLE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);
extern bool KheTimeLT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);
extern bool KheTimeGT(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);
extern bool KheTimeGE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);
extern bool KheTimeEQ(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);
extern bool KheTimeNE(KHE_TIME time1, int delta1, KHE_TIME time2, int delta2);

extern int KheTimeIntervalsOverlap(KHE_TIME time1, int durn1,
  KHE_TIME time2, int durn2);
extern bool KheTimeIntervalsOverlapInterval(KHE_TIME time1, int durn1,
  KHE_TIME time2, int durn2, KHE_TIME *overlap_time, int *overlap_durn);

extern KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t);

/* 3.5.1 Resource types */
extern bool KheResourceTypeMake(KHE_INSTANCE ins, char *id, char *name,
  bool has_partitions, KHE_RESOURCE_TYPE *rt);
extern void KheResourceTypeSetBack(KHE_RESOURCE_TYPE rt, void *back);
extern void *KheResourceTypeBack(KHE_RESOURCE_TYPE rt);

extern KHE_INSTANCE KheResourceTypeInstance(KHE_RESOURCE_TYPE rt);
extern int KheResourceTypeIndex(KHE_RESOURCE_TYPE rt);
extern char *KheResourceTypeId(KHE_RESOURCE_TYPE rt);
extern char *KheResourceTypeName(KHE_RESOURCE_TYPE rt);
extern bool KheResourceTypeHasPartitions(KHE_RESOURCE_TYPE rt);

extern int KheResourceTypeResourceGroupCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypeResourceGroup(KHE_RESOURCE_TYPE rt,
  int i);
extern bool KheResourceTypeRetrieveResourceGroup(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE_GROUP *rg);

extern int KheResourceTypePartitionCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypePartition(KHE_RESOURCE_TYPE rt, int i);

extern KHE_RESOURCE_GROUP KheResourceTypeFullResourceGroup(
  KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypeEmptyResourceGroup(
  KHE_RESOURCE_TYPE rt);

extern int KheResourceTypeResourceCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE KheResourceTypeResource(KHE_RESOURCE_TYPE rt, int i);
extern bool KheResourceTypeRetrieveResource(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE *r);

extern bool KheResourceTypeDemandIsAllPreassigned(KHE_RESOURCE_TYPE rt);
extern int KheResourceTypeAvoidSplitAssignmentsCount(KHE_RESOURCE_TYPE rt);

/* 3.5.2 Resource groups */
extern bool KheResourceGroupMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  bool is_partition, KHE_RESOURCE_GROUP *rg);
extern void KheResourceGroupSetBack(KHE_RESOURCE_GROUP rg, void *back);
extern void *KheResourceGroupBack(KHE_RESOURCE_GROUP rg);

extern KHE_RESOURCE_TYPE KheResourceGroupResourceType(KHE_RESOURCE_GROUP rg);
extern KHE_INSTANCE KheResourceGroupInstance(KHE_RESOURCE_GROUP rg);
extern char *KheResourceGroupId(KHE_RESOURCE_GROUP rg);
extern char *KheResourceGroupName(KHE_RESOURCE_GROUP rg);
extern bool KheResourceGroupIsPartition(KHE_RESOURCE_GROUP rg);

extern void KheResourceGroupAddResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r);
extern void KheResourceGroupSubResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r);
extern void KheResourceGroupUnion(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupIntersect(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupDifference(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);

extern int KheResourceGroupResourceCount(KHE_RESOURCE_GROUP rg);
extern KHE_RESOURCE KheResourceGroupResource(KHE_RESOURCE_GROUP rg, int i);

extern bool KheResourceGroupContains(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern bool KheResourceGroupEqual(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);
extern bool KheResourceGroupSubset(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);
extern bool KheResourceGroupDisjoint(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);

extern KHE_RESOURCE_GROUP KheResourceGroupPartition(KHE_RESOURCE_GROUP rg);

extern void KheResourceGroupDebug(KHE_RESOURCE_GROUP rg, int verbosity,
  int indent, FILE *fp);

/* 3.5.3 Resources */
extern bool KheResourceMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  KHE_RESOURCE_GROUP partition, KHE_RESOURCE *r);
extern void KheResourceSetBack(KHE_RESOURCE r, void *back);
extern void *KheResourceBack(KHE_RESOURCE r);

extern KHE_INSTANCE KheResourceInstance(KHE_RESOURCE r);
extern int KheResourceInstanceIndex(KHE_RESOURCE r);
extern KHE_RESOURCE_TYPE KheResourceResourceType(KHE_RESOURCE r);
extern int KheResourceResourceTypeIndex(KHE_RESOURCE r);
extern char *KheResourceId(KHE_RESOURCE r);
extern char *KheResourceName(KHE_RESOURCE r);
extern KHE_RESOURCE_GROUP KheResourcePartition(KHE_RESOURCE r);

extern KHE_RESOURCE_GROUP KheResourceSingletonResourceGroup(KHE_RESOURCE r);

extern int KheResourcePreassignedEventResourceCount(KHE_RESOURCE r);
extern KHE_EVENT_RESOURCE KheResourcePreassignedEventResource(KHE_RESOURCE r,
  int i);

extern int KheResourceConstraintCount(KHE_RESOURCE r);
extern KHE_CONSTRAINT KheResourceConstraint(KHE_RESOURCE r, int i);

extern KHE_TIME_GROUP KheResourceHardUnavailableTimeGroup(KHE_RESOURCE r);
extern KHE_TIME_GROUP KheResourceHardAndSoftUnavailableTimeGroup(
  KHE_RESOURCE r);

extern bool KheResourceHasAvoidClashesConstraint(KHE_RESOURCE r, KHE_COST cost);
extern int KheResourcePreassignedEventsDuration(KHE_RESOURCE r, KHE_COST cost);

extern void KheResourceDebug(KHE_RESOURCE r, int verbosity,
  int indent, FILE *fp);

/* 3.5.4 Resource layers */
extern int KheResourceLayerEventCount(KHE_RESOURCE r);
extern KHE_EVENT KheResourceLayerEvent(KHE_RESOURCE r, int i);
extern int KheResourceLayerDuration(KHE_RESOURCE r);

/* 3.5.5 Resource similarity and inferring resource partitions */
extern bool KheResourceSimilar(KHE_RESOURCE r1, KHE_RESOURCE r2);

/* 3.6.1 Event groups */
extern bool KheEventGroupMake(KHE_INSTANCE ins, KHE_EVENT_GROUP_KIND kind,
  char *id, char *name, KHE_EVENT_GROUP *eg);
extern void KheEventGroupSetBack(KHE_EVENT_GROUP eg, void *back);
extern void *KheEventGroupBack(KHE_EVENT_GROUP eg);

extern KHE_INSTANCE KheEventGroupInstance(KHE_EVENT_GROUP eg);
extern KHE_EVENT_GROUP_KIND KheEventGroupKind(KHE_EVENT_GROUP eg);
extern char *KheEventGroupId(KHE_EVENT_GROUP eg);
extern char *KheEventGroupName(KHE_EVENT_GROUP eg);

extern void KheEventGroupAddEvent(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupSubEvent(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupUnion(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupIntersect(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupDifference(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);

extern int KheEventGroupEventCount(KHE_EVENT_GROUP eg);
extern KHE_EVENT KheEventGroupEvent(KHE_EVENT_GROUP eg, int i);

extern bool KheEventGroupContains(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern bool KheEventGroupEqual(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);
extern bool KheEventGroupSubset(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);
extern bool KheEventGroupDisjoint(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);

extern int KheEventGroupConstraintCount(KHE_EVENT_GROUP eg);
extern KHE_CONSTRAINT KheEventGroupConstraint(KHE_EVENT_GROUP eg, int i);

extern void KheEventGroupDebug(KHE_EVENT_GROUP eg, int verbosity,
  int indent, FILE *fp);

/* 3.6.2 Events */
extern bool KheEventMake(KHE_INSTANCE ins, char *id, char *name, char *color,
  int duration, int workload, KHE_TIME preassigned_time, KHE_EVENT *e);
extern void KheEventSetBack(KHE_EVENT e, void *back);
extern void *KheEventBack(KHE_EVENT e);

extern KHE_INSTANCE KheEventInstance(KHE_EVENT e);
extern char *KheEventId(KHE_EVENT e);
extern char *KheEventName(KHE_EVENT e);
extern char *KheEventColor(KHE_EVENT e);
extern int KheEventDuration(KHE_EVENT e);
extern int KheEventWorkload(KHE_EVENT e);
extern KHE_TIME KheEventPreassignedTime(KHE_EVENT e);

extern int KheEventIndex(KHE_EVENT e);
extern int KheEventDemand(KHE_EVENT e);

extern int KheEventResourceCount(KHE_EVENT e);
extern KHE_EVENT_RESOURCE KheEventResource(KHE_EVENT e, int i);
extern bool KheEventRetrieveEventResource(KHE_EVENT e, char *role,
  KHE_EVENT_RESOURCE *er);

extern int KheEventResourceGroupCount(KHE_EVENT e);
extern KHE_EVENT_RESOURCE_GROUP KheEventResourceGroup(KHE_EVENT e, int i);

extern KHE_EVENT_GROUP KheEventSingletonEventGroup(KHE_EVENT e);

extern int KheEventConstraintCount(KHE_EVENT e);
extern KHE_CONSTRAINT KheEventConstraint(KHE_EVENT e, int i);

extern bool KheEventSimilar(KHE_EVENT e1, KHE_EVENT e2);
extern bool KheEventMergeable(KHE_EVENT e1, KHE_EVENT e2, int slack);
extern bool KheEventSharePreassignedResource(KHE_EVENT e1, KHE_EVENT e2,
  KHE_RESOURCE *r);

extern void KheEventDebug(KHE_EVENT e, int verbosity, int indent, FILE *fp);

/* 3.6.3 Event resources */
extern bool KheEventResourceMake(KHE_EVENT event, KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE preassigned_resource, char *role, int workload,
  KHE_EVENT_RESOURCE *er);
extern void KheEventResourceSetBack(KHE_EVENT_RESOURCE er, void *back);
extern void *KheEventResourceBack(KHE_EVENT_RESOURCE er);

extern KHE_INSTANCE KheEventResourceInstance(KHE_EVENT_RESOURCE er);
extern int KheEventResourceInstanceIndex(KHE_EVENT_RESOURCE er);
extern KHE_EVENT KheEventResourceEvent(KHE_EVENT_RESOURCE er);
extern int KheEventResourceEventIndex(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE_TYPE KheEventResourceResourceType(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE KheEventResourcePreassignedResource(KHE_EVENT_RESOURCE er);
extern char *KheEventResourceRole(KHE_EVENT_RESOURCE er);
extern int KheEventResourceWorkload(KHE_EVENT_RESOURCE er);

extern int KheEventResourceConstraintCount(KHE_EVENT_RESOURCE er);
extern KHE_CONSTRAINT KheEventResourceConstraint(KHE_EVENT_RESOURCE er, int i);
extern int KheEventResourceConstraintEventGroupIndex(KHE_EVENT_RESOURCE er,
  int i);

extern KHE_RESOURCE_GROUP KheEventResourceHardDomain(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE_GROUP KheEventResourceHardAndSoftDomain(
  KHE_EVENT_RESOURCE er);

extern void KheEventResourceDebug(KHE_EVENT_RESOURCE er, int verbosity,
  int indent, FILE *fp);

/* 3.6.4 Event resource groups */
extern KHE_EVENT_RESOURCE_GROUP KheEventResourceGroupMake(KHE_EVENT event,
  KHE_RESOURCE_GROUP rg);

extern KHE_EVENT KheEventResourceGroupEvent(KHE_EVENT_RESOURCE_GROUP erg);
extern KHE_RESOURCE_GROUP KheEventResourceGroupResourceGroup(
  KHE_EVENT_RESOURCE_GROUP erg);

extern KHE_EVENT_RESOURCE_GROUP KheEventResourceEventResourceGroup(
  KHE_EVENT_RESOURCE er);

extern void KheEventResourceGroupDebug(KHE_EVENT_RESOURCE_GROUP erg,
  int verbosity, int indent, FILE *fp);

/* 3.7 Constraints */
extern void KheConstraintSetBack(KHE_CONSTRAINT c, void *back);
extern void *KheConstraintBack(KHE_CONSTRAINT c);

extern KHE_INSTANCE KheConstraintInstance(KHE_CONSTRAINT c);
extern char *KheConstraintId(KHE_CONSTRAINT c);
extern char *KheConstraintName(KHE_CONSTRAINT c);
extern bool KheConstraintRequired(KHE_CONSTRAINT c);
extern int KheConstraintWeight(KHE_CONSTRAINT c);
extern KHE_COST KheConstraintCombinedWeight(KHE_CONSTRAINT c);
extern KHE_COST_FUNCTION KheConstraintCostFunction(KHE_CONSTRAINT c);
extern int KheConstraintIndex(KHE_CONSTRAINT c);
extern KHE_CONSTRAINT_TAG KheConstraintTag(KHE_CONSTRAINT c);

extern int KheConstraintAppliesToCount(KHE_CONSTRAINT c);

extern char *KheConstraintTagShow(KHE_CONSTRAINT_TAG tag);
extern char *KheConstraintTagShowSpaced(KHE_CONSTRAINT_TAG tag);
extern KHE_CONSTRAINT_TAG KheStringToConstraintTag(char *str);
extern char *KheCostFunctionShow(KHE_COST_FUNCTION cf);

extern void KheConstraintDebug(KHE_CONSTRAINT c, int verbosity,
  int indent, FILE *fp);

extern KHE_CONSTRAINT KheFromAssignResourceConstraint(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAssignTimeConstraint(
  KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromSplitEventsConstraint(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromDistributeSplitEventsConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromPreferResourcesConstraint(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromPreferTimesConstraint(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidSplitAssignmentsConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromSpreadEventsConstraint(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLinkEventsConstraint(
  KHE_LINK_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromOrderEventsConstraint(
  KHE_ORDER_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidClashesConstraint(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidUnavailableTimesConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitIdleTimesConstraint(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromClusterBusyTimesConstraint(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitBusyTimesConstraint(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitWorkloadConstraint(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);

extern KHE_ASSIGN_RESOURCE_CONSTRAINT
  KheToAssignResourceConstraint(KHE_CONSTRAINT c);
extern KHE_ASSIGN_TIME_CONSTRAINT
  KheToAssignTimeConstraint(KHE_CONSTRAINT c);
extern KHE_SPLIT_EVENTS_CONSTRAINT
  KheToSplitEventsConstraint(KHE_CONSTRAINT c);
extern KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheToDistributeSplitEventsConstraint(KHE_CONSTRAINT c);
extern KHE_PREFER_RESOURCES_CONSTRAINT
  KheToPreferResourcesConstraint(KHE_CONSTRAINT c);
extern KHE_PREFER_TIMES_CONSTRAINT
  KheToPreferTimesConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheToAvoidSplitAssignmentsConstraint(KHE_CONSTRAINT c);
extern KHE_SPREAD_EVENTS_CONSTRAINT
  KheToSpreadEventsConstraint(KHE_CONSTRAINT c);
extern KHE_LINK_EVENTS_CONSTRAINT
  KheToLinkEventsConstraint(KHE_CONSTRAINT c);
extern KHE_ORDER_EVENTS_CONSTRAINT
  KheToOrderEventsConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_CLASHES_CONSTRAINT
  KheToAvoidClashesConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheToAvoidUnavailableTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_IDLE_TIMES_CONSTRAINT
  KheToLimitIdleTimesConstraint(KHE_CONSTRAINT c);
extern KHE_CLUSTER_BUSY_TIMES_CONSTRAINT
  KheToClusterBusyTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_BUSY_TIMES_CONSTRAINT
  KheToLimitBusyTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_WORKLOAD_CONSTRAINT
  KheToLimitWorkloadConstraint(KHE_CONSTRAINT c);

/* 3.7.1 Assign resource constraints */
extern bool KheAssignResourceConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_ASSIGN_RESOURCE_CONSTRAINT *c);
extern char *KheAssignResourceConstraintRole(KHE_ASSIGN_RESOURCE_CONSTRAINT c);

extern void KheAssignResourceConstraintAddEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_RESOURCE er);
extern int KheAssignResourceConstraintEventResourceCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT_RESOURCE KheAssignResourceConstraintEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

extern void KheAssignResourceConstraintAddEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e);
extern int KheAssignResourceConstraintEventCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT KheAssignResourceConstraintEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

extern void KheAssignResourceConstraintAddEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheAssignResourceConstraintEventGroupCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAssignResourceConstraintEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

/* 3.7.2 Assign time constraints */
extern bool KheAssignTimeConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_ASSIGN_TIME_CONSTRAINT *c);

extern void KheAssignTimeConstraintAddEvent(KHE_ASSIGN_TIME_CONSTRAINT c,
  KHE_EVENT e);
extern int KheAssignTimeConstraintEventCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_EVENT KheAssignTimeConstraintEvent(KHE_ASSIGN_TIME_CONSTRAINT c,
  int i);

extern void KheAssignTimeConstraintAddEventGroup(KHE_ASSIGN_TIME_CONSTRAINT c,
  KHE_EVENT_GROUP eg);
extern int KheAssignTimeConstraintEventGroupCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAssignTimeConstraintEventGroup(
  KHE_ASSIGN_TIME_CONSTRAINT c, int i);

/* 3.7.3 Split events constraints */
extern bool KheSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int min_duration, int max_duration, int min_amount, int max_amount,
  KHE_SPLIT_EVENTS_CONSTRAINT *c);

extern int KheSplitEventsConstraintMinDuration(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMaxDuration(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMinAmount(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMaxAmount(KHE_SPLIT_EVENTS_CONSTRAINT c);

extern void KheSplitEventsConstraintAddEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KHE_EVENT e);
extern int KheSplitEventsConstraintEventCount(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT KheSplitEventsConstraintEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,
  int i);

extern void KheSplitEventsConstraintAddEventGroup(
  KHE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheSplitEventsConstraintEventGroupCount(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheSplitEventsConstraintEventGroup(
  KHE_SPLIT_EVENTS_CONSTRAINT c, int i);

/* 3.7.4 Distribute split events constraints */
extern bool KheDistributeSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int duration, int minimum, int maximum,
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT *c);

extern int KheDistributeSplitEventsConstraintDuration(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheDistributeSplitEventsConstraintMinimum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheDistributeSplitEventsConstraintMaximum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);

extern void KheDistributeSplitEventsConstraintAddEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT e);
extern int KheDistributeSplitEventsConstraintEventCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT KheDistributeSplitEventsConstraintEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i);

extern void KheDistributeSplitEventsConstraintAddEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheDistributeSplitEventsConstraintEventGroupCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheDistributeSplitEventsConstraintEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i);

/* 3.7.5 Prefer resources constraints */
extern bool KhePreferResourcesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_PREFER_RESOURCES_CONSTRAINT *c);
extern char *KhePreferResourcesConstraintRole(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

extern bool KhePreferResourcesConstraintAddResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KhePreferResourcesConstraintResourceGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KhePreferResourcesConstraintResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE r);
extern int KhePreferResourcesConstraintResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_RESOURCE KhePreferResourcesConstraintResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern KHE_RESOURCE_GROUP KhePreferResourcesConstraintDomain(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

extern bool KhePreferResourcesConstraintAddEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_RESOURCE er);
extern int KhePreferResourcesConstraintEventResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT_RESOURCE KhePreferResourcesConstraintEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e);
extern int KhePreferResourcesConstraintEventCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT KhePreferResourcesConstraintEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event);
extern int KhePreferResourcesConstraintEventGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT_GROUP KhePreferResourcesConstraintEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

/* 3.7.6 Prefer times constraints */
extern bool KhePreferTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int duration, KHE_PREFER_TIMES_CONSTRAINT *c);
extern int KhePreferTimesConstraintDuration(KHE_PREFER_TIMES_CONSTRAINT c);

extern void KhePreferTimesConstraintAddTimeGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KhePreferTimesConstraintTimeGroupCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KhePreferTimesConstraintTimeGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern void KhePreferTimesConstraintAddTime(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_TIME t);
extern int KhePreferTimesConstraintTimeCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_TIME KhePreferTimesConstraintTime(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern KHE_TIME_GROUP KhePreferTimesConstraintDomain(
  KHE_PREFER_TIMES_CONSTRAINT c);

extern void KhePreferTimesConstraintAddEvent(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT e);
extern int KhePreferTimesConstraintEventCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_EVENT KhePreferTimesConstraintEvent(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern void KhePreferTimesConstraintAddEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KhePreferTimesConstraintEventGroupCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_EVENT_GROUP KhePreferTimesConstraintEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

/* 3.7.7 Avoid split assignments constraints */
extern bool KheAvoidSplitAssignmentsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT *c);
extern char *KheAvoidSplitAssignmentsConstraintRole(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);

extern bool KheAvoidSplitAssignmentsConstraintAddEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event);
extern int KheAvoidSplitAssignmentsConstraintEventGroupCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAvoidSplitAssignmentsConstraintEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int i);

extern void KheAvoidSplitAssignmentsConstraintAddEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index,
  KHE_EVENT_RESOURCE er);
extern int KheAvoidSplitAssignmentsConstraintEventResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index);
extern KHE_EVENT_RESOURCE KheAvoidSplitAssignmentsConstraintEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index, int er_index);

/* 3.7.8 Spread events constraints */
extern bool KheSpreadEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_TIME_SPREAD ts, KHE_SPREAD_EVENTS_CONSTRAINT *c);
extern KHE_TIME_SPREAD KheSpreadEventsConstraintTimeSpread(
  KHE_SPREAD_EVENTS_CONSTRAINT c);

extern KHE_TIME_SPREAD KheTimeSpreadMake(KHE_INSTANCE ins);
extern void KheTimeSpreadAddLimitedTimeGroup(KHE_TIME_SPREAD ts,
  KHE_LIMITED_TIME_GROUP ltg);
extern int KheTimeSpreadLimitedTimeGroupCount(KHE_TIME_SPREAD ts);
extern KHE_LIMITED_TIME_GROUP KheTimeSpreadLimitedTimeGroup(KHE_TIME_SPREAD ts,
  int i);

extern KHE_LIMITED_TIME_GROUP KheLimitedTimeGroupMake(KHE_TIME_GROUP tg,
  int minimum, int maximum);
extern KHE_TIME_GROUP KheLimitedTimeGroupTimeGroup(KHE_LIMITED_TIME_GROUP ltg);
extern int KheLimitedTimeGroupMinimum(KHE_LIMITED_TIME_GROUP ltg);
extern int KheLimitedTimeGroupMaximum(KHE_LIMITED_TIME_GROUP ltg);

extern bool KheTimeSpreadTimeGroupsDisjoint(KHE_TIME_SPREAD ts);
extern bool KheTimeSpreadCoversWholeCycle(KHE_TIME_SPREAD ts);

extern void KheSpreadEventsConstraintAddEventGroup(
  KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheSpreadEventsConstraintEventGroupCount(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheSpreadEventsConstraintEventGroup(
  KHE_SPREAD_EVENTS_CONSTRAINT c, int i);

/* 3.7.9 Link events constraints */
extern bool KheLinkEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_LINK_EVENTS_CONSTRAINT *c);

extern void KheLinkEventsConstraintAddEventGroup(KHE_LINK_EVENTS_CONSTRAINT c,
  KHE_EVENT_GROUP eg);
extern int KheLinkEventsConstraintEventGroupCount(KHE_LINK_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheLinkEventsConstraintEventGroup(
  KHE_LINK_EVENTS_CONSTRAINT c, int i);

/* 3.7.10 Order events constraints */
extern bool KheOrderEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_ORDER_EVENTS_CONSTRAINT *c);

extern void KheOrderEventsConstraintAddEventPair(KHE_ORDER_EVENTS_CONSTRAINT c,
  KHE_EVENT first_event, KHE_EVENT second_event, int min_separation,
  int max_separation);

extern int KheOrderEventsConstraintEventPairCount(
  KHE_ORDER_EVENTS_CONSTRAINT c);
extern KHE_EVENT KheOrderEventsConstraintFirstEvent(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i);
extern KHE_EVENT KheOrderEventsConstraintSecondEvent(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i);
extern int KheOrderEventsConstraintMinSeparation(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i);
extern int KheOrderEventsConstraintMaxSeparation(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i);

/* 3.7.11 Avoid clashes constraints */
extern bool KheAvoidClashesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_AVOID_CLASHES_CONSTRAINT *c);

extern void KheAvoidClashesConstraintAddResourceGroup(
  KHE_AVOID_CLASHES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheAvoidClashesConstraintResourceGroupCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheAvoidClashesConstraintResourceGroup(
  KHE_AVOID_CLASHES_CONSTRAINT c, int i);

extern void KheAvoidClashesConstraintAddResource(
  KHE_AVOID_CLASHES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheAvoidClashesConstraintResourceCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_RESOURCE KheAvoidClashesConstraintResource(
  KHE_AVOID_CLASHES_CONSTRAINT c, int i);

/* 3.7.12 Avoid unavailable times constraints */
extern bool KheAvoidUnavailableTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT *c);

extern void KheAvoidUnavailableTimesConstraintAddResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheAvoidUnavailableTimesConstraintResourceGroupCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheAvoidUnavailableTimesConstraintResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheAvoidUnavailableTimesConstraintResourceCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheAvoidUnavailableTimesConstraintResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheAvoidUnavailableTimesConstraintTimeGroupCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME t);
extern int KheAvoidUnavailableTimesConstraintTimeCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_TIME KheAvoidUnavailableTimesConstraintTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintUnavailableTimes(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintAvailableTimes(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);

/* 3.7.13 Limit idle times constraints */
extern bool KheLimitIdleTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_IDLE_TIMES_CONSTRAINT *c);
extern int KheLimitIdleTimesConstraintMinimum(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern int KheLimitIdleTimesConstraintMaximum(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);

extern void KheLimitIdleTimesConstraintAddTimeGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheLimitIdleTimesConstraintTimeGroupCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheLimitIdleTimesConstraintTimeGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

extern bool KheLimitIdleTimesConstraintTimeGroupsDisjoint(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern bool KheLimitIdleTimesConstraintTimeGroupsCoverWholeCycle(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);

extern void KheLimitIdleTimesConstraintAddResourceGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitIdleTimesConstraintResourceGroupCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitIdleTimesConstraintResourceGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

extern void KheLimitIdleTimesConstraintAddResource(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitIdleTimesConstraintResourceCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheLimitIdleTimesConstraintResource(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

/* 3.7.14 Cluster busy times constraints */
extern bool KheClusterBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT *c);
extern int KheClusterBusyTimesConstraintMinimum(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern int KheClusterBusyTimesConstraintMaximum(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);

extern void KheClusterBusyTimesConstraintAddTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheClusterBusyTimesConstraintTimeGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheClusterBusyTimesConstraintTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheClusterBusyTimesConstraintAddResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheClusterBusyTimesConstraintResourceGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheClusterBusyTimesConstraintResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheClusterBusyTimesConstraintAddResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheClusterBusyTimesConstraintResourceCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheClusterBusyTimesConstraintResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

/* 3.7.15 Limit busy times constraints */
extern bool KheLimitBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_BUSY_TIMES_CONSTRAINT *c);
extern int KheLimitBusyTimesConstraintMinimum(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern int KheLimitBusyTimesConstraintMaximum(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);

extern void KheLimitBusyTimesConstraintAddTimeGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheLimitBusyTimesConstraintTimeGroupCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheLimitBusyTimesConstraintTimeGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

extern KHE_TIME_GROUP KheLimitBusyTimesConstraintDomain(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);

extern void KheLimitBusyTimesConstraintAddResourceGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitBusyTimesConstraintResourceGroupCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitBusyTimesConstraintResourceGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheLimitBusyTimesConstraintAddResource(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitBusyTimesConstraintResourceCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheLimitBusyTimesConstraintResource(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

/* 3.7.16 Limit workload constraints */
extern bool KheLimitWorkloadConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_WORKLOAD_CONSTRAINT *c);
extern int KheLimitWorkloadConstraintMinimum(KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern int KheLimitWorkloadConstraintMaximum(KHE_LIMIT_WORKLOAD_CONSTRAINT c);

extern void KheLimitWorkloadConstraintAddResourceGroup(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitWorkloadConstraintResourceGroupCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitWorkloadConstraintResourceGroup(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, int i);

extern void KheLimitWorkloadConstraintAddResource(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitWorkloadConstraintResourceCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_RESOURCE KheLimitWorkloadConstraintResource(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, int i);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 4.   Solutions                                                 */
/*                                                                           */
/*****************************************************************************/

/* 4.2 Solution objects */
extern KHE_SOLN KheSolnMake(KHE_INSTANCE ins);
extern void KheSolnDelete(KHE_SOLN soln);

extern int KheSolnSolnGroupCount(KHE_SOLN soln);
extern KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln, int i);

extern void KheSolnSetDescription(KHE_SOLN soln, char *description);
extern char *KheSolnDescription(KHE_SOLN soln);
extern void KheSolnSetRunningTime(KHE_SOLN soln, float running_time);
extern float KheSolnRunningTime(KHE_SOLN soln);

extern void KheSolnSetBack(KHE_SOLN soln, void *back);
extern void *KheSolnBack(KHE_SOLN soln);

extern KHE_INSTANCE KheSolnInstance(KHE_SOLN soln);

extern KHE_SOLN KheSolnCopy(KHE_SOLN soln);

extern int KheSolnMonitorCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i);

extern int KheSolnMeetCount(KHE_SOLN soln);
extern KHE_MEET KheSolnMeet(KHE_SOLN soln, int i);
extern int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e);
extern KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i);

extern int KheSolnTaskCount(KHE_SOLN soln);
extern KHE_TASK KheSolnTask(KHE_SOLN soln, int i);
extern int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er);
extern KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er,
  int i);

extern int KheSolnNodeCount(KHE_SOLN soln);
extern KHE_NODE KheSolnNode(KHE_SOLN soln, int i);

extern int KheSolnTaskingCount(KHE_SOLN soln);
extern KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i);

extern void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp);

/* 4.3 Complete representation and preassignment conversion */
extern bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,
  KHE_EVENT *problem_event);
extern void KheSolnAssignPreassignedTimes(KHE_SOLN soln);
extern void KheSolnAssignPreassignedResources(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt);

/* 4.4 Solution time, resource, and event groups */
extern void KheSolnTimeGroupBegin(KHE_SOLN soln);
extern void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME t);
extern void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME t);
extern void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern void KheSolnTimeGroupIntersect(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern void KheSolnTimeGroupDifference(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln);

extern void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);
extern void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r);
extern void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r);
extern void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2);
extern void KheSolnResourceGroupIntersect(KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg2);
extern void KheSolnResourceGroupDifference(KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg2);
extern KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln);

extern void KheSolnEventGroupBegin(KHE_SOLN soln);
extern void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e);
extern void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e);
extern void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln);

/* 4.5 Diversification */
extern void KheSolnSetDiversifier(KHE_SOLN soln, int val);
extern int KheSolnDiversifier(KHE_SOLN soln);
extern int KheSolnDiversifierChoose(KHE_SOLN soln, int c);

/* 4.6 Visit numbers */
extern void KheSolnSetGlobalVisitNum(KHE_SOLN soln, int num);
extern int KheSolnGlobalVisitNum(KHE_SOLN soln);
extern void KheSolnNewGlobalVisit(KHE_SOLN soln);

/* 4.7 Running times and time limits */
extern float KheSolnTimeNow(KHE_SOLN soln);
extern void KheSolnSetTimeLimit(KHE_SOLN soln, float limit_in_secs);
extern float KheSolnTimeLimit(KHE_SOLN soln);
extern bool KheSolnTimeLimitReached(KHE_SOLN soln);

/* 4.8 Meets */
extern KHE_MEET KheMeetMake(KHE_SOLN soln, int duration, KHE_EVENT e);
extern void KheMeetDelete(KHE_MEET meet);

extern void KheMeetSetBack(KHE_MEET meet, void *back);
extern void *KheMeetBack(KHE_MEET meet);

extern void KheMeetSetVisitNum(KHE_MEET meet, int num);
extern int KheMeetVisitNum(KHE_MEET meet);
extern bool KheMeetVisited(KHE_MEET meet, int slack);
extern void KheMeetVisit(KHE_MEET meet);
extern void KheMeetUnVisit(KHE_MEET meet);

extern KHE_SOLN KheMeetSoln(KHE_MEET meet);
extern int KheMeetSolnIndex(KHE_MEET meet);
extern int KheMeetDuration(KHE_MEET meet);
extern KHE_EVENT KheMeetEvent(KHE_MEET meet);
extern bool KheMeetIsPreassigned(KHE_MEET meet, KHE_TIME *time);

extern int KheMeetAssignedDuration(KHE_MEET meet);
extern int KheMeetDemand(KHE_MEET meet);

extern int KheMeetTaskCount(KHE_MEET meet);
extern KHE_TASK KheMeetTask(KHE_MEET meet, int i);
extern bool KheMeetRetrieveTask(KHE_MEET meet, char *role, KHE_TASK *task);
extern bool KheMeetFindTask(KHE_MEET meet, KHE_EVENT_RESOURCE er,
  KHE_TASK *task);

extern bool KheMeetContainsResourcePreassignment(KHE_MEET meet,
  KHE_RESOURCE r, KHE_TASK *task);
extern bool KheMeetContainsResourceAssignment(KHE_MEET meet,
  KHE_RESOURCE r, KHE_TASK *task);

extern KHE_NODE KheMeetNode(KHE_MEET meet);
extern int KheMeetNodeIndex(KHE_MEET meet);

extern void KheMeetDebug(KHE_MEET meet, int verbosity, int indent, FILE *fp);

/* 4.8.1 Splitting and merging */
extern bool KheMeetSplitCheck(KHE_MEET meet, int duration1, bool recursive);
extern bool KheMeetSplit(KHE_MEET meet, int duration1, bool recursive,
  KHE_MEET *meet1, KHE_MEET *meet2);
extern bool KheMeetMergeCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetMerge(KHE_MEET meet1, KHE_MEET meet2, bool recursive,
  KHE_MEET *meet);

/* 4.8.2 Assignment */
extern bool KheMeetMoveCheck(KHE_MEET meet, KHE_MEET target_meet, int offset);
extern bool KheMeetMove(KHE_MEET meet, KHE_MEET target_meet, int offset);

extern bool KheMeetAssignCheck(KHE_MEET meet, KHE_MEET target_meet, int offset);
extern bool KheMeetAssign(KHE_MEET meet, KHE_MEET target_meet, int offset);

extern bool KheMeetUnAssignCheck(KHE_MEET meet);
extern bool KheMeetUnAssign(KHE_MEET meet);

extern bool KheMeetSwapCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetSwap(KHE_MEET meet1, KHE_MEET meet2);

extern bool KheMeetBlockSwapCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetBlockSwap(KHE_MEET meet1, KHE_MEET meet2);

extern KHE_MEET KheMeetAsst(KHE_MEET meet);
extern int KheMeetAsstOffset(KHE_MEET meet);

extern int KheMeetAssignedToCount(KHE_MEET target_meet);
extern KHE_MEET KheMeetAssignedTo(KHE_MEET target_meet, int i);

extern KHE_MEET KheMeetRoot(KHE_MEET meet, int *offset_in_root);
extern bool KheMeetOverlap(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetAdjacent(KHE_MEET meet1, KHE_MEET meet2, bool *swap);

extern void KheMeetAssignFix(KHE_MEET meet);
extern void KheMeetAssignUnFix(KHE_MEET meet);
extern bool KheMeetAssignIsFixed(KHE_MEET meet);

extern bool KheMeetIsMovable(KHE_MEET meet);
extern KHE_MEET KheMeetFirstMovable(KHE_MEET meet, int *offset_in_result);
extern KHE_MEET KheMeetLastFixed(KHE_MEET meet, int *offset_in_result);

/* 4.8.3 Cycle meets and time assignment */
extern bool KheMeetIsCycleMeet(KHE_MEET meet);
extern KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t);
extern int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t);
extern KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration);
extern void KheSolnSplitCycleMeet(KHE_SOLN soln);

extern bool KheMeetMoveTimeCheck(KHE_MEET meet, KHE_TIME t);
extern bool KheMeetMoveTime(KHE_MEET meet, KHE_TIME t);

extern bool KheMeetAssignTimeCheck(KHE_MEET meet, KHE_TIME t);
extern bool KheMeetAssignTime(KHE_MEET meet, KHE_TIME t);
extern bool KheMeetUnAssignTimeCheck(KHE_MEET meet);
extern bool KheMeetUnAssignTime(KHE_MEET meet);
extern KHE_TIME KheMeetAsstTime(KHE_MEET meet);

/* 4.8.4 Meet domains and bounds */
extern KHE_TIME_GROUP KheMeetDomain(KHE_MEET meet);
extern KHE_MEET_BOUND KheMeetBoundMake(KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern bool KheMeetBoundDeleteCheck(KHE_MEET_BOUND mb);
extern bool KheMeetBoundDelete(KHE_MEET_BOUND mb);

extern KHE_SOLN KheMeetBoundSoln(KHE_MEET_BOUND mb);
extern int KheMeetBoundSolnIndex(KHE_MEET_BOUND mb);
extern bool KheMeetBoundOccupancy(KHE_MEET_BOUND mb);
extern KHE_TIME_GROUP KheMeetBoundDefaultTimeGroup(KHE_MEET_BOUND mb);

extern void KheMeetBoundAddTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP KheMeetBoundTimeGroup(KHE_MEET_BOUND mb, int duration);
extern KHE_TIME_GROUP KheSolnStartingTimeGroup(KHE_SOLN soln, int duration,
  KHE_TIME_GROUP tg);

extern bool KheMeetAddMeetBoundCheck(KHE_MEET meet, KHE_MEET_BOUND mb);
extern bool KheMeetAddMeetBound(KHE_MEET meet, KHE_MEET_BOUND mb);
extern bool KheMeetDeleteMeetBoundCheck(KHE_MEET meet, KHE_MEET_BOUND mb);
extern bool KheMeetDeleteMeetBound(KHE_MEET meet, KHE_MEET_BOUND mb);

extern int KheMeetMeetBoundCount(KHE_MEET meet);
extern KHE_MEET_BOUND KheMeetMeetBound(KHE_MEET meet, int i);
extern int KheMeetBoundMeetCount(KHE_MEET_BOUND mb);
extern KHE_MEET KheMeetBoundMeet(KHE_MEET_BOUND mb, int i);

/* 4.8.5 Automatic domains */
extern bool KheMeetSetAutoDomainCheck(KHE_MEET meet, bool automatic);
extern bool KheMeetSetAutoDomain(KHE_MEET meet, bool automatic);
extern KHE_TIME_GROUP KheMeetDescendantsDomain(KHE_MEET meet);

/* 4.9 Tasks */
extern KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KheTaskDelete(KHE_TASK task);

extern void KheTaskSetBack(KHE_TASK task, void *back);
extern void *KheTaskBack(KHE_TASK task);

extern void KheTaskSetVisitNum(KHE_TASK task, int num);
extern int KheTaskVisitNum(KHE_TASK task);
extern bool KheTaskVisited(KHE_TASK task, int slack);
extern void KheTaskVisit(KHE_TASK task);
extern void KheTaskUnVisit(KHE_TASK task);

extern KHE_MEET KheTaskMeet(KHE_TASK task);
extern int KheTaskMeetIndex(KHE_TASK task);
extern int KheTaskDuration(KHE_TASK task);
extern float KheTaskWorkload(KHE_TASK task);

extern KHE_SOLN KheTaskSoln(KHE_TASK task);
extern int KheTaskSolnIndex(KHE_TASK task);
extern KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task);
extern KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task);
extern bool KheTaskIsPreassigned(KHE_TASK task, KHE_RESOURCE *r);

extern KHE_TASKING KheTaskTasking(KHE_TASK task);
extern int KheTaskTaskingIndex(KHE_TASK task);

extern void KheTaskDebug(KHE_TASK task, int verbosity,
  int indent, FILE *fp);

/* 4.9.1 Assignment */
extern bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task);
extern bool KheTaskMove(KHE_TASK task, KHE_TASK target_task);

extern bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task);
extern bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task);

extern bool KheTaskUnAssignCheck(KHE_TASK task);
extern bool KheTaskUnAssign(KHE_TASK task);

extern bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2);
extern bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2);

extern KHE_TASK KheTaskAsst(KHE_TASK task);

extern int KheTaskAssignedToCount(KHE_TASK target_task);
extern KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i);
extern int KheTaskTotalDuration(KHE_TASK task);
extern float KheTaskTotalWorkload(KHE_TASK task);

extern KHE_TASK KheTaskRoot(KHE_TASK task);

extern void KheTaskAssignFix(KHE_TASK task);
extern void KheTaskAssignUnFix(KHE_TASK task);
extern bool KheTaskAssignIsFixed(KHE_TASK task);
extern KHE_TASK KheTaskFirstUnFixed(KHE_TASK task);

/* 4.9.2 Cycle tasks and resource assignment */
extern bool KheTaskIsCycleTask(KHE_TASK task);
extern KHE_TASK KheSolnResourceCycleTask(KHE_SOLN soln, KHE_RESOURCE r);

extern bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r);

extern bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskUnAssignResourceCheck(KHE_TASK task);
extern bool KheTaskUnAssignResource(KHE_TASK task);
extern KHE_RESOURCE KheTaskAsstResource(KHE_TASK task);

extern int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i);

/* 4.9.3 Task domains and bounds */
extern KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task);
extern KHE_TASK_BOUND KheTaskBoundMake(KHE_SOLN soln, KHE_RESOURCE_GROUP rg);
extern bool KheTaskBoundDeleteCheck(KHE_TASK_BOUND tb);
extern bool KheTaskBoundDelete(KHE_TASK_BOUND tb);

extern KHE_SOLN KheTaskBoundSoln(KHE_TASK_BOUND tb);
extern int KheTaskBoundSolnIndex(KHE_TASK_BOUND tb);
extern KHE_RESOURCE_GROUP KheTaskBoundResourceGroup(KHE_TASK_BOUND tb);

extern bool KheTaskAddTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb);
extern bool KheTaskAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb);
extern bool KheTaskDeleteTaskBoundCheck(KHE_TASK task, KHE_TASK_BOUND tb);
extern bool KheTaskDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb);

extern int KheTaskTaskBoundCount(KHE_TASK task);
extern KHE_TASK_BOUND KheTaskTaskBound(KHE_TASK task, int i);
extern int KheTaskBoundTaskCount(KHE_TASK_BOUND tb);
extern KHE_TASK KheTaskBoundTask(KHE_TASK_BOUND tb, int i);

/* 4.10 Marks and paths */
extern KHE_MARK KheMarkBegin(KHE_SOLN soln);
extern void KheMarkEnd(KHE_MARK mark, bool undo);
extern void KheMarkUndo(KHE_MARK mark);
extern KHE_SOLN KheMarkSoln(KHE_MARK mark);
extern KHE_COST KheMarkSolnCost(KHE_MARK mark);
extern KHE_PATH KheMarkAddPath(KHE_MARK mark);
extern KHE_PATH KheMarkAddBestPath(KHE_MARK mark, int k);
extern int KheMarkPathCount(KHE_MARK mark);
extern KHE_PATH KheMarkPath(KHE_MARK mark, int i);
extern void KheMarkPathSort(KHE_MARK mark,
  int(*compar)(const void *, const void *));

extern int KhePathIncreasingSolnCostCmp(const void *t1, const void *t2);
extern KHE_SOLN KhePathSoln(KHE_PATH path);
extern KHE_COST KhePathSolnCost(KHE_PATH path);
extern KHE_MARK KhePathMark(KHE_PATH path);
extern void KhePathDelete(KHE_PATH path);
extern void KhePathRedo(KHE_PATH path);

/* 4.11 Placeholder and invalid solutions */
extern void KheSolnReduceToPlaceholder(KHE_SOLN soln);
extern bool KheSolnIsPlaceholder(KHE_SOLN soln);
extern bool KheSolnIsInvalid(KHE_SOLN soln);
extern KML_ERROR KheSolnInvalidError(KHE_SOLN soln);
extern void KheSolnReduceToInvalid(KHE_SOLN soln, KML_ERROR ke);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 5.   Extra Types for Solving                                   */
/*                                                                           */
/*****************************************************************************/

/* 5.2 Nodes */
extern KHE_NODE KheNodeMake(KHE_SOLN soln);
extern void KheNodeSetBack(KHE_NODE node, void *back);
extern void *KheNodeBack(KHE_NODE node);

extern void KheNodeSetVisitNum(KHE_NODE n, int num);
extern int KheNodeVisitNum(KHE_NODE n);
extern bool KheNodeVisited(KHE_NODE n, int slack);
extern void KheNodeVisit(KHE_NODE n);
extern void KheNodeUnVisit(KHE_NODE n);

extern KHE_SOLN KheNodeSoln(KHE_NODE node);
extern int KheNodeSolnIndex(KHE_NODE node);

extern bool KheNodeDeleteCheck(KHE_NODE node);
extern bool KheNodeDelete(KHE_NODE node);

extern bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeDeleteParentCheck(KHE_NODE child_node);
extern bool KheNodeDeleteParent(KHE_NODE child_node);
extern KHE_NODE KheNodeParent(KHE_NODE node);

extern int KheNodeChildCount(KHE_NODE node);
extern KHE_NODE KheNodeChild(KHE_NODE node, int i);

extern bool KheNodeIsDescendant(KHE_NODE node, KHE_NODE ancestor_node);
extern bool KheNodeIsProperDescendant(KHE_NODE node, KHE_NODE ancestor_node);

extern void KheNodeSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2);

extern bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet);

extern int KheNodeMeetCount(KHE_NODE node);
extern KHE_MEET KheNodeMeet(KHE_NODE node, int i);
extern void KheNodeMeetSort(KHE_NODE node,
  int(*compar)(const void *, const void *));
extern int KheMeetDecreasingDurationCmp(const void *p1, const void *p2);
extern int KheMeetIncreasingAsstCmp(const void *p1, const void *p2);

extern bool KheNodeIsCycleNode(KHE_NODE node);

extern int KheNodeDuration(KHE_NODE node);
extern int KheNodeAssignedDuration(KHE_NODE node);
extern int KheNodeDemand(KHE_NODE node);

extern bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count);
extern int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r);

void KheNodeDebug(KHE_NODE node, int verbosity, int indent, FILE *fp);
extern void KheNodePrintTimetable(KHE_NODE node, int cell_width,
  int indent, FILE *fp);

/* 5.3 Layers */
extern KHE_LAYER KheLayerMake(KHE_NODE parent_node);
extern void KheLayerSetBack(KHE_LAYER layer, void *back);
extern void *KheLayerBack(KHE_LAYER layer);

extern void KheLayerSetVisitNum(KHE_LAYER layer, int num);
extern int KheLayerVisitNum(KHE_LAYER layer);
extern bool KheLayerVisited(KHE_LAYER layer, int slack);
extern void KheLayerVisit(KHE_LAYER layer);
extern void KheLayerUnVisit(KHE_LAYER layer);

extern KHE_NODE KheLayerParentNode(KHE_LAYER layer);
extern int KheLayerParentNodeIndex(KHE_LAYER layer);
extern KHE_SOLN KheLayerSoln(KHE_LAYER layer);
extern void KheLayerDelete(KHE_LAYER layer);

extern void KheLayerAddChildNode(KHE_LAYER layer, KHE_NODE child_node);
extern void KheLayerDeleteChildNode(KHE_LAYER layer, KHE_NODE child_node);
extern int KheLayerChildNodeCount(KHE_LAYER layer);
extern KHE_NODE KheLayerChildNode(KHE_LAYER layer, int i);
extern void KheLayerChildNodesSort(KHE_LAYER layer,
  int(*compar)(const void *, const void *));

extern void KheLayerAddResource(KHE_LAYER layer, KHE_RESOURCE r);
extern void KheLayerDeleteResource(KHE_LAYER layer, KHE_RESOURCE r);
extern int KheLayerResourceCount(KHE_LAYER layer);
extern KHE_RESOURCE KheLayerResource(KHE_LAYER layer, int i);

extern int KheNodeChildLayerCount(KHE_NODE parent_node);
extern KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i);
extern void KheNodeChildLayersSort(KHE_NODE parent_node,
  int(*compar)(const void *, const void *));
extern void KheNodeChildLayersDelete(KHE_NODE parent_node);

extern int KheNodeParentLayerCount(KHE_NODE child_node);
extern KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i);
extern bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2);

extern int KheLayerDuration(KHE_LAYER layer);
extern int KheLayerMeetCount(KHE_LAYER layer);
extern int KheLayerAssignedDuration(KHE_LAYER layer);
extern int KheLayerDemand(KHE_LAYER layer);

extern bool KheLayerEqual(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerSubset(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerDisjoint(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerContains(KHE_LAYER layer, KHE_NODE n);

extern bool KheLayerSame(KHE_LAYER layer1, KHE_LAYER layer2, int *same_count);
extern bool KheLayerSimilar(KHE_LAYER layer1, KHE_LAYER layer2,
  int *similar_count);
extern bool KheLayerRegular(KHE_LAYER layer1, KHE_LAYER layer2,
  int *regular_count);
extern bool KheLayerAlign(KHE_LAYER layer1, KHE_LAYER layer2,
  bool (*node_equiv)(KHE_NODE node1, KHE_NODE node2), int *count);

extern void KheLayerMerge(KHE_LAYER layer1, KHE_LAYER layer2, KHE_LAYER *res);
extern void KheLayerDebug(KHE_LAYER layer, int verbosity, int indent, FILE *fp);

/* 5.4 Zones */
extern KHE_ZONE KheZoneMake(KHE_NODE node);
extern KHE_NODE KheZoneNode(KHE_ZONE zone);
extern int KheZoneNodeIndex(KHE_ZONE zone);
extern KHE_SOLN KheZoneSoln(KHE_ZONE zone);
extern void KheZoneSetBack(KHE_ZONE zone, void *back);
extern void *KheZoneBack(KHE_ZONE zone);

extern void KheZoneSetVisitNum(KHE_ZONE zone, int num);
extern int KheZoneVisitNum(KHE_ZONE zone);
extern bool KheZoneVisited(KHE_ZONE zone, int slack);
extern void KheZoneVisit(KHE_ZONE zone);
extern void KheZoneUnVisit(KHE_ZONE zone);

extern void KheZoneDelete(KHE_ZONE zone);
extern void KheNodeDeleteZones(KHE_NODE node);

extern void KheZoneAddMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset);
extern void KheZoneDeleteMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset);
extern KHE_ZONE KheMeetOffsetZone(KHE_MEET meet, int offset);
extern int KheNodeZoneCount(KHE_NODE node);
extern KHE_ZONE KheNodeZone(KHE_NODE node, int i);
extern int KheZoneMeetOffsetCount(KHE_ZONE zone);
extern void KheZoneMeetOffset(KHE_ZONE zone, int i,
  KHE_MEET *meet, int *offset);
extern void KheZoneDebug(KHE_ZONE zone, int verbosity, int indent, FILE *fp);
extern bool KheMeetMovePreservesZones(KHE_MEET meet1, int offset1,
  KHE_MEET meet2, int offset2, int durn);
extern int KheNodeIrregularity(KHE_NODE node);

/* 5.5 Taskings */
extern KHE_TASKING KheTaskingMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);
extern KHE_SOLN KheTaskingSoln(KHE_TASKING tasking);
extern KHE_RESOURCE_TYPE KheTaskingResourceType(KHE_TASKING tasking);
extern void KheTaskingDelete(KHE_TASKING tasking);

extern void KheTaskingAddTask(KHE_TASKING tasking, KHE_TASK task);
extern void KheTaskingDeleteTask(KHE_TASKING tasking, KHE_TASK task);
extern int KheTaskingTaskCount(KHE_TASKING tasking);
extern KHE_TASK KheTaskingTask(KHE_TASKING tasking, int i);

extern void KheTaskingDebug(KHE_TASKING tasking, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 6.   Solution Monitoring                                       */
/*                                                                           */
/*****************************************************************************/

/* 6.1 Measuring cost */
extern KHE_COST KheSolnCost(KHE_SOLN soln);
extern KHE_COST KheCost(int hard_cost, int soft_cost);
extern int KheHardCost(KHE_COST combined_cost);
extern int KheSoftCost(KHE_COST combined_cost);
extern int KheCostCmp(KHE_COST cost1, KHE_COST cost2);
extern double KheCostShow(KHE_COST combined_cost);

/* 6.2 Monitors */
extern void KheMonitorSetBack(KHE_MONITOR m, void *back);
extern void *KheMonitorBack(KHE_MONITOR m);

extern void KheMonitorSetVisitNum(KHE_MONITOR m, int num);
extern int KheMonitorVisitNum(KHE_MONITOR m);
extern bool KheMonitorVisited(KHE_MONITOR m, int slack);
extern void KheMonitorVisit(KHE_MONITOR m);
extern void KheMonitorUnVisit(KHE_MONITOR m);

extern KHE_SOLN KheMonitorSoln(KHE_MONITOR m);
extern int KheMonitorSolnIndex(KHE_MONITOR m);
extern KHE_COST KheMonitorCost(KHE_MONITOR m);
extern KHE_COST KheMonitorLowerBound(KHE_MONITOR m);

extern KHE_MONITOR_TAG KheMonitorTag(KHE_MONITOR m);
extern KHE_CONSTRAINT KheMonitorConstraint(KHE_MONITOR m);
extern char *KheMonitorAppliesToName(KHE_MONITOR m);

extern int KheMonitorDeviation(KHE_MONITOR m);
extern char *KheMonitorDeviationDescription(KHE_MONITOR m);

extern void KheMonitorDebug(KHE_MONITOR m, int verbosity, int indent, FILE *fp);
extern char *KheMonitorTagShow(KHE_MONITOR_TAG tag);
extern char *KheMonitorLabel(KHE_MONITOR m);

/* 6.3 Attaching, detaching, and provably zero fixed cost */
extern void KheMonitorDetachFromSoln(KHE_MONITOR m);
extern void KheMonitorAttachToSoln(KHE_MONITOR m);
extern bool KheMonitorAttachedToSoln(KHE_MONITOR m);
extern void KheSolnEnsureOfficialCost(KHE_SOLN soln);

/* 6.4 Event monitors */
extern int KheSolnEventMonitorCount(KHE_SOLN soln, KHE_EVENT e);
extern KHE_MONITOR KheSolnEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i);

extern KHE_COST KheSolnEventCost(KHE_SOLN soln, KHE_EVENT e);
extern KHE_COST KheSolnEventMonitorCost(KHE_SOLN soln, KHE_EVENT e,
  KHE_MONITOR_TAG tag);

/* 6.4.1 Split events monitors */
extern KHE_SPLIT_EVENTS_CONSTRAINT KheSplitEventsMonitorConstraint(
  KHE_SPLIT_EVENTS_MONITOR m);
extern KHE_EVENT KheSplitEventsMonitorEvent(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorLimits(KHE_SPLIT_EVENTS_MONITOR m,
  int *min_duration, int *max_duration, int *min_amount, int *max_amount);

/* 6.4.2 Distribute split events monitors */
extern KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheDistributeSplitEventsMonitorConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern KHE_EVENT KheDistributeSplitEventsMonitorEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeEventsMonitorLimits(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m,
  int *duration, int *minimum, int *maximum, int *meet_count);

/* 6.4.3 Assign time monitors */
extern KHE_ASSIGN_TIME_CONSTRAINT KheAssignTimeMonitorConstraint(
  KHE_ASSIGN_TIME_MONITOR m);
extern KHE_EVENT KheAssignTimeMonitorEvent(KHE_ASSIGN_TIME_MONITOR m);

/* 6.4.4 Prefer times monitors */
extern KHE_PREFER_TIMES_CONSTRAINT KhePreferTimesMonitorConstraint(
  KHE_PREFER_TIMES_MONITOR m);
extern KHE_EVENT KhePreferTimesMonitorEvent(KHE_PREFER_TIMES_MONITOR m);

/* 6.4.5 Spread events monitors */
extern KHE_SPREAD_EVENTS_CONSTRAINT KheSpreadEventsMonitorConstraint(
  KHE_SPREAD_EVENTS_MONITOR m);
extern KHE_EVENT_GROUP KheSpreadEventsMonitorEventGroup(
  KHE_SPREAD_EVENTS_MONITOR m);
extern int KheSpreadEventsMonitorTimeGroupCount(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorTimeGroup(KHE_SPREAD_EVENTS_MONITOR m, int i,
  KHE_TIME_GROUP *time_group, int *minimum, int *maximum, int *incidences);

/* 6.4.6 Link events monitors */
extern KHE_LINK_EVENTS_CONSTRAINT KheLinkEventsMonitorConstraint(
  KHE_LINK_EVENTS_MONITOR m);
extern KHE_EVENT_GROUP KheLinkEventsMonitorEventGroup(
  KHE_LINK_EVENTS_MONITOR m);

/* 6.4.7 Order events monitors */
extern KHE_ORDER_EVENTS_CONSTRAINT KheOrderEventsMonitorConstraint(
  KHE_ORDER_EVENTS_MONITOR m);
extern KHE_EVENT KheOrderEventsMonitorFirstEvent(KHE_ORDER_EVENTS_MONITOR m);
extern KHE_EVENT KheOrderEventsMonitorSecondEvent(KHE_ORDER_EVENTS_MONITOR m);
extern int KheOrderEventsMonitorMinSeparation(KHE_ORDER_EVENTS_MONITOR m);
extern int KheOrderEventsMonitorMaxSeparation(KHE_ORDER_EVENTS_MONITOR m);

/* 6.5 Event resource monitors */
extern int KheSolnEventResourceMonitorCount(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er);
extern KHE_MONITOR KheSolnEventResourceMonitor(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, int i);

extern KHE_COST KheSolnEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er);
extern KHE_COST KheSolnEventResourceMonitorCost(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, KHE_MONITOR_TAG tag);

/* 6.5.1 Assign resource monitors */
extern KHE_ASSIGN_RESOURCE_CONSTRAINT KheAssignResourceMonitorConstraint(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern KHE_EVENT_RESOURCE KheAssignResourceMonitorEventResource(
  KHE_ASSIGN_RESOURCE_MONITOR m);

/* 6.5.2 Prefer resources monitors */
extern KHE_PREFER_RESOURCES_CONSTRAINT KhePreferResourcesMonitorConstraint(
  KHE_PREFER_RESOURCES_MONITOR m);
extern KHE_EVENT_RESOURCE KhePreferResourcesMonitorEventResource(
  KHE_PREFER_RESOURCES_MONITOR m);

/* 6.5.3 Avoid split assignments monitors */
extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheAvoidSplitAssignmentsMonitorConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorEventGroupIndex(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

extern int KheAvoidSplitAssignmentsMonitorResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern KHE_RESOURCE KheAvoidSplitAssignmentsMonitorResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);
extern int KheAvoidSplitAssignmentsMonitorResourceMultiplicity(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);

/* 6.6 Resource monitors */
extern int KheSolnResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_MONITOR KheSolnResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i);

extern KHE_COST KheSolnResourceCost(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_COST KheSolnResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,
  KHE_MONITOR_TAG tag);

/* 6.6.1 Avoid clashes monitors */
extern KHE_AVOID_CLASHES_CONSTRAINT KheAvoidClashesMonitorConstraint(
  KHE_AVOID_CLASHES_MONITOR m);
extern KHE_RESOURCE KheAvoidClashesMonitorResource(KHE_AVOID_CLASHES_MONITOR m);

/* 6.6.2 Avoid unavailable times monitors */
extern KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheAvoidUnavailableTimesMonitorConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern KHE_RESOURCE KheAvoidUnavailableTimesMonitorResource(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* 6.6.3 Limit idle times monitors */
extern KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern KHE_RESOURCE KheLimitIdleTimesMonitorResource(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

extern int KheLimitIdleTimesMonitorTimeGroupMonitorCount(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern KHE_TIME_GROUP_MONITOR KheLimitIdleTimesMonitorTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i);

/* 6.6.4 Cluster busy times monitors */
extern KHE_CLUSTER_BUSY_TIMES_CONSTRAINT KheClusterBusyTimesMonitorConstraint(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE KheClusterBusyTimesMonitorResource(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

extern void KheClusterBusyTimesMonitorBusyGroupCount(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int *busy_group_count, int *minimum, int *maximum);
extern int KheClusterBusyTimesMonitorTimeGroupMonitorCount(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern KHE_TIME_GROUP_MONITOR KheClusterBusyTimesMonitorTimeGroupMonitor(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i);

/* 6.6.5 Limit busy times monitors */
extern KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE KheLimitBusyTimesMonitorResource(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern int KheLimitBusyTimesMonitorDefectiveTimeGroupCount(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorDefectiveTimeGroup(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i, KHE_TIME_GROUP *tg,
  int *busy_count, int *minimum, int *maximum);

extern void KheLimitBusyTimesMonitorSetCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int ceiling);
extern int KheLimitBusyTimesMonitorCeiling(KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* 6.6.6 Limit workload monitors */
extern KHE_LIMIT_WORKLOAD_CONSTRAINT KheLimitWorkloadMonitorConstraint(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern KHE_RESOURCE KheLimitWorkloadMonitorResource(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern float KheLimitWorkloadMonitorWorkload(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorWorkloadAndLimits(
  KHE_LIMIT_WORKLOAD_MONITOR m, float *workload,
  int *minimum, int *maximum);

extern void KheLimitWorkloadMonitorSetCeiling(KHE_LIMIT_WORKLOAD_MONITOR m,
  int ceiling);
extern int KheLimitWorkloadMonitorCeiling(KHE_LIMIT_WORKLOAD_MONITOR m);

/* 6.7 Timetable monitors */
extern KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln,
  KHE_EVENT e);

extern int KheTimetableMonitorTimeMeetCount(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME time);
extern KHE_MEET KheTimetableMonitorTimeMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME time, int i);
extern bool KheTimetableMonitorTimeAvailable(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, KHE_TIME time);

extern int KheTimetableMonitorClashingTimeCount(KHE_TIMETABLE_MONITOR tm);
extern KHE_TIME KheTimetableMonitorClashingTime(KHE_TIMETABLE_MONITOR tm,int i);

extern void KheTimetableMonitorPrintTimetable(KHE_TIMETABLE_MONITOR tm,
  int cell_width, int indent, FILE *fp);

/* 6.8 Time group monitors */
extern KHE_TIMETABLE_MONITOR KheTimeGroupMonitorTimetableMonitor(
  KHE_TIME_GROUP_MONITOR m);
extern KHE_TIME_GROUP KheTimeGroupMonitorTimeGroup(KHE_TIME_GROUP_MONITOR m);
extern int KheTimeGroupMonitorBusyCount(KHE_TIME_GROUP_MONITOR m);
extern int KheTimeGroupMonitorIdleCount(KHE_TIME_GROUP_MONITOR m);
extern void KheTimeGroupMonitorFirstAndLastBusyTimes(
  KHE_TIME_GROUP_MONITOR tgm, KHE_TIME times[2], int *count);

/* 6.9.1 Basic operations on group monitors */
extern KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label);
extern int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm);
extern char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm);
extern void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm);

extern void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m);
extern void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm,
  KHE_MONITOR m);
extern bool KheGroupMonitorHasChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m);
extern void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm);
extern void KheSolnBypassAndDeleteAllGroupMonitors(KHE_SOLN soln);

extern int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm);
extern KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i);
extern int KheSolnChildMonitorCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i);

extern int KheMonitorParentMonitorCount(KHE_MONITOR m);
extern KHE_GROUP_MONITOR KheMonitorParentMonitor(KHE_MONITOR m, int i);
extern bool KheMonitorDescendant(KHE_MONITOR m1, KHE_MONITOR m2);

/* 6.9.2 Defects */
extern int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm);
extern KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i);
extern int KheSolnDefectCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i);

extern void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);

extern KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm,
  KHE_MONITOR_TAG tag, int *defect_count);
extern KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,
  int *defect_count);

extern void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);
extern void KheSolnCostByTypeDebug(KHE_SOLN soln,
  int verbosity, int indent, FILE *fp);

/* 6.9.3 Tracing */
extern KHE_TRACE KheTraceMake(KHE_GROUP_MONITOR gm);
extern KHE_GROUP_MONITOR KheTraceGroupMonitor(KHE_TRACE t);
extern void KheTraceDelete(KHE_TRACE t);
extern void KheTraceBegin(KHE_TRACE t);
extern void KheTraceEnd(KHE_TRACE t);

extern KHE_COST KheTraceInitCost(KHE_TRACE t);
extern int KheTraceMonitorCount(KHE_TRACE t);
extern KHE_MONITOR KheTraceMonitor(KHE_TRACE t, int i);
extern KHE_COST KheTraceMonitorInitCost(KHE_TRACE t, int i);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 7.   Matchings and Evenness                                    */
/*                                                                           */
/*****************************************************************************/

/* 7.2 Setting up */
extern void KheSolnMatchingBegin(KHE_SOLN soln);
extern void KheSolnMatchingEnd(KHE_SOLN soln);
extern bool KheSolnHasMatching(KHE_SOLN soln);
extern void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST weight);
extern KHE_COST KheSolnMatchingWeight(KHE_SOLN soln);
extern KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln);
extern KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln);
extern void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt);
extern void KheSolnMatchingMarkBegin(KHE_SOLN soln);
extern void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo);
extern void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity,
  int indent, FILE *fp);

/* 7.3 Ordinary supply and demand nodes */
extern int KheTaskDemandMonitorCount(KHE_TASK task);
extern KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i);
extern KHE_TASK KheOrdinaryDemandMonitorTask(KHE_ORDINARY_DEMAND_MONITOR m);
extern int KheOrdinaryDemandMonitorOffset(KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln);
extern void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln);

/* 7.4 Workload demand nodes */
extern void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln);
extern int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg, KHE_MONITOR *m);
extern void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int num, KHE_TIME_GROUP tg, KHE_MONITOR m);
extern void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_RESOURCE KheWorkloadDemandMonitorResource(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern KHE_TIME_GROUP KheWorkloadDemandMonitorTimeGroup(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern KHE_MONITOR KheWorkloadDemandMonitorOriginatingMonitor(
  KHE_WORKLOAD_DEMAND_MONITOR m);

/* 7.5.1 Visiting unmatched demand nodes */
extern int KheSolnMatchingDefectCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i);

/* 7.5.2 Hall sets */
extern int KheSolnMatchingHallSetCount(KHE_SOLN soln);
extern int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i);
extern int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i);
extern bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,
  int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r);
extern KHE_MONITOR KheSolnMatchingHallSetDemandNode(KHE_SOLN soln,
  int i, int j);
extern void KheSolnMatchingHallSetsDebug(KHE_SOLN soln,
  int verbosity, int indent, FILE *fp);

/* 7.5.3 Finding competitors */
extern void KheSolnMatchingSetCompetitors(KHE_SOLN soln, KHE_MONITOR m);
extern int KheSolnMatchingCompetitorCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnMatchingCompetitor(KHE_SOLN soln, int i);

/* 7.6 Evenness monitoring */
extern void KheSolnEvennessBegin(KHE_SOLN soln);
extern void KheSolnEvennessEnd(KHE_SOLN soln);
extern bool KheSolnHasEvenness(KHE_SOLN soln);
extern void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln);
extern void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln);
extern KHE_RESOURCE_GROUP KheEvennessMonitorPartition(KHE_EVENNESS_MONITOR m);
extern KHE_TIME KheEvennessMonitorTime(KHE_EVENNESS_MONITOR m);
extern int KheEvennessMonitorCount(KHE_EVENNESS_MONITOR m);
extern int KheEvennessMonitorLimit(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorSetLimit(KHE_EVENNESS_MONITOR m, int limit);
extern KHE_COST KheEvennessMonitorWeight(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorSetWeight(KHE_EVENNESS_MONITOR m,
  KHE_COST weight);
extern void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight);


/*****************************************************************************/
/*                                                                           */
/*                    Part B:  Solving                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter  8.  Introducing Solving                                       */
/*                                                                           */
/*****************************************************************************/

/* 8.1 General solving */
extern KHE_SOLN KheGeneralSolve2014(KHE_SOLN soln, KHE_OPTIONS options);

/* 8.2 Parallel solving */
extern void KheArchiveParallelSolve(KHE_ARCHIVE archive, int thread_count,
  int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options,
  int keep_solns, KHE_SOLN_GROUP soln_group);
extern KHE_SOLN KheInstanceParallelSolve(KHE_INSTANCE ins, int thread_count,
  int make_solns, KHE_GENERAL_SOLVER solver, KHE_OPTIONS options);

/* 8.3 Benchmarking */
extern bool KheBenchmarkTryInstance(KHE_INSTANCE ins);
extern void KheBenchmark(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, char test_label,
  KHE_STATS_TABLE_TYPE table_type);

/* 8.4 Options */
extern KHE_OPTIONS KheOptionsMake(void);
extern void KheOptionsDelete(KHE_OPTIONS options);
extern KHE_OPTIONS KheOptionsCopy(KHE_OPTIONS options);

/* 8.4.1 General options */
extern bool KheOptionsDiversify(KHE_OPTIONS options);
extern void KheOptionsSetDiversify(KHE_OPTIONS options, bool diversify);
extern bool KheOptionsMonitorEvenness(KHE_OPTIONS options);
extern void KheOptionsSetMonitorEvenness(KHE_OPTIONS options,
  bool monitor_evenness);
extern bool KheOptionsTimeAssignmentOnly(KHE_OPTIONS options);
extern void KheOptionsSetTimeAssignmentOnly(KHE_OPTIONS options,
  bool time_assignment_only);

/* 8.4.2 Structural solver options */
extern KHE_TIME_EQUIV KheOptionsStructuralTimeEquiv(KHE_OPTIONS options);
extern void KheOptionsSetStructuralTimeEquiv(KHE_OPTIONS options,
  KHE_TIME_EQUIV structural_time_equiv);
extern KHE_SPLIT_ANALYSER KheOptionsStructuralSplitAnalyser(
  KHE_OPTIONS options);
extern void KheOptionsSetStructuralSplitAnalyser(KHE_OPTIONS options,
  KHE_SPLIT_ANALYSER structural_split_analyser);

/* 8.4.3 Time solver options */
extern bool KheOptionsTimeClusterMeetDomains(KHE_OPTIONS options);
extern void KheOptionsSetTimeClusterMeetDomains(KHE_OPTIONS options,
  bool time_cluster_meet_domains);
extern bool KheOptionsTimeTightenDomains(KHE_OPTIONS options);
extern void KheOptionsSetTimeTightenDomains(KHE_OPTIONS options,
  bool time_tighten_domains);
extern bool KheOptionsTimeNodeRepair(KHE_OPTIONS options);
extern void KheOptionsSetTimeNodeRepair(KHE_OPTIONS options,
  bool time_node_repair);
extern bool KheOptionsTimeNodeRegularity(KHE_OPTIONS options);
extern void KheOptionsSetTimeNodeRegularity(KHE_OPTIONS options,
  bool time_node_regularity);
extern bool KheOptionsTimeLayerSwap(KHE_OPTIONS options);
extern void KheOptionsSetTimeLayerSwap(KHE_OPTIONS options,
  bool time_layer_swap);
extern bool KheOptionsTimeLayerRepair(KHE_OPTIONS options);
extern void KheOptionsSetTimeLayerRepair(KHE_OPTIONS options,
  bool time_layer_repair);
extern bool KheOptionsTimeLayerRepairBackoff(KHE_OPTIONS options);
extern void KheOptionsSetTimeLayerRepairBackoff(KHE_OPTIONS options,
  bool time_layer_repair_backoff);
extern bool KheOptionsTimeLayerRepairLong(KHE_OPTIONS options);
extern void KheOptionsSetTimeLayerRepairLong(KHE_OPTIONS options,
  bool time_layer_repair_long);
extern KHE_KEMPE_STATS KheOptionsTimeKempeStats(KHE_OPTIONS options);
extern void KheOptionsSetTimeKempeStats(KHE_OPTIONS options,
  KHE_KEMPE_STATS time_kempe_stats);

/* 8.4.4 Resource solver options */
extern bool KheOptionsResourceInvariant(KHE_OPTIONS options);
extern void KheOptionsSetResourceInvariant(KHE_OPTIONS options,
  bool resource_invariant);
extern bool KheOptionsResourceRematch(KHE_OPTIONS options);
extern void KheOptionsSetResourceRematch(KHE_OPTIONS options,
  bool resource_rematch);
extern KHE_OPTIONS_RESOURCE_PAIR KheOptionsResourcePair(KHE_OPTIONS options);
extern void KheOptionsSetResourcePair(KHE_OPTIONS options,
  KHE_OPTIONS_RESOURCE_PAIR resource_pair);
extern int KheOptionsResourcePairCalls(KHE_OPTIONS options);
extern void KheOptionsSetResourcePairCalls(KHE_OPTIONS options,
  int resource_pair_calls);
extern int KheOptionsResourcePairSuccesses(KHE_OPTIONS options);
extern void KheOptionsSetResourcePairSuccesses(KHE_OPTIONS options,
  int resource_pair_successes);
extern int KheOptionsResourcePairTruncs(KHE_OPTIONS options);
extern void KheOptionsSetResourcePairTruncs(KHE_OPTIONS options,
  int resource_pair_truncs);

/* 8.4.5 Ejection chain options */
extern KHE_EJECTOR KheOptionsEjector(KHE_OPTIONS options, int index);
extern void KheOptionsSetEjector(KHE_OPTIONS options, int index,
  KHE_EJECTOR ej);
extern char *KheOptionsEjectorSchedulesString(KHE_OPTIONS options);
extern void KheOptionsSetEjectorSchedulesString(KHE_OPTIONS options,
  char *ejector_schedules_string);
extern bool KheOptionsEjectorPromoteDefects(KHE_OPTIONS options);
extern void KheOptionsSetEjectorPromoteDefects(KHE_OPTIONS options,
  bool ejector_promote_defects);
extern bool KheOptionsEjectorFreshVisits(KHE_OPTIONS options);
extern void KheOptionsSetEjectorFreshVisits(KHE_OPTIONS options,
  bool ejector_fresh_visits);
extern bool KheOptionsEjectorRepairTimes(KHE_OPTIONS options);
extern void KheOptionsSetEjectorRepairTimes(KHE_OPTIONS options,
  bool ejector_repair_times);
extern bool KheOptionsEjectorVizierNode(KHE_OPTIONS options);
extern void KheOptionsSetEjectorVizierNode(KHE_OPTIONS options,
  bool ejector_vizier_node);
extern bool KheOptionsEjectorNodesBeforeMeets(KHE_OPTIONS options);
extern void KheOptionsSetEjectorNodesBeforeMeets(KHE_OPTIONS options,
  bool ejector_nodes_before_meets);
extern KHE_OPTIONS_KEMPE KheOptionsEjectorUseKempeMoves(KHE_OPTIONS options);
extern void KheOptionsSetEjectorUseKempeMoves(KHE_OPTIONS options,
  KHE_OPTIONS_KEMPE ejector_use_kempe_moves);
extern bool KheOptionsEjectorUseFuzzyMoves(KHE_OPTIONS options);
extern void KheOptionsSetEjectorUseFuzzyMoves(KHE_OPTIONS options,
  bool ejector_use_fuzzy_moves);
extern bool KheOptionsEjectorUseSplitMoves(KHE_OPTIONS options);
extern void KheOptionsSetEjectorUseSplitMoves(KHE_OPTIONS options,
  bool ejector_use_split_moves);
extern bool KheOptionsEjectorEjectingNotBasic(KHE_OPTIONS options);
extern void KheOptionsSetEjectorEjectingNotBasic(KHE_OPTIONS options,
  bool ejector_ejecting_not_basic);
extern KHE_NODE KheOptionsEjectorLimitNode(KHE_OPTIONS options);
extern void KheOptionsSetEjectorLimitNode(KHE_OPTIONS options,
  KHE_NODE ejector_limit_node);
extern bool KheOptionsEjectorRepairResources(KHE_OPTIONS options);
extern void KheOptionsSetEjectorRepairResources(KHE_OPTIONS options,
  bool ejector_repair_resources);
extern int KheOptionsEjectorLimitDefects(KHE_OPTIONS options);
extern void KheOptionsSetEjectorLimitDefects(KHE_OPTIONS options,
  int ejector_limit_defects);

/* 8.5 Gathering statistics */
extern KHE_STATS_TIMER KheStatsTimerMake(void);
extern void KheStatsTimerReset(KHE_STATS_TIMER st);
extern float KheStatsTimerNow(KHE_STATS_TIMER st);
extern void KheStatsTimerDelete(KHE_STATS_TIMER st);
extern KHE_STATS_TIMER KheStatsTimerCopy(KHE_STATS_TIMER st);

extern char *KheStatsDateToday(void);

extern void KheStatsFileBegin(char *file_name);
extern void KheStatsFileEnd(char *file_name);

extern void KheStatsTableBegin(char *file_name, KHE_STATS_TABLE_TYPE table_type,
  int col_width, char *corner, bool with_average_row, bool with_total_row,
  bool highlight_cost_minima, bool highlight_time_minima,
  bool highlight_int_minima);
extern void KheStatsTableEnd(char *file_name);
extern void KheStatsCaptionAdd(char *file_name, char *fmt, ...);
extern void KheStatsRowAdd(char *file_name, char *row_label, bool rule_below);
extern void KheStatsColAdd(char *file_name, char *col_label, bool rule_after);

extern void KheStatsEntryAddString(char *file_name, char *row_label,
  char *col_label, char *str);
extern void KheStatsEntryAddCost(char *file_name, char *row_label,
  char *col_label, KHE_COST cost);
extern void KheStatsEntryAddTime(char *file_name, char *row_label,
  char *col_label, float time);
extern void KheStatsEntryAddInt(char *file_name, char *row_label,
  char *col_label, int val);

extern void KheStatsGraphBegin(char *file_name);
extern void KheStatsGraphEnd(char *file_name);
extern void KheStatsGraphSetWidth(char *file_name, float width);
extern void KheStatsGraphSetHeight(char *file_name, float height);
extern void KheStatsGraphSetXMax(char *file_name, float xmax);
extern void KheStatsGraphSetYMax(char *file_name, float ymax);
extern void KheStatsGraphSetAboveCaption(char *file_name, char *val);
extern void KheStatsGraphSetBelowCaption(char *file_name, char *val);
extern void KheStatsGraphSetLeftCaption(char *file_name, char *val);
extern void KheStatsGraphSetRightCaption(char *file_name, char *val);
extern void KheStatsDataSetAdd(char *file_name, char *dataset_label,
  KHE_STATS_DATASET_TYPE dataset_type);
extern void KheStatsPointAdd(char *file_name, char *dataset_label,
  float x, float y);

/* 8.6 Exponential backoff */
extern KHE_BACKOFF KheBackoffBegin(KHE_BACKOFF_TYPE backoff_type);
extern bool KheBackoffAcceptOpportunity(KHE_BACKOFF bk);
extern void KheBackoffResult(KHE_BACKOFF bk, bool success);
extern void KheBackoffEnd(KHE_BACKOFF bk);

extern char *KheBackoffShowNextDecision(KHE_BACKOFF bk);
extern void KheBackoffDebug(KHE_BACKOFF bk, int verbosity, int indent,FILE *fp);
extern void KheBackoffTest(FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 9.   Structural Solvers                                        */
/*                                                                           */
/*****************************************************************************/

/* 9.1 Layer tree construction */
extern KHE_NODE KheLayerTreeMake(KHE_SOLN soln);

/* 9.2 Time-equivalence */
extern KHE_TIME_EQUIV KheTimeEquivMake(void);
extern void KheTimeEquivDelete(KHE_TIME_EQUIV te);
extern void KheTimeEquivSolve(KHE_TIME_EQUIV te, KHE_SOLN soln);

extern int KheTimeEquivEventGroupCount(KHE_TIME_EQUIV te);
extern KHE_EVENT_GROUP KheTimeEquivEventGroup(KHE_TIME_EQUIV te, int i);
extern KHE_EVENT_GROUP KheTimeEquivEventEventGroup(KHE_TIME_EQUIV te,
  KHE_EVENT e);
extern int KheTimeEquivEventEventGroupIndex(KHE_TIME_EQUIV te, KHE_EVENT e);

extern int KheTimeEquivResourceGroupCount(KHE_TIME_EQUIV te);
extern KHE_RESOURCE_GROUP KheTimeEquivResourceGroup(KHE_TIME_EQUIV te, int i);
extern KHE_RESOURCE_GROUP KheTimeEquivResourceResourceGroup(KHE_TIME_EQUIV te,
  KHE_RESOURCE r);
extern int KheTimeEquivResourceResourceGroupIndex(KHE_TIME_EQUIV te,
  KHE_RESOURCE r);

/* 9.3.1 Layer construction */
extern KHE_LAYER KheLayerMakeFromResource(KHE_NODE parent_node,
  KHE_RESOURCE r);
extern void KheNodeChildLayersMake(KHE_NODE parent_node);
extern void KheNodeChildLayersReduce(KHE_NODE parent_node);

/* 9.3.2 Layer coordination */
extern void KheCoordinateLayers(KHE_NODE parent_node, bool with_domination);

/* 9.4 Runarounds */
extern bool KheMinimumRunaroundDuration(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER time_solver, KHE_OPTIONS options, int *duration);
extern void KheBuildRunarounds(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER mrd_solver, KHE_OPTIONS mrd_options,
  KHE_NODE_TIME_SOLVER runaround_solver, KHE_OPTIONS runaround_options);

/* 9.5.1 Node merging */
extern bool KheNodeMergeCheck(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeMerge(KHE_NODE node1, KHE_NODE node2, KHE_NODE *res);

/* 9.5.2 Node meet splitting and merging */
extern void KheNodeMeetSplit(KHE_NODE node, bool recursive);
extern void KheNodeMeetMerge(KHE_NODE node, bool recursive);

/* 9.5.3 Node moving */
extern bool KheNodeMoveCheck(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeMove(KHE_NODE child_node, KHE_NODE parent_node);

/* 9.5.4 Vizier nodes */
extern KHE_NODE KheNodeVizierMake(KHE_NODE parent_node);
extern void KheNodeVizierDelete(KHE_NODE parent_node);

/* 9.5.5 Flattening */
extern void KheNodeBypass(KHE_NODE node);
extern void KheNodeFlatten(KHE_NODE parent_node);

/* 9.6 Adding zones */
extern void KheLayerInstallZonesInParent(KHE_LAYER layer);
extern void KheNodeExtendZones(KHE_NODE node);

/* 9.7.1 Analysing split defects */
extern KHE_SPLIT_ANALYSER KheSplitAnalyserMake(void);
extern void KheSplitAnalyserDelete(KHE_SPLIT_ANALYSER sa);
extern void KheSplitAnalyserAnalyse(KHE_SPLIT_ANALYSER sa,
  KHE_SOLN soln, KHE_EVENT e);
extern int KheSplitAnalyserSplitSuggestionCount(KHE_SPLIT_ANALYSER sa);
extern void KheSplitAnalyserSplitSuggestion(KHE_SPLIT_ANALYSER sa, int i,
  int *merged_durn, int *split1_durn);
extern int KheSplitAnalyserMergeSuggestionCount(KHE_SPLIT_ANALYSER sa);
extern void KheSplitAnalyserMergeSuggestion(KHE_SPLIT_ANALYSER sa, int i,
  int *split1_durn, int *split2_durn);
extern void KheSplitAnalyserDebug(KHE_SPLIT_ANALYSER sa, int verbosity,
  int indent, FILE *fp);

/* 9.7.2 Merging adjacent meets */
extern void KheMergeMeets(KHE_SOLN soln);

/* 9.8 Monitor attachment and grouping */
extern char *KheSubTagLabel(KHE_SUBTAG_STANDARD_TYPE sub_tag);
extern KHE_SUBTAG_STANDARD_TYPE KheSubTagFromTag(KHE_MONITOR_TAG tag);
extern bool KheMonitorHasParent(KHE_MONITOR m, int sub_tag,
  KHE_GROUP_MONITOR *res_gm);
extern void KheMonitorAddSelfOrParent(KHE_MONITOR m, int sub_tag,
  KHE_GROUP_MONITOR gm);
extern void KheMonitorDeleteAllParentsRecursive(KHE_MONITOR m);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 10.   Time Solvers                                             */
/*                                                                           */
/*****************************************************************************/

/* 10.2.1 Node assignment functions */
extern bool KheNodeMeetSwapCheck(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeMeetSwap(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeMeetRegularAssignCheck(KHE_NODE node, KHE_NODE sibling_node);
extern bool KheNodeMeetRegularAssign(KHE_NODE node, KHE_NODE sibling_node);
extern void KheNodeMeetUnAssign(KHE_NODE node);

/* 10.2.2 Kempe and ejecting meet moves */
extern KHE_GROUP_MONITOR KheKempeDemandGroupMonitorMake(KHE_SOLN soln);
extern bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand, bool *basic,
  KHE_KEMPE_STATS kempe_stats);
extern bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand, bool *basic,
  KHE_KEMPE_STATS kempe_stats);

extern KHE_KEMPE_STATS KheKempeStatsMake(void);
extern void KheKempeStatsDelete(KHE_KEMPE_STATS kempe_stats);

extern int KheKempeStatsStepHistoMax(KHE_KEMPE_STATS kempe_stats);
extern int KheKempeStatsStepHistoFrequency(KHE_KEMPE_STATS kempe_stats,
  int step_count);
extern int KheKempeStatsStepHistoTotal(KHE_KEMPE_STATS kempe_stats);
extern float KheKempeStatsStepHistoAverage(KHE_KEMPE_STATS kempe_stats);

extern int KheKempeStatsPhaseHistoMax(KHE_KEMPE_STATS kempe_stats);
extern int KheKempeStatsPhaseHistoFrequency(KHE_KEMPE_STATS kempe_stats,
  int phase_count);
extern int KheKempeStatsPhaseHistoTotal(KHE_KEMPE_STATS kempe_stats);
extern float KheKempeStatsPhaseHistoAverage(KHE_KEMPE_STATS kempe_stats);

extern bool KheEjectingMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand, bool *basic);
extern bool KheEjectingMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand, bool *basic);
extern bool KheBasicMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int offset, bool preserve_regularity, int *demand);
extern bool KheBasicMeetMoveTime(KHE_MEET meet, KHE_TIME t,
  bool preserve_regularity, int *demand);

/* 10.3.1 Meet bound groups */
extern KHE_MEET_BOUND_GROUP KheMeetBoundGroupMake(void);
extern void KheMeetBoundGroupAddMeetBound(KHE_MEET_BOUND_GROUP mbg,
  KHE_MEET_BOUND mb);
extern int KheMeetBoundGroupMeetBoundCount(KHE_MEET_BOUND_GROUP mbg);
extern KHE_MEET_BOUND KheMeetBoundGroupMeetBound(KHE_MEET_BOUND_GROUP mbg,
  int i);
extern bool KheMeetBoundGroupDelete(KHE_MEET_BOUND_GROUP mbg);

/* 10.3.2 Exposing resource unavailability */
extern void KheMeetAddUnavailableBound(KHE_MEET meet, KHE_COST min_weight,
  KHE_MEET_BOUND_GROUP mbg);
extern void KheSolnAddUnavailableBounds(KHE_SOLN soln, KHE_COST min_weight,
  KHE_MEET_BOUND_GROUP mbg);

/* 10.3.3 Preventing cluster busy times defects */
/* ***
extern void KheSolnClusterMeetDomains(KHE_SOLN soln, KHE_COST min_weight,
  KHE_MEET_BOUND_GROUP mbg);
*** */

/* 10.3.3 Preventing cluster busy times and limit idle times defects */
extern void KheSolnClusterAndLimitMeetDomains(KHE_SOLN soln,
  KHE_COST min_cluster_weight, KHE_COST min_idle_weight,
  float slack, KHE_MEET_BOUND_GROUP mbg, KHE_OPTIONS options);

/* 10.4 Some basic time solvers */
extern bool KheNodeSimpleAssignTimes(KHE_NODE parent_node, KHE_OPTIONS options);
extern bool KheLayerSimpleAssignTimes(KHE_LAYER layer, KHE_OPTIONS options);

extern bool KheNodeRecursiveAssignTimes(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER solver, KHE_OPTIONS options);

extern bool KheNodeUnAssignTimes(KHE_NODE parent_node, KHE_OPTIONS options);
extern bool KheLayerUnAssignTimes(KHE_LAYER layer, KHE_OPTIONS options);

extern bool KheNodeAllChildMeetsAssigned(KHE_NODE parent_node);
extern bool KheLayerAllChildMeetsAssigned(KHE_LAYER layer);

extern bool KheNodePreassignedAssignTimes(KHE_NODE root_node,
  KHE_OPTIONS options);
extern bool KheLayerPreassignedAssignTimes(KHE_LAYER layer,
  KHE_OPTIONS options);

extern bool KheSolnTryMeetUnAssignments(KHE_SOLN soln);

/* 10.5 A time solver for runarounds */
extern bool KheRunaroundNodeAssignTimes(KHE_NODE parent_node,
  KHE_OPTIONS options);

/* 10.6 Extended layer matching with Elm */
extern bool KheElmLayerAssign(KHE_LAYER layer,
  KHE_SPREAD_EVENTS_CONSTRAINT sec, KHE_OPTIONS options);

/* 10.7.1 Node-regular time repair using layer node matching */
extern bool KheLayerNodeMatchingNodeRepairTimes(KHE_NODE parent_node,
  KHE_OPTIONS options);
extern bool KheLayerNodeMatchingLayerRepairTimes(KHE_LAYER layer,
  KHE_OPTIONS options);

/* 10.7.3 Tree search layer time repair */
extern bool KheTreeSearchLayerRepairTimes(KHE_SOLN soln, KHE_RESOURCE r);
extern bool KheTreeSearchRepairTimes(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  bool with_defects);

/* 10.7.4 Meet set time repair */
extern KHE_MEET_SET_SOLVER KheMeetSetSolveBegin(KHE_SOLN soln, int max_meets);
extern void KheMeetSetSolveAddMeet(KHE_MEET_SET_SOLVER mss, KHE_MEET meet);
extern bool KheMeetSetSolveEnd(KHE_MEET_SET_SOLVER mss);

extern bool KheFuzzyMeetMove(KHE_MEET meet, KHE_MEET target_meet, int offset,
  int width, int depth, int max_meets);

/* 10.8.1 Layer assignments */
extern KHE_LAYER_ASST KheLayerAsstMake(void);
extern void KheLayerAsstDelete(KHE_LAYER_ASST layer_asst);
extern void KheLayerAsstBegin(KHE_LAYER_ASST layer_asst, KHE_LAYER layer);
extern void KheLayerAsstEnd(KHE_LAYER_ASST layer_asst);
extern void KheLayerAsstUndo(KHE_LAYER_ASST layer_asst);
extern void KheLayerAsstRedo(KHE_LAYER_ASST layer_asst);
extern void KheLayerAsstDebug(KHE_LAYER_ASST layer_asst, int verbosity,
  int indent, FILE *fp);

/* 10.8.2 A solver for layered time assignment */
extern bool KheNodeLayeredAssignTimes(KHE_NODE parent_node,KHE_OPTIONS options);
extern int KheNodeLayeredLayerCmp(const void *t1, const void *t2);
extern bool KheLayerParallelAssignTimes(KHE_LAYER layer, KHE_OPTIONS options);

/* 10.8.3 A complete time solver */
extern bool KheCycleNodeAssignTimes(KHE_NODE cycle_node, KHE_OPTIONS options);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 11.  Resource Solvers                                          */
/*                                                                           */
/*****************************************************************************/

/* 11.2 The resource assignment invariant */
extern void KheAtomicOperationBegin(KHE_SOLN soln, KHE_MARK *mark,
  int *init_count, bool resource_invariant);
extern bool KheAtomicOperationEnd(KHE_SOLN soln, KHE_MARK *mark,
  int *init_count, bool resource_invariant, bool success);
extern void KheDisconnectAllDemandMonitors(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);

/* 11.3.1 Task bound groups */
extern KHE_TASK_BOUND_GROUP KheTaskBoundGroupMake(void);
extern void KheTaskBoundGroupAddTaskBound(KHE_TASK_BOUND_GROUP tbg,
  KHE_TASK_BOUND tb);
extern int KheTaskBoundGroupTaskBoundCount(KHE_TASK_BOUND_GROUP tbg);
extern KHE_TASK_BOUND KheTaskBoundGroupTaskBound(KHE_TASK_BOUND_GROUP tbg,
  int i);
extern bool KheTaskBoundGroupDelete(KHE_TASK_BOUND_GROUP tbg);

/* 11.3.3 Task tree construction */
extern void KheTaskTreeMake(KHE_SOLN soln, KHE_TASK_JOB_TYPE tjt,
  KHE_OPTIONS options);
extern KHE_TASKING KheTaskingMakeFromResourceType(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt);
extern bool KheTaskingMakeTaskTree(KHE_TASKING tasking, KHE_TASK_JOB_TYPE tjt,
  KHE_TASK_BOUND_GROUP tbg, KHE_OPTIONS options);

/* 11.3.4 Other task tree solvers */
extern bool KheTaskingTightenToPartition(KHE_TASKING tasking,
  KHE_TASK_BOUND_GROUP tbg, KHE_OPTIONS options);
extern void KheTaskingAllowSplitAssignments(KHE_TASKING tasking,
  bool unassigned_only);
extern void KheTaskingEnlargeDomains(KHE_TASKING tasking, bool unassigned_only);

/* 11.3.5 Task groups */
extern KHE_TASK_GROUPS KheTaskGroupsMakeFromTasking(KHE_TASKING tasking);
extern void KheTaskGroupsDelete(KHE_TASK_GROUPS task_groups);
extern int KheTaskGroupsTaskGroupCount(KHE_TASK_GROUPS task_groups);
extern KHE_TASK_GROUP KheTaskGroupsTaskGroup(KHE_TASK_GROUPS task_groups,
  int i);
extern int KheTaskGroupTaskCount(KHE_TASK_GROUP task_group);
extern KHE_TASK KheTaskGroupTask(KHE_TASK_GROUP task_group, int i);

extern int KheTaskGroupTotalDuration(KHE_TASK_GROUP task_group);
extern float KheTaskGroupTotalWorkload(KHE_TASK_GROUP task_group);
extern KHE_RESOURCE_GROUP KheTaskGroupDomain(KHE_TASK_GROUP task_group);
extern int KheTaskGroupDecreasingDurationCmp(KHE_TASK_GROUP tg1,
  KHE_TASK_GROUP tg2);

extern int KheTaskGroupUnassignedTaskCount(KHE_TASK_GROUP task_group);
extern bool KheTaskGroupAssignCheck(KHE_TASK_GROUP task_group, KHE_RESOURCE r);
extern bool KheTaskGroupAssign(KHE_TASK_GROUP task_group, KHE_RESOURCE r);
extern void KheTaskGroupUnAssign(KHE_TASK_GROUP task_group, KHE_RESOURCE r);

extern void KheTaskGroupDebug(KHE_TASK_GROUP task_group, int verbosity,
  int indent, FILE *fp);
extern void KheTaskGroupsDebug(KHE_TASK_GROUPS task_groups, int verbosity,
  int indent, FILE *fp);

/* 11.4 Most-constrained-first assignment */
extern bool KheMostConstrainedFirstAssignResources(KHE_TASKING tasking,
  KHE_OPTIONS options);

/* 11.5 Resource packing */
extern bool KheResourcePackAssignResources(KHE_TASKING tasking,
  KHE_OPTIONS options);

/* 11.6 Split assignments */
extern bool KheFindSplitResourceAssignments(KHE_TASKING tasking,
  KHE_OPTIONS options);

/* 11.7 Kempe and ejecting resource moves */
extern bool KheTaskEjectingMoveResource(KHE_TASK task, KHE_RESOURCE r);

/* 11.9 Resource pair repair */
extern bool KheResourcePairReassign(KHE_SOLN soln, KHE_RESOURCE r1,
  KHE_RESOURCE r2, bool resource_invariant, bool fix_splits);
extern bool KheResourcePairRepair(KHE_TASKING tasking, KHE_OPTIONS options);

/* 11.10 Resource rematching */
extern bool KheResourceRematch(KHE_TASKING tasking, KHE_OPTIONS options);

/* 11.11 Trying unassignments */
extern bool KheSolnTryTaskUnAssignments(KHE_SOLN soln);

/* 11.12 Putting it all together */
extern bool KheTaskingAssignResourcesStage1(KHE_TASKING tasking,
  KHE_OPTIONS options);
extern bool KheTaskingAssignResourcesStage2(KHE_TASKING tasking,
  KHE_OPTIONS options);
extern bool KheTaskingAssignResourcesStage3(KHE_TASKING tasking,
  KHE_OPTIONS options);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 12.  Ejection Chains                                           */
/*                                                                           */
/*****************************************************************************/

/* 12.2 Ejector construction */
extern KHE_EJECTOR KheEjectorMakeBegin(void);
extern void KheEjectorMakeEnd(KHE_EJECTOR ej);
extern KHE_EJECTOR KheEjectorCopy(KHE_EJECTOR ej);
extern void KheEjectorDelete(KHE_EJECTOR ej);

/* 12.2 Ejector construction - major schedules */
extern int KheEjectorMajorScheduleCount(KHE_EJECTOR ej);
extern KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorMajorSchedule(KHE_EJECTOR ej,
  int i);

extern void KheEjectorMajorScheduleBegin(KHE_EJECTOR ej);
extern void KheEjectorMajorScheduleEnd(KHE_EJECTOR ej);
extern int KheEjectorMajorScheduleMinorScheduleCount(
  KHE_EJECTOR_MAJOR_SCHEDULE ejm);
extern KHE_EJECTOR_MINOR_SCHEDULE KheEjectorMajorScheduleMinorSchedule(
  KHE_EJECTOR_MAJOR_SCHEDULE ejm, int i);

/* 12.2 Ejector construction - minor schedules */
extern void KheEjectorMinorScheduleAdd(KHE_EJECTOR ej,
  int max_depth, bool may_revisit);
extern int KheEjectorMinorScheduleMaxDepth(KHE_EJECTOR_MINOR_SCHEDULE ejms);
extern bool KheEjectorMinorScheduleMayRevisit(KHE_EJECTOR_MINOR_SCHEDULE ejms);

/* 12.2 Ejector construction - other schedules */
extern void KheEjectorAddDefaultSchedules(KHE_EJECTOR ej);
extern void KheEjectorSetSchedulesFromString(KHE_EJECTOR ej,
  char *ejector_schedules_string);

/* 12.2 Ejector construction - augment functions */
extern void KheEjectorAddAugment(KHE_EJECTOR ej, KHE_MONITOR_TAG tag,
  KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type);
extern void KheEjectorAddGroupAugment(KHE_EJECTOR ej, int sub_tag,
  KHE_EJECTOR_AUGMENT_FN augment_fn, int augment_type);

/* 12.3 Ejector solving */
extern bool KheEjectorSolve(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,
  KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options);
extern void KheEjectorSolveBegin(KHE_EJECTOR ej, KHE_GROUP_MONITOR start_gm,
  KHE_GROUP_MONITOR continue_gm, KHE_OPTIONS options);
extern bool KheEjectorSolveEnd(KHE_EJECTOR ej);

/* 12.3 Ejector solving - cost limits */
extern void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej,
  KHE_MONITOR m, KHE_COST cost_limit);
extern void KheEjectorAddMonitorCostLimitReducing(KHE_EJECTOR ej,
  KHE_MONITOR m);
extern int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej);
extern void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,
  KHE_MONITOR *m, KHE_COST *cost_limit, bool *reducing);

/* 12.3 Ejector solving - queries */
extern KHE_GROUP_MONITOR KheEjectorStartGroupMonitor(KHE_EJECTOR ej);
extern KHE_GROUP_MONITOR KheEjectorContinueGroupMonitor(KHE_EJECTOR ej);
extern KHE_OPTIONS KheEjectorOptions(KHE_EJECTOR ej);
extern KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej);
extern KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej);
extern KHE_EJECTOR_MAJOR_SCHEDULE KheEjectorCurrMajorSchedule(KHE_EJECTOR ej);
extern KHE_EJECTOR_MINOR_SCHEDULE KheEjectorCurrMinorSchedule(KHE_EJECTOR ej);
extern bool KheEjectorCurrMayRevisit(KHE_EJECTOR ej);
extern int KheEjectorCurrDepth(KHE_EJECTOR ej);
extern int KheEjectorCurrAugmentCount(KHE_EJECTOR ej);

/* 12.3 Ejector solving - repairs */
extern void KheEjectorRepairBegin(KHE_EJECTOR ej);
extern bool KheEjectorRepairEndLong(KHE_EJECTOR ej, int repair_type,
  bool success, int max_sub_chains, bool save_and_sort,
  void (*on_success_fn)(void *on_success_val), void *on_success_val);
extern bool KheEjectorRepairEnd(KHE_EJECTOR ej, int repair_type, bool success);

/* 12.6.3 Statistics describing a single solve */
extern int KheEjectorImprovementCount(KHE_EJECTOR ej);
extern int KheEjectorImprovementNumberOfRepairs(KHE_EJECTOR ej, int i);
extern float KheEjectorImprovementTime(KHE_EJECTOR ej, int i);
extern KHE_COST KheEjectorImprovementCost(KHE_EJECTOR ej, int i);
extern int KheEjectorImprovementDefects(KHE_EJECTOR ej, int i);

extern KHE_COST KheEjectorInitCost(KHE_EJECTOR ej);
extern int KheEjectorInitDefects(KHE_EJECTOR ej);

/* 12.6.4 Statistics describing multiple solves */
extern int KheEjectorImprovementRepairHistoMax(KHE_EJECTOR ej);
extern int KheEjectorImprovementRepairHistoFrequency(KHE_EJECTOR ej,
  int repair_count);
extern int KheEjectorImprovementRepairHistoTotal(KHE_EJECTOR ej);
extern float KheEjectorImprovementRepairHistoAverage(KHE_EJECTOR ej);

extern int KheEjectorImprovementAugmentHistoMax(KHE_EJECTOR ej);
extern int KheEjectorImprovementAugmentHistoFrequency(KHE_EJECTOR ej,
  int augment_count);
extern int KheEjectorImprovementAugmentHistoTotal(KHE_EJECTOR ej);
extern float KheEjectorImprovementAugmentHistoAverage(KHE_EJECTOR ej);

extern int KheEjectorTotalRepairs(KHE_EJECTOR ej, int augment_type);
extern int KheEjectorTotalRepairsByType(KHE_EJECTOR ej, int augment_type,
  int repair_type);
extern int KheEjectorSuccessfulRepairs(KHE_EJECTOR ej, int augment_type);
extern int KheEjectorSuccessfulRepairsByType(KHE_EJECTOR ej, int augment_type,
  int repair_type);

extern void KheEjectorAddAugmentType(KHE_EJECTOR ej, int augment_type,
  char *augment_label);
extern int KheEjectorAugmentTypeCount(KHE_EJECTOR ej);
extern int KheEjectorAugmentType(KHE_EJECTOR ej, int i);
extern char *KheEjectorAugmentTypeLabel(KHE_EJECTOR ej, int augment_type);

extern void KheEjectorAddRepairType(KHE_EJECTOR ej, int repair_type,
  char *repair_label);
extern int KheEjectorRepairTypeCount(KHE_EJECTOR ej);
extern int KheEjectorRepairType(KHE_EJECTOR ej, int i);
extern char *KheEjectorRepairTypeLabel(KHE_EJECTOR ej, int repair_type);

/* 12.7 Ejection chain time and resource repair functions */
extern bool KheEjectionChainNodeRepairTimes(KHE_NODE parent_node,
  KHE_OPTIONS options);
extern bool KheEjectionChainLayerRepairTimes(KHE_LAYER layer,
  KHE_OPTIONS options);
extern bool KheEjectionChainRepairResources(KHE_TASKING tasking,
  KHE_OPTIONS options);

extern KHE_EJECTOR KheEjectionChainEjectorMake(KHE_OPTIONS options);
extern void KheEjectionChainPrepareMonitors(KHE_SOLN soln);
extern void KheEjectionChainUnPrepareMonitors(KHE_SOLN soln);

extern KHE_GROUP_MONITOR KheNodeTimeRepairStartGroupMonitorMake(KHE_NODE node);
extern KHE_GROUP_MONITOR KheLayerTimeRepairStartGroupMonitorMake(
  KHE_LAYER layer);
extern KHE_GROUP_MONITOR KheLayerTimeRepairLongStartGroupMonitorMake(
  KHE_LAYER layer);
extern KHE_GROUP_MONITOR KheTaskingStartGroupMonitorMake(KHE_TASKING tasking);
extern KHE_GROUP_MONITOR KheGroupEventMonitors(KHE_SOLN soln,
  KHE_MONITOR_TAG tag, KHE_SUBTAG_STANDARD_TYPE sub_tag);
extern KHE_GROUP_MONITOR KheAllDemandGroupMonitorMake(KHE_SOLN soln);

#endif
