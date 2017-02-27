
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
/*  FILE:         khe_soln_group_metadata.c                                  */
/*  DESCRIPTION:  Solution group metadata                                    */
/*                                                                           */
/*****************************************************************************/
#include <stdarg.h>
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP_METADATA - solution group metadata                        */
/*                                                                           */
/*****************************************************************************/

struct khe_soln_group_metadata_rec {
  char			*contributor;		/* contributor               */
  char			*date;			/* date                      */
  char			*description;		/* description               */
  char			*publication;		/* publication (optional)    */
  char			*remarks;		/* remarks (optional)        */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP_METADATA KheSolnGroupMetaDataMake(char *contributor,      */
/*    char *date, char *description, char *publication, char *remarks)       */
/*                                                                           */
/*  Make solution group metadata with these attributes.                      */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP_METADATA KheSolnGroupMetaDataMake(char *contributor,
  char *date, char *description, char *publication, char *remarks)
{
  KHE_SOLN_GROUP_METADATA res;
  MMake(res);
  res->contributor = contributor;
  res->date = date;
  res->description = description;
  res->publication = publication;
  res->remarks = remarks;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupMetaDataContributor(KHE_SOLN_GROUP_METADATA md)        */
/*                                                                           */
/*  Return the Contributor attribute of md.                                  */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupMetaDataContributor(KHE_SOLN_GROUP_METADATA md)
{
  return md->contributor;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupMetaDataDate(KHE_SOLN_GROUP_METADATA md)               */
/*                                                                           */
/*  Return the Date attribute of md.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupMetaDataDate(KHE_SOLN_GROUP_METADATA md)
{
  return md->date;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupMetaDataDescription(KHE_SOLN_GROUP_METADATA md)        */
/*                                                                           */
/*  Return the Description attribute of md.                                  */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupMetaDataDescription(KHE_SOLN_GROUP_METADATA md)
{
  return md->description;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupMetaDataPublication(KHE_SOLN_GROUP_METADATA md)        */
/*                                                                           */
/*  Return the optional Publication attribute of md.                         */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupMetaDataPublication(KHE_SOLN_GROUP_METADATA md)
{
  return md->publication;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupMetaDataRemarks(KHE_SOLN_GROUP_METADATA md)            */
/*                                                                           */
/*  Return the optional Remarks attribute of md.                             */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupMetaDataRemarks(KHE_SOLN_GROUP_METADATA md)
{
  return md->remarks;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnGroupMetaDataMakeFromKml(KML_ELT md_elt,                     */
/*    KHE_SOLN_GROUP soln_group, KML_ERROR *ke)                              */
/*                                                                           */
/*  Make a solution group metadata object based on md_elt and add it to      */
/*  soln_group.                                                              */
/*                                                                           */
/*****************************************************************************/

bool KheSolnGroupMetaDataMakeFromKml(KML_ELT md_elt,
  KHE_SOLN_GROUP soln_group, KML_ERROR *ke)
{
  KHE_SOLN_GROUP_METADATA res;  KML_ELT e;
  char *contributor, *date, *description, *publication, *remarks;
  if( !KmlCheck(md_elt,
      ": $Contributor $Date $Description +$Publication +$Remarks", ke) )
    return false;
  contributor = KmlExtractText(KmlChild(md_elt, 0));
  date = KmlExtractText(KmlChild(md_elt, 1));
  description = KmlExtractText(KmlChild(md_elt, 2));
  if( KmlContainsChild(md_elt, "Publication", &e) )
    publication = KmlExtractText(e);
  else
    publication = NULL;
  if( KmlContainsChild(md_elt, "Remarks", &e) )
    remarks = KmlExtractText(e);
  else
    remarks = NULL;
  res = KheSolnGroupMetaDataMake(contributor, date, description,
    publication, remarks);
  KheSolnGroupSetMetaData(soln_group, res);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupMetaDataWrite(KHE_SOLN_GROUP_METADATA md, KML_FILE kf)  */
/*                                                                           */
/*  Write instance metadata to xml.                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupMetaDataWrite(KHE_SOLN_GROUP_METADATA md, KML_FILE kf)
{
  KmlBegin(kf, "MetaData");

  /* contributor */
  MAssert(md->contributor != NULL,
    "KheArchiveWrite: Contributor missing in solution group MetaData");
  KmlEltPlainText(kf, "Contributor", md->contributor);

  /* date */
  MAssert(md->date != NULL,
    "KheArchiveWrite: Date missing in solution group MetaData");
  KmlEltPlainText(kf, "Date", md->date);

  /* description */
  MAssert(md->description != NULL,
    "KheArchiveWrite: Description missing in solution group MetaData");
  KmlEltPlainText(kf, "Description", md->description);

  /* publication (optional) */
  if( md->publication != NULL && strlen(md->publication) > 0 )
    KmlEltPlainText(kf, "Publication", md->publication);

  /* remarks (optional) */
  if( md->remarks != NULL && strlen(md->remarks) > 0 )
    KmlEltPlainText(kf, "Remarks", md->remarks);

  /* that's all */
  KmlEnd(kf, "MetaData");
}
