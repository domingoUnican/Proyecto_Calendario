
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
/*  FILE:         khe_main.c                                                 */
/*  DESCRIPTION:  Tests of KHE                                               */
/*                                                                           */
/*  If you are having trouble compiling this file, try replacing the 1       */
/*  by 0 in line "#define KHE_USE_TIMING 1" in khe.h.                        */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "khe_partition.h"
#include "khe_lset.h"

#if KHE_USE_TIMING
#include <time.h>
#include <sys/time.h>
#endif

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG_STATS 0
#define KHE_SOLE_DIVERSIFIER 0

/*****************************************************************************/
/*                                                                           */
/*  bool KheDivSameFn(int (*fn)(int, int), int d, int *other_d)              */
/*                                                                           */
/*  If fn(d, *) is the same as fn(a, *) for some a < c, return true with     */
/*  *other_d set to the first such a.                                        */
/*                                                                           */
/*****************************************************************************/

static bool KheDivSameFn(int (*fn)(int, int), int cols, int d, int *other_d)
{
  int k, c, same_count;
  for( k = 0;  k < d;  k++ )
  {
    same_count = 0;
    for( c = 1;  c <= cols;  c++ )
      if( fn(d, c) == fn(k, c) )
	same_count++;
    if( same_count == cols )
    {
      *other_d = k;
      return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDivColTest(int (*fn)(int, int), int cols, int indent, FILE *fp)  */
/*                                                                           */
/*  Print a test of fn onto fp with the given indent.                        */
/*                                                                           */
/*****************************************************************************/

static void KheDivColTest(int (*fn)(int, int), int cols, int *same_count,
  int indent, FILE *fp)
{
  int c, d, other_d, rows;
  fprintf(fp, "%*s[ %d cols\n", indent, "", cols);
  fprintf(fp, "%*s  %3s |", indent, "", "d");
  for( c = 1;  c <= cols;  c++ )
    fprintf(fp, "%3d", c);
  fprintf(fp, "\n");
  fprintf(fp, "%*s  ----+", indent, "");
  rows = 1;
  for( c = 1;  c <= cols;  c++ )
  {
    rows *= c;
    fprintf(fp, "---");
  }
  fprintf(fp, "\n");
  for( d = 0;  d < rows;  d++ )
  {
    fprintf(fp, "%*s  %3d |", indent, "", d);
    for( c = 1;  c <= cols;  c++ )
      fprintf(fp, "%3d", fn(d, c));
    if( KheDivSameFn(fn, cols, d, &other_d) )
    {
      fprintf(fp, "  (same as %d)", other_d);
      (*same_count)++;
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "%*s  ----+", indent, "");
  for( c = 1;  c <= cols;  c++ )
    fprintf(fp, "---");
  fprintf(fp, "\n");
  fprintf(fp, "%*s]\n", indent, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDivTest(int (*fn)(int, int), char *label, int indent, FILE *fp)  */
/*                                                                           */
/*****************************************************************************/

static void KheDivTest(int (*fn)(int, int), char *label, int max_cols,
  int indent, FILE *fp)
{
  int cols, same_count;
  fprintf(fp, "%*s[ Function %s\n", indent, "", label);
  same_count = 0;
  for( cols = 1;  cols <= max_cols;  cols++ )
  {
    if( cols > 1 )
      fprintf(fp, "\n");
    KheDivColTest(fn, cols, &same_count, indent + 2, fp);
  }
  fprintf(fp, "%*s] (same_count %d)\n", indent, "", same_count);
}


/*****************************************************************************/
/*                                                                           */
/*  Various diversification functions, for testing                           */
/*                                                                           */
/*****************************************************************************/

static int KheMod(int d, int c) { return d % c; }
static int KheOffset3Mod(int d, int c) { return (d + ((c >= 4) && (d >= 12))) % c; }
static int KheFact(int d, int c)
{
  int i, c1f;
  c1f = 1;
  for( i = 1;  i < c && c1f <= d;  i++ )
    c1f *= i;
  return (d / c1f) % c;
}
static int KheFactMod(int d, int c)
{
  int i, c1f;
  c1f = 1;
  for( i = 1;  i < c && c1f <= d;  i++ )
    c1f *= i;
  return ((d / c1f) + (d % c1f)) % c;
}
/* ***
static int KheOffset4Mod(int d, int c) {
  return (d + ((c >= 4) && (d >= 12)) + ((c >= 5) && (d >= 60))) % c; }
static int KheOffset2Mod(int d, int c) { return (d + (c >= 4)) % c; }
static int KhePrimeMod(int d, int c) { return (103 * d) % c; }
static int KheSquareMod(int d, int c) { return (d * d) % c; }
static int KheOffset3Mod(int d, int c) { return (d + (c >= 5)) % c; }
static int KheDiv(int d, int c) { return ((d + 1) / c) % c; }
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDiversificationTest(FILE *fp)                                    */
/*                                                                           */
/*  Test some diversification functions, and show the results on fp.         */
/*                                                                           */
/*****************************************************************************/

static void KheDiversificationTest(int max_cols, FILE *fp)
{
  KheDivTest(&KheMod, "d % c", max_cols, 2, fp);
  fprintf(fp, "\n");
  KheDivTest(&KheOffset3Mod, "(d + ((c >= 4) && (d >= 12))) % c",
    max_cols, 2,fp);
  KheDivTest(&KheFact, "(d / fact(c-1)) % c", max_cols, 2,fp);
  KheDivTest(&KheFactMod, "((d / fact(c - 1)) + (d % fact(c - 1))) % c",
    max_cols, 2,fp);
  /* ***
  fprintf(fp, "\n");
  KheDivTest(&KheOffset4Mod,
    "(d + ((c >= 4) && (d >= 12)) + ((c >= 5) && (d >= 60))) % c",
    max_cols, 2,fp);
  KheDivTest(&KheOffset2Mod, "(d + (c >= 4)) % c", max_cols, 2, fp);
  KheDivTest(&KhePrimeMod, "(103 * d) % c", max_cols, 2, fp);
  KheDivTest(&KheSquareMod, "(d * d) % c", max_cols, 2, fp);
  KheDivTest(&KheDiv, "((d + 1) / c) % c", 2, fp);
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE ReadArchive(char *fname)                                     */
/*                                                                           */
/*  Read an archive from fname, returning only if successful.                */
/*                                                                           */
/*****************************************************************************/

static KHE_ARCHIVE ReadArchive(char *fname)
{
  FILE *fp;  KHE_ARCHIVE res;  KML_ERROR ke;
  char *leftover;  int leftover_len;
  fp = fopen(fname, "r");
  if( fp == NULL )
  {
    fprintf(stderr, "khe: cannot open file \"%s\" for reading\n", fname);
    exit(1);
  }
  if( !KheArchiveRead(fp, &res, &ke, true, false,&leftover,&leftover_len,NULL) )
  {
    fprintf(stderr, "%s:%d:%d: %s\n", fname, KmlErrorLineNum(ke),
      KmlErrorColNum(ke), KmlErrorString(ke));
    exit(1);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUsageMessageAndExit(void)                                        */
/*                                                                           */
/*  Print a usage message and exit.                                          */
/*                                                                           */
/*****************************************************************************/
#define p(s) fprintf(stderr, s "\n")

static void KheUsageMessageAndExit(void)
{
  p("usage:");
  p("");
  p("  khe -b                      Test exponential backoff");
  p("  khe -c archive              Test KheSolnCopy() on archive");
  p("  khe -d                      Test diversification functions");
  p("  khe -e archive              Explode archive into one per instance");
  p("  khe -E archive              Explode archive into one per country");
  p("  khe -g<letters> archive     Solve archive and generate Lout stats");
  p("  khe -h<letters> archive     Solve archive and generate LaTeX stats");
  p("  khe -l                      Test the internal lset module");
  p("  khe -m                      Test the M module");
  p("  khe -p <a> <b> <c> archive  Solve archive, write to stdout");
  p("  khe -s<num> archive         Solve archive, write to stdout");
  p("  khe [ -t<num> ] archive     Like -s, but write nothing to stdout");
  p("  khe [ -u ]                  Write this usage message to stderr");
  p("  khe -v                      Write version number to stderr");
  p("  khe -w archive              Write archive to stdout");
  p("");
  p("where archive an XML timetable archive file.");
  p("");
  p("For each letter in <letters>, the -g and -h flags run the test labelled");
  p("by that letter on archive, which solves the archive (possibly more than");
  p("once), and generates some files into directory \"stats\" in the current");
  p("directory.  This directory must exist.  The files written depend on the");
  p("test label.  Each letter of <letters> is taken as a separate label, and");
  p("every test specified by one letter is run.");
  p("");
  p("For -p, <a> is the number of parallel threads to run; <b> is the");
  p("number of diversified solutions to make for each instance; and <c>");
  p("is the number of diversified solutions to keep for each instance");
  p("(the best <c> solutions are kept).");
  p("");
  p("For -s and -t, <num> is an optional positive integer; if it is present");
  p("KheParallelSolve is called with <num> for thread_count.");
  exit(1);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP KheTestMakeSolnGroup(KHE_ARCHIVE archive)                 */
/*                                                                           */
/*  Make a new, empty soln group for archive to hold the solns produced      */
/*  by this test run.                                                        */
/*                                                                           */
/*****************************************************************************/

static KHE_SOLN_GROUP KheTestMakeSolnGroup(KHE_ARCHIVE archive)
{
#if KHE_USE_TIMING
  struct timeval tv;
#endif
  static char id[200], date_str[200];  int i;
  KHE_SOLN_GROUP res;  KHE_SOLN_GROUP_METADATA md;

  /* construct an Id that is not already in use */
  sprintf(id, "KHE_%s", KHE_VERSION);
  for( i = 1;  KheArchiveRetrieveSolnGroup(archive, id, &res);  i++ )
    sprintf(id, "KHE_%s_%d", KHE_VERSION, i);

  /* construct metadata */
#if KHE_USE_TIMING
  gettimeofday(&tv, NULL);
  strftime(date_str, 100, "%d %B %Y", localtime(&(tv.tv_sec)));
#else
  strcpy(date_str, "no date");
#endif
  md = KheSolnGroupMetaDataMake("Jeffrey H. Kingston",
    date_str[0] == '0' ? &date_str[1] : date_str,
    "Produced by KHE Version " KHE_VERSION, NULL, NULL);

  /* make and return a new soln group */
  if( !KheSolnGroupMake(archive, id, md, &res) )
    MAssert(false, "KheTestMakeSolnGroup internal error");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestSolveInstances(KHE_ARCHIVE archive, int thread_count,        */
/*    KHE_SOLN_GROUP soln_group, int sole_diversifier)                       */
/*                                                                           */
/*  Solve the instances of archive, using multiple threads if thread_count   */
/*  is non-zero; and if soln_group is non-NULL, add the solutions to it.     */
/*  If thread_count == 0, set the diversifier of the sole solution created   */
/*  to sole_diversifier.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheTestSolveInstances(KHE_ARCHIVE archive, int make_solns,
  KHE_SOLN_GROUP soln_group, int sole_diversifier)
{
  int i;  KHE_SOLN soln;  KHE_INSTANCE ins;  KHE_OPTIONS options;
  /* ***
  if( DEBUG_STATS )
  {
    KheStatsBegin("khe_stats_out", KHE_STATS_TABLE_PLAIN);
    KheStatsTableBegin(3, 10);
    KheStatsRowBegin();
    KheStatsEntryString(KheArchiveId(archive));
    KheStatsEntryString("Cost");
    KheStatsEntryString("Time");
    KheStatsRowEnd(true);
  }
  *** */
  options = KheOptionsMake();
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    if( KheBenchmarkTryInstance(ins) )
    {
      if( DEBUG2 )
	fprintf(stderr, "solving %s:\n", KheInstanceId(ins));
      if( make_solns == 0 )
      {
	soln = KheSolnMake(ins);
	KheSolnSetDiversifier(soln, sole_diversifier);
	soln = KheGeneralSolve2014(soln, options);
      }
      else
	soln = KheInstanceParallelSolve(ins, 4, make_solns,
	  &KheGeneralSolve2014, options);
      if( soln_group != NULL )
	KheSolnGroupAddSoln(soln_group, soln);
    }
    else if( DEBUG2 )
      fprintf(stderr, "skipping %s:\n", KheInstanceId(ins));
  }
  /* ***
  if( DEBUG_STATS )
  {
    KheStatsTableEnd();
    KheStatsEnd();
  }
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestExplodeArchive(KHE_ARCHIVE archive)                          */
/*                                                                           */
/*  Write one archive for each instance of archive, containing that          */
/*  instance.                                                                */
/*                                                                           */
/*****************************************************************************/

static void KheTestExplodeArchive(KHE_ARCHIVE archive)
{
  KHE_INSTANCE ins;  KHE_ARCHIVE_METADATA md, md2;  KHE_ARCHIVE archive2;
  KHE_SOLN_GROUP soln_group, soln_group2;  KHE_SOLN soln;  int i, j, k;
  FILE *fp;  char fname[500];
  for( i = 0;  i < KheArchiveInstanceCount(archive);  i++ )
  {
    ins = KheArchiveInstance(archive, i);
    md = KheArchiveMetaData(archive);

    /* make a new archive corresponding to ins and add ins to it */
    md2 = (md == NULL ? NULL : KheArchiveMetaDataMake(KheInstanceId(ins),
      KheArchiveMetaDataContributor(md), KheArchiveMetaDataDate(md),
      KheArchiveMetaDataDescription(md), KheArchiveMetaDataRemarks(md)));
    archive2 = KheArchiveMake(KheInstanceId(ins), md2);
    /* KheArchiveDeleteInstance(archive, ins); */
    KheArchiveAddInstance(archive2, ins);

    /* add each of ins's solutions to the archive */
    for( j = 0;  j < KheArchiveSolnGroupCount(archive);  j++ )
    {
      soln_group = KheArchiveSolnGroup(archive, j);
      soln_group2 = NULL;
      for( k = 0;  k < KheSolnGroupSolnCount(soln_group);  k++ )
      {
	soln = KheSolnGroupSoln(soln_group, k);
	if( KheSolnInstance(soln) == ins )
	{
	  /* make a soln group to hold this soln if not done already */
	  if( soln_group2 == NULL )
	  {
	    if( !KheSolnGroupMake(archive2, KheSolnGroupId(soln_group),
		  KheSolnGroupMetaData(soln_group), &soln_group2) )
	      MAssert(false, "KheTestExplodeArchive internal error");
	  }

	  /* add soln to this soln group */
	  KheSolnGroupAddSoln(soln_group2, soln);
	}
      }
    }

    /* write the new archive */
    snprintf(fname, 500, "%s.xml", KheInstanceId(ins));
    fp = fopen(fname, "w");
    if( fp == NULL )
    {
      fprintf(stderr, "KheTestExplodeArchive: cannot write to file %s\n",fname);
      exit(1);
    }
    KheArchiveWrite(archive2, false, fp);
    fclose(fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheCountryPrefix(char *str)                                        */
/*                                                                           */
/*  Return the prefix of str which denotes the country; concretely, it       */
/*  is that part of the string before the first '-' or '.'.  A new string.   */
/*                                                                           */
/*****************************************************************************/

static char *KheCountryPrefix(char *str)
{
  char *p, *res;
  res = MStringCopy(str);
  for( p = res;  *p != '\0' && *p != '-' && *p != '.';  p++ );
  *p = '\0';
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestWriteArchive(KHE_ARCHIVE archive, char *name_prefix)         */
/*                                                                           */
/*  Write archive to name_prefix.xml.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheTestWriteArchive(KHE_ARCHIVE archive, char *name_prefix)
{
  FILE *fp;  char fname[500];
  snprintf(fname, 500, "%s.xml", name_prefix);
  fp = fopen(fname, "w");
  MAssert(fp != NULL,
    "KheTestExplodeArchiveByCountry: cannot write to file %s\n", fname);
  KheArchiveWrite(archive, false, fp);
  fclose(fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTestExplodeArchiveByCountry(KHE_ARCHIVE archive)                 */
/*                                                                           */
/*  Similar to KheTestExplodeArchive except that there is one archive        */
/*  per country, not one per instance.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheTestExplodeArchiveByCountry(KHE_ARCHIVE archive)
{
  KHE_INSTANCE ins;  char *prefix, *prev_prefix;
  KHE_ARCHIVE_METADATA md, md2;  KHE_ARCHIVE curr_archive;
  md = KheArchiveMetaData(archive);
  curr_archive = NULL;  prev_prefix = NULL;
  while( KheArchiveInstanceCount(archive) > 0 )
  {
    ins = KheArchiveInstance(archive, 0);
    KheArchiveDeleteInstance(archive, ins);
    if( KheInstanceId(ins) != NULL )
    {
      prefix = KheCountryPrefix(KheInstanceId(ins));

      /* do what needs to be done if prefix is incompatible with prev_prefix */
      if( prev_prefix == NULL || strcmp(prev_prefix, prefix) != 0 )
      {
	/* write curr_archive, if any */
	if( curr_archive != NULL )
          KheTestWriteArchive(curr_archive, prev_prefix);

	/* start off a new archive */
	md2 = (md == NULL ? NULL : KheArchiveMetaDataMake(prefix,
	  KheArchiveMetaDataContributor(md), KheArchiveMetaDataDate(md),
	  KheArchiveMetaDataDescription(md), KheArchiveMetaDataRemarks(md)));
	curr_archive = KheArchiveMake(prefix, md2);
      }

      /* add ins to curr_archive */
      KheArchiveAddInstance(curr_archive, ins);
      prev_prefix = prefix;
    }
  }
  if( curr_archive != NULL )
    KheTestWriteArchive(curr_archive, prev_prefix);
}


/*****************************************************************************/
/*                                                                           */
/*  int main(int argc, char *argv[])                                         */
/*                                                                           */
/*  Main program:  read a file, solve each instance, then write.             */
/*                                                                           */
/*****************************************************************************/

int main(int argc, char *argv[])
{
  KHE_ARCHIVE archive;  KHE_SOLN soln;  int i, j, num, len;
  char op;  KHE_SOLN_GROUP soln_group;  KHE_OPTIONS options;
  int thread_count, make_solns, keep_solns;

  /* decide which operation is wanted */
  num = 0;
  if( argc == 1 )
    op = 'u';
  else if( argc >= 2 && argv[1][0] == '-' && argv[1][1] != '\0' )
  {
    op = argv[1][1];
    if( argv[1][2] != '\0' && op != 'g' && op != 'h' )
      if( sscanf(&argv[1][2], "%d", &num) != 1 || num <= 0 )
      {
	fprintf(stderr, "%s: invalid command line option %s\n",
	  argv[0], argv[1]);
	exit(1);
      }
  }
  else
    op = 't';
  if( DEBUG1 )
    fprintf(stderr, "khe -%c (argc = %d)\n", op, argc);

  /* do the operation */
  switch( op )
  {
    case 'b':

      /* khe -b */
      if( argc != 2 )
        KheUsageMessageAndExit();
      KheBackoffTest(stdout);
      break;

    case 'c':

      /* khe -c archive */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      for( i = 0;  i < KheArchiveSolnGroupCount(archive);  i++ )
      {
	soln_group = KheArchiveSolnGroup(archive, i);
	for( j = 0;  j < KheSolnGroupSolnCount(soln_group);  j++ )
	{
	  soln = KheSolnGroupSoln(soln_group, j);
	  soln = KheSolnCopy(soln);
	}
      }
      break;

    case 'd':

      /* khe -d */
      if( argc != 2 )
        KheUsageMessageAndExit();
      if( num <= 0 )
	num = 3;
      KheDiversificationTest(num, stdout);
      break;

    case 'e':

      /* khe -e archive */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      KheTestExplodeArchive(archive);
      break;

    case 'E':

      /* khe -E archive */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      KheTestExplodeArchiveByCountry(archive);
      break;

    case 'g':
    case 'h':

      /* khe -g<letters> archive */
      /* khe -h<letters> archive */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      len = strlen(argv[1]);
      for( i = 2;  i < len;  i++ )
	KheBenchmark(archive, &KheGeneralSolve2014, "KHE14",
	  "Jeffrey H. Kingston", argv[1][i],
	  op == 'g' ? KHE_STATS_TABLE_LOUT : KHE_STATS_TABLE_LATEX);
      break;

    case 'l':

      /* khe -l */
      if( argc != 2 )
        KheUsageMessageAndExit();
      LSetTest(stdout);
      break;

    case 'm':

      /* khe -m */
      if( argc != 2 )
        KheUsageMessageAndExit();
      MTest(stdout);
      break;

    case 'p':

      /* khe -p <n> <n> <n> archive */
      if( argc != 6 )
        KheUsageMessageAndExit();
      if( sscanf(argv[2], "%d", &thread_count) != 1 )
        KheUsageMessageAndExit();
      if( sscanf(argv[3], "%d", &make_solns) != 1 )
        KheUsageMessageAndExit();
      if( sscanf(argv[4], "%d", &keep_solns) != 1 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[5]);
      soln_group = KheTestMakeSolnGroup(archive);
      options = KheOptionsMake();
      KheArchiveParallelSolve(archive, thread_count, make_solns,
	&KheGeneralSolve2014, options, keep_solns, soln_group);
      KheArchiveWrite(archive, true, stdout);
      break;

    case 's':

      /* khe -s<num> archive */
      /* equivalent to khe -p <num> <num> <num> archive */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      soln_group = KheTestMakeSolnGroup(archive);
      KheTestSolveInstances(archive, num, soln_group, KHE_SOLE_DIVERSIFIER);
      KheArchiveWrite(archive, true, stdout);
      break;

    case 't':

      /* khe [ -t ] archive */
      if( argc != 2 && argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      KheTestSolveInstances(archive, num, NULL, KHE_SOLE_DIVERSIFIER);
      break;

    case 'u':

      KheUsageMessageAndExit();
      break;

    case 'v':

      fprintf(stderr, "KHE Version " KHE_VERSION "\n");
      break;

    case 'w':

      /* write */
      if( argc != 3 )
        KheUsageMessageAndExit();
      archive = ReadArchive(argv[argc - 1]);
      KheArchiveWrite(archive, true, stdout);
      break;
  }
}
