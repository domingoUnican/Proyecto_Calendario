
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
/*  FILE:         khe_archive.c                                              */
/*  DESCRIPTION:  An archive holding instances and solution groups           */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#include <stdarg.h>
#define MAX_ERROR_STRING 200
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 1


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE - one archive of problems and solution groups                */
/*                                                                           */
/*****************************************************************************/

struct khe_archive_rec {
  void				*back;			/* back pointer      */
  char				*id;			/* optional Id       */
  KHE_ARCHIVE_METADATA		meta_data;		/* optional MetaData */
  ARRAY_KHE_INSTANCE		instance_array;		/* instance array    */
  TABLE_KHE_INSTANCE		instance_table;		/* instance table    */
  ARRAY_KHE_SOLN_GROUP		soln_group_array;	/* solution groups   */
  TABLE_KHE_SOLN_GROUP		soln_group_table;	/* solution groups   */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE KheArchiveMake(void)                                         */
/*                                                                           */
/*  Make an initially empty archive.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE KheArchiveMake(char *id, KHE_ARCHIVE_METADATA md)
{
  KHE_ARCHIVE res;
  MMake(res);
  res->back = NULL;
  res->id = id;
  res->meta_data = md;
  MArrayInit(res->instance_array);
  MTableInit(res->instance_table);
  MArrayInit(res->soln_group_array);
  MTableInit(res->soln_group_table);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheArchiveId(KHE_ARCHIVE archive)                                  */
/*                                                                           */
/*  Return the Id of archive, possibly NULL.                                 */
/*                                                                           */
/*****************************************************************************/

char *KheArchiveId(KHE_ARCHIVE archive)
{
  return archive->id;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveSetBack(KHE_ARCHIVE archive, void *back)                  */
/*                                                                           */
/*  Set the back pointer of archive.                                         */
/*                                                                           */
/*****************************************************************************/

void KheArchiveSetBack(KHE_ARCHIVE archive, void *back)
{
  archive->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheArchiveBack(KHE_ARCHIVE archive)                                */
/*                                                                           */
/*  Retrieve the back pointer of archive.                                    */
/*                                                                           */
/*****************************************************************************/

void *KheArchiveBack(KHE_ARCHIVE archive)
{
  return archive->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive)             */
/*                                                                           */
/*  Return the MetaData attribute of archive.                                */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive)
{
  return archive->meta_data;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md) */
/*                                                                           */
/*  Set the metadata attribute of archive to md.                             */
/*                                                                           */
/*****************************************************************************/

void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md)
{
  archive->meta_data = md;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "instances"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)        */
/*                                                                           */
/*  Add ins to archive, assuming it is safe to do so (not duplicate Id).     */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)
{
  KHE_INSTANCE x;
  if( KheInstanceId(ins) != NULL &&
      !MTableInsertUnique(archive->instance_table, KheInstanceId(ins), ins,&x) )
    return false;
  KheInstanceAddArchive(ins, archive);
  MArrayAddLast(archive->instance_array, ins);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveDeleteInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)     */
/*                                                                           */
/*  Delete ins from archive.  Also delete any solutions for ins.             */
/*                                                                           */
/*****************************************************************************/

void KheArchiveDeleteInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)
{
  int pos, i;  KHE_SOLN_GROUP sg;

  /* delete the link between archive and ins */
  MAssert(MArrayContains(archive->instance_array, ins, &pos),
    "KheArchiveDeleteInstance: ins not in archive");
  MArrayRemove(archive->instance_array, pos);
  KheInstanceDeleteArchive(ins, archive);
  if( KheInstanceId(ins) != NULL )
  {
    if( !MTableContains(archive->instance_table, KheInstanceId(ins), &pos) )
      MAssert(false, "KheArchiveDeleteInstance: internal error");
    MTableDelete(archive->instance_table, pos);
  }

  /* delete solutions for ins in archive */
  MArrayForEach(archive->soln_group_array, &sg, &i)
    SolnGroupDeleteSolnsForInstance(sg, ins);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheArchiveInstanceCount(KHE_ARCHIVE archive)                         */
/*                                                                           */
/*  Return the number of instances in archive.                               */
/*                                                                           */
/*****************************************************************************/

int KheArchiveInstanceCount(KHE_ARCHIVE archive)
{
  return MArraySize(archive->instance_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i)              */
/*                                                                           */
/*  Return the i'th instance of archive.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i)
{
  return MArrayGet(archive->instance_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,           */
/*    KHE_INSTANCE *ins)                                                     */
/*                                                                           */
/*  Retrieve an instance with the given id from archive.                     */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,
  KHE_INSTANCE *ins)
{
  int pos;
  return MTableRetrieve(archive->instance_table, id, ins, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Sumbodule "solution groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveAddSolnGroup(KHE_ARCHIVE archive,                         */
/*    KHE_SOLN_GROUP soln_group)                                             */
/*                                                                           */
/*  Add soln_group to archive, assuming it is safe (not duplicate Id).       */
/*                                                                           */
/*****************************************************************************/

void KheArchiveAddSolnGroup(KHE_ARCHIVE archive, KHE_SOLN_GROUP soln_group)
{
  MArrayAddLast(archive->soln_group_array, soln_group);
  if( KheSolnGroupId(soln_group) != NULL )
    MTableInsert(archive->soln_group_table, KheSolnGroupId(soln_group),
      soln_group);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheArchiveSolnGroupCount(KHE_ARCHIVE archive)                        */
/*                                                                           */
/*  Return the number of solution groups in archive.                         */
/*                                                                           */
/*****************************************************************************/

int KheArchiveSolnGroupCount(KHE_ARCHIVE archive)
{
  return MArraySize(archive->soln_group_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i)           */
/*                                                                           */
/*  Return the i'th solution group of archive.                               */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i)
{
  return MArrayGet(archive->soln_group_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,          */
/*    KHE_SOLN_GROUP *soln_group)                                            */
/*                                                                           */
/*  Retrieve a solution group with the given id from archive.                */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP *soln_group)
{
  int pos;
  return MTableRetrieve(archive->soln_group_table, id, soln_group, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing archives"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveEltToArchive(KML_ELT archive_elt, KHE_ARCHIVE *archive,   */
/*    bool infer_resource_partitions, bool allow_invalid_solns,KML_ERROR *ke)*/
/*                                                                           */
/*  Convert archive_elt into *archive.                                       */
/*                                                                           */
/*****************************************************************************/

static bool KheArchiveEltToArchive(KML_ELT archive_elt, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, bool allow_invalid_solns, KML_ERROR *ke)
{
  char *id;  KML_ELT metadata_elt, instances_elt, instance_elt;
  KML_ELT soln_groups_elt, soln_group_elt;  KHE_ARCHIVE res;  int i;
  KHE_INSTANCE ins;

  /* fail if archive_elt has problems */
  *archive = NULL;
  if( strcmp(KmlLabel(archive_elt), "HighSchoolTimetableArchive") != 0 )
    return KmlError(ke, KmlLineNum(archive_elt), KmlColNum(archive_elt),
      "file does not begin with <HighSchoolTimetableArchive>");
  if( !KmlCheck(archive_elt, "+Id : +MetaData +Instances +SolutionGroups", ke) )
    return false;

  /* create archive with optional id and optional metadata */
  id = KmlAttributeCount(archive_elt) == 0 ? NULL :
    KmlExtractAttributeValue(archive_elt, 0);
  res = KheArchiveMake(id, NULL);
  if( KmlContainsChild(archive_elt, "MetaData", &metadata_elt) &&
      !KheArchiveMetaDataMakeFromKml(metadata_elt, res, ke) )
  {
    MAssert(*ke != NULL, "KheArchiveEltToArchive internal error 1");
    return false;
  }

  /* build and add instances */
  if( KmlContainsChild(archive_elt, "Instances", &instances_elt) )
  {
    if( !KmlCheck(instances_elt, ": *Instance", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(instances_elt);  i++ )
    {
      instance_elt = KmlChild(instances_elt, i);
      if( !KheInstanceMakeFromKml(instance_elt, infer_resource_partitions,
	  &ins, ke) )
      {
	MAssert(*ke != NULL, "KheArchiveEltToArchive internal error 2");
	return false;
      }
      if( !KheArchiveAddInstance(res, ins) )
	return KmlError(ke, KmlLineNum(instance_elt), KmlColNum(instance_elt),
	  "instance Id \"%s\" used previously", KheInstanceId(ins));
    }
  }

  /* build and add solution groups */
  if( KmlContainsChild(archive_elt, "SolutionGroups", &soln_groups_elt) )
  {
    if( !KmlCheck(soln_groups_elt, ": *SolutionGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(soln_groups_elt);  i++ )
    {
      soln_group_elt = KmlChild(soln_groups_elt, i);
      if( !KheSolnGroupMakeFromKml(soln_group_elt, res,allow_invalid_solns,ke) )
      {
	MAssert(*ke != NULL, "KheArchiveEltToArchive internal error 3");
	return false;
      }
    }
  }

  /* KmlFree(archive_elt, true, true); */
  *archive = res;
  *ke = NULL;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive, KML_ERROR *ke,       */
/*    bool infer_resource_partitions, char **leftover, int *leftover_len,    */
/*    FILE *echo_fp)                                                         */
/*                                                                           */
/*  Read *archive from fp.                                                   */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive, KML_ERROR *ke,
  bool infer_resource_partitions, bool allow_invalid_solns,
  char **leftover, int *leftover_len, FILE *echo_fp)
{
  return KheArchiveReadIncremental(fp, archive, ke, infer_resource_partitions,
    allow_invalid_solns, leftover, leftover_len, echo_fp, NULL, NULL, NULL,
    NULL, NULL, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,           */
/*    KML_ERROR *ke, bool infer_resource_partitions,bool allow_invalid_solns)*/
/*                                                                           */
/*  Like KheArchiveRead except that the archive is read from str.            */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,
  KML_ERROR *ke, bool infer_resource_partitions, bool allow_invalid_solns)
{
  KML_ELT archive_elt;
  return KmlReadString(str, &archive_elt, ke) &&
    KheArchiveEltToArchive(archive_elt, archive, infer_resource_partitions,
      allow_invalid_solns, ke);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCheckSolnGroupContext(KML_ELT sg, KML_READ_INFO ri)              */
/*                                                                           */
/*  Check that sg, whose label is <SolutionGroup>, has occurred in the       */
/*  correct context.                                                         */
/*                                                                           */
/*****************************************************************************/

static void KheCheckSolnGroupContext(KML_ELT sg, KML_READ_INFO ri)
{
  KML_ELT sgs, hsta;
  sgs = KmlParent(sg);
  if( sgs == NULL || strcmp(KmlLabel(sgs), "SolutionGroups") != 0 )
    KmlReadFail(ri, KmlErrorMake(KmlLineNum(sg), KmlColNum(sg),
      "<SolutionGroup> does not lie within <SolutionGroups>"));
  hsta = KmlParent(sgs);
  if( hsta == NULL || strcmp(KmlLabel(hsta), "HighSchoolTimetableArchive") != 0)
    KmlReadFail(ri, KmlErrorMake(KmlLineNum(sgs), KmlColNum(sgs),
      "<SolutionGroups> does not lie within <HighSchoolTimetableArchive>"));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCheckSolnContext(KML_ELT s, KML_READ_INFO ri)                    */
/*                                                                           */
/*  Check that s, whose label is <Solution>, has occurred in the correct     */
/*  context.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheCheckSolnContext(KML_ELT s, KML_READ_INFO ri)
{
  KML_ELT sg;
  sg = KmlParent(s);
  if( sg == NULL || strcmp(KmlLabel(sg), "SolutionGroup") != 0 )
    KmlReadFail(ri, KmlErrorMake(KmlLineNum(s), KmlColNum(s),
      "<Solution> does not lie within <SolutionGroup>"));
  KheCheckSolnGroupContext(sg, ri);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCheckInstanceContext(KML_ELT ins, KML_READ_INFO ri)              */
/*                                                                           */
/*  Check that ins, whose label is <Instance>, has occurred in the correct   */
/*  context.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheCheckInstanceContext(KML_ELT ins, KML_READ_INFO ri)
{
  KML_ELT inss, hsta;
  inss = KmlParent(ins);
  if( inss == NULL || strcmp(KmlLabel(inss), "Instances") != 0 )
    KmlReadFail(ri, KmlErrorMake(KmlLineNum(ins), KmlColNum(ins),
      "<Instance> does not lie within <Instances>"));
  hsta = KmlParent(inss);
  if( hsta == NULL || strcmp(KmlLabel(hsta), "HighSchoolTimetableArchive") != 0)
    KmlReadFail(ri, KmlErrorMake(KmlLineNum(inss), KmlColNum(inss),
      "<Instances> does not lie within <HighSchoolTimetableArchive>"));
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_READ_STATE - info to pass around during incremental file reads       */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_read_state_rec {
  KHE_ARCHIVE		archive;
  KHE_SOLN_GROUP	soln_group;
  bool			infer_resource_partitions;
  bool			allow_invalid_solns;
  KHE_ARCHIVE_FN	archive_begin_fn;
  KHE_ARCHIVE_FN	archive_end_fn;
  KHE_SOLN_GROUP_FN	soln_group_begin_fn;
  KHE_SOLN_GROUP_FN	soln_group_end_fn;
  KHE_SOLN_FN		soln_fn;
  void			*impl;
} *KHE_READ_STATE;


/*****************************************************************************/
/*                                                                           */
/*  void KheReadBeginArchive(KHE_READ_STATE krs, KHE_ARCHIVE archive)        */
/*                                                                           */
/*  Function to call when beginning an archive.                              */
/*                                                                           */
/*****************************************************************************/

static void KheReadBeginArchive(KHE_READ_STATE krs, KHE_ARCHIVE archive)
{
  MAssert(krs->archive == NULL, "KheReadBeginArchive internal error 1");
  krs->archive = archive;
  if( krs->archive_begin_fn != NULL )
    krs->archive_begin_fn(krs->archive, krs->impl);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheReadEndArchive(KHE_READ_STATE krs)                               */
/*                                                                           */
/*  Function to call when ending an archive.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheReadEndArchive(KHE_READ_STATE krs)
{
  MAssert(krs->archive != NULL, "KheReadEndArchive internal error 1");
  if( krs->archive_end_fn != NULL )
    krs->archive_end_fn(krs->archive, krs->impl);
  krs->archive = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheReadInArchive(KHE_READ_STATE krs)                                */
/*                                                                           */
/*  Return true if krs is currently within an archive.                       */
/*                                                                           */
/*****************************************************************************/

static bool KheReadInArchive(KHE_READ_STATE krs)
{
  return krs->archive != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheReadBeginSolnGroup(KHE_READ_STATE krs, KHE_SOLN_GROUP soln_group)*/
/*                                                                           */
/*  Function to call when beginning a soln group.                            */
/*                                                                           */
/*****************************************************************************/

static void KheReadBeginSolnGroup(KHE_READ_STATE krs, KHE_SOLN_GROUP soln_group)
{
  MAssert(krs->archive != NULL, "KheReadBeginSolnGroup internal error 1");
  MAssert(krs->soln_group == NULL, "KheReadBeginSolnGroup internal error 2");
  krs->soln_group = soln_group;
  if( krs->soln_group_begin_fn != NULL )
    krs->soln_group_begin_fn(krs->soln_group, krs->impl);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheReadEndSolnGroup(KHE_READ_STATE krs, KHE_SOLN_GROUP soln_group)  */
/*                                                                           */
/*  Function to call when ending a soln group.                               */
/*                                                                           */
/*****************************************************************************/

static void KheReadEndSolnGroup(KHE_READ_STATE krs, KHE_SOLN_GROUP soln_group)
{
  MAssert(krs->archive != NULL, "KheReadEndSolnGroup internal error 1");
  MAssert(krs->soln_group != NULL, "KheReadEndSolnGroup internal error 2");
  if( krs->soln_group_end_fn != NULL )
    krs->soln_group_end_fn(krs->soln_group, krs->impl);
  krs->soln_group = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheReadInSolnGroup(KHE_READ_STATE krs)                              */
/*                                                                           */
/*  Return true if krs is currently within a solution group.                 */
/*                                                                           */
/*****************************************************************************/

static bool KheReadInSolnGroup(KHE_READ_STATE krs)
{
  return krs->soln_group != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheReadEltFn(KML_ELT elt, KML_READ_INFO ri)                         */
/*                                                                           */
/*  Element function called during incremental reading.                      */
/*                                                                           */
/*****************************************************************************/

static void KheReadEltFn(KML_ELT elt, KML_READ_INFO ri)
{
  KML_ERROR ke;  int count;  KHE_SOLN soln;  KHE_INSTANCE ins;
  KHE_ARCHIVE archive;  KHE_SOLN_GROUP soln_group;
  KHE_READ_STATE krs = (KHE_READ_STATE) KmlReadImpl(ri);
  if( DEBUG2 )
    fprintf(stderr, "[ KheReadEltFn(%s, %d)\n",
      KmlLabel(elt), KmlReadCurrDepth(ri));

  /* after completing the read of an instance */
  ke = NULL;
  if( strcmp(KmlLabel(elt), "Instance") == 0 )
  {
    KheCheckInstanceContext(elt, ri);
    if( !KheReadInArchive(krs) )
    {
      /* this must be the very first instance */
      if( !KheArchiveEltToArchive(KmlParent(KmlParent(elt)), &archive,
	    krs->infer_resource_partitions, krs->allow_invalid_solns, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 1");
	KmlReadFail(ri, ke);
      }
      KheReadBeginArchive(krs, archive);
      count = KheArchiveSolnGroupCount(krs->archive);
      MAssert(count == 0, "KheReadEltFn internal error 2");
      count = KheArchiveInstanceCount(krs->archive);
      MAssert(count == 1, "KheReadEltFn internal error 3");
      if( DEBUG2 )
      {
	ins = KheArchiveInstance(krs->archive, 0);
	fprintf(stderr, "  adding initial instance %s to archive\n",
	  KheInstanceId(ins));
      }
    }
    else
    {
      /* this must be at least the second instance in this archive */
      if( !KheInstanceMakeFromKml(elt, krs->infer_resource_partitions,
	  &ins, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 4");
        KmlReadFail(ri, ke);
      }
      if( !KheArchiveAddInstance(krs->archive, ins) )
      {
	KmlError(&ke, KmlLineNum(elt), KmlColNum(elt),
	  "instance Id \"%s\" used previously", KheInstanceId(ins));
	KmlReadFail(ri, ke);
      }
      count = KheArchiveInstanceCount(krs->archive);
      MAssert(count >= 2, "KheReadEltFn internal error 5");
      if( DEBUG2 )
      {
	ins = KheArchiveInstance(krs->archive, count - 1);
	fprintf(stderr, "  adding instance %s to archive\n",
	  KheInstanceId(ins));
      }
    }
    KmlDeleteChild(KmlParent(elt), elt);
    KmlFree(elt, true, true);
  }

  /* after completing the read of a solution */
  else if( strcmp(KmlLabel(elt), "Solution") == 0 )
  {
    KheCheckSolnContext(elt, ri);
    if( !KheReadInArchive(krs) )
    {
      /* this must be the very first solution */
      if( !KheArchiveEltToArchive(KmlParent(KmlParent(KmlParent(elt))),
	    &archive, krs->infer_resource_partitions,
	    krs->allow_invalid_solns, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 6");
	KmlReadFail(ri, ke);
      }
      KheReadBeginArchive(krs, archive);
      count = KheArchiveSolnGroupCount(krs->archive);
      MAssert(count == 1, "KheReadEltFn internal error 7");
      soln_group = KheArchiveSolnGroup(krs->archive, 0);
      MAssert(KheSolnGroupSolnCount(soln_group) == 1,
	"KheReadEltFn internal error 2");
      KheReadBeginSolnGroup(krs, soln_group);
      if( DEBUG2 )
      {
	soln = KheSolnGroupSoln(krs->soln_group, 0);
	fprintf(stderr, "  adding initial solution (to %s) to soln group %s\n",
	  KheInstanceId(KheSolnInstance(soln)),KheSolnGroupId(krs->soln_group));
      }
    }
    else if( !KheReadInSolnGroup(krs) )
    {
      /* this must be the first solution in this solution group */
      if( !KheSolnGroupMakeFromKml(KmlParent(elt), krs->archive,
	    krs->allow_invalid_solns, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 8");
	KmlReadFail(ri, ke);
      }
      count = KheArchiveSolnGroupCount(krs->archive);
      MAssert(count >= 1, "KheReadEltFn internal error 9");
      soln_group = KheArchiveSolnGroup(krs->archive, count - 1);
      MAssert(KheSolnGroupSolnCount(soln_group) == 1,
	"KheReadEltFn internal error 10");
      KheReadBeginSolnGroup(krs, soln_group);
      if( DEBUG2 )
      {
	soln = KheSolnGroupSoln(krs->soln_group, 0);
	fprintf(stderr, "  adding first solution (to %s) to soln group %s\n",
	  KheInstanceId(KheSolnInstance(soln)),KheSolnGroupId(krs->soln_group));
      }
    }
    else
    {
      /* this must be at least the second solution in this solution group */
      if( !KheSolnMakeFromKml(elt, krs->archive, krs->allow_invalid_solns,
	    &soln, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 11");
        KmlReadFail(ri, ke);
      }
      KheSolnGroupAddSoln(krs->soln_group, soln);
      if( DEBUG2 )
	fprintf(stderr, "  adding solution to soln group %s\n",
          KheSolnGroupId(krs->soln_group));
    }
    KmlDeleteChild(KmlParent(elt), elt);
    KmlFree(elt, true, true);
    if( krs->soln_fn != NULL )
    {
      count = KheSolnGroupSolnCount(krs->soln_group);
      krs->soln_fn(KheSolnGroupSoln(krs->soln_group, count - 1), krs->impl);
      if( DEBUG2 )
	fprintf(stderr, "  calling soln_fn\n");
    }
  }

  /* after completing the read of a solution group */
  else if( strcmp(KmlLabel(elt), "SolutionGroup") == 0 )
  {
    KheCheckSolnGroupContext(elt, ri);
    if( !KheReadInArchive(krs) )
    {
      /* this must be the very first solution group, and it must contain */
      /* no solutions, which is unlikely but legal */
      if( !KheArchiveEltToArchive(KmlParent(KmlParent(elt)), &archive,
	    krs->infer_resource_partitions, krs->allow_invalid_solns, &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 12");
	KmlReadFail(ri, ke);
      }
      KheReadBeginArchive(krs, archive);
      count = KheArchiveSolnGroupCount(krs->archive);
      MAssert(count == 1, "KheReadEltFn internal error 13");
      soln_group = KheArchiveSolnGroup(krs->archive, 0);
      KheReadBeginSolnGroup(krs, soln_group);
      KheReadEndSolnGroup(krs, soln_group);
      if( DEBUG2 )
	fprintf(stderr, "  adding empty first solution group to archive\n");
    }
    else if( !KheReadInSolnGroup(krs) )
    {
      /* this must be a solution group after the first, and it must */
      /* contain no solutions */
      if( !KheSolnGroupMakeFromKml(elt, krs->archive, krs->allow_invalid_solns,
	    &ke) )
      {
	MAssert(ke != NULL, "KheReadEltFn internal error 14");
	KmlReadFail(ri, ke);
      }
      count = KheArchiveSolnGroupCount(krs->archive);
      soln_group = KheArchiveSolnGroup(krs->archive, count - 1);
      KheReadBeginSolnGroup(krs, soln_group);
      KheReadEndSolnGroup(krs, soln_group);
      if( DEBUG2 )
	fprintf(stderr, "  adding empty solution group to archive\n");
    }
    else
    {
      /* must have seen at least one solution, which will have done */
      /* all the real work.  So just end this solution group now    */
      if( DEBUG2 )
	fprintf(stderr, "  ending soln group %s\n",
          KheSolnGroupId(krs->soln_group));
      KheReadEndSolnGroup(krs, krs->soln_group);
    }
  }
  if( DEBUG2 )
    fprintf(stderr, "] KheReadEltFn returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveReadIncremental(FILE *fp, KHE_ARCHIVE *archive,           */
/*    KML_ERROR *ke,bool infer_resource_partitions,bool allow_invalid_solns, */
/*    char **leftover, int *leftover_len, FILE *echo_fp,                     */
/*    KHE_ARCHIVE_FN archive_begin_fn, KHE_ARCHIVE_FN archive_end_fn,        */
/*    KHE_SOLN_GROUP_FN soln_group_begin_fn,                                 */
/*    KHE_SOLN_GROUP_FN soln_group_end_fn, KHE_SOLN_FN soln_fn, void *impl)  */
/*                                                                           */
/*  Like KheArchiveRead but also calls the callback functions as needed.     */
/*                                                                           */
/*  Implementation note.  A callback to KheReadEltFn above occurs as each    */
/*  category of depth at most 3 is completed.  Here is the grammar:          */
/*                                                                           */
/*    HighSchoolTimetableArchive +Id                                         */
/*       +MetaData                                                           */
/*       +Instances                                                          */
/*          *Instance                                                        */
/*       +SolutionGroups                                                     */
/*          *SolutionGroup Id                                                */
/*             MetaData                                                      */
/*             *Solution                                                     */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveReadIncremental(FILE *fp, KHE_ARCHIVE *archive,
  KML_ERROR *ke, bool infer_resource_partitions, bool allow_invalid_solns,
  char **leftover, int *leftover_len, FILE *echo_fp,
  KHE_ARCHIVE_FN archive_begin_fn, KHE_ARCHIVE_FN archive_end_fn,
  KHE_SOLN_GROUP_FN soln_group_begin_fn,
  KHE_SOLN_GROUP_FN soln_group_end_fn, KHE_SOLN_FN soln_fn, void *impl)
{
  struct khe_read_state_rec krs_rec;  KML_ELT archive_elt;  bool res;

  /* initialize archive and incremental read data */
  krs_rec.archive = NULL;
  krs_rec.soln_group = NULL;
  krs_rec.infer_resource_partitions = infer_resource_partitions;
  krs_rec.allow_invalid_solns = allow_invalid_solns;
  krs_rec.archive_begin_fn = archive_begin_fn;
  krs_rec.archive_end_fn = archive_end_fn;
  krs_rec.soln_group_begin_fn = soln_group_begin_fn;
  krs_rec.soln_group_end_fn = soln_group_end_fn;
  krs_rec.soln_fn = soln_fn;
  krs_rec.impl = impl;

  /* read fp incrementally, calling back after each high-level thing */
  *ke = NULL;
  if( !KmlReadFileIncremental(fp, &archive_elt, ke,
	leftover == NULL ? NULL : "</HighSchoolTimetableArchive>",
	leftover, leftover_len, echo_fp, &KheReadEltFn, &krs_rec, 3) )
  {
    /* the incremental read did not work */
    *archive = NULL;
    MAssert(*ke != NULL, "KheArchiveReadIncremental internal error 1");
    if( DEBUG3 )
      fprintf(stderr, "KheArchiveReadIncremental ret false (a)\n");
    res = false;
  }
  else if( KheReadInSolnGroup(&krs_rec) )
  {
    /* the incremental read ended up inside a solution group */
    *archive = NULL;
    MAssert(*ke != NULL, "KheArchiveReadIncremental internal error 2");
    if( DEBUG3 )
      fprintf(stderr, "KheArchiveReadIncremental ret false (b)\n");
    res = false;
  }
  else if( KheReadInArchive(&krs_rec) )
  {
    /* the incremental read worked and found at least one soln or soln group */
    *archive = krs_rec.archive;
    KheReadEndArchive(&krs_rec);
    res = true;
    KmlFree(archive_elt, true, true);
    if( DEBUG3 )
      fprintf(stderr, "KheArchiveReadIncremental ret true (c)\n");
  }
  else
  {
    /* the incremental read worked but found no solns or soln groups */
    res = KheArchiveEltToArchive(archive_elt, archive,
      infer_resource_partitions, allow_invalid_solns, ke);
    KheReadBeginArchive(&krs_rec, *archive);
    KheReadEndArchive(&krs_rec);
    KmlFree(archive_elt, true, true);
    if( !res )
    {
      MAssert(*ke != NULL, "KheArchiveReadIncremental internal error 3");
      if( DEBUG3 )
	fprintf(stderr, "KheArchiveReadIncremental ret false (d)\n");
    }
    else if( DEBUG3 )
      fprintf(stderr, "KheArchiveReadIncremental ret true (e)\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveWriteHeaderAndInstances(KHE_ARCHIVE archive, KML_FILE kf) */
/*                                                                           */
/*  Write the header and instances of archive to kf.                         */
/*                                                                           */
/*****************************************************************************/

static void KheArchiveWriteHeaderAndInstances(KHE_ARCHIVE archive, KML_FILE kf)
{
  KHE_INSTANCE ins;  int i;

  /* header with optional Id, followed by optional metadata */
  KmlBegin(kf, "HighSchoolTimetableArchive");
  if( archive->id != NULL )
    KmlAttribute(kf, "Id", archive->id);
  if( archive->meta_data != NULL )
    KheArchiveMetaDataWrite(archive->meta_data, kf);

  /* instances */
  if( MArraySize(archive->instance_array) > 0 )
  {
    KmlBegin(kf, "Instances");
    MArrayForEach(archive->instance_array, &ins, &i)
      KheInstanceWrite(ins, kf);
    KmlEnd(kf, "Instances");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp)   */
/*                                                                           */
/*  Write archive to fp, which must be open for writing.  Include reports    */
/*  in the written archive if with_reports is true.                          */
/*                                                                           */
/*****************************************************************************/

void KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp)
{
  KML_FILE kf;  int i;  KHE_SOLN_GROUP soln_group;

  /* header and instances */
  kf = KmlMakeFile(fp, 0, 2);
  KheArchiveWriteHeaderAndInstances(archive, kf);

  /* soln groups */
  if( MArraySize(archive->soln_group_array) > 0 )
  {
    KmlBegin(kf, "SolutionGroups");
    MArrayForEach(archive->soln_group_array, &soln_group, &i)
      KheSolnGroupWrite(soln_group, with_reports, kf);
    KmlEnd(kf, "SolutionGroups");
  }

  /* close header and exit */
  KmlEnd(kf, "HighSchoolTimetableArchive");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveWriteSolnGroup(KHE_ARCHIVE archive,                       */
/*    KHE_SOLN_GROUP soln_group, bool with_reports, FILE *fp)                */
/*                                                                           */
/*  Write archive, but only include one of its solution groups:  soln_group. */
/*                                                                           */
/*****************************************************************************/

void KheArchiveWriteSolnGroup(KHE_ARCHIVE archive,
  KHE_SOLN_GROUP soln_group, bool with_reports, FILE *fp)
{
  KML_FILE kf;

  /* header and instances */
  MAssert(KheSolnGroupArchive(soln_group) == archive,
    "KheArchiveWriteSolnGroup:  soln_group not in archive");
  kf = KmlMakeFile(fp, 0, 2);
  KheArchiveWriteHeaderAndInstances(archive, kf);

  /* soln group */
  KmlBegin(kf, "SolutionGroups");
  KheSolnGroupWrite(soln_group, with_reports, kf);
  KmlEnd(kf, "SolutionGroups");

  /* close header and exit */
  KmlEnd(kf, "HighSchoolTimetableArchive");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveWriteWithoutSolnGroups(KHE_ARCHIVE archive, FILE *fp)     */
/*                                                                           */
/*  Write archive, omitting its solution groups.                             */
/*                                                                           */
/*****************************************************************************/

void KheArchiveWriteWithoutSolnGroups(KHE_ARCHIVE archive, FILE *fp)
{
  KML_FILE kf;

  /* header and instances */
  kf = KmlMakeFile(fp, 0, 2);
  KheArchiveWriteHeaderAndInstances(archive, kf);

  /* close header and exit */
  KmlEnd(kf, "HighSchoolTimetableArchive");
}
