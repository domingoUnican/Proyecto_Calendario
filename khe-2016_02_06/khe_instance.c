
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
/*  FILE:         khe_instance.c                                             */
/*  DESCRIPTION:  One instance of the high school timetabling problem        */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0
#define DEBUG3 0
#define DEBUG4 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE - one instance of the high school timetabling problem       */
/*                                                                           */
/*****************************************************************************/

struct khe_instance_rec {
  void				*back;			/* back pointer      */
  ARRAY_KHE_ARCHIVE		archives;		/* archives          */
  char				*id;			/* optional id       */
  KHE_INSTANCE_METADATA		meta_data;		/* instance metadata */
  ARRAY_KHE_TIME_GROUP		time_group_array;	/* user's time groups*/
  TABLE_KHE_TIME_GROUP		time_group_table;	/* time groups       */
  KHE_TIME_GROUP		empty_time_group;	/* empty time group  */
  KHE_TIME_GROUP		full_time_group;	/* full time group   */
  ARRAY_KHE_TIME		time_array;		/* times             */
  TABLE_KHE_TIME		time_table;		/* times symtab      */
  ARRAY_KHE_RESOURCE_TYPE	resource_type_array;	/* resource types    */
  TABLE_KHE_RESOURCE_TYPE	resource_type_table;	/* resource types    */
  ARRAY_KHE_RESOURCE_GROUP	partition_array;	/* all partitions    */
  ARRAY_KHE_RESOURCE		resource_array;		/* all resources     */
  ARRAY_KHE_EVENT_GROUP		event_group_array;	/* event groups      */
  TABLE_KHE_EVENT_GROUP		event_group_table;	/* event groups      */
  KHE_EVENT_GROUP		full_event_group;	/* full event group  */
  KHE_EVENT_GROUP		empty_event_group;	/* empty event group */
  ARRAY_KHE_EVENT		event_array;		/* events            */
  TABLE_KHE_EVENT		event_table;		/* events            */
  int				max_event_duration;	/* max event durn    */
  ARRAY_KHE_EVENT_RESOURCE	event_resources;	/* event resources   */
  ARRAY_KHE_CONSTRAINT		constraint_array;	/* constraints       */
  TABLE_KHE_CONSTRAINT		constraint_table;	/* constraints       */
  ARRAY_INT			density_counts;         /* density counts    */
  ARRAY_INT			density_totals;         /* density totals    */
  KHE_TIME_GROUP_NHOOD		singleton_tgn;		/* singleton tgn     */
  bool				complete;		/* completely read   */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive)        */
/*                                                                           */
/*  Add archive to ins, assuming it is safe to do so.                        */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive)
{
  MArrayAddLast(ins->archives, archive);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceDeleteArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive)     */
/*                                                                           */
/*  Delete archive from ins.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheInstanceDeleteArchive(KHE_INSTANCE ins, KHE_ARCHIVE archive)
{
  int pos;
  if( !MArrayContains(ins->archives, archive, &pos) )
    MAssert(false, "KheInstanceDeleteArchive internal error");
  MArrayRemove(ins->archives, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheInstanceMakeBegin(char *id, KHE_INSTANCE_METADATA md)    */
/*                                                                           */
/*  Make an initially empty instance with these attributes.                  */
/*                                                                           */
/*  Also add the `all' subgroups for time groups and event groups.           */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheInstanceMakeBegin(char *id, KHE_INSTANCE_METADATA md)
{
  KHE_INSTANCE res;

  MMake(res);
  res->back = NULL;
  MArrayInit(res->archives);
  res->id = id;
  res->meta_data = md;
  MArrayInit(res->time_group_array);
  MTableInit(res->time_group_table);
  res->empty_time_group = KheTimeGroupMakeInternal(res,
    KHE_TIME_GROUP_TYPE_EMPTY, NULL, KHE_TIME_GROUP_KIND_ORDINARY,
    NULL, NULL, LSetNew());
  res->full_time_group = KheTimeGroupMakeInternal(res,
    KHE_TIME_GROUP_TYPE_FULL, NULL, KHE_TIME_GROUP_KIND_ORDINARY,
    NULL, NULL, LSetNew());
  MArrayInit(res->time_array);
  MTableInit(res->time_table);
  MArrayInit(res->resource_type_array);
  MTableInit(res->resource_type_table);
  MArrayInit(res->partition_array);
  MArrayInit(res->resource_array);
  MArrayInit(res->event_group_array);
  MTableInit(res->event_group_table);
  res->full_event_group = KheEventGroupMakeInternal(res,
    KHE_EVENT_GROUP_TYPE_FULL, NULL, KHE_EVENT_GROUP_KIND_ORDINARY, NULL, NULL);
  res->empty_event_group = KheEventGroupMakeInternal(res,
    KHE_EVENT_GROUP_TYPE_EMPTY, NULL, KHE_EVENT_GROUP_KIND_ORDINARY, NULL,NULL);
  MArrayInit(res->event_array);
  MTableInit(res->event_table);
  res->max_event_duration = 0;
  MArrayInit(res->event_resources);
  MArrayInit(res->constraint_array);
  MTableInit(res->constraint_table);
  MArrayInit(res->density_counts);
  MArrayInit(res->density_totals);
  res->singleton_tgn = NULL;  /* initialized when finalizing */
  res->complete = false;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceSetBack(KHE_INSTANCE ins, void *back)                    */
/*                                                                           */
/*  Set the back pointer of ins.                                             */
/*                                                                           */
/*****************************************************************************/

void KheInstanceSetBack(KHE_INSTANCE ins, void *back)
{
  MAssert(!KheInstanceComplete(ins),
    "KheInstanceSetBack called after KheInstanceMakeEnd");
  ins->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheInstanceBack(KHE_INSTANCE ins)                                  */
/*                                                                           */
/*  Return the back pointer of ins.                                          */
/*                                                                           */
/*****************************************************************************/

void *KheInstanceBack(KHE_INSTANCE ins)
{
  return ins->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_NHOOD KheInstanceSingletonTimeGroupNeighbourhood(         */
/*    KHE_INSTANCE ins)                                                      */
/*                                                                           */
/*  Return the unique singleton time group neighbourhood.                    */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_NHOOD KheInstanceSingletonTimeGroupNeighbourhood(
  KHE_INSTANCE ins)
{
  return ins->singleton_tgn;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceMakeEnd(KHE_INSTANCE ins, bool infer_resource_partitions)*/
/*                                                                           */
/*  End the construction of ins; initialize data structures.                 */
/*                                                                           */
/*****************************************************************************/

void KheInstanceMakeEnd(KHE_INSTANCE ins, bool infer_resource_partitions)
{
  int i; KHE_TIME_GROUP tg;  KHE_EVENT_GROUP eg;  KHE_RESOURCE r;
  KHE_TIME t;  KHE_RESOURCE_TYPE rt;  KHE_EVENT e;  KHE_CONSTRAINT c;
  if( DEBUG4 )
    fprintf(stderr, "[ KheInstanceMakeEnd(ins, %s)\n",
      infer_resource_partitions ? "true" : "false");
  MAssert(!ins->complete, "KheInstanceMakeEnd called twice");

  /* finalize time groups and time nhoods */
  ins->singleton_tgn = KheTimeGroupNHoodMakeEmpty(MArraySize(ins->time_array));
  MArrayForEach(ins->time_group_array, &tg, &i)
    KheTimeGroupFinalize(tg, NULL, NULL, 0);
  KheTimeGroupFinalize(ins->empty_time_group, NULL, NULL, 0);
  KheTimeGroupFinalize(ins->full_time_group, NULL, NULL, 0);
  MArrayForEach(ins->time_array, &t, &i)
  {
    tg = KheTimeSingletonTimeGroup(t);
    KheTimeGroupNHoodSetTimeGroup(ins->singleton_tgn, i, tg);
    KheTimeGroupFinalize(tg, NULL, ins->singleton_tgn, i);
  }

  /* finalize resource types */
  MArrayForEach(ins->resource_type_array, &rt, &i)
    KheResourceTypeFinalize(rt);
  
  /* finalize event groups */
  MArrayForEach(ins->event_group_array, &eg, &i)
    KheEventGroupSetEventsArrayInternal(eg);
  KheEventGroupSetEventsArrayInternal(ins->full_event_group);
  KheEventGroupSetEventsArrayInternal(ins->empty_event_group);
  MArrayForEach(ins->event_array, &e, &i)
    KheEventGroupSetEventsArrayInternal(KheEventSingletonEventGroup(e));

  /* record completeness (KheEventFinalize assumes this flag is set) */
  ins->complete = true;

  /* finalize constraints (must be done after inferring time breaks) */
  MArrayForEach(ins->constraint_array, &c, &i)
    KheConstraintFinalize(c);

  /* finalize events (must be done after finalizing constraints) */
  MArrayForEach(ins->event_array, &e, &i)
    KheEventFinalize(e);

  /* finalize resources (must be done after finalizing constraints) */
  MArrayForEach(ins->resource_array, &r, &i)
    KheResourceFinalize(r);

  /* infer resource partitions, if required */
  if( infer_resource_partitions )
  {
    MArrayForEach(ins->event_array, &e, &i)
      KheEventPartitionSetAdmissible(e);
    MArrayForEach(ins->resource_type_array, &rt, &i)
      if( !KheResourceTypeHasPartitions(rt) )
	KheResourceTypeInferPartitions(rt);
  }

  if( DEBUG4 )
    fprintf(stderr, "] KheInstanceMakeEnd returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceArchiveCount(KHE_INSTANCE ins)                            */
/*                                                                           */
/*  Return the number of archives containing ins.                            */
/*                                                                           */
/*****************************************************************************/

int KheInstanceArchiveCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->archives);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE KheInstanceArchive(KHE_INSTANCE ins, int i)                  */
/*                                                                           */
/*  Return the i'th archive containing ins.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE KheInstanceArchive(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->archives, i);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceId(KHE_INSTANCE ins)                                    */
/*                                                                           */
/*  Return the Id attribute of ins.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceId(KHE_INSTANCE ins)
{
  return ins->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceName(KHE_INSTANCE ins)                                  */
/*                                                                           */
/*  Convenience function which returns the name of ins.                      */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceName(KHE_INSTANCE ins)
{
  return KheInstanceMetaDataName(KheInstanceMetaData(ins));
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE_METADATA KheInstanceMetaData(KHE_INSTANCE ins)              */
/*                                                                           */
/*  Return the metadata attribute of ins.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE_METADATA KheInstanceMetaData(KHE_INSTANCE ins)
{
  return ins->meta_data;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceSetMetaData(KHE_INSTANCE ins, KHE_INSTANCE_METADATA md)  */
/*                                                                           */
/*  Set the metadata attribute of ins.                                       */
/*                                                                           */
/*****************************************************************************/

void KheInstanceSetMetaData(KHE_INSTANCE ins, KHE_INSTANCE_METADATA md)
{
  ins->meta_data = md;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceComplete(KHE_INSTANCE ins)                               */
/*                                                                           */
/*  Return true if ins is complete.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceComplete(KHE_INSTANCE ins)
{
  return ins->complete;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time groups"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddTimeGroup(KHE_INSTANCE ins, KHE_TIME_GROUP tg)        */
/*                                                                           */
/*  Add user-defined time group tg to ins, assuming it is safe to do so.     */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddTimeGroup(KHE_INSTANCE ins, KHE_TIME_GROUP tg)
{
  MArrayAddLast(ins->time_group_array, tg);
  if( KheTimeGroupId(tg) != NULL )
    MTableInsert(ins->time_group_table, KheTimeGroupId(tg), tg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceTimeGroupCount(KHE_INSTANCE ins)                          */
/*                                                                           */
/*  Return the number of time groups in ins.                                 */
/*                                                                           */
/*****************************************************************************/

int KheInstanceTimeGroupCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->time_group_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheInstanceTimeGroup(KHE_INSTANCE ins, int i)             */
/*                                                                           */
/*  Return the i'th time group of ins.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheInstanceTimeGroup(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->time_group_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveTimeGroup(KHE_INSTANCE ins, char *id,            */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  Retrieve a time group by Id.                                             */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveTimeGroup(KHE_INSTANCE ins, char *id,
  KHE_TIME_GROUP *tg)
{
  int pos;
  return MTableRetrieve(ins->time_group_table, id, tg, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheInstanceEmptyTimeGroup(KHE_INSTANCE ins)               */
/*                                                                           */
/*  Return the `empty' time group of ins.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheInstanceEmptyTimeGroup(KHE_INSTANCE ins)
{
  return ins->empty_time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheInstanceFullTimeGroupInternal(KHE_INSTANCE ins)        */
/*                                                                           */
/*  Return the `all' time group of ins, even before instance is complete.    */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheInstanceFullTimeGroupInternal(KHE_INSTANCE ins)
{
  /* return MArrayFirst(ins->packing_time_groups); */
  return ins->full_time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheInstanceFullTimeGroup(KHE_INSTANCE ins)                 */
/*                                                                           */
/*  Return the `all' time group of ins.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheInstanceFullTimeGroup(KHE_INSTANCE ins)
{
  MAssert(ins->complete,
    "KheInstanceFullTimeGroup called before KheInstanceMakeEnd");
  return KheInstanceFullTimeGroupInternal(ins);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddTime(KHE_INSTANCE ins, KHE_TIME t)                    */
/*                                                                           */
/*  Add t to ins.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddTime(KHE_INSTANCE ins, KHE_TIME t)
{
  MArrayAddLast(ins->time_array, t);
  if( KheTimeId(t) != NULL )
    MTableInsert(ins->time_table, KheTimeId(t), t);
  KheTimeGroupAddTimeInternal(ins->full_time_group, t);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceTimeCount(KHE_INSTANCE ins)                               */
/*                                                                           */
/*  Return the number of times in ins.                                       */
/*                                                                           */
/*****************************************************************************/

int KheInstanceTimeCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->time_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheInstanceTime(KHE_INSTANCE ins, int i)                        */
/*                                                                           */
/*  Return the i'th time of ins.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheInstanceTime(KHE_INSTANCE ins, int i)
{
  MAssert(i >= 0 && i < MArraySize(ins->time_array),
    "KheInstanceTime: i (%d) out of range (0 - %d)", i,
    MArraySize(ins->time_array) - 1);
  return MArrayGet(ins->time_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveTime(KHE_INSTANCE ins, char *id, KHE_TIME *t)    */
/*                                                                           */
/*  Retrieve a time from ins.                                                */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveTime(KHE_INSTANCE ins, char *id, KHE_TIME *t)
{
  int pos;
  return MTableRetrieve(ins->time_table, id, t, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource types"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddResourceType(KHE_INSTANCE ins, KHE_RESOURCE_TYPE rt,  */
/*    int *index)                                                            */
/*                                                                           */
/*  Add rt to ins, setting *index to its index.                              */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddResourceType(KHE_INSTANCE ins, KHE_RESOURCE_TYPE rt,
  int *index)
{
  if( DEBUG3 )
    fprintf(stderr, "KheInstanceAddResourceType(%p, %p %s)\n",
      (void *) ins, (void *) rt, KheResourceTypeId(rt) != NULL ?
      KheResourceTypeId(rt) : "-");
  *index = MArraySize(ins->resource_type_array);
  MArrayAddLast(ins->resource_type_array, rt);
  if( KheResourceTypeId(rt) != NULL )
  {
    MTableInsert(ins->resource_type_table, KheResourceTypeId(rt), rt);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceResourceTypeCount(KHE_INSTANCE ins)                       */
/*                                                                           */
/*  Return the number of resource types of ins.                              */
/*                                                                           */
/*****************************************************************************/

int KheInstanceResourceTypeCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->resource_type_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheInstanceResourceType(KHE_INSTANCE ins, int i)       */
/*                                                                           */
/*  Return the i'th resource type of ins.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheInstanceResourceType(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->resource_type_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveResourceType(KHE_INSTANCE ins, char *id,         */
/*    KHE_RESOURCE_TYPE *rt)                                                 */
/*                                                                           */
/*  Retrieve a resource type by Id.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveResourceType(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_TYPE *rt)
{
  int pos;
  if( DEBUG3 )
    fprintf(stderr, "KheInstanceRetrieveResourceType(%p, %s)\n",
      (void *) ins, id);
  return MTableRetrieve(ins->resource_type_table, id, rt, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partitions"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddPartition(KHE_INSTANCE ins, KHE_RESOURCE_GROUP rg,    */
/*    int *index)                                                            */
/*                                                                           */
/*  Add partition rg to ins, returning its index in *index.                  */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddPartition(KHE_INSTANCE ins, KHE_RESOURCE_GROUP rg,
  int *index)
{
  *index = MArraySize(ins->partition_array);
  MArrayAddLast(ins->partition_array, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstancePartitionCount(KHE_INSTANCE ins)                          */
/*                                                                           */
/*  Return the number of partitions of ins.                                  */
/*                                                                           */
/*****************************************************************************/

int KheInstancePartitionCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->partition_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheInstancePartition(KHE_INSTANCE ins, int i)         */
/*                                                                           */
/*  Return the i'th partition of ins.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheInstancePartition(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->partition_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource groups and resources"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveResourceGroup(KHE_INSTANCE ins, char *id,        */
/*    KHE_RESOURCE_GROUP *rg)                                                */
/*                                                                           */
/*  Retrieve a resource group by Id from the resource types of ins.          */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveResourceGroup(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_GROUP *rg)
{
  int i;  KHE_RESOURCE_TYPE rt;
  MArrayForEach(ins->resource_type_array, &rt, &i)
    if( KheResourceTypeRetrieveResourceGroup(rt, id, rg) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveResource(KHE_INSTANCE ins, char *id,             */
/*    KHE_RESOURCE *r)                                                       */
/*                                                                           */
/*  Retrieve a resource by Id from the resource types of ins.                */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveResource(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE *r)
{
  int i;  KHE_RESOURCE_TYPE rt;
  MArrayForEach(ins->resource_type_array, &rt, &i)
    if( KheResourceTypeRetrieveResource(rt, id, r) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceResourceCount(KHE_INSTANCE ins)                           */
/*                                                                           */
/*  Return the total number of resources in the instance.                    */
/*                                                                           */
/*****************************************************************************/

int KheInstanceResourceCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->resource_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheInstanceResource(KHE_INSTANCE ins, int i)                */
/*                                                                           */
/*  Return the i'th resource of ins.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheInstanceResource(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->resource_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddResource(KHE_INSTANCE ins, KHE_RESOURCE r)            */
/*                                                                           */
/*  Add r to ins.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddResource(KHE_INSTANCE ins, KHE_RESOURCE r)
{
  MArrayAddLast(ins->resource_array, r);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event groups"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddEventGroup(KHE_INSTANCE ins, KHE_EVENT_GROUP eg)      */
/*                                                                           */
/*  Add eg to ins.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddEventGroup(KHE_INSTANCE ins, KHE_EVENT_GROUP eg)
{
  if( DEBUG1 )
    fprintf(stderr, "KheInstanceAddEventGroup(ins, \"%s\")\n",
      KheEventGroupId(eg) != NULL ? KheEventGroupId(eg) : "<null>");
  MArrayAddLast(ins->event_group_array, eg);
  if( KheEventGroupId(eg) != NULL )
    MTableInsert(ins->event_group_table, KheEventGroupId(eg), eg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceEventGroupCount(KHE_INSTANCE ins)                         */
/*                                                                           */
/*  Return the number of event groups of ins.                                */
/*                                                                           */
/*****************************************************************************/

int KheInstanceEventGroupCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->event_group_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheInstanceEventGroup(KHE_INSTANCE ins, int i)           */
/*                                                                           */
/*  Return the i'th event group of ins.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheInstanceEventGroup(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->event_group_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveEventGroup(KHE_INSTANCE ins, char *id,           */
/*    KHE_EVENT_GROUP *eg)                                                   */
/*                                                                           */
/*  Retrieve an event group from ins by id.                                  */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveEventGroup(KHE_INSTANCE ins, char *id,
  KHE_EVENT_GROUP *eg)
{
  int pos;
  if( DEBUG1 )
    fprintf(stderr, "KheInstanceRetrieveEventGroup(ins, \"%s\")\n", id);
  return MTableRetrieve(ins->event_group_table, id, eg, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheInstanceFullEventGroup(KHE_INSTANCE ins)              */
/*                                                                           */
/*  Return the `all' event group of ins.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheInstanceFullEventGroup(KHE_INSTANCE ins)
{
  return ins->full_event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheInstanceEmptyEventGroup(KHE_INSTANCE ins)             */
/*                                                                           */
/*  Return the empty event group of ins.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheInstanceEmptyEventGroup(KHE_INSTANCE ins)
{
  return ins->empty_event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddEvent(KHE_INSTANCE ins, KHE_EVENT e)                  */
/*                                                                           */
/*  Add e to ins.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddEvent(KHE_INSTANCE ins, KHE_EVENT e)
{
  MArrayAddLast(ins->event_array, e);
  if( KheEventId(e) != NULL )
    MTableInsert(ins->event_table, KheEventId(e), e);
  if( KheEventDuration(e) > ins->max_event_duration )
    ins->max_event_duration = KheEventDuration(e);
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_KHE_EVENT KheInstanceEventsArray(KHE_INSTANCE ins)                 */
/*                                                                           */
/*  Return the event_array array of ins.                                     */
/*                                                                           */
/*****************************************************************************/

ARRAY_KHE_EVENT KheInstanceEventsArray(KHE_INSTANCE ins)
{
  return ins->event_array;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceMaxEventDuration(KHE_INSTANCE ins)                        */
/*                                                                           */
/*  Return the maximum duration of any event of ins.                         */
/*                                                                           */
/*****************************************************************************/

int KheInstanceMaxEventDuration(KHE_INSTANCE ins)
{
  return ins->max_event_duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceEventCount(KHE_INSTANCE ins)                              */
/*                                                                           */
/*  Return the number of events in ins.                                      */
/*                                                                           */
/*****************************************************************************/

int KheInstanceEventCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->event_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheInstanceEvent(KHE_INSTANCE ins, int i)                      */
/*                                                                           */
/*  Return the i'th event of ins.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheInstanceEvent(KHE_INSTANCE ins, int i)
{
  MAssert(i >= 0 && i < MArraySize(ins->event_array),
    "KheInstanceEvent: i out of range");
  return MArrayGet(ins->event_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveEvent(KHE_INSTANCE ins, char *id, KHE_EVENT *e)  */
/*                                                                           */
/*  Retrieve an event of ins by id.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveEvent(KHE_INSTANCE ins, char *id, KHE_EVENT *e)
{
  int pos;
  return MTableRetrieve(ins->event_table, id, e, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddEventResource(KHE_INSTANCE ins, KHE_EVENT_RESOURCE er)*/
/*                                                                           */
/*  Return a new index number for an event resource.                         */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddEventResource(KHE_INSTANCE ins, KHE_EVENT_RESOURCE er)
{
  MArrayAddLast(ins->event_resources, er);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceEventResourceCount(KHE_INSTANCE ins)                      */
/*                                                                           */
/*  Return the number of event resources in ins.                             */
/*                                                                           */
/*****************************************************************************/

int KheInstanceEventResourceCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheInstanceEventResource(KHE_INSTANCE ins, int i)     */
/*                                                                           */
/*  Return the i'th event resource of ins.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheInstanceEventResource(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->event_resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraints"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceAddConstraint(KHE_INSTANCE ins, KHE_CONSTRAINT c)        */
/*                                                                           */
/*  Add c to ins.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheInstanceAddConstraint(KHE_INSTANCE ins, KHE_CONSTRAINT c)
{
  MArrayAddLast(ins->constraint_array, c);
  if( KheConstraintId(c) != NULL )
    MTableInsert(ins->constraint_table, KheConstraintId(c), c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceConstraintCount(KHE_INSTANCE ins)                         */
/*                                                                           */
/*  Return the number of constraints of ins.                                 */
/*                                                                           */
/*****************************************************************************/

int KheInstanceConstraintCount(KHE_INSTANCE ins)
{
  return MArraySize(ins->constraint_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheInstanceConstraint(KHE_INSTANCE ins, int i)            */
/*                                                                           */
/*  Return the i'th constraint of ins.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheInstanceConstraint(KHE_INSTANCE ins, int i)
{
  return MArrayGet(ins->constraint_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRetrieveConstraint(KHE_INSTANCE ins, char *id,           */
/*    KHE_CONSTRAINT *c)                                                     */
/*                                                                           */
/*  Retrieve a constraint from ins by id.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceRetrieveConstraint(KHE_INSTANCE ins, char *id,
  KHE_CONSTRAINT *c)
{
  int pos;
  return MTableRetrieve(ins->constraint_table, id, c, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceSetConstraintDensities(KHE_INSTANCE ins)                 */
/*                                                                           */
/*  Set the constraint densities of ins.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheInstanceSetConstraintDensities(KHE_INSTANCE ins)
{
  KHE_CONSTRAINT_TAG tag;  KHE_CONSTRAINT c;  KHE_EVENT_RESOURCE er;
  KHE_EVENT e;  int i, count, unpreass_event_resources, unpreass_events;

  /* find the number of unpreassigned event resources */
  unpreass_event_resources = 0;
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( KheEventResourcePreassignedResource(er) == NULL )
      unpreass_event_resources++;
  }

  /* find the number of unpreassigned events */
  unpreass_events = 0;
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    if( KheEventPreassignedTime(e) == NULL )
      unpreass_events++;
  }

  /* initialize the density_counts and density_totals arrays to 0 */
  for( tag = 0;  tag < KHE_CONSTRAINT_TAG_COUNT;  tag++ )
  {
    MArrayAddLast(ins->density_counts, 0);
    MArrayAddLast(ins->density_totals, 0);
  }

  /* set the density totals */
  MArrayPut(ins->density_totals, KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG,
    unpreass_event_resources);
  MArrayPut(ins->density_totals, KHE_ASSIGN_TIME_CONSTRAINT_TAG,
    unpreass_events);
  MArrayPut(ins->density_totals, KHE_SPLIT_EVENTS_CONSTRAINT_TAG,
    KheInstanceEventCount(ins));
  MArrayPut(ins->density_totals, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG,
    KheInstanceEventCount(ins));
  MArrayPut(ins->density_totals, KHE_PREFER_RESOURCES_CONSTRAINT_TAG,
    unpreass_event_resources);
  MArrayPut(ins->density_totals, KHE_PREFER_TIMES_CONSTRAINT_TAG,
    unpreass_events);
  MArrayPut(ins->density_totals, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG,
    unpreass_event_resources);
  MArrayPut(ins->density_totals, KHE_SPREAD_EVENTS_CONSTRAINT_TAG,
    KheInstanceEventCount(ins));
  MArrayPut(ins->density_totals, KHE_LINK_EVENTS_CONSTRAINT_TAG,
    KheInstanceEventCount(ins));
  MArrayPut(ins->density_totals, KHE_ORDER_EVENTS_CONSTRAINT_TAG,
    KheInstanceEventCount(ins));
  MArrayPut(ins->density_totals, KHE_AVOID_CLASHES_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));
  MArrayPut(ins->density_totals, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));
  MArrayPut(ins->density_totals, KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));
  MArrayPut(ins->density_totals, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));
  MArrayPut(ins->density_totals, KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));
  MArrayPut(ins->density_totals, KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG,
    KheInstanceResourceCount(ins));

  /* accumulate the counts for each kind of constraint */
  MArrayForEach(ins->constraint_array, &c, &i)
  {
    tag = KheConstraintTag(c);
    count = MArrayGet(ins->density_counts, tag);
    MArrayPut(ins->density_counts, tag, count + KheConstraintDensityCount(c));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceConstraintDensityCount(KHE_INSTANCE ins,                  */
/*    KHE_CONSTRAINT_TAG constraint_tag)                                     */
/*                                                                           */
/*  Return the density count of constraints with this tag.                   */
/*                                                                           */
/*****************************************************************************/

int KheInstanceConstraintDensityCount(KHE_INSTANCE ins,
  KHE_CONSTRAINT_TAG constraint_tag)
{
  if( MArraySize(ins->density_counts) == 0 )
    KheInstanceSetConstraintDensities(ins);
  return MArrayGet(ins->density_counts, constraint_tag);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheInstanceConstraintDensityTotal(KHE_INSTANCE ins,                  */
/*    KHE_CONSTRAINT_TAG constraint_tag)                                     */
/*                                                                           */
/*  Return the density total of constraints with this tag.                   */
/*                                                                           */
/*****************************************************************************/

int KheInstanceConstraintDensityTotal(KHE_INSTANCE ins,
  KHE_CONSTRAINT_TAG constraint_tag)
{
  if( MArraySize(ins->density_counts) == 0 )
    KheInstanceSetConstraintDensities(ins);
  return MArrayGet(ins->density_totals, constraint_tag);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool AddTimes(KML_ELT times_elt, KHE_INSTANCE ins, KML_ERROR *ke)        */
/*                                                                           */
/*  Add time groups and times to ins based on times_elt.                     */
/*                                                                           */
/*****************************************************************************/

static bool AddTimes(KML_ELT times_elt, KHE_INSTANCE ins, KML_ERROR *ke)
{
  KML_ELT time_groups_elt, time_group_elt, time_elt;
  int i, first_time_index;

  /* verify times_elt */
  if( !KmlCheck(times_elt, ": +TimeGroups *Time", ke) )
    return false;

  /* TimeGroups */
  first_time_index = 0;
  if( KmlContainsChild(times_elt, "TimeGroups", &time_groups_elt) )
  {
    first_time_index = 1;
    if( !KmlCheck(time_groups_elt, ": *Week *Day *TimeGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(time_groups_elt);  i++ )
    {
      time_group_elt = KmlChild(time_groups_elt, i);
      if( !KheTimeGroupMakeFromKml(time_group_elt, ins, ke) )
	return false;
    }
  }

  /* times */
  for( i = first_time_index;  i < KmlChildCount(times_elt);  i++ )
  {
    /* make and add one time */
    time_elt = KmlChild(times_elt, i);
    if( !KheTimeMakeFromKml(time_elt, ins, ke) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool AddResources(KML_ELT resources_elt, KHE_INSTANCE ins, KML_ERROR *ke)*/
/*                                                                           */
/*  Add resource types, resources groups, and resources to ins based on      */
/*  resources_elt.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool AddResources(KML_ELT resources_elt, KHE_INSTANCE ins, KML_ERROR *ke)
{
  KML_ELT resource_types_elt, resource_type_elt;
  KML_ELT resource_groups_elt, resource_group_elt;
  int i, first_resource_index;

  /* check */
  if( !KmlCheck(resources_elt, ": ResourceTypes +ResourceGroups *Resource",ke) )
    return false;

  /* resource types */
  resource_types_elt = KmlChild(resources_elt, 0);
  if( !KmlCheck(resource_types_elt, ": *ResourceType", ke) )
    return false;
  for( i = 0;  i < KmlChildCount(resource_types_elt);  i++ )
  {
    resource_type_elt = KmlChild(resource_types_elt, i);
    if( !KheResourceTypeMakeFromKml(resource_type_elt, ins, ke) )
      return false;
  }

  /* resource groups */
  first_resource_index = 1;
  if( KmlContainsChild(resources_elt, "ResourceGroups", &resource_groups_elt) )
  {
    if( !KmlCheck(resource_groups_elt, ": *ResourceGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(resource_groups_elt);  i++ )
    {
      resource_group_elt = KmlChild(resource_groups_elt, i);
      if( !KheResourceGroupMakeFromKml(resource_group_elt, ins, ke) )
	return false;
    }
    first_resource_index = 2;
  }

  /* resources */
  for( i = first_resource_index;  i < KmlChildCount(resources_elt);  i++ )
    if( !KheResourceMakeFromKml(KmlChild(resources_elt, i), ins, ke) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool AddEventGroups(KML_ELT event_groups_elt, KHE_INSTANCE ins,          */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Add event groups from event_groups_elt to ins.                           */
/*                                                                           */
/*****************************************************************************/

static bool AddEventGroups(KML_ELT event_groups_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  KML_ELT event_group_elt;  int i;
  if( !KmlCheck(event_groups_elt, ": *Course *EventGroup", ke) )
    return false;
  for( i = 0;  i < KmlChildCount(event_groups_elt);  i++ )
  {
    event_group_elt = KmlChild(event_groups_elt, i);
    if( !KheEventGroupMakeFromKml(event_group_elt, ins, ke) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool AddEvents(KML_ELT events_elt, KHE_INSTANCE ins, KML_ERROR *ke)      */
/*                                                                           */
/*  Add events from events_elt to ins.                                       */
/*                                                                           */
/*****************************************************************************/

static bool AddEvents(KML_ELT events_elt, KHE_INSTANCE ins, KML_ERROR *ke)
{
  int i, first_event_index;  KML_ELT event_groups_elt;
  if( !KmlCheck(events_elt, ": +EventGroups *Event", ke) )
    return false;
  first_event_index = 0;
  if( KmlContainsChild(events_elt, "EventGroups", &event_groups_elt) )
  {
    if( !AddEventGroups(event_groups_elt, ins, ke) )
      return false;
    first_event_index = 1;
  }
  for( i = first_event_index;  i < KmlChildCount(events_elt);  i++ )
    if( !KheEventMakeFromKml(KmlChild(events_elt, i), ins, ke) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool AddConstraints(KML_ELT constraints_elt, KHE_INSTANCE ins,           */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Add constraints from constraints_elt to ins.                             */
/*                                                                           */
/*****************************************************************************/

static bool AddConstraints(KML_ELT constraints_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  int i;
  if( !KmlCheck(constraints_elt, ":"
      " *AssignResourceConstraint"
      " *AssignTimeConstraint"
      " *SplitEventsConstraint"
      " *DistributeSplitEventsConstraint"
      " *PreferResourcesConstraint"
      " *PreferTimesConstraint"
      " *AvoidSplitAssignmentsConstraint"
      " *SpreadEventsConstraint"
      " *LinkEventsConstraint"
      " *OrderEventsConstraint"
      " *AvoidClashesConstraint"
      " *AvoidUnavailableTimesConstraint"
      " *LimitIdleTimesConstraint"
      " *ClusterBusyTimesConstraint"
      " *LimitBusyTimesConstraint"
      " *LimitWorkloadConstraint",
      ke) )
  {
    MAssert(*ke != NULL, "AddConstraints internal error 1");
    return false;
  }
  for( i = 0;  i < KmlChildCount(constraints_elt);  i++ )
    if( !KheConstraintMakeFromKml(KmlChild(constraints_elt, i), ins, ke) )
    {
      MAssert(*ke != NULL, "AddConstraints internal error 2");
      return false;
    }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceMakeFromKml(KML_ELT instance_elt, KHE_ARCHIVE archive,   */
/*    bool infer_resource_partitions, KML_ERROR *ke)                         */
/*                                                                           */
/*  Make an instance and add it to archive.  Pass infer_resource_partitions  */
/*  to KheInstanceMakeEnd.                                                   */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceMakeFromKml(KML_ELT instance_elt, /* KHE_ARCHIVE archive, */
  bool infer_resource_partitions, KHE_INSTANCE *ins, KML_ERROR *ke)
{
  char *id;

  /* check instance_elt and make *ins */
  if( !KmlCheck(instance_elt,
      "Id : MetaData Times Resources Events Constraints", ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 1");
    return false;
  }
  id = KmlExtractAttributeValue(instance_elt, 0);
  *ins = KheInstanceMakeBegin(id, NULL);

  /* add metadata, times, resources, events, and constraints */
  if( !KheInstanceMetaDataMakeFromKml(KmlChild(instance_elt, 0), *ins, ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 2");
    return false;
  }
  if( !AddTimes(KmlChild(instance_elt, 1), *ins, ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 3");
    return false;
  }
  if( !AddResources(KmlChild(instance_elt, 2), *ins, ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 4");
    return false;
  }
  if( !AddEvents(KmlChild(instance_elt, 3), *ins, ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 5");
    return false;
  }
  if( !AddConstraints(KmlChild(instance_elt, 4), *ins, ke) )
  {
    MAssert(*ke != NULL, "KheInstanceMakeFromKml internal error 6");
    return false;
  }
  KheInstanceMakeEnd(*ins, infer_resource_partitions);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceSortConstraints(KHE_INSTANCE ins)                        */
/*                                                                           */
/*  Sort the constraints of ins into increasing tag order.                   */
/*                                                                           */
/*  Implementation note.  We need a stable sort here, so we can't use        */
/*  qsort.  Instead, we use an insertion sort that will run in linear        */
/*  time when the constraints are already sorted.                            */
/*                                                                           */
/*****************************************************************************/

static void KheInstanceSortConstraints(KHE_INSTANCE ins)
{
  KHE_CONSTRAINT ci, cj;  int i, j;
  MArrayForEach(ins->constraint_array, &ci, &i)
  {
    /* insert ci into its place among its predecessors */
    for( j = i - 1;  j >= 0;  j-- )
    {
      cj = MArrayGet(ins->constraint_array, j);
      if( KheConstraintTag(cj) <= KheConstraintTag(ci) )
	break;
      MArrayPut(ins->constraint_array, j + 1, cj);
    }
    MArrayPut(ins->constraint_array, j + 1, ci);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheInstanceWrite(KHE_INSTANCE ins, KML_FILE kf)                     */
/*                                                                           */
/*  Write ins onto kf.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheInstanceWrite(KHE_INSTANCE ins, KML_FILE kf)
{
  KHE_TIME_GROUP tg;  KHE_TIME t;  int i, rg_count;
  KHE_RESOURCE_TYPE rt;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_CONSTRAINT c;

  /* header and metadata */
  KmlBegin(kf, "Instance");
  MAssert(ins->id != NULL,
    "KheArchiveWrite:  Id missing in Instance");
  KmlAttribute(kf, "Id", ins->id);
  MAssert(ins->meta_data != NULL,
    "KheArchiveWrite:  MetaData missing in Instance");
  KheInstanceMetaDataWrite(ins->meta_data, kf);

  /* times */
  KmlBegin(kf, "Times");
  if( MArraySize(ins->time_group_array) > 0 )
  {
    KmlBegin(kf, "TimeGroups");
    /* patch suggested by Valentin Dreismann 9 Oct 2015 ***
    MArrayForEach(ins->time_group_array, &tg, &i)
      if( !KheTimeGroupWrite(tg, kf) )
	return false;
    *** */
    MArrayForEach(ins->time_group_array, &tg, &i)
      if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_WEEK )
	KheTimeGroupWrite(tg, kf);
    MArrayForEach(ins->time_group_array, &tg, &i)
      if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_DAY )
	KheTimeGroupWrite(tg, kf);
    MArrayForEach(ins->time_group_array, &tg, &i)
      if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_ORDINARY )
	KheTimeGroupWrite(tg, kf);
    KmlEnd(kf, "TimeGroups");
  }
  MArrayForEach(ins->time_array, &t, &i)
    KheTimeWrite(t, kf);
  KmlEnd(kf, "Times");

  /* resource types */
  KmlBegin(kf, "Resources");
  KmlBegin(kf, "ResourceTypes");
  rg_count = 0;
  MArrayForEach(ins->resource_type_array, &rt, &i)
  {
    rg_count += KheResourceTypeResourceGroupCount(rt);
    KheResourceTypeWrite(rt, kf);
  }
  KmlEnd(kf, "ResourceTypes");

  /* resource groups */
  if( rg_count > 0 )
  {
    KmlBegin(kf, "ResourceGroups");
    MArrayForEach(ins->resource_type_array, &rt, &i)
      KheResourceTypeWriteResourceGroups(rt, kf);
    KmlEnd(kf, "ResourceGroups");
  }

  /* resources */
  MArrayForEach(ins->resource_type_array, &rt, &i)
    KheResourceTypeWriteResources(rt, kf);
  KmlEnd(kf, "Resources");

  /* event groups */
  KmlBegin(kf, "Events");
  if( MArraySize(ins->event_group_array) > 0 )
  {
    KmlBegin(kf, "EventGroups");
    /* patch corresponding to Valentin Dreismann's above, 9 Oct 2015 ***
    MArrayForEach(ins->event_group_array, &eg, &i)
      if( !KheEventGroupWrite(eg, kf) )
	return false;
    *** */
    MArrayForEach(ins->event_group_array, &eg, &i)
      if( KheEventGroupKind(eg) == KHE_EVENT_GROUP_KIND_COURSE )
	KheEventGroupWrite(eg, kf);
    MArrayForEach(ins->event_group_array, &eg, &i)
      if( KheEventGroupKind(eg) == KHE_EVENT_GROUP_KIND_ORDINARY )
	KheEventGroupWrite(eg, kf);
    KmlEnd(kf, "EventGroups");
    MArrayForEach(ins->event_array, &e, &i)
      KheEventWrite(e, kf);
  }
  KmlEnd(kf, "Events");

  /* constraints (sort them by tag first, using a stable sort) */
  KheInstanceSortConstraints(ins);
  KmlBegin(kf, "Constraints");
  MArrayForEach(ins->constraint_array, &c, &i)
    KheConstraintWrite(c, kf);
  KmlEnd(kf, "Constraints");

  /* footer */
  KmlEnd(kf, "Instance");
}
