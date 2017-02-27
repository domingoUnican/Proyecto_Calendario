
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
/*  FILE:         kml.h                                                      */
/*  MODULE:       XML reading and writing                                    */
/*                                                                           */
/*****************************************************************************/
#ifndef KML_HEADER_FILE
#define KML_HEADER_FILE
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

/* type declarations */
typedef struct kml_file_rec *KML_FILE;		/* an XML direct write file */
typedef struct kml_elt_rec *KML_ELT;		/* an XML element           */
typedef struct kml_error_rec *KML_ERROR;	/* an XML error record      */
typedef struct kml_read_info_rec *KML_READ_INFO; /* read info               */
typedef void (*KML_ELT_FN)(KML_ELT elt, KML_READ_INFO ri);

/* A.4.1 Representing XML in memory */
extern int KmlLineNum(KML_ELT elt);
extern int KmlColNum(KML_ELT elt);
extern char *KmlLabel(KML_ELT elt);
extern KML_ELT KmlParent(KML_ELT elt);
extern int KmlAttributeCount(KML_ELT elt);
extern char *KmlAttributeName(KML_ELT elt, int index);
extern char *KmlAttributeValue(KML_ELT elt, int index);
extern int KmlChildCount(KML_ELT elt);
extern KML_ELT KmlChild(KML_ELT elt, int index);
extern bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt);
extern char *KmlText(KML_ELT elt);

extern KML_ELT KmlMakeElt(int line_num, int col_num, char *label);
extern void KmlAddAttribute(KML_ELT elt, char *name, char *value);
extern void KmlAddChild(KML_ELT elt, KML_ELT child);
extern void KmlDeleteChild(KML_ELT elt, KML_ELT child);
extern void KmlAddText(KML_ELT elt, char *text);

extern void KmlFree(KML_ELT elt, bool free_attribute_values, bool free_text);

extern char *KmlExtractLabel(KML_ELT elt);
extern char *KmlExtractAttributeName(KML_ELT elt, int index);
extern char *KmlExtractAttributeValue(KML_ELT elt, int index);
extern char *KmlExtractText(KML_ELT elt);

/* A.4.2 Error handing and format checking */
extern int KmlErrorLineNum(KML_ERROR ke);
extern int KmlErrorColNum(KML_ERROR ke);
extern char *KmlErrorString(KML_ERROR ke);
extern KML_ERROR KmlErrorMake(int line_num, int col_num, char *fmt, ...);
extern KML_ERROR KmlVErrorMake(int line_num, int col_num, char *fmt,
  va_list ap);
extern bool KmlError(KML_ERROR *ke, int line_num, int col_num, char *fmt, ...);
extern bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke);

/* A.4.3 Reading XML files */
extern bool KmlReadFile(FILE *fp, KML_ELT *res, KML_ERROR *ke,
  char *end_label, char **leftover, int *leftover_len, FILE *echo_fp);
extern bool KmlReadString(char *str, KML_ELT *res, KML_ERROR *ke);
extern bool KmlReadFileIncremental(FILE *fp, KML_ELT *res, KML_ERROR *ke,
  char *end_label, char **leftover, int *leftover_len, FILE *echo_fp,
  KML_ELT_FN elt_fn, void *impl, int max_depth);

extern void *KmlReadImpl(KML_READ_INFO ri);
extern int KmlReadMaxDepth(KML_READ_INFO ri);
extern int KmlReadCurrDepth(KML_READ_INFO ri);
extern void KmlReadFail(KML_READ_INFO ri, KML_ERROR ke);

/* A.4.4 Writing XML files */
extern KML_FILE KmlMakeFile(FILE *fp, int initial_indent, int indent_step);
extern void KmlWrite(KML_ELT elt, KML_FILE kf);

extern void KmlBegin(KML_FILE kf, char *label);
extern void KmlAttribute(KML_FILE kf, char *name, char *text);
extern void KmlPlainText(KML_FILE kf, char *text);
extern void KmlFmtText(KML_FILE kf, char *fmt, ...);
extern void KmlEnd(KML_FILE kf, char *label);

extern void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value);
extern void KmlEltPlainText(KML_FILE kf, char *label, char *text);
extern void KmlEltFmtText(KML_FILE kf, char *label, char *fmt, ...);

/* these not documented, although doc mentions that some such exist */
extern void KmlEltAttributeEltPlainText(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *text);
extern void KmlEltAttributeEltFmtText(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *fmt, ...);

#endif
