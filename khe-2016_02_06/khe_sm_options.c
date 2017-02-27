
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
/*  FILE:         khe_sm_options.c                                           */
/*  DESCRIPTION:  Options for solvers                                        */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS                                                              */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_EJECTOR) ARRAY_KHE_EJECTOR;

struct khe_options_rec {

  /* options for general solving */
  bool			diversify;			/* diversify soln    */
  bool			monitor_evenness;		/* monitor evenness  */
  bool			time_assignment_only;		/* time asst only    */

  /* options for structural solvers */
  KHE_TIME_EQUIV	structural_time_equiv;		/* time-equivalence  */
  KHE_SPLIT_ANALYSER	structural_split_analyser;	/* split analyser    */

  /* options for time solvers (other than ejection chain repair) */
  bool			time_cluster_meet_domains;	/* cluster meets     */
  bool			time_tighten_domains;		/* domain tightening */
  bool			time_node_repair;		/* node repair       */
  bool			time_node_regularity;		/* node regularity   */
  bool			time_layer_swap;		/* swap layers       */
  bool			time_layer_repair;		/* repair layers     */
  bool			time_layer_repair_backoff;	/* exp. backoff      */
  bool			time_layer_repair_long;		/* long repair       */
  KHE_KEMPE_STATS	time_kempe_stats;		/* Kempe move stats  */

  /* options for resource solvers (other than ejection chain repair) */
  bool			resource_invariant;		/* preserve invt     */
  bool			resource_rematch;		/* resource rematch  */
  KHE_OPTIONS_RESOURCE_PAIR resource_pair;		/* resource pairs    */
  int			resource_pair_calls;		/* r. pair calls     */
  int			resource_pair_successes;	/* r. pair successes */
  int			resource_pair_truncs;		/* r. pair truncs    */

  /* options for ejectors and ejection chain repair generally */
  ARRAY_KHE_EJECTOR	ejectors;			/* ejectors          */
  char			*ejector_schedules_string;	/* schedules         */
  bool			ejector_promote_defects;	/* promote defects   */
  bool			ejector_fresh_visits;		/* fresh visits      */
  /* bool		ejector_suppress_recent; */	/* suppress recent   */
  bool			ejector_repair_times;		/* repair times      */
  bool			ejector_vizier_node;		/* vizier nodes      */
  bool			ejector_nodes_before_meets;	/* nodes before meets*/
  bool			ejector_use_fuzzy_moves;	/* fuzzy moves       */
  bool			ejector_use_split_moves;	/* split moves       */
  KHE_OPTIONS_KEMPE	ejector_use_kempe_moves;	/* Kempe moves       */
  bool			ejector_ejecting_not_basic;	/* ejecting moves    */
  KHE_NODE		ejector_limit_node;		/* node limit        */
  bool			ejector_repair_resources;	/* repair resources  */
  int			ejector_limit_defects;		/* limit defects rpd */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS KheOptionsMake(void)                                         */
/*                                                                           */
/*  Make a new options object with default values for all options.           */
/*                                                                           */
/*  NB although some of these values are never changed, others are           */
/*  changed by solvers on the fly.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_OPTIONS KheOptionsMake(void)
{
  KHE_OPTIONS res;
  MMake(res);

  /* options for general solving */
  res->diversify = true;
  res->monitor_evenness = false;
  res->time_assignment_only = false;

  /* options for structural solvers */
  res->structural_split_analyser = KheSplitAnalyserMake();
  res->structural_time_equiv = KheTimeEquivMake();

  /* options for time solvers */
  res->time_cluster_meet_domains = true;
  res->time_tighten_domains = true;
  res->time_node_repair = true;
  res->time_node_regularity = true;
  res->time_layer_swap = false;
  res->time_layer_repair = true;
  res->time_layer_repair_backoff = false;
  res->time_layer_repair_long = false;
  res->time_kempe_stats = KheKempeStatsMake();

  /* options for resource solvers */
  res->resource_invariant = false;
  res->resource_rematch = true;
  res->resource_pair = KHE_OPTIONS_RESOURCE_PAIR_SPLITS;
  res->resource_pair_calls = 0;
  res->resource_pair_successes = 0;
  res->resource_pair_truncs = 0;

  /* options for ejectors and ejection chains */
  MArrayInit(res->ejectors);
  res->ejector_schedules_string = "1+,u-";
  res->ejector_promote_defects = true;
  res->ejector_fresh_visits = true;
  /* res->ejector_suppress_recent = false; */
  res->ejector_repair_times = true;
  res->ejector_vizier_node = false;
  res->ejector_nodes_before_meets = false;
  res->ejector_use_kempe_moves = KHE_OPTIONS_KEMPE_AUTO;
  res->ejector_use_fuzzy_moves = false;
  res->ejector_use_split_moves = false;
  res->ejector_ejecting_not_basic = true;
  res->ejector_limit_node = NULL;
  res->ejector_repair_resources = true;
  res->ejector_limit_defects = INT_MAX;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOptionsDelete(KHE_OPTIONS options)                               */
/*                                                                           */
/*  Delete options.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheOptionsDelete(KHE_OPTIONS options)
{
  MArrayFree(options->ejectors);
  KheSplitAnalyserDelete(options->structural_split_analyser);
  KheTimeEquivDelete(options->structural_time_equiv);
  KheKempeStatsDelete(options->time_kempe_stats);
  MFree(options);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorsCopyUnique(KHE_OPTIONS options, KHE_OPTIONS res)         */
/*                                                                           */
/*  Make a copy of each ejector in options and place it in res.  But if      */
/*  an ejector appears several times in options->ejectors, make sure that    */
/*  a single copy of that ejector appears throughout res->ejectors.          */
/*                                                                           */
/*****************************************************************************/

void KheEjectorsCopyUnique(KHE_OPTIONS options, KHE_OPTIONS res)
{
  KHE_EJECTOR ej, ej2;  int i, j;

  /* place a NULL value into every slot in res */
  MArrayInit(res->ejectors);
  MArrayForEach(options->ejectors, &ej, &i)
    MArrayAddLast(res->ejectors, NULL);

  /* for each non-NULL slot in options, if it is not already done in */
  /* res, make a copy of it and add it everywhere the original is */
  MArrayForEach(options->ejectors, &ej, &i)
    if( ej != NULL && MArrayGet(res->ejectors, i) == NULL )
    {
      ej2 = KheEjectorCopy(ej);
      MArrayPut(res->ejectors, i, ej2);
      for( j = i + 1;  j < MArraySize(options->ejectors);  j++ )
	if( MArrayGet(options->ejectors, j) == ej )
	  MArrayPut(res->ejectors, j, ej2);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS KheOptionsCopy(KHE_OPTIONS options)                          */
/*                                                                           */
/*  Return a copy of options.                                                */
/*                                                                           */
/*****************************************************************************/

KHE_OPTIONS KheOptionsCopy(KHE_OPTIONS options)
{
  KHE_OPTIONS res;
  MMake(res);

  /* options for general solving */
  res->diversify = options->diversify;
  res->monitor_evenness = options->monitor_evenness;
  res->time_assignment_only = options->time_assignment_only;

  /* options for structural solvers */
  res->structural_split_analyser = KheSplitAnalyserMake();
  res->structural_time_equiv = KheTimeEquivMake();

  /* options for time solvers */
  res->time_cluster_meet_domains = options->time_cluster_meet_domains;
  res->time_tighten_domains = options->time_tighten_domains;
  res->time_node_repair = options->time_node_repair;
  res->time_node_regularity = options->time_node_regularity;
  res->time_layer_swap = options->time_layer_swap;
  res->time_layer_repair = options->time_layer_repair;
  res->time_layer_repair_backoff = options->time_layer_repair_backoff;
  res->time_layer_repair_long = options->time_layer_repair_long;
  res->time_kempe_stats = KheKempeStatsMake();

  /* options for resource solvers */
  res->resource_invariant = options->resource_invariant;
  res->resource_rematch = options->resource_rematch;
  res->resource_pair = options->resource_pair;
  res->resource_pair_calls = options->resource_pair_calls;
  res->resource_pair_successes = options->resource_pair_successes;
  res->resource_pair_truncs = options->resource_pair_truncs;

  /* options for ejectors and ejection chains */
  KheEjectorsCopyUnique(options, res);
  res->ejector_schedules_string = options->ejector_schedules_string;
  res->ejector_promote_defects = options->ejector_promote_defects;
  res->ejector_fresh_visits = options->ejector_fresh_visits;
  /* res->ejector_suppress_recent = options->ejector_suppress_recent; */
  res->ejector_repair_times = options->ejector_repair_times;
  res->ejector_vizier_node = options->ejector_vizier_node;
  res->ejector_nodes_before_meets = options->ejector_nodes_before_meets;
  res->ejector_use_kempe_moves = options->ejector_use_kempe_moves;
  res->ejector_use_fuzzy_moves = options->ejector_use_fuzzy_moves;
  res->ejector_use_split_moves = options->ejector_use_split_moves;
  res->ejector_ejecting_not_basic = options->ejector_ejecting_not_basic;
  res->ejector_limit_node = options->ejector_limit_node;
  res->ejector_repair_resources = options->ejector_repair_resources;
  res->ejector_limit_defects = options->ejector_limit_defects;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "general options"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsDiversify(KHE_OPTIONS options)                            */
/*  void KheOptionsSetDiversify(KHE_OPTIONS options, bool diversify)         */
/*                                                                           */
/*  Retrieve and set the diversify option of options.                        */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsDiversify(KHE_OPTIONS options)
{
  return options->diversify;
}

void KheOptionsSetDiversify(KHE_OPTIONS options, bool diversify)
{
  options->diversify = diversify;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsMonitorEvenness(KHE_OPTIONS options)                      */
/*  void KheOptionsSetMonitorEvenness(KHE_OPTIONS options,                   */
/*    bool monitor_evenness)                                                 */
/*                                                                           */
/*  Retrieve and set the monitor_evenness option of options.                 */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsMonitorEvenness(KHE_OPTIONS options)
{
  return options->monitor_evenness;
}

void KheOptionsSetMonitorEvenness(KHE_OPTIONS options,
  bool monitor_evenness)
{
  options->monitor_evenness = monitor_evenness;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeAssignmentOnly(KHE_OPTIONS options)                   */
/*  void KheOptionsSetTimeAssignmentOnly(KHE_OPTIONS options,                */
/*    bool time_assignment_only)                                             */
/*                                                                           */
/*  Retrieve and set the time_assignment_only option of options.             */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeAssignmentOnly(KHE_OPTIONS options)
{
  return options->time_assignment_only;
}

void KheOptionsSetTimeAssignmentOnly(KHE_OPTIONS options,
  bool time_assignment_only)
{
  options->time_assignment_only = time_assignment_only;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "structural solver options"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_ANALYSER KheOptionsStructuralSplitAnalyser(KHE_OPTIONS options)*/
/*  void KheOptionsSetStructuralSplitAnalyser(KHE_OPTIONS options,           */
/*    KHE_SPLIT_ANALYSER structural_split_analyser)                          */
/*                                                                           */
/*  Retrieve and set the structural_split_analyser option of options.        */
/*                                                                           */
/*****************************************************************************/

KHE_SPLIT_ANALYSER KheOptionsStructuralSplitAnalyser(KHE_OPTIONS options)
{
  return options->structural_split_analyser;
}

void KheOptionsSetStructuralSplitAnalyser(KHE_OPTIONS options,
  KHE_SPLIT_ANALYSER structural_split_analyser)
{
  options->structural_split_analyser = structural_split_analyser;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_EQUIV KheOptionsStructuralTimeEquiv(KHE_OPTIONS options)        */
/*  void KheOptionsSetStructuralTimeEquiv(KHE_OPTIONS options,               */
/*    KHE_TIME_EQUIV structural_time_equiv)                                  */
/*                                                                           */
/*  Retrieve and set the structural_time_equiv option of options.            */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_EQUIV KheOptionsStructuralTimeEquiv(KHE_OPTIONS options)
{
  return options->structural_time_equiv;
}

void KheOptionsSetStructuralTimeEquiv(KHE_OPTIONS options,
  KHE_TIME_EQUIV structural_time_equiv)
{
  options->structural_time_equiv = structural_time_equiv;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time solver options"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeClusterMeetDomains(KHE_OPTIONS options)               */
/*  void KheOptionsSetTimeClusterMeetDomains(KHE_OPTIONS options,            */
/*    bool time_cluster_meet_domains)                                        */
/*                                                                           */
/*  Retrieve and set the time_cluster_meet_domains option of options.        */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeClusterMeetDomains(KHE_OPTIONS options)
{
  return options->time_cluster_meet_domains;
}

void KheOptionsSetTimeClusterMeetDomains(KHE_OPTIONS options,
  bool time_cluster_meet_domains)
{
  options->time_cluster_meet_domains = time_cluster_meet_domains;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeTightenDomains(KHE_OPTIONS options)                   */
/*  void KheOptionsSetTimeTightenDomains(KHE_OPTIONS options,                */
/*    bool time_tighten_domains)                                             */
/*                                                                           */
/*  Retrieve and set the time_tighten_domains option of options.             */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeTightenDomains(KHE_OPTIONS options)
{
  return options->time_tighten_domains;
}

void KheOptionsSetTimeTightenDomains(KHE_OPTIONS options,
  bool time_tighten_domains)
{
  options->time_tighten_domains = time_tighten_domains;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeNodeRepair(KHE_OPTIONS options)                       */
/*  void KheOptionsSetTimeNodeRepair(KHE_OPTIONS options,                    */
/*    bool time_node_repair)                                                 */
/*                                                                           */
/*  Retrieve and set the time_node_repair option of options.                 */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeNodeRepair(KHE_OPTIONS options)
{
  return options->time_node_repair;
}

void KheOptionsSetTimeNodeRepair(KHE_OPTIONS options,
  bool time_node_repair)
{
  options->time_node_repair = time_node_repair;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeNodeRegularity(KHE_OPTIONS options)                   */
/*  void KheOptionsSetTimeNodeRegularity(KHE_OPTIONS options,                */
/*    bool time_node_regularity)                                             */
/*                                                                           */
/*  Retrieve and set the time_node_regularity option of options.             */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeNodeRegularity(KHE_OPTIONS options)
{
  return options->time_node_regularity;
}

void KheOptionsSetTimeNodeRegularity(KHE_OPTIONS options,
  bool time_node_regularity)
{
  options->time_node_regularity = time_node_regularity;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeLayerSwap(KHE_OPTIONS options)                        */
/*  void KheOptionsSetTimeLayerSwap(KHE_OPTIONS options,                     */
/*    bool time_layer_swap)                                                  */
/*                                                                           */
/*  Retrieve and set the time_layer_swap option of options.                  */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeLayerSwap(KHE_OPTIONS options)
{
  return options->time_layer_swap;
}

void KheOptionsSetTimeLayerSwap(KHE_OPTIONS options, bool time_layer_swap)
{
  options->time_layer_swap = time_layer_swap;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeLayerRepair(KHE_OPTIONS options)                      */
/*  void KheOptionsSetTimeLayerRepair(KHE_OPTIONS options,                   */
/*    bool time_layer_repair)                                                */
/*                                                                           */
/*  Retrieve and set the time_layer_repair option of options.                */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeLayerRepair(KHE_OPTIONS options)
{
  return options->time_layer_repair;
}

void KheOptionsSetTimeLayerRepair(KHE_OPTIONS options,
  bool time_layer_repair)
{
  options->time_layer_repair = time_layer_repair;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeLayerRepairBackoff(KHE_OPTIONS options)               */
/*  void KheOptionsSetTimeLayerRepairBackoff(KHE_OPTIONS options,            */
/*    bool time_layer_repair_backoff)                                        */
/*                                                                           */
/*  Retrieve and set the time_layer_repair_backoff option of options.        */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeLayerRepairBackoff(KHE_OPTIONS options)
{
  return options->time_layer_repair_backoff;
}

void KheOptionsSetTimeLayerRepairBackoff(KHE_OPTIONS options,
  bool time_layer_repair_backoff)
{
  options->time_layer_repair_backoff = time_layer_repair_backoff;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsTimeLayerRepairLong(KHE_OPTIONS options)                  */
/*  void KheOptionsSetTimeLayerRepairLong(KHE_OPTIONS options,               */
/*    bool time_layer_repair_long)                                           */
/*                                                                           */
/*  Retrieve and set the time_layer_repair_long option of options.           */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsTimeLayerRepairLong(KHE_OPTIONS options)
{
  return options->time_layer_repair_long;
}

void KheOptionsSetTimeLayerRepairLong(KHE_OPTIONS options,
  bool time_layer_repair_long)
{
  options->time_layer_repair_long = time_layer_repair_long;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_STATS KheOptionsTimeKempeStats(KHE_OPTIONS options)            */
/*  void KheOptionsSetTimeKempeStats(KHE_OPTIONS options,                    */
/*    KHE_KEMPE_STATS time_kempe_stats)                                      */
/*                                                                           */
/*  Retrieve and set the time_kempe_stats option of options.                 */
/*                                                                           */
/*****************************************************************************/

KHE_KEMPE_STATS KheOptionsTimeKempeStats(KHE_OPTIONS options)
{
  return options->time_kempe_stats;
}

void KheOptionsSetTimeKempeStats(KHE_OPTIONS options,
  KHE_KEMPE_STATS time_kempe_stats)
{
  options->time_kempe_stats = time_kempe_stats;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource solver options"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsResourceInvariant(KHE_OPTIONS options)                    */
/*  void KheOptionsSetResourceInvariant(KHE_OPTIONS options,                 */
/*    bool resource_invariant)                                               */
/*                                                                           */
/*  Retrieve and set the resource_invariant option of options.               */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsResourceInvariant(KHE_OPTIONS options)
{
  return options->resource_invariant;
}

void KheOptionsSetResourceInvariant(KHE_OPTIONS options,
  bool resource_invariant)
{
  options->resource_invariant = resource_invariant;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsResourceRematch(KHE_OPTIONS options)                      */
/*  void KheOptionsSetResourceRematch(KHE_OPTIONS options,                   */
/*    bool resource_rematch)                                                 */
/*                                                                           */
/*  Retrieve and set the resource_rematch option of options.                 */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsResourceRematch(KHE_OPTIONS options)
{
  return options->resource_rematch;
}

void KheOptionsSetResourceRematch(KHE_OPTIONS options,
  bool resource_rematch)
{
  options->resource_rematch = resource_rematch;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS_RESOURCE_PAIR KheOptionsResourcePair(KHE_OPTIONS options)    */
/*  void KheOptionsSetResourcePair(KHE_OPTIONS options,                      */
/*    KHE_OPTIONS_RESOURCE_PAIR resource_pair)                               */
/*                                                                           */
/*  Retrieve and set the resource_pair option of options.                    */
/*                                                                           */
/*****************************************************************************/

KHE_OPTIONS_RESOURCE_PAIR KheOptionsResourcePair(KHE_OPTIONS options)
{
  return options->resource_pair;
}

void KheOptionsSetResourcePair(KHE_OPTIONS options,
  KHE_OPTIONS_RESOURCE_PAIR resource_pair)
{
  options->resource_pair = resource_pair;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOptionsResourcePairCalls(KHE_OPTIONS options)                     */
/*  void KheOptionsSetResourcePairCalls(KHE_OPTIONS options,                 */
/*    int resource_pair_calls)                                               */
/*                                                                           */
/*  Retrieve and set the resource_pair_calls option of options.              */
/*                                                                           */
/*****************************************************************************/

int KheOptionsResourcePairCalls(KHE_OPTIONS options)
{
  return options->resource_pair_calls;
}


void KheOptionsSetResourcePairCalls(KHE_OPTIONS options,
  int resource_pair_calls)
{
  options->resource_pair_calls = resource_pair_calls;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOptionsResourcePairSuccesses(KHE_OPTIONS options)                 */
/*  void KheOptionsSetResourcePairSuccesses(KHE_OPTIONS options,             */
/*    int resource_pair_successes)                                           */
/*                                                                           */
/*  Retrieve and set the resource_pair_successes option of options.          */
/*                                                                           */
/*****************************************************************************/

int KheOptionsResourcePairSuccesses(KHE_OPTIONS options)
{
  return options->resource_pair_successes;
}


void KheOptionsSetResourcePairSuccesses(KHE_OPTIONS options,
  int resource_pair_successes)
{
  options->resource_pair_successes = resource_pair_successes;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOptionsResourcePairTruncs(KHE_OPTIONS options)                    */
/*  void KheOptionsSetResourcePairTruncs(KHE_OPTIONS options,                */
/*    int resource_pair_truncs)                                              */
/*                                                                           */
/*  Retrieve and set the resource_pair_truncs option of options.             */
/*                                                                           */
/*****************************************************************************/

int KheOptionsResourcePairTruncs(KHE_OPTIONS options)
{
  return options->resource_pair_truncs;
}


void KheOptionsSetResourcePairTruncs(KHE_OPTIONS options,
  int resource_pair_truncs)
{
  options->resource_pair_truncs = resource_pair_truncs;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector options"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR KheOptionsEjector(KHE_OPTIONS options, int index)            */
/*  void KheOptionsSetEjector(KHE_OPTIONS options, int index, KHE_EJECTOR ej)*/
/*                                                                           */
/*  Retrieve and set the index'th ejector option of options.                 */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR KheOptionsEjector(KHE_OPTIONS options, int index)
{
  if( index < MArraySize(options->ejectors) )
    return MArrayGet(options->ejectors, index);
  else
    return NULL;
}

void KheOptionsSetEjector(KHE_OPTIONS options, int index, KHE_EJECTOR ej)
{
  MArrayFill(options->ejectors, index + 1, NULL);
  MArrayPut(options->ejectors, index, ej);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheOptionsEjectorSchedulesString(KHE_OPTIONS options)              */
/*  void KheOptionsSetEjectorSchedulesString(KHE_OPTIONS options,            */
/*    char *ejector_schedules_string)                                        */
/*                                                                           */
/*  Retrieve and set the ejector_schedules_string option of options.         */
/*                                                                           */
/*****************************************************************************/

char *KheOptionsEjectorSchedulesString(KHE_OPTIONS options)
{
  return options->ejector_schedules_string;
}

void KheOptionsSetEjectorSchedulesString(KHE_OPTIONS options,
  char *ejector_schedules_string)
{
  options->ejector_schedules_string = MStringCopy(ejector_schedules_string);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorPromoteDefects(KHE_OPTIONS options)                */
/*  void KheOptionsSetEjectorPromoteDefects(KHE_OPTIONS options,             */
/*    bool ejector_promote_defects)                                          */
/*                                                                           */
/*  Retrieve and set the ejector_promote_defects option of options.          */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorPromoteDefects(KHE_OPTIONS options)
{
  return options->ejector_promote_defects;
}

void KheOptionsSetEjectorPromoteDefects(KHE_OPTIONS options,
  bool ejector_promote_defects)
{
  options->ejector_promote_defects = ejector_promote_defects;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorFreshVisits(KHE_OPTIONS options)                   */
/*  void KheOptionsSetEjectorFreshVisits(KHE_OPTIONS options,                */
/*    bool ejector_fresh_visits)                                             */
/*                                                                           */
/*  Retrieve and set the ejector_fresh_visits option of options.             */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorFreshVisits(KHE_OPTIONS options)
{
  return options->ejector_fresh_visits;
}

void KheOptionsSetEjectorFreshVisits(KHE_OPTIONS options,
  bool ejector_fresh_visits)
{
  options->ejector_fresh_visits = ejector_fresh_visits;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorSuppressRecent(KHE_OPTIONS options)                */
/*  void KheOptionsSetEjectorSuppressRecent(KHE_OPTIONS options,             */
/*    bool ejector_suppress_recent)                                          */
/*                                                                           */
/*  Retrieve and set the ejector_suppress_recent option of options.          */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheOptionsEjectorSuppressRecent(KHE_OPTIONS options)
{
  return options->ejector_suppress_recent;
}

void KheOptionsSetEjectorSuppressRecent(KHE_OPTIONS options,
  bool ejector_suppress_recent)
{
  options->ejector_suppress_recent = ejector_suppress_recent;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorRepairTimes(KHE_OPTIONS options)                   */
/*  void KheOptionsSetEjectorRepairTimes(KHE_OPTIONS options,                */
/*    bool ejector_repair_times)                                             */
/*                                                                           */
/*  Retrieve and set the ejector_repair_times option of options.             */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorRepairTimes(KHE_OPTIONS options)
{
  return options->ejector_repair_times;
}

void KheOptionsSetEjectorRepairTimes(KHE_OPTIONS options,
  bool ejector_repair_times)
{
  options->ejector_repair_times = ejector_repair_times;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorVizierNode(KHE_OPTIONS options)                    */
/*  void KheOptionsSetEjectorVizierNode(KHE_OPTIONS options,                 */
/*    bool ejector_vizier_node)                                              */
/*                                                                           */
/*  Retrieve and set the ejector_vizier_node option of options.              */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorVizierNode(KHE_OPTIONS options)
{
  return options->ejector_vizier_node;
}

void KheOptionsSetEjectorVizierNode(KHE_OPTIONS options,
  bool ejector_vizier_node)
{
  options->ejector_vizier_node = ejector_vizier_node;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorNodesBeforeMeets(KHE_OPTIONS options)              */
/*  void KheOptionsSetEjectorNodesBeforeMeets(KHE_OPTIONS options,           */
/*    bool ejector_nodes_before_meets)                                       */
/*                                                                           */
/*  Retrieve and set the ejector_nodes_before_meets option of options.       */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorNodesBeforeMeets(KHE_OPTIONS options)
{
  return options->ejector_nodes_before_meets;
}

void KheOptionsSetEjectorNodesBeforeMeets(KHE_OPTIONS options,
  bool ejector_nodes_before_meets)
{
  options->ejector_nodes_before_meets = ejector_nodes_before_meets;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_OPTIONS_KEMPE KheOptionsEjectorUseKempeMoves(KHE_OPTIONS options)    */
/*  void KheOptionsSetEjectorUseKempeMoves(KHE_OPTIONS options,              */
/*    KHE_OPTIONS_KEMPE ejector_use_kempe_moves)                             */
/*                                                                           */
/*  Retrieve and set the ejector_use_kempe_moves option of options.          */
/*                                                                           */
/*****************************************************************************/

KHE_OPTIONS_KEMPE KheOptionsEjectorUseKempeMoves(KHE_OPTIONS options)
{
  return options->ejector_use_kempe_moves;
}

void KheOptionsSetEjectorUseKempeMoves(KHE_OPTIONS options,
  KHE_OPTIONS_KEMPE ejector_use_kempe_moves)
{
  options->ejector_use_kempe_moves = ejector_use_kempe_moves;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorUseFuzzyMoves(KHE_OPTIONS options)                 */
/*  void KheOptionsSetEjectorUseFuzzyMoves(KHE_OPTIONS options,              */
/*    bool ejector_use_fuzzy_moves)                                          */
/*                                                                           */
/*  Retrieve and set the ejector_use_fuzzy_moves option of options.          */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorUseFuzzyMoves(KHE_OPTIONS options)
{
  return options->ejector_use_fuzzy_moves;
}

void KheOptionsSetEjectorUseFuzzyMoves(KHE_OPTIONS options,
  bool ejector_use_fuzzy_moves)
{
  options->ejector_use_fuzzy_moves = ejector_use_fuzzy_moves;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorUseSplitMoves(KHE_OPTIONS options)                 */
/*  void KheOptionsSetEjectorUseSplitMoves(KHE_OPTIONS options,              */
/*    bool ejector_use_split_moves)                                          */
/*                                                                           */
/*  Retrieve and set the ejector_use_split_moves option of options.          */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorUseSplitMoves(KHE_OPTIONS options)
{
  return options->ejector_use_split_moves;
}

void KheOptionsSetEjectorUseSplitMoves(KHE_OPTIONS options,
  bool ejector_use_split_moves)
{
  options->ejector_use_split_moves = ejector_use_split_moves;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorEjectingNotBasic(KHE_OPTIONS options)              */
/*  void KheOptionsSetEjectorEjectingNotBasic(KHE_OPTIONS options,           */
/*    bool ejector_ejecting_not_basic)                                       */
/*                                                                           */
/*  Retrieve and set the ejector_ejecting_not_basic option of options.       */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorEjectingNotBasic(KHE_OPTIONS options)
{
  return options->ejector_ejecting_not_basic;
}

void KheOptionsSetEjectorEjectingNotBasic(KHE_OPTIONS options,
  bool ejector_ejecting_not_basic)
{
  options->ejector_ejecting_not_basic = ejector_ejecting_not_basic;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheOptionsEjectorLimitNode(KHE_OPTIONS options)                 */
/*  void KheOptionsSetEjectorLimitNode(KHE_OPTIONS options,                  */
/*    KHE_NODE ejector_limit_node)                                           */
/*                                                                           */
/*  Retrieve and set the ejector_limit_node option of options.               */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheOptionsEjectorLimitNode(KHE_OPTIONS options)
{
  return options->ejector_limit_node;
}

void KheOptionsSetEjectorLimitNode(KHE_OPTIONS options,
  KHE_NODE ejector_limit_node)
{
  options->ejector_limit_node = ejector_limit_node;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOptionsEjectorRepairResources(KHE_OPTIONS options)               */
/*  void KheOptionsSetEjectorRepairResources(KHE_OPTIONS options,            */
/*    bool ejector_repair_resources)                                         */
/*                                                                           */
/*  Retrieve and set the ejector_repair_resources option of options.         */
/*                                                                           */
/*****************************************************************************/

bool KheOptionsEjectorRepairResources(KHE_OPTIONS options)
{
  return options->ejector_repair_resources;
}

void KheOptionsSetEjectorRepairResources(KHE_OPTIONS options,
  bool ejector_repair_resources)
{
  options->ejector_repair_resources = ejector_repair_resources;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOptionsEjectorLimitDefects(KHE_OPTIONS options)                   */
/*  void KheOptionsSetEjectorLimitDefects(KHE_OPTIONS options,               */
/*    int ejector_limit_defects)                                             */
/*                                                                           */
/*  Retrieve and set the ejector_limit_defects option of options.            */
/*                                                                           */
/*****************************************************************************/

int KheOptionsEjectorLimitDefects(KHE_OPTIONS options)
{
  return options->ejector_limit_defects;
}

void KheOptionsSetEjectorLimitDefects(KHE_OPTIONS options,
  int ejector_limit_defects)
{
  options->ejector_limit_defects = ejector_limit_defects;
}
