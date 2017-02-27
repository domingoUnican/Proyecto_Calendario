
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
/*  FILE:         khe_order_events_constraint.c                              */
/*  DESCRIPTION:  An order events constraint                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_PAIR - a pair of events (private)                              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_event_pair_rec {
  KHE_EVENT			first_event;		/* first event       */
  KHE_EVENT			second_event;		/* second event      */
  int				min_separation;		/* min separation    */
  int				max_separation;		/* max separation    */
} *KHE_EVENT_PAIR;

typedef MARRAY(KHE_EVENT_PAIR) ARRAY_KHE_EVENT_PAIR;


/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDER_EVENTS_CONSTRAINT - an order events constraint                 */
/*                                                                           */
/*****************************************************************************/

struct khe_order_events_constraint_rec {
  INHERIT_CONSTRAINT
  ARRAY_KHE_EVENT_PAIR		event_pairs;		/* event pairs       */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheOrderEventsConstraintMake(KHE_INSTANCE ins, char *id,            */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    KHE_ORDER_EVENTS_CONSTRAINT *c)                                        */
/*                                                                           */
/*  Make a new order events constraint, add it to ins, and return it.        */
/*                                                                           */
/*****************************************************************************/

bool KheOrderEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_ORDER_EVENTS_CONSTRAINT *c)
{
  KHE_ORDER_EVENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheOrderEventsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_ORDER_EVENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->event_pairs);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrderEventsConstraintAddEventPair(KHE_ORDER_EVENTS_CONSTRAINT c, */
/*    KHE_EVENT first_event, KHE_EVENT second_event, int min_separation,     */
/*    int max_separation)                                                    */
/*                                                                           */
/*  Add an event pair with these attribures to c.                            */
/*                                                                           */
/*****************************************************************************/

void KheOrderEventsConstraintAddEventPair(KHE_ORDER_EVENTS_CONSTRAINT c,
  KHE_EVENT first_event, KHE_EVENT second_event, int min_separation,
  int max_separation)
{
  KHE_EVENT_PAIR ep;
  MMake(ep);
  ep->first_event = first_event;
  ep->second_event = second_event;
  ep->min_separation = min_separation;
  ep->max_separation = max_separation;
  MArrayAddLast(c->event_pairs, ep);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrderEventsConstraintEventPairCount(KHE_ORDER_EVENTS_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheOrderEventsConstraintEventPairCount(KHE_ORDER_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_pairs);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrderEventsConstraintAppliesToCount(KHE_ORDER_EVENTS_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of points of application of c (same as previous).      */
/*                                                                           */
/*****************************************************************************/

int KheOrderEventsConstraintAppliesToCount(KHE_ORDER_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_pairs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheOrderEventsConstraintFirstEvent(                            */
/*    KHE_ORDER_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the first event of the ith point of application of c.             */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheOrderEventsConstraintFirstEvent(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i)
{
  KHE_EVENT_PAIR ep;
  ep = MArrayGet(c->event_pairs, i);
  return ep->first_event;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheOrderEventsConstraintSecondEvent(                           */
/*    KHE_ORDER_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the second event of the ith point of application of c.            */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheOrderEventsConstraintSecondEvent(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i)
{
  KHE_EVENT_PAIR ep;
  ep = MArrayGet(c->event_pairs, i);
  return ep->second_event;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrderEventsConstraintMinSeparation(                               */
/*    KHE_ORDER_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the min separation of the ith point of application of c.          */
/*                                                                           */
/*****************************************************************************/

int KheOrderEventsConstraintMinSeparation(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i)
{
  KHE_EVENT_PAIR ep;
  ep = MArrayGet(c->event_pairs, i);
  return ep->min_separation;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrderEventsConstraintMaxSeparation(                               */
/*    KHE_ORDER_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the max separation of the ith point of application of c.          */
/*                                                                           */
/*****************************************************************************/

int KheOrderEventsConstraintMaxSeparation(
  KHE_ORDER_EVENTS_CONSTRAINT c, int i)
{
  KHE_EVENT_PAIR ep;
  ep = MArrayGet(c->event_pairs, i);
  return ep->max_separation;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrderEventsConstraintFinalize(KHE_ORDER_EVENTS_CONSTRAINT c)     */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheOrderEventsConstraintFinalize(KHE_ORDER_EVENTS_CONSTRAINT c)
{
  /* nothing to do in this case */
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrderEventsConstraintDensityCount(KHE_ORDER_EVENTS_CONSTRAINT c)  */
/*                                                                           */
/*  Return the density count of c.                                           */
/*                                                                           */
/*****************************************************************************/

int KheOrderEventsConstraintDensityCount(KHE_ORDER_EVENTS_CONSTRAINT c)
{
  return KheOrderEventsConstraintAppliesToCount(c);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheOrderEventsConstraintMakeFromKml(KML_ELT cons_elt,               */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make an order events constraint based on cons_elt and add it to ins.     */
/*                                                                           */
/*****************************************************************************/

bool KheOrderEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name, *ref;  bool reqd;  int wt, i, min_sep, max_sep;
  KML_ELT elt, event_pairs_elt, event_pair_elt;
  KHE_COST_FUNCTION cf;  KHE_ORDER_EVENTS_CONSTRAINT res;  KHE_EVENT e1, e2;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* build and insert the constraint object */
  if( !KheOrderEventsConstraintMake(ins, id, name, reqd, wt, cf, &res) )
    return KmlError(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<OrderEventsConstraint> Id \"%s\" used previously", id);

  /* add the event pairs */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": EventPairs", ke) )
    return false;
  event_pairs_elt = KmlChild(elt, 0);
  if( !KmlCheck(event_pairs_elt, ": *EventPair", ke) )
    return false;
  for( i = 0;  i < KmlChildCount(event_pairs_elt);  i++ )
  {
    /* get and check the ith event pair */
    event_pair_elt = KmlChild(event_pairs_elt, i);
    if( !KmlCheck(event_pair_elt,
	": FirstEvent SecondEvent +#MinSeparation +#MaxSeparation", ke) )
      return false;

    /* get the first event */
    elt = KmlChild(event_pair_elt, 0);
    if( !KmlCheck(elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(elt, 0);
    if( !KheInstanceRetrieveEvent(ins, ref, &e1) )
      return KmlError(ke, KmlLineNum(elt), KmlColNum(elt),
	"<FirstEvent> Reference \"%s\" unknown", ref);

    /* get the second event */
    elt = KmlChild(event_pair_elt, 1);
    if( !KmlCheck(elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(elt, 0);
    if( !KheInstanceRetrieveEvent(ins, ref, &e2) )
      return KmlError(ke, KmlLineNum(elt), KmlColNum(elt),
	"<SecondEvent> Reference \"%s\" unknown", ref);

    /* get the min separation */
    if( KmlContainsChild(event_pair_elt, "MinSeparation", &elt) )
      sscanf(KmlText(elt), "%d", &min_sep);
    else
      min_sep = 0;

    /* get the max separation */
    if( KmlContainsChild(event_pair_elt, "MaxSeparation", &elt) )
      sscanf(KmlText(elt), "%d", &max_sep);
    else
      max_sep = INT_MAX;

    if( min_sep > max_sep )
      return KmlError(ke, KmlLineNum(elt), KmlColNum(elt),
	"in <EventPair>, <MinSeparation> exceeds <MaxSeparation>", ref);

    /* add the ith event pair */
    KheOrderEventsConstraintAddEventPair(res, e1, e2, min_sep, max_sep);
  }
  if( KheOrderEventsConstraintAppliesToCount(res) == 0 )
    return KmlError(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<OrderEventsConstraint> applies to 0 pairs of events");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrderEventsConstraintWrite(KHE_ORDER_EVENTS_CONSTRAINT c,        */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheOrderEventsConstraintWrite(KHE_ORDER_EVENTS_CONSTRAINT c, KML_FILE kf)
{
  KHE_EVENT_PAIR ep;  int i;
  KmlBegin(kf, "OrderEventsConstraint");
  MAssert(c->id != NULL,
    "KheArchiveWrite: Id missing in OrderEventsConstraint");
  KmlAttribute(kf, "Id", c->id);
  KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf);
  KmlBegin(kf, "AppliesTo");
  KmlBegin(kf, "EventPairs");
  MArrayForEach(c->event_pairs, &ep, &i)
  {
    KmlBegin(kf, "EventPair");
    MAssert(KheEventId(ep->first_event) != NULL, "KheArchiveWrite:  Id "
      " missing in FirstEvent referenced from OrderEventsConstraint %s", c->id);
    KmlEltAttribute(kf, "FirstEvent", "Reference",KheEventId(ep->first_event));
    MAssert(KheEventId(ep->second_event) != NULL, "KheArchiveWrite:  Id "
      " missing in SecondEvent referenced from OrderEventsConstraint %s",c->id);
    KmlEltAttribute(kf, "SecondEvent","Reference",KheEventId(ep->second_event));
    if( ep->min_separation != 0 )
      KmlEltFmtText(kf, "MinSeparation", "%d", ep->min_separation);
    if( ep->max_separation != INT_MAX )
      KmlEltFmtText(kf, "MaxSeparation", "%d", ep->max_separation);
    KmlEnd(kf, "EventPair");
  }
  KmlEnd(kf, "EventPairs");
  KmlEnd(kf, "AppliesTo");
  KmlEnd(kf, "OrderEventsConstraint");
}
