
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
/*  FILE:         khe_cluster_busy_times_constraint.c                        */
/*  DESCRIPTION:  A cluster busy times constraint                            */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT - a cluster busy times constraint      */
/*                                                                           */
/*****************************************************************************/

struct khe_cluster_busy_times_constraint_rec {
  INHERIT_CONSTRAINT
  ARRAY_KHE_RESOURCE_GROUP	resource_groups;	/* applies to        */
  ARRAY_KHE_RESOURCE		resources;		/* applies to        */
  int				minimum;		/* minimum           */
  int				maximum;		/* maximum           */
  ARRAY_KHE_TIME_GROUP		time_groups;		/* time groups       */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,       */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    int minimum, int maximum, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT *c)        */
/*                                                                           */
/*  Make a new cluster busy times constraint with these attributes, add it   */
/*  to ins, and return it.                                                   */
/*                                                                           */
/*****************************************************************************/

bool KheClusterBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT *c)
{
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheClusterBusyTimesConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->resource_groups);
  MArrayInit(res->resources);
  res->minimum = minimum;
  res->maximum = maximum;
  MArrayInit(res->time_groups);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintMinimum(                                */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the minimum attribute of c.                                       */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintMinimum(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return c->minimum;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintMaximum(                                */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the maximum attribute of c.                                       */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintMaximum(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return c->maximum;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintAppliesToCount(                         */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintAppliesToCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  int i, res;  KHE_RESOURCE_GROUP rg;
  res = MArraySize(c->resources);
  MArrayForEach(c->resource_groups, &rg, &i)
    res += KheResourceGroupResourceCount(rg);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesConstraintFinalize(                              */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesConstraintFinalize(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  /* nothing to do in this case */
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintDensityCount(                           */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the density count of c; just the applies to count in this case.   */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintDensityCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return KheClusterBusyTimesConstraintAppliesToCount(c);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesConstraintAddResourceGroup(                      */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)            */
/*                                                                           */
/*  Add rg to c.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesConstraintAddResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)
{
  int i;  KHE_RESOURCE r;
  MArrayAddLast(c->resource_groups, rg);
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    KheResourceAddConstraint(r, (KHE_CONSTRAINT) c);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintResourceGroupCount(                     */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the number of resource groups in c.                               */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintResourceGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return MArraySize(c->resource_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheClusterBusyTimesConstraintResourceGroup(           */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)                            */
/*                                                                           */
/*  Return the i'th resource group of c.                                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheClusterBusyTimesConstraintResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->resource_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resources"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesConstraintAddResource(                           */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r)                   */
/*                                                                           */
/*  Add r to c.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesConstraintAddResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r)
{
  MArrayAddLast(c->resources, r);
  KheResourceAddConstraint(r, (KHE_CONSTRAINT) c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintResourceCount(                          */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the number of resources of c.                                     */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintResourceCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return MArraySize(c->resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheClusterBusyTimesConstraintResource(                      */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)                            */
/*                                                                           */
/*  Return the i'th resource of c.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheClusterBusyTimesConstraintResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time groups"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesConstraintAddTimeGroup(                          */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg)                */
/*                                                                           */
/*  Add tg to c.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesConstraintAddTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg)
{
  MArrayAddLast(c->time_groups, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesConstraintTimeGroupCount(                         */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                                   */
/*                                                                           */
/*  Return the number of time groups of c.                                   */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesConstraintTimeGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return MArraySize(c->time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheClusterBusyTimesConstraintTimeGroup(                   */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)                            */
/*                                                                           */
/*  Return the i'th time group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheClusterBusyTimesConstraintTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->time_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing "                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,          */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make a cluster busy times constraint based on cons_elt and add to ins.   */
/*                                                                           */
/*****************************************************************************/

bool KheClusterBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT res;  int minimum, maximum;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo TimeGroups #Minimum #Maximum", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* get minimum and maximum */
  sscanf(KmlText(KmlChild(cons_elt, 6)), "%d", &minimum);
  sscanf(KmlText(KmlChild(cons_elt, 7)), "%d", &maximum);

  /* build and insert the constraint object */
  if( !KheClusterBusyTimesConstraintMake(ins, id, name, reqd, wt, cf,
        minimum, maximum, &res) )
    return KmlError(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<ClusterBusyTimesConstraint> Id \"%s\" used previously", id);

  /* add the resource groups and resources */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": +ResourceGroups +Resources", ke) )
    return false;
  if( !KheConstraintAddResourceGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( !KheConstraintAddResourcesFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( KheClusterBusyTimesConstraintAppliesToCount(res) == 0 )
    return KmlError(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<ClusterBusyTimesConstraint> applies to 0 resources");

  /* add the time groups */
  if( !KheConstraintAddTimeGroupsFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  return true;
}

/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesConstraintWrite(                                 */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KML_FILE kf)                      */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesConstraintWrite(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c,
  KML_FILE kf)
{
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  int i;  KHE_TIME_GROUP tg;
  KmlBegin(kf, "ClusterBusyTimesConstraint");
  MAssert(c->id != NULL,
    "KheArchiveWrite: Id missing in ClusterBusyTimesConstraint");
  KmlAttribute(kf, "Id", c->id);
  KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf);
  KmlBegin(kf, "AppliesTo");
  if( MArraySize(c->resource_groups) > 0 )
  {
    KmlBegin(kf, "ResourceGroups");
    MArrayForEach(c->resource_groups, &rg, &i)
    {
      MAssert(KheResourceGroupId(rg) != NULL, "KheArchiveWrite:  Id missing in "
        "ResourceGroup referenced from ClusterBusyTimesConstraint %s", c->id);
      KmlEltAttribute(kf, "ResourceGroup", "Reference", KheResourceGroupId(rg));
    }
    KmlEnd(kf, "ResourceGroups");
  }
  if( MArraySize(c->resources) > 0 )
  {
    KmlBegin(kf, "Resources");
    MArrayForEach(c->resources, &r, &i)
    {
      MAssert(KheResourceId(r) != NULL, "KheArchiveWrite:  Id missing in "
        "Resource referenced from ClusterBusyTimesConstraint %s", c->id);
      KmlEltAttribute(kf, "Resource", "Reference", KheResourceId(r));
    }
    KmlEnd(kf, "Resources");
  }
  KmlEnd(kf, "AppliesTo");
  KmlBegin(kf, "TimeGroups");
  MArrayForEach(c->time_groups, &tg, &i)
  {
    MAssert(KheTimeGroupId(tg) != NULL, "KheArchiveWrite:  Id missing in "
      "TimeGroup referenced from ClusterBusyTimesConstraint %s", c->id);
    KmlEltAttribute(kf, "TimeGroup", "Reference", KheTimeGroupId(tg));
  }
  KmlEnd(kf, "TimeGroups");
  KmlEltFmtText(kf, "Minimum", "%d", c->minimum);
  KmlEltFmtText(kf, "Maximum", "%d", c->maximum);
  KmlEnd(kf, "ClusterBusyTimesConstraint");
}
