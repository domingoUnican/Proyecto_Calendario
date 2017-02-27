
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
/*  FILE:         khe_part_graph.c                                           */
/*  DESCRIPTION:  Generic graph partitioning module (for two-colouring)      */
/*                                                                           */
/*****************************************************************************/
#include "khe_part_graph.h"
#include "m.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_PART - one part of a component                            */
/*                                                                           */
/*****************************************************************************/

struct khe_part_graph_part_rec {
  KHE_PART_GRAPH_COMPONENT	component;	/* enclosing component       */
  ARRAY_VOIDP			nodes;		/* nodes of this part        */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_COMPONENT - one connected component of a graph            */
/*                                                                           */
/*****************************************************************************/

struct khe_part_graph_component_rec {
  KHE_PART_GRAPH		graph;		/* enclosing graph           */
  ARRAY_VOIDP			nodes;		/* nodes of this component   */
  bool				parts_done;	/* true after solving        */
  KHE_PART_GRAPH_PART		parts[2];	/* parts of this component   */
};

typedef MARRAY(KHE_PART_GRAPH_COMPONENT) ARRAY_KHE_PART_GRAPH_COMPONENT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH - a graph to be partitioned                               */
/*                                                                           */
/*****************************************************************************/

struct khe_part_graph_graph_rec {
  KHE_PART_GRAPH_REL_FN		rel_fn;		/* node relationship fn      */
  ARRAY_VOIDP			nodes;		/* nodes of this graph       */
  bool				components_done; /* true after solving       */
  ARRAY_KHE_PART_GRAPH_COMPONENT components;	/* the components            */
};


/*****************************************************************************/
/*                                                                           */
/*  Parts                                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_PART KhePartGraphPartMake(KHE_PART_GRAPH_COMPONENT comp)  */
/*                                                                           */
/*  Make a new part object with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

KHE_PART_GRAPH_PART KhePartGraphPartMake(KHE_PART_GRAPH_COMPONENT comp)
{
  KHE_PART_GRAPH_PART res;
  MMake(res);
  res->component = comp;
  MArrayInit(res->nodes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphPartDelete(KHE_PART_GRAPH_PART part)                    */
/*                                                                           */
/*  Delete part.                                                             */
/*                                                                           */
/*****************************************************************************/

void KhePartGraphPartDelete(KHE_PART_GRAPH_PART part)
{
  MArrayFree(part->nodes);
  MFree(part);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePartGraphPartRelateNode(KHE_PART_GRAPH_PART part, void *node,    */
/*    KHE_PART_GRAPH_REL *rel)                                               */
/*                                                                           */
/*  Work out the relation of part to node.                                   */
/*                                                                           */
/*  If there is a consistent relation, set *rel to it and return true.       */
/*  If there is an inconsistent relation, return false.                      */
/*                                                                           */
/*****************************************************************************/

bool KhePartGraphPartRelateNode(KHE_PART_GRAPH_PART part, void *node,
  KHE_PART_GRAPH_REL *rel)
{
  void *node2;  int i;
  *rel = KHE_PART_GRAPH_UNRELATED;
  MArrayForEach(part->nodes, &node2, &i)
    switch( part->component->graph->rel_fn(node, node2) )
    {
      case KHE_PART_GRAPH_UNRELATED:

	/* as we were */
	break;

      case KHE_PART_GRAPH_SAME:

	if( *rel == KHE_PART_GRAPH_DIFFERENT )
	  return false;
	*rel = KHE_PART_GRAPH_SAME;
	break;

      case KHE_PART_GRAPH_DIFFERENT:

	if( *rel == KHE_PART_GRAPH_SAME )
	  return false;
        *rel = KHE_PART_GRAPH_DIFFERENT;
	break;

      default:

	MAssert(false, "KhePartGraphPartRelateNode: unknown rel_fn result");
	break;
    }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphPartAddNode(KHE_PART_GRAPH_PART part, void *node)       */
/*                                                                           */
/*  Add node to part.                                                        */
/*                                                                           */
/*****************************************************************************/

static void KhePartGraphPartAddNode(KHE_PART_GRAPH_PART part, void *node)
{
  MArrayAddLast(part->nodes, node);
}


/*****************************************************************************/
/*                                                                           */
/*  Components                                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_COMPONENT KhePartGraphComponentMake(                      */
/*    KHE_PART_GRAPH graph)                                                  */
/*                                                                           */
/*  Make a new, empty component with these attributes.                       */
/*                                                                           */
/*****************************************************************************/

static KHE_PART_GRAPH_COMPONENT KhePartGraphComponentMake(
  KHE_PART_GRAPH graph)
{
  KHE_PART_GRAPH_COMPONENT res;
  MMake(res);
  res->graph = graph;
  MArrayInit(res->nodes);
  res->parts_done = false;
  res->parts[0] = res->parts[1] = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphComponentDelete(KHE_PART_GRAPH_COMPONENT comp)          */
/*                                                                           */
/*  Delete comp.                                                             */
/*                                                                           */
/*****************************************************************************/

static void KhePartGraphComponentDelete(KHE_PART_GRAPH_COMPONENT comp)
{
  MArrayFree(comp->nodes);
  MFree(comp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH KhePartGraphComponentGraph(KHE_PART_GRAPH_COMPONENT comp) */
/*                                                                           */
/*  Return the graph that comp is a component of.                            */
/*                                                                           */
/*****************************************************************************/

KHE_PART_GRAPH KhePartGraphComponentGraph(KHE_PART_GRAPH_COMPONENT comp)
{
  return comp->graph;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePartGraphComponentAcceptsNode(KHE_PART_GRAPH_COMPONENT comp,     */
/*    void *node)                                                            */
/*                                                                           */
/*  Return true if comp accepts node.                                        */
/*                                                                           */
/*****************************************************************************/

static bool KhePartGraphComponentAcceptsNode(KHE_PART_GRAPH_COMPONENT comp,
  void *node)
{
  void *node2;  int i;
  MArrayForEach(comp->nodes, &node2, &i)
    if( comp->graph->rel_fn(node, node2) != KHE_PART_GRAPH_UNRELATED )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphComponentAddNode(KHE_PART_GRAPH_COMPONENT comp,         */
/*    void *node)                                                            */
/*                                                                           */
/*  Add node to comp.                                                        */
/*                                                                           */
/*****************************************************************************/

static void KhePartGraphComponentAddNode(KHE_PART_GRAPH_COMPONENT comp,
  void *node)
{
  MArrayAddLast(comp->nodes, node);
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePartGraphComponentNodeCount(KHE_PART_GRAPH_COMPONENT comp)        */
/*                                                                           */
/*  Return the number of nodes in comp.                                      */
/*                                                                           */
/*****************************************************************************/

int KhePartGraphComponentNodeCount(KHE_PART_GRAPH_COMPONENT comp)
{
  return MArraySize(comp->nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KhePartGraphComponentNode(KHE_PART_GRAPH_COMPONENT comp, int i)    */
/*                                                                           */
/*  Return the i'th node of comp.                                            */
/*                                                                           */
/*****************************************************************************/

void *KhePartGraphComponentNode(KHE_PART_GRAPH_COMPONENT comp, int i)
{
  return MArrayGet(comp->nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphComponentMerge(KHE_PART_GRAPH_COMPONENT comp1,          */
/*    KHE_PART_GRAPH_COMPONENT comp2)                                        */
/*                                                                           */
/*  Merge comp2 into comp1 and delete comp2.                                 */
/*                                                                           */
/*****************************************************************************/

static void KhePartGraphComponentMerge(KHE_PART_GRAPH_COMPONENT comp1,
  KHE_PART_GRAPH_COMPONENT comp2)
{
  int i;
  MArrayAppend(comp1->nodes, comp2->nodes, i);
  KhePartGraphComponentDelete(comp2);
}


/*****************************************************************************/
/*                                                                           */
/*  Graphs                                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH KhePartGraphMake(KHE_PART_GRAPH_REL_FN rel_fn)            */
/*                                                                           */
/*  Make a new partitioned graph object.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_PART_GRAPH KhePartGraphMake(KHE_PART_GRAPH_REL_FN rel_fn)
{
  KHE_PART_GRAPH res;
  MMake(res);
  res->rel_fn = rel_fn;
  MArrayInit(res->nodes);
  res->components_done = false;
  MArrayInit(res->components);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphDelete(KHE_PART_GRAPH graph)                            */
/*                                                                           */
/*  Delete graph, including deleting its components.                         */
/*                                                                           */
/*****************************************************************************/

void KhePartGraphDelete(KHE_PART_GRAPH graph)
{
  MArrayFree(graph->nodes);
  while( MArraySize(graph->components) > 0 )
    KhePartGraphComponentDelete(MArrayRemoveLast(graph->components));
  MArrayFree(graph->components);
  MFree(graph);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphAddNode(KHE_PART_GRAPH graph, void *node)               */
/*                                                                           */
/*  Add node to graph.                                                       */
/*                                                                           */
/*****************************************************************************/

void KhePartGraphAddNode(KHE_PART_GRAPH graph, void *node)
{
  int pos;
  MAssert(!graph->components_done, "KhePartGraphAddNode called after solving");
  MAssert(!MArrayContains(graph->nodes, node, &pos),
    "KhePartGraphAddNode: node already present");
  MArrayAddLast(graph->nodes, node);
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePartGraphNodeCount(KHE_PART_GRAPH graph)                          */
/*                                                                           */
/*  Return the number of nodes in graph.                                     */
/*                                                                           */
/*****************************************************************************/

int KhePartGraphNodeCount(KHE_PART_GRAPH graph)
{
  return MArraySize(graph->nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KhePartGraphNode(KHE_PART_GRAPH graph, int i)                      */
/*                                                                           */
/*  Return the i'th node of graph.                                           */
/*                                                                           */
/*****************************************************************************/

void *KhePartGraphNode(KHE_PART_GRAPH graph, int i)
{
  return MArrayGet(graph->nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphComponentSolve(KHE_PART_GRAPH_COMPONENT comp)           */
/*                                                                           */
/*  Solve one component, that is, find its parts.  The first part,           */
/*  comp->parts[0], must contain at least one node.                          */
/*                                                                           */
/*****************************************************************************/

void KhePartGraphComponentFindParts(KHE_PART_GRAPH_COMPONENT comp)
{
  int i, still_to_do;  bool progressing;  void *node, *tmp;
  KHE_PART_GRAPH_REL rel0, rel1;

  /* make sure not already done */
  MAssert(!comp->parts_done,
    "KhePartGraphComponentFindParts called twice on same component");
  comp->parts_done = true;

  /* make parts and place the last node into them */
  comp->parts[0] = KhePartGraphPartMake(comp);
  comp->parts[1] = KhePartGraphPartMake(comp);
  MAssert(MArraySize(comp->nodes) > 0,
    "KhePartGraphComponentSolve internal error");
  KhePartGraphPartAddNode(comp->parts[0], MArrayLast(comp->nodes));

  /* still_to_do is the index of the first node in comp->nodes which */
  /* is in a part; it is also the number of nodes in comp->nodes which */
  /* have not yet been placed in a part */
  still_to_do = MArraySize(comp->nodes) - 1;
  while( still_to_do > 0 )
  {
    progressing = false;
    for( i = 0;  i < still_to_do;  i++ )
    {
      node = MArrayGet(comp->nodes, i);
      if( !KhePartGraphPartRelateNode(comp->parts[0], node, &rel0) ||
	  !KhePartGraphPartRelateNode(comp->parts[1], node, &rel1) ||
	  (rel0 == rel1 && rel0 != KHE_PART_GRAPH_UNRELATED) )
      {
	/* contradiction, reset and exit */
	KhePartGraphPartDelete(comp->parts[0]);
	KhePartGraphPartDelete(comp->parts[1]);
	comp->parts[0] = comp->parts[1] = NULL;
	return;
      }
      else if( rel0==KHE_PART_GRAPH_SAME || rel1==KHE_PART_GRAPH_DIFFERENT )
      {
	KhePartGraphPartAddNode(comp->parts[0], node);
	still_to_do--;
	MArraySwap(comp->nodes, i, still_to_do, tmp);
	progressing = true;
      }
      else if( rel0==KHE_PART_GRAPH_DIFFERENT || rel1==KHE_PART_GRAPH_SAME )
      {
	KhePartGraphPartAddNode(comp->parts[1], node);
	still_to_do--;
	MArraySwap(comp->nodes, i, still_to_do, tmp);
	progressing = true;
      }
    }
    MAssert(progressing, "KhePartGraphComponentHasParts internal error");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartGraphFindConnectedComponents(KHE_PART_GRAPH graph)           */
/*                                                                           */
/*  Find the connected components of graph.                                  */
/*                                                                           */
/*****************************************************************************/

void KhePartGraphFindConnectedComponents(KHE_PART_GRAPH graph)
{
  int i, j;  KHE_PART_GRAPH_COMPONENT comp, res_comp;  void *node;

  /* build the components */
  MAssert(!graph->components_done,
    "KhePartGraphFindConnectedComponents called twice");
  MArrayForEach(graph->nodes, &node, &i)
  {
    res_comp = NULL;
    MArrayForEach(graph->components, &comp, &j)
      if( KhePartGraphComponentAcceptsNode(comp, node) )
      {
	/* make sure res_c is the component that will take node */
	if( res_comp == NULL )
	  res_comp = comp;
	else
	{
          KhePartGraphComponentMerge(res_comp, comp);
	  MArrayRemove(graph->components, j);
	  j--;
	}
      }
    if( res_comp == NULL )
    {
      res_comp = KhePartGraphComponentMake(graph);
      MArrayAddLast(graph->components, res_comp);
    }
    KhePartGraphComponentAddNode(res_comp, node);
  }
  graph->components_done = true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePartGraphComponentCount(KHE_PART_GRAPH graph)                     */
/*                                                                           */
/*  Return the number of components in graph.                                */
/*                                                                           */
/*****************************************************************************/

int KhePartGraphComponentCount(KHE_PART_GRAPH graph)
{
  MAssert(graph->components_done, "KhePartGraphComponentCount called "
    "before KhePartGraphFindConnectedComponents");
  return MArraySize(graph->components);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_COMPONENT KhePartGraphComponent(KHE_PART_GRAPH graph,     */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th component of graph.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_PART_GRAPH_COMPONENT KhePartGraphComponent(KHE_PART_GRAPH graph, int i)
{
  MAssert(graph->components_done, "KhePartGraphComponent called "
    "before KhePartGraphFindConnectedComponents");
  return MArrayGet(graph->components, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePartGraphComponentParts(KHE_PART_GRAPH_COMPONENT comp,           */
/*    KHE_PART_GRAPH_PART *part1, KHE_PART_GRAPH_PART *part2)                */
/*                                                                           */
/*  If KhePartGraphComponentFindParts succeeded, return true and set         */
/*  *part1 and *part2 to the two parts.  Otherwise return false.             */
/*                                                                           */
/*****************************************************************************/

bool KhePartGraphComponentParts(KHE_PART_GRAPH_COMPONENT comp,
  KHE_PART_GRAPH_PART *part1, KHE_PART_GRAPH_PART *part2)
{
  MAssert(comp->parts_done, "KhePartGraphComponentHasParts "
    "called before KhePartGraphComponentFindParts");
  *part1 = comp->parts[0];
  *part2 = comp->parts[1];
  return comp->parts[0] != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PART_GRAPH_COMPONENT KhePartGraphPartComponent(                      */
/*    KHE_PART_GRAPH_PART part)                                              */
/*                                                                           */
/*  Return the component that part is a part of.                             */
/*                                                                           */
/*****************************************************************************/

KHE_PART_GRAPH_COMPONENT KhePartGraphPartComponent(KHE_PART_GRAPH_PART part)
{
  return part->component;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePartGraphPartNodeCount(KHE_PART_GRAPH_PART part)                  */
/*                                                                           */
/*  Return the number of nodes in part.                                      */
/*                                                                           */
/*****************************************************************************/

int KhePartGraphPartNodeCount(KHE_PART_GRAPH_PART part)
{
  return MArraySize(part->nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KhePartGraphPartNode(KHE_PART_GRAPH_PART part, int i)              */
/*                                                                           */
/*  Return the i'th node of part.                                            */
/*                                                                           */
/*****************************************************************************/

void *KhePartGraphPartNode(KHE_PART_GRAPH_PART part, int i)
{
  return MArrayGet(part->nodes, i);
}
