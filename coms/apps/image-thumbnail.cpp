//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: RP from image-thumbnail 
//
// Description: image thumbnail application.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <sstream>
#include <string>
#include <oms/Init.h>
#include <oms/Util.h>

bool getBool(char *input)
{
  std::stringstream ss(input);
  bool result;
  if (ss >> std::boolalpha >> result) return result;
  return true;
}

int main(int argc, char *argv[])
{
   if (argc < 3)
   {
      std::cout << argv[0] << " <input> <output> <size> [entryId] [<rsetDir> <histogramFile> <stretchType>]" << std::endl;
      return 1;
   }

   oms::Init::instance()->initialize(argc, argv);
   
   ossimString inputFile = ossimString(argv[1]);
   ossim_uint32 entryId = 0;
   ossimString outputFile = ossimString(argv[2]);
   ossimString writerType = "image/jpeg";
   ossim_uint32 xRes = ossimString(argv[3]).toUInt32();
   ossim_uint32 yRes = ossimString(argv[3]).toUInt32();
   ossimString histogramFile = "";
   ossimString supplementalDir = "";
   ossimString stretchType = "linear_auto_min_max";

   if (argc == 4) entryId = ossimString(argv[4]).toUInt32();
   // RP - allow pass of an alternative histogram location and stretch type
   if ( argc == 6 )
   {
     supplementalDir = ossimString(argv[4]);
     histogramFile = ossimString(argv[5]);
     stretchType = ossimString(argv[6]);
   }
   if ( argc == 6 )
   {
     entryId = ossimString(argv[4]).toUInt32();
     supplementalDir = ossimString(argv[5]);
     histogramFile = ossimString(argv[6]);
     stretchType = ossimString(argv[7]);
   }
   // RP - Added support to Util.cpp for post renderer histogram for target RSET
   if (histogramFile.contains("none")) histogramFile = "";
   if (supplementalDir.contains("none")) supplementalDir = "";

   bool keepAspectFlag = true;
   // Need to add supplementalDir 
   bool status = oms::Util::writeImageSpaceThumbnail( inputFile, entryId, outputFile, writerType, xRes, yRes, histogramFile, stretchType, keepAspectFlag);
   if ( !status )
   {
     std::cerr << "Error: cannot create thumbnail using args:"
       << "\tinputFile: " << inputFile
       << "\tentryId: " << entryId
       << "\toutputFile: " << outputFile
       << "\twriterType: " << writerType
       << "\txRes: " << xRes
       << "\tyRes: " << yRes
       << "\tsupplementalDir: " << supplementalDir
       << "\thistogramFile: " << histogramFile
       << "\tstretchType: " << stretchType
       << "\tkeepAspectFlag: " << keepAspectFlag << std::endl;
     return -1;
   }
   
   return 0;
}

