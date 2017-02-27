
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
/*  FILE:         khe_sm_stats.c                                             */
/*  DESCRIPTION:  Tables of statistics                                       */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ENTRY - one entry of a table                                   */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_STATS_ENTRY_NONE,
  KHE_STATS_ENTRY_STRING,
  KHE_STATS_ENTRY_COST,
  KHE_STATS_ENTRY_TIME,
  KHE_STATS_ENTRY_INT
} KHE_STATS_ENTRY_TYPE;

typedef struct khe_stats_entry_rec {
  KHE_STATS_ENTRY_TYPE	entry_type;		/* type of value assigned    */
  bool			rule_after;		/* used in first row only    */
  bool			highlight;		/* highlight this entry      */
  union {
    char		*string_val;		/* if KHE_STATS_ENTRY_STRING */
    KHE_COST		cost_val;		/* if KHE_STATS_ENTRY_COST   */
    float		time_val;		/* if KHE_STATS_ENTRY_TIME   */
    int			int_val;		/* if KHE_STATS_ENTRY_INT    */
  } v;
} *KHE_STATS_ENTRY;

typedef MARRAY(KHE_STATS_ENTRY) ARRAY_KHE_STATS_ENTRY;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ROW - one row of a table                                       */
/*                                                                           */
/*  The label for this row is the value of its first entry.                  */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_stats_row_rec {
  bool			rule_below;		/* true if rule below here   */
  ARRAY_KHE_STATS_ENTRY	entries;		/* entries for this row      */
} *KHE_STATS_ROW;

typedef MARRAY(KHE_STATS_ROW) ARRAY_KHE_STATS_ROW;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_TABLE - one stats table                                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_stats_table_rec {
  KHE_STATS_TABLE_TYPE	table_type;		/* table format type         */
  int			col_width;		/* column width (plain text) */
  bool			with_average_row;	/* with average row          */
  bool			with_total_row;		/* with total row            */
  bool			highlight_cost_minima;	/* highlight cost minima     */
  bool			highlight_time_minima;	/* highlight time minima     */
  bool			highlight_int_minima;	/* highlight int minima      */
  ARRAY_KHE_STATS_ROW	rows;			/* the rows                  */
  ARRAY_STRING		caption_lines;		/* the lines of the caption  */
} *KHE_STATS_TABLE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_POINT - one point in a stats graph dataset                     */
/*                                                                           */
/*****************************************************************************/

typedef struct {
  float			x;
  float			y;
} KHE_STATS_POINT;

typedef MARRAY(KHE_STATS_POINT) ARRAY_KHE_STATS_POINT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_DATASET - one dataset in a stats graph                         */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_stats_dataset_rec {
  char				*dataset_label;	/* label of dataset          */
  KHE_STATS_DATASET_TYPE	dataset_type;	/* format for printing data  */
  ARRAY_KHE_STATS_POINT		points;		/* points of the dataset     */
} *KHE_STATS_DATASET;

typedef MARRAY(KHE_STATS_DATASET) ARRAY_KHE_STATS_DATASET;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_GRAPH - one stats graph                                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_stats_graph_rec {
  float				width;		/* width of graph            */
  float				height;		/* height of graph           */
  float				xmax;		/* xmax of graph             */
  float				ymax;		/* ymax of graph             */
  char				*above_caption;	/* above caption of graph    */
  char				*below_caption;	/* below caption of graph    */
  char				*left_caption;	/* left caption of graph     */
  char				*right_caption;	/* right caption of graph    */
  ARRAY_KHE_STATS_DATASET	datasets;	/* the datasets              */
  ARRAY_STRING			caption_lines;	/* the lines of the caption  */
} *KHE_STATS_GRAPH;


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE - one stats file                                          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_stats_file_rec {
  char			*file_name;		/* file name                 */
  FILE			*fp;			/* file (when open)          */
  KHE_STATS_TABLE	curr_table;		/* current stats table       */
  KHE_STATS_GRAPH	curr_graph;		/* current stats graph       */
  int			thing_count;		/* no. of things so far      */
} *KHE_STATS_FILE;

typedef MTABLE(KHE_STATS_FILE) TABLE_KHE_STATS_FILE;


/*****************************************************************************/
/*                                                                           */
/*  Static variables holding the state of the module.                        */
/*                                                                           */
/*****************************************************************************/

static	bool			s_init;		/* true if initialized       */
static	TABLE_KHE_STATS_FILE	s_files;	/* files to generate         */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "timing"                                                       */
/*                                                                           */
/*****************************************************************************/

#if KHE_USE_TIMING
#include <time.h>
#include <sys/time.h>
struct khe_stats_timer_rec {
  struct timeval		s_tv;		/* start time                */
};
#else
struct khe_stats_timer_rec {
  int				junk;		/* C does not allow empty    */
};
#endif


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_TIMER KheStatsTimerMake(void)                                  */
/*                                                                           */
/*  Make a new timer, and store the time this was done in the timer.         */
/*                                                                           */
/*****************************************************************************/

KHE_STATS_TIMER KheStatsTimerMake(void)
{
  KHE_STATS_TIMER res;
  MMake(res);
  KheStatsTimerReset(res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTimerReset(KHE_STATS_TIMER st)                              */
/*                                                                           */
/*  Reset st to the current time.                                            */
/*                                                                           */
/*****************************************************************************/

void KheStatsTimerReset(KHE_STATS_TIMER st)
{
#if KHE_USE_TIMING
  gettimeofday(&st->s_tv, NULL);
#else
  st->junk = 0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  float KheStatsTimerNow(KHE_STATS_TIMER st)                               */
/*                                                                           */
/*  Return the elapsed wall clock time between the time that st was made     */
/*  and now.                                                                 */
/*                                                                           */
/*****************************************************************************/

float KheStatsTimerNow(KHE_STATS_TIMER st)
{
#if KHE_USE_TIMING
  struct timeval now_tv;
  gettimeofday(&now_tv, NULL);
  return (float) (now_tv.tv_sec - st->s_tv.tv_sec) +
    (float) (now_tv.tv_usec - st->s_tv.tv_usec) / 1000000.0;
#else
  return -1.0;
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_TIMER KheStatsTimerCopy(KHE_STATS_TIMER st)                    */
/*                                                                           */
/*  Return a copy of st.                                                     */
/*                                                                           */
/*****************************************************************************/

KHE_STATS_TIMER KheStatsTimerCopy(KHE_STATS_TIMER st)
{
  KHE_STATS_TIMER res;
  MMake(res);
#if KHE_USE_TIMING
  res->s_tv = st->s_tv;
#else
  res->junk = st->junk;
#endif
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTimerDelete(KHE_STATS_TIMER st)                             */
/*                                                                           */
/*  Delete st, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

void KheStatsTimerDelete(KHE_STATS_TIMER st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheStatsDateToday(void)                                            */
/*                                                                           */
/*  Return today's date as a string in static memory.                        */
/*                                                                           */
/*****************************************************************************/

static char *KheMonthName(int tm_mon)
{
  char *months[] = { "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December" };
  MAssert(tm_mon >= 0 && tm_mon <= 11, "KheMonthName internal error");
  return months[tm_mon];
}

char *KheStatsDateToday(void)
{
#if KHE_USE_TIMING
  static char buff[100];
  time_t t;
  struct tm *time_now;
  time(&t);
  time_now = localtime(&t);
  snprintf(buff, 100, "%d %s %d", time_now->tm_mday,
    KheMonthName(time_now->tm_mon), 1900 + time_now->tm_year);
  return buff;
#else
  return "?";
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "table entries" (private)                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ENTRY KheStatsEntryMake(bool rule_after)                       */
/*                                                                           */
/*  Make a new stats table entry, initially with no value.                   */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_ENTRY KheStatsEntryMake(bool rule_after)
{
  KHE_STATS_ENTRY res;
  MMake(res);
  res->entry_type = KHE_STATS_ENTRY_NONE;
  res->highlight = false;
  res->rule_after = rule_after;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryDelete(KHE_STATS_ENTRY entry)                          */
/*                                                                           */
/*  Delete entry, reclaiming its memory.                                     */
/*                                                                           */
/*****************************************************************************/

void KheStatsEntryDelete(KHE_STATS_ENTRY entry)
{
  if( entry->entry_type == KHE_STATS_ENTRY_STRING )
    MFree(entry->v.string_val);
  MFree(entry);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEntryAssignString(KHE_STATS_ENTRY entry, char *string_val)       */
/*  void KheEntryAssignCost(KHE_STATS_ENTRY entry, KHE_COST cost_val)        */
/*  void KheEntryAssignTime(KHE_STATS_ENTRY entry, float time_val)           */
/*  void KheEntryAssignInt(KHE_STATS_ENTRY entry, int int_val)               */
/*                                                                           */
/*  Assign the given value to entry, irrespective of any existing value.     */
/*                                                                           */
/*****************************************************************************/

static void KheEntryAssignString(KHE_STATS_ENTRY entry, char *string_val)
{
  entry->entry_type = KHE_STATS_ENTRY_STRING;
  entry->v.string_val = MStringCopy(string_val);
}

static void KheEntryAssignCost(KHE_STATS_ENTRY entry, KHE_COST cost_val)
{
  entry->entry_type = KHE_STATS_ENTRY_COST;
  entry->v.cost_val = cost_val;
}

static void KheEntryAssignTime(KHE_STATS_ENTRY entry, float time_val)
{
  entry->entry_type = KHE_STATS_ENTRY_TIME;
  entry->v.time_val = time_val;
}

static void KheEntryAssignInt(KHE_STATS_ENTRY entry, int int_val)
{
  entry->entry_type = KHE_STATS_ENTRY_INT;
  entry->v.int_val = int_val;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryWrite(KHE_STATS_ENTRY entry, KHE_STATS_TABLE tbl,      */
/*    int col_num, bool first_col, bool last_col, FILE *fp)                  */
/*                                                                           */
/*  Write one entry of tbl onto fp.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheStatsEntryWrite(KHE_STATS_ENTRY entry, KHE_STATS_TABLE tbl,
  int col_num, bool first_col, bool last_col, FILE *fp)
{
  char buff[100];

  /* get the string value of what is to be written */
  switch( entry->entry_type )
  {
    case KHE_STATS_ENTRY_NONE:

      snprintf(buff, 100, "%s", "");
      break;

    case KHE_STATS_ENTRY_STRING:

      snprintf(buff, 100, "%s", entry->v.string_val);
      break;

    case KHE_STATS_ENTRY_COST:

      snprintf(buff, 100, "%.5f", KheCostShow(entry->v.cost_val));
      break;

    case KHE_STATS_ENTRY_TIME:

      snprintf(buff, 100, "%.1f", entry->v.time_val);
      break;

    case KHE_STATS_ENTRY_INT:

      snprintf(buff, 100, "%d", entry->v.int_val);
      break;

    default:

      MAssert(false, "KheStatsEntryWrite internal error");
      snprintf(buff, 100, "??");  /* keep compiler happy */
      break;
  }

  /* write the string */
  switch( tbl->table_type )
  {
    case KHE_STATS_TABLE_PLAIN:

      if( entry->highlight )
	fprintf(fp, first_col ? "*%-*s" : "%*s*", tbl->col_width - 1, buff);
      else
	fprintf(fp, first_col ? "%-*s" : "%*s", tbl->col_width, buff);
      break;

    case KHE_STATS_TABLE_LOUT:

      MAssert(col_num < 12, "KheStatsEntry: too many columns in Lout table");
      if( entry->highlight )
	fprintf(fp, "  %c { @B { %s } }\n", (char) ('A' + col_num), buff);
      else
	fprintf(fp, "  %c { %s }\n", (char) ('A' + col_num), buff);
      break;

    case KHE_STATS_TABLE_LATEX:

      if( !first_col )
	fprintf(fp, " & ");
      if( entry->highlight )
	fprintf(fp, "{\\bf %s}", buff);
      else
	fprintf(fp, "%s", buff);
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "table rows" (private)                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ROW KheStatsRowMake(char *row_label, bool rule_below)          */
/*                                                                           */
/*  Make a new row with these attributes.  The row label goes into the       */
/*  first entry of the row, which must exist.                                */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_ROW KheStatsRowMake(char *row_label, bool rule_below)
{
  KHE_STATS_ROW res;  KHE_STATS_ENTRY entry;

  /* make the object */
  MMake(res);
  res->rule_below = rule_below;
  MArrayInit(res->entries);

  /* make and add its first entry, holding row_label */
  entry = KheStatsEntryMake(false);
  KheEntryAssignString(entry, row_label);
  MArrayAddLast(res->entries, entry);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowDelete(KHE_STATS_ROW row)                                */
/*                                                                           */
/*  Delete row and its entries, reclaiming their memory.                     */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowDelete(KHE_STATS_ROW row)
{
  while( MArraySize(row->entries) > 0 )
    KheStatsEntryDelete(MArrayRemoveLast(row->entries));
  MFree(row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowFill(KHE_STATS_ROW row, int col_index)                   */
/*                                                                           */
/*  Fill row so that col_index has an entry.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowFill(KHE_STATS_ROW row, int col_index)
{
  KHE_STATS_ENTRY entry;
  while( MArraySize(row->entries) <= col_index )
  {
    entry = KheStatsEntryMake(false);
    MArrayAddLast(row->entries, entry);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheStatsRowLabel(KHE_STATS_ROW row)                                */
/*                                                                           */
/*  Return the label of row.                                                 */
/*                                                                           */
/*****************************************************************************/

static char *KheStatsRowLabel(KHE_STATS_ROW row)
{
  KHE_STATS_ENTRY entry;
  MAssert(MArraySize(row->entries) > 0, "KheStatsRowLabel internal error 1");
  entry = MArrayFirst(row->entries);
  MAssert(entry->entry_type == KHE_STATS_ENTRY_STRING,
    "KheStatsRowLabel internal error 2");
  return entry->v.string_val;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowHighlightMinimaCost(KHE_STATS_ROW row)                   */
/*                                                                           */
/*  Highlight minimal entries of cost type in row, if any.                   */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowHighlightMinimaCost(KHE_STATS_ROW row)
{
  KHE_STATS_ENTRY entry;  int i;  bool min_found;  KHE_COST min_cost_val;

  /* find a minimum value, if any suitable values are present */
  min_found = false;
  min_cost_val = 0;  /* keep compiler happy; undefined really */
  MArrayForEach(row->entries, &entry, &i)
    if( entry->entry_type == KHE_STATS_ENTRY_COST &&
        (!min_found || entry->v.cost_val < min_cost_val) )
      min_found = true, min_cost_val = entry->v.cost_val;

  /* if a minimum value was found, highlight all occurrences of it */
  if( min_found )
    MArrayForEach(row->entries, &entry, &i)
      if( entry->entry_type == KHE_STATS_ENTRY_COST &&
	  entry->v.cost_val == min_cost_val )
	entry->highlight = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowHighlightMinimaTime(KHE_STATS_ROW row)                   */
/*                                                                           */
/*  Highlight minimal entries of time type in row, if any.                   */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowHighlightMinimaTime(KHE_STATS_ROW row)
{
  KHE_STATS_ENTRY entry;  int i;  bool min_found;  float min_time_val;

  /* find a minimum value, if any suitable values are present */
  min_found = false;
  min_time_val = 0.0;  /* keep compiler happy; undefined really */
  MArrayForEach(row->entries, &entry, &i)
    if( entry->entry_type == KHE_STATS_ENTRY_TIME &&
        (!min_found || entry->v.time_val < min_time_val) )
      min_found = true, min_time_val = entry->v.time_val;

  /* if a minimum value was found, highlight all occurrences of it */
  if( min_found )
    MArrayForEach(row->entries, &entry, &i)
      if( entry->entry_type == KHE_STATS_ENTRY_TIME &&
	  entry->v.time_val == min_time_val )
	entry->highlight = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowHighlightMinimaInt(KHE_STATS_ROW row)                    */
/*                                                                           */
/*  Highlight minimal entries of int type in row, if any.                    */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowHighlightMinimaInt(KHE_STATS_ROW row)
{
  KHE_STATS_ENTRY entry;  int i;  bool min_found;  int min_int_val;

  /* find a minimum value, if any suitable values are present */
  min_found = false;
  min_int_val = 0;  /* keep compiler happy; undefined really */
  MArrayForEach(row->entries, &entry, &i)
    if( entry->entry_type == KHE_STATS_ENTRY_INT &&
        (!min_found || entry->v.int_val < min_int_val) )
      min_found = true, min_int_val = entry->v.int_val;

  /* if a minimum value was found, highlight all occurrences of it */
  if( min_found )
    MArrayForEach(row->entries, &entry, &i)
      if( entry->entry_type == KHE_STATS_ENTRY_INT &&
	  entry->v.int_val == min_int_val )
	entry->highlight = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowWriteBegin(KHE_STATS_ROW row,                            */
/*    bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)          */
/*                                                                           */
/*  Begin writing a row onto fp.                                             */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowWriteBegin(KHE_STATS_ROW row,
  bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)
{
  switch( tbl->table_type )
  {
    case KHE_STATS_TABLE_PLAIN:

      /* nothing to do */
      break;

    case KHE_STATS_TABLE_LOUT:

      fprintf(fp, first_row ? "@Rowb\n" : "@Rowa\n");
      break;

    case KHE_STATS_TABLE_LATEX:

      /* nothing to do */
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowWriteEnd(KHE_STATS_ROW row,                              */
/*    bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)          */
/*                                                                           */
/*  End writing a row of tbl onto fp.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowWriteEnd(KHE_STATS_ROW row,
  bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)
{
  int i, plain_total_width;  KHE_STATS_ROW first_r;
  switch( tbl->table_type )
  {
    case KHE_STATS_TABLE_PLAIN:

      fprintf(fp, "\n");
      if( row->rule_below )
      {
	first_r = MArrayFirst(tbl->rows);
	plain_total_width = tbl->col_width * MArraySize(first_r->entries);
	for( i = 0;  i < plain_total_width;  i++ )
	  fprintf(fp, "-");
	fprintf(fp, "\n");
      }
      break;

    case KHE_STATS_TABLE_LOUT:

      if( row->rule_below )
	fprintf(fp, "  rb { yes }\n");
      break;

    case KHE_STATS_TABLE_LATEX:

      if( !last_row || row->rule_below )
	fprintf(fp, " \\\\");
      fprintf(fp, "\n");
      if( row->rule_below )
	fprintf(fp, "\\noalign{}\\hline\\noalign{}\n");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowWrite(KHE_STATS_ROW row,                                 */
/*    bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)          */
/*                                                                           */
/*  Write a row of tbl onto fp.   If first_row is true, this is the first    */
/*  row.  If last_row is true, this is the last row.  Both could be true.    */
/*                                                                           */
/*****************************************************************************/

static void KheStatsRowWrite(KHE_STATS_ROW row,
  bool first_row, bool last_row, KHE_STATS_TABLE tbl, FILE *fp)
{
  KHE_STATS_ENTRY entry;  int i;
  KheStatsRowWriteBegin(row, first_row, last_row, tbl, fp);
  MArrayForEach(row->entries, &entry, &i)
    KheStatsEntryWrite(entry, tbl, i, i == 0,
      i == MArraySize(row->entries) - 1, fp);
  KheStatsRowWriteEnd(row, first_row, last_row, tbl, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tables" (private)                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_TABLE KheStatsTableMake(KHE_STATS_TABLE_TYPE table_type,       */
/*    int col_width, char *corner, bool with_average_row,                    */
/*    bool highlight_cost_minima, bool highlight_time_minima,                */
/*    bool highlight_int_minima)                                             */
/*                                                                           */
/*  Make a new stats_table object with these attributes and one row          */
/*  with one entry.                                                          */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_TABLE KheStatsTableMake(KHE_STATS_TABLE_TYPE table_type,
  int col_width, char *corner, bool with_average_row, bool with_total_row,
  bool highlight_cost_minima, bool highlight_time_minima,
  bool highlight_int_minima)
{
  KHE_STATS_TABLE res;  KHE_STATS_ROW row;

  /* check reasonable */
  MAssert(table_type == KHE_STATS_TABLE_PLAIN ||
    table_type == KHE_STATS_TABLE_LOUT || table_type == KHE_STATS_TABLE_LATEX,
    "KheStatsTableMake: unknown table type %d\n", table_type);
  if( table_type == KHE_STATS_TABLE_PLAIN )
    MAssert(col_width >= 1, "KheStatsTableMake: col_width out of range (%d)",
      col_width);
  MAssert(corner != NULL, "KheStatsTableMake: corner == NULL");

  /* build the object */
  MMake(res);
  res->table_type = table_type;
  res->col_width = col_width;
  res->with_average_row = with_average_row;
  res->with_total_row = with_total_row;
  res->highlight_cost_minima = highlight_cost_minima;
  res->highlight_time_minima = highlight_time_minima;
  res->highlight_int_minima = highlight_int_minima;
  MArrayInit(res->rows);
  MArrayInit(res->caption_lines);

  /* add the compulsory first row (and column), and return */
  row = KheStatsRowMake(corner, true);
  MArrayAddLast(res->rows, row);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableDelete(KHE_STATS_TABLE tbl)                            */
/*                                                                           */
/*  Delete tbl, reclaiming its memory.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableDelete(KHE_STATS_TABLE tbl)
{
  while( MArraySize(tbl->rows) > 0 )
    KheStatsRowDelete(MArrayRemoveLast(tbl->rows));
  MArrayFree(tbl->rows);
  while( MArraySize(tbl->caption_lines) > 0 )
    MFree(MArrayRemoveLast(tbl->caption_lines));
  MArrayFree(tbl->caption_lines);
  MFree(tbl);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheStatsTableContainsRow(KHE_STATS_TABLE tbl, char *row_label,      */
/*    KHE_STATS_ROW *row)                                                    */
/*                                                                           */
/*  If tbl contains a row with this label, return true and set *row to       */
/*  the row, otherwise return false.  Ignore the first row.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheStatsTableContainsRow(KHE_STATS_TABLE tbl, char *row_label,
  KHE_STATS_ROW *row)
{
  int i;  KHE_STATS_ROW row2;
  for( i = 1;  i < MArraySize(tbl->rows);  i++ )
  {
    row2 = MArrayGet(tbl->rows, i);
    if( strcmp(row_label, KheStatsRowLabel(row2)) == 0 )
    {
      *row = row2;
      return true;
    }
  }
  *row = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheStatsTableContainsCol(KHE_STATS_TABLE tbl, char *col_label,      */
/*    int *col_index)                                                        */
/*    int *pos)                                                              */
/*                                                                           */
/*  If tbl contains a col with this label, return true and set *col_index    */
/*  to its index, otherwise return false.                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheStatsTableContainsCol(KHE_STATS_TABLE tbl, char *col_label,
  int *col_index)
{
  int i;  KHE_STATS_ROW first_row;  KHE_STATS_ENTRY entry;
  MAssert(MArraySize(tbl->rows) > 0,
    "KheStatsTableContainsCol internal error 1");
  first_row = MArrayFirst(tbl->rows);
  for( i = 1;  i < MArraySize(first_row->entries);  i++ )
  {
    entry = MArrayGet(first_row->entries, i);
    MAssert(entry->entry_type == KHE_STATS_ENTRY_STRING,
      "KheStatsTableContainsCol internal error 2");
    if( strcmp(col_label, entry->v.string_val) == 0 )
    {
      *col_index = i;
      return true;
    }
  }
  *col_index = -1;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ENTRY_TYPE KheStatsTableSoleNumericEntryType(                  */
/*    KHE_STATS_TABLE tbl, int col_index)                                    */
/*                                                                           */
/*  Return the unique entry type of column col_index of tbl, ignoring        */
/*  NONE an STRING, or NONE if none.                                         */
/*                                                                           */
/*****************************************************************************/
#define EntryTypeIsNumeric(et) (et >= KHE_STATS_ENTRY_COST)

static KHE_STATS_ENTRY_TYPE KheStatsTableSoleNumericEntryType(
  KHE_STATS_TABLE tbl, int col_index)
{
  int i; KHE_STATS_ROW row;  KHE_STATS_ENTRY entry;
  KHE_STATS_ENTRY_TYPE res;
  res = KHE_STATS_ENTRY_NONE;
  MArrayForEach(tbl->rows, &row, &i)
  {
    KheStatsRowFill(row, col_index);
    entry = MArrayGet(row->entries, col_index);
    if( EntryTypeIsNumeric(entry->entry_type) )
    {
      if( res == KHE_STATS_ENTRY_NONE )
	res = entry->entry_type;
      else if( res != entry->entry_type )
	return KHE_STATS_ENTRY_NONE;
    }
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddAverageCost(KHE_STATS_TABLE tbl, int col_index,     */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the average cost of column col_index, if any.            */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddAverageCost(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  int hard_total, soft_total, count, i;
  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  hard_total = soft_total = count = 0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_COST )
    {
      hard_total += KheHardCost(entry2->v.cost_val);
      soft_total += KheSoftCost(entry2->v.cost_val);
      count += 1;
    }
  }
  if( count >= 1 )
    KheEntryAssignCost(entry, KheCost(hard_total/count, soft_total/count));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddAverageTime(KHE_STATS_TABLE tbl, int col_index,     */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the average time of column col_index of tbl, if any.     */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddAverageTime(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  int count, i;  float total_time;
  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  count = 0;  total_time = 0.0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_TIME )
    {
      total_time += entry2->v.time_val;
      count += 1;
    }
  }
  if( count >= 1 )
    KheEntryAssignTime(entry, total_time / count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddAverageInt(KHE_STATS_TABLE tbl, int col_index,      */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the average int_val of column col_index of tbl, if any.  */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddAverageInt(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  int total, count, i;
  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  total = count = 0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_INT )
    {
      total += entry2->v.int_val;
      count += 1;
    }
  }
  if( count >= 1 )
    KheEntryAssignInt(entry, total / count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddAverageRow(KHE_STATS_TABLE tbl)                     */
/*                                                                           */
/*  Add an average row to tbl.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddAverageRow(KHE_STATS_TABLE tbl)
{
  KHE_STATS_ROW first_row, average_row;  KHE_STATS_ENTRY entry;  int i;

  /* get the first row */
  MAssert(MArraySize(tbl->rows) > 0,
    "KheStatsTableAddAverageRow internal error 1");
  first_row = MArrayFirst(tbl->rows);

  /* make and add an average row */
  average_row = KheStatsRowMake("Average", false);
  MArrayAddLast(tbl->rows, average_row);

  /* for every column of first_row except the first, add an average entry */
  for( i = 1;  i < MArraySize(first_row->entries);  i++ )
  {
    entry = KheStatsEntryMake(false);
    MArrayAddLast(average_row->entries, entry);
    switch( KheStatsTableSoleNumericEntryType(tbl, i) )
    {
      case KHE_STATS_ENTRY_NONE:
      case KHE_STATS_ENTRY_STRING:

	/* no average, so change nothing here */
	break;

      case KHE_STATS_ENTRY_COST:

        KheStatsTableAddAverageCost(tbl, i, entry);
	break;

      case KHE_STATS_ENTRY_TIME:

        KheStatsTableAddAverageTime(tbl, i, entry);
	break;

      case KHE_STATS_ENTRY_INT:

        KheStatsTableAddAverageInt(tbl, i, entry);
	break;

      default:

	MAssert(false, "KheStatsTableAddAverageRow internal error 2");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddTotalCost(KHE_STATS_TABLE tbl, int col_index,       */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the total cost of column col_index, if any.              */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddTotalCost(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  KHE_COST cost_total;  int i;  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  cost_total = 0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_COST )
      cost_total += entry2->v.cost_val;
  }
  KheEntryAssignCost(entry, cost_total);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddTotalTime(KHE_STATS_TABLE tbl, int col_index,       */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the total time of column col_index of tbl, if any.       */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddTotalTime(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  int i;  float total_time;  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  total_time = 0.0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_TIME )
      total_time += entry2->v.time_val;
  }
  KheEntryAssignTime(entry, total_time);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddTotalInt(KHE_STATS_TABLE tbl, int col_index,        */
/*    KHE_STATS_ENTRY entry)                                                 */
/*                                                                           */
/*  Assign to entry the total int_val of column col_index of tbl, if any.    */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddTotalInt(KHE_STATS_TABLE tbl, int col_index,
  KHE_STATS_ENTRY entry)
{
  int total, i;  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry2;
  total = 0;
  MArrayForEach(tbl->rows, &row, &i)
  {
    entry2 = MArrayGet(row->entries, col_index);
    if( entry2->entry_type == KHE_STATS_ENTRY_INT )
      total += entry2->v.int_val;
  }
  KheEntryAssignInt(entry, total);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableAddTotalRow(KHE_STATS_TABLE tbl)                       */
/*                                                                           */
/*  Add a total row to tbl.                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableAddTotalRow(KHE_STATS_TABLE tbl)
{
  KHE_STATS_ROW first_row, total_row;  KHE_STATS_ENTRY entry;  int i;

  /* get the first row */
  MAssert(MArraySize(tbl->rows) > 0,
    "KheStatsTableAddTotalRow internal error 1");
  first_row = MArrayFirst(tbl->rows);

  /* make and add an average row */
  total_row = KheStatsRowMake("Total", false);
  MArrayAddLast(tbl->rows, total_row);

  /* for every column of first_row except the first, add an average entry */
  for( i = 1;  i < MArraySize(first_row->entries);  i++ )
  {
    entry = KheStatsEntryMake(false);
    MArrayAddLast(total_row->entries, entry);
    switch( KheStatsTableSoleNumericEntryType(tbl, i) )
    {
      case KHE_STATS_ENTRY_NONE:
      case KHE_STATS_ENTRY_STRING:

	/* no average, so change nothing here */
	break;

      case KHE_STATS_ENTRY_COST:

        KheStatsTableAddTotalCost(tbl, i, entry);
	break;

      case KHE_STATS_ENTRY_TIME:

        KheStatsTableAddTotalTime(tbl, i, entry);
	break;

      case KHE_STATS_ENTRY_INT:

        KheStatsTableAddTotalInt(tbl, i, entry);
	break;

      default:

	MAssert(false, "KheStatsTableAddTotalRow internal error 2");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableHighlightMinima(KHE_STATS_TABLE tbl)                   */
/*                                                                           */
/*  Highlight minima in the rows of tbl (except the first row).              */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableHighlightMinima(KHE_STATS_TABLE tbl)
{
  int i;  KHE_STATS_ROW row;
  for( i = 1;  i < MArraySize(tbl->rows);  i++ )
  {
    row = MArrayGet(tbl->rows, i);
    if( tbl->highlight_cost_minima )
      KheStatsRowHighlightMinimaCost(row);
    if( tbl->highlight_time_minima )
      KheStatsRowHighlightMinimaTime(row);
    if( tbl->highlight_int_minima )
      KheStatsRowHighlightMinimaInt(row);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableWriteBegin(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)   */
/*                                                                           */
/*  Begin writing tbl onto file.                                             */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableWriteBegin(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)
{
  int i;  char *str;  KHE_STATS_ROW row;  KHE_STATS_ENTRY entry;
  MAssert(MArraySize(tbl->rows) > 0, "KheStatsTableWriteBegin internal error");
  row = MArrayFirst(tbl->rows);
  switch( tbl->table_type )
  {
    case KHE_STATS_TABLE_PLAIN:

      /* nothing to do except optionally write a caption */
      if( MArraySize(tbl->caption_lines) > 0 )
      {
	MArrayForEach(tbl->caption_lines, &str, &i)
	  fprintf(file->fp, "%s", str);
	fprintf(file->fp, "\n");
      }
      break;

    case KHE_STATS_TABLE_LOUT:

      MAssert(MArraySize(row->entries) <= 12,
	"KheStatsTableWriteBegin: too many cols for table in file \"%s\"",
	file->file_name);
      fprintf(file->fp, "@Table\n");
      fprintf(file->fp, "  @OnePage { Yes }\n");
      if( file->thing_count == 1 )
	fprintf(file->fp, "  @Tag { %s }\n", file->file_name);
      else
	fprintf(file->fp, "  @Tag { %s_%d }\n", file->file_name,
	  file->thing_count);
      if( MArraySize(tbl->caption_lines) > 0 )
      {
	fprintf(file->fp, "  @Caption {\n");
	MArrayForEach(tbl->caption_lines, &str, &i)
	  fprintf(file->fp, "%s", str);
	fprintf(file->fp, "  }\n");
      }
      fprintf(file->fp, "-2p @Font -2px @Break @OneRow @Tbl\n");
      fprintf(file->fp, "  mv { 0.5vx }\n");
      fprintf(file->fp, "  aformat { ");
      MArrayForEach(row->entries, &entry, &i)
	fprintf(file->fp, "%s@Cell %s%s%s %c", i == 0 ? "" : " | ",
	  i == 0 ? "ml { 0i }" : "i { right }",
	  entry->rule_after ? " rr { yes }" : "",
	  i == MArraySize(row->entries) - 1 ? " mr { 0i }" : "",
	  (char) ('A' + i));
      fprintf(file->fp, " }\n");
      fprintf(file->fp, "  bformat { ");
      MArrayForEach(row->entries, &entry, &i)
	fprintf(file->fp, "%s@Cell %s%s%s %c", i == 0 ? "" : " | ",
	  i == 0 ? "ml { 0i }" : "i { ctr }",
	  entry->rule_after ? " rr { yes }" : "",
	  i == MArraySize(row->entries) - 1 ? " mr { 0i }" : "",
	  (char) ('A' + i));
      fprintf(file->fp, " }\n");
      fprintf(file->fp, "{\n");
      break;

    case KHE_STATS_TABLE_LATEX:

      fprintf(file->fp, "\\begin{table}\n");
      fprintf(file->fp, "\\caption{");
      MArrayForEach(tbl->caption_lines, &str, &i)
	fprintf(file->fp, "%s", str);
      fprintf(file->fp, "}\n");
      if( file->thing_count == 1 )
	fprintf(file->fp, "\\label{%s}\n", file->file_name);
      else
	fprintf(file->fp, "\\label{%s:%d}\n", file->file_name,
	  file->thing_count);
      fprintf(file->fp, "\\center\\begin{tabular}{");
      MArrayForEach(row->entries, &entry, &i)
	fprintf(file->fp, "%c%s", i == 0 ? 'l' : 'r',
	  entry->rule_after ? "|" : "");
      fprintf(file->fp, "}\n");
      /* fprintf(file->fp, "\\hline\\noalign{}\n"); */
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableWriteEnd(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)     */
/*                                                                           */
/*  End writing tbl onto file.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableWriteEnd(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)
{
  switch( tbl->table_type )
  {
    case KHE_STATS_TABLE_PLAIN:

      /* nothing to do */
      break;

    case KHE_STATS_TABLE_LOUT:

      fprintf(file->fp, "}\n");
      break;

    case KHE_STATS_TABLE_LATEX:

      fprintf(file->fp, "\\end{tabular}\n");
      fprintf(file->fp, "\\end{table}\n");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableWrite(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)        */
/*                                                                           */
/*  Write tbl onto file.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheStatsTableWrite(KHE_STATS_TABLE tbl, KHE_STATS_FILE file)
{
  KHE_STATS_ROW row;  int i;
  KheStatsTableWriteBegin(tbl, file);
  MArrayForEach(tbl->rows, &row, &i)
    KheStatsRowWrite(row, i == 0, i == MArraySize(tbl->rows)-1, tbl, file->fp);
  KheStatsTableWriteEnd(tbl, file);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "stats files" (private; includes s_files)                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheStatsFileEnsureInit(void)                                        */
/*                                                                           */
/*  Ensure that this module is initialized.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheStatsFileEnsureInit(void)
{
  if( !s_init )
  {
    MTableInit(s_files);
    s_init = true;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE KheStatsFileFind(char *file_name, char *err)              */
/*                                                                           */
/*  Return the stats file object identified by file_name, or abort if none.  */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_FILE KheStatsFileFind(char *file_name, char *err)
{
  KHE_STATS_FILE res;  int pos;
  KheStatsFileEnsureInit();
  if( !MTableRetrieve(s_files, file_name, &res, &pos) )
    MAssert(false, "%s: unknown file name \"%s\"", err, file_name);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE KheStatsFileFindWithTable(char *file_name, char *err)     */
/*                                                                           */
/*  Find file and check that it is currently has a table.                    */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_FILE KheStatsFileFindWithTable(char *file_name, char *err)
{
  KHE_STATS_FILE res;
  res = KheStatsFileFind(file_name, err);
  MAssert(res->curr_table != NULL, "%s: no table in file \"%s\"", err,
    file_name);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE KheStatsFileFindWithGraph(char *file_name, char *err)     */
/*                                                                           */
/*  Find file and check that it is currently has a graph.                    */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_FILE KheStatsFileFindWithGraph(char *file_name, char *err)
{
  KHE_STATS_FILE res;
  res = KheStatsFileFind(file_name, err);
  MAssert(res->curr_graph != NULL, "%s: no graph in file \"%s\"", err,
    file_name);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE KheStatsFileFindWithTableOrGraph(char *file_name,         */
/*    char *err)                                                             */
/*                                                                           */
/*  Find file and check that it is currently has a table or graph.           */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_FILE KheStatsFileFindWithTableOrGraph(char *file_name,
  char *err)
{
  KHE_STATS_FILE res;
  res = KheStatsFileFind(file_name, err);
  MAssert(res->curr_table != NULL || res->curr_graph != NULL,
    "%s: no table or graph in file \"%s\"", err, file_name);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_FILE KheStatsFileMake(char *file_name)                         */
/*                                                                           */
/*  Make a new stats file object with these attributes.  This includes       */
/*  opening it for writing.                                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_FILE KheStatsFileMake(char *file_name)
{
  KHE_STATS_FILE res;  char buff[200];
  MMake(res);
  res->file_name = MStringCopy(file_name);
  snprintf(buff, 200, "stats/%s", file_name);
  res->fp = fopen(buff, "w");
  MAssert(res->fp != NULL, "KheStatsFileMake: cannot open file \"%s\"\n",buff);
  res->curr_table = NULL;
  res->curr_graph = NULL;
  res->thing_count = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsFileDelete(KHE_STATS_FILE file)                             */
/*                                                                           */
/*  Delete file, closing it and reclaiming its memory.                       */
/*                                                                           */
/*****************************************************************************/

static void KheStatsFileDelete(KHE_STATS_FILE file)
{
  MAssert(file->curr_table == NULL,
    "KheStatsFileDelete: cannot delete file \"%s\" (it has an open table)",
    file->file_name);
  MFree(file->file_name);
  fclose(file->fp);
  MFree(file);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "public functions - files"                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheStatsFileBegin(char *file_name)                                  */
/*                                                                           */
/*  Begin a new file with these attributes.                                  */
/*                                                                           */
/*****************************************************************************/

void KheStatsFileBegin(char *file_name)
{
  KHE_STATS_FILE res;  int pos;
  KheStatsFileEnsureInit();
  MAssert(!MTableContains(s_files, file_name, &pos),
    "KheStatsFileBegin: file_name \"%s\" already in use", file_name);
  res = KheStatsFileMake(file_name);
  MTableInsert(s_files, file_name, res);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsFileEnd(char *file_name)                                    */
/*                                                                           */
/*  End a file with these attributes.                                        */
/*                                                                           */
/*****************************************************************************/

void KheStatsFileEnd(char *file_name)
{
  KHE_STATS_FILE res;  int pos;
  KheStatsFileEnsureInit();
  if( !MTableRetrieve(s_files, file_name, &res, &pos) )
    MAssert(false, "KheStatsFileEnd: unknown file \"%s\"", file_name);
  MTableDelete(s_files, pos);
  KheStatsFileDelete(res);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "public functions - tables"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableBegin(char *file_name, KHE_STATS_TABLE_TYPE table_type,*/
/*    int col_width, char *corner, bool with_average_row,                    */
/*    bool with_total_row, bool highlight_cost_minima,                       */
/*    bool highlight_time_minima, bool highlight_int_minima)                 */
/*                                                                           */
/*  Begin a table in file_name with these attributes.                        */
/*                                                                           */
/*****************************************************************************/

void KheStatsTableBegin(char *file_name, KHE_STATS_TABLE_TYPE table_type,
  int col_width, char *corner, bool with_average_row, bool with_total_row,
  bool highlight_cost_minima, bool highlight_time_minima,
  bool highlight_int_minima)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFind(file_name, "KheStatsTableBegin");
  MAssert(file->curr_table == NULL, "KheStatsTableBegin: table already begun");
  MAssert(file->curr_graph == NULL, "KheStatsTableBegin: graph already begun");
  file->curr_table = KheStatsTableMake(table_type, col_width, corner,
    with_average_row, with_total_row, highlight_cost_minima,
    highlight_time_minima, highlight_int_minima);
  file->thing_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsTableEnd(char *file_name)                                   */
/*                                                                           */
/*  End a table.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheStatsTableEnd(char *file_name)
{
  KHE_STATS_FILE file;  KHE_STATS_ROW row;

  /* the last row (not counting any average row) has a rule below it */
  file = KheStatsFileFindWithTable(file_name, "KheStatsTableEnd");
  row = MArrayLast(file->curr_table->rows);
  row->rule_below = true;

  /* add an average row if requested */
  if( file->curr_table->with_average_row )
    KheStatsTableAddAverageRow(file->curr_table);

  /* add a total row if requested */
  if( file->curr_table->with_total_row )
    KheStatsTableAddTotalRow(file->curr_table);

  /* highlight minima, as requested */
  KheStatsTableHighlightMinima(file->curr_table);

  /* write the table */
  KheStatsTableWrite(file->curr_table, file);

  /* delete the table ready for a fresh start */
  KheStatsTableDelete(file->curr_table);
  file->curr_table = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsCaptionAdd(char *file_name, char *fmt, ...)                 */
/*                                                                           */
/*  Add one caption line to *file_name.                                      */
/*                                                                           */
/*****************************************************************************/

void KheStatsCaptionAdd(char *file_name, char *fmt, ...)
{
  va_list args;  char buff[2000];  char *str;  KHE_STATS_FILE file;

  /* get a malloced copy of the line */
  va_start(args, fmt);
  vsnprintf(buff, 2000, fmt, args);
  va_end(args);
  str = MStringCopy(buff);

  /* add it to the table or graph */
  file = KheStatsFileFindWithTableOrGraph(file_name, "KheStatsCaptionAdd");
  if( file->curr_table != NULL )
    MArrayAddLast(file->curr_table->caption_lines, str);
  else
    MArrayAddLast(file->curr_graph->caption_lines, str);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsRowAdd(char *file_name, char *row_label, bool rule_below)   */
/*                                                                           */
/*  Make a new row of file_name with these attributes.                       */
/*                                                                           */
/*****************************************************************************/

void KheStatsRowAdd(char *file_name, char *row_label, bool rule_below)
{
  KHE_STATS_FILE file;  KHE_STATS_ROW row;
  file = KheStatsFileFindWithTable(file_name, "KheStatsMakeRow");
  if( KheStatsTableContainsRow(file->curr_table, row_label, &row) )
    MAssert(false, "KheStatsMakeRow: row \"%s\" of table \"%s\" exists",
      row_label, file_name);
  row = KheStatsRowMake(row_label, rule_below);
  MArrayAddLast(file->curr_table->rows, row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsColAdd(char *file_name, char *col_label, bool rule_after)   */
/*                                                                           */
/*  Make a new column of file_name with these attributes.                    */
/*                                                                           */
/*****************************************************************************/

void KheStatsColAdd(char *file_name, char *col_label, bool rule_after)
{
  KHE_STATS_FILE file;  int pos;
  KHE_STATS_ENTRY entry;  KHE_STATS_ROW first_row;
  file = KheStatsFileFindWithTable(file_name, "KheStatsMakeCol");
  if( KheStatsTableContainsCol(file->curr_table, col_label, &pos) )
    MAssert(false, "KheStatsMakeCol: col \"%s\" of table \"%s\" exists",
      col_label, file_name);
  entry = KheStatsEntryMake(rule_after);
  KheEntryAssignString(entry, col_label);
  first_row = MArrayFirst(file->curr_table->rows);
  MArrayAddLast(first_row->entries, entry);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_ENTRY KheStatsFindEntry(char *file_name, char *row_label,      */
/*    char *col_label)                                                       */
/*                                                                           */
/*  Return the entry with these labels, or abort if none.                    */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_ENTRY KheStatsFindEntry(char *file_name, char *row_label,
  char *col_label)
{
  KHE_STATS_FILE file;  int col_index;  KHE_STATS_ROW row;
  KHE_STATS_ENTRY entry;

  /* make sure there is an entry */
  file = KheStatsFileFindWithTable(file_name, "KheStatsFindEntry");
  if( !KheStatsTableContainsRow(file->curr_table, row_label, &row) )
    MAssert(false, "KheStats: no row \"%s\" in table \"%s\"",
      row_label, file_name);
  if( !KheStatsTableContainsCol(file->curr_table, col_label, &col_index) )
    MAssert(false, "KheStats: no col \"%s\" in table \"%s\"",
      col_label, file_name);

  /* make sure the row has an entry at col_index */
  KheStatsRowFill(row, col_index);

  /* make sure the entry has not been assigned a value already, and return it */
  entry = MArrayGet(row->entries, col_index);
  MAssert(entry->entry_type == KHE_STATS_ENTRY_NONE,
    "KheStatsFindEntry(\"%s\", \"%s\", \"%s\") already has a value",
    file_name, row_label, col_label);
  return entry;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryAddString(char *file_name, char *row_label,            */
/*    char *col_label, char *str)                                            */
/*                                                                           */
/*  Set the value of the indicated entry to str.                             */
/*                                                                           */
/*****************************************************************************/

void KheStatsEntryAddString(char *file_name, char *row_label,
  char *col_label, char *str)
{
  KHE_STATS_ENTRY entry;
  entry = KheStatsFindEntry(file_name, row_label, col_label);
  KheEntryAssignString(entry, str);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryAddCost(char *file_name, char *row_label,              */
/*    char *col_label, KHE_COST cost)                                        */
/*                                                                           */
/*  Set the value of the indicated entry to cost.                            */
/*                                                                           */
/*****************************************************************************/

void KheStatsEntryAddCost(char *file_name, char *row_label,
  char *col_label, KHE_COST cost)
{
  KHE_STATS_ENTRY entry;
  entry = KheStatsFindEntry(file_name, row_label, col_label);
  KheEntryAssignCost(entry, cost);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryAddTime(char *file_name, char *row_label,              */
/*    char *col_label, float time)                                           */
/*                                                                           */
/*  Set the value of the indicated entry to time.                            */
/*                                                                           */
/*****************************************************************************/

void KheStatsEntryAddTime(char *file_name, char *row_label,
  char *col_label, float time)
{
  KHE_STATS_ENTRY entry;
  entry = KheStatsFindEntry(file_name, row_label, col_label);
  KheEntryAssignTime(entry, time);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsEntryAddInt(char *file_name, char *row_label,               */
/*    char *col_label, int val)                                              */
/*                                                                           */
/*  Set the value of the indicated entry to val.                             */
/*                                                                           */
/*****************************************************************************/

void KheStatsEntryAddInt(char *file_name, char *row_label,
  char *col_label, int val)
{
  KHE_STATS_ENTRY entry;
  entry = KheStatsFindEntry(file_name, row_label, col_label);
  KheEntryAssignInt(entry, val);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "datasets"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_DATASET KheStatsDataSetMake(char *dataset_label,               */
/*    KHE_STATS_DATASET_TYPE dataset_type)                                   */
/*                                                                           */
/*  Make a new dataset with these attributes.                                */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_DATASET KheStatsDataSetMake(char *dataset_label,
  KHE_STATS_DATASET_TYPE dataset_type)
{
  KHE_STATS_DATASET res;
  MAssert(dataset_type >= KHE_STATS_DATASET_HISTO &&
    dataset_type <= KHE_STATS_DATASET_HISTO,
    "KheStatsDataSetMake: illegal dataset_type %d", dataset_type);
  MMake(res);
  res->dataset_label = MStringCopy(dataset_label);
  res->dataset_type = dataset_type;
  MArrayInit(res->points);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsDataSetDelete(KHE_STATS_DATASET ds)                         */
/*                                                                           */
/*  Delete ds, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheStatsDataSetDelete(KHE_STATS_DATASET ds)
{
  MFree(ds->dataset_label);
  MArrayFree(ds->points);
  MFree(ds);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsDataSetWrite(KHE_STATS_DATASET ds, KHE_STATS_FILE file)     */
/*                                                                           */
/*  Write ds to file.                                                        */
/*                                                                           */
/*****************************************************************************/

static void KheStatsDataSetWrite(KHE_STATS_DATASET ds, KHE_STATS_FILE file)
{
  KHE_STATS_POINT point;  int i;
  switch( ds->dataset_type )
  {
    case KHE_STATS_DATASET_HISTO:

      fprintf(file->fp, "  @Data pairs { filledyhisto }\n");
      fprintf(file->fp, "  {\n");
      MArrayForEach(ds->points, &point, &i)
	fprintf(file->fp, "    %.2f %.2f\n", point.x - 0.5, point.y);
      if( MArraySize(ds->points) > 0 )
      {
	point = MArrayLast(ds->points);
	fprintf(file->fp, "    %.2f %.2f\n", point.x + 0.5, 0.0);
      }
      fprintf(file->fp, "  }\n");
      break;

    default:

      MAssert(false, "KheStatsDataSetWrite internal error");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "graphs"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_STATS_GRAPH KheStatsGraphMake(void)                                  */
/*                                                                           */
/*  Make a new graph with these attributes.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_STATS_GRAPH KheStatsGraphMake(void)
{
  KHE_STATS_GRAPH res;
  MMake(res);
  res->width = -1.0;
  res->height = -1.0;
  res->xmax = -1.0;
  res->ymax = -1.0;
  res->above_caption = NULL;
  res->below_caption = NULL;
  res->left_caption = NULL;
  res->right_caption = NULL;
  MArrayInit(res->datasets);
  MArrayInit(res->caption_lines);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsGraphDelete(KHE_STATS_GRAPH graph)                          */
/*                                                                           */
/*  Delete graph, reclaiming its memory.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheStatsGraphDelete(KHE_STATS_GRAPH graph)
{
  if( graph->above_caption != NULL )
    MFree(graph->above_caption);
  if( graph->below_caption != NULL )
    MFree(graph->below_caption);
  if( graph->left_caption != NULL )
    MFree(graph->left_caption);
  if( graph->right_caption != NULL )
    MFree(graph->right_caption);
  while( MArraySize(graph->datasets) > 0 )
    KheStatsDataSetDelete(MArrayRemoveLast(graph->datasets));
  MArrayFree(graph->datasets);
  while( MArraySize(graph->caption_lines) > 0 )
    MFree(MArrayRemoveLast(graph->caption_lines));
  MArrayFree(graph->caption_lines);
  MFree(graph);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheStatsGraphContainsDataSet(KHE_STATS_GRAPH graph,                 */
/*    char *dataset_label, KHE_STATS_DATASET *ds)                            */
/*                                                                           */
/*  If graph contains a dataset with this label, return true and set *ds     */
/*  to that dataset.  Otherwise return false and set *ds to NULL.            */
/*                                                                           */
/*****************************************************************************/

static bool KheStatsGraphContainsDataSet(KHE_STATS_GRAPH graph,
  char *dataset_label, KHE_STATS_DATASET *ds)
{
  KHE_STATS_DATASET ds2;  int i;
  MArrayForEach(graph->datasets, &ds2, &i)
    if( strcmp(dataset_label, ds2->dataset_label) == 0 )
    {
      *ds = ds2;
      return true;
    }
  *ds = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsGraphWrite(KHE_STATS_GRAPH graph, KHE_STATS_FILE file)      */
/*                                                                           */
/*  Write graph to file.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheStatsGraphWrite(KHE_STATS_GRAPH graph, KHE_STATS_FILE file)
{
  KHE_STATS_DATASET ds;  int i;  char *str;

  /* write @Figure */
  fprintf(file->fp, "@Figure\n");
  fprintf(file->fp, "  @OnePage { Yes }\n");
  if( file->thing_count == 1 )
    fprintf(file->fp, "  @Tag { %s }\n", file->file_name);
  else
    fprintf(file->fp, "  @Tag { %s_%d }\n", file->file_name, file->thing_count);
  if( MArraySize(graph->caption_lines) > 0 )
  {
    fprintf(file->fp, "  @Caption {\n");
    MArrayForEach(graph->caption_lines, &str, &i)
      fprintf(file->fp, "%s", str);
    fprintf(file->fp, "  }\n");
  }

  /* write @Graph and its options */
  fprintf(file->fp,
    "-2p @Font -2px @Break @OneRow @Graph\n");
  fprintf(file->fp,
    "  xextra { 0c } yextra { 0c }");
  if( graph->width > 0 )
    fprintf(file->fp, " width { %.2fc }", graph->width);
  if( graph->height > 0 )
    fprintf(file->fp, " height { %.2fc }", graph->height);
  if( graph->xmax > 0 )
    fprintf(file->fp, " xmax { %.2f }", graph->xmax);
  if( graph->ymax > 0 )
    fprintf(file->fp, " ymax { %.2f }", graph->ymax);
  fprintf(file->fp, "\n");

  if( graph->above_caption != NULL )
    fprintf(file->fp, "  abovecaption { %s }\n", graph->above_caption);
  if( graph->below_caption != NULL )
  {
    fprintf(file->fp, "  belowgap { 0.2c }\n");
    fprintf(file->fp, "  belowcaption { %s }\n", graph->below_caption);
  }
  if( graph->left_caption != NULL )
    fprintf(file->fp, "  leftcaption { %s }\n", graph->left_caption);
  if( graph->right_caption != NULL )
    fprintf(file->fp, "  rightcaption { %s }\n", graph->right_caption);
  fprintf(file->fp, "{\n");

  /* write datasets */
  MArrayForEach(graph->datasets, &ds, &i)
    KheStatsDataSetWrite(ds, file);

  /* write figure footer */
  fprintf(file->fp, "}\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "public functions - graphs"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheStatsGraphBegin(char *file_name)                                 */
/*                                                                           */
/*  Begin a graph.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheStatsGraphBegin(char *file_name)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFind(file_name, "KheStatsGraphBegin");
  MAssert(file->curr_table == NULL, "KheStatsGraphBegin: table already begun");
  MAssert(file->curr_graph == NULL, "KheStatsGraphBegin: graph already begun");
  file->curr_graph = KheStatsGraphMake();
  file->thing_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsGraphEnd(char *file_name)                                   */
/*                                                                           */
/*  End a graph, including printing it.                                      */
/*                                                                           */
/*****************************************************************************/

void KheStatsGraphEnd(char *file_name)
{
  KHE_STATS_FILE file;

  /* write the graph */
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphEnd");
  KheStatsGraphWrite(file->curr_graph, file);

  /* delete the graph ready for a fresh start */
  KheStatsGraphDelete(file->curr_graph);
  file->curr_graph = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsGraphSetWidth(char *file_name, float width)                 */
/*  void KheStatsGraphSetHeight(char *file_name, float height)               */
/*  void KheStatsGraphSetXMax(char *file_name, float xmax)                   */
/*  void KheStatsGraphSetYMax(char *file_name, float ymax)                   */
/*  void KheStatsGraphSetAboveCaption(char *file_name, char *val)            */
/*  void KheStatsGraphSetBelowCaption(char *file_name, char *val)            */
/*  void KheStatsGraphSetLeftCaption(char *file_name, char *val)             */
/*  void KheStatsGraphSetRightCaption(char *file_name, char *val)            */
/*                                                                           */
/*  Set these various global attributes of the graph.                        */
/*                                                                           */
/*****************************************************************************/

void KheStatsGraphSetWidth(char *file_name, float width)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetWidth");
  MAssert(width > 0, "KheStatsGraphSetWidth: width out of range");
  file->curr_graph->width = width;
}

void KheStatsGraphSetHeight(char *file_name, float height)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetWidth");
  MAssert(height > 0, "KheStatsGraphSetHeight: height out of range");
  file->curr_graph->height = height;
}

void KheStatsGraphSetXMax(char *file_name, float xmax)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetXMax");
  MAssert(xmax > 0, "KheStatsGraphSetXMax: xmax out of range");
  file->curr_graph->xmax = xmax;
}

void KheStatsGraphSetYMax(char *file_name, float ymax)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetYMax");
  MAssert(ymax > 0, "KheStatsGraphSetYMax: ymax out of range");
  file->curr_graph->ymax = ymax;
}

void KheStatsGraphSetAboveCaption(char *file_name, char *val)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetAboveCaption");
  MAssert(file->curr_graph->above_caption == NULL,
    "KheStatsGraphSetAboveCaption: above_caption already set");
  file->curr_graph->above_caption = MStringCopy(val);
}

void KheStatsGraphSetBelowCaption(char *file_name, char *val)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetBelowCaption");
  MAssert(file->curr_graph->below_caption == NULL,
    "KheStatsGraphSetBelowCaption: below_caption already set");
  file->curr_graph->below_caption = MStringCopy(val);
}

void KheStatsGraphSetLeftCaption(char *file_name, char *val)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetLeftCaption");
  MAssert(file->curr_graph->left_caption == NULL,
    "KheStatsGraphSetLeftCaption: left_caption already set");
  file->curr_graph->left_caption = MStringCopy(val);
}

void KheStatsGraphSetRightCaption(char *file_name, char *val)
{
  KHE_STATS_FILE file;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsGraphSetRightCaption");
  MAssert(file->curr_graph->right_caption == NULL,
    "KheStatsGraphSetRightCaption: right_caption already set");
  file->curr_graph->right_caption = MStringCopy(val);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsDataSetAdd(char *file_name, char *dataset_label,            */
/*    KHE_STATS_DATASET_TYPE dataset_type)                                   */
/*                                                                           */
/*  Add a new dataset with this label to the graph denoted by file_name.     */
/*                                                                           */
/*****************************************************************************/

void KheStatsDataSetAdd(char *file_name, char *dataset_label,
  KHE_STATS_DATASET_TYPE dataset_type)
{
  KHE_STATS_FILE file;  KHE_STATS_DATASET ds;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsDataSetAdd");
  if( KheStatsGraphContainsDataSet(file->curr_graph, dataset_label, &ds) )
    MAssert(false, "KheStatsDataSetAdd: dataset \"%s\" of graph \"%s\" exists",
      dataset_label, file_name);
  ds = KheStatsDataSetMake(dataset_label, dataset_type);
  MArrayAddLast(file->curr_graph->datasets, ds);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheStatsPointAdd(char *file_name, char *dataset_label,              */
/*    float x, float y)                                                      */
/*                                                                           */
/*  Add a point to the given dataset of the current graph.                   */
/*                                                                           */
/*****************************************************************************/

void KheStatsPointAdd(char *file_name, char *dataset_label,
  float x, float y)
{
  KHE_STATS_FILE file;  KHE_STATS_DATASET ds;  KHE_STATS_POINT point;
  file = KheStatsFileFindWithGraph(file_name, "KheStatsPointAdd");
  if( !KheStatsGraphContainsDataSet(file->curr_graph, dataset_label, &ds) )
    MAssert(false,
      "KheStatsPointAdd: dataset \"%s\" of graph \"%s\" does not exist",
      dataset_label, file_name);
  point.x = x;
  point.y = y;
  MArrayAddLast(ds->points, point);
}
