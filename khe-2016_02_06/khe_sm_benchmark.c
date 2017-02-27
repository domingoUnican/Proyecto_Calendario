
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
/*  FILE:         khe_sm_benchmark.c                                         */
/*  DESCRIPTION:  Benchmarking                                               */
/*                                                                           */
/*  The tests run and the files produced are as follows.  Here, <sn> stands  */
/*  for the solver name, and <sn>x8 stands for running that solver 8 times   */
/*  with diversifiers 0 .. 7 and keeping the best of the 8 solutions.        */
/*                                                                           */
/*  TestA                                                                    */
/*    tab:<sn>                    Cost and time of <sn> and <sn>x8           */
/*    tab:<sn>x8:defects:a        Defects after running <sn>x8               */
/*    tab:<sn>x8:defects:b        Defects after running <sn>x8               */
/*    <sn>_solns.xml              Archive of instances and <sn> solns        */
/*    <sn>x8_solns.xml            Archive of instances and <sn>x8 solns      */
/*  TestB                                                                    */
/*    tab:nodereg                 Tests with/without node regularity         */
/*  TestC                                                                    */
/*    tab:frvz:cost               Costs with vizier nodes/full repair        */
/*    tab:frvz:runtime            Time with vizier nodes/full repair         */
/*  TestD                                                                    */
/*    tab:room:lbub               Room assignment lower bounds/upper bounds  */
/*  TestE                                                                    */
/*    tab:rm                      Tests of resource rematching               */
/*  TestF                                                                    */
/*    tab:pr                      Tests of resource pair repair              */
/*  TestG                                                                    */
/*    tab:chaintype               Tests using various ejection chain types   */
/*  TestH                                                                    */
/*    figure:kempe:steps          Kempe meet moves - number of steps         */
/*    figure:kempe:phases         Kempe meet moves - number of phases        */
/*    figure:chainlength:time     Ejection chain lengths (time repair)       */
/*    figure:chainlength:resource Ejection chain lengths (resource repair)   */
/*    figure:augment:time         Ejection chain augments (time repair)      */
/*    figure:augment:resource     Ejection chain augments (resource repair)  */
/*    tab:chainaugment:time       Ejection chain augments (time repair)      */
/*    tab:chainaugment:resource   Ejection chain augments (resource repair)  */
/*    tab:chainrepair:time        Ejection chain repairs (time repair)       */
/*    tab:chainrepair:resource    Ejection chain repairs (resource repair)   */
/*  TestI                                                                    */
/*    tab:moves                   Tests of Kempe, ejecting, and basic moves  */
/*  TestJ                                                                    */
/*    tab:nodesfirst              Tests of nodes before meets                */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 1
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  Submodule "testing alternative options"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TEST_OPTIONS                                                         */
/*                                                                           */
/*  Function test_options(i, options, &id) returns true when a column i      */
/*  is wanted (starting from 0), sets whatever options of options need       */
/*  to be set for that case, and returns an id for the column, which will    */
/*  be prefixed by "C:" in cost columns and "T:" in running time columns.    */
/*                                                                           */
/*****************************************************************************/

typedef bool (*KHE_TEST_OPTIONS)(int i, KHE_OPTIONS options, char **id);


/*****************************************************************************/
/*                                                                           */
/*  bool KheBenchmarkTryInstance(KHE_INSTANCE ins)                           */
/*                                                                           */
/*  Return true if ins is an instance for which solving should be tried.     */
/*                                                                           */
/*  With the revision of DK-HG-12 and the adoption of a 15-minute soft       */
/*  time limit, there is currently no reason not to try any instance.        */
/*                                                                           */
/*****************************************************************************/

bool KheBenchmarkTryInstance(KHE_INSTANCE ins)
{
  return true;
  /* ***
  return strcmp(KheInstanceId(ins), "DK-HG-12") != 0 &&
    strcmp(KheInstanceId(ins), "NL-KP-05") != 0 &&
    strcmp(KheInstanceId(ins), "NL-KP-09") != 0;
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInsId(KHE_INSTANCE ins, int i)                                  */
/*                                                                           */
/*  Return an Id for ins, preferably the instance Id, if not, then           */
/*  "Instance i"                                                             */
/*                                                                           */
/*****************************************************************************/

char *KheInsId(KHE_INSTANCE ins, int i)
{
  char buff[20];
  if( KheInstanceId(ins) != NULL )
    return KheInstanceId(ins);
  else
  {
    sprintf(buff, "Instance %d", i);
    return MStringCopy(buff);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestOptions(KHE_ARCHIVE archive,                        */
/*    KHE_OPTIONS options, KHE_TEST_OPTIONS options_fn,                      */
/*    int thread_count, KHE_GENERAL_SOLVER solver,                           */
/*    char *table_id, KHE_STATS_TABLE_TYPE table_type, char *caption)        */
/*                                                                           */
/*  Produce a table with one row for each instance of archive, one cost      */
/*  column for each i >= 0 such that options_fn(j, ...) returns true for     */
/*  all j s.t. 0 <= j <= i, and one time column for each cost column.        */
/*  Report the cost and run time of the result of solver on each instance.   */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestOptions(KHE_ARCHIVE archive,
  KHE_OPTIONS options, KHE_TEST_OPTIONS options_fn,
  int thread_count, KHE_GENERAL_SOLVER solver,
  char *table_id, KHE_STATS_TABLE_TYPE table_type, char *caption)
{
  int i, j, col_count;  char *id, tbuff[25], cbuff[25];  KHE_SOLN_GROUP sg;
  KHE_INSTANCE ins;  KHE_SOLN soln;

  /* find out how many options there are */
  for( col_count = 0;  col_count < 1000;  col_count++ )
  {
    if( !options_fn(col_count, options, &id) )
      break;
    MAssert(strlen(id)<=20, "KheBenchmarkTestOptions: id \"%s\" too long", id);
  }

  /* start off the file and table */
  KheStatsFileBegin(table_id);
  KheStatsTableBegin(table_id, table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd(table_id, caption);

  /* one cost column for each option, with a vrule after the last column */
  for( i = 0;  i < col_count;  i++ )
  {
    options_fn(i, options, &id);
    sprintf(cbuff, "C:%s", id);
    KheStatsColAdd(table_id, cbuff, i == col_count - 1);
  }

  /* one time column for each option */
  for( i = 0;  i < col_count;  i++ )
  {
    options_fn(i, options, &id);
    sprintf(tbuff, "T:%s", id);
    KheStatsColAdd(table_id, tbuff, false);
  }

  /* one row for each instance */
  for( j = 0;  j < KheArchiveInstanceCount(archive);  j++ )
  {
    ins = KheArchiveInstance(archive, j);
    id = KheInstanceId(ins);
    MAssert(id != NULL, "KheBenchmarkTestOptions: NULL instance id");
    KheStatsRowAdd(table_id, id, false);
  }

  /* solve the archive once for each option */
  for( i = 0;  i < col_count;  i++ )
  {
    /* get the options and the column names */
    options_fn(i, options, &id);
    sprintf(cbuff, "C:%s", id);
    sprintf(tbuff, "T:%s", id);

    /* solve the archive in parallel with these options */
    if( !KheSolnGroupMake(archive, NULL, NULL, &sg) )
      MAssert(false, "KheBenchmarkTestOptions internal error 1");
    KheArchiveParallelSolve(archive, thread_count, 1, solver, options, 1, sg);

    /* add a cost and time entry for each instance */
    for( j = 0;  j < KheSolnGroupSolnCount(sg);  j++ )
    {
      soln = KheSolnGroupSoln(sg, j);
      id = KheInstanceId(KheSolnInstance(soln));
      KheStatsEntryAddCost(table_id, id, cbuff, KheSolnCost(soln));
      KheStatsEntryAddTime(table_id, id, tbuff, KheSolnRunningTime(soln));
    }

    /* delete all solutions */
    while( KheSolnGroupSolnCount(sg) > 0 )
      KheSolnDelete(KheSolnGroupSoln(sg, 0));
  }

  /* finish off the table and file */
  KheStatsTableEnd(table_id);
  KheStatsFileEnd(table_id);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "the various tests"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestA(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test A, producing the following files:                     */
/*                                                                           */
/*  TestA                                                                    */
/*    tab:<sn>                    Cost and time of <sn> and <sn>x8           */
/*    tab:<sn>x8:defects:a        Defects after running <sn>x8               */
/*    tab:<sn>x8:defects:b        Defects after running <sn>x8               */
/*    <sn>_solns.xml              Archive of instances and <sn> solns        */
/*    <sn>x8_solns.xml            Archive of instances and <sn>x8 solns      */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestA(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, j, dcount;  KHE_INSTANCE ins;  KHE_SOLN soln;
  KHE_OPTIONS options;  char *id;  KHE_STATS_TIMER st;
  KHE_SOLN_GROUP sg, sgx8;  FILE *fp;
  KHE_SOLN_GROUP_METADATA md;
  char tab_str[100], cost_str[100], cost_str_x8[100];
  char time_str[100], time_str_x8[100], defects_a[100], defects_b[100];
  char produced_by[100], produced_by_x8[100], solver_name_x8[100];
  char file_name[100], file_name_x8[100];

  /* column headers and related info for table:<sn>x8:defects:a */
  int acount = 7;
  char *astr[] = { "SS", "DS", "AT", "PT", "SE", "LE", "OE" };
  KHE_CONSTRAINT_TAG acon[] = {
    KHE_SPLIT_EVENTS_CONSTRAINT_TAG, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG,
    KHE_ASSIGN_TIME_CONSTRAINT_TAG, KHE_PREFER_TIMES_CONSTRAINT_TAG,
    KHE_SPREAD_EVENTS_CONSTRAINT_TAG, KHE_LINK_EVENTS_CONSTRAINT_TAG,
    KHE_ORDER_EVENTS_CONSTRAINT_TAG };
  KHE_MONITOR_TAG amon[] = {
    KHE_SPLIT_EVENTS_MONITOR_TAG, KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG,
    KHE_ASSIGN_TIME_MONITOR_TAG, KHE_PREFER_TIMES_MONITOR_TAG,
    KHE_SPREAD_EVENTS_MONITOR_TAG, KHE_LINK_EVENTS_MONITOR_TAG,
    KHE_ORDER_EVENTS_MONITOR_TAG };

  /* column headers and related info for tab:<sn>x8:defects:b */
  int bcount = 9;
  char *bstr[] = { "AR", "PR", "AS", "AC", "AU", "LI", "CB", "LB", "LW" };
  KHE_CONSTRAINT_TAG bcon[] = {
    KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG, KHE_PREFER_RESOURCES_CONSTRAINT_TAG,
    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG,
    KHE_AVOID_CLASHES_CONSTRAINT_TAG,
    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG,
    KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG,
    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG, KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG,
    KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG };
  KHE_MONITOR_TAG bmon[] = {
    KHE_ASSIGN_RESOURCE_MONITOR_TAG, KHE_PREFER_RESOURCES_MONITOR_TAG,
    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG, KHE_AVOID_CLASHES_MONITOR_TAG,
    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG, KHE_LIMIT_IDLE_TIMES_MONITOR_TAG,
    KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG, KHE_LIMIT_BUSY_TIMES_MONITOR_TAG,
    KHE_LIMIT_WORKLOAD_MONITOR_TAG };

  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestA(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* initialize various strings containing solver_name */
  snprintf(tab_str, 100, "tab:%s", solver_name);
  snprintf(cost_str, 100, "C:%s", solver_name);
  snprintf(cost_str_x8, 100, "C:%sx8", solver_name);
  snprintf(time_str, 100, "T:%s", solver_name);
  snprintf(time_str_x8, 100, "T:%sx8", solver_name);
  snprintf(defects_a, 100, "tab:%sx8:defects:a", solver_name);
  snprintf(defects_b, 100, "tab:%sx8:defects:b", solver_name);
  snprintf(produced_by, 100, "Produced by %s", solver_name);
  snprintf(produced_by_x8, 100, "Produced by %sx8", solver_name);
  snprintf(solver_name_x8, 100, "%sx8", solver_name);
  snprintf(file_name, 100, "stats/%s_solns.xml", solver_name);
  snprintf(file_name_x8, 100, "stats/%sx8_solns.xml", solver_name);

  /* set up cost and runtime table */
  KheStatsFileBegin(tab_str);
  KheStatsTableBegin(tab_str, table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd(tab_str,
    "Effectiveness of %s and %s.  Details as previously.\n"
    "Different solutions to one instance vary in run time, so\n"
    "finding eight solutions on a quad-core machine often takes\n"
    "more than twice as long as finding one.\n", solver_name, solver_name_x8);
  KheStatsColAdd(tab_str, cost_str, false);
  KheStatsColAdd(tab_str, cost_str_x8, true);
  KheStatsColAdd(tab_str, time_str, false);
  KheStatsColAdd(tab_str, time_str_x8, false);

  /* set up defects table a */
  KheStatsFileBegin(defects_a);
  KheStatsTableBegin(defects_a, table_type,
    12, "Instance", false, true, false, false, false);
  KheStatsCaptionAdd(defects_a,
    "Event defects in the solutions produced by %s.  Each column shows\n"
    "the number of defects of one kind of event constraint.  A dash indicates\n"
    "that the instance contains no constraints of that type.  The columns\n"
    "appear in the same order as the rows of Table 1.\n", solver_name_x8);
  for( j = 0;  j < acount;  j++ )
    KheStatsColAdd(defects_a, astr[j], false);

  /* set up defects table b */
  KheStatsFileBegin(defects_b);
  KheStatsTableBegin(defects_b, table_type,
    12, "Instance", false, true, false, false, false);
  KheStatsCaptionAdd(defects_b,
    "Event resource and resource defects produced by\n"
    "%s.  Details as previously.\n", solver_name_x8);
  for( j = 0;  j < bcount;  j++ )
    KheStatsColAdd(defects_b, bstr[j], false);
    /* true after avoid split assignments - still to do */

  /* set up solution groups */
  md = KheSolnGroupMetaDataMake(author_name, KheStatsDateToday(),
    produced_by, NULL, NULL);
  if( !KheSolnGroupMake(archive, solver_name, md, &sg) )
    MAssert(false, "KheBenchmarkTestA: cannot make soln group %s", solver_name);
  md = KheSolnGroupMetaDataMake(author_name, KheStatsDateToday(),
    produced_by_x8, NULL, NULL);
  if( !KheSolnGroupMake(archive, solver_name_x8, md, &sgx8) )
    MAssert(false, "KheBenchmarkTestA: cannot make soln group %s",
      solver_name_x8);

  /* solve the instances and record the results in the tables */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd(tab_str, id, false);
    KheStatsRowAdd(defects_a, id, false);
    KheStatsRowAdd(defects_b, id, false);
    if( KheBenchmarkTryInstance(ins) )
    {
      /* try solving ins single-threaded */
      if( DEBUG1 )
	fprintf(stderr, "solving instance \"%s\" single-threaded:\n",
	  KheInstanceId(ins));
      st = KheStatsTimerMake();
      soln = KheSolnMake(ins);
      soln = solver(soln, options);
      KheStatsEntryAddCost(tab_str, id, cost_str, KheSolnCost(soln));
      KheStatsEntryAddTime(tab_str, id, time_str, KheStatsTimerNow(st));
      KheSolnGroupAddSoln(sg, soln);
      /* KheSolnDelete(soln); */
      KheStatsTimerDelete(st);

      /* try solving ins multi-threaded */
      if( DEBUG1 )
	fprintf(stderr, "solving instance \"%s\" multi-threaded:\n",
	  KheInstanceId(ins));
      st = KheStatsTimerMake();
      soln = KheInstanceParallelSolve(ins, 4, 8, solver, options);
      KheStatsEntryAddCost(tab_str, id, cost_str_x8, KheSolnCost(soln));
      KheStatsEntryAddTime(tab_str, id, time_str_x8, KheStatsTimerNow(st));
      for( j = 0;  j < acount;  j++ )
      {
	KheSolnCostByType(soln, amon[j], &dcount);
	if( KheInstanceConstraintDensityCount(ins, acon[j]) == 0 )
	{
	  MAssert(dcount == 0, "KheBenchmarkTestA internal error 1");
          KheStatsEntryAddString(defects_a, id, astr[j], "-");
	}
	else
	  KheStatsEntryAddInt(defects_a, id, astr[j], dcount);
      }
      for( j = 0;  j < bcount;  j++ )
      {
	KheSolnCostByType(soln, bmon[j], &dcount);
        if( KheInstanceConstraintDensityCount(ins, bcon[j]) == 0 )
	{
	  MAssert(dcount == 0, "KheBenchmarkTestA internal error 2");
	  KheStatsEntryAddString(defects_b, id, bstr[j], "-");
	}
	else
	  KheStatsEntryAddInt(defects_b, id, bstr[j], dcount);
      }
      KheSolnGroupAddSoln(sgx8, soln);
      /* KheSolnDelete(soln); */
      KheStatsTimerDelete(st);
    }
  }

  /* finish off the two tables */
  KheStatsTableEnd(tab_str);
  KheStatsFileEnd(tab_str);
  KheStatsTableEnd(defects_a);
  KheStatsFileEnd(defects_a);
  KheStatsTableEnd(defects_b);
  KheStatsFileEnd(defects_b);
  KheOptionsDelete(options);

  /* write the two solution groups (in a single archive) */
  fp = fopen(file_name, "w");
  KheArchiveWriteSolnGroup(archive, sg, false, fp);
  fclose(fp);
  fp = fopen(file_name_x8, "w");
  KheArchiveWriteSolnGroup(archive, sgx8, false, fp);
  fclose(fp);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestA returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestBCase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id, bool time_node_regularity)        */
/*                                                                           */
/*  Carry out one case of Test 2.                                            */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheTestBCase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id, bool time_node_regularity)
{
  char cbuff[10], tbuff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  sprintf(cbuff, "C:%cRF", time_node_regularity ? '+' : '-');
  sprintf(tbuff, "T:%cRF", time_node_regularity ? '+' : '-');
  KheOptionsSetTimeNodeRegularity(options, time_node_regularity);
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), cbuff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins, NULL);
  soln = solver(soln, options);
  KheStatsEntryAddCost("tab:nodereg", id, cbuff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:nodereg", id, tbuff, KheStatsTimerNow(st));
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestB(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test B, producing the following files:                     */
/*                                                                           */
/*  TestB                                                                    */
/*    tab:nodereg                 Tests with/without node regularity         */
/*                                                                           */
/*****************************************************************************/

/* *** old version
static void KheBenchmarkTestB(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestB(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  ** set up table **
  KheStatsFileBegin("tab:nodereg");
  KheStatsTableBegin("tab:nodereg", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:nodereg",
    "Encouraging regularity between forms:  -RF and +RF denote without it\n"
    "and with it.  KHE14 uses +RF.  In all tables in this paper, columns\n"
    "headed C: contain solution costs.  Hard costs appear to the left of\n"
    "the decimal point; soft costs appear as five-digit integers to the\n"
    "right of the point.  The minimum costs in each row are highlighted.\n"
    "Columns headed T: contain run times in seconds.  All tables and graphs\n"
    "(including captions) were generated by KHE and incorporated unchanged.\n"
    "They can be regenerated by any user of KHE.\n");
  KheStatsColAdd("tab:nodereg", "C:-RF", false);
  KheStatsColAdd("tab:nodereg", "C:+RF", true);
  KheStatsColAdd("tab:nodereg", "T:-RF", false);
  KheStatsColAdd("tab:nodereg", "T:+RF", false);

  ** solve the instance and record the results in the two tables **
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:nodereg", id, false);
    if( KheBenchmarkTryInstance(ins) )
    {
      KheTestBCase(ins, options, solver, id, false);
      KheTestBCase(ins, options, solver, id, true);
    }
  }

  ** finish off the two tables **
  KheStatsTableEnd("tab:nodereg");
  KheStatsFileEnd("tab:nodereg");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestB returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestB(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test B, producing the following files:                     */
/*                                                                           */
/*  TestB                                                                    */
/*    tab:nodereg                 Tests with/without node regularity         */
/*                                                                           */
/*****************************************************************************/

static bool KheTestBOptionsFn(int i, KHE_OPTIONS options, char **id)
{
  switch( i )
  {
    case 0:

      KheOptionsSetTimeNodeRegularity(options, false);
      *id = "-RF";
      return true;

    case 1:

      KheOptionsSetTimeNodeRegularity(options, true);
      *id = "+RF";
      return true;

    default:

      *id = NULL;
      return false;
  }
}

static void KheBenchmarkTestB(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  KHE_OPTIONS options;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestB(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();
  KheBenchmarkTestOptions(archive, options, &KheTestBOptionsFn, 4, solver,
    "tab:nodereg", table_type,
    "Encouraging regularity between forms:  -RF and +RF denote without it\n"
    "and with it.  KHE14 uses +RF.  In all tables in this paper, columns\n"
    "headed C: contain solution costs.  Hard costs appear to the left of\n"
    "the decimal point; soft costs appear as five-digit integers to the\n"
    "right of the point.  The minimum costs in each row are highlighted.\n"
    "Columns headed T: contain run times in seconds.  All tables and graphs\n"
    "(including captions) were generated by KHE and incorporated unchanged.\n"
    "They can be regenerated by any user of KHE.\n");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestB returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestCCase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id,                                   */
/*    bool time_layer_repair_long, bool time_vizier_node)                    */
/*                                                                           */
/*  Carry out one case of Test C.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTestCCase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id,
  bool time_layer_repair_long, bool time_vizier_node)
{
  char buff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  sprintf(buff, "%cFR%cVZ", time_layer_repair_long ? '+' : '-',
    time_vizier_node ? '+' : '-');
  KheOptionsSetTimeLayerRepairLong(options, time_layer_repair_long);
  KheOptionsSetEjectorVizierNode(options, time_vizier_node);
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), buff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins);
  soln = solver(soln, options);
  KheStatsEntryAddCost("tab:frvz:cost", id, buff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:frvz:runtime", id, buff, KheStatsTimerNow(st));
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestC(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test C, producing these files:                             */
/*                                                                           */
/*  TestC                                                                    */
/*    tab:frvz:cost               Costs with vizier nodes/full repair        */
/*    tab:frvz:runtime            Time with vizier nodes/full repair         */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestC(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, o1, o2;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  int opt1_count = 2;  bool opt1[] = {false, true};
  int opt2_count = 2;  bool opt2[] = {false, true};
  /* ***
  int opt2_count = 2;  KHE_OPTIONS_TIME_LAYER_REPAIR opt2[] =
    {KHE_OPTIONS_TIME_LAYER_REPAIR_LAYER, KHE_OPTIONS_TIME_LAYER_REPAIR_NODE};
  *** */
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestC(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* set up costs table */
  KheStatsFileBegin("tab:frvz:cost");
  KheStatsTableBegin("tab:frvz:cost", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:frvz:cost",
    "Effectiveness of variants of KHE14's time repair algorithm:\n"
    "-FR and +FR denote without and with full repair after assigning\n"
    "each layer, and -VZ and +VZ denote without and with a vizier node.\n"
    "KHE14 uses -VZ-FR.  The two variants are tested together since they\n"
    "seem likely to interact.\n");
  KheStatsColAdd("tab:frvz:cost", "-FR-VZ", false);
  KheStatsColAdd("tab:frvz:cost", "-FR+VZ", false);
  KheStatsColAdd("tab:frvz:cost", "+FR-VZ", false);
  KheStatsColAdd("tab:frvz:cost", "+FR+VZ", false);

  /* set up run times table */
  KheStatsFileBegin("tab:frvz:runtime");
  KheStatsTableBegin("tab:frvz:runtime", table_type,
    12, "Instance", true, false, false, false, false);
  KheStatsCaptionAdd("tab:frvz:runtime",
    "Run times (in seconds) of the vizier node and full repair variants\n"
    "of KHE14.\n");
  KheStatsColAdd("tab:frvz:runtime", "-FR-VZ", false);
  KheStatsColAdd("tab:frvz:runtime", "-FR+VZ", false);
  KheStatsColAdd("tab:frvz:runtime", "+FR-VZ", false);
  KheStatsColAdd("tab:frvz:runtime", "+FR+VZ", false);

  /* solve the instance and record the results in the two tables */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:frvz:cost", id, false);
    KheStatsRowAdd("tab:frvz:runtime", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( o1 = 0;  o1 < opt1_count;  o1++ )
	for( o2 = 0;  o2 < opt2_count;  o2++ )
	  KheTestCCase(ins, options, solver, id, opt1[o1], opt2[o2]);
  }

  /* finish off the two tables */
  KheStatsTableEnd("tab:frvz:cost");
  KheStatsFileEnd("tab:frvz:cost");
  KheStatsTableEnd("tab:frvz:runtime");
  KheStatsFileEnd("tab:frvz:runtime");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestC returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceRequiresRoomAssignment(KHE_INSTANCE ins)                 */
/*                                                                           */
/*  Return true if ins requires room assignment.                             */
/*                                                                           */
/*****************************************************************************/

static bool KheInstanceRequiresRoomAssignment(KHE_INSTANCE ins)
{
  KHE_RESOURCE_TYPE rt;  int i;
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( KheResourceTypeAvoidSplitAssignmentsCount(rt) == 0 )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTestDLowerBound(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the lower bound needed by test3:  over all resource types for     */
/*  which there are no avoid split assignments constraints, the number of    */
/*  unmatched demand nodes of those types in the global tixel matching.      */
/*                                                                           */
/*  This function assumes that the solve ended after time assignment.        */
/*                                                                           */
/*****************************************************************************/

static int KheTestDLowerBound(KHE_SOLN soln)
{
  /* KHE_INSTANCE ins; */  KHE_RESOURCE_TYPE rt;  int i, res;  KHE_MONITOR m;
  KHE_TASK task;  KHE_RESOURCE r;
  if( DEBUG2 )
    fprintf(stderr, "  [ KheTestDLowerBound(soln)\n");

  /* find the unsatisfied demand monitors */
  /* ins = KheSolnInstance(soln); */
  res = 0;
  for( i = 0;  i < KheSolnMatchingDefectCount(soln);  i++ )
  {
    /* get the defect and its resource type */
    m = KheSolnMatchingDefect(soln, i);
    if( DEBUG2 )
      KheMonitorDebug(m, 2, 4, stderr);
    if( KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG )
    {
      /* ordinary demand monitor, get resource type via task */
      task = KheOrdinaryDemandMonitorTask((KHE_ORDINARY_DEMAND_MONITOR) m);
      rt = KheTaskResourceType(task);
    }
    else
    {
      /* workload demand monitor, get resource type via resource */
      MAssert(KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG,
	"KheTestDLowerBound internal error");
      r = KheWorkloadDemandMonitorResource((KHE_WORKLOAD_DEMAND_MONITOR) m);
      rt = KheResourceResourceType(r);
    }

    /* if the resource type is one we are interested in, add to res */
    if( KheResourceTypeAvoidSplitAssignmentsCount(rt) == 0 )
    {
      if( DEBUG2 )
	fprintf(stderr, "    res++\n");
      res++;
    }
  }
  if( DEBUG2 )
    fprintf(stderr, "  ] KheTestDLowerBound returning %d\n", res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTestDUpperBound(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the upper bound needed by test3:  over all resource types for     */
/*  which there are no avoid split assignments constraints, the total        */
/*  duration of hard constraint violations of suitable kinds.                */
/*                                                                           */
/*****************************************************************************/

static int KheTestDUpperBound(KHE_SOLN soln)
{
  KHE_RESOURCE_TYPE rt;  int i, /* j, */ res;  KHE_MONITOR m;
  KHE_RESOURCE r;  KHE_CONSTRAINT c;  KHE_EVENT_RESOURCE er;

  res = 0;
  for( i = 0;  i < KheSolnDefectCount(soln);  i++ )
  {
    /* find m's resource type; how this is done depends on m's type */
    m = KheSolnDefect(soln, i);
    rt = NULL;
    c = KheMonitorConstraint(m);
    if( c != NULL && KheConstraintCombinedWeight(c) >= KheCost(1, 0) )
    {
      switch( KheMonitorTag(m) )
      {
	case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

	  er = KheAssignResourceMonitorEventResource(
	    (KHE_ASSIGN_RESOURCE_MONITOR) m);
	  rt = KheEventResourceResourceType(er);
	  break;

	case KHE_PREFER_RESOURCES_MONITOR_TAG:

	  er = KhePreferResourcesMonitorEventResource(
	    (KHE_PREFER_RESOURCES_MONITOR) m);
	  rt = KheEventResourceResourceType(er);
	  break;

	case KHE_AVOID_CLASHES_MONITOR_TAG:

	  r = KheAvoidClashesMonitorResource((KHE_AVOID_CLASHES_MONITOR) m);
	  rt = KheResourceResourceType(r);
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

	  r = KheAvoidUnavailableTimesMonitorResource(
	    (KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
	  rt = KheResourceResourceType(r);
	  break;

	default:

	  /* not interested, so let rt remain NULL */
	  break;
      }
    }

    /* count m's defects if it's the right resource type */
    if( rt != NULL && KheResourceTypeAvoidSplitAssignmentsCount(rt) == 0 )
      res += KheMonitorDeviation(m);
      /* ***
      for( j = 0;  j < KheMonitorDeviationCount(m);  j++ )
	res += KheMonitorDeviation(m, j);
      *** */
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestD(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test D, producing these files:                             */
/*                                                                           */
/*  TestD                                                                    */
/*    tab:room:lbub               Room assignment lower bounds/upper bounds  */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestD(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;  KHE_SOLN soln;
  bool save_time_assignment_only;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestD(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* set up defects table */
  KheStatsFileBegin("tab:room:lbub");
  KheStatsTableBegin("tab:room:lbub", table_type,
    12, "Instance", false, false, false, false, false);
  KheStatsCaptionAdd("tab:room:lbub",
    "Effectiveness of KHE14's room assignment algorithm.  LB is the lower\n"
    "bound obtained from the global tixel matching after time assignment;\n"
    "UB is the upper bound achieved by KHE14 (the total duration of missing\n"
    "room assignments, unavailable rooms, and clashes).  Dashes indicate\n"
    "instances where no room assignment is required.\n");
  KheStatsColAdd("tab:room:lbub", "LB", false);
  KheStatsColAdd("tab:room:lbub", "UB", false);

  /* solve the instance and record the results in the two tables */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:room:lbub", id, false);
    if( KheBenchmarkTryInstance(ins) )
    {
      if( DEBUG1 )
	fprintf(stderr, "solving instance \"%s\":\n", KheInstanceId(ins));

      if( KheInstanceRequiresRoomAssignment(ins) )
      {
	/* time assignment only, for lower bound */
	save_time_assignment_only = KheOptionsTimeAssignmentOnly(options);
	KheOptionsSetTimeAssignmentOnly(options, true);
	soln = KheSolnMake(ins);
	soln = solver(soln, options);
	KheStatsEntryAddInt("tab:room:lbub",id,"LB",KheTestDLowerBound(soln));
	KheSolnDelete(soln);
	KheOptionsSetTimeAssignmentOnly(options, save_time_assignment_only);

	/* full solution, for upper bound */
	soln = KheSolnMake(ins);
	soln = solver(soln, options);
	KheStatsEntryAddInt("tab:room:lbub",id,"UB",KheTestDUpperBound(soln));
	KheSolnDelete(soln);
      }
      else
      {
	/* no room assignment needed, print a row of dashes */
	KheStatsEntryAddString("tab:room:lbub", id, "LB", "-");
	KheStatsEntryAddString("tab:room:lbub", id, "UB", "-");
      }
    }
  }

  /* finish off the two tables */
  KheStatsTableEnd("tab:room:lbub");
  KheStatsFileEnd("tab:room:lbub");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestD returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestECase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id, bool resource_rematch,            */
/*    KHE_OPTIONS_RESOURCE_PAIR resource_pair)                               */
/*                                                                           */
/*  Carry out one case of Test 5.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTestECase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id, bool resource_rematch)
{
  char cbuff[10], tbuff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  KheOptionsSetResourceRematch(options, resource_rematch);
  sprintf(cbuff, "C:%cRM", resource_rematch ? '+' : '-');
  sprintf(tbuff, "T:%cRM", resource_rematch ? '+' : '-');
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), cbuff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins);
  soln = solver(soln, options);
  KheStatsEntryAddCost("tab:rm", id, cbuff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:rm", id, tbuff, KheStatsTimerNow(st));
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestE(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test E, producing these files:                             */
/*                                                                           */
/*  TestE                                                                    */
/*    tab:rm                      Tests of resource rematching               */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestE(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, o1;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  int opt1_count = 2;  bool opt1[] = {false, true};
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestE(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* set up costs table */
  KheStatsFileBegin("tab:rm");
  KheStatsTableBegin("tab:rm", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:rm",
    "Effectiveness of resource rematching:  -RM and +RM denote without it\n"
    "and with it.  KHE14 uses +RM.  Other details as previously.\n");
  KheStatsColAdd("tab:rm", "C:-RM", false);
  KheStatsColAdd("tab:rm", "C:+RM", true);
  KheStatsColAdd("tab:rm", "T:-RM", false);
  KheStatsColAdd("tab:rm", "T:+RM", false);

  /* solve the instance and record the results in the table */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:rm", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( o1 = 0;  o1 < opt1_count;  o1++ )
	KheTestECase(ins, options, solver, id, opt1[o1]);
  }

  /* finish off the table */
  KheStatsTableEnd("tab:rm");
  KheStatsFileEnd("tab:rm");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestE returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  char KheResourcePairChar(KHE_OPTIONS_RESOURCE_PAIR rp)                   */
/*                                                                           */
/*  Return a character denoting a value of rp:                               */
/*                                                                           */
/*    KHE_OPTIONS_RESOURCE_PAIR_NONE               '-'                       */
/*    KHE_OPTIONS_RESOURCE_PAIR_SPLITS             '+'                       */
/*    KHE_OPTIONS_RESOURCE_PAIR_PARTITIONS         '!'                       */
/*    KHE_OPTIONS_RESOURCE_PAIR_ALL                '*'                       */
/*                                                                           */
/*****************************************************************************/

static char KheResourcePairChar(KHE_OPTIONS_RESOURCE_PAIR rp)
{
  switch( rp )
  {
    case KHE_OPTIONS_RESOURCE_PAIR_NONE:	return '-';
    case KHE_OPTIONS_RESOURCE_PAIR_SPLITS:	return '+';
    case KHE_OPTIONS_RESOURCE_PAIR_PARTITIONS:	return '!';
    case KHE_OPTIONS_RESOURCE_PAIR_ALL:		return '*';

    default:

      MAssert(false, "KheResourcePairChar; unknown rp %d", rp);
      return '?';  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestFCase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id,                                   */
/*    KHE_OPTIONS_RESOURCE_PAIR resource_pair)                               */
/*                                                                           */
/*  Carry out one case of Test 6.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTestFCase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id, KHE_OPTIONS_RESOURCE_PAIR resource_pair)
{
  char cbuff[10], tbuff[10], xbuff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  int calls, /* successes, */ truncs;
  sprintf(cbuff, "C:%cPR", KheResourcePairChar(resource_pair));
  sprintf(tbuff, "T:%cPR", KheResourcePairChar(resource_pair));
  sprintf(xbuff, "X:%cPR", KheResourcePairChar(resource_pair));
  KheOptionsSetResourcePair(options, resource_pair);
  KheOptionsSetResourcePairCalls(options, 0);
  KheOptionsSetResourcePairSuccesses(options, 0);
  KheOptionsSetResourcePairTruncs(options, 0);
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), cbuff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins);
  soln = solver(soln, options);
  calls = KheOptionsResourcePairCalls(options);
  /* successes = KheOptionsResourcePairSuccesses(options); */
  truncs = KheOptionsResourcePairTruncs(options);
  KheStatsEntryAddCost("tab:pr", id, cbuff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:pr", id, tbuff, KheStatsTimerNow(st));
  if( (resource_pair == KHE_OPTIONS_RESOURCE_PAIR_SPLITS ||
       resource_pair == KHE_OPTIONS_RESOURCE_PAIR_ALL) && calls > 0 )
    KheStatsEntryAddTime("tab:pr", id, xbuff,
      ((float) truncs / (float) calls) * 100.0);
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestF(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test F, producing these files:                             */
/*                                                                           */
/*  TestF                                                                    */
/*    tab:pr                      Tests of resource pair repair              */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestF(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, o2;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  int opt2_count = 3;  KHE_OPTIONS_RESOURCE_PAIR opt2[] =
  {KHE_OPTIONS_RESOURCE_PAIR_NONE, KHE_OPTIONS_RESOURCE_PAIR_SPLITS,
   KHE_OPTIONS_RESOURCE_PAIR_ALL};
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestF(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* set up table */
  KheStatsFileBegin("tab:pr");
  KheStatsTableBegin("tab:pr", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:pr",
    "Effectiveness of resource pair repair:  -PR, +PR and *PR denote\n"
    "without it, with it for pairs of resources in split assignments only,\n"
    "and with it for all pairs of resources.  KHE14 uses +PR.  The last two\n"
    "columns report the percentage of calls where the search was truncated,\n"
    "losing the optimality guarantee.  Other details as previously.\n");
  KheStatsColAdd("tab:pr", "C:-PR", false);
  KheStatsColAdd("tab:pr", "C:+PR", false);
  KheStatsColAdd("tab:pr", "C:*PR", true);
  KheStatsColAdd("tab:pr", "T:-PR", false);
  KheStatsColAdd("tab:pr", "T:+PR", false);
  KheStatsColAdd("tab:pr", "T:*PR", true);
  KheStatsColAdd("tab:pr", "X:+PR", false);
  KheStatsColAdd("tab:pr", "X:*PR", false);

  /* solve the instance and record the results in the two tables */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:pr", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( o2 = 0;  o2 < opt2_count;  o2++ )
	KheTestFCase(ins, options, solver, id, opt2[o2]);
  }

  /* finish off the two tables */
  KheStatsTableEnd("tab:pr");
  KheStatsFileEnd("tab:pr");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestF returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestG(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test G, producing these files:                             */
/*                                                                           */
/*  TestG                                                                    */
/*    tab:chaintype               Tests using various ejection chain types   */
/*                                                                           */
/*****************************************************************************/

static bool KheTestGOptionsFn(int i, KHE_OPTIONS options, char **id)
{
  switch( i )
  {
    case 0:

      *id = "u-";
      KheOptionsSetEjectorSchedulesString(options, *id);
      return true;

    case 1:

      *id = "1+,u-";
      KheOptionsSetEjectorSchedulesString(options, *id);
      return true;

    case 2:

      *id = "1+,2+,u-";
      KheOptionsSetEjectorSchedulesString(options, *id);
      return true;

    default:

      *id = NULL;
      return false;
  }
}

static void KheBenchmarkTestG(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  KHE_OPTIONS options;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestG(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();
  KheBenchmarkTestOptions(archive, options, &KheTestGOptionsFn, 4, solver,
    "tab:chaintype", table_type,
    "Effectiveness of variants of KHE14's ejection chain algorithm.  Each\n"
    "pair of characters represents one complete restart of the algorithm:\n"
    "a digit denotes a maximum chain length (u means unlimited); + denotes\n"
    "allowing entities to be revisited along one chain, and - denotes not\n"
    "allowing it.  KHE14 uses 1+,u-.  Other details as previously.\n");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestG returning\n");
}


/* *** old implementation
static void KheBenchmarkTestG(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, j;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;  KHE_SOLN soln;
  char *opt[] = { "u-", "1+,u-", "1+,2+,u-", NULL };
  char cbuff[12], tbuff[12];  KHE_STATS_TIMER st;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestG(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  ** set up table **
  KheStatsFileBegin("tab:chaintype");
  KheStatsTableBegin("tab:chaintype", table_type,
    12, "Instance", true, false, true, true, true);
  KheStatsCaptionAdd("tab:chaintype",
    "Effectiveness of variants of KHE14's ejection chain algorithm.  Each\n"
    "pair of characters represents one complete restart of the algorithm:\n"
    "a digit denotes a maximum chain length (u means unlimited); + denotes\n"
    "allowing entities to be revisited along one chain, and - denotes not\n"
    "allowing it.  KHE14 uses 1+,u-.  Other details as previously.\n");
  for( j = 0;  opt[j] != NULL;  j++ )
  {
    sprintf(cbuff, "C:%s", opt[j]);
    KheStatsColAdd("tab:chaintype", cbuff, opt[j+1] == NULL);
  }
  for( j = 0;  opt[j] != NULL;  j++ )
  {
    sprintf(tbuff, "T:%s", opt[j]);
    KheStatsColAdd("tab:chaintype", tbuff, false);
  }

  ** solve the instance and record the results in the two tables **
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:chaintype", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( j = 0;  opt[j] != NULL;  j++ )
      {
	if( DEBUG1 )
	  fprintf(stderr, "solving instance \"%s\" %s:\n", id, opt[j]);
	KheOptionsSetEjectorSchedulesString(options, opt[j]);
	st = KheStatsTimerMake();
	soln = KheSolnMake(ins, NULL);
	soln = solver(soln, options);
	sprintf(cbuff, "C:%s", opt[j]);
	KheStatsEntryAddCost("tab:chaintype", id, cbuff, KheSolnCost(soln));
	sprintf(tbuff, "T:%s", opt[j]);
	KheStatsEntryAddTime("tab:chaintype", id, tbuff,KheStatsTimerNow(st));
	KheSolnDelete(soln);
	KheStatsTimerDelete(st);
      }
  }

  ** finish off the table **
  KheStatsTableEnd("tab:chaintype");
  KheStatsFileEnd("tab:chaintype");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestG returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteKempeStatsStepHisto(char *file_name,          */
/*    KHE_OPTIONS options, char *archive_id)                                 */
/*                                                                           */
/*  Write a histogram of Kempe meet move steps.                              */
/*                                                                           */
/*****************************************************************************/
#define MAX_STEPS_HISTO 20

static void KheBenchmarkTestHWriteKempeStatsStepHisto(char *file_name,
  KHE_OPTIONS options, char *archive_id)
{
  int i, len, overs;  KHE_KEMPE_STATS ks;
  KheStatsFileBegin(file_name);
  KheStatsGraphBegin(file_name);
  KheStatsGraphSetBelowCaption(file_name, "Number of steps");
  KheStatsGraphSetXMax(file_name, MAX_STEPS_HISTO + 1.0);
  KheStatsGraphSetWidth(file_name, 8.0);
  KheStatsCaptionAdd(file_name,
    "For each number of steps, the number of Kempe meet moves with that\n"
    "number of steps, over all instances of archive %s.\n", archive_id);
  ks = KheOptionsTimeKempeStats(options);
  if( KheKempeStatsStepHistoMax(ks) > 0 )
  {
    KheStatsCaptionAdd(file_name,
      "There were %d Kempe meet moves altogether, and their average number\n"
      "of steps was %.1f.\n", KheKempeStatsStepHistoTotal(ks),
      KheKempeStatsStepHistoAverage(ks));
    if( KheKempeStatsStepHistoMax(ks) > MAX_STEPS_HISTO )
      KheStatsCaptionAdd(file_name,
	"All Kempe meet moves with more than %d steps are shown as\n"
	"having %d steps.  The longest Kempe meet move had %d steps.\n",
        MAX_STEPS_HISTO, MAX_STEPS_HISTO, KheKempeStatsStepHistoMax(ks));
  }
  KheStatsDataSetAdd(file_name, "all", KHE_STATS_DATASET_HISTO);
  overs = 0;
  for( i = 1;  i <= KheKempeStatsStepHistoMax(ks);  i++ )
  {
    len = KheKempeStatsStepHistoFrequency(ks, i);
    if( i < MAX_STEPS_HISTO )
      KheStatsPointAdd(file_name, "all", (float) i, (float) len);
    else
      overs += len;
  }
  if( overs > 0 )
    KheStatsPointAdd(file_name, "all", (float) MAX_STEPS_HISTO, (float) overs);
  KheStatsGraphEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteKempeStatsPhaseHisto(char *file_name,         */
/*    KHE_OPTIONS options, char *archive_id)                                 */
/*                                                                           */
/*  Write a histogram of Kempe meet move phases.                             */
/*                                                                           */
/*****************************************************************************/
#define MAX_PHASES_HISTO 20

static void KheBenchmarkTestHWriteKempeStatsPhaseHisto(char *file_name,
  KHE_OPTIONS options, char *archive_id)
{
  int i, len, overs;  KHE_KEMPE_STATS ks;
  KheStatsFileBegin(file_name);
  KheStatsGraphBegin(file_name);
  KheStatsGraphSetBelowCaption(file_name, "Number of phases");
  KheStatsGraphSetXMax(file_name, MAX_PHASES_HISTO + 1.0);
  KheStatsGraphSetWidth(file_name, 8.0);
  KheStatsCaptionAdd(file_name,
    "For each number of phases, the number of Kempe meet moves with that\n"
    "number of phases, over all instances of archive %s.\n", archive_id);
  ks = KheOptionsTimeKempeStats(options);
  if( KheKempeStatsPhaseHistoMax(ks) > 0 )
  {
    KheStatsCaptionAdd(file_name,
      "There were %d Kempe meet moves altogether, and their average number\n"
      "of phases was %.1f.\n", KheKempeStatsPhaseHistoTotal(ks),
      KheKempeStatsPhaseHistoAverage(ks));
    if( KheKempeStatsPhaseHistoMax(ks) > MAX_PHASES_HISTO )
      KheStatsCaptionAdd(file_name,
	"All Kempe meet moves with more than %d phases are shown as\n"
	"having %d phases.  The longest Kempe meet move had %d phases.\n",
        MAX_PHASES_HISTO, MAX_PHASES_HISTO, KheKempeStatsPhaseHistoMax(ks));
  }
  KheStatsDataSetAdd(file_name, "all", KHE_STATS_DATASET_HISTO);
  overs = 0;
  for( i = 1;  i <= KheKempeStatsPhaseHistoMax(ks);  i++ )
  {
    len = KheKempeStatsPhaseHistoFrequency(ks, i);
    if( i < MAX_PHASES_HISTO )
      KheStatsPointAdd(file_name, "all", (float) i, (float) len);
    else
      overs += len;
  }
  if( overs > 0 )
    KheStatsPointAdd(file_name, "all", (float) MAX_PHASES_HISTO, (float) overs);
  KheStatsGraphEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteImprovementRepairHisto(char *file_name,       */
/*    char *type, KHE_EJECTOR ej, char *archive_id)                          */
/*                                                                           */
/*  Write a histogram of improvement repair counts.                          */
/*                                                                           */
/*****************************************************************************/
#define MAX_REPAIR_HISTO 39

static void KheBenchmarkTestHWriteImprovementRepairHisto(char *file_name,
  char *type, KHE_EJECTOR ej, char *archive_id)
{
  int i, len, overs;
  KheStatsFileBegin(file_name);
  KheStatsGraphBegin(file_name);
  KheStatsGraphSetBelowCaption(file_name, "Number of repairs");
  KheStatsGraphSetXMax(file_name, MAX_REPAIR_HISTO + 1.0);
  KheStatsGraphSetWidth(file_name, 8.0);
  KheStatsCaptionAdd(file_name,
    "For each number of repairs, the number of improvements (successful\n"
    "chains or trees) with that number of repairs found during %s repair,\n"
    "over all instances of archive %s.\n", type, archive_id);
  if( KheEjectorImprovementRepairHistoMax(ej) > 0 )
  {
    KheStatsCaptionAdd(file_name,
      "There were %d improvements altogether, and their average number\n"
      "of repairs was %.1f.\n", KheEjectorImprovementRepairHistoTotal(ej),
      KheEjectorImprovementRepairHistoAverage(ej));
    if( KheEjectorImprovementRepairHistoMax(ej) > MAX_REPAIR_HISTO )
      KheStatsCaptionAdd(file_name,
	"All improvements with more than %d repairs are shown as\n"
	"having %d repairs.  The longest improvement had %d repairs.\n",
        MAX_REPAIR_HISTO, MAX_REPAIR_HISTO, 
	KheEjectorImprovementRepairHistoMax(ej));
  }
  KheStatsDataSetAdd(file_name, "all", KHE_STATS_DATASET_HISTO);
  overs = 0;
  for( i = 1;  i <= KheEjectorImprovementRepairHistoMax(ej);  i++ )
  {
    len = KheEjectorImprovementRepairHistoFrequency(ej, i);
    if( i < MAX_REPAIR_HISTO )
      KheStatsPointAdd(file_name, "all", (float) i, (float) len);
    else
      overs += len;
  }
  if( overs > 0 )
    KheStatsPointAdd(file_name, "all", (float) MAX_REPAIR_HISTO, (float) overs);
  KheStatsGraphEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteImprovementAugmentHisto(char *file_name,      */
/*    char *type, KHE_EJECTOR ej, char *archive_id)                          */
/*                                                                           */
/*  Write a histogram of improvement augment counts.                         */
/*                                                                           */
/*****************************************************************************/
#define MAX_AUGMENT_HISTO 200

static void KheBenchmarkTestHWriteImprovementAugmentHisto(char *file_name,
  char *type, KHE_EJECTOR ej, char *archive_id)
{
  int i, len, overs;
  KheStatsFileBegin(file_name);
  KheStatsGraphBegin(file_name);
  KheStatsGraphSetBelowCaption(file_name, "Number of augments");
  KheStatsGraphSetXMax(file_name, MAX_AUGMENT_HISTO + 1.0);
  KheStatsGraphSetWidth(file_name, 8.0);
  KheStatsCaptionAdd(file_name,
    "For each number of augments, the number of improvements (successful\n"
    "chains or trees) found during %s repair after that many augments, over\n"
    "all instances of archive %s.\n", type, archive_id);
  if( KheEjectorImprovementAugmentHistoMax(ej) > 0 )
  {
    KheStatsCaptionAdd(file_name,
      "There were %d improvements altogether, and their average number\n"
      "of augments was %.1f.\n", KheEjectorImprovementAugmentHistoTotal(ej),
      KheEjectorImprovementAugmentHistoAverage(ej));
    if( KheEjectorImprovementAugmentHistoMax(ej) > MAX_AUGMENT_HISTO )
      KheStatsCaptionAdd(file_name,
	"All improvements with more than %d augments are shown as\n"
	"having %d augments.  The maximum number of augments was %d.\n",
	MAX_AUGMENT_HISTO, MAX_AUGMENT_HISTO,
	KheEjectorImprovementAugmentHistoMax(ej));
  }
  KheStatsDataSetAdd(file_name, "all", KHE_STATS_DATASET_HISTO);
  overs = 0;
  for( i = 1;  i <= KheEjectorImprovementAugmentHistoMax(ej);  i++ )
  {
    len = KheEjectorImprovementAugmentHistoFrequency(ej, i);
    if( i < MAX_AUGMENT_HISTO )
      KheStatsPointAdd(file_name, "all", (float) i, (float) len);
    else
      overs += len;
  }
  if( overs > 0 )
    KheStatsPointAdd(file_name,"all", (float) MAX_AUGMENT_HISTO, (float) overs);
  KheStatsGraphEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteChainAugmentTable(char *file_name,            */
/*    KHE_STATS_TABLE_TYPE table_type, char *type, KHE_EJECTOR ej,           */
/*    char *archive_id)                                                      */
/*                                                                           */
/*  Write a table of ejection chain augment function successes.              */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestHWriteChainAugmentTable(char *file_name,
  KHE_STATS_TABLE_TYPE table_type, char *type, KHE_EJECTOR ej, char *archive_id)
{
  int augment_type, i, total, successful;  char *augment_label;
  KheStatsFileBegin(file_name);
  KheStatsTableBegin(file_name, table_type,
    12, "Augment function", false, false, false, false, false);
  KheStatsColAdd(file_name, "Total", false);
  KheStatsColAdd(file_name, "Successful", false);
  KheStatsColAdd(file_name, "Percent", false);
  KheStatsCaptionAdd(file_name,
    "Effectiveness of augment functions during %s repair.\n"
    "For each augment function, the number of calls to the function\n"
    "during %s repair, the number of successful calls, and the ratio\n"
    "of the two as a percentage, over all instances of archive %s.\n"
    "Only non-zero rows are shown.\n", type, type, archive_id);
  for( i = 0;  i < KheEjectorAugmentTypeCount(ej);  i++ )
  {
    /* one row for each augment type */
    augment_type = KheEjectorAugmentType(ej, i);
    augment_label = KheEjectorAugmentTypeLabel(ej, augment_type);
    total = KheEjectorTotalRepairs(ej, augment_type);
    successful = KheEjectorSuccessfulRepairs(ej, augment_type);
    if( total > 0 )
    {
      KheStatsRowAdd(file_name, augment_label, false);
      KheStatsEntryAddInt(file_name, augment_label, "Total", total);
      KheStatsEntryAddInt(file_name, augment_label, "Successful", successful);
      KheStatsEntryAddTime(file_name, augment_label, "Percent",
	((float) successful / (float) total) * 100.0);
    }
  }
  KheStatsTableEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestHWriteChainRepairTable(char *file_name,             */
/*    KHE_STATS_TABLE_TYPE table_type, char *type, KHE_EJECTOR ej,           */
/*    char *archive_id)                                                      */
/*                                                                           */
/*  Write a table of augment function successes.                             */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestHWriteChainRepairTable(char *file_name,
  KHE_STATS_TABLE_TYPE table_type, char *type, KHE_EJECTOR ej, char *archive_id)
{
  int augment_type, repair_type, i, j, total, successful;
  char *augment_label, *repair_label, buff[300];
  KheStatsFileBegin(file_name);
  KheStatsTableBegin(file_name, table_type, 12,
    "Augment function : Repair operation", false, false, false, false, false);
  KheStatsColAdd(file_name, "Total", false);
  KheStatsColAdd(file_name, "Successful", false);
  KheStatsColAdd(file_name, "Percent", false);
  KheStatsCaptionAdd(file_name,
    "Effectiveness of repair operations during %s repair.  For\n"
    "each augment function and repair operation, the number of calls on that\n"
    "repair operation made by that augment function during %s repair, the\n"
    "number of successful calls, and the ratio of the two as a percentage,\n"
    "over all instances of archive %s.  Only non-zero rows are shown.\n",
    type, type, archive_id);
  for( i = 0;  i < KheEjectorAugmentTypeCount(ej);  i++ )
  {
    /* one row for each augment type/repair method pair */
    augment_type = KheEjectorAugmentType(ej, i);
    augment_label = KheEjectorAugmentTypeLabel(ej, augment_type);
    for( j = 0;  j < KheEjectorRepairTypeCount(ej);  j++ )
    {
      /* repair_type = KheEjectorAugmentTypeRepairType(ej, augment_type, j); */
      repair_type = KheEjectorRepairType(ej, j);
      repair_label = KheEjectorRepairTypeLabel(ej, repair_type);
      total = KheEjectorTotalRepairsByType(ej, augment_type, repair_type);
      successful = KheEjectorSuccessfulRepairsByType(ej, augment_type,
	repair_type);
      if( total > 0 )
      {
	snprintf(buff, 300, "%s : %s", augment_label, repair_label);
	KheStatsRowAdd(file_name, buff, false);
	KheStatsEntryAddInt(file_name, buff, "Total", total);
	KheStatsEntryAddInt(file_name, buff, "Successful", successful);
	KheStatsEntryAddTime(file_name, buff, "Percent",
	  ((float) successful / (float) total) * 100.0);
      }
    }
  }
  KheStatsTableEnd(file_name);
  KheStatsFileEnd(file_name);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestH(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test H, producing these files:                             */
/*                                                                           */
/*  TestH                                                                    */
/*    figure:kempe:steps          Kempe meet moves - number of steps         */
/*    figure:kempe:phases         Kempe meet moves - number of phases        */
/*    figure:chainlength:time     Ejection chain lengths (time repair)       */
/*    figure:chainlength:resource Ejection chain lengths (resource repair)   */
/*    tab:chainaugment:time       Ejection chain augments (time repair)      */
/*    tab:chainaugment:resource   Ejection chain augments (resource repair)  */
/*    tab:chainrepair:time        Ejection chain repairs (time repair)       */
/*    tab:chainrepair:resource    Ejection chain repairs (resource repair)   */
/*                                                                           */
/*  Implementation note.  Parallelizing this test would be non-trivial,      */
/*  because the statistics accumulate in the ejectors.  A separate ejector   */
/*  would be needed for each thread, and the statistics from all the         */
/*  ejectors would need to be combined.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestH(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;  KHE_SOLN soln;
  KHE_EJECTOR time_ej, resource_ej;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestH(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* add a time ejector and a resource ejector to options */
  time_ej = KheEjectionChainEjectorMake(options);
  KheOptionsSetEjector(options, 0, time_ej);
  KheOptionsSetEjector(options, 1, time_ej);
  resource_ej = KheEjectionChainEjectorMake(options);
  KheOptionsSetEjector(options, 2, resource_ej);

  /* solve each instance once; the statistics accumulate in the ejectors */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    if( KheBenchmarkTryInstance(ins) )
    {
      if( DEBUG1 )
	fprintf(stderr, "solving instance \"%s\":\n", id);
      soln = KheSolnMake(ins);
      soln = solver(soln, options);
      KheSolnDelete(soln);
    }
  }

  /* write the graphs and tables */
  id = KheArchiveId(archive);
  KheBenchmarkTestHWriteKempeStatsStepHisto("figure:kempe:steps", options, id);
  KheBenchmarkTestHWriteKempeStatsPhaseHisto("figure:kempe:phases", options,id);
  KheBenchmarkTestHWriteImprovementRepairHisto("figure:chainlength:time",
    "time", time_ej, id);
  KheBenchmarkTestHWriteImprovementRepairHisto("figure:chainlength:resource",
    "resource", resource_ej, id);
  KheBenchmarkTestHWriteImprovementAugmentHisto("figure:augment:time",
    "time", time_ej, id);
  KheBenchmarkTestHWriteImprovementAugmentHisto("figure:augment:resource",
    "resource", resource_ej, id);
  KheBenchmarkTestHWriteChainAugmentTable("tab:chainaugment:time",
    table_type, "time", time_ej, id);
  KheBenchmarkTestHWriteChainAugmentTable("tab:chainaugment:resource",
    table_type, "resource", resource_ej, id);
  KheBenchmarkTestHWriteChainRepairTable("tab:chainrepair:time",
    table_type, "time", time_ej, id);
  KheBenchmarkTestHWriteChainRepairTable("tab:chainrepair:resource",
    table_type, "resource", resource_ej, id);
  KheEjectorDelete(time_ej);
  KheEjectorDelete(resource_ej);
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestH returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestICase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id, bool time_ejecting_not_basic)     */
/*                                                                           */
/*  Carry out one case of Test 9.                                            */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheTestICase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id, bool time_use_kempe_moves,
  bool time_ejecting_not_basic)
{
  char cbuff[10], tbuff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  KheOptionsSetEjectorUseKempeMoves(options, time_use_kempe_moves ?
    KHE_OPTIONS_KEMPE_YES : KHE_OPTIONS_KEMPE_NO);
  KheOptionsSetEjectorEjectingNotBasic(options, time_ejecting_not_basic);
  sprintf(cbuff, "C:%c%c", time_use_kempe_moves ? 'K' : 'X',
    time_ejecting_not_basic ? 'E' : 'B');
  sprintf(tbuff, "T:%c%c", time_use_kempe_moves ? 'K' : 'X',
    time_ejecting_not_basic ? 'E' : 'B');
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), cbuff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins, NULL);
  soln = solver(soln, options);
  KheStatsEntryAddCost("tab:moves", id, cbuff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:moves", id, tbuff, KheStatsTimerNow(st));
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestI(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test I, producing these files:                             */
/*                                                                           */
/*  TestI                                                                    */
/*    tab:moves                   Tests of Kempe, ejecting, and basic moves  */
/*                                                                           */
/*****************************************************************************/

static bool KheTestIOptionsFn(int i, KHE_OPTIONS options, char **id)
{
  switch( i )
  {
    case 0:

      KheOptionsSetEjectorUseKempeMoves(options, KHE_OPTIONS_KEMPE_YES);
      KheOptionsSetEjectorEjectingNotBasic(options, true);
      *id = "KE";
      return true;

    case 1:

      KheOptionsSetEjectorUseKempeMoves(options, KHE_OPTIONS_KEMPE_YES);
      KheOptionsSetEjectorEjectingNotBasic(options, false);
      *id = "KB";
      return true;

    case 2:

      KheOptionsSetEjectorUseKempeMoves(options, KHE_OPTIONS_KEMPE_NO);
      KheOptionsSetEjectorEjectingNotBasic(options, true);
      *id = "XE";
      return true;

    case 3:

      KheOptionsSetEjectorUseKempeMoves(options, KHE_OPTIONS_KEMPE_NO);
      KheOptionsSetEjectorEjectingNotBasic(options, false);
      *id = "XB";
      return true;

    default:

      *id = NULL;
      return false;
  }
}

static void KheBenchmarkTestI(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  KHE_OPTIONS options;
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestI(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();
  KheBenchmarkTestOptions(archive, options, &KheTestIOptionsFn, 4, solver,
    "tab:moves", table_type,
    "Kempe, ejecting, and basic moves during time assignment.  Where the\n"
    "main text states that Kempe meet moves are tried, K means to try them\n"
    "and X means to omit them.  Where it states that ejecting meet moves\n"
    "are tried, E means to try them and B means to try basic meet moves\n"
    "instead.  KHE14 uses KE.  Other details as previously.\n");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestI returning\n");
}


/* *** old implementation
static void KheBenchmarkTestI(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, o1, o2;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  int opt1_count = 2;  bool opt1[] = {false, true};
  int opt2_count = 2;  bool opt2[] = {false, true};
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestI(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  ** set up costs table **
  KheStatsFileBegin("tab:moves");
  KheStatsTableBegin("tab:moves", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:moves",
    "Kempe, ejecting, and basic moves during time assignment.  Where the\n"
    "main text states that Kempe meet moves are tried, K means to try them\n"
    "and X means to omit them.  Where it states that ejecting meet moves\n"
    "are tried, E means to try them and B means to try basic meet moves\n"
    "instead.  KHE14 uses KE.  Other details as previously.\n");
  KheStatsColAdd("tab:moves", "C:KE", false);
  KheStatsColAdd("tab:moves", "C:KB", false);
  KheStatsColAdd("tab:moves", "C:XE", false);
  KheStatsColAdd("tab:moves", "C:XB", true);
  KheStatsColAdd("tab:moves", "T:KE", false);
  KheStatsColAdd("tab:moves", "T:KB", false);
  KheStatsColAdd("tab:moves", "T:XE", false);
  KheStatsColAdd("tab:moves", "T:XB", false);

  ** solve the instance and record the results in the table **
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:moves", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( o1 = 0;  o1 < opt1_count;  o1++ )
	for( o2 = 0;  o2 < opt2_count;  o2++ )
	KheTestICase(ins, options, solver, id, opt1[o1], opt2[o2]);
  }

  ** finish off the table **
  KheStatsTableEnd("tab:moves");
  KheStatsFileEnd("tab:moves");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestI returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheTestJCase(KHE_INSTANCE ins, KHE_OPTIONS options,                 */
/*    KHE_GENERAL_SOLVER solver, char *id, bool time_ejecting_not_basic)     */
/*                                                                           */
/*  Carry out one case of Test 9.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTestJCase(KHE_INSTANCE ins, KHE_OPTIONS options,
  KHE_GENERAL_SOLVER solver, char *id, bool time_nodes_before_meets)
{
  char cbuff[10], tbuff[10];  KHE_SOLN soln;  KHE_STATS_TIMER st;
  KheOptionsSetEjectorNodesBeforeMeets(options, time_nodes_before_meets);
  sprintf(cbuff, "C:%s", time_nodes_before_meets ? "NM" : "MN");
  sprintf(tbuff, "T:%s", time_nodes_before_meets ? "NM" : "MN");
  if( DEBUG1 )
    fprintf(stderr, "solving instance \"%s\" %s:\n", KheInstanceId(ins), cbuff);
  st = KheStatsTimerMake();
  soln = KheSolnMake(ins);
  soln = solver(soln, options);
  KheStatsEntryAddCost("tab:nodesfirst", id, cbuff, KheSolnCost(soln));
  KheStatsEntryAddTime("tab:nodesfirst", id, tbuff, KheStatsTimerNow(st));
  KheSolnDelete(soln);
  KheStatsTimerDelete(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmarkTestJ(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,   */
/*    char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type) */
/*                                                                           */
/*  Run benchmark test J, producing these files:                             */
/*                                                                           */
/*  TestJ                                                                    */
/*    tab:nodesfirst              Tests of nodes before meets                */
/*                                                                           */
/*****************************************************************************/

static void KheBenchmarkTestJ(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, KHE_STATS_TABLE_TYPE table_type)
{
  int i, o1;  KHE_INSTANCE ins;  KHE_OPTIONS options;  char *id;
  int opt1_count = 2;  bool opt1[] = {false, true};
  if( DEBUG1 )
    fprintf(stderr, "[ KheBenchmarkTestJ(archive, solver, \"%s\")\n",
      solver_name);
  options = KheOptionsMake();

  /* set up costs table */
  KheStatsFileBegin("tab:nodesfirst");
  KheStatsTableBegin("tab:nodesfirst", table_type,
    12, "Instance", true, false, true, false, false);
  KheStatsCaptionAdd("tab:nodesfirst",
    "Node swaps before meet moves.  MN means Kempe meet moves first, NM\n"
    "means node swaps first.  KHE uses MN.  Other details as previously.\n");
  KheStatsColAdd("tab:nodesfirst", "C:MN", false);
  KheStatsColAdd("tab:nodesfirst", "C:NM", true);
  KheStatsColAdd("tab:nodesfirst", "T:MN", false);
  KheStatsColAdd("tab:nodesfirst", "T:NM", false);

  /* solve the instance and record the results in the table */
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    id = KheInsId(ins, i + 1);
    KheStatsRowAdd("tab:nodesfirst", id, false);
    if( KheBenchmarkTryInstance(ins) )
      for( o1 = 0;  o1 < opt1_count;  o1++ )
	KheTestJCase(ins, options, solver, id, opt1[o1]);
  }

  /* finish off the table */
  KheStatsTableEnd("tab:nodesfirst");
  KheStatsFileEnd("tab:nodesfirst");
  KheOptionsDelete(options);
  if( DEBUG1 )
    fprintf(stderr, "] KheBenchmarkTestJ returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBenchmark(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,        */
/*    char *solver_name, char *author_name, char test_label,                 */
/*    KHE_STATS_TABLE_TYPE table_type)                                       */
/*                                                                           */
/*  Run benchmark test test_label on solver, whose brief name is solver_name */
/*  and whose author is author_name.                                         */
/*                                                                           */
/*****************************************************************************/

void KheBenchmark(KHE_ARCHIVE archive, KHE_GENERAL_SOLVER solver,
  char *solver_name, char *author_name, char test_label,
  KHE_STATS_TABLE_TYPE table_type)
{
  switch( test_label )
  {
    case 'A':
      KheBenchmarkTestA(archive, solver, solver_name, author_name, table_type);
      break;

    case 'B':
      KheBenchmarkTestB(archive, solver, solver_name, author_name, table_type);
      break;

    case 'C':
      KheBenchmarkTestC(archive, solver, solver_name, author_name, table_type);
      break;

    case 'D':
      KheBenchmarkTestD(archive, solver, solver_name, author_name, table_type);
      break;

    case 'E':
      KheBenchmarkTestE(archive, solver, solver_name, author_name, table_type);
      break;

    case 'F':
      KheBenchmarkTestF(archive, solver, solver_name, author_name, table_type);
      break;

    case 'G':
      KheBenchmarkTestG(archive, solver, solver_name, author_name, table_type);
      break;

    case 'H':
      KheBenchmarkTestH(archive, solver, solver_name, author_name, table_type);
      break;

    case 'I':
      KheBenchmarkTestI(archive, solver, solver_name, author_name, table_type);
      break;

    case 'J':
      KheBenchmarkTestJ(archive, solver, solver_name, author_name, table_type);
      break;

    default:

      MAssert(false, "KheBenchmark: unknown test_label '%c'", test_label);
      break;
  }
}
