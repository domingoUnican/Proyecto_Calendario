
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
/*  FILE:         khe_group_monitor.c                                        */
/*  DESCRIPTION:  A group monitor                                            */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR - monitors a group of other monitors.                  */
/*                                                                           */
/*****************************************************************************/

struct khe_group_monitor_rec {
  INHERIT_GROUP_MONITOR
  KHE_GROUP_MONITOR	   copy;		/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,        */
/*    char *sub_tag_label)                                                   */
/*                                                                           */
/*  Make a new group monitor with these attributes, and no parent or         */
/*  children.                                                                */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label)
{
  KHE_GROUP_MONITOR res;  /* int i; */
  if( DEBUG1 )
    fprintf(stderr, "[ KheGroupMonitorMake(soln, %d, %s)\n",
      sub_tag, sub_tag_label);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln, KHE_GROUP_MONITOR_TAG);
  MArrayInit(res->child_links);
  MArrayInit(res->defect_links);
  /* MArrayInit(res->defect_links_copy); */
  MArrayInit(res->traces);
  res->sub_tag = sub_tag;
  res->sub_tag_label = sub_tag_label;
  res->copy = NULL;
  if( DEBUG1 )
    fprintf(stderr, "] KheGroupMonitorMake returning %p\n", (void *) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Delete gm.  This should *not* delete its child monitors!                 */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm)
{
  MAssert(gm != (KHE_GROUP_MONITOR) gm->soln,
    "KheGroupMonitorDelete:  gm is soln");
  MAssert(MArraySize(gm->traces) == 0,
    "KheGroupMonitorDelete:  gm is currently being traced");
  while( KheGroupMonitorChildMonitorCount(gm) > 0 )
    KheGroupMonitorDeleteChildMonitor(gm, KheGroupMonitorChildMonitor(gm, 0));
  KheMonitorDeleteAllParentMonitors((KHE_MONITOR) gm);
  KheSolnDeleteMonitor(gm->soln, (KHE_MONITOR) gm);
  MArrayFree(gm->child_links);
  MArrayFree(gm->defect_links);
  /* MArrayFree(gm->defect_links_copy); */
  MArrayFree(gm->traces);
  MFree(gm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm)                */
/*                                                                           */
/*  Move gm's child monitors to be children of gm's parent, if any, then     */
/*  delete gm.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm)
{
  KHE_MONITOR_LINK plk, clk;  int i, j;
  MArrayForEach(gm->parent_links, &plk, &i)
    MArrayForEach(gm->child_links, &clk, &j)
      KheGroupMonitorAddChildMonitor(plk->parent, clk->child);
  KheGroupMonitorDelete(gm);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm)        */
/*                                                                           */
/*  Carry out Phase 1 of copying gm.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm)
{
  KHE_GROUP_MONITOR copy;  int i;  KHE_MONITOR_LINK link;
  if( gm->soln == (KHE_SOLN) gm )
  {
    /* gm is actually the soln object */
    return (KHE_GROUP_MONITOR) KheSolnCopyPhase1((KHE_SOLN) gm);
  }
  else
  {
    if( gm->copy == NULL )
    {
      /* KheSolnMatchingUpdate(gm->soln); */
      MAssert(MArraySize(gm->traces) == 0,
        "KheGroupMonitorCopy cannot copy:  gm is currently being traced");
      MMake(copy);
      gm->copy = copy;
      KheMonitorCopyCommonFieldsPhase1((KHE_MONITOR) copy, (KHE_MONITOR) gm);
      MArrayInit(copy->child_links);
      MArrayForEach(gm->child_links, &link, &i)
        MArrayAddLast(copy->child_links, KheMonitorLinkCopyPhase1(link));
      MArrayInit(copy->defect_links);
      MArrayForEach(gm->defect_links, &link, &i)
        MArrayAddLast(copy->defect_links, KheMonitorLinkCopyPhase1(link));
      /* MArrayInit(copy->defect_links_copy); */
      MArrayInit(copy->traces);
      copy->sub_tag = gm->sub_tag;
      copy->sub_tag_label = gm->sub_tag_label;
      copy->copy = NULL;
    }
    return gm->copy;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm)                     */
/*                                                                           */
/*  Carry out Phase 2 of copying gm.                                         */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm)
{
  KHE_MONITOR_LINK link;  int i;
  if( gm->soln == (KHE_SOLN) gm )
  {
    /* gm is actually the soln object */
    KheSolnCopyPhase2((KHE_SOLN) gm);
  }
  else if( gm->copy != NULL )
  {
    gm->copy = NULL;
    KheMonitorCopyCommonFieldsPhase2((KHE_MONITOR) gm);
    MArrayForEach(gm->child_links, &link, &i)
      KheMonitorLinkCopyPhase2(link);
    MArrayForEach(gm->defect_links, &link, &i)
      KheMonitorLinkCopyPhase2(link);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm)                          */
/*                                                                           */
/*  Return the sub_tag attribute of gm.                                      */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm)
{
  return gm->sub_tag;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm)                   */
/*                                                                           */
/*  Return the sub_tag_label attribute of gm.                                */
/*                                                                           */
/*****************************************************************************/

char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm)
{
  return gm->sub_tag_label;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "child monitors"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheGroupMonitorCycle(KHE_GROUP_MONITOR gm, KHE_MONITOR m)           */
/*                                                                           */
/*  Return true if adding m to gm would cause a monitor cycle.               */
/*                                                                           */
/*****************************************************************************/

static bool KheGroupMonitorCycle(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  return KheMonitorTag(m) != KHE_GROUP_MONITOR_TAG ? false :
    KheMonitorPathCount((KHE_MONITOR) gm, m) >= 1;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m,       */
/*    KHE_MONITOR_LINK link)                                                 */
/*                                                                           */
/*  Add m to the list of defects of gm.  Here link joins gm and m.           */
/*                                                                           */
/*****************************************************************************/

static void KheGroupMonitorAddDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_MONITOR_LINK link)
{
  int pos;
  if( DEBUG3 )
  {
    MAssert(link->parent_defects_index == -1,
      "KheGroupMonitorAddDefect internal error 1");
    MAssert(!MArrayContains(gm->defect_links, link, &pos),
      "KheGroupMonitorAddDefect internal error 2");
  }
  link->parent_defects_index = MArraySize(gm->defect_links);
  MArrayAddLast(gm->defect_links, link);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m,    */
/*    KHE_MONITOR_LINK link)                                                 */
/*                                                                           */
/*  Delete m from the list of defects of gm.  Here link links gm and m.      */
/*                                                                           */
/*****************************************************************************/

static void KheGroupMonitorDeleteDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_MONITOR_LINK link)
{
  int pos;  KHE_MONITOR_LINK link2;
  if( DEBUG2 )
  {
    fprintf(stderr,
      "  [ KheGroupMonitorDeleteDefect(gm %p (%d defects), m (di %d)\n", 
      (void *) gm, MArraySize(gm->defect_links), link->parent_defects_index);
    KheMonitorDebug((KHE_MONITOR) gm, 1, 4, stderr);
    KheMonitorDebug(m, 2, 4, stderr);
  }
  /* bug here */
  MAssert(MArrayGet(gm->defect_links, link->parent_defects_index) == link,
    "KheGroupMonitorDeleteDefect internal error 1");
  link2 = MArrayRemoveLast(gm->defect_links);
  if( link2 != link )
  {
    MArrayPut(gm->defect_links, link->parent_defects_index, link2);
    link2->parent_defects_index = link->parent_defects_index;
  }
  link->parent_defects_index = -1;
  if( DEBUG3 )
  {
    MAssert(link->parent_defects_index == -1,
      "KheGroupMonitorDeleteDefect internal error 2");
    MAssert(!MArrayContains(gm->defect_links, link, &pos),
      "KheGroupMonitorDeleteDefect internal error 3");
  }
  if( DEBUG2 )
    fprintf(stderr, "  ] KheGroupMonitorDeleteDefect\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)      */
/*                                                                           */
/*  Internal function which adds m to gm, assuming m is currently not the    */
/*  child of any monitor.                                                    */
/*                                                                           */
/*****************************************************************************/

/* *** folded into KheGroupMonitorAddChildMonitor now 
void KheGroupMonitorAddMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  MAssert(!KheGroupMonitorCycle(gm, m),
    "KheGroupMonitorAddChildMonitor: operation would cause a monitor cycle");
  ** *** not doing this stuff any more
  if( KheMonitorAttached(m) && !KheMonitorAttached((KHE_MONITOR) gm) )
    KheMonitorAttach((KHE_MONITOR) gm);
  *** **

  ** add m to child_monitors **
  KheMonitorSetParentMonitorAndIndex(m, gm, MArraySize(gm->child_monitors));
  MArrayAddLast(gm->child_monitors, m);

  ** change gm's cost and add m to defects, if m has non-zero cost **
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, 0, KheMonitorCost(m));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)   */
/*                                                                           */
/*  Internal function which deletes m from gm.                               */
/*                                                                           */
/*****************************************************************************/

/* folded into KheGroupMonitorDeleteChildMonitor now
void KheGroupMonitorDeleteMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR m2;

  ** remove m from child_monitors **
  MAssert(MArrayGet(gm->child_monitors, KheMonitorParentIndex(m)) == m,
    "KheGroupMonitorDeleteMonitor internal error");
  m2 = MArrayRemoveLast(gm->child_monitors);
  if( m2 != m )
  {
    MArrayPut(gm->child_monitors, KheMonitorParentIndex(m), m2);
    KheMonitorSetParentMonitorAndIndex(m2, gm, KheMonitorParentIndex(m));
  }

  ** change gm's cost and remove m from defects, if m has non-zero cost **
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, KheMonitorCost(m), 0);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorChangeLowerBound(KHE_GROUP_MONITOR gm,               */
/*    KHE_COST delta)                                                        */
/*                                                                           */
/*  Add delta to the lower bound of gm and its ancestors along all paths.    */
/*                                                                           */
/*****************************************************************************/

static void KheGroupMonitorChangeLowerBound(KHE_GROUP_MONITOR gm,
  KHE_COST delta)
{
  KHE_MONITOR_LINK link;  int i;
  gm->lower_bound += delta;
  MArrayForEach(gm->parent_links, &link, &i)
    KheGroupMonitorChangeLowerBound(link->parent, delta);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m) */
/*                                                                           */
/*  Add m as a child to gm.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR_LINK link;

  /* make sure m's cost is up to date (IMPORTANT - this is a nasty bug fix!) */
  KheMonitorCost(m);

  /* make sure the add would not cause a monitor cycle */
  MAssert(!KheGroupMonitorCycle(gm, m),
    "KheGroupMonitorAddChildMonitor: operation would cause a monitor cycle");

  /* get a new link object from the soln, or make one, and initialize it */
  link = KheSolnGetMonitorLinkFromFreeList(gm->soln);
  if( link == NULL )
    MMake(link);
  link->parent = gm;
  link->child = m;
  link->parent_index = MArraySize(gm->child_links);
  MArrayAddLast(gm->child_links, link);
  link->parent_defects_index = -1;  /* not considered a defect yet */
  KheMonitorAddParentLink(m, link);  /* will set link->index_in_child */
  link->copy = NULL;

  /* update gm's lower bound */
  if( KheMonitorLowerBound(m) > 0 )
    KheGroupMonitorChangeLowerBound(gm, KheMonitorLowerBound(m));

  /* change gm's cost and add m to defects, if m has non-zero cost */
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, link, 0, KheMonitorCost(m));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm,             */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Delete m from gm.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR_LINK link, link2;

  /* change gm's cost and remove m from defects, if m has non-zero cost */
  if( !KheMonitorHasParentLink(m, gm, &link) )
    MAssert(false, "KheGroupMonitorDeleteChildMonitor: gm not parent of m");
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, link, KheMonitorCost(m), 0);

  /* update gm's lower bound */
  if( KheMonitorLowerBound(m) > 0 )
    KheGroupMonitorChangeLowerBound(gm, - KheMonitorLowerBound(m));

  /* remove link from m's parent links */
  KheMonitorDeleteParentLink(m, link);

  /* remove link from gm's child_links */
  link2 = MArrayRemoveLast(gm->child_links);
  if( link2 != link )
  {
    MArrayPut(gm->child_links, link->parent_index, link2);
    link2->parent_index = link->parent_index;
  }
  link->parent_index = -1;

  /* free link */
  KheSolnAddMonitorLinkToFreeList(gm->soln, link);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheGroupMonitorHasChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m) */
/*                                                                           */
/*  Return true when m is a child of gm.                                     */
/*                                                                           */
/*  Implementation note.  It will usually be faster to search the parent     */
/*  links of m rather than the child links of gm, since monitors usually     */
/*  have very few parents.                                                   */
/*                                                                           */
/*****************************************************************************/

bool KheGroupMonitorHasChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR_LINK link;
  return KheMonitorHasParentLink(m, gm, &link);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm)               */
/*                                                                           */
/*  Return the number of monitors reporting to gm.                           */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm)
{
  return MArraySize(gm->child_links);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i)     */
/*                                                                           */
/*  Return the i'th monitor reporting to gm.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i)
{
  KHE_MONITOR_LINK link;
  link = MArrayGet(gm->child_links, i);
  return link->child;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm)                     */
/*                                                                           */
/*  Return the number of defects (child monitors of non-zero cost) of gm.    */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm)
{
  KheSolnMatchingUpdate(gm->soln);
  return MArraySize(gm->defect_links);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i)           */
/*                                                                           */
/*  Return the i'th defect (child monitor of non-zero cost) of gm.           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i)
{
  KHE_MONITOR_LINK link;
  KheSolnMatchingUpdate(gm->soln);
  link = MArrayGet(gm->defect_links, i);
  return link->child;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDefectSort(KHE_GROUP_MONITOR gm, bool diversify)     */
/*                                                                           */
/*  Sort the defects of gm into decreasing cost order.  If diversify is      */
/*  true, break ties in cost using gm's solution's diversifier.              */
/*                                                                           */
/*****************************************************************************/

/* ***
static int KheMonitorLinkDecreasingCostCmp(const void *t1, const void *t2)
{
  KHE_MONITOR_LINK link1 = * (KHE_MONITOR_LINK *) t1;
  KHE_MONITOR_LINK link2 = * (KHE_MONITOR_LINK *) t2;
  KHE_MONITOR m1 = link1->child;
  KHE_MONITOR m2 = link2->child;
  int cmp = KheCostCmp(KheMonitorCost(m1), KheMonitorCost(m2));
  if( cmp != 0 )
    return cmp;
  else
    return KheMonitorIndex InSoln(m1) - KheMonitorIndex InSoln(m2);
}

static int KheMonitorLinkDecreasingCostDiversifyCmp(const void *t1,
  const void *t2)
{
  KHE_MONITOR_LINK link1 = * (KHE_MONITOR_LINK *) t1;
  KHE_MONITOR_LINK link2 = * (KHE_MONITOR_LINK *) t2;
  KHE_MONITOR m1 = link1->child;
  KHE_MONITOR m2 = link2->child;
  int cmp = KheCostCmp(KheMonitorCost(m1), KheMonitorCost(m2));
  if( cmp != 0 )
    return cmp;
  else if( KheSolnDiver sifierChoose(KheMonitorSoln(m1), 2) == 0 )
    return KheMonitorIndex InSoln(m1) - KheMonitorIndex InSoln(m2);
  else
    return KheMonitorIndex InSoln(m2) - KheMonitorIndex InSoln(m1);
}

void KheGroupMonitorDefectSort(KHE_GROUP_MONITOR gm, bool diversify)
{
  int i;  KHE_MONITOR_LINK link;

  ** make sure soln's cost is up to date (IMPORTANT - nasty bug fix!) **
  KheSolnCost(gm->soln);
  MArraySort(gm->defect_links, diversify ?
    KheMonitorLinkDecreasingCostDiversifyCmp : KheMonitorLinkDecreasingCostCmp);
  MArrayForEach(gm->defect_links, &link, &i)
    link->parent_defects_index = i;
  if( DEBUG3 )
    for( i = 1;  i < MArraySize(gm->defect_links);  i++ )
      MAssert(MArrayGet(gm->defect_links,i) != MArrayGet(gm->defect_links,i-1),
	"KheGroupMonitorDefectSort internal error");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCopyDefects(KHE_GROUP_MONITOR gm)                    */
/*                                                                           */
/*  Initialize the copied defect list of gm.                                 */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheGroupMonitorCopyDefects(KHE_GROUP_MONITOR gm)
{ 
  int i;
  MArrayClear(gm->defect_links_copy);
  MArrayAppend(gm->defect_links_copy, gm->defect_links, i);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDefectCopyCount(KHE_GROUP_MONITOR gm)                 */
/*                                                                           */
/*  Return the number of elements on gm's copied defect list.                */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheGroupMonitorDefectCopyCount(KHE_GROUP_MONITOR gm)
{
  return MArraySize(gm->defect_links_copy);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorDefectCopy(KHE_GROUP_MONITOR gm, int i)       */
/*                                                                           */
/*  Return the i'th element of gm's copied defect list.                      */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_MONITOR KheGroupMonitorDefectCopy(KHE_GROUP_MONITOR gm, int i)
{
  KHE_MONITOR_LINK link;
  link = MArrayGet(gm->defect_links_copy, i);
  return link->child;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm,                 */
/*    KHE_MONITOR_TAG tag, int *defect_count)                                */
/*                                                                           */
/*  Find the cost of monitors of type tag in the subtree rooted at gm,       */
/*  and the number of defects.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm, KHE_MONITOR_TAG tag,
  int *defect_count)
{
  KHE_COST res;  int i, dc;  KHE_MONITOR_LINK link;  KHE_MONITOR m;
  res = 0;  *defect_count = 0;
  if( tag == KHE_GROUP_MONITOR_TAG )
    return res;
  KheSolnMatchingUpdate(gm->soln);
  MArrayForEach(gm->defect_links, &link, &i)
  {
    m = link->child;
    if( KheMonitorTag(m) == tag )
    {
      res += KheMonitorCost(m);
      *defect_count += 1;
    }
    else if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
    {
      res += KheGroupMonitorCostByType((KHE_GROUP_MONITOR) m, tag, &dc);
      *defect_count += dc;
    }
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of gm's cost onto fp.                                        */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp)
{
  int tag, defect_count, total_defect_count;  KHE_COST cost, total_cost;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s%-31s %9s %13s\n", indent, "", gm->sub_tag_label,
      "Defects", "Cost");
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    total_cost = 0;  total_defect_count = 0;
    for( tag = 0;  tag < KHE_MONITOR_TAG_COUNT;  tag++ )
    {
      cost = KheGroupMonitorCostByType(gm, tag, &defect_count);
      if( cost != 0 || defect_count != 0 )
	fprintf(fp, "%*s%-34s %6d %13.5f\n", indent, "", KheMonitorTagShow(tag),
	  defect_count, KheCostShow(cost));
      total_cost += cost;
      total_defect_count += defect_count;
    }
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    fprintf(fp, "%*s%-34s %6d %13.5f\n", indent, "", "Total",
      total_defect_count, KheCostShow(total_cost));
    MAssert(total_cost == gm->cost, "KheGroupMonitorCostByTypeDebug "
      " internal error (total_cost %.5f, gm->cost %.5f)\n",
      KheCostShow(total_cost), KheCostShow(gm->cost));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAttach(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Attach gm.   It is known to be currently detached with cost 0.           */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used with group monitors
void KheGroupMonitorAttach(KHE_GROUP_MONITOR gm)
{
  MAssert(gm->cost == 0, "KheGroupMonitorAttach internal error");
  gm->attached = true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDetach(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Detach gm.  It is known to be currently attached.                        */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used with group monitors
void KheGroupMonitorDetach(KHE_GROUP_MONITOR gm)
{
  int i;  KHE_MONITOR m;
  MArrayForEach(gm->child_monitors, &m, &i)
    if( KheMonitorAttached(m) )
      KheMonitorDetach(m);
  gm->attached = false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "update and tracing"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)        */
/*                                                                           */
/*  Begin tracing t.                                                         */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)
{
  MArrayAddLast(gm->traces, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)          */
/*                                                                           */
/*  End tracing t.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)
{
  int pos;
  if( !MArrayContains(gm->traces, t, &pos) )
    MAssert(false, "KheGroupMonitorEndTrace internal error");
  MArrayRemove(gm->traces, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,      */
/*    KHE_COST old_cost, KHE_COST new_cost)                                  */
/*                                                                           */
/*  Let gm know that its child monitor m (with the given link) has changed   */
/*  its cost from old_cost to new_cost.                                      */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_MONITOR_LINK link, KHE_COST old_cost, KHE_COST new_cost)
{
  KHE_TRACE t;  int i;  KHE_COST delta_cost;  KHE_MONITOR_LINK link2;
  if( DEBUG2 )
  {
    /* important: can't call KheMonitorCost in this function! */
    fprintf(stderr,
      "[ KheGroupMonitorChangeCost(gm(%.5f), m, %.5f, %.5f)\n",
      KheCostShow((gm->cost)), KheCostShow(old_cost), KheCostShow(new_cost));
  }
  if( DEBUG3 )
  {
    MAssert(MArrayGet(gm->child_links, link->parent_index) == link,
      "KheGroupMonitorChangeCost internal error 1");
    if( old_cost > 0 )
    {
      MAssert(link->parent_defects_index >= 0 &&
        MArrayGet(gm->defect_links, link->parent_defects_index) == link,
        "KheGroupMonitorChangeCost internal error 2");
    }
    MAssert(new_cost >= 0, "KheGroupMonitorChangeCost internal error 3");
    MAssert(new_cost != old_cost, "KheGroupMonitorChangeCost internal error 4");
  }
  MArrayForEach(gm->traces, &t, &i)
    KheTraceChangeCost(t, m, old_cost);
  delta_cost = new_cost - old_cost;
  gm->cost += delta_cost;
  if( old_cost == 0 )
    KheGroupMonitorAddDefect(gm, m, link);
  else if( new_cost == 0 )
    KheGroupMonitorDeleteDefect(gm, m, link);
  MArrayForEach(gm->parent_links, &link2, &i)
    KheGroupMonitorChangeCost(link2->parent, (KHE_MONITOR) gm, link2,
      gm->cost - delta_cost, gm->cost);
  if( DEBUG2 )
    fprintf(stderr, "] KheGroupMonitorChangeCost returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDeviationCount(KHE_GROUP_MONITOR m)                   */
/*                                                                           */
/*  Return the deviations of m (0 in this case).                             */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDeviationCount(KHE_GROUP_MONITOR m)
{
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDeviation(KHE_GROUP_MONITOR m, int i)                 */
/*                                                                           */
/*  Return the i'th deviation of m.  There are none it's an error.           */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDeviation(KHE_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheGroupMonitorDeviation: i out of range");
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheGroupMonitorDeviationDescription(KHE_GROUP_MONITOR m, int i)    */
/*                                                                           */
/*  Return a description of the i'th deviation of m.  There are no           */
/*  deviations so it's an error.                                             */
/*                                                                           */
/*****************************************************************************/

char *KheGroupMonitorDeviationDescription(KHE_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheGroupMonitorDeviationDescription: i out of range");
  return NULL;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,                    */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of gm, showing only the defective child monitors.            */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp)
{
  int i;  KHE_MONITOR m;  KHE_MONITOR_LINK link;
  if( verbosity >= 1 )
  {
    KheSolnMatchingUpdate(gm->soln);
    KheMonitorDebugWithTagBegin((KHE_MONITOR) gm,
      gm->sub_tag_label != NULL ? gm->sub_tag_label : "GroupMonitor",
      indent, fp);
    fprintf(fp, " (sub_tag %d) %d %s", gm->sub_tag,MArraySize(gm->defect_links),
      MArraySize(gm->defect_links) == 1 ? "defect" : "defects");
    if( indent >= 0 && verbosity >= 2 && MArraySize(gm->defect_links) > 0 )
    {
      fprintf(fp, "\n");
      MArrayForEach(gm->defect_links, &link, &i)
      {
	m = link->child;
	if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
	  KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) m,
	    verbosity, indent + 2, fp);
	else
	  KheMonitorDebug(m, verbosity, indent + 2, fp);
      }
      KheMonitorDebugEnd((KHE_MONITOR) gm, false, indent, fp);
    }
    else
      KheMonitorDebugEnd((KHE_MONITOR) gm, true, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm,                          */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm, int verbosity,
  int indent, FILE *fp)
{
  int i;  KHE_MONITOR_LINK link;
  if( verbosity >= 1 )
  {
    KheMonitorDebugWithTagBegin((KHE_MONITOR) gm,
      gm->sub_tag_label != NULL ? gm->sub_tag_label : "GroupMonitor",
      indent, fp);
    fprintf(fp, " (sub_tag %d) %d %s", gm->sub_tag,
      MArraySize(gm->child_links),
      MArraySize(gm->child_links) == 1 ? "child" : "children");
    if( indent >= 0 && verbosity >= 2 && MArraySize(gm->child_links) > 0 )
    {
      fprintf(fp, "\n");
      MArrayForEach(gm->child_links, &link, &i)
        KheMonitorDebug(link->child, verbosity, indent + 2, fp);
      KheMonitorDebugEnd((KHE_MONITOR) gm, false, indent, fp);
    }
    else
      KheMonitorDebugEnd((KHE_MONITOR) gm, true, indent, fp);
  }
}
