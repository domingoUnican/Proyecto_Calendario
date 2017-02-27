
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
/*  FILE:         khe_node.c                                                 */
/*  DESCRIPTION:  A layer tree node                                          */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG10 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE - a layer tree node                                             */
/*                                                                           */
/*****************************************************************************/

struct khe_node_rec {
  void				*back;			/* back pointer      */
  int				visit_num;		/* visit_number      */
  int				reference_count;	/* reference count   */
  KHE_SOLN			soln;			/* enclosing soln    */
  int				soln_index;		/* index in soln     */
  int				duration;		/* total durn of ses */
  KHE_NODE			parent_node;		/* optional parent   */
  ARRAY_KHE_LAYER		parent_layers;		/* parent layers     */
  ARRAY_KHE_NODE		child_nodes;		/* child nodes       */
  ARRAY_KHE_LAYER		child_layers;		/* child layers      */
  ARRAY_KHE_MEET		meets;			/* meets             */
  ARRAY_KHE_ZONE		zones;			/* zones             */
  KHE_NODE			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "back pointers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelSetBack(KHE_NODE node, void *back)                     */
/*                                                                           */
/*  Set the back pointer of node to back, assuming all is well.              */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelSetBack(KHE_NODE node, void *back)
{
  KheSolnOpNodeSetBack(node->soln, node, node->back, back);
  node->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelSetBackUndo(KHE_NODE node, void *old_back,             */
/*    void *new_back)                                                        */
/*                                                                           */
/*  Undo KheNodeKernelSetBack.                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelSetBackUndo(KHE_NODE node, void *old_back, void *new_back)
{
  node->back = old_back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetBack(KHE_NODE node, void *back)                           */
/*                                                                           */
/*  Set the back pointer of node.                                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetBack(KHE_NODE node, void *back)
{
  if( back != node->back )
    KheNodeKernelSetBack(node, back);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheNodeBack(KHE_NODE node)                                         */
/*                                                                           */
/*  Return the back pointer of node.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheNodeBack(KHE_NODE node)
{
  return node->back;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetVisitNum(KHE_NODE n, int num)                             */
/*                                                                           */
/*  Set the visit number of n.                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetVisitNum(KHE_NODE n, int num)
{
  n->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeVisitNum(KHE_NODE n)                                          */
/*                                                                           */
/*  Return the visit number of n.                                            */
/*                                                                           */
/*****************************************************************************/

int KheNodeVisitNum(KHE_NODE n)
{
  return n->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeVisited(KHE_NODE n, int slack)                               */
/*                                                                           */
/*  Return true if n has been visited recently.                              */
/*                                                                           */
/*****************************************************************************/

bool KheNodeVisited(KHE_NODE n, int slack)
{
  return KheSolnGlobalVisitNum(n->soln) - n->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeVisit(KHE_NODE n)                                            */
/*                                                                           */
/*  Visit n.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheNodeVisit(KHE_NODE n)
{
  n->visit_num = KheSolnGlobalVisitNum(n->soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnVisit(KHE_NODE n)                                          */
/*                                                                           */
/*  Unvisit n.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeUnVisit(KHE_NODE n)
{
  n->visit_num = KheSolnGlobalVisitNum(n->soln) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "other simple attributes"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheNodeSoln(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the solution containing node.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheNodeSoln(KHE_NODE node)
{
  return node->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetSoln(KHE_NODE node, KHE_SOLN soln)                        */
/*                                                                           */
/*  Set the soln attribute of node.                                          */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetSoln(KHE_NODE node, KHE_SOLN soln)
{
  node->soln = soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeSolnIndex(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the soln_index attribute of node.                                 */
/*                                                                           */
/*****************************************************************************/

int KheNodeSolnIndex(KHE_NODE node)
{
  return node->soln_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetSolnIndex(KHE_NODE node, int soln_index)                  */
/*                                                                           */
/*  Set the soln_index attribute of node.                                    */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetSolnIndex(KHE_NODE node, int soln_index)
{
  int i;  KHE_LAYER layer;
  if( soln_index != node->soln_index )
  {
    MArrayForEach(node->parent_layers, &layer, &i)
      KheLayerChangeChildNodeIndex(layer, node->soln_index, soln_index);
    node->soln_index = soln_index;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeIsCycleNode(KHE_NODE node)                                   */
/*                                                                           */
/*  Return true if node is the cycle node (if it has at least one meet,      */
/*  and its first meet is a cycle meet).                                     */
/*                                                                           */
/*****************************************************************************/

bool KheNodeIsCycleNode(KHE_NODE node)
{
  return MArraySize(node->meets) > 0 &&
    KheMeetIsCycleMeet(MArrayFirst(node->meets));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDuration(KHE_NODE node)                                       */
/*                                                                           */
/*  Return the total duration of the meets of node.                          */
/*                                                                           */
/*****************************************************************************/

int KheNodeDuration(KHE_NODE node)
{
  return node->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeAssignedDuration(KHE_NODE node)                               */
/*                                                                           */
/*  Return the total assigned duration of the meets of node.                 */
/*                                                                           */
/*****************************************************************************/

int KheNodeAssignedDuration(KHE_NODE node)
{
  KHE_MEET meet;  int i, res;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    res += KheMeetAssignedDuration(meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAssignedDurationDebug(KHE_NODE node, int verbosity,          */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print showing node's assigned duration.                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeAssignedDurationDebug(KHE_NODE node, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET meet;  int i;
  MArrayForEach(node->meets, &meet, &i)
    KheMeetAssignedDurationDebug(meet, verbosity, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDemand(KHE_NODE node)                                         */
/*                                                                           */
/*  Return the total demand of the meets of node.                            */
/*                                                                           */
/*****************************************************************************/

int KheNodeDemand(KHE_NODE node)
{
  KHE_MEET meet;  int i, res;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    res += KheMeetDemand(meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "creation and deletion"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeDoMake(void)                                             */
/*                                                                           */
/*  Obtain a new node from the memory allocator; initialize its arrays.      */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE KheNodeDoMake(void)
{
  KHE_NODE res;
  MMake(res);
  MArrayInit(res->parent_layers);
  MArrayInit(res->child_nodes);
  MArrayInit(res->child_layers);
  MArrayInit(res->meets);
  MArrayInit(res->zones);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnMake(KHE_NODE node)                                        */
/*                                                                           */
/*  Undo KheNodeDoMake, returning node's memory to the memory allocator.     */
/*                                                                           */
/*****************************************************************************/

void KheNodeUnMake(KHE_NODE node)
{
  MArrayFree(node->parent_layers);
  MArrayFree(node->child_nodes);
  MArrayFree(node->child_layers);
  MArrayFree(node->meets);
  MArrayFree(node->zones);
  MFree(node);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeDoGet(KHE_SOLN soln)                                     */
/*                                                                           */
/*  Get a node object, either from soln's free list or allocated.            */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE KheNodeDoGet(KHE_SOLN soln)
{
  KHE_NODE res;
  res = KheSolnGetNodeFromFreeList(soln);
  if( res == NULL )
    res = KheNodeDoMake();
  res->reference_count = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnGet(KHE_NODE node)                                         */
/*                                                                           */
/*  Undo KheNodeDoGet, adding node to its soln's free list.                  */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUnGet(KHE_NODE node)
{
  KheSolnAddNodeToFreeList(node->soln, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReferenceCountIncrement(KHE_NODE node)                       */
/*                                                                           */
/*  Increment node's reference count.                                        */
/*                                                                           */
/*****************************************************************************/

void KheNodeReferenceCountIncrement(KHE_NODE node)
{
  node->reference_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReferenceCountDecrement(KHE_NODE node)                       */
/*                                                                           */
/*  Decrement node's reference count, and possibly add it to the free list.  */
/*                                                                           */
/*****************************************************************************/

void KheNodeReferenceCountDecrement(KHE_NODE node)
{
  MAssert(node->reference_count >= 1,
    "KheNodeReferenceCountDecrement internal error");
  if( --node->reference_count == 0 )
    KheNodeUnGet(node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoAdd(KHE_NODE node, KHE_SOLN soln)                          */
/*                                                                           */
/*  Initialize node and add it to soln.                                      */
/*                                                                           */
/*  Implementation node.  See KheNodeSetSolnIndex to understand the two      */
/*  cryptic lines before KheSolnAddNode.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoAdd(KHE_NODE node, KHE_SOLN soln)
{
  node->back = NULL;
  node->visit_num = 0;
  KheNodeReferenceCountIncrement(node);
  MArrayClear(node->parent_layers);  /* must precede KheSolnAddNode */
  node->soln_index = -1;             /* not strictly necessary      */
  KheSolnAddNode(soln, node);  /* will set soln and soln_index */
  node->duration = 0;
  node->parent_node = NULL;
  MArrayClear(node->child_nodes);
  MArrayClear(node->child_layers);
  MArrayClear(node->meets);
  MArrayClear(node->zones);
  node->copy = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnAdd(KHE_NODE node)                                         */
/*                                                                           */
/*  Undo KheNodeDoAdd, leaving node unlinked from the solution.              */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUnAdd(KHE_NODE node)
{
  /* delete from soln */
  KheSolnDeleteNode(node->soln, node);

  /* node is now not referenced from solution (this call may free node) */
  KheNodeReferenceCountDecrement(node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAdd(KHE_NODE node, KHE_SOLN soln)                      */
/*                                                                           */
/*  Kernel operation which adds node to soln (but does not make it).         */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAdd(KHE_NODE node, KHE_SOLN soln)
{
  KheNodeDoAdd(node, soln);
  KheSolnOpNodeAdd(soln, node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAddUndo(KHE_NODE node)                                 */
/*                                                                           */
/*  Undo KheNodeKernelAdd.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAddUndo(KHE_NODE node)
{
  KheNodeUnAdd(node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDelete(KHE_NODE node)                                  */
/*                                                                           */
/*  Kernel operation which deletes node (but does not free it).              */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDelete(KHE_NODE node)
{
  KheSolnOpNodeDelete(node->soln, node);
  KheNodeUnAdd(node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDeleteUndo(KHE_NODE node, KHE_SOLN soln)               */
/*                                                                           */
/*  Undo KheNodeKernelDelete.                                                */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDeleteUndo(KHE_NODE node, KHE_SOLN soln)
{
  KheNodeDoAdd(node, soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeMake(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Make a new, empty node lying in soln.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeMake(KHE_SOLN soln)
{
  KHE_NODE res;

  /* make and initialize a new node object from scratch */
  res = KheNodeDoGet(soln);

  /* add it to the soln */
  KheNodeKernelAdd(res, soln);

  /* return it */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteCheck(KHE_NODE node)                                   */
/*                                                                           */
/*  Check whether it is safe to delete node, which is true when all of the   */
/*  meets lying in node are willing to be removed from it.                   */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteCheck(KHE_NODE node)
{
  KHE_MEET meet;  int i;
  MArrayForEach(node->meets, &meet, &i)
    if( !KheMeetDeleteNodeCheck(meet, node) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDelete(KHE_NODE node)                                        */
/*                                                                           */
/*  Delete node.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDelete(KHE_NODE node)
{
  /* check safe to proceed */
  if( !KheNodeDeleteCheck(node) )
    return false;

  /* clear the back pointer of node (since it will be recreated NULL) */
  if( node->back != NULL )
    KheNodeSetBack(node, NULL);

  /* delete zones */
  while( MArraySize(node->zones) > 0 )
    KheZoneDelete(MArrayLast(node->zones));

  /* delete meets */
  while( MArraySize(node->meets) > 0 )
    if( !KheNodeDeleteMeet(node, MArrayLast(node->meets)) )
      MAssert(false, "KheNodeDelete internal error 2");

  /* delete from parent layers */
  while( MArraySize(node->parent_layers) > 0 )
    KheLayerDeleteChildNode(MArrayLast(node->parent_layers), node);

  /* delete from parent */
  if( node->parent_node != NULL && !KheNodeDeleteParent(node) )
    MAssert(false, "KheNodeDelete internal error 3");

  /* delete all child layers */
  KheNodeChildLayersDelete(node);

  /* delete from child_nodes */
  while( MArraySize(node->child_nodes) > 0 )
    if( !KheNodeDeleteParent(MArrayLast(node->child_nodes)) )
      MAssert(false, "KheNodeDelete internal error 4");

  /* carry out the kernel delete operation (may free node) */
  KheNodeKernelDelete(node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "copy"                                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeCopyPhase1(KHE_NODE node)                                */
/*                                                                           */
/*  Carry out Phase 1 of copying node.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeCopyPhase1(KHE_NODE node)
{
  KHE_NODE copy, child_node;  KHE_MEET meet;  int i;  KHE_LAYER layer;
  KHE_ZONE zone;
  if( node->copy == NULL )
  {
    MMake(copy);
    node->copy = copy;
    copy->back = node->back;
    copy->soln = KheSolnCopyPhase1(node->soln);
    copy->soln_index = node->soln_index;
    copy->visit_num = node->visit_num;
    copy->reference_count = 1;  /* no paths, and node is linked in */
    copy->parent_node = (node->parent_node == NULL ? NULL :
      KheNodeCopyPhase1(node->parent_node)); /* bug fix here */
    MArrayInit(copy->parent_layers);
    MArrayForEach(node->parent_layers, &layer, &i)
      MArrayAddLast(copy->parent_layers, KheLayerCopyPhase1(layer));
    MArrayInit(copy->child_nodes);
    MArrayForEach(node->child_nodes, &child_node, &i)
      MArrayAddLast(copy->child_nodes, KheNodeCopyPhase1(child_node));
    MArrayInit(copy->child_layers);
    MArrayForEach(node->child_layers, &layer, &i)
      MArrayAddLast(copy->child_layers, KheLayerCopyPhase1(layer));
    MArrayInit(copy->meets);
    MArrayForEach(node->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->zones);
    MArrayForEach(node->zones, &zone, &i)
      MArrayAddLast(copy->zones, KheZoneCopyPhase1(zone));
    copy->duration = node->duration;
    copy->copy = NULL;
  }
  return node->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeCopyPhase2(KHE_NODE node)                                    */
/*                                                                           */
/*  Carry out Phase 2 of copying node.                                       */
/*                                                                           */
/*****************************************************************************/

void KheNodeCopyPhase2(KHE_NODE node)
{
  int i;  KHE_ZONE zone;
  if( node->copy != NULL )
  {
    node->copy = NULL;
    MArrayForEach(node->zones, &zone, &i)
      KheZoneCopyPhase2(zone);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "parents and children"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoAddParent(KHE_NODE child_node, KHE_NODE parent_node)       */
/*                                                                           */
/*  Add child_node to parent_node; initially, child_node has no parent.      */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoAddParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  MAssert(child_node->parent_node == NULL,
    "KheNodeDoAddChildNode internal error");
  MArrayAddLast(parent_node->child_nodes, child_node);
  child_node->parent_node = parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoDeleteParent(KHE_NODE child_node, KHE_NODE parent_node)    */
/*                                                                           */
/*  Delete child_node from parent_node, assuming everything is in order.     */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoDeleteParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  int pos;
  MAssert(child_node->parent_node == parent_node,
    "KheNodeDoDeleteParent internal error");
  if( !MArrayContains(parent_node->child_nodes, child_node, &pos) )
    MAssert(false, "KheNodeDoDeleteParent internal error");
  while( MArraySize(child_node->parent_layers) > 0 )
    KheLayerDeleteChildNode(MArrayLast(child_node->parent_layers), child_node);
  child_node->parent_node = NULL;
  MArrayRemove(parent_node->child_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAddParent(KHE_NODE child_node, KHE_NODE parent_node)   */
/*                                                                           */
/*  Make child_node a child of parent_node, assuming all is well.            */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAddParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  KheSolnOpNodeAddParent(child_node->soln, child_node, parent_node);
  KheNodeDoAddParent(child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAddParentUndo(KHE_NODE child_node,KHE_NODE parent_node)*/
/*                                                                           */
/*  Undo KheNodeKernelAddParent.                                             */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAddParentUndo(KHE_NODE child_node, KHE_NODE parent_node)
{
  KheNodeDoDeleteParent(child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDeleteParent(KHE_NODE child_node, KHE_NODE parent_node)*/
/*                                                                           */
/*  Delete parent from child_node, assuming all is well.                     */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDeleteParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  KheSolnOpNodeDeleteParent(child_node->soln, child_node, parent_node);
  KheNodeDoDeleteParent(child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDeleteParentUndo(KHE_NODE child_node,                  */
/*    KHE_NODE parent_node)                                                  */
/*                                                                           */
/*  Undo KheNodeKernelDeleteParent.                                          */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDeleteParentUndo(KHE_NODE child_node, KHE_NODE parent_node)
{
  /* bug! KheNodeKernelAddParent(child_node, parent_node); */
  KheNodeDoAddParent(child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node)    */
/*                                                                           */
/*  Check whether linking these two nodes as child and parent is possible.   */
/*  There is no possibility of a violation of the node rule.                 */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_NODE node;

  /* check for already assigned */
  MAssert(child_node->parent_node == NULL,
    "KheNodeAddParentCheck: child_node already has a parent");

  /* check for cycles, including a node being its own parent */
  for( node = parent_node;  node != NULL;  node = node->parent_node )
    if( node == child_node )
      return false;  /* cycle */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node)         */
/*                                                                           */
/*  Link these two nodes as child and parent, if possible.                   */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  /* check safe to do */
  if( !KheNodeAddParentCheck(child_node, parent_node) )
    return false;

  /* link children and parents */
  KheNodeKernelAddParent(child_node, parent_node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteParentCheck(KHE_NODE child_node)                       */
/*                                                                           */
/*  Check whether deleting the link connecting child_node to its parent is   */
/*  possible.  There is no possibility of violating the cycle rule.          */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteParentCheck(KHE_NODE child_node)
{
  KHE_MEET meet;  int i;

  /* check already assigned */
  MAssert(child_node->parent_node != NULL,
    "KheNodeDeleteParentCheck: child_node has no parent");

  /* check for potential violations of the node rule */
  MArrayForEach(child_node->meets, &meet, &i)
    if( KheMeetAsst(meet) != NULL )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteParent(KHE_NODE child_node)                            */
/*                                                                           */
/*  Delete the link connecting child_node to its parent, if possible.        */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteParent(KHE_NODE child_node)
{
  /* check safe to do */
  if( !KheNodeDeleteParentCheck(child_node) )
    return false;

  /* unlink children and parents */
  KheNodeKernelDeleteParent(child_node, child_node->parent_node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeParent(KHE_NODE node)                                    */
/*                                                                           */
/*  Return the parent of node, or NULL if none.                              */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeParent(KHE_NODE node)
{
  return node->parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeChildCount(KHE_NODE node)                                     */
/*                                                                           */
/*  Return the number of children of node.                                   */
/*                                                                           */
/*****************************************************************************/

int KheNodeChildCount(KHE_NODE node)
{
  return MArraySize(node->child_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeChild(KHE_NODE node, int i)                              */
/*                                                                           */
/*  Return the i'th child of node.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeChild(KHE_NODE node, int i)
{
  return MArrayGet(node->child_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeIsDescendant(KHE_NODE node, KHE_NODE ancestor_node)          */
/*                                                                           */
/*  Return true if node is a descendant of ancestor_node.                    */
/*                                                                           */
/*****************************************************************************/

bool KheNodeIsDescendant(KHE_NODE node, KHE_NODE ancestor_node)
{
  while( node != NULL && node != ancestor_node )
    node = node->parent_node;
  return node == ancestor_node;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeIsProperDescendant(KHE_NODE node, KHE_NODE ancestor_node)    */
/*                                                                           */
/*  Return true if node is a proper descendant of ancestor_node.             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeIsProperDescendant(KHE_NODE node, KHE_NODE ancestor_node)
{
  return node != ancestor_node &&
    KheNodeIsDescendant(node->parent_node, ancestor_node);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "swapping child nodes and child layers"                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnAssignAllChildMeets(KHE_NODE node)                         */
/*                                                                           */
/*  Ensure that all meets in all child nodes of node are unassigned.         */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUnAssignAllChildMeets(KHE_NODE node)
{
  KHE_MEET meet, child_meet;  int i, j;
  MArrayForEach(node->meets, &meet, &i)
    for( j = 0;  j < KheMeetAssignedToCount(meet);  j++ )
    {
      child_meet = KheMeetAssignedTo(meet, j);
      if( KheMeetNode(child_meet) != NULL )
      {
	if( !KheMeetUnAssign(child_meet) )
	  MAssert(false,
	    "KheNodeSwapChildNodesAndLayers: cannot unassign child meet");
	j--;
      }
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)    */
/*                                                                           */
/*  Carry out the KheNodeSwapChildNodesAndLayers operation.                  */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)
{
  ARRAY_KHE_NODE tmp_nodes;  ARRAY_KHE_LAYER tmp_layers;
  KHE_NODE child_node;  KHE_LAYER child_layer;  int i;

  /* ensure that all meets in all child nodes are unassigned */
  MAssert(node1 != node2, "KheNodeSwapChildNodesAndLayers: node1 == node2");
  KheNodeUnAssignAllChildMeets(node1);
  KheNodeUnAssignAllChildMeets(node2);

  /* swap node1->child_nodes with node2->child_nodes */
  MArrayWholeSwap(node1->child_nodes, node2->child_nodes, tmp_nodes);
  MArrayForEach(node1->child_nodes, &child_node, &i)
    child_node->parent_node = node1;
  MArrayForEach(node2->child_nodes, &child_node, &i)
    child_node->parent_node = node2;

  /* swap node1->child_layers with node2->child_layers */
  MArrayWholeSwap(node1->child_layers, node2->child_layers, tmp_layers);
  MArrayForEach(node1->child_layers, &child_layer, &i)
    KheLayerSetParentNode(child_layer, node1);
  MArrayForEach(node2->child_layers, &child_layer, &i)
    KheLayerSetParentNode(child_layer, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)*/
/*                                                                           */
/*  Kernel operation for swapping node1 and node2's child nodes and layers.  */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)
{
  KheSolnOpNodeSwapChildNodesAndLayers(node1->soln, node1, node2);
  KheNodeDoSwapChildNodesAndLayers(node1, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelSwapChildNodesAndLayersUndo(KHE_NODE node1, node2)     */
/*                                                                           */
/*  Undo KheNodeKernelSwapChildNodesAndLayers.                               */
/*                                                                           */
/*  Implementation note.  KheNodeKernelSwapChildNodesAndLayers is a kind     */
/*  of swap, and hence is its own inverse.                                   */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelSwapChildNodesAndLayersUndo(KHE_NODE node1, KHE_NODE node2)
{
  KheNodeDoSwapChildNodesAndLayers(node1, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)      */
/*                                                                           */
/*  Swap the child nodes and layers of node1 and node2, after first          */
/*  ensuring that all meets in their child nodes are unassigned.             */
/*                                                                           */
/*****************************************************************************/

void KheNodeSwapChildNodesAndLayers(KHE_NODE node1, KHE_NODE node2)
{
  KheNodeKernelSwapChildNodesAndLayers(node1, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "parent layers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer)         */
/*                                                                           */
/*  Add layer to child_node's set of parent layers.                          */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer)
{
  MArrayAddLast(child_node->parent_layers, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer)      */
/*                                                                           */
/*  Remove layer from child_node's set of parent layers.                     */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer)
{
  int pos;
  if( !MArrayContains(child_node->parent_layers, layer, &pos) )
    MAssert(false, "KheNodeDeleteParentLayer internal error");
  MArrayRemove(child_node->parent_layers, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeParentLayerCount(KHE_NODE child_node)                         */
/*                                                                           */
/*  Return the number of parent layers of child_node.                        */
/*                                                                           */
/*****************************************************************************/

int KheNodeParentLayerCount(KHE_NODE child_node)
{
  return MArraySize(child_node->parent_layers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i)                 */
/*                                                                           */
/*  Return the i                                                             */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i)
{
  return MArrayGet(child_node->parent_layers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2)             */
/*                                                                           */
/*  Return true if node1 and node2 have the same parent layers.              */
/*                                                                           */
/*****************************************************************************/

bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2)
{
  KHE_LAYER layer;  int i, pos;
  if( MArraySize(node1->parent_layers) != MArraySize(node2->parent_layers) )
    return false;
  MArrayForEach(node1->parent_layers, &layer, &i)
    if( !MArrayContains(node2->parent_layers, layer, &pos) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "child layers"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer)         */
/*                                                                           */
/*  Add layer as a new child layer to parent_node.                           */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer)
{
  KheLayerSetParentNode(layer, parent_node);
  KheLayerSetParentNodeIndex(layer, MArraySize(parent_node->child_layers));
  MArrayAddLast(parent_node->child_layers, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer)      */
/*                                                                           */
/*  Delete this child layer from parent_node.                                */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer)
{
  int pos, i;
  if( !MArrayContains(parent_node->child_layers, layer, &pos) )
    MAssert(false, "KheNodeDeleteChildLayer internal error");
  MArrayRemove(parent_node->child_layers, pos);
  for( i = pos;  i < MArraySize(parent_node->child_layers);  i++ )
  {
    layer = MArrayGet(parent_node->child_layers, i);
    KheLayerSetParentNodeIndex(layer, i);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeChildLayerCount(KHE_NODE parent_node)                         */
/*                                                                           */
/*  Return the number of child layers of parent_node.                        */
/*                                                                           */
/*****************************************************************************/

int KheNodeChildLayerCount(KHE_NODE parent_node)
{
  return MArraySize(parent_node->child_layers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i)                 */
/*                                                                           */
/*  Return the i'th child layer of parent_node.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i)
{
  return MArrayGet(parent_node->child_layers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCNodehildLayersSort(KHE_NODE parent_node,                        */
/*    int(*compar)(const void *, const void *))                              */
/*                                                                           */
/*  Sort the child arrays of parent_node, using comparison function compar.  */
/*                                                                           */
/*****************************************************************************/

void KheNodeChildLayersSort(KHE_NODE parent_node,
  int(*compar)(const void *, const void *))
{
  KHE_LAYER layer;  int i;
  MArraySort(parent_node->child_layers, compar);
  MArrayForEach(parent_node->child_layers, &layer, &i)
    KheLayerSetParentNodeIndex(layer, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeChildLayersDelete(KHE_NODE parent_node)                      */
/*                                                                           */
/*  Delete all the child layers of parent_node (solver, not kernel).         */
/*                                                                           */
/*****************************************************************************/

void KheNodeChildLayersDelete(KHE_NODE parent_node)
{
  while( MArraySize(parent_node->child_layers) > 0 )
    KheLayerDelete(MArrayLast(parent_node->child_layers));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoAddMeet(KHE_NODE node, KHE_MEET meet)                      */
/*                                                                           */
/*  Add meet to node.                                                        */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoAddMeet(KHE_NODE node, KHE_MEET meet)
{
  KHE_LAYER layer;  int i;
  KheMeetSetNode(meet, node);
  KheMeetSetNodeIndex(meet, MArraySize(node->meets));
  MArrayAddLast(node->meets, meet);
  node->duration += KheMeetDuration(meet);
  MArrayForEach(node->parent_layers, &layer, &i)
    KheLayerAddDuration(layer, KheMeetDuration(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoDeleteMeet(KHE_NODE node, KHE_MEET meet)                   */
/*                                                                           */
/*  Delete meet from node.                                                   */
/*                                                                           */
/*****************************************************************************/

static void KheNodeDoDeleteMeet(KHE_NODE node, KHE_MEET meet)
{
  KHE_MEET tmp;  KHE_LAYER layer;  int i;
  tmp = MArrayRemoveLast(node->meets);
  if( tmp != meet )
  {
    /* fill the hole left behind by meet with tmp */
    KheMeetSetNodeIndex(tmp, KheMeetNodeIndex(meet));
    MArrayPut(node->meets, KheMeetNodeIndex(meet), tmp);
  }
  KheMeetSetNode(meet, NULL);
  KheMeetSetNodeIndex(meet, -1);
  node->duration -= KheMeetDuration(meet);
  MArrayForEach(node->parent_layers, &layer, &i)
    KheLayerSubtractDuration(layer, KheMeetDuration(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAddMeet(KHE_NODE node, KHE_MEET meet)                  */
/*                                                                           */
/*  Add meet to node.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAddMeet(KHE_NODE node, KHE_MEET meet)
{
  KheSolnOpNodeAddMeet(node->soln, node, meet);
  KheNodeDoAddMeet(node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelAddMeetUndo(KHE_NODE node, KHE_MEET meet)              */
/*                                                                           */
/*  Undo KheNodeKernelAddMeet.                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelAddMeetUndo(KHE_NODE node, KHE_MEET meet)
{
  KheNodeDoDeleteMeet(node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDeleteMeet(KHE_NODE node, KHE_MEET meet)               */
/*                                                                           */
/*  Delete meet from node.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDeleteMeet(KHE_NODE node, KHE_MEET meet)
{
  KheSolnOpNodeDeleteMeet(node->soln, node, meet);
  KheNodeDoDeleteMeet(node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeKernelDeleteMeetUndo(KHE_NODE node, KHE_MEET meet)           */
/*                                                                           */
/*  Undo KheNodeKernelDeleteMeet.                                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeKernelDeleteMeetUndo(KHE_NODE node, KHE_MEET meet)
{
  KheNodeDoAddMeet(node, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet)                   */
/*                                                                           */
/*  Check whether adding meet to node is possible.                           */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet)
{
  return KheMeetAddNodeCheck(meet, node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet)                        */
/*                                                                           */
/*  Add meet to node, if possible.                                           */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet)
{
  /* check safe to do */
  if( !KheNodeAddMeetCheck(node, meet) )
    return false;

  /* add meet to node, and add node to meet */
  KheNodeKernelAddMeet(node, meet);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet)                */
/*                                                                           */
/*  Check whether deleting meet from node is possible.                       */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet)
{
  return KheMeetDeleteNodeCheck(meet, node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet)                     */
/*                                                                           */
/*  Delete meet from node, if possible.                                      */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet)
{
  /* check safe to do */
  if( !KheNodeDeleteMeetCheck(node, meet) )
    return false;

  /* delete meet from node */
  KheNodeKernelDeleteMeet(node, meet);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet)                   */
/*                                                                           */
/*  Similar to KheNodeAddMeet except that meet was created by a split,       */
/*  so no checks or changes to the duration or segments are needed.          */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet)
{
  KheMeetSetNodeIndex(meet, MArraySize(node->meets));
  MArrayAddLast(node->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet)                */
/*                                                                           */
/*  Similar to KheNodeDeleteMeet except that meet is being removed by        */
/*  a merge operation, so no checks or changes to the duration or segments   */
/*  are needed.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet)
{
  KHE_MEET tmp;
  tmp = MArrayGet(node->meets, KheMeetNodeIndex(meet));
  MAssert(tmp == meet, "KheNodeDeleteSplitMeet internal error");
  tmp = MArrayRemoveLast(node->meets);
  if( tmp != meet )
  {
    /* fill the hole left behind by meet with tmp */
    KheMeetSetNodeIndex(tmp, KheMeetNodeIndex(meet));
    MArrayPut(node->meets, KheMeetNodeIndex(meet), tmp);
  }
  KheMeetSetNode(meet, NULL);
  KheMeetSetNodeIndex(meet, -1);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeMeetCount(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the number of meets of node.                                      */
/*                                                                           */
/*****************************************************************************/

int KheNodeMeetCount(KHE_NODE node)
{
  return MArraySize(node->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheNodeMeet(KHE_NODE node, int i)                               */
/*                                                                           */
/*  Return the i'th meet of node.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheNodeMeet(KHE_NODE node, int i)
{
  return MArrayGet(node->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMeetSort(KHE_NODE node,                                      */
/*    int(*compar)(const void *, const void *))                              */
/*                                                                           */
/*  Function for sorting the meets of node using compar for comparison.      */
/*                                                                           */
/*****************************************************************************/

void KheNodeMeetSort(KHE_NODE node, int(*compar)(const void *, const void *))
{
  KHE_MEET meet;  int i;
  MArraySort(node->meets, compar);
  MArrayForEach(node->meets, &meet, &i)
    KheMeetSetNodeIndex(meet, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetDecreasingDurationCmp(const void *p1, const void *p2)         */
/*                                                                           */
/*  Comparison function for sorting an array of meets by decreasing          */
/*  duration order.                                                          */
/*                                                                           */
/*****************************************************************************/

int KheMeetDecreasingDurationCmp(const void *p1, const void *p2)
{
  KHE_MEET meet1 = * (KHE_MEET *) p1;
  KHE_MEET meet2 = * (KHE_MEET *) p2;
  if( KheMeetDuration(meet1) != KheMeetDuration(meet2) )
    return KheMeetDuration(meet2) - KheMeetDuration(meet1);
  else
    return KheMeetSolnIndex(meet1) - KheMeetSolnIndex(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingAsstCmp(const void *p1, const void *p2)             */
/*                                                                           */
/*  Comparison function for sorting an array of meets by increasing          */
/*  assignment order.                                                        */
/*                                                                           */
/*****************************************************************************/

int KheMeetIncreasingAsstCmp(const void *p1, const void *p2)
{
  KHE_MEET meet1 = * (KHE_MEET *) p1;
  KHE_MEET meet2 = * (KHE_MEET *) p2;
  int index1, index2;
  if( KheMeetAsst(meet1) == NULL )
  {
    if( KheMeetAsst(meet2) == NULL )
      return 0;
    else
      return 1;
  }
  else
  {
    if( KheMeetAsst(meet2) == NULL )
      return -1;
    else
    {
      index1 = KheMeetSolnIndex(KheMeetAsst(meet1));
      index2 = KheMeetSolnIndex(KheMeetAsst(meet2));
      if( index1 != index2 )
	return index1 - index2;
      else
	return KheMeetAsstOffset(meet1) - KheMeetAsstOffset(meet2);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "similarity and regularity"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2)                      */
/*                                                                           */
/*  Return true if node1 and node2 are similar.                              */
/*                                                                           */
/*****************************************************************************/

#define cleanup_and_return(cond)					     \
  return (MArrayFree(events1), MArrayFree(events2), MArrayFree(durations1),  \
    MArrayFree(durations2), MArrayFree(domains1), MArrayFree(domains2), cond)

bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2)
{
  ARRAY_KHE_EVENT events1, events2;  KHE_EVENT e1, e2, tmp;
  ARRAY_INT durations1, durations2;
  ARRAY_KHE_RESOURCE_GROUP domains1, domains2;
  KHE_MEET meet1, meet2;  int i1, i2, pos;
  if( DEBUG5 )
    fprintf(stderr, "[ KheNodeSimilar(Node %d, Node %d)\n",
      KheNodeSolnIndex(node1), KheNodeSolnIndex(node2));

  /* identical nodes are similar */
  if( node1 == node2 )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning true (same node)\n");
    return true;
  }

  /* make sure the durations and number of meets match */
  if( node1->duration != node2->duration )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (durations differ)\n");
    return false;
  }
  if( MArraySize(node1->meets) != MArraySize(node2->meets) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (meet counts)\n");
    return false;
  }

  /* make sure every meet has an event */
  MArrayForEach(node1->meets, &meet1, &i1)
    if( KheMeetEvent(meet1) == NULL )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (meet1 no event)\n");
      return false;
    }
  MArrayForEach(node2->meets, &meet2, &i2)
    if( KheMeetEvent(meet2) == NULL )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (meet2 no event)\n");
      return false;
    }

  /* find the events of each node, and their durations */
  MArrayInit(events1);
  MArrayInit(events2);
  MArrayInit(durations1);
  MArrayInit(durations2);
  MArrayInit(domains1);
  MArrayInit(domains2);
  MArrayForEach(node1->meets, &meet1, &i1)
  {
    e1 = KheMeetEvent(meet1);
    if( !MArrayContains(events1, e1, &pos) )
    {
      pos = MArraySize(events1);
      MArrayAddLast(events1, e1);
      MArrayAddLast(durations1, 0);
    }
    MArrayPut(durations1, pos,
      MArrayGet(durations1, pos) + KheMeetDuration(meet1));
  }
  MArrayForEach(node2->meets, &meet2, &i2)
  {
    e2 = KheMeetEvent(meet2);
    if( !MArrayContains(events2, e2, &pos) )
    {
      pos = MArraySize(events2);
      MArrayAddLast(events2, e2);
      MArrayAddLast(durations2, 0);
    }
    MArrayPut(durations2, pos,
      MArrayGet(durations2, pos) + KheMeetDuration(meet2));
  }

  /* make sure that every event that's here at all is here completely */
  MArrayForEach(events1, &e1, &i1)
    if( KheEventDuration(e1) != MArrayGet(durations1, i1) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (incomplete e1)\n");
      cleanup_and_return(false);
    }
  MArrayForEach(events2, &e2, &i2)
    if( KheEventDuration(e2) != MArrayGet(durations2, i2) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (incomplete e2)\n");
      cleanup_and_return(false);
    }

  /* reorder events2 so that similar events are at equal indexes */
  if( MArraySize(events1) != MArraySize(events2) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (event counts)\n");
    cleanup_and_return(false);
  }
  MArrayForEach(events1, &e1, &i1)
  {
    for( i2 = i1;  i2 < MArraySize(events2);  i2++ )
    {
      e2 = MArrayGet(events2, i2);
      if( KheEventPartitionSimilar(e1, e2, &domains1, &domains2) )
	break;
    }
    if( i2 >= MArraySize(events2) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (e1 not similar)\n");
      cleanup_and_return(false);
    }
    MArraySwap(events2, i1, i2, tmp);
  }
  if( DEBUG2 || DEBUG5 )
  {
    fprintf(stderr, "[ KheNodeSimilar(Node %d, Node %d) returning true:\n",
      KheNodeSolnIndex(node1), KheNodeSolnIndex(node2));
    for( i1 = 0;  i1 < MArraySize(events1);  i1++ )
    {
      e1 = MArrayGet(events1, i1);
      e2 = MArrayGet(events2, i1);
      fprintf(stderr, "    %-30s  <-->  %s\n",
	KheEventId(e1) != NULL ? KheEventId(e1) : "-",
	KheEventId(e2) != NULL ? KheEventId(e2) : "-");
    }
    fprintf(stderr, "]\n");
  }
  cleanup_and_return(true);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetRegular(KHE_MEET meet1, KHE_MEET meet2)                      */
/*                                                                           */
/*  Return true if these two meets are regular; that is, if they             */
/*  have the same duration and the same time domain.                         */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetRegular(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_TIME_GROUP tg1, tg2;
  if( KheMeetDuration(meet1) != KheMeetDuration(meet2) )
    return false;
  tg1 = KheMeetDomain(meet1);
  tg2 = KheMeetDomain(meet2);
  return tg1 != NULL && tg2 != NULL && KheTimeGroupEqual(tg1, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count)  */
/*                                                                           */
/*  Return true if node1 and node2 are regular, and in any case reorder      */
/*  the meets of both nodes so that the first *regular_count soln events     */
/*  have the same durations and current time domains.                        */
/*                                                                           */
/*****************************************************************************/

bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count)
{
  int i, j;  KHE_MEET meet1, meet2, tmp;  bool res;
  *regular_count = MArraySize(node1->meets);
  if( node1 == node2 )
    return true;
  for( i = 0;  i < *regular_count;  i++ )
  {
    meet1 = MArrayGet(node1->meets, i);
    for( j = i;  j < MArraySize(node2->meets);  j++ )
    {
      meet2 = MArrayGet(node2->meets, j);
      if( KheMeetRegular(meet1, meet2) )
	break;
    }
    if( j < MArraySize(node2->meets) )
    {
      /* meet1 and meet2 are regular */
      MArraySwap(node2->meets, i, j, tmp);
    }
    else
    {
      /* meet1 is not regular to any node of node2 */
      (*regular_count)--;
      MArraySwap(node1->meets, *regular_count, i, tmp);
      i--;
    }
  }
  res = MArraySize(node1->meets) == MArraySize(node2->meets) &&
    MArraySize(node1->meets) == *regular_count;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r)               */
/*                                                                           */
/*  Return the total duration of meets lying in node and its descendants     */
/*  that contain a preassignment of r.                                       */
/*                                                                           */
/*****************************************************************************/

int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r)
{
  int i, res;  KHE_MEET meet;  KHE_NODE child_node;  KHE_TASK task;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    if( KheMeetContainsResourcePreassignment(meet, r, &task) )
      res += KheMeetDuration(meet);
  MArrayForEach(node->child_nodes, &child_node, &i)
    res += KheNodeResourceDuration(child_node, r);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "zones"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone)                        */
/*                                                                           */
/*  Add zone to node, zone's node and node_index attributes as usual.        */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone)
{
  KheZoneSetNode(zone, node);
  KheZoneSetNodeIndex(zone, MArraySize(node->zones));
  MArrayAddLast(node->zones, zone);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone)                     */
/*                                                                           */
/*  Delete zone from node.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone)
{
  KHE_ZONE tmp;  int index;
  index = KheZoneNodeIndex(zone);
  MAssert(MArrayGet(node->zones, index) == zone,
    "KheNodeDeleteZone internal error");
  tmp = MArrayRemoveLast(node->zones);
  if( tmp != zone )
  {
    MArrayPut(node->zones, index, tmp);
    KheZoneSetNodeIndex(tmp, index);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeZoneCount(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the number of zones of node.                                      */
/*                                                                           */
/*****************************************************************************/

int KheNodeZoneCount(KHE_NODE node)
{
  return MArraySize(node->zones);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheNodeZone(KHE_NODE node, int i)                               */
/*                                                                           */
/*  Return the i'th zone of node.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheNodeZone(KHE_NODE node, int i)
{
  return MArrayGet(node->zones, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteZones(KHE_NODE node)                                   */
/*                                                                           */
/*  Delete the zones of node.                                                */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteZones(KHE_NODE node)
{
  while( KheNodeZoneCount(node) > 0 )
    KheZoneDelete(KheNodeZone(node, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeIrregularity(KHE_NODE node)                                   */
/*                                                                           */
/*  Return the irregularity of node.                                         */
/*                                                                           */
/*****************************************************************************/

int KheNodeIrregularity(KHE_NODE node)
{
  int i, j, res, offset, start, durn, target_offset;
  KHE_MEET meet, target_meet;  KHE_ZONE zone, zone2;

  /* if no parent, or parent has no zones or one zone, result is trivial */
  if( node->parent_node == NULL || KheNodeZoneCount(node->parent_node) <= 1 )
    return 0;

  /* use node->zones as temporary storage for zones of parent */
  start = MArraySize(node->zones);
  MArrayForEach(node->meets, &meet, &i)
  {
    target_meet = KheMeetAsst(meet); 
    if( target_meet != NULL )
    {
      target_offset = KheMeetAsstOffset(meet); 
      durn = KheMeetDuration(meet);
      for( offset = 0;  offset < durn;  offset++ )
      {
	zone = KheMeetOffsetZone(target_meet, target_offset + offset);
	for( j = MArraySize(node->zones) - 1;  j >= start;  j-- )
	{
	  zone2 = MArrayGet(node->zones, j);
	  if( zone2 == zone )
	    break;
	}
	if( j < start )
	  MArrayAddLast(node->zones, zone);
      }
    }
  }
  res = MArraySize(node->zones) - start - 1;
  if( res < 0 ) res = 0;
  MArrayDropFromEnd(node->zones, MArraySize(node->zones) - start);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetNamedMeet(KHE_MEET meet)                                 */
/*                                                                           */
/*  Return a named meet of maximum duration among meet and the meets         */
/*  assigned to it, directly or indirectly.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheMeetNamedMeet(KHE_MEET meet)
{
  KHE_MEET res, child_meet;  int i;

  /* try meet itself */
  if( KheMeetEvent(meet) != NULL && KheEventId(KheMeetEvent(meet)) != NULL )
    return meet;

  /* try the meets assigned to meet, directly or indirectly */
  res = NULL;
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetNamedMeet(KheMeetAssignedTo(meet, i));
    if( child_meet != NULL )
    {
      if( res == NULL || KheMeetDuration(child_meet) > KheMeetDuration(res) )
	res = child_meet;
    }
  }
  return res;
}

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheNamedMeet(KHE_NODE node)                                     */
/*                                                                           */
/*  Return a named meet of maximum duration among the meets of node, or      */
/*  NULL if none.                                                            */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheNodeNamedMeet(KHE_NODE node)
{
  KHE_MEET res, child_meet;  int i;
  res = NULL;
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    child_meet = KheMeetNamedMeet(KheNodeMeet(node, i));
    if( child_meet != NULL )
    {
      if( res == NULL || KheMeetDuration(child_meet) > KheMeetDuration(res) )
	res = child_meet;
    }
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDebug(KHE_NODE node, int indent, FILE *fp)                   */
/*                                                                           */
/*  Debug print of node onto fp with the given indent.                       */
/*                                                                           */
/*****************************************************************************/

void KheNodeDebug(KHE_NODE node, int verbosity, int indent, FILE *fp)
{
  KHE_MEET meet, child_meet;  int i, j, count;
  KHE_NODE child_node;  KHE_ZONE zone;
  if( verbosity >= 2 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Node %d (duration %d):\n", indent, "",
      node->soln_index, node->duration);
    if( verbosity >= 3 )
      MArrayForEach(node->zones, &zone, &i)
	KheZoneDebug(zone, verbosity, indent + 2, fp);
    MArrayForEach(node->meets, &meet, &i)
    {
      KheMeetDebug(meet, verbosity, indent + 2, fp);
      if( verbosity >= 4 )
	for( j = 0;  j < KheMeetAssignedToCount(meet);  j++ )
	{
	  child_meet = KheMeetAssignedTo(meet, j);
	  if( KheMeetNode(child_meet) == NULL )
	  {
	    fprintf(fp, "%*s<- ", indent + 2, "");
            KheMeetDebug(child_meet, 1, 0, fp);
	  }
	}
    }
    if( verbosity >= 4 )
      MArrayForEach(node->child_nodes, &child_node, &i)
	KheNodeDebug(child_node, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]", indent, "");
    count = MArraySize(node->child_nodes);
    if( count == 0 )
      fprintf(fp, "\n");
    else if( count == 1 )
      fprintf(fp, " (1 child)\n");
    else
      fprintf(fp, " (%d children)\n", count);
  }
  else
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "Node_%d", node->soln_index);
    meet = KheNodeNamedMeet(node);
    if( meet != NULL )
    {
      fprintf(fp, "_");
      KheMeetDebug(meet, 1, -1, fp);
    }
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "timetable printing"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_CELL - one cell in the timetable (may span)                          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_cell_rec {
  KHE_MEET		target_meet;		/* lies under meet           */
  int			target_offset;          /* at this offset            */
  int			span_count;		/* columns spanned           */
  ARRAY_KHE_MEET	meets;			/* the meets in this entry   */
} *KHE_CELL;

typedef MARRAY(KHE_CELL) ARRAY_KHE_CELL;


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW  - one row of cells                                              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_row_rec {
  KHE_LAYER		layer;			/* the layer represented     */
  int			width;			/* the initial no. of cells  */
  ARRAY_KHE_CELL	cells;			/* the cells                 */
} *KHE_ROW;

typedef MARRAY(KHE_ROW) ARRAY_KHE_ROW;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE - the table                                                    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_table_rec {
  KHE_NODE		node;			/* the node represented      */
  KHE_ROW		header_row;		/* the header row            */
  ARRAY_KHE_ROW		rows;			/* the rows                  */
} *KHE_TABLE;


/*****************************************************************************/
/*                                                                           */
/*  void KhePrint(char *str, bool in_cell, int span_count,                   */
/*    int cell_width, FILE *fp)                                              */
/*                                                                           */
/*  Print str onto fp, with a margin, taking care not to overrun.            */
/*                                                                           */
/*****************************************************************************/

static void KhePrint(char *str, bool in_cell, int span_count,
  int cell_width, FILE *fp)
{
  char buff[200];  int width;
  width = span_count * cell_width - 3;
  snprintf(buff, width, "%s", str);
  fprintf(fp, "%c %-*s ", in_cell ? '|' : ' ', width, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRule(bool major, int cell_width, FILE *fp)                  */
/*                                                                           */
/*  Print a rule of the given cell_width onto fp.                            */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRule(bool major, int cell_width, FILE *fp)
{
  int i;
  fprintf(fp, "+");
  for( i = 0;  i < cell_width - 1;  i++ )
    fprintf(fp, major ? "=" : "-");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRuleLine(int cells, bool major, int cell_width,             */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print a full-width rule, for this many cells of this width, onto fp      */
/*  with the given indent.                                                   */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRuleLine(int cells, bool major, int cell_width,
  int indent, FILE *fp)
{
  int i;
  fprintf(fp, "%*s", indent, "");
  for( i = 0;  i < cells;  i++ )
    KhePrintRule(major, cell_width, fp);
  fprintf(fp, "+\n");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CELL KheCellMake(KHE_MEET target_meet, int target_offset)            */
/*                                                                           */
/*  Make a new cell with these attributes.                                   */
/*                                                                           */
/*****************************************************************************/

static KHE_CELL KheCellMake(KHE_MEET target_meet, int target_offset)
{
  KHE_CELL res;
  MMake(res);
  res->target_meet = target_meet;
  res->target_offset = target_offset;
  res->span_count = 1;
  MArrayInit(res->meets);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellDelete(KHE_CELL cell)                                        */
/*                                                                           */
/*  Delete cell.                                                             */
/*                                                                           */
/*****************************************************************************/

static void KheCellDelete(KHE_CELL cell)
{
  MArrayFree(cell->meets);
  MFree(cell);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellCheckAndAcceptMeet(KHE_CELL cell, KHE_MEET meet)             */
/*                                                                           */
/*  Check whether cell can accept meet, and add it if so.                    */
/*                                                                           */
/*****************************************************************************/

static void KheCellCheckAndAcceptMeet(KHE_CELL cell, KHE_MEET meet)
{
  if( KheMeetAsst(meet) == cell->target_meet &&
      KheMeetAsstOffset(meet) <= cell->target_offset &&
      KheMeetAsstOffset(meet) + KheMeetDuration(meet) > cell->target_offset )
    MArrayAddLast(cell->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheCellMergeable(KHE_CELL cell1, KHE_CELL cell2)                    */
/*                                                                           */
/*  Return true if cell1 and cell2 are mergeable, because they contain       */
/*  the same meets.                                                          */
/*                                                                           */
/*****************************************************************************/

static bool KheCellMergeable(KHE_CELL cell1, KHE_CELL cell2)
{
  int i;
  if( MArraySize(cell1->meets) != MArraySize(cell2->meets) )
    return false;
  for( i = 0;  i < MArraySize(cell1->meets);  i++ )
    if( MArrayGet(cell1->meets, i) != MArrayGet(cell2->meets, i) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellMerge(KHE_CELL cell1, KHE_CELL cell2)                        */
/*                                                                           */
/*  Merge cell2 into cell1 and delete cell2.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheCellMerge(KHE_CELL cell1, KHE_CELL cell2)
{
  cell1->span_count += cell2->span_count;
  KheCellDelete(cell2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellPrint(KHE_CELL cell, int line, int cell_width, FILE *fp)     */
/*                                                                           */
/*  Print the line'th line of cell.  This will be a meet name if there       */
/*  is one, or an empty space otherwise.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheCellPrint(KHE_CELL cell, int line, int cell_width, FILE *fp)
{
  KHE_MEET meet;  char *str;  char buff[20];
  if( line < MArraySize(cell->meets) )
  {
    meet = MArrayGet(cell->meets, line);
    if( KheMeetEvent(meet) == NULL || KheEventId(KheMeetEvent(meet)) == NULL )
    {
      sprintf(buff, "#%d#", KheMeetSolnIndex(meet));
      str = buff;
    }
    else
      str = KheEventId(KheMeetEvent(meet));
  }
  else
    str = "";
  KhePrint(str, true, cell->span_count, cell_width, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheRowMake(KHE_LAYER layer)                                      */
/*                                                                           */
/*  Make a new row with these attributes.                                    */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheRowMake(KHE_LAYER layer)
{
  KHE_ROW res;
  MMake(res);
  res->layer = layer;
  res->width = 0;
  MArrayInit(res->cells);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowDelete(KHE_ROW row)                                           */
/*                                                                           */
/*  Delete row, including deleting its cells.                                */
/*                                                                           */
/*****************************************************************************/

static void KheRowDelete(KHE_ROW row)
{
  while( MArraySize(row->cells) > 0 )
    KheCellDelete(MArrayRemoveLast(row->cells));
  MArrayFree(row->cells);
  MFree(row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowAddMeet(KHE_ROW row, KHE_MEET meet)                           */
/*                                                                           */
/*  Add meet to the appropriate cells of row.                                */
/*                                                                           */
/*****************************************************************************/

static void KheRowAddMeet(KHE_ROW row, KHE_MEET meet)
{
  KHE_CELL cell;  int i;
  if( KheMeetAsst(meet) != NULL )
    MArrayForEach(row->cells, &cell, &i)
      KheCellCheckAndAcceptMeet(cell, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheRowBuild(KHE_LAYER layer)                                     */
/*                                                                           */
/*  Build a new row for layer.                                               */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheRowBuild(KHE_LAYER layer)
{
  KHE_ROW res;  KHE_NODE parent_node, child_node;
  KHE_MEET meet;  int offset, i, j;

  /* build the row with its cells, initially empty */
  res = KheRowMake(layer);
  parent_node = KheLayerParentNode(layer);
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    meet = KheNodeMeet(parent_node, i);
    for( offset = 0;  offset < KheMeetDuration(meet);  offset++ )
      MArrayAddLast(res->cells, KheCellMake(meet, offset));
  }
  res->width = MArraySize(res->cells);

  /* add the meets of the layer's child nodes to the row */
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
      KheRowAddMeet(res, KheNodeMeet(child_node, j));
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheHeaderRowBuild(KHE_NODE parent_node)                          */
/*                                                                           */
/*  Build a header row holding the meets of parent_node.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheHeaderRowBuild(KHE_NODE parent_node)
{
  KHE_ROW res;  KHE_MEET meet;  int i;  KHE_CELL cell;

  res = KheRowMake(NULL);
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    meet = KheNodeMeet(parent_node, i);
    cell = KheCellMake(NULL, 0);
    cell->span_count = KheMeetDuration(meet);
    MArrayAddLast(cell->meets, meet);
    MArrayAddLast(res->cells, cell);
  }
  res->width = KheNodeDuration(parent_node);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowMergeCells(KHE_ROW row)                                       */
/*                                                                           */
/*  Merge identical cells in row.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheRowMergeCells(KHE_ROW row)
{
  KHE_CELL cell, prev_cell;  int i;
  prev_cell = NULL;
  MArrayForEach(row->cells, &cell, &i)
  {
    if( prev_cell != NULL && KheCellMergeable(prev_cell, cell) )
    {
      MArrayRemove(row->cells, i);
      i--;
      KheCellMerge(prev_cell, cell);
    }
    else
      prev_cell = cell;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowPrint(KHE_ROW row, bool major, int cell_width, int indent,    */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Print row onto fp with the given indent.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheRowPrint(KHE_ROW row, bool major, int cell_width,
  int indent, FILE *fp)
{
  int max_lines, i, line;  KHE_CELL cell;  char *str;  KHE_RESOURCE r;

  /* find the maximum number of lines in any cell of the row */
  max_lines = 0;
  MArrayForEach(row->cells, &cell, &i)
    if( MArraySize(cell->meets) > max_lines )
      max_lines = MArraySize(cell->meets);

  /* print those lines followed by a rule */
  if( max_lines > 0 )
  {
    for( line = 0;  line < max_lines;  line++ )
    {
      /* print the first cell, which is the layer's resources if first */
      fprintf(fp, "%*s", indent, "");
      if( line == 0 && KheLayerResourceCount(row->layer) > 0 )
      {
	r = KheLayerResource(row->layer, 0);
	str = KheResourceId(r) == NULL ? "-" : KheResourceId(r);
      }
      else
	str = "";
      KhePrint(str, true, 1, cell_width, fp);

      /* print subsequent cells, or rather one line of each */
      MArrayForEach(row->cells, &cell, &i)
        KheCellPrint(cell, line, cell_width, fp);
      fprintf(fp, "|\n");
    }
    KhePrintRuleLine(row->width + 1, major, cell_width, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheHeaderRowPrint(KHE_ROW row, KHE_NODE node, int cell_width,       */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print row (a header row) onto fp with the given indent.                  */
/*                                                                           */
/*****************************************************************************/

static void KheHeaderRowPrint(KHE_ROW row, KHE_NODE node, int cell_width,
  int indent, FILE *fp)
{
  int i;  KHE_CELL cell;  char buff[20];

  /* print a rule above the row */
  KhePrintRuleLine(row->width + 1, true, cell_width, indent, fp);

  /* print the first cell, which is node's index */
  fprintf(fp, "%*s", indent, "");
  sprintf(buff, "Node %d", KheNodeSolnIndex(node));
  KhePrint(buff, true, 1, cell_width, fp);

  /* print subsequent cells */
  MArrayForEach(row->cells, &cell, &i)
    KheCellPrint(cell, 0, cell_width, fp);
  fprintf(fp, "|\n");

  /* print a rule below the row */
  KhePrintRuleLine(row->width + 1, true, cell_width, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE KheTableMake(KHE_NODE node)                                    */
/*                                                                           */
/*  Make a new, empty table for node.                                        */
/*                                                                           */
/*****************************************************************************/

static KHE_TABLE KheTableMake(KHE_NODE node)
{
  KHE_TABLE res;
  MMake(res);
  res->node = node;
  res->header_row = NULL;
  MArrayInit(res->rows);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTableDelete(KHE_TABLE table)                                     */
/*                                                                           */
/*  Delete table, including deleting its rows.                               */
/*                                                                           */
/*****************************************************************************/

static void KheTableDelete(KHE_TABLE table)
{
  while( MArraySize(table->rows) > 0 )
    KheRowDelete(MArrayRemoveLast(table->rows));
  MArrayFree(table->rows);
  MFree(table);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE KheTableBuild(KHE_NODE node)                                   */
/*                                                                           */
/*  Assuming that node has layers, build a table for it.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_TABLE KheTableBuild(KHE_NODE node)
{
  KHE_TABLE res;  int i;
  res = KheTableMake(node);
  res->header_row = KheHeaderRowBuild(node);
  for( i = 0;  i < KheNodeChildLayerCount(node);  i++ )
    MArrayAddLast(res->rows, KheRowBuild(KheNodeChildLayer(node, i)));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTableMergeCells(KHE_TABLE table)                                 */
/*                                                                           */
/*  Merge adjacent identical cells in table.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTableMergeCells(KHE_TABLE table)
{
  KHE_ROW row;  int i;
  MArrayForEach(table->rows, &row, &i)
    KheRowMergeCells(row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTablePrint(KHE_TABLE table, int cell_width,                      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print table onto fp with the given indent.                               */
/*                                                                           */
/*****************************************************************************/

static void KheTablePrint(KHE_TABLE table, int cell_width,
  int indent, FILE *fp)
{
  KHE_ROW row;  int i;

  /* print header row */
  KheHeaderRowPrint(table->header_row, table->node, cell_width, indent, fp);

  /* print ordinary rows */
  MArrayForEach(table->rows, &row, &i)
    KheRowPrint(row, i == MArraySize(table->rows) - 1, cell_width, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodePrintTimetable(KHE_NODE node, int cell_width,                */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print the timetable of node.                                             */
/*                                                                           */
/*****************************************************************************/

void KheNodePrintTimetable(KHE_NODE node, int cell_width,
  int indent, FILE *fp)
{
  bool make_layers;  KHE_TABLE table;

  /* make layers if not already present */
  make_layers = (KheNodeChildLayerCount(node) == 0);
  if( make_layers )
    KheNodeChildLayersMake(node);

  /* build the table, merge identical cells, print it, and delete it */
  table = KheTableBuild(node);
  KheTableMergeCells(table);
  KheTablePrint(table, cell_width, indent, fp);
  KheTableDelete(table);

  /* remove layers if we added them */
  if( make_layers )
    KheNodeChildLayersDelete(node);
}
