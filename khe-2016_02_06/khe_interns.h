
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
/*  FILE:         khe_interns.h                                              */
/*  DESCRIPTION:  Internal declarations for KHE; don't include this file.    */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_lset.h"
#include "khe_matching.h"
#include <string.h>
#include <limits.h>
#include <math.h>

/*****************************************************************************/
/*                                                                           */
/*  Abstract supertypes                                                      */
/*                                                                           */
/*****************************************************************************/

#define INHERIT_CONSTRAINT						  \
  void			*back;			/* back pointer      */	  \
  unsigned char		tag;			/* type tag          */	  \
  unsigned char		cost_function;		/* CostFunction      */	  \
  bool			required;		/* hard or soft      */	  \
  int			weight;			/* Weight            */	  \
  int			index;			/* index number      */	  \
  KHE_INSTANCE		instance;		/* enclosing instance*/	  \
  char			*id;			/* id                */	  \
  char			*name;			/* name              */	  \
  KHE_COST		combined_weight;	/* Weight            */	  \


typedef struct khe_monitor_link_rec *KHE_MONITOR_LINK;

struct khe_monitor_link_rec {
  KHE_GROUP_MONITOR	parent;
  KHE_MONITOR		child;
  int			parent_index;
  int			parent_defects_index;
  int			child_index;
  KHE_MONITOR_LINK	copy;
};

typedef MARRAY(KHE_MONITOR_LINK) ARRAY_KHE_MONITOR_LINK;

#define INHERIT_MONITOR							  \
  KHE_SOLN		soln;			/* encl. solution    */	  \
  unsigned char		tag;			/* tag field         */	  \
  bool			attached;		/* true if attached  */	  \
  int			soln_index;		/* index in soln     */	  \
  int			visit_num;		/* visit number      */	  \
  void			*back;			/* back pointer      */	  \
  ARRAY_KHE_MONITOR_LINK parent_links;		/* links to parents  */	  \
  KHE_COST		cost;			/* current cost      */	  \
  KHE_COST		lower_bound;		/* cost lower bound  */	  \

#define INHERIT_GROUP_MONITOR						  \
  INHERIT_MONITOR							  \
  ARRAY_KHE_MONITOR_LINK   child_links;		/* child links       */	  \
  ARRAY_KHE_MONITOR_LINK   defect_links;	/* defect links      */	  \
  ARRAY_KHE_TRACE	   traces; 		/* traces            */	  \
  int			   sub_tag;		/* sub tag           */	  \
  char			   *sub_tag_label;	/* sub tag label     */	  \

#define INHERIT_MATCHING_DEMAND_NODE					  \
  INHERIT_MONITOR							  \
  KHE_MATCHING_DEMAND_CHUNK	demand_chunk;				  \
  ARRAY_SHORT			domain;					  \
  KHE_MATCHING_SUPPLY_NODE	demand_asst;				  \
  short				demand_asst_index;			  \
  short				unmatched_pos;				  \
  KHE_MATCHING_DEMAND_NODE	bfs_next;				  \
  KHE_MATCHING_DEMAND_NODE	bfs_parent;				  \
  KHE_MATCHING_HALL_SET		hall_set;				  \


/*****************************************************************************/
/*                                                                           */
/*  Typedefs                                                                 */
/*                                                                           */
/*****************************************************************************/

#define NO_TIME_INDEX -1

typedef enum {
  KHE_TIME_GROUP_TYPE_FULL,
  KHE_TIME_GROUP_TYPE_EMPTY,
  KHE_TIME_GROUP_TYPE_SINGLETON,
  KHE_TIME_GROUP_TYPE_USER,
  KHE_TIME_GROUP_TYPE_NEIGHBOUR,
  KHE_TIME_GROUP_TYPE_CONSTRUCTED,
  KHE_TIME_GROUP_TYPE_SOLN
} KHE_TIME_GROUP_TYPE;

typedef enum {
  KHE_RESOURCE_GROUP_TYPE_FULL,
  KHE_RESOURCE_GROUP_TYPE_EMPTY,
  KHE_RESOURCE_GROUP_TYPE_SINGLETON,
  KHE_RESOURCE_GROUP_TYPE_USER,
  KHE_RESOURCE_GROUP_TYPE_PARTITION,
  KHE_RESOURCE_GROUP_TYPE_CONSTRUCTED,
  KHE_RESOURCE_GROUP_TYPE_SOLN
} KHE_RESOURCE_GROUP_TYPE;

typedef enum {
  KHE_EVENT_GROUP_TYPE_FULL,
  KHE_EVENT_GROUP_TYPE_EMPTY,
  KHE_EVENT_GROUP_TYPE_SINGLETON,
  KHE_EVENT_GROUP_TYPE_USER,
  KHE_EVENT_GROUP_TYPE_SOLN
} KHE_EVENT_GROUP_TYPE;

/* time group neighbourhoods */
typedef struct khe_time_group_nhood_rec *KHE_TIME_GROUP_NHOOD;
typedef MARRAY(KHE_TIME_GROUP_NHOOD) ARRAY_KHE_TIME_GROUP_NHOOD;

/* event in soln objects */
typedef struct khe_event_in_soln_rec *KHE_EVENT_IN_SOLN;
typedef MARRAY(KHE_EVENT_IN_SOLN) ARRAY_KHE_EVENT_IN_SOLN;

/* event resource in soln objects */
typedef struct khe_event_resource_in_soln_rec *KHE_EVENT_RESOURCE_IN_SOLN;
typedef MARRAY(KHE_EVENT_RESOURCE_IN_SOLN) ARRAY_KHE_EVENT_RESOURCE_IN_SOLN;

/* resource in soln objects */
typedef struct khe_resource_in_soln_rec *KHE_RESOURCE_IN_SOLN;
typedef MARRAY(KHE_RESOURCE_IN_SOLN) ARRAY_KHE_RESOURCE_IN_SOLN;

/* evenness handler */
typedef struct khe_evenness_handler_rec *KHE_EVENNESS_HANDLER;

/* array and symbol table types */
typedef MARRAY(KHE_ARCHIVE) ARRAY_KHE_ARCHIVE;
typedef MARRAY(KHE_SOLN_GROUP) ARRAY_KHE_SOLN_GROUP;
typedef MTABLE(KHE_SOLN_GROUP) TABLE_KHE_SOLN_GROUP;
typedef MARRAY(KHE_INSTANCE) ARRAY_KHE_INSTANCE;
typedef MTABLE(KHE_INSTANCE) TABLE_KHE_INSTANCE;
typedef MARRAY(KHE_TIME_GROUP) ARRAY_KHE_TIME_GROUP;
typedef MTABLE(KHE_TIME_GROUP) TABLE_KHE_TIME_GROUP;
typedef MARRAY(KHE_TIME) ARRAY_KHE_TIME;
typedef MTABLE(KHE_TIME) TABLE_KHE_TIME;
typedef MARRAY(KHE_RESOURCE_TYPE) ARRAY_KHE_RESOURCE_TYPE;
typedef MTABLE(KHE_RESOURCE_TYPE) TABLE_KHE_RESOURCE_TYPE;
typedef MARRAY(KHE_RESOURCE_GROUP) ARRAY_KHE_RESOURCE_GROUP;
typedef MTABLE(KHE_RESOURCE_GROUP) TABLE_KHE_RESOURCE_GROUP;
typedef MARRAY(KHE_RESOURCE) ARRAY_KHE_RESOURCE;
typedef MTABLE(KHE_RESOURCE) TABLE_KHE_RESOURCE;
typedef MARRAY(KHE_EVENT_GROUP) ARRAY_KHE_EVENT_GROUP;
typedef MTABLE(KHE_EVENT_GROUP) TABLE_KHE_EVENT_GROUP;
typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;
typedef MTABLE(KHE_EVENT) TABLE_KHE_EVENT;
typedef MARRAY(KHE_EVENT_RESOURCE) ARRAY_KHE_EVENT_RESOURCE;
typedef MARRAY(KHE_EVENT_RESOURCE_GROUP) ARRAY_KHE_EVENT_RESOURCE_GROUP;
typedef MARRAY(KHE_CONSTRAINT) ARRAY_KHE_CONSTRAINT;
typedef MTABLE(KHE_CONSTRAINT) TABLE_KHE_CONSTRAINT;

typedef MARRAY(KHE_PREFER_RESOURCES_CONSTRAINT)
  ARRAY_KHE_PREFER_RESOURCES_CONSTRAINT;

typedef MARRAY(KHE_TIME_SPREAD) ARRAY_KHE_TIME_SPREAD;
typedef MARRAY(KHE_LIMITED_TIME_GROUP) ARRAY_KHE_LIMITED_TIME_GROUP;

typedef MARRAY(KHE_SOLN) ARRAY_KHE_SOLN;
typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_MEET_BOUND) ARRAY_KHE_MEET_BOUND;
typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;
typedef MARRAY(KHE_TASK_BOUND) ARRAY_KHE_TASK_BOUND;
typedef MARRAY(KHE_TASK_BOUND_GROUP) ARRAY_KHE_TASK_BOUND_GROUP;
typedef MARRAY(KHE_MARK) ARRAY_KHE_MARK;
typedef MARRAY(KHE_PATH) ARRAY_KHE_PATH;
typedef MARRAY(KHE_TRACE) ARRAY_KHE_TRACE;
typedef MARRAY(KHE_NODE) ARRAY_KHE_NODE;
typedef MARRAY(KHE_LAYER) ARRAY_KHE_LAYER;
typedef MARRAY(KHE_ZONE) ARRAY_KHE_ZONE;
typedef MARRAY(KHE_TASKING) ARRAY_KHE_TASKING;

/* monitor array types */
typedef MARRAY(KHE_MONITOR) ARRAY_KHE_MONITOR;
typedef MARRAY(KHE_ASSIGN_RESOURCE_MONITOR) ARRAY_KHE_ASSIGN_RESOURCE_MONITOR;
typedef MARRAY(KHE_ASSIGN_TIME_MONITOR) ARRAY_KHE_ASSIGN_TIME_MONITOR;
typedef MARRAY(KHE_SPLIT_EVENTS_MONITOR) ARRAY_KHE_SPLIT_EVENTS_MONITOR;
typedef MARRAY(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR)
  ARRAY_KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR;
typedef MARRAY(KHE_PREFER_RESOURCES_MONITOR) ARRAY_KHE_PREFER_RESOURCES_MONITOR;
typedef MARRAY(KHE_PREFER_TIMES_MONITOR) ARRAY_KHE_PREFER_TIMES_MONITOR;
typedef MARRAY(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR)
  ARRAY_KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR;
typedef MARRAY(KHE_SPREAD_EVENTS_MONITOR) ARRAY_KHE_SPREAD_EVENTS_MONITOR;
typedef MARRAY(KHE_LINK_EVENTS_MONITOR) ARRAY_KHE_LINK_EVENTS_MONITOR;
typedef MARRAY(KHE_ORDER_EVENTS_MONITOR) ARRAY_KHE_ORDER_EVENTS_MONITOR;
typedef MARRAY(KHE_AVOID_CLASHES_MONITOR) ARRAY_KHE_AVOID_CLASHES_MONITOR;
typedef MARRAY(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR)
  ARRAY_KHE_AVOID_UNAVAILABLE_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_IDLE_TIMES_MONITOR) ARRAY_KHE_LIMIT_IDLE_TIMES_MONITOR;
typedef MARRAY(KHE_CLUSTER_BUSY_TIMES_MONITOR)
  ARRAY_KHE_CLUSTER_BUSY_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_BUSY_TIMES_MONITOR) ARRAY_KHE_LIMIT_BUSY_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_WORKLOAD_MONITOR) ARRAY_KHE_LIMIT_WORKLOAD_MONITOR;
typedef MARRAY(KHE_TIME_GROUP_MONITOR) ARRAY_KHE_TIME_GROUP_MONITOR;
typedef MARRAY(KHE_GROUP_MONITOR) ARRAY_KHE_GROUP_MONITOR;
typedef MARRAY(KHE_ORDINARY_DEMAND_MONITOR) ARRAY_KHE_ORDINARY_DEMAND_MONITOR;
typedef MARRAY(KHE_WORKLOAD_DEMAND_MONITOR) ARRAY_KHE_WORKLOAD_DEMAND_MONITOR;


/*****************************************************************************/
/*                                                                           */
/*  khe_archive.c                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md);

/* solution groups */
extern void KheArchiveAddSolnGroup(KHE_ARCHIVE archive,
  KHE_SOLN_GROUP soln_group);


/*****************************************************************************/
/*                                                                           */
/*  khe_archive_metadata.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheArchiveMetaDataMakeFromKml(KML_ELT md_elt,
  KHE_ARCHIVE archive, KML_ERROR *ke);
extern void KheArchiveMetaDataWrite(KHE_ARCHIVE_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln_group.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheSolnGroupSetMetaData(KHE_SOLN_GROUP soln_group,
  KHE_SOLN_GROUP_METADATA md);
extern void SolnGroupDeleteSolnsForInstance(KHE_SOLN_GROUP soln_group,
  KHE_INSTANCE ins);

/* reading and writing */
extern bool KheSolnGroupMakeFromKml(KML_ELT soln_group_elt,
  KHE_ARCHIVE archive, bool allow_invalid_solns, KML_ERROR *ke);
extern void KheSolnGroupWrite(KHE_SOLN_GROUP soln_group, bool with_reports,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln_group_metadata.c                                                */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheSolnGroupMetaDataMakeFromKml(KML_ELT md_elt,
  KHE_SOLN_GROUP soln_group, KML_ERROR *ke);
extern void KheSolnGroupMetaDataWrite(KHE_SOLN_GROUP_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_instance.c                                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheInstanceAddArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive);
extern void KheInstanceDeleteArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive);
extern bool KheInstanceComplete(KHE_INSTANCE ins);
extern void KheInstanceSetMetaData(KHE_INSTANCE ins, KHE_INSTANCE_METADATA md);

/* time groups */
extern void KheInstanceAddTimeGroup(KHE_INSTANCE ins, KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP KheInstanceFullTimeGroupInternal(KHE_INSTANCE ins);
extern KHE_TIME_GROUP_NHOOD KheInstanceSingletonTimeGroupNeighbourhood(
  KHE_INSTANCE ins);

/* times */
extern void KheInstanceAddTime(KHE_INSTANCE ins, KHE_TIME t);
/* extern ARRAY_KHE_TIME KheInstanceTimesArray(KHE_INSTANCE ins); */

/* partitions */
extern void KheInstanceAddPartition(KHE_INSTANCE ins, KHE_RESOURCE_GROUP rg,
  int *index);
extern int KheInstancePartitionCount(KHE_INSTANCE ins);
extern KHE_RESOURCE_GROUP KheInstancePartition(KHE_INSTANCE ins, int i);

/* resource types and resources */
extern void KheInstanceAddResourceType(KHE_INSTANCE ins, KHE_RESOURCE_TYPE rt,
  int *index);
extern void KheInstanceAddResource(KHE_INSTANCE ins, KHE_RESOURCE r);

/* event groups */
extern void KheInstanceAddEventGroup(KHE_INSTANCE ins, KHE_EVENT_GROUP eg);

/* events */
extern void KheInstanceAddEvent(KHE_INSTANCE ins, KHE_EVENT e);
extern ARRAY_KHE_EVENT KheInstanceEventsArray(KHE_INSTANCE ins);
extern int KheInstanceMaxEventDuration(KHE_INSTANCE ins);

/* event resources */
extern void KheInstanceAddEventResource(KHE_INSTANCE ins,
  KHE_EVENT_RESOURCE er);

/* constraints */
extern void KheInstanceAddConstraint(KHE_INSTANCE ins, KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheInstanceMakeFromKml(KML_ELT instance_elt,
  /* KHE_ARCHIVE archive, */ bool infer_resource_partitions,
  KHE_INSTANCE *ins, KML_ERROR *ke);
extern void KheInstanceWrite(KHE_INSTANCE ins, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_instance_metadata.c                                                  */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheInstanceMetaDataMakeFromKml(KML_ELT md_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern void KheInstanceMetaDataWrite(KHE_INSTANCE_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group_nhood.c                                                   */
/*                                                                           */
/*****************************************************************************/

extern KHE_TIME_GROUP_NHOOD KheTimeGroupNhoodMake(KHE_TIME_GROUP tg,
  KHE_SOLN soln, int *index_in_nhood);
extern KHE_TIME_GROUP_NHOOD KheTimeGroupNHoodMakeEmpty(int count);
extern void KheTimeGroupNHoodSetTimeGroup(KHE_TIME_GROUP_NHOOD tgn,
  int pos, KHE_TIME_GROUP tg);
extern void KheTimeGroupNHoodDelete(KHE_TIME_GROUP_NHOOD tgn);
extern KHE_TIME_GROUP KheTimeGroupNHoodNeighbour(KHE_TIME_GROUP_NHOOD tgn,
  int pos);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIME_GROUP KheTimeGroupMakeInternal(KHE_INSTANCE ins,
  KHE_TIME_GROUP_TYPE time_group_type, KHE_SOLN soln,
  KHE_TIME_GROUP_KIND kind, char *id, char *name, LSET times_set);
extern void KheTimeGroupAddTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupSubTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupUnionInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupIntersectInternal(KHE_TIME_GROUP tg,
  KHE_TIME_GROUP tg2);
extern void KheTimeGroupDifferenceInternal(KHE_TIME_GROUP tg,
  KHE_TIME_GROUP tg2);
extern void KheTimeGroupFinalize(KHE_TIME_GROUP tg,
  KHE_SOLN soln, KHE_TIME_GROUP_NHOOD tgn, int pos_in_tgn);
extern void KheTimeGroupDelete(KHE_TIME_GROUP tg);

/* times queries */
extern bool KheTimeGroupContainsIndex(KHE_TIME_GROUP tg, int time_index);
extern bool KheTimeGroupDomainsAllowAssignment(KHE_TIME_GROUP domain,
  KHE_TIME_GROUP target_domain, int target_offset);
extern int KheTimeGroupTimePos(KHE_TIME_GROUP tg, int time_index);
extern ARRAY_SHORT KheTimeGroupTimeIndexes(KHE_TIME_GROUP tg);
extern LSET KheTimeGroupTimeSet(KHE_TIME_GROUP tg);

/* reading and writing */
extern bool KheTimeGroupMakeFromKml(KML_ELT time_group_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern void KheTimeGroupWrite(KHE_TIME_GROUP tg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_time.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern void KheTimeWrite(KHE_TIME t, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_type.c                                                      */
/*                                                                           */
/*****************************************************************************/

extern void KheResourceTypeAddResource(KHE_RESOURCE_TYPE rt, KHE_RESOURCE r);
extern void KheResourceTypeAddResourceGroup(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP rg);
extern void KheResourceTypeFinalize(KHE_RESOURCE_TYPE rt);

/* partitions */
extern void KheResourceTypeAddPartition(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP rg);
extern void KheResourceTypeInferPartitions(KHE_RESOURCE_TYPE rt);

/* reading and writing */
extern bool KheResourceTypeMakeFromKml(KML_ELT resource_type_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheResourceTypeWrite(KHE_RESOURCE_TYPE rt, KML_FILE kf);
extern void KheResourceTypeWriteResourceGroups(KHE_RESOURCE_TYPE rt,
  KML_FILE kf);
extern void KheResourceTypeWriteResources(KHE_RESOURCE_TYPE rt, KML_FILE kf);

/* demand and avoid split assignments count */
extern void KheResourceTypeDemandNotAllPreassigned(KHE_RESOURCE_TYPE rt);
extern void KheResourceTypeIncrementAvoidSplitAssignmentsCount(
  KHE_RESOURCE_TYPE rt);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_group.c                                                     */
/*                                                                           */
/*****************************************************************************/

extern KHE_RESOURCE_GROUP_TYPE KheResourceGroupType(KHE_RESOURCE_GROUP rg);
extern KHE_RESOURCE_GROUP KheResourceGroupMakeInternal(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP_TYPE resource_group_type, KHE_SOLN soln,
  char *id, char *name, LSET lset);
extern void KheResourceGroupAddResourceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern void KheResourceGroupSubResourceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern void KheResourceGroupUnionInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupIntersectInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupDifferenceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupSetResourcesArrayInternal(KHE_RESOURCE_GROUP rg);
extern void KheResourceGroupDelete(KHE_RESOURCE_GROUP rg);

/* domains and partitions */
extern ARRAY_SHORT KheResourceGroupResourceIndexes(KHE_RESOURCE_GROUP rg);
extern LSET KheResourceGroupResourceSet(KHE_RESOURCE_GROUP rg);
extern bool KheResourceGroupPartitionAdmissible(KHE_RESOURCE_GROUP rg);
extern void KheResourceGroupDeclarePartition(KHE_RESOURCE_GROUP rg);
extern void KheResourceGroupSetPartition(KHE_RESOURCE_GROUP rg);
extern int KheResourceGroupPartitionIndex(KHE_RESOURCE_GROUP rg);

/* reading and writing */
extern bool KheResourceGroupMakeFromKml(KML_ELT resource_group_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheResourceGroupWrite(KHE_RESOURCE_GROUP rg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource.c                                                           */
/*                                                                           */
/*****************************************************************************/

extern void KheResourceAddConstraint(KHE_RESOURCE r, KHE_CONSTRAINT c);
extern void KheResourceAddPreassignedEventResource(KHE_RESOURCE r,
  KHE_EVENT_RESOURCE er);
extern void KheResourceAddUserResourceGroup(KHE_RESOURCE r,
  KHE_RESOURCE_GROUP rg);

/* resource partition */
extern void KheResourceSetPartition(KHE_RESOURCE r, KHE_RESOURCE_GROUP rg);

/* finalizing */
extern void KheResourceFinalize(KHE_RESOURCE r);

/* reading and writing */
extern bool KheResourceMakeFromKml(KML_ELT resource_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheResourceWrite(KHE_RESOURCE r, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_group.c                                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
/* extern KHE_EVENT_GROUP_TYPE KheEventGroupType(KHE_EVENT_GROUP eg); */
extern KHE_EVENT_GROUP KheEventGroupMakeInternal(KHE_INSTANCE ins,
  KHE_EVENT_GROUP_TYPE event_group_type, KHE_SOLN soln,
  KHE_EVENT_GROUP_KIND kind, char *id, char *name);
extern void KheEventGroupAddEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupSubEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT t);
extern void KheEventGroupUnionInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupIntersectInternal(KHE_EVENT_GROUP eg,
  KHE_EVENT_GROUP eg2);
extern void KheEventGroupDifferenceInternal(KHE_EVENT_GROUP eg,
  KHE_EVENT_GROUP eg2);
extern void KheEventGroupSetEventsArrayInternal(KHE_EVENT_GROUP eg);
extern void KheEventGroupDelete(KHE_EVENT_GROUP eg);

/* constraints */
extern void KheEventGroupAddConstraint(KHE_EVENT_GROUP eg, KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheEventGroupMakeFromKml(KML_ELT event_group_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheEventGroupWrite(KHE_EVENT_GROUP eg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event.c                                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheEventAddEventResource(KHE_EVENT e, KHE_EVENT_RESOURCE er,
  int *index);
extern void KheEventAddUserEventGroup(KHE_EVENT e, KHE_EVENT_GROUP eg);
extern void KheEventAddEventResourceGroup(KHE_EVENT event,
  KHE_EVENT_RESOURCE_GROUP erg);

/* constraints */
extern void KheEventAddConstraint(KHE_EVENT e, KHE_CONSTRAINT c);

/* time domains */
extern void KheEventFinalize(KHE_EVENT e);

/* infer resource partitions */
extern void KheEventPartitionSetAdmissible(KHE_EVENT e);
extern bool KheEventPartitionAdmissible(KHE_EVENT e);
extern bool KheEventPartitionSimilar(KHE_EVENT e1, KHE_EVENT e2,
  ARRAY_KHE_RESOURCE_GROUP *domains1, ARRAY_KHE_RESOURCE_GROUP *domains2);

/* reading and writing */
extern bool KheEventMakeFromKml(KML_ELT event_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern void KheEventWrite(KHE_EVENT e, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_resource.c                                                     */
/*                                                                           */
/*****************************************************************************/

/* event resource groups */
extern void KheEventResourceSetEventResourceGroup(KHE_EVENT_RESOURCE er,
  KHE_EVENT_RESOURCE_GROUP erg);

/* constraints */
extern void KheEventResourceAddConstraint(KHE_EVENT_RESOURCE er,
  KHE_CONSTRAINT c, int eg_index);

/* finalizing */
extern void KheEventResourceFinalize(KHE_EVENT_RESOURCE er);

/* reading and writing */
extern bool KheEventResourceMakeFromKml(KML_ELT resource_elt, KHE_EVENT e,
  KML_ERROR *ke);
extern void KheEventResourceWrite(KHE_EVENT_RESOURCE er, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  Constraints                                                              */
/*                                                                           */
/*  KHE_CONSTRAINT                                                           */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT                                         */
/*    KHE_ASSIGN_TIME_CONSTRAINT                                             */
/*    KHE_SPLIT_EVENTS_CONSTRAINT                                            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT                                 */
/*    KHE_PREFER_RESOURCES_CONSTRAINT                                        */
/*    KHE_PREFER_TIMES_CONSTRAINT                                            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT                                 */
/*    KHE_SPREAD_EVENTS_CONSTRAINT                                           */
/*    KHE_LINK_EVENTS_CONSTRAINT                                             */
/*    KHE_ORDER_EVENTS_CONSTRAINT                                            */
/*    KHE_AVOID_CLASHES_CONSTRAINT                                           */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT                                 */
/*    KHE_LIMIT_IDLE_TIMES_CONSTRAINT                                        */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT                                      */
/*    KHE_LIMIT_BUSY_TIMES_CONSTRAINT                                        */
/*    KHE_LIMIT_WORKLOAD_CONSTRAINT                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  khe_constraint.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheConstraintFinalize(KHE_CONSTRAINT c);
extern int KheConstraintDensityCount(KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheConstraintMakeFromKml(KML_ELT constraint_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheConstraintCheckKml(KML_ELT cons_elt, char **id, char **name,
  bool *required, int *weight, KHE_COST_FUNCTION *cf, KML_ERROR *ke);
extern bool KheConstraintAddTimeGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddTimesFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddResourceGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddResourcesFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddEventGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddEventsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern void KheConstraintWrite(KHE_CONSTRAINT c, KML_FILE kf);
extern void KheConstraintWriteCommonFields(KHE_CONSTRAINT c, KML_FILE kf);

/* evaluation */
extern KHE_COST KheConstraintCost(KHE_CONSTRAINT c, int dev);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_resource_constraint.c                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAssignResourceConstraintAppliesToCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern void KheAssignResourceConstraintFinalize(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern int KheAssignResourceConstraintDensityCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);

/* reading and writing */
extern bool KheAssignResourceConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheAssignResourceConstraintWrite(KHE_ASSIGN_RESOURCE_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_time_constraint.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAssignTimeConstraintAppliesToCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern void KheAssignTimeConstraintFinalize(KHE_ASSIGN_TIME_CONSTRAINT c);
extern int KheAssignTimeConstraintDensityCount(KHE_ASSIGN_TIME_CONSTRAINT c);

/* reading and writing */
extern bool KheAssignTimeConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheAssignTimeConstraintWrite(KHE_ASSIGN_TIME_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_split_events_constraint.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheSplitEventsConstraintAppliesToCount(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheSplitEventsConstraintFinalize(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintDensityCount(KHE_SPLIT_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheSplitEventsConstraintWrite(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_distribute_split_events_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheDistributeSplitEventsConstraintAppliesToCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheDistributeSplitEventsConstraintFinalize(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheDistributeSplitEventsConstraintDensityCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheDistributeSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheDistributeSplitEventsConstraintWrite(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_resources_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KhePreferResourcesConstraintAppliesToCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern void KhePreferResourcesConstraintFinalize(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern int KhePreferResourcesConstraintDensityCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

/* reading and writing */
extern bool KhePreferResourcesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KhePreferResourcesConstraintWrite(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_times_constraint.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KhePreferTimesConstraintAppliesToCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern void KhePreferTimesConstraintFinalize(KHE_PREFER_TIMES_CONSTRAINT c);
extern int KhePreferTimesConstraintDensityCount(KHE_PREFER_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KhePreferTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KhePreferTimesConstraintWrite(KHE_PREFER_TIMES_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_split_assignments_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidSplitAssignmentsConstraintAppliesToCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern void KheAvoidSplitAssignmentsConstraintFinalize(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern int KheAvoidSplitAssignmentsConstraintDensityCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidSplitAssignmentsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheAvoidSplitAssignmentsConstraintWrite(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_spread_events_constraint.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheSpreadEventsConstraintAppliesToCount(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern void KheSpreadEventsConstraintFinalize(KHE_SPREAD_EVENTS_CONSTRAINT c);
extern int KheSpreadEventsConstraintDensityCount(
  KHE_SPREAD_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheSpreadEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheSpreadEventsConstraintWrite(KHE_SPREAD_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_link_events_constraint.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLinkEventsConstraintAppliesToCount(KHE_LINK_EVENTS_CONSTRAINT c);
extern void KheLinkEventsConstraintFinalize(KHE_LINK_EVENTS_CONSTRAINT c);
extern int KheLinkEventsConstraintDensityCount(KHE_LINK_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheLinkEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheLinkEventsConstraintWrite(KHE_LINK_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_order_events_constraint.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheOrderEventsConstraintAppliesToCount(
  KHE_ORDER_EVENTS_CONSTRAINT c);
extern void KheOrderEventsConstraintFinalize(KHE_ORDER_EVENTS_CONSTRAINT c);
extern int KheOrderEventsConstraintDensityCount(KHE_ORDER_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheOrderEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheOrderEventsConstraintWrite(KHE_ORDER_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_clashes_constraint.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidClashesConstraintAppliesToCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern void KheAvoidClashesConstraintFinalize(KHE_AVOID_CLASHES_CONSTRAINT c);
extern int KheAvoidClashesConstraintDensityCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidClashesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheAvoidClashesConstraintWrite(KHE_AVOID_CLASHES_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_unavailable_times_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidUnavailableTimesConstraintAppliesToCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern void KheAvoidUnavailableTimesConstraintFinalize(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern int KheAvoidUnavailableTimesConstraintDensityCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidUnavailableTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheAvoidUnavailableTimesConstraintWrite(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_idle_times_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitIdleTimesConstraintAppliesToCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern void KheLimitIdleTimesConstraintFinalize(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern int KheLimitIdleTimesConstraintDensityCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitIdleTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheLimitIdleTimesConstraintWrite(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_cluster_busy_times_constraint.c                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheClusterBusyTimesConstraintAppliesToCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern void KheClusterBusyTimesConstraintFinalize(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern int KheClusterBusyTimesConstraintDensityCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheClusterBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheClusterBusyTimesConstraintWrite(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_busy_times_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitBusyTimesConstraintAppliesToCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern void KheLimitBusyTimesConstraintFinalize(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern int KheLimitBusyTimesConstraintDensityCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheLimitBusyTimesConstraintWrite(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_workload_constraint.c                                          */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitWorkloadConstraintAppliesToCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern void KheLimitWorkloadConstraintFinalize(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern int KheLimitWorkloadConstraintDensityCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitWorkloadConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern void KheLimitWorkloadConstraintWrite(KHE_LIMIT_WORKLOAD_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheSolnAddSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group);
extern void KheSolnDeleteSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group);
extern KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln);
extern void KheSolnCopyPhase2(KHE_SOLN soln);

/* time groups and neighbourhoods */
extern void KheSolnAddTimeGroup(KHE_SOLN soln, KHE_TIME_GROUP tg);
extern void KheSolnAddTimeNHood(KHE_SOLN soln, KHE_TIME_GROUP_NHOOD nhood);

/* resource groups */
void KheSolnAddResourceGroup(KHE_SOLN soln, KHE_RESOURCE_GROUP rg);

/* event groups */
void KheSolnAddEventGroup(KHE_SOLN soln, KHE_EVENT_GROUP eg);

/* meet bounds */
extern void KheSolnAddMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb);
extern void KheSolnDeleteMeetBound(KHE_SOLN soln, KHE_MEET_BOUND mb);
extern KHE_MEET_BOUND KheSolnGetMeetBoundFromFreeList(KHE_SOLN soln);
extern void KheSolnAddMeetBoundToFreeList(KHE_SOLN soln, KHE_MEET_BOUND mb);

/* task bounds */
extern void KheSolnAddTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb);
extern void KheSolnDeleteTaskBound(KHE_SOLN soln, KHE_TASK_BOUND tb);
extern KHE_TASK_BOUND KheSolnGetTaskBoundFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTaskBoundToFreeList(KHE_SOLN soln, KHE_TASK_BOUND tb);

/* evenness handler */
extern KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln);

/* monitor links */
extern KHE_MONITOR_LINK KheSolnGetMonitorLinkFromFreeList(KHE_SOLN soln);
extern void KheSolnAddMonitorLinkToFreeList(KHE_SOLN soln, KHE_MONITOR_LINK ml);

/* traces */
extern KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t);

/* marks */
extern void KheSolnAddMarkToFreeList(KHE_SOLN soln, KHE_MARK mark);
extern KHE_MARK KheSolnTakeMarkFromFreeList(KHE_SOLN soln);
extern void KheSolnMarkBegin(KHE_SOLN soln, KHE_MARK mark,
  int *index, int *start_pos);
extern void KheSolnMarkEnd(KHE_SOLN soln, KHE_MARK mark);
extern bool KheSolnMarkOnTop(KHE_SOLN soln, KHE_MARK mark);

/* paths */
extern KHE_PATH KheSolnMainPath(KHE_SOLN soln);
extern void KheSolnAddPathToFreeList(KHE_SOLN soln, KHE_PATH path);
extern KHE_PATH KheSolnTakePathFromFreeList(KHE_SOLN soln);

/* path operation loading - meets */
extern void KheSolnOpMeetSetBack(KHE_SOLN soln, KHE_MEET meet,
  void *old_back, void *new_back);
extern void KheSolnOpMeetAdd(KHE_SOLN soln, KHE_MEET meet, int duration,
  KHE_EVENT e);
extern void KheSolnOpMeetDelete(KHE_SOLN soln, KHE_MEET meet, int duration,
  KHE_EVENT e);
extern void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,
  int durn1);
extern void KheSolnOpMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2,
  int durn1);
extern void KheSolnOpMeetMove(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET old_target_meet, int old_target_offset,
  KHE_MEET new_target_meet, int new_target_offset);
extern void KheSolnOpMeetSetAutoDomain(KHE_SOLN soln, KHE_MEET meet,
  bool automatic);
extern void KheSolnOpMeetAssignFix(KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnOpMeetAssignUnFix(KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnOpMeetAddMeetBound(KHE_SOLN soln,
  KHE_MEET meet, KHE_MEET_BOUND mb);
extern void KheSolnOpMeetDeleteMeetBound(KHE_SOLN soln,
  KHE_MEET meet, KHE_MEET_BOUND mb);

/* path operation loading - meet bounds */
extern void KheSolnOpMeetBoundAdd(KHE_SOLN soln, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern void KheSolnOpMeetBoundDelete(KHE_SOLN soln, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern void KheSolnOpMeetBoundAddTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern void KheSolnOpMeetBoundDeleteTimeGroup(KHE_SOLN soln, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);

/* path operation loading - tasks */
extern void KheSolnOpTaskSetBack(KHE_SOLN soln, KHE_TASK task,
  void *old_back, void *new_back);
extern void KheSolnOpTaskAdd(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KheSolnOpTaskDelete(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KheSolnOpTaskSplit(KHE_SOLN soln, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2);
extern void KheSolnOpTaskMerge(KHE_SOLN soln, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2);
extern void KheSolnOpTaskMove(KHE_SOLN soln, KHE_TASK task,
  KHE_TASK old_target_task, KHE_TASK new_target_task);
extern void KheSolnOpTaskAssignFix(KHE_SOLN soln, KHE_TASK task);
extern void KheSolnOpTaskAssignUnFix(KHE_SOLN soln, KHE_TASK task);
extern void KheSolnOpTaskAddTaskBound(KHE_SOLN soln,
  KHE_TASK task, KHE_TASK_BOUND tb);
extern void KheSolnOpTaskDeleteTaskBound(KHE_SOLN soln,
  KHE_TASK task, KHE_TASK_BOUND tb);

/* path operation loading - task bounds */
extern void KheSolnOpTaskBoundAdd(KHE_SOLN soln, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg);
extern void KheSolnOpTaskBoundDelete(KHE_SOLN soln, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg);

/* path operation loading - nodes */
extern void KheSolnOpNodeSetBack(KHE_SOLN soln, KHE_NODE node,
  void *old_back, void *new_back);
extern void KheSolnOpNodeAdd(KHE_SOLN soln, KHE_NODE node);
extern void KheSolnOpNodeDelete(KHE_SOLN soln, KHE_NODE node);
extern void KheSolnOpNodeAddParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KheSolnOpNodeDeleteParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KheSolnOpNodeSwapChildNodesAndLayers(KHE_SOLN soln,
  KHE_NODE node1, KHE_NODE node2);
extern void KheSolnOpNodeAddMeet(KHE_SOLN soln, KHE_NODE node, KHE_MEET meet);
extern void KheSolnOpNodeDeleteMeet(KHE_SOLN soln, KHE_NODE node,KHE_MEET meet);

/* path operation loading - layers */
extern void KheSolnOpLayerSetBack(KHE_SOLN soln, KHE_LAYER layer,
  void *old_back, void *new_back);
extern void KheSolnOpLayerAdd(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE parent_node);
extern void KheSolnOpLayerDelete(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE parent_node);
extern void KheSolnOpLayerAddChildNode(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE child_node);
extern void KheSolnOpLayerDeleteChildNode(KHE_SOLN soln, KHE_LAYER layer,
  KHE_NODE child_node);
extern void KheSolnOpLayerAddResource(KHE_SOLN soln, KHE_LAYER layer,
  KHE_RESOURCE resource);
extern void KheSolnOpLayerDeleteResource(KHE_SOLN soln, KHE_LAYER layer,
  KHE_RESOURCE resource);

/* path operation loading - zones */
extern void KheSolnOpZoneSetBack(KHE_SOLN soln, KHE_ZONE zone,
  void *old_back, void *new_back);
extern void KheSolnOpZoneAdd(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node);
extern void KheSolnOpZoneDelete(KHE_SOLN soln, KHE_ZONE zone, KHE_NODE node);
extern void KheSolnOpZoneAddMeetOffset(KHE_SOLN soln,
  KHE_ZONE zone, KHE_MEET meet, int offset);
extern void KheSolnOpZoneDeleteMeetOffset(KHE_SOLN soln,
  KHE_ZONE zone, KHE_MEET meet, int offset);

/* resource in soln objects */
extern KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i);

/* event in soln objects */
extern KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i);

/* monitors */
extern void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m);
extern void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m);

/* meets */
extern KHE_MEET KheSolnGetMeetFromFreeList(KHE_SOLN soln);
extern void KheSolnAddMeetToFreeList(KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet);

/* cycle meets */
extern void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1,
  KHE_MEET meet2);
extern void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1,
  KHE_MEET meet2);

/* nodes */
extern KHE_NODE KheSolnGetNodeFromFreeList(KHE_SOLN soln);
extern void KheSolnAddNodeToFreeList(KHE_SOLN soln, KHE_NODE node);
extern void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node);
extern void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node);

/* layers */
extern KHE_LAYER KheSolnGetLayerFromFreeList(KHE_SOLN soln);
extern void KheSolnAddLayerToFreeList(KHE_SOLN soln, KHE_LAYER layer);

/* zones */
extern KHE_ZONE KheSolnGetZoneFromFreeList(KHE_SOLN soln);
extern void KheSolnAddZoneToFreeList(KHE_SOLN soln, KHE_ZONE zone);

/* tasks */
extern void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task);
extern void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task);
extern KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task);

/* taskings */
extern void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking);
extern void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking);
extern KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking);

/* matchings - zero domain */
extern ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln);

/* matchings - free supply chunks */
extern KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(
  KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,
  KHE_MATCHING_SUPPLY_CHUNK sc);

/* matchings - general */
extern KHE_MATCHING KheSolnMatching(KHE_SOLN soln);
extern void KheSolnMatchingUpdate(KHE_SOLN soln);

/* reading and writing */
extern bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_ARCHIVE archive,
  bool allow_invalid_solns, KHE_SOLN *soln, KML_ERROR *ke);
extern void KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_evenness_handler.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENNESS_HANDLER KheEvennessHandlerMake(KHE_SOLN soln);
extern KHE_EVENNESS_HANDLER KheEvennessHandlerCopyPhase1(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerCopyPhase2(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerDelete(KHE_EVENNESS_HANDLER eh);

/* configuration */
extern void KheEvennessHandlerAttachAllEvennessMonitors(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerDetachAllEvennessMonitors(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandleSetAllEvennessMonitorWeights(
  KHE_EVENNESS_HANDLER eh, KHE_COST weight);

/* monitoring calls */
extern void KheEvennessHandlerMonitorAttach(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerMonitorDetach(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerAddTask(KHE_EVENNESS_HANDLER eh,
  KHE_TASK task, int assigned_time_index);
extern void KheEvennessHandlerDeleteTask(KHE_EVENNESS_HANDLER eh,
  KHE_TASK task, int assigned_time_index);

/* debug */
extern void KheEvennessHandlerDebug(KHE_EVENNESS_HANDLER eh,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_meet.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheMeetKernelSetBack(KHE_MEET meet, void *back);
extern void KheMeetKernelSetBackUndo(KHE_MEET meet, void *old_back,
  void *new_back);

extern void KheMeetKernelAdd(KHE_MEET meet, KHE_SOLN soln, int duration,
  KHE_EVENT e);
extern void KheMeetKernelAddUndo(KHE_MEET meet);
extern void KheMeetKernelDelete(KHE_MEET meet);
extern void KheMeetKernelDeleteUndo(KHE_MEET meet, KHE_SOLN soln, int duration,
  KHE_EVENT e);

extern void KheMeetKernelSplit(KHE_MEET meet1, KHE_MEET meet2, int durn1);
extern void KheMeetKernelSplitUndo(KHE_MEET meet1, KHE_MEET meet2, int durn1);
extern void KheMeetKernelMerge(KHE_MEET meet1, KHE_MEET meet2, int durn1);
extern void KheMeetKernelMergeUndo(KHE_MEET meet1, KHE_MEET meet2, int durn1);

extern void KheMeetKernelMove(KHE_MEET meet, KHE_MEET target_meet, int offset);
extern void KheMeetKernelMoveUndo(KHE_MEET meet,
  KHE_MEET old_target_meet, int old_offset,
  KHE_MEET new_target_meet, int new_offset);

extern void KheMeetKernelAssignFix(KHE_MEET meet);
extern void KheMeetKernelAssignFixUndo(KHE_MEET meet);
extern void KheMeetKernelAssignUnFix(KHE_MEET meet);
extern void KheMeetKernelAssignUnFixUndo(KHE_MEET meet);

extern void KheMeetKernelSetAutoDomain(KHE_MEET meet, bool automatic);
extern void KheMeetKernelSetAutoDomainUndo(KHE_MEET meet, bool automatic);

extern void KheMeetKernelAddMeetBound(KHE_MEET meet, KHE_MEET_BOUND mb);
extern void KheMeetKernelAddMeetBoundUndo(KHE_MEET meet, KHE_MEET_BOUND mb);
extern void KheMeetKernelDeleteMeetBound(KHE_MEET meet, KHE_MEET_BOUND mb);
extern void KheMeetKernelDeleteMeetBoundUndo(KHE_MEET meet, KHE_MEET_BOUND mb);

/* construction and query */
extern void KheMeetUnMake(KHE_MEET meet);
extern KHE_MEET KheMeetCopyPhase1(KHE_MEET meet);
extern void KheMeetCopyPhase2(KHE_MEET meet);
extern void KheMeetSetSoln(KHE_MEET meet, KHE_SOLN soln);
extern void KheMeetSetSolnIndex(KHE_MEET meet, int soln_index);
extern void KheMeetSetAssignedTimeIndexAndDomain(KHE_MEET meet, KHE_TIME t);
extern int KheMeetAssignedTimeIndex(KHE_MEET meet);
extern KHE_EVENT_IN_SOLN KheMeetEventInSoln(KHE_MEET meet);

/* nodes */
extern bool KheMeetAddNodeCheck(KHE_MEET meet, KHE_NODE node);
extern void KheMeetSetNode(KHE_MEET meet, KHE_NODE node);
extern void KheMeetSetNodeIndex(KHE_MEET meet, int node_index);
extern bool KheMeetDeleteNodeCheck(KHE_MEET meet, KHE_NODE node);

/* tasks */
extern void KheMeetAddTask(KHE_MEET meet, KHE_TASK task, bool add_demand);
extern void KheMeetDeleteTask(KHE_MEET meet, int task_index);
extern void KheMeetAssignPreassignedResources(KHE_MEET meet,
  KHE_RESOURCE_TYPE rt);
extern void KheMeetPartitionTaskCount(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int *count);
extern void KheMeetPartitionTaskDebug(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int verbosity, int indent, FILE *fp);

/* zones */
extern void KheMeetOffsetAddZone(KHE_MEET meet, int offset, KHE_ZONE zone);
extern void KheMeetOffsetDeleteZone(KHE_MEET meet, int offset);

/* matching */
extern void KheMeetMatchingReset(KHE_MEET meet);
extern KHE_MATCHING_DEMAND_CHUNK KheMeetDemandChunk(KHE_MEET meet, int offset);
extern int KheMeetSupplyNodeOffset(KHE_MEET meet, KHE_MATCHING_SUPPLY_NODE sn);
extern void KheMeetMatchingAttachAllOrdinaryDemandMonitors(KHE_MEET meet);
extern void KheMeetMatchingDetachAllOrdinaryDemandMonitors(KHE_MEET meet);
extern void KheMeetMatchingSetWeight(KHE_MEET meet, KHE_COST new_weight);
extern void KheMeetMatchingBegin(KHE_MEET meet);
extern void KheMeetMatchingEnd(KHE_MEET meet);

/* reference counting */
extern void KheMeetReferenceCountIncrement(KHE_MEET meet);
extern void KheMeetReferenceCountDecrement(KHE_MEET meet);

/* reading and writing */
extern bool KheMeetMakeFromKml(KML_ELT meet_elt, KHE_SOLN soln, KML_ERROR *ke);
extern int KheMeetAssignedTimeCmp(const void *t1, const void *t2);
extern void KheMeetCheckForWriting(KHE_MEET meet);
extern bool KheMeetMustWrite(KHE_MEET meet);
extern void KheMeetWrite(KHE_MEET meet, KML_FILE kf);

/* debug */
extern void KheMeetAssignedDurationDebug(KHE_MEET meet, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_meet_bound.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheMeetBoundKernelAdd(KHE_MEET_BOUND mb, KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern void KheMeetBoundKernelAddUndo(KHE_MEET_BOUND mb);
extern void KheMeetBoundKernelDelete(KHE_MEET_BOUND mb);
extern void KheMeetBoundKernelDeleteUndo(KHE_MEET_BOUND mb, KHE_SOLN soln,
  bool occupancy, KHE_TIME_GROUP dft_tg);

extern void KheMeetBoundKernelAddTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern void KheMeetBoundKernelAddTimeGroupUndo(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern void KheMeetBoundKernelDeleteTimeGroup(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern void KheMeetBoundKernelDeleteTimeGroupUndo(KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);

/* creation and deletion */
extern void KheMeetBoundUnMake(KHE_MEET_BOUND mb);
extern void KheMeetBoundReferenceCountIncrement(KHE_MEET_BOUND mb);
extern void KheMeetBoundReferenceCountDecrement(KHE_MEET_BOUND mb);

/* copy */
extern KHE_MEET_BOUND KheMeetBoundCopyPhase1(KHE_MEET_BOUND mb);
extern void KheMeetBoundCopyPhase2(KHE_MEET_BOUND mb);

/* solution */
extern void KheMeetBoundSetSoln(KHE_MEET_BOUND mb, KHE_SOLN soln);
extern void KheMeetBoundSetSolnIndex(KHE_MEET_BOUND mb, int soln_index);

/* meets */
extern void KheMeetBoundAddMeet(KHE_MEET_BOUND mb, KHE_MEET meet);
extern void KheMeetBoundDeleteMeet(KHE_MEET_BOUND mb, KHE_MEET meet);


/*****************************************************************************/
/*                                                                           */
/*  khe_task.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheTaskKernelSetBack(KHE_TASK task, void *back);
extern void KheTaskKernelSetBackUndo(KHE_TASK task, void *old_back,
  void *new_back);

extern void KheTaskKernelAdd(KHE_TASK task, KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KheTaskKernelAddUndo(KHE_TASK task);
extern void KheTaskKernelDelete(KHE_TASK task);
extern void KheTaskKernelDeleteUndo(KHE_TASK task, KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);

extern void KheTaskKernelSplit(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2);
extern void KheTaskKernelSplitUndo(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2);
extern void KheTaskKernelMerge(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2);
extern void KheTaskKernelMergeUndo(KHE_TASK task1, KHE_TASK task2, int durn1,
  KHE_MEET meet2);

extern void KheTaskKernelMove(KHE_TASK task, KHE_TASK target_task);
extern void KheTaskKernelMoveUndo(KHE_TASK task,
  KHE_TASK old_target_task, KHE_TASK new_target_task);

extern void KheTaskKernelAssignFix(KHE_TASK task);
extern void KheTaskKernelAssignFixUndo(KHE_TASK task);
extern void KheTaskKernelAssignUnFix(KHE_TASK task);
extern void KheTaskKernelAssignUnFixUndo(KHE_TASK task);

extern void KheTaskKernelAddTaskBound(KHE_TASK task, KHE_TASK_BOUND tb);
extern void KheTaskKernelAddTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb);
extern void KheTaskKernelDeleteTaskBound(KHE_TASK task, KHE_TASK_BOUND tb);
extern void KheTaskKernelDeleteTaskBoundUndo(KHE_TASK task, KHE_TASK_BOUND tb);

/* construction and query */
extern KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_TASK KheTaskCopyPhase1(KHE_TASK task);
extern void KheTaskCopyPhase2(KHE_TASK task);
extern void KheTaskUnMake(KHE_TASK task);
extern KHE_TASK KheTaskDoGet(KHE_SOLN soln);

/* fixed leader (promotion to public? still to do) */
extern KHE_TASK KheTaskFixedLeader(KHE_TASK task);

/* relation with enclosing soln */
extern int KheTaskSolnIndex(KHE_TASK task);
extern void KheTaskSetSolnIndex(KHE_TASK task, int soln_index);

/* relation with enclosing meet */
extern void KheTaskSetMeet(KHE_TASK task, KHE_MEET meet);
extern void KheTaskSetMeetIndex(KHE_TASK task, int num);

/* relation with enclosing tasking */
extern void KheTaskSetTasking(KHE_TASK task, KHE_TASKING tasking);
extern void KheTaskSetTaskingIndex(KHE_TASK task, int tasking_index);

/* domains */
extern void KheTaskSetDomainUnchecked(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);

/* demand monitor domains */
extern KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task);
extern void KheTaskMatchingReset(KHE_TASK task);

/* time assignment */
extern void KheTaskAssignTime(KHE_TASK task, int assigned_time_index);
extern void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index);

/* splitting and merging */
extern bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2);

/* demand monitors */
extern void KheTaskMatchingBegin(KHE_TASK task);
extern void KheTaskMatchingEnd(KHE_TASK task);
extern void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task);
extern void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task);
extern void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight);
extern void KheTaskAddDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskDeleteDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskAttachDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskDetachDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);

/* reference counting */
extern void KheTaskReferenceCountIncrement(KHE_TASK task);
extern void KheTaskReferenceCountDecrement(KHE_TASK task);

/* reading and writing */
extern bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet,
  KML_ERROR *ke);
extern void KheTaskCheckForWriting(KHE_TASK task);
extern bool KheTaskMustWrite(KHE_TASK task);
extern void KheTaskWrite(KHE_TASK task, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_task_bound.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheTaskBoundKernelAdd(KHE_TASK_BOUND tb, KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg);
extern void KheTaskBoundKernelAddUndo(KHE_TASK_BOUND tb);
extern void KheTaskBoundKernelDelete(KHE_TASK_BOUND tb);
extern void KheTaskBoundKernelDeleteUndo(KHE_TASK_BOUND tb, KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg);

/* creation and deletion */
extern void KheTaskBoundUnMake(KHE_TASK_BOUND tb);
extern void KheTaskBoundReferenceCountIncrement(KHE_TASK_BOUND tb);
extern void KheTaskBoundReferenceCountDecrement(KHE_TASK_BOUND tb);

/* copy */
extern KHE_TASK_BOUND KheTaskBoundCopyPhase1(KHE_TASK_BOUND tb);
extern void KheTaskBoundCopyPhase2(KHE_TASK_BOUND tb);

/* solution */
extern void KheTaskBoundSetSoln(KHE_TASK_BOUND tb, KHE_SOLN soln);
extern void KheTaskBoundSetSolnIndex(KHE_TASK_BOUND tb, int soln_index);

/* task bound group */
extern void KheTaskBoundSetTaskBoundGroup(KHE_TASK_BOUND tb,
  KHE_TASK_BOUND_GROUP tbg);
extern void KheTaskBoundSetTaskBoundGroupIndex(KHE_TASK_BOUND tb,
  int tbg_index);

/* task */
extern void KheTaskBoundAddTask(KHE_TASK_BOUND tb, KHE_TASK task);
extern void KheTaskBoundDeleteTask(KHE_TASK_BOUND tb, KHE_TASK task);


/*****************************************************************************/
/*                                                                           */
/*  khe_mark.c                                                               */
/*                                                                           */
/*****************************************************************************/

extern bool KheMarkIsCurrent(KHE_MARK mark);
extern void KheMarkDeletePath(KHE_MARK mark, KHE_PATH path);
extern void KheMarkFree(KHE_MARK mark);
extern void KheMarkDebug(KHE_MARK mark, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_path.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_PATH KhePathMake(KHE_SOLN soln);
extern void KhePathFree(KHE_PATH path);
extern int KhePathCount(KHE_PATH path);

/* copy, undo, and redo */
extern KHE_PATH KhePathCopy(KHE_PATH path, int start_pos, KHE_MARK mark);
extern void KhePathUndo(KHE_PATH path, int start_pos);
extern void KhePathRedo(KHE_PATH path);

/* operation loading - meets */
extern void KhePathOpMeetSetBack(KHE_PATH path,
  KHE_MEET meet, void *old_back, void *new_back);
extern void KhePathOpMeetAdd(KHE_PATH path, KHE_MEET meet, int duration,
  KHE_EVENT e);
extern void KhePathOpMeetDelete(KHE_PATH path, KHE_MEET meet, int duration,
  KHE_EVENT e);
extern void KhePathOpMeetSplit(KHE_PATH path, KHE_MEET meet1, KHE_MEET meet2,
  int durn1);
extern void KhePathOpMeetMerge(KHE_PATH path, KHE_MEET meet1, KHE_MEET meet2,
  int durn1);
extern void KhePathOpMeetMove(KHE_PATH path, KHE_MEET meet,
  KHE_MEET old_target_meet, int old_target_offset,
  KHE_MEET new_target_meet, int new_target_offset);
extern void KhePathOpMeetSetAutoDomain(KHE_PATH path, KHE_MEET meet,
  bool automatic);
extern void KhePathOpMeetAssignFix(KHE_PATH path, KHE_MEET meet);
extern void KhePathOpMeetAssignUnFix(KHE_PATH path, KHE_MEET meet);
extern void KhePathOpMeetAddMeetBound(KHE_PATH path, KHE_MEET meet,
  KHE_MEET_BOUND mb);
extern void KhePathOpMeetDeleteMeetBound(KHE_PATH path, KHE_MEET meet,
  KHE_MEET_BOUND mb);

/* operation loading - meet bounds */
extern void KhePathOpMeetBoundAdd(KHE_PATH path, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern void KhePathOpMeetBoundDelete(KHE_PATH path, KHE_MEET_BOUND mb,
  bool occupancy, KHE_TIME_GROUP dft_tg);
extern void KhePathOpMeetBoundAddTimeGroup(KHE_PATH path, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);
extern void KhePathOpMeetBoundDeleteTimeGroup(KHE_PATH path, KHE_MEET_BOUND mb,
  int duration, KHE_TIME_GROUP tg);

/* operation loading - tasks */
extern void KhePathOpTaskSetBack(KHE_PATH path, KHE_TASK task,
  void *old_back, void *new_back);
extern void KhePathOpTaskAdd(KHE_PATH path, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KhePathOpTaskDelete(KHE_PATH path, KHE_TASK task,
  KHE_RESOURCE_TYPE rt, KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KhePathOpTaskSplit(KHE_PATH path, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2);
extern void KhePathOpTaskMerge(KHE_PATH path, KHE_TASK task1,
  KHE_TASK task2, int durn1, KHE_MEET meet2);
extern void KhePathOpTaskMove(KHE_PATH path, KHE_TASK task,
  KHE_TASK old_target_task, KHE_TASK new_target_task);
extern void KhePathOpTaskAssignFix(KHE_PATH path, KHE_TASK task);
extern void KhePathOpTaskAssignUnFix(KHE_PATH path, KHE_TASK task);
extern void KhePathOpTaskAddTaskBound(KHE_PATH path, KHE_TASK task,
  KHE_TASK_BOUND tb);
extern void KhePathOpTaskDeleteTaskBound(KHE_PATH path, KHE_TASK task,
  KHE_TASK_BOUND tb);

/* operation loading - task bounds */
extern void KhePathOpTaskBoundAdd(KHE_PATH path, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg);
extern void KhePathOpTaskBoundDelete(KHE_PATH path, KHE_TASK_BOUND tb,
  KHE_RESOURCE_GROUP rg);

/* operation loading - nodes */
extern void KhePathOpNodeSetBack(KHE_PATH path,
  KHE_NODE node, void *old_back, void *new_back);
extern void KhePathOpNodeAdd(KHE_PATH path, KHE_NODE node);
extern void KhePathOpNodeDelete(KHE_PATH path, KHE_NODE node);
extern void KhePathOpNodeAddParent(KHE_PATH path,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KhePathOpNodeDeleteParent(KHE_PATH path,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KhePathOpNodeSwapChildNodesAndLayers(KHE_PATH path,
  KHE_NODE node1, KHE_NODE node2);
extern void KhePathOpNodeAddMeet(KHE_PATH path, KHE_NODE node, KHE_MEET meet);
extern void KhePathOpNodeDeleteMeet(KHE_PATH path, KHE_NODE node,KHE_MEET meet);

/* operation loading - layers */
extern void KhePathOpLayerSetBack(KHE_PATH path,
  KHE_LAYER layer, void *old_back, void *new_back);
extern void KhePathOpLayerAdd(KHE_PATH path, KHE_LAYER layer,
  KHE_NODE parent_node);
extern void KhePathOpLayerDelete(KHE_PATH path, KHE_LAYER layer,
  KHE_NODE parent_node);
extern void KhePathOpLayerAddChildNode(KHE_PATH path, KHE_LAYER layer,
  KHE_NODE child_node);
extern void KhePathOpLayerDeleteChildNode(KHE_PATH path, KHE_LAYER layer,
  KHE_NODE child_node);
extern void KhePathOpLayerAddResource(KHE_PATH path, KHE_LAYER layer,
  KHE_RESOURCE resource);
extern void KhePathOpLayerDeleteResource(KHE_PATH path, KHE_LAYER layer,
  KHE_RESOURCE resource);

/* operation loading - zones */
extern void KhePathOpZoneSetBack(KHE_PATH path,
  KHE_ZONE zone, void *old_back, void *new_back);
extern void KhePathOpZoneAdd(KHE_PATH path, KHE_ZONE zone, KHE_NODE node);
extern void KhePathOpZoneDelete(KHE_PATH path, KHE_ZONE zone, KHE_NODE node);
extern void KhePathOpZoneAddMeetOffset(KHE_PATH path,
  KHE_ZONE zone, KHE_MEET meet, int offset);
extern void KhePathOpZoneDeleteMeetOffset(KHE_PATH path,
  KHE_ZONE zone, KHE_MEET meet, int offset);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_in_soln.c                                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENT_IN_SOLN KheEventInSolnMake(KHE_SOLN soln, KHE_EVENT e);
extern KHE_EVENT_IN_SOLN KheEventInSolnCopyPhase1(KHE_EVENT_IN_SOLN es);
extern void KheEventInSolnCopyPhase2(KHE_EVENT_IN_SOLN es);
extern bool KheEventInSolnMakeCompleteRepresentation(KHE_EVENT_IN_SOLN es,
  KHE_EVENT *problem_event);
extern KHE_SOLN KheEventInSolnSoln(KHE_EVENT_IN_SOLN es);
extern KHE_EVENT KheEventInSolnEvent(KHE_EVENT_IN_SOLN es);
extern char *KheEventInSolnId(KHE_EVENT_IN_SOLN es);
extern void KheEventInSolnDelete(KHE_EVENT_IN_SOLN es);

/* meets */
extern void KheEventInSolnAddMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet);
extern void KheEventInSolnDeleteMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet);
extern void KheEventInSolnSplitMeet(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheEventInSolnMergeMeet(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet1, KHE_MEET meet2);
extern int KheEventInSolnMeetCount(KHE_EVENT_IN_SOLN es);
extern KHE_MEET KheEventInSolnMeet(KHE_EVENT_IN_SOLN es, int i);
extern void KheEventInSolnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index);
extern void KheEventInSolnUnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index);
extern int KheEventInSolnMinTimeIndex(KHE_EVENT_IN_SOLN es);
extern int KheEventInSolnMaxTimeIndexPlusDuration(KHE_EVENT_IN_SOLN es);

/* fix and unfix */
extern void KheEventInSolnMeetAssignFix(KHE_EVENT_IN_SOLN es);
extern void KheEventInSolnMeetAssignUnFix(KHE_EVENT_IN_SOLN es);

/* event resources in soln */
extern int KheEventInSolnEventResourceInSolnCount(KHE_EVENT_IN_SOLN es);
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventInSolnEventResourceInSoln(
  KHE_EVENT_IN_SOLN es, int i);

/* reading and writing */
extern void KheEventInSolnWrite(KHE_EVENT_IN_SOLN es, KML_FILE kf,
  bool *event_written);

/* monitors */
extern void KheEventInSolnAttachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern void KheEventInSolnDetachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);

/* user monitors, cost, and timetables */
extern void KheEventInSolnAddMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern void KheEventInSolnDeleteMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern int KheEventInSolnMonitorCount(KHE_EVENT_IN_SOLN es);
extern KHE_MONITOR KheEventInSolnMonitor(KHE_EVENT_IN_SOLN es, int i);
extern KHE_COST KheEventInSolnCost(KHE_EVENT_IN_SOLN es);
extern KHE_COST KheEventInSolnMonitorCost(KHE_EVENT_IN_SOLN es,
  KHE_MONITOR_TAG tag);
extern KHE_TIMETABLE_MONITOR KheEventInSolnTimetableMonitor(
  KHE_EVENT_IN_SOLN es);

/* debug */
void KheEventInSolnDebug(KHE_EVENT_IN_SOLN es, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_resource_in_soln.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventResourceInSolnMake(
  KHE_EVENT_IN_SOLN es, KHE_EVENT_RESOURCE er);
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventResourceInSolnCopyPhase1(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern void KheEventResourceInSolnCopyPhase2(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_EVENT_IN_SOLN KheEventResourceInSolnEventInSoln(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_EVENT_RESOURCE KheEventResourceInSolnEventResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern char *EventResourceInSolnId(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern void KheEventResourceInSolnDelete(KHE_EVENT_RESOURCE_IN_SOLN ers);

/* soln resources */
extern void KheEventResourceInSolnAddTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task);
extern void KheEventResourceInSolnDeleteTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task);
extern void KheEventResourceInSolnSplitTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task1, KHE_TASK task2);
extern void KheEventResourceInSolnMergeTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task1, KHE_TASK task2);
extern void KheEventResourceInSolnAssignResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task, KHE_RESOURCE r);
extern void KheEventResourceInSolnUnAssignResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task, KHE_RESOURCE r);

extern int KheEventResourceInSolnTaskCount(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_TASK KheEventResourceInSolnTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, int i);

/* monitors */
extern void KheEventResourceInSolnAttachMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern void KheEventResourceInSolnDetachMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);

/* fix and unfix */
extern void KheEventResourceInSolnTaskAssignFix(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern void KheEventResourceInSolnTaskAssignUnFix(
  KHE_EVENT_RESOURCE_IN_SOLN ers);

/* user monitors and cost */
extern void KheEventResourceInSolnAddMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern void KheEventResourceInSolnDeleteMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern int KheEventResourceInSolnMonitorCount(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_MONITOR KheEventResourceInSolnMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  int i);
extern KHE_COST KheEventResourceInSolnCost(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_COST KheEventResourceInSolnMonitorCost(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_MONITOR_TAG tag);

/* debug */
void KheEventResourceInSolnDebug(KHE_EVENT_RESOURCE_IN_SOLN ers,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_in_soln.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_RESOURCE_IN_SOLN KheResourceInSolnMake(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_RESOURCE_IN_SOLN KheResourceInSolnCopyPhase1(
  KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnCopyPhase2(KHE_RESOURCE_IN_SOLN rs);
extern KHE_SOLN KheResourceInSolnSoln(KHE_RESOURCE_IN_SOLN rs);
extern KHE_RESOURCE KheResourceInSolnResource(KHE_RESOURCE_IN_SOLN rs);
extern char *KheResourceInSolnId(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnDelete(KHE_RESOURCE_IN_SOLN rs);

/* monitors */
extern void KheResourceInSolnAttachMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);
extern void KheResourceInSolnDetachMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);

/* monitoring calls */
extern void KheResourceInSolnSplitTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2);
extern void KheResourceInSolnMergeTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2);
extern void KheResourceInSolnAssignResource(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task);
extern void KheResourceInSolnUnAssignResource(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task);
extern void KheResourceInSolnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index);
extern void KheResourceInSolnUnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index);

/* workload requirements (for matchings) */
extern int KheResourceInSolnWorkloadRequirementCount(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int i, int *num, KHE_TIME_GROUP *tg, KHE_MONITOR *m);
extern void KheResourceInSolnBeginWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnAddWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int num, KHE_TIME_GROUP tg, KHE_MONITOR m);
extern void KheResourceInSolnEndWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnMatchingSetWeight(KHE_RESOURCE_IN_SOLN rs,
  KHE_COST new_weight);

/* assigned tasks */
extern int KheResourceInSolnAssignedTaskCount(KHE_RESOURCE_IN_SOLN rs);
extern KHE_TASK KheResourceInSolnAssignedTask(KHE_RESOURCE_IN_SOLN rs, int i);

/* user monitors, cost, and timetables */
extern void KheResourceInSolnAddMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m);
extern void KheResourceInSolnDeleteMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);
extern int KheResourceInSolnMonitorCount(KHE_RESOURCE_IN_SOLN rs);
extern KHE_MONITOR KheResourceInSolnMonitor(KHE_RESOURCE_IN_SOLN rs, int i);
extern KHE_COST KheResourceInSolnCost(KHE_RESOURCE_IN_SOLN rs);
extern KHE_COST KheResourceInSolnMonitorCost(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR_TAG tag);
extern KHE_TIMETABLE_MONITOR KheResourceInSolnTimetableMonitor(
  KHE_RESOURCE_IN_SOLN rs);

/* debug */
extern void KheResourceInSolnDebug(KHE_RESOURCE_IN_SOLN rs,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_trace.c                                                              */
/*                                                                           */
/*****************************************************************************/

extern void KheTraceFree(KHE_TRACE t);
extern void KheTraceChangeCost(KHE_TRACE t, KHE_MONITOR m, KHE_COST old_cost);


/*****************************************************************************/
/*                                                                           */
/*  khe_monitor.c                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheMonitorPathCount(KHE_MONITOR lower_m, KHE_MONITOR higher_m);
extern void KheMonitorChangeCost(KHE_MONITOR m, KHE_COST new_cost);
extern KHE_MONITOR_LINK KheMonitorLinkCopyPhase1(KHE_MONITOR_LINK link);
extern void KheMonitorLinkCopyPhase2(KHE_MONITOR_LINK link);
extern void KheMonitorInitCommonFields(KHE_MONITOR m, KHE_SOLN soln,
  KHE_MONITOR_TAG tag);
extern void KheMonitorCopyCommonFieldsPhase1(KHE_MONITOR copy,KHE_MONITOR orig);
extern void KheMonitorCopyCommonFieldsPhase2(KHE_MONITOR orig);
extern void KheMonitorSetSolnIndex(KHE_MONITOR m, int soln_index);
extern KHE_MONITOR KheMonitorCopyPhase1(KHE_MONITOR m);
extern void KheMonitorCopyPhase2(KHE_MONITOR m);
extern void KheMonitorAddParentLink(KHE_MONITOR m, KHE_MONITOR_LINK link);
extern void KheMonitorDeleteParentLink(KHE_MONITOR m, KHE_MONITOR_LINK link);
extern bool KheMonitorHasParentLink(KHE_MONITOR m, KHE_GROUP_MONITOR gm,
  KHE_MONITOR_LINK *link);
extern void KheMonitorDeleteAllParentMonitors(KHE_MONITOR m);
extern void KheMonitorDelete(KHE_MONITOR m);

/* calls emanating from KHE_EVENT_IN_SOLN objects */
extern void KheMonitorAddMeet(KHE_MONITOR m, KHE_MEET meet);
extern void KheMonitorDeleteMeet(KHE_MONITOR m, KHE_MEET meet);
extern void KheMonitorSplitMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2);
extern void KheMonitorMergeMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2);
extern void KheMonitorAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index);
extern void KheMonitorUnAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index);

/* calls emanating from KHE_EVENT_RESOURCE_IN_SOLN objects */
extern void KheMonitorAddTask(KHE_MONITOR m, KHE_TASK task);
extern void KheMonitorDeleteTask(KHE_MONITOR m, KHE_TASK task);
extern void KheMonitorSplitTask(KHE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheMonitorMergeTask(KHE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheMonitorAssignResource(KHE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r);
extern void KheMonitorUnAssignResource(KHE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r);

/* calls emanating from KHE_RESOURCE_IN_SOLN objects */
extern void KheMonitorTaskAssignTime(KHE_MONITOR m,
  KHE_TASK task, int assigned_time_index);
extern void KheMonitorTaskUnAssignTime(KHE_MONITOR m,
  KHE_TASK task, int assigned_time_index);

/* calls emanating from KHE_TIMETABLE_MONITOR objects */
extern void KheMonitorAssignNonClash(KHE_MONITOR m, int assigned_time_index);
extern void KheMonitorUnAssignNonClash(KHE_MONITOR m, int assigned_time_index);
extern void KheMonitorFlush(KHE_MONITOR m);

/* calls emanating from KHE_TIME_GROUP_MONITOR objects */
extern void KheMonitorAddBusyAndIdle(KHE_MONITOR m,
  KHE_TIME_GROUP_MONITOR tgm, int busy_count, int idle_count);
extern void KheMonitorDeleteBusyAndIdle(KHE_MONITOR m,
  KHE_TIME_GROUP_MONITOR tgm, int busy_count, int idle_count);
extern void KheMonitorChangeBusyAndIdle(KHE_MONITOR m,
  KHE_TIME_GROUP_MONITOR tgm, int old_busy_count, int new_busy_count,
  int old_idle_count, int new_idle_count);

/* debug */
extern void KheMonitorDebugWithTagBegin(KHE_MONITOR m, char *tag,
  int indent, FILE *fp);
extern void KheMonitorDebugBegin(KHE_MONITOR m, int indent, FILE *fp);
extern void KheMonitorDebugEnd(KHE_MONITOR m, bool single_line,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_resource_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorCopyPhase1(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorCopyPhase2(KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorDelete(KHE_ASSIGN_RESOURCE_MONITOR m);

/* attach and detach */
extern void KheAssignResourceMonitorAttachToSoln(KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorDetachFromSoln(
  KHE_ASSIGN_RESOURCE_MONITOR m);

/* monitoring calls */
extern void KheAssignResourceMonitorAddTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task);
extern void KheAssignResourceMonitorDeleteTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task);
extern void KheAssignResourceMonitorSplitTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KheAssignResourceMonitorMergeTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KheAssignResourceMonitorAssignResource(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheAssignResourceMonitorUnAssignResource(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheAssignResourceMonitorDeviation(KHE_ASSIGN_RESOURCE_MONITOR m);
extern char *KheAssignResourceMonitorDeviationDescription(
  KHE_ASSIGN_RESOURCE_MONITOR m);

/* debug */
extern void KheAssignResourceMonitorDebug(
  KHE_ASSIGN_RESOURCE_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_time_monitor.c                                                */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorCopyPhase1(
  KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorCopyPhase2(KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorDelete(KHE_ASSIGN_TIME_MONITOR m);

/* attach and detach */
extern void KheAssignTimeMonitorAttachToSoln(KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorDetachFromSoln(KHE_ASSIGN_TIME_MONITOR m);

/* monitoring calls */
extern void KheAssignTimeMonitorAddMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet);
extern void KheAssignTimeMonitorDeleteMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet);
extern void KheAssignTimeMonitorSplitMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheAssignTimeMonitorMergeMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheAssignTimeMonitorAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KheAssignTimeMonitorUnAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KheAssignTimeMonitorDeviation(KHE_ASSIGN_TIME_MONITOR m);
extern char *KheAssignTimeMonitorDeviationDescription(
  KHE_ASSIGN_TIME_MONITOR m);

/* debug */
extern void KheAssignTimeMonitorDebug(KHE_ASSIGN_TIME_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_split_events_monitor.c                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPLIT_EVENTS_MONITOR KheSplitEventsMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_SPLIT_EVENTS_MONITOR KheSplitEventsMonitorCopyPhase1(
  KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorCopyPhase2(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorDelete(KHE_SPLIT_EVENTS_MONITOR m);

/* attach and detach */
extern void KheSplitEventsMonitorAttachToSoln(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorDetachFromSoln(KHE_SPLIT_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheSplitEventsMonitorAddMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSplitEventsMonitorDeleteMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSplitEventsMonitorSplitMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSplitEventsMonitorMergeMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);

/* deviations */
extern int KheSplitEventsMonitorDeviation(KHE_SPLIT_EVENTS_MONITOR m);
extern char *KheSplitEventsMonitorDeviationDescription(
  KHE_SPLIT_EVENTS_MONITOR m);

/* debug */
extern void KheSplitEventsMonitorDebug(KHE_SPLIT_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_distribute_split_events_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR KheDistributeSplitEventsMonitorMake(
  KHE_EVENT_IN_SOLN es, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR
  KheDistributeSplitEventsMonitorCopyPhase1(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorCopyPhase2(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorDelete(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

/* attach and detach */
extern void KheDistributeSplitEventsMonitorAttachToSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorDetachFromSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheDistributeSplitEventsMonitorAddMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet);
extern void KheDistributeSplitEventsMonitorDeleteMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet);
extern void KheDistributeSplitEventsMonitorSplitMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheDistributeSplitEventsMonitorMergeMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);

/* deviations */
extern int KheDistributeSplitEventsMonitorDeviation(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern char *KheDistributeSplitEventsMonitorDeviationDescription(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

/* debug */
extern void KheDistributeSplitEventsMonitorDebug(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_resources_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorCopyPhase1(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorCopyPhase2(KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorDelete(KHE_PREFER_RESOURCES_MONITOR m);

/* attach and detach */
extern void KhePreferResourcesMonitorAttachToSoln(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorDetachFromSoln(
  KHE_PREFER_RESOURCES_MONITOR m);

/* fix and unfix */
extern void KhePreferResourcesMonitorTaskDomainFix(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorTaskDomainUnFix(
  KHE_PREFER_RESOURCES_MONITOR m);

/* monitoring calls */
extern void KhePreferResourcesMonitorAddTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task);
extern void KhePreferResourcesMonitorDeleteTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task);
extern void KhePreferResourcesMonitorSplitTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KhePreferResourcesMonitorMergeTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KhePreferResourcesMonitorAssignResource(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KhePreferResourcesMonitorUnAssignResource(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KhePreferResourcesMonitorDeviation(
  KHE_PREFER_RESOURCES_MONITOR m);
extern char *KhePreferResourcesMonitorDeviationDescription(
  KHE_PREFER_RESOURCES_MONITOR m);

/* debug */
extern void KhePreferResourcesMonitorDebug(
  KHE_PREFER_RESOURCES_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_times_monitor.c                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorCopyPhase1(
  KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorCopyPhase2(KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorDelete(KHE_PREFER_TIMES_MONITOR m);

/* attach and detach */
extern void KhePreferTimesMonitorAttachToSoln(KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorDetachFromSoln(KHE_PREFER_TIMES_MONITOR m);

/* monitoring calls */
extern void KhePreferTimesMonitorAddMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet);
extern void KhePreferTimesMonitorDeleteMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet);
extern void KhePreferTimesMonitorSplitMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KhePreferTimesMonitorMergeMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KhePreferTimesMonitorAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KhePreferTimesMonitorUnAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KhePreferTimesMonitorDeviation(KHE_PREFER_TIMES_MONITOR m);
extern char *KhePreferTimesMonitorDeviationDescription(
  KHE_PREFER_TIMES_MONITOR m);

/* debug */
extern void KhePreferTimesMonitorDebug(KHE_PREFER_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_split_assignments_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR KheAvoidSplitAssignmentsMonitorMake(
  KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index);
extern KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR
  KheAvoidSplitAssignmentsMonitorCopyPhase1(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorCopyPhase2(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorDelete(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheAvoidSplitAssignmentsMonitorConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorEventGroupIndex(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* attach and detach */
extern void KheAvoidSplitAssignmentsMonitorAttachToSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorDetachFromSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* fix and unfix */
extern void KheAvoidSplitAssignmentsMonitorTaskAssignFix(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorTaskAssignUnFix(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* monitoring calls */
extern void KheAvoidSplitAssignmentsMonitorAddTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task);
extern void KheAvoidSplitAssignmentsMonitorDeleteTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task);
extern void KheAvoidSplitAssignmentsMonitorSplitTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheAvoidSplitAssignmentsMonitorMergeTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheAvoidSplitAssignmentsMonitorAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheAvoidSplitAssignmentsMonitorUnAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheAvoidSplitAssignmentsMonitorDeviation(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern char *KheAvoidSplitAssignmentsMonitorDeviationDescription(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* debug */
void KheAvoidSplitAssignmentsMonitorDebug(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_spread_events_monitor.c                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorMake(KHE_SOLN soln,
  KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorCopyPhase1(
  KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorCopyPhase2(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorDelete(KHE_SPREAD_EVENTS_MONITOR m);

/* attach and detach */
extern void KheSpreadEventsMonitorAttachToSoln(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorDetachFromSoln(KHE_SPREAD_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheSpreadEventsMonitorAddMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSpreadEventsMonitorDeleteMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSpreadEventsMonitorSplitMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSpreadEventsMonitorMergeMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSpreadEventsMonitorAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KheSpreadEventsMonitorUnAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KheSpreadEventsMonitorDeviation(KHE_SPREAD_EVENTS_MONITOR m);
extern char *KheSpreadEventsMonitorDeviationDescription(
  KHE_SPREAD_EVENTS_MONITOR m);

/* debug */
extern void KheSpreadEventsMonitorDebug(KHE_SPREAD_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_link_events_monitor.c                                                */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorMake(KHE_SOLN soln,
  KHE_LINK_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorCopyPhase1(
  KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorCopyPhase2(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorDelete(KHE_LINK_EVENTS_MONITOR m);

/* attach and detach */
extern void KheLinkEventsMonitorAttachToSoln(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorDetachFromSoln(KHE_LINK_EVENTS_MONITOR m);

/* fix and unfix */
extern void KheLinkEventsMonitorMeetAssignFix(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorMeetAssignUnFix(KHE_LINK_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheLinkEventsMonitorAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index);
extern void KheLinkEventsMonitorUnAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index);
extern void KheLinkEventsMonitorFlush(KHE_LINK_EVENTS_MONITOR m);

/* deviations */
extern int KheLinkEventsMonitorDeviation(KHE_LINK_EVENTS_MONITOR m);
extern char *KheLinkEventsMonitorDeviationDescription(
  KHE_LINK_EVENTS_MONITOR m);

/* debug */
extern void KheLinkEventsMonitorDebug(KHE_LINK_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_order_events_monitor.c                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ORDER_EVENTS_MONITOR KheOrderEventsMonitorMake(KHE_SOLN soln,
  KHE_ORDER_EVENTS_CONSTRAINT c, KHE_EVENT first_event,
  KHE_EVENT second_event, int min_separation, int max_separation);
extern KHE_ORDER_EVENTS_MONITOR KheOrderEventsMonitorCopyPhase1(
  KHE_ORDER_EVENTS_MONITOR m);
extern void KheOrderEventsMonitorCopyPhase2(KHE_ORDER_EVENTS_MONITOR m);
extern void KheOrderEventsMonitorDelete(KHE_ORDER_EVENTS_MONITOR m);

/* attach and detach */
extern void KheOrderEventsMonitorAttachToSoln(KHE_ORDER_EVENTS_MONITOR m);
extern void KheOrderEventsMonitorDetachFromSoln(KHE_ORDER_EVENTS_MONITOR m);

/* fix and unfix */
extern void KheOrderEventsMonitorMeetAssignFix(KHE_ORDER_EVENTS_MONITOR m);
extern void KheOrderEventsMonitorMeetAssignUnFix(KHE_ORDER_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheOrderEventsMonitorAddMeet(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheOrderEventsMonitorDeleteMeet(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheOrderEventsMonitorSplitMeet(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheOrderEventsMonitorMergeMeet(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheOrderEventsMonitorAssignTime(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KheOrderEventsMonitorUnAssignTime(KHE_ORDER_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KheOrderEventsMonitorDeviation(KHE_ORDER_EVENTS_MONITOR m);
extern char *KheOrderEventsMonitorDeviationDescription(
  KHE_ORDER_EVENTS_MONITOR m);

/* debug */
extern void KheOrderEventsMonitorDebug(KHE_ORDER_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_clashes_monitor.c                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorCopyPhase1(
  KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorCopyPhase2(KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorDelete(KHE_AVOID_CLASHES_MONITOR m);

/* attach and detach */
extern void KheAvoidClashesMonitorAttachToSoln(KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorDetachFromSoln(KHE_AVOID_CLASHES_MONITOR m);

/* monitoring calls */
extern void KheAvoidClashesMonitorChangeClashCount(KHE_AVOID_CLASHES_MONITOR m,
  int old_clash_count, int new_clash_count);
extern void KheAvoidClashesMonitorFlush(KHE_AVOID_CLASHES_MONITOR m);

/* deviations */
extern int KheAvoidClashesMonitorDeviation(KHE_AVOID_CLASHES_MONITOR m);
extern char *KheAvoidClashesMonitorDeviationDescription(
  KHE_AVOID_CLASHES_MONITOR m);

/* debug */
extern void KheAvoidClashesMonitorDebug(KHE_AVOID_CLASHES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_unavailable_times_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_UNAVAILABLE_TIMES_MONITOR KheAvoidUnavailableTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_AVOID_UNAVAILABLE_TIMES_MONITOR
  KheAvoidUnavailableTimesMonitorCopyPhase1(
    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorCopyPhase2(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorDelete(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* attach and detach */
extern void KheAvoidUnavailableTimesMonitorAttachToSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorDetachFromSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* monitoring calls */
extern void KheAvoidUnavailableTimesMonitorAddBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheAvoidUnavailableTimesMonitorChangeBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count,
  int old_idle_count, int new_idle_count);

/* deviations */
extern int KheAvoidUnavailableTimesMonitorDeviation(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern char *KheAvoidUnavailableTimesMonitorDeviationDescription(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* debug */
extern void KheAvoidUnavailableTimesMonitorDebug(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_idle_times_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

/* time group monitors */
void KheLimitIdleTimesMonitorAddTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm);
void KheLimitIdleTimesMonitorDeleteTimeGroupMonitor(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm);

/* attach and detach */
extern void KheLimitIdleTimesMonitorAttachToSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorDetachFromSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

/* monitoring calls */
extern void KheLimitIdleTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheLimitIdleTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheLimitIdleTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count,
  int old_idle_count, int new_idle_count);

/* deviations */
extern int KheLimitIdleTimesMonitorDeviation(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern char *KheLimitIdleTimesMonitorDeviationDescription(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

/* debug */
extern void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_cluster_busy_times_monitor.c                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorCopyPhase1(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorCopyPhase2(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorDelete(KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheClusterBusyTimesMonitorResourceInSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

/* time group monitors */
extern void KheClusterBusyTimesMonitorAddTimeGroupMonitor(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm);
extern void KheClusterBusyTimesMonitorDeleteTimeGroupMonitor(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm);

/* attach and detach */
extern void KheClusterBusyTimesMonitorAttachToSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorDetachFromSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

/* monitoring calls */
extern void KheClusterBusyTimesMonitorAddBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheClusterBusyTimesMonitorDeleteBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheClusterBusyTimesMonitorChangeBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count,
  int old_idle_count, int new_idle_count);

/* deviations */
extern int KheClusterBusyTimesMonitorDeviation(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern char *KheClusterBusyTimesMonitorDeviationDescription(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

/* debug */
extern void KheClusterBusyTimesMonitorDebug(KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_busy_times_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* attach and detach */
extern void KheLimitBusyTimesMonitorAttachToSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorDetachFromSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* monitoring calls */
extern void KheLimitBusyTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheLimitBusyTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int busy_count, int idle_count);
extern void KheLimitBusyTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, KHE_TIME_GROUP_MONITOR tgm,
  int old_busy_count, int new_busy_count, int old_idle_count,
  int new_idle_count);

/* deviations */
extern int KheLimitBusyTimesMonitorDeviation(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern char *KheLimitBusyTimesMonitorDeviationDescription(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* debug */
extern void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_workload_monitor.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorCopyPhase1(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorCopyPhase2(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorDelete(KHE_LIMIT_WORKLOAD_MONITOR m);

/* attach and detach */
extern void KheLimitWorkloadMonitorAttachToSoln(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorDetachFromSoln(KHE_LIMIT_WORKLOAD_MONITOR m);

/* monitoring calls */
extern void KheLimitWorkloadMonitorAssignResource(
  KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheLimitWorkloadMonitorUnAssignResource(
  KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheLimitWorkloadMonitorDeviation(KHE_LIMIT_WORKLOAD_MONITOR m);
extern char *KheLimitWorkloadMonitorDeviationDescription(
  KHE_LIMIT_WORKLOAD_MONITOR m);

/* debug */
extern void KheLimitWorkloadMonitorDebug(KHE_LIMIT_WORKLOAD_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_timetable_monitor.c                                                  */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIMETABLE_MONITOR KheTimetableMonitorMake(KHE_SOLN soln,
  KHE_RESOURCE_IN_SOLN rs, KHE_EVENT_IN_SOLN es);
extern KHE_TIMETABLE_MONITOR KheTimetableMonitorCopyPhase1(
  KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorCopyPhase2(KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorDelete(KHE_TIMETABLE_MONITOR tm);
extern bool KheTimetableMonitorIsOccupied(KHE_TIMETABLE_MONITOR tm,
  int time_index);

/* monitors */
extern void KheTimetableMonitorAttachMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_MONITOR m);
extern void KheTimetableMonitorDetachMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_MONITOR m);
extern bool KheTimetableMonitorContainsTimeGroupMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_TIME_GROUP tg, KHE_TIME_GROUP_MONITOR *tgm);

/* attach and detach */
extern void KheTimetableMonitorAttachToSoln(KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorDetachFromSoln(KHE_TIMETABLE_MONITOR tm);

/* monitoring calls from KHE_EVENT_IN_SOLN */
extern void KheTimetableMonitorAddMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet);
extern void KheTimetableMonitorDeleteMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet);
extern void KheTimetableMonitorSplitMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheTimetableMonitorMergeMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheTimetableMonitorAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index);
extern void KheTimetableMonitorUnAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index);

/* monitoring calls from KHE_RESOURCE_IN_SOLN */
extern void KheTimetableMonitorSplitTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2);
extern void KheTimetableMonitorMergeTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2);
extern void KheTimetableMonitorTaskAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, int assigned_time_index);
extern void KheTimetableMonitorTaskUnAssignTime(
  KHE_TIMETABLE_MONITOR tm, KHE_TASK task, int assigned_time_index);
extern void KheTimetableMonitorAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r);
extern void KheTimetableMonitorUnAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r);

/* debug */
extern void KheTimetableMonitorDebug(KHE_TIMETABLE_MONITOR tm, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group_monitor.c                                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorMake(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorCopyPhase1(
  KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorCopyPhase2(KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorDelete(KHE_TIME_GROUP_MONITOR tgm);

/* attach and detach */
extern void KheTimeGroupMonitorAttachToSoln(KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorDetachFromSoln(KHE_TIME_GROUP_MONITOR tgm);

/* monitors */
extern void KheTimeGroupMonitorAttachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m);
extern void KheTimeGroupMonitorDetachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m);

/* monitoring calls */
extern void KheTimeGroupMonitorAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index);
extern void KheTimeGroupMonitorUnAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index);
extern void KheTimeGroupMonitorFlush(KHE_TIME_GROUP_MONITOR tgm);

/* debug */
extern void KheTimeGroupMonitorDebug(KHE_TIME_GROUP_MONITOR tgm,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_group_monitor.c                                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm);
extern void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm);

/* update */
extern void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t);
extern void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t);
extern void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_MONITOR_LINK link, KHE_COST old_cost, KHE_COST new_cost);

/* debug */
extern void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_ordinary_demand_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_TASK task, int offset);
extern void KheOrdinaryDemandMonitorSetTaskAndOffset(
  KHE_ORDINARY_DEMAND_MONITOR m, KHE_TASK task, int offset);
extern KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorCopyPhase1(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorCopyPhase2(KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorDelete(KHE_ORDINARY_DEMAND_MONITOR m);

/* attach and detach */
extern void KheOrdinaryDemandMonitorAttachToSoln(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorDetachFromSoln(
  KHE_ORDINARY_DEMAND_MONITOR m);

/* monitoring calls */
extern void KheOrdinaryDemandMonitorSetDomain(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_RESOURCE_GROUP rg, KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type);
extern void KheOrdinaryDemandMonitorSetWeight(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_COST new_weight);

/* deviations */
extern int KheOrdinaryDemandMonitorDeviation(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern char *KheOrdinaryDemandMonitorDeviationDescription(
  KHE_ORDINARY_DEMAND_MONITOR m);

/* debug */
extern void KheOrdinaryDemandMonitorDebug(KHE_ORDINARY_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_workload_demand_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_RESOURCE_IN_SOLN rs, KHE_TIME_GROUP tg,
  KHE_MONITOR originating_monitor);
extern KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorCopyPhase1(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorCopyPhase2(KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorDelete(KHE_WORKLOAD_DEMAND_MONITOR m);

/* attach and detach */
extern void KheWorkloadDemandMonitorAttachToSoln(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorDetachFromSoln(
  KHE_WORKLOAD_DEMAND_MONITOR m);

/* monitoring calls */
extern void KheWorkloadDemandMonitorSetWeight(KHE_WORKLOAD_DEMAND_MONITOR m,
  KHE_COST new_weight);

/* deviations */
extern int KheWorkloadDemandMonitorDeviation(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern char *KheWorkloadDemandMonitorDeviationDescription(
  KHE_WORKLOAD_DEMAND_MONITOR m);

/* debug */
extern void KheWorkloadDemandMonitorDebug(KHE_WORKLOAD_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_evenness_monitor.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENNESS_MONITOR KheEvennessMonitorMake(KHE_SOLN soln,
  KHE_RESOURCE_GROUP partition, KHE_TIME time);
extern KHE_EVENNESS_MONITOR KheEvennessMonitorCopyPhase1(
  KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorCopyPhase2(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorDelete(KHE_EVENNESS_MONITOR m);

/* attach and detach */
extern void KheEvennessMonitorAttachToSoln(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorDetachFromSoln(KHE_EVENNESS_MONITOR m);

/* monitoring calls */
void KheEvennessMonitorAddTask(KHE_EVENNESS_MONITOR m);
void KheEvennessMonitorDeleteTask(KHE_EVENNESS_MONITOR m);

/* deviations */
extern int KheEvennessMonitorDeviation(KHE_EVENNESS_MONITOR m);
extern char *KheEvennessMonitorDeviationDescription(KHE_EVENNESS_MONITOR m);

/* debug */
extern void KheEvennessMonitorDebug(KHE_EVENNESS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_node.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheNodeKernelSetBack(KHE_NODE node, void *back);
extern void KheNodeKernelSetBackUndo(KHE_NODE node, void *old_back,
  void *new_back);

extern void KheNodeKernelAdd(KHE_NODE node, KHE_SOLN soln);
extern void KheNodeKernelAddUndo(KHE_NODE node);
extern void KheNodeKernelDelete(KHE_NODE node);
extern void KheNodeKernelDeleteUndo(KHE_NODE node, KHE_SOLN soln);

extern void KheNodeKernelAddParent(KHE_NODE child_node,
  KHE_NODE parent_node);
extern void KheNodeKernelAddParentUndo(KHE_NODE child_node,
  KHE_NODE parent_node);
extern void KheNodeKernelDeleteParent(KHE_NODE child_node,
  KHE_NODE parent_node);
extern void KheNodeKernelDeleteParentUndo(KHE_NODE child_node,
  KHE_NODE parent_node);

extern void KheNodeKernelSwapChildNodesAndLayers(KHE_NODE node1,
  KHE_NODE node2);
extern void KheNodeKernelSwapChildNodesAndLayersUndo(KHE_NODE node1,
  KHE_NODE node2);

extern void KheNodeKernelAddMeet(KHE_NODE node, KHE_MEET meet);
extern void KheNodeKernelAddMeetUndo(KHE_NODE node, KHE_MEET meet);
extern void KheNodeKernelDeleteMeet(KHE_NODE node, KHE_MEET meet);
extern void KheNodeKernelDeleteMeetUndo(KHE_NODE node, KHE_MEET meet);

/* construction and query */
extern void KheNodeUnMake(KHE_NODE node);
extern void KheNodeReferenceCountIncrement(KHE_NODE node);
extern void KheNodeReferenceCountDecrement(KHE_NODE node);
extern KHE_NODE KheNodeCopyPhase1(KHE_NODE node);
extern void KheNodeCopyPhase2(KHE_NODE node);
extern void KheNodeSetSoln(KHE_NODE node, KHE_SOLN soln);
extern void KheNodeSetSolnIndex(KHE_NODE node, int index);

/* parent layers */
extern void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer);
extern void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer);

/* child layers */
extern void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer);
extern void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer);

/* meets */
extern void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet);
extern void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet);
extern void KheNodeAssignedDurationDebug(KHE_NODE node, int verbosity,
  int indent, FILE *fp);

/* zones */
extern void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone);
extern void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone);


/*****************************************************************************/
/*                                                                           */
/*  khe_layer.c                                                              */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheLayerKernelSetBack(KHE_LAYER layer, void *back);
extern void KheLayerKernelSetBackUndo(KHE_LAYER layer, void *old_back,
  void *new_back);

extern void KheLayerKernelAdd(KHE_LAYER layer, KHE_NODE parent_node);
extern void KheLayerKernelAddUndo(KHE_LAYER layer);
extern void KheLayerKernelDelete(KHE_LAYER layer);
extern void KheLayerKernelDeleteUndo(KHE_LAYER layer, KHE_NODE parent_node);

extern void KheLayerKernelAddChildNode(KHE_LAYER layer, KHE_NODE child_node);
extern void KheLayerKernelAddChildNodeUndo(KHE_LAYER layer,
  KHE_NODE child_node);
extern void KheLayerKernelDeleteChildNode(KHE_LAYER layer,
  KHE_NODE child_node);
extern void KheLayerKernelDeleteChildNodeUndo(KHE_LAYER layer,
  KHE_NODE child_node);

extern void KheLayerKernelAddResource(KHE_LAYER layer, KHE_RESOURCE resource);
extern void KheLayerKernelAddResourceUndo(KHE_LAYER layer,
  KHE_RESOURCE resource);
extern void KheLayerKernelDeleteResource(KHE_LAYER layer,
  KHE_RESOURCE resource);
extern void KheLayerKernelDeleteResourceUndo(KHE_LAYER layer,
  KHE_RESOURCE resource);

/* construction and query */
extern void KheLayerUnMake(KHE_LAYER layer);
extern void KheLayerReferenceCountIncrement(KHE_LAYER layer);
extern void KheLayerReferenceCountDecrement(KHE_LAYER layer);
extern void KheLayerSetParentNode(KHE_LAYER layer, KHE_NODE parent_node);
extern void KheLayerSetParentNodeIndex(KHE_LAYER layer, int index);
extern void KheLayerChangeChildNodeIndex(KHE_LAYER layer,
  int old_index, int new_index);
extern KHE_LAYER KheLayerCopyPhase1(KHE_LAYER layer);
extern void KheLayerCopyPhase2(KHE_LAYER layer);

extern void KheLayerAddDuration(KHE_LAYER layer, int durn);
extern void KheLayerSubtractDuration(KHE_LAYER layer, int durn);

/* debug */
extern void KheLayerAssignedDurationDebug(KHE_LAYER layer, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_zone.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* kernel operations */
extern void KheZoneKernelSetBack(KHE_ZONE zone, void *back);
extern void KheZoneKernelSetBackUndo(KHE_ZONE zone, void *old_back,
  void *new_back);

extern void KheZoneKernelAdd(KHE_ZONE zone, KHE_NODE node);
extern void KheZoneKernelAddUndo(KHE_ZONE zone);
extern void KheZoneKernelDelete(KHE_ZONE zone);
extern void KheZoneKernelDeleteUndo(KHE_ZONE zone, KHE_NODE node);

extern void KheZoneKernelAddMeetOffset(KHE_ZONE zone, KHE_MEET meet,
  int offset);
extern void KheZoneKernelAddMeetOffsetUndo(KHE_ZONE zone, KHE_MEET meet,
  int offset);
extern void KheZoneKernelDeleteMeetOffset(KHE_ZONE zone, KHE_MEET meet,
  int offset);
extern void KheZoneKernelDeleteMeetOffsetUndo(KHE_ZONE zone, KHE_MEET meet,
  int offset);

/* construction and query */
extern void KheZoneUnMake(KHE_ZONE zone);
extern void KheZoneReferenceCountIncrement(KHE_ZONE zone);
extern void KheZoneReferenceCountDecrement(KHE_ZONE zone);
extern void KheZoneSetNode(KHE_ZONE zone, KHE_NODE node);
extern void KheZoneSetNodeIndex(KHE_ZONE zone, int node_index);
extern KHE_ZONE KheZoneCopyPhase1(KHE_ZONE zone);
extern void KheZoneCopyPhase2(KHE_ZONE zone);
extern void KheZoneUpdateMeetOffset(KHE_ZONE zone, KHE_MEET old_meet,
  int old_offset, KHE_MEET new_meet, int new_offset);


/*****************************************************************************/
/*                                                                           */
/*  khe_tasking.c                                                            */
/*                                                                           */
/*****************************************************************************/

extern KHE_TASKING KheTaskingCopyPhase1(KHE_TASKING tasking);
extern void KheTaskingCopyPhase2(KHE_TASKING tasking);
extern int KheTaskingSolnIndex(KHE_TASKING tasking);
extern void KheTaskingSetSolnIndex(KHE_TASKING tasking, int val);
extern void KheTaskingFree(KHE_TASKING tasking);
