
/*****************************************************************************/
/*                                                                           */
/*  THE KTS TIMETABLING SYSTEM                                               */
/*  COPYRIGHT (C) 2004, 2008 Jeffrey H. Kingston                             */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  FILE:         kml.c                                                      */
/*  MODULE:       XML reading and writing                                    */
/*                                                                           */
/*****************************************************************************/
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>
#include <expat.h>
#include "kml.h"
#include "m.h"

#define BUFF_SIZE 1024
#define KML_MAX_STR 200
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 1


/*****************************************************************************/
/*                                                                           */
/*  Submodule "type declarations"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KML_FILE - an XML file open for writing (not reading)                    */
/*                                                                           */
/*****************************************************************************/

struct kml_file_rec {
  FILE			*fp;			/* file to write XML to      */
  int			curr_indent;		/* current indent            */
  int			indent_step;		/* indent step               */
  bool			attribute_allowed;	/* state of print            */
};


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT - an XML element                                                 */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KML_ELT) ARRAY_KML_ELT;

struct kml_elt_rec {
  int			line_num;		/* line number of element    */
  int			col_num;		/* column number of element  */
  char			*label;			/* label of element          */
  KML_ELT		parent;			/* parent of element         */
  ARRAY_STRING		attribute_names;	/* attribute names           */
  ARRAY_STRING		attribute_values;	/* attribute values          */
  ARRAY_KML_ELT		children;		/* children                  */
  char			*text;			/* text                      */
};


/*****************************************************************************/
/*                                                                           */
/*  KML_ERROR                                                                */
/*                                                                           */
/*****************************************************************************/

struct kml_error_rec {
  int			line_num;		/* line number of error      */
  int			col_num;		/* column number of error    */
  char			string[KML_MAX_STR];	/* error string              */
};


/*****************************************************************************/
/*                                                                           */
/*  KML_READ_INFO - read info passed through Expat and to the user           */
/*                                                                           */
/*****************************************************************************/

typedef MTABLE(char *) TABLE_STRING;

struct kml_read_info_rec {
  KML_ELT_FN		elt_fn;			/* call after each elt       */
  void			*impl;			/* pass to elt_fn            */
  int			max_depth;		/* max depth of callbacks    */
  int			curr_depth;		/* current depth of parse    */
  KML_ELT		curr_elt;		/* current element parsed    */
  bool			succeeded;		/* success flag              */
  TABLE_STRING		strings;		/* strings                   */
  KML_ERROR		ke;			/* error object              */
  jmp_buf		jmp_env;		/* early return              */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "string handling (private)"                                    */
/*                                                                           */
/*****************************************************************************/
#define is_space(ch) ((ch)=='\n' || (ch)=='\r' || (ch)=='\t' || (ch)==' ')
#define is_digit(ch) ((ch) >= '0' && ch <= '9')

/*****************************************************************************/
/*                                                                           */
/*  char *KmlStringCopy(const char *str)                                     */
/*                                                                           */
/*  Return a copy of str in malloced memory.                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlStringCopy(const char *str)
{
  char *res;
  res = (char *) malloc((strlen(str) + 1) * sizeof(char));
  strcpy(res, str);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlStringUniquify(KML_READ_INFO ri, const char *str)               */
/*                                                                           */
/*  Return a uniquified copy of str.                                         */
/*                                                                           */
/*****************************************************************************/

static char *KmlStringUniquify(KML_READ_INFO ri, const char *str)
{
  char *val;  int pos;
  if( !MTableRetrieve(ri->strings, str, &val, &pos) )
  {
    val = KmlStringCopy(str);
    MTableInsert(ri->strings, val, val);
  }
  return val;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlStringHasNonWhite(const char *str, int len)                      */
/*                                                                           */
/*  Return true if str has non-white spaces in it.                           */
/*                                                                           */
/*****************************************************************************/

static bool KmlStringHasNonWhite(const char *str, int len)
{
  int i;
  for( i = 0;  i < len;  i++ )
    if( !is_space(str[i]) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlStringContainsOneIntegerOnly(char *s)                            */
/*                                                                           */
/*  Return true if s contains exactly one non-negative integer, possibly     */
/*  with white space on either side.                                         */
/*                                                                           */
/*****************************************************************************/

static bool KmlStringContainsOneIntegerOnly(char *s)
{
  char *p;
  if( s == NULL )
    return false;
  
  /* skip zero or more preceding spaces */
  for( p = s;  is_space(*p);  p++ );

  /* skip one or more digits */
  if( !is_digit(*p) )
    return false;
  do { p++; } while( is_digit(*p) );

  /* skip zero or more following spaces */
  while( is_space(*p) )  p++;

  /* that must be the end */
  return *p == '\0';
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlStringContainsOneFloatOnly(char *s)                              */
/*                                                                           */
/*  Return true if s contains exactly one non-negative float, possibly       */
/*  with white space on either side.                                         */
/*                                                                           */
/*****************************************************************************/

static bool KmlStringContainsOneFloatOnly(char *s)
{
  char *p;
  if( s == NULL )
    return false;
  
  /* skip zero or more preceding spaces */
  for( p = s;  is_space(*p);  p++ );

  if( *p == '.' )
  {
    /* grammar for this case is .[0-9]+ */
    p++;
    if( !is_digit(*p) )
      return false;
    do { p++; } while( is_digit(*p) );
  }
  else
  {
    /* grammar for this case is [0-9]+[.[0-9]*] */
    if( !is_digit(*p) )
      return false;
    do { p++; } while( is_digit(*p) );
    if( *p == '.' )
    {
      p++;
      while( is_digit(*p) )
	p++;
    }
  }

  /* skip zero or more following spaces */
  while( is_space(*p) )  p++;

  /* that must be the end */
  return *p == '\0';
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "A.4.1 Representing XML in memory"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KmlLineNum(KML_ELT elt)                                              */
/*                                                                           */
/*  Return the line number of elt.                                           */
/*                                                                           */
/*****************************************************************************/

int KmlLineNum(KML_ELT elt)
{
  return elt->line_num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlColNum(KML_ELT elt)                                               */
/*                                                                           */
/*  Return the column number of elt.                                         */
/*                                                                           */
/*****************************************************************************/

int KmlColNum(KML_ELT elt)
{
  return elt->col_num;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlLabel(KML_ELT elt)                                              */
/*                                                                           */
/*  Return the label of elt.                                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlLabel(KML_ELT elt)
{
  return elt->label;
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlParent(KML_ELT elt)                                           */
/*                                                                           */
/*  Return the parent of elt.  This may be NULL.                             */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlParent(KML_ELT elt)
{
  return elt->parent;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlAttributeCount(KML_ELT elt)                                       */
/*                                                                           */
/*  Return the number of attributes of elt.                                  */
/*                                                                           */
/*****************************************************************************/

int KmlAttributeCount(KML_ELT elt)
{
  return MArraySize(elt->attribute_names);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlAttributeName(KML_ELT elt, int index)                           */
/*                                                                           */
/*  Return the name of the index'th attribute of elt, where                  */
/*  0 <= index <= KmlAttributeCount(elt).                                    */
/*                                                                           */
/*****************************************************************************/

char *KmlAttributeName(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_names));
  return MArrayGet(elt->attribute_names, index);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlAttributeValue(KML_ELT elt, int index)                          */
/*                                                                           */
/*  Return the value of the index'th attribute of elt, where                 */
/*  0 <= index <= KmlAttributeCount(elt).                                    */
/*                                                                           */
/*****************************************************************************/

char *KmlAttributeValue(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_values));
  return MArrayGet(elt->attribute_values, index);
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlChildCount(KML_ELT elt)                                           */
/*                                                                           */
/*  Return the number of children of elt.                                    */
/*                                                                           */
/*****************************************************************************/

int KmlChildCount(KML_ELT elt)
{
  return MArraySize(elt->children);
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlChild(KML_ELT elt, int index)                                 */
/*                                                                           */
/*  Return the index'th child of elt, where 0 <= index < KmlChildCount(elt), */
/*  or if index is negative, from the back (-1 means last, etc.).            */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlChild(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlChildCount(elt) + index;
  assert(index < MArraySize(elt->children));
  return MArrayGet(elt->children, index);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlLabelMatches(char *label, char *pattern)                         */
/*                                                                           */
/*  Return true if *label matches *pattern.                                  */
/*                                                                           */
/*****************************************************************************/

/* true if the first len characters of p1 and p2 (which exist) are equal */
/* ***
static bool streqlen(char *p1, char *p2, int len)
{
  int i;
  for( i = 0;  i < len;  i++ )
    if( p1[i] != p2[i] )
      return false;
  return true;
}

static bool KmlLabelMatches(char *label, char *pattern)
{
  char *p, *next_p;  int len;
  len = strlen(label);
  for( p = pattern;  true;  p = next_p + 1 )
  {
    for( next_p = p;  *next_p != '\0' && *next_p != '|';  next_p++ );
    if( len == (next_p - p) && streqlen(label, p, len) )
      return true;
    if( *next_p == '\0' )
      break;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt)      */
/*                                                                           */
/*  If elt contains at least one child with this label, return true and      */
/*  set *child_elt to the first such child.  Otherwise return false and      */
/*  set *child_elt to NULL.                                                  */
/*                                                                           */
/*****************************************************************************/

bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt)
{
  KML_ELT e;  int i;
  MArrayForEach(elt->children, &e, &i)
    /* if( KmlLabelMatches(e->label, label) ) */
    if( strcmp(e->label, label) == 0 )
    {
      *child_elt = e;
      return true;
    }
  *child_elt = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlText(KML_ELT elt)                                               */
/*                                                                           */
/*  Return the text of elt.  This could be NULL.                             */
/*                                                                           */
/*****************************************************************************/

char *KmlText(KML_ELT elt)
{
  return elt->text;
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlMakeElt(int line_num, int col_num, char *label)               */
/*                                                                           */
/*  Make a new XML element with these attributes.  Initially the node        */
/*  has no parent, but if it is added as a child by KmlAddChild it will      */
/*  get a parent then.                                                       */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlMakeElt(int line_num, int col_num, char *label)
{
  KML_ELT res;
  res = (KML_ELT) malloc(sizeof(struct kml_elt_rec));
  res->line_num = line_num;
  res->col_num = col_num;
  res->label = label;
  res->parent = NULL;
  MArrayInit(res->attribute_names);
  MArrayInit(res->attribute_values);
  MArrayInit(res->children);
  res->text = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddAttribute(KML_ELT elt, char *name, char *value)               */
/*                                                                           */
/*  Add an attribute with this name and value to elt.                        */
/*                                                                           */
/*****************************************************************************/

void KmlAddAttribute(KML_ELT elt, char *name, char *value)
{
  assert(name != NULL && strlen(name)>0 && value != NULL && strlen(value)>0);
  MArrayAddLast(elt->attribute_names, name);
  MArrayAddLast(elt->attribute_values, value);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddChild(KML_ELT elt, KML_ELT child)                             */
/*                                                                           */
/*  Add this child to elt.                                                   */
/*                                                                           */
/*****************************************************************************/

void KmlAddChild(KML_ELT elt, KML_ELT child)
{
  if( DEBUG2 )
  {
    int i;  KML_ELT c;
    fprintf(stderr, "[ KmlAddChild(%d:%d:<%s>, %d:%d:<%s>)\n",
      elt->line_num, elt->col_num, elt->label,
      child->line_num, child->col_num, child->label);
    fprintf(stderr, "  %d children:\n", MArraySize(elt->children));
    MArrayForEach(elt->children, &c, &i)
      fprintf(stderr, "    %d: %s\n", i, c->label);
  }
  /* allowing text now, but will be an error later
  assert(elt->text == NULL);
  *** */
  MArrayAddLast(elt->children, child);
  child->parent = elt;
  if( DEBUG2 )
    fprintf(stderr, "] KmlAddChild returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlDeleteChild(KML_ELT elt, KML_ELT child)                          */
/*                                                                           */
/*  Delete child from elt.  It must be present.                              */
/*                                                                           */
/*****************************************************************************/

void KmlDeleteChild(KML_ELT elt, KML_ELT child)
{
  int pos;
  if( !MArrayContains(elt->children, child, &pos) )
    assert(false);
  MArrayRemove(elt->children, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddTextLen(KML_ELT elt, const char *text, int len)               */
/*                                                                           */
/*  Add text, which is known to have the given length, to any text that      */
/*  is already present.  There may be no children.                           */
/*                                                                           */
/*  If the string to be added contains nothing but white space, then it      */
/*  is not added at all.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KmlAddTextLen(KML_ELT elt, const char *text, int len)
{
  int curr_len;
  if( KmlStringHasNonWhite(text, len) )
  {
    assert(MArraySize(elt->children) == 0);
    curr_len = (elt->text != NULL ? strlen(elt->text) : 0);
    elt->text = (char *) realloc(elt->text, (curr_len+len+1) * sizeof(char));
    strncpy(&elt->text[curr_len], text, len);
    elt->text[curr_len + len] = '\0';
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddText(KML_ELT elt, char *text)                                 */
/*                                                                           */
/*  Add text to elt, appending it to any text already present.  There may    */
/*  not be any children beforehand.                                          */
/*                                                                           */
/*****************************************************************************/

void KmlAddText(KML_ELT elt, char *text)
{
  KmlAddTextLen(elt, text, strlen(text));
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlFree(KML_ELT elt, bool free_attribute_values, bool free_text)    */
/*                                                                           */
/*  Free the memory occuped by elt and its descendants.                      */
/*                                                                           */
/*  Implementation note.  This function utilizes a detail of the             */
/*  specification of free():  if its parameter is NULL, it does nothing.     */
/*                                                                           */
/*****************************************************************************/

void KmlFree(KML_ELT elt, bool free_attribute_values, bool free_text)
{
  KML_ELT child;  char *str;  int i;
  if( DEBUG3 )
    fprintf(stderr, "[ KmlFree(%p %s, ...)\n", (void *) elt, elt->label);

  /* free the attribute names array */
  MArrayFree(elt->attribute_names);

  /* free the attribute values */
  if( free_attribute_values )
    MArrayForEach(elt->attribute_values, &str, &i)
      MFree(str);
  MArrayFree(elt->attribute_values);

  /* free the children */
  MArrayForEach(elt->children, &child, &i)
    KmlFree(child, free_attribute_values, free_text);
  MArrayFree(elt->children);

  /* free the text and elt itself */
  if( free_text )
    MFree(elt->text);
  MFree(elt);
  if( DEBUG3 )
    fprintf(stderr, "] KmlFree\n");
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractLabel(KML_ELT elt)                                       */
/*                                                                           */
/*  Return the label of elt, and set it to NULL at the same time.            */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractLabel(KML_ELT elt)
{
  char *res;
  res = elt->label;
  elt->label = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractAttributeName(KML_ELT elt, int index)                    */
/*                                                                           */
/*  Return the name of the index'th attribute of elt, and set it to NULL     */
/*  at the same time.                                                        */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractAttributeName(KML_ELT elt, int index)
{
  char *res;
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_names));
  res = MArrayGet(elt->attribute_names, index);
  MArrayPut(elt->attribute_names, index, NULL);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractAttributeValue(KML_ELT elt, int index)                   */
/*                                                                           */
/*  Return the value of the index'th attribute of elt, and set it to NULL    */
/*  at the same time.                                                        */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractAttributeValue(KML_ELT elt, int index)
{
  char *res;
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_values));
  res = MArrayGet(elt->attribute_values, index);
  MArrayPut(elt->attribute_values, index, NULL);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractText(KML_ELT elt)                                        */
/*                                                                           */
/*  Return the text of elt and set it to NULL at the same time.              */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractText(KML_ELT elt)
{
  char *res;
  res = elt->text;
  elt->text = NULL;
  return res == NULL ? "" : res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "A.4.2 Error handing and format checking"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KmlErrorLineNum(KML_ERROR ke)                                        */
/*                                                                           */
/*  Return the line number attribute of error ke.                            */
/*                                                                           */
/*****************************************************************************/

int KmlErrorLineNum(KML_ERROR ke)
{
  return ke->line_num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlErrorColNum(KML_ERROR ke)                                         */
/*                                                                           */
/*  Return the column number attribute of error ke.                          */
/*                                                                           */
/*****************************************************************************/

int KmlErrorColNum(KML_ERROR ke)
{
  return ke->col_num;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlErrorString(KML_ERROR ke)                                       */
/*                                                                           */
/*  Return the string attribute of error ke.                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlErrorString(KML_ERROR ke)
{
  return ke->string;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlErrorMake(KML_ERROR *ke, int line_num, int col_num,              */
/*    char *fmt, ...)                                                        */
/*                                                                           */
/*  Make a KML error object unless ke is NULL.  For convenience when using   */
/*  this function, *ke is set to the new object, and false is returned.      */
/*                                                                           */
/*****************************************************************************/

KML_ERROR KmlErrorMake(int line_num, int col_num, char *fmt, ...)
{
  KML_ERROR res;  va_list ap;
  MMake(res);
  res->line_num = line_num;
  res->col_num = col_num;
  va_start(ap, fmt);
  vsnprintf(res->string, KML_MAX_STR, fmt, ap);
  va_end(ap);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ERROR KmlVErrorMake(int line_num, int col_num, char *fmt,            */
/*    va_list ap)                                                            */
/*                                                                           */
/*  Make a KML error object.                                                 */
/*                                                                           */
/*****************************************************************************/

KML_ERROR KmlVErrorMake(int line_num, int col_num, char *fmt, va_list ap)
{
  KML_ERROR res;
  MMake(res);
  res->line_num = line_num;
  res->col_num = col_num;
  vsnprintf(res->string, KML_MAX_STR, fmt, ap);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlError(KML_ERROR *ke, int line_num, int col_num,                  */
/*    char *fmt, ...)                                                        */
/*                                                                           */
/*  Make a KML error object unless ke is NULL.  For convenience when using   */
/*  this function, *ke is set to the new object, and false is returned.      */
/*                                                                           */
/*****************************************************************************/

bool KmlError(KML_ERROR *ke, int line_num, int col_num, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  *ke = KmlVErrorMake(line_num, col_num, fmt, ap);
  va_end(ap);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool NextTerm(char **fmt, char *name, TERM_TYPE *et, FACTOR_TYPE *ft)    */
/*                                                                           */
/*  If *fmt contains a next term, set name, *et, and *ft to its              */
/*  attributes, move *fmt past the term ready to read the next term.        */
/*  and return true.  If there is no next term, return false.                */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  TERM_COLON,
  TERM_COMPULSORY,
  TERM_OPTIONAL,
  TERM_SEQUENCE
} TERM_TYPE;

typedef enum {
  FACTOR_NO_TEXT,
  FACTOR_OPTIONAL_TEXT,
  FACTOR_INTEGER_TEXT,
  FACTOR_FLOAT_TEXT
} FACTOR_TYPE;

static bool NextTerm(char **fmt, char *name, TERM_TYPE *et, FACTOR_TYPE *ft)
{
  /* skip initial spaces and return false if exhausted */
  while( **fmt == ' ')
    (*fmt)++;
  if( **fmt == '\0' )
    return false;

  /* colon is special case */
  if( **fmt == ':' )
  {
    *et = TERM_COLON;
    (*fmt)++;
    return true;
  }

  /* optional + or * */
  if( **fmt == '+' )
  {
    *et = TERM_OPTIONAL;
    (*fmt)++;
  }
  else if( **fmt == '*' )
  {
    *et = TERM_SEQUENCE;
    (*fmt)++;
  }
  else
    *et = TERM_COMPULSORY;

  /* optional $ or # */
  if( **fmt == '$' )
  {
    *ft = FACTOR_OPTIONAL_TEXT;
    (*fmt)++;
  }
  else if( **fmt == '#' )
  {
    *ft = FACTOR_INTEGER_TEXT;
    (*fmt)++;
  }
  else if( **fmt == '%' )
  {
    *ft = FACTOR_FLOAT_TEXT;
    (*fmt)++;
  }
  else
    *ft = FACTOR_NO_TEXT;

  /* label proper */
  sscanf(*fmt, "%s", name);
  *fmt += strlen(name);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool NextAttribute(KML_ELT elt, int *index, char *name, char *val)       */
/*                                                                           */
/*  If elt contains a next attribute (at *index, that is) with the given     */
/*  name, set *val to its value, move *index past it ready for the next      */
/*  attribute, and return true.  Otherwise return false.                     */
/*                                                                           */
/*****************************************************************************/

static bool NextAttribute(KML_ELT elt, int *index, char *name, char **val)
{
  if( *index < KmlAttributeCount(elt) &&
      /* KmlLabelMatches(MArrayGet(elt->attribute_names, *index), name) ) */
      strcmp(MArrayGet(elt->attribute_names, *index), name) == 0 )
  {
    *val = MArrayGet(elt->attribute_values, *index);
    (*index)++;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool NextChild(KML_ELT elt, int *index, char *name, KML_ELT *child)      */
/*                                                                           */
/*  If elt contains a next child (at *index, that is) with the given         */
/*  name, set *child to that child, move *index past it ready for the next   */
/*  child, and return true.  Otherwise return false.                         */
/*                                                                           */
/*****************************************************************************/

static bool NextChild(KML_ELT elt, int *index, char *name, KML_ELT *child)
{
  if( *index < KmlChildCount(elt) &&
      /* KmlLabelMatches(KmlLabel(MArrayGet(elt->children, *index)), name) ) */
      strcmp(KmlLabel(MArrayGet(elt->children, *index)), name) == 0 )
  {
    *child = MArrayGet(elt->children, *index);
    (*index)++;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke)                     */
/*                                                                           */
/*  If the attributes and children of elt conform to fmt, return true.       */
/*  Otherwise, return false and set *ke to an error object describing the    */
/*  first problem encountered.                                               */
/*                                                                           */
/*  Each fmt string is a sequences of zero or more terms separated by        */
/*  one or more space characters:                                            */
/*                                                                           */
/*    fmt ::= { term } [ ":" { term } ]                                      */
/*                                                                           */
/*  Terms before the colon (if any) describe attributes; terms after the     */
/*  colon describe children.  Each term may be any one of                    */
/*                                                                           */
/*    term ::= factor               Exactly one must be present              */
/*    term ::= "+" factor           Zero or one must be present              */
/*    term ::= "*" factor           Zero or more must be present             */
/*                                                                           */
/*  Each factor may be any one of                                            */
/*                                                                           */
/*    factor ::= label              No text allowed                          */
/*    factor ::= "$" label          Text may be present, children may not    */
/*    factor ::= "#" label          Text denoting integer must be present    */
/*    factor ::= "%" label          Text denoting float must be present      */
/*                                                                           */
/*  There may be no spaces within terms and factors.                         */
/*                                                                           */
/*****************************************************************************/

#define test_attribute()						\
{									\
  if( ft == FACTOR_INTEGER_TEXT &&					\
      !KmlStringContainsOneIntegerOnly(val) )				\
  if( ft == FACTOR_FLOAT_TEXT &&					\
      !KmlStringContainsOneFloatOnly(val) )				\
    return KmlError(ke, elt->line_num, elt->col_num,			\
      "in <%s>, attribute %s does not have float value",		\
      elt->label, name);						\
}

#define test_child()							\
{									\
  if( ft != FACTOR_NO_TEXT && KmlChildCount(child) > 0 )		\
    return KmlError(ke, child->line_num, child->col_num,		\
      "child <%s> of <%s> has unexpected (out of order?) children",	\
      child->label, elt->label);					\
  if( ft == FACTOR_INTEGER_TEXT &&					\
      !KmlStringContainsOneIntegerOnly(child->text) )			\
    return KmlError(ke, child->line_num, child->col_num,		\
      "child <%s> of <%s> does not have integer value",			\
      child->label, elt->label, child->text);				\
  if( ft == FACTOR_FLOAT_TEXT &&					\
      !KmlStringContainsOneFloatOnly(child->text) )			\
    return KmlError(ke, child->line_num, child->col_num,		\
      "child <%s> of <%s> does not have float value",			\
      child->label, elt->label, child->text);				\
}


bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke)
{
  int index;  char *p, *val;
  char name[200];  TERM_TYPE et;  FACTOR_TYPE ft;  KML_ELT child;
  if( DEBUG1 )
    fprintf(stderr, "[ KmlCheck(elt, \"%s\")\n", fmt);

  /* check attributes */
  p = fmt;  index = 0;
  while( NextTerm(&p, name, &et, &ft) && et != TERM_COLON ) switch( et )
  {
    case TERM_COMPULSORY:

      if( !NextAttribute(elt, &index, name, &val) )
	return KmlError(ke, elt->line_num, elt->col_num,
	  "in <%s>, attribute %s missing or out of order", elt->label, name);
      test_attribute();
      break;

    case TERM_OPTIONAL:

      if( NextAttribute(elt, &index, name, &val) )
	test_attribute();
      break;

    case TERM_SEQUENCE:

      while( NextAttribute(elt, &index, name, &val) )
	test_attribute();
      break;

    default:

      assert(false);
  }
  if( index < KmlAttributeCount(elt) )
    return KmlError(ke, elt->line_num, elt->col_num,
      "in <%s>, unexpected (out of order?) attribute %s", elt->label,
      MArrayGet(elt->attribute_names, index));

  /* check children */
  index = 0;
  while( NextTerm(&p, name, &et, &ft) )  switch( et )
  {
    case TERM_COMPULSORY:

      if( !NextChild(elt, &index, name, &child) )
	return KmlError(ke, elt->line_num, elt->col_num,
	  "in <%s>, child <%s> missing or out of order", elt->label, name);
      test_child();
      break;

    case TERM_OPTIONAL:

      if( NextChild(elt, &index, name, &child) )
	test_child();
      break;

    case TERM_SEQUENCE:

      while( NextChild(elt, &index, name, &child) )
	test_child();
      break;

    default:

      assert(false);
  }
  if( index < KmlChildCount(elt) )
  {
    child = KmlChild(elt, index);
    return KmlError(ke, child->line_num, child->col_num,
      "unexpected (out of order?) child <%s> in <%s>",
      child->label, elt->label);
  }
  if( KmlChildCount(elt) > 0 && elt->text != NULL )
    return KmlError(ke, elt->line_num, elt->col_num,
      "element <%s> contains both children and text", elt->label);

  /* all fine */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "A.4.3 Reading XML files"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void CharacterDataHandler(void *userData, const XML_Char *s, int len)    */
/*                                                                           */
/*  Character data handler.                                                  */
/*                                                                           */
/*****************************************************************************/

static void CharacterDataHandler(void *userData, const XML_Char *s, int len)
{
  XML_Parser p;  KML_READ_INFO ri;
  p = (XML_Parser) userData;
  ri = (KML_READ_INFO) XML_GetUserData(p);
  KmlAddTextLen(ri->curr_elt, s, len);
}


/*****************************************************************************/
/*                                                                           */
/*  void StartElementHandler(void *userData, const XML_Char *name,           */
/*     const XML_Char **atts)                                                */
/*                                                                           */
/*  Handler for starting an element.                                         */
/*                                                                           */
/*****************************************************************************/

static void StartElementHandler(void *userData, const XML_Char *name,
   const XML_Char **atts)
{
  XML_Parser p;  KML_ELT parent, child;  int i;  KML_READ_INFO ri;

  /* get user data */
  p = (XML_Parser) userData;
  ri = (KML_READ_INFO) XML_GetUserData(p);
  parent = ri->curr_elt;

  /* create child and add to parent */
  child = KmlMakeElt(XML_GetCurrentLineNumber(p),
    XML_GetCurrentColumnNumber(p)+1, KmlStringUniquify(ri, name));
  KmlAddChild(parent, child);

  /* add attributes to child */
  for( i = 0;  atts[i] != NULL;  i += 2 )
    KmlAddAttribute(child, KmlStringUniquify(ri, atts[i]),
      KmlStringCopy(atts[i+1]));

  /* set current element to child */
  ri->curr_elt = child;
  ri->curr_depth++;

  /* set user data to child */
  /* XML_SetUserData(p, (void *) child); */
}


/*****************************************************************************/
/*                                                                           */
/*  void EndElementHandler(void *userData, const XML_Char *name)             */
/*                                                                           */
/*  Handler for ending an element.                                           */
/*                                                                           */
/*****************************************************************************/

static void EndElementHandler(void *userData, const XML_Char *name)
{
  XML_Parser p;  KML_ELT elt;  int i;  KML_READ_INFO ri;
  p = (XML_Parser) userData;
  ri = (KML_READ_INFO) XML_GetUserData(p);
  elt = ri->curr_elt;
  assert(elt != NULL);
  ri->curr_elt = elt->parent;
  /* XML_SetUserData(p, (void *) elt->parent); */

  /* remove trailing white space from text */
  if( elt->text != NULL )
  {
    for( i = strlen(elt->text) - 1;  i >= 0 && is_space(elt->text[i]);  i-- );
    elt->text[i+1] = '\0';
  }

  /* call back if required */
  if( ri->elt_fn != NULL && ri->curr_depth <= ri->max_depth )
    ri->elt_fn(elt, ri);
  ri->curr_depth--;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlReadFile(FILE *fp, KML_ELT *res, KML_ERROR *ke,                  */
/*    char *end_label, char **leftover, int *leftover_len, FILE *echo_fp)    */
/*                                                                           */
/*  Read an XML element from fp.  If successful, set *res to the element     */
/*  and return true.  Otherwise set *ke to an error object describing the    */
/*  first problem encountered, and return false.                             */
/*                                                                           */
/*  Also, if end_label is non-NULL, set *leftover to point to the first      */
/*  character in the last buffer following </end_label>, and leftover_len    */
/*  to the number of characters in the buffer from that point onwards.       */
/*                                                                           */
/*****************************************************************************/

bool KmlReadFile(FILE *fp, KML_ELT *res, KML_ERROR *ke,
  char *end_label, char **leftover, int *leftover_len, FILE *echo_fp)
{
  return KmlReadFileIncremental(fp, res, ke, end_label, leftover,
    leftover_len, echo_fp, NULL, NULL, -1);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlReadString(char *str, char *stop_str, KML_ELT *res,KML_ERROR *ke)*/
/*                                                                           */
/*  Like KmlReadFile just above, but reading from a string, not from a file. */
/*                                                                           */
/*****************************************************************************/

bool KmlReadString(char *str, KML_ELT *res, KML_ERROR *ke)
{
  XML_Parser p;  KML_ELT root;
  struct kml_read_info_rec ri_rec;

  /* set up the expat parser */
  p = XML_ParserCreate(NULL);
  XML_UseParserAsHandlerArg(p);
  XML_SetElementHandler(p, &StartElementHandler, &EndElementHandler);
  XML_SetCharacterDataHandler(p, &CharacterDataHandler);
  root = KmlMakeElt(0, 0, "Root");

  /* initialize the user data record */
  ri_rec.elt_fn = NULL;
  ri_rec.impl = NULL;
  ri_rec.max_depth = -1;
  ri_rec.curr_depth = -1;
  ri_rec.curr_elt = root;
  ri_rec.succeeded = true;
  MTableInit(ri_rec.strings);
  ri_rec.ke = NULL;
  XML_SetUserData(p, (void *) &ri_rec);

  /* parse the string */
  *res = NULL;
  if( !XML_Parse(p, str, strlen(str), true) )
    return KmlError(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "%s",
      XML_ErrorString(XML_GetErrorCode(p)));

  /* check the result */
  if( ri_rec.curr_elt != root )
    return KmlError(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "input string terminated early");
  if( MArraySize(root->children) != 1 )
    return KmlError(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "%d outer elements in input string",
      MArraySize(root->children));

  /* return the first child as result */
  *res = MArrayFirst(root->children);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlReadFileIncremental(FILE *fp, KML_ELT *res, KML_ERROR *ke,       */
/*    char *end_label, char **leftover, int *leftover_len,  FILE *echo_fp,   */
/*    KML_ELT_FN elt_fn, void *impl, int max_depth)                          */
/*                                                                           */
/*  Incrementally read fp, calling back after each element of depth at       */
/*  most max_depth.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KmlReadFileIncremental(FILE *fp, KML_ELT *res, KML_ERROR *ke,
  char *end_label, char **leftover, int *leftover_len,  FILE *echo_fp,
  KML_ELT_FN elt_fn, void *impl, int max_depth)
{
  int bytes_read, end_label_len_sofar, end_label_len;  bool last_buff;
  XML_Parser parser;  KML_ELT root;  char *buff, *p, *stop_p;
  struct kml_read_info_rec ri_rec;
  if( DEBUG4 )
    fprintf(stderr, "[ KmlReadFileIncremental(fp, ... , %s, ... , max %d)\n",
      end_label == NULL ? NULL : end_label, max_depth);

  /* if end_label in use, make sure it is all in order */
  if( end_label != NULL )
  {
    end_label_len = strlen(end_label);
    assert(leftover != NULL);
    assert(leftover_len != NULL);
    assert(end_label_len >= 1);
    assert(end_label_len <= BUFF_SIZE);
    end_label_len_sofar = 0;
  }
  else
  {
    /* these variables unused in this case; this keeps the compiler happy */
    end_label_len = 0;
    end_label_len_sofar = 0;
  }

  /* set up the expat parser, and get a malloced buffer */
  parser = XML_ParserCreate(NULL);
  XML_UseParserAsHandlerArg(parser);
  XML_SetElementHandler(parser, &StartElementHandler, &EndElementHandler);
  XML_SetCharacterDataHandler(parser, &CharacterDataHandler);
  root = KmlMakeElt(0, 0, "Root");
  buff = malloc(BUFF_SIZE * sizeof(char));
  assert(buff != NULL);

  /* initialize the user data record */
  ri_rec.elt_fn = elt_fn;
  ri_rec.impl = impl;
  ri_rec.max_depth = max_depth;
  ri_rec.curr_depth = -1;
  ri_rec.curr_elt = root;
  ri_rec.succeeded = true;
  MTableInit(ri_rec.strings);
  ri_rec.ke = NULL;
  XML_SetUserData(parser, (void *) &ri_rec);

  /* set *res to NULL now in case an error return occurs */
  *res = NULL;

  /* read the file */
  do
  {
    /* read one buffer's worth of fp */
    bytes_read = fread(buff, sizeof(char), BUFF_SIZE, fp);
    if( DEBUG4 )
      fprintf(stderr, "  fread(buff, %d, %d, fp) = %d (sofar = %d)\n",
        (int) sizeof(char), BUFF_SIZE, bytes_read, end_label_len_sofar);
    if( echo_fp != NULL && bytes_read > 0 )
      fwrite(buff, sizeof(char), bytes_read, echo_fp);

    /* sort out whether the read is ending, and if so where in buff */
    last_buff = false;
    if( end_label == NULL )
    {
      /* read ends at end of file */
      last_buff = (bytes_read < BUFF_SIZE);
    }
    else
    {
      /* if no hope of finding end_label, report an error */
      if( bytes_read == 0 )
      {
	if( DEBUG4 )
	  fprintf(stderr, "] KmlReadFileIncremental ret false (a)\n");
	return KmlError(ke, XML_GetCurrentLineNumber(parser),
	  XML_GetCurrentColumnNumber(parser), "end_label \"%s\" missing",
	  end_label);
      }
  
      /* if prefix of end_label ended prev buff, check for suffix at start */
      if( end_label_len_sofar > 0 &&
	  bytes_read >= (end_label_len - end_label_len_sofar) &&
	  memcmp(buff, &end_label[end_label_len_sofar],
	    end_label_len - end_label_len_sofar) == 0 )
      {
	last_buff = true;
	*leftover = &buff[end_label_len - end_label_len_sofar];
	*leftover_len = bytes_read - (*leftover - buff);
	bytes_read -= *leftover_len;
	if( DEBUG4 )
	  fprintf(stderr, "    suffix of end_label found\n");
      }

      /* check whether all of end_label appears anywhere within buff */
      if( !last_buff && bytes_read >= end_label_len )
      {
	stop_p = &buff[bytes_read - end_label_len + 1];
	for( p = buff;  p != stop_p;  p++ )
	  if( memcmp(p, end_label, end_label_len) == 0 )
	  {
	    /* XML ends just before p[end_label_len] */
	    last_buff = true;
	    *leftover = &p[end_label_len];
	    *leftover_len = bytes_read - (*leftover - buff);
	    bytes_read -= *leftover_len;
	    if( DEBUG4 )
	      fprintf(stderr, "    whole of end_label found\n");
	    break;
	  }
      }

      /* check whether a prefix of end_label appears at the end of buff */
      if( !last_buff )
      {
	end_label_len_sofar = end_label_len - 1;
	if( end_label_len_sofar > bytes_read )
	  end_label_len_sofar = bytes_read;
	for( p = &buff[bytes_read - end_label_len_sofar];
	     end_label_len_sofar > 0;  p++, end_label_len_sofar-- )
	  if( memcmp(p, end_label, end_label_len_sofar) == 0 )
	  {
	    if( DEBUG4 )
	      fprintf(stderr, "    prefix of end_label found; sofar = %d\n",
		end_label_len_sofar);
	    break;
	  }
      }
    }

    /* parse either all of buff or up to where it ends */
    if( setjmp(ri_rec.jmp_env) == 0 )
    {
      if( !XML_Parse(parser, buff, bytes_read, last_buff) )
      {
	MTableFree(ri_rec.strings);
	if( DEBUG4 )
	  fprintf(stderr, "] KmlReadFileIncremental ret false (b)\n");
	return KmlError(ke, XML_GetCurrentLineNumber(parser),
	  XML_GetCurrentColumnNumber(parser), "%s",
	  XML_ErrorString(XML_GetErrorCode(parser)));
      }
    }
    if( !ri_rec.succeeded )
    {
      *ke = ri_rec.ke;
      assert(*ke != NULL);
      MTableFree(ri_rec.strings);
      if( DEBUG4 )
	fprintf(stderr, "] KmlReadFileIncremental ret false (c)\n");
      return false;
    }

  } while( !last_buff );

  /* check the result and reclaim memory */
  MTableFree(ri_rec.strings);
  if( ri_rec.curr_elt != root )
  {
    if( DEBUG4 )
      fprintf(stderr, "] KmlReadFileIncremental ret false (d)\n");
    return KmlError(ke, XML_GetCurrentLineNumber(parser),
      XML_GetCurrentColumnNumber(parser), "XML terminated early");
  }
  if( MArraySize(root->children) != 1 )
  {
    if( DEBUG4 )
      fprintf(stderr, "] KmlReadFileIncremental ret false (e)\n");
    return KmlError(ke, XML_GetCurrentLineNumber(parser),
      XML_GetCurrentColumnNumber(parser), "%d outer elements in input file",
      MArraySize(root->children));
  }
  XML_ParserFree(parser);
  if( end_label == NULL )
    free(buff);

  /* return the first child as result */
  *res = MArrayFirst(root->children);
  if( DEBUG4 )
    fprintf(stderr, "] KmlReadFileIncremental return true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KmlReadImpl(KML_READ_INFO ri)                                      */
/*                                                                           */
/*  Return the impl parameter of KmlReadFileIncremental.                     */
/*                                                                           */
/*****************************************************************************/

void *KmlReadImpl(KML_READ_INFO ri)
{
  return ri->impl;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlReadMaxDepth(KML_READ_INFO ri)                                    */
/*                                                                           */
/*  Return the max_depth parameter of KmlReadFileIncremental.                */
/*                                                                           */
/*****************************************************************************/

int KmlReadMaxDepth(KML_READ_INFO ri)
{
  return ri->max_depth;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlReadCurrDepth(KML_READ_INFO ri)                                   */
/*                                                                           */
/*  Return the current depth of the read.                                    */
/*                                                                           */
/*****************************************************************************/

int KmlReadCurrDepth(KML_READ_INFO ri)
{
  return ri->curr_depth;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlReadFail(KML_READ_INFO ri, KML_ERROR ke)                         */
/*                                                                           */
/*  Make an immediate fail exit from the current incremental read, with      */
/*  the given ke.                                                            */
/*                                                                           */
/*****************************************************************************/

void KmlReadFail(KML_READ_INFO ri, KML_ERROR ke)
{
  ri->succeeded = false;
  ri->ke = ke;
  longjmp(ri->jmp_env, 1);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "A.4.4 Writing XML files"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KML_FILE KmlMakeFile(FILE *fp, int initial_indent, int indent_step)      */
/*                                                                           */
/*****************************************************************************/

KML_FILE KmlMakeFile(FILE *fp, int initial_indent, int indent_step)
{
  KML_FILE res;
  res = (KML_FILE) malloc(sizeof(struct kml_file_rec));
  res->fp = fp;
  res->curr_indent = initial_indent;
  res->indent_step = indent_step;
  res->attribute_allowed = false;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlWrite(KML_ELT elt, KML_FILE kf)                                  */
/*                                                                           */
/*  Write elt to kf.                                                         */
/*                                                                           */
/*****************************************************************************/
static void KmlEndNoIndent(KML_FILE kf, char *label);

void KmlWrite(KML_ELT elt, KML_FILE kf)
{
  int i;  char *str;  KML_ELT child;
  assert(elt != NULL);
  KmlBegin(kf, elt->label);
  MArrayForEach(elt->attribute_names, &str, &i)
    KmlAttribute(kf, str, MArrayGet(elt->attribute_values, i));
  if( elt->text != NULL )
  {
    KmlPlainText(kf, elt->text);
    KmlEndNoIndent(kf, elt->label);
  }
  else
  {
    MArrayForEach(elt->children, &child, &i)
      KmlWrite(child, kf);
    KmlEnd(kf, elt->label);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlPrintText(FILE *fp, char *text)                                  */
/*                                                                           */
/*  Print text onto fp, taking care to handle KML predefined entities.       */
/*                                                                           */
/*****************************************************************************/

static void KmlPrintText(FILE *fp, char *text)
{
  char *p;
  for( p = text;  *p != '\0';  p++ ) switch( *p )
  {
    case '&':

      fputs("&amp;", fp);
      break;

    case '<':

      fputs("&lt;", fp);
      break;

    case '>':

      fputs("&gt;", fp);
      break;

    case '\'':

      fputs("&apos;", fp);
      break;

    case '"':

      fputs("&quot;", fp);
      break;

    default:

      putc(*p, fp);
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlBegin(KML_FILE kf, char *label)                                  */
/*                                                                           */
/*  Begin printing an element with this label.                               */
/*                                                                           */
/*****************************************************************************/

void KmlBegin(KML_FILE kf, char *label)
{
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">\n");
  fprintf(kf->fp, "%*s<%s", kf->curr_indent, "", label);
  kf->curr_indent += kf->indent_step;
  kf->attribute_allowed = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAttribute(KML_FILE kf, char *name, char *text)                   */
/*                                                                           */
/*  Add an attribute to kf.                                                  */
/*                                                                           */
/*****************************************************************************/

void KmlAttribute(KML_FILE kf, char *name, char *text)
{
  assert(kf->attribute_allowed);
  fprintf(kf->fp, " %s=\"", name);
  KmlPrintText(kf->fp, text);
  fprintf(kf->fp, "\"");
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlPlainText(KML_FILE kf, char *text)                               */
/*                                                                           */
/*  Print unformatted text, possibly with funny characters.                  */
/*                                                                           */
/*****************************************************************************/

void KmlPlainText(KML_FILE kf, char *text)
{
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  KmlPrintText(kf->fp, text);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlFmtText(KML_FILE kf, char *fmt, ...)                             */
/*                                                                           */
/*  Like printf but for kf.  The text may not have any funny characters.     */
/*                                                                           */
/*****************************************************************************/

void KmlFmtText(KML_FILE kf, char *fmt, ...)
{
  va_list args;
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEnd(KML_FILE kf, char *label)                                    */
/*                                                                           */
/*  End print of category label.                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEnd(KML_FILE kf, char *label)
{
  kf->curr_indent -= kf->indent_step;
  assert(kf->curr_indent >= 0);
  if( kf->attribute_allowed )
    fprintf(kf->fp, "/>\n");
  else
    fprintf(kf->fp, "%*s</%s>\n", kf->curr_indent, "", label);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEndNoIndent(KML_FILE kf, char *label)                            */
/*                                                                           */
/*  Like KmlEnd but with no indent.                                          */
/*                                                                           */
/*****************************************************************************/

static void KmlEndNoIndent(KML_FILE kf, char *label)
{
  kf->curr_indent -= kf->indent_step;
  if( kf->attribute_allowed )
    fprintf(kf->fp, "/>\n");
  else
    fprintf(kf->fp, "</%s>\n", label);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value)  */
/*                                                                           */
/*  Print category label with one attribute and no children or text.         */
/*                                                                           */
/*****************************************************************************/

void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value)
{
  KmlBegin(kf, label);
  KmlAttribute(kf, name, value);
  KmlEnd(kf, label);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltPlainText(KML_FILE kf, char *label, char *text)               */
/*                                                                           */
/*  Print the equivalent of                                                  */
/*                                                                           */
/*    KmlBegin(kf, label);                                                   */
/*    KmlPlainText(kf, text);                                                */
/*    KmlEnd(kf, label);                                                     */
/*                                                                           */
/*  on one line.                                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEltPlainText(KML_FILE kf, char *label, char *text)
{
  KmlBegin(kf, label);
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  KmlPrintText(kf->fp, text);
  kf->attribute_allowed = false;
  KmlEndNoIndent(kf, label);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltFmtText(KML_FILE kf, char *label, char *fmt, ...)             */
/*                                                                           */
/*  Print the equivalent of                                                  */
/*                                                                           */
/*    KmlBegin(kf, label);                                                   */
/*    KmlFmtText(kf, fmt, ...);                                              */
/*    KmlEnd(kf, label);                                                     */
/*                                                                           */
/*  on one line.                                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEltFmtText(KML_FILE kf, char *label, char *fmt, ...)
{
  va_list args;
  KmlBegin(kf, label);
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  kf->attribute_allowed = false;
  KmlEndNoIndent(kf, label);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltAttributeEltPlainText(KML_FILE kf, char *label, char *name,   */
/*    char *value, char *label2, char *text)                                 */
/*                                                                           */
/*  Equivalent to                                                            */
/*                                                                           */
/*    KmlEltAttribute(kf, label, name, value);                               */
/*    KmlEltPlainText(kf, label2, text);                                     */
/*                                                                           */
/*  on one line.                                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEltAttributeEltPlainText(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *text)
{
  KmlBegin(kf, label);
  KmlAttribute(kf, name, value);
  fprintf(kf->fp, "><%s>", label2);
  KmlPrintText(kf->fp, text);
  fprintf(kf->fp, "</%s></%s>\n", label2, label);
  kf->curr_indent -= kf->indent_step;
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltAttributeEltFmtText(KML_FILE kf, char *label, char *name,     */
/*    char *value, char *label2, char *fmt, ...)                             */
/*                                                                           */
/*  Equivalent to                                                            */
/*                                                                           */
/*    KmlEltAttribute(kf, label, name, value);                               */
/*    KmlEltFmtText(kf, label2, fmt, ...);                                   */
/*                                                                           */
/*  on one line.                                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEltAttributeEltFmtText(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *fmt, ...)
{
  va_list args;
  KmlBegin(kf, label);
  KmlAttribute(kf, name, value);
  fprintf(kf->fp, "><%s>", label2);
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  fprintf(kf->fp, "</%s></%s>\n", label2, label);
  kf->curr_indent -= kf->indent_step;
  kf->attribute_allowed = false;
}
