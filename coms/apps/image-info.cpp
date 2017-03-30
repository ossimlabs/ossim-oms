//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: RP from oms-image-info 
//
// Description: JSON image info application.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <sstream>
#include <string>
#include <oms/Init.h>
#include <oms/ImageInfo.h>

bool getBool(char *input)
{
  std::stringstream ss(input);
  bool result;
  if (ss >> std::boolalpha >> result) return result;
  return true;
}

int main(int argc, char *argv[])
{
   oms::Init::instance()->initialize(argc, argv);

   if (argc < 2)
   {
      std::cout << argv[0] << " <file>\nWill dump info for file." << std::endl;
      return 1;
   }

   oms::ImageInfo info;
   if ( info.open(std::string(argv[1])) )
   {
	std::string s;
	info.setJson(true);
	info.setShowdetails(false);
	info.setMulti(true);
	info.setOmardb(true);
	s = info.getInfo();

        std::cout << s.c_str() << std::endl;
   }
   else
   {
      std::cerr << "Could not open: " << argv[1] << std::endl;
      return 1;
   }
   
   return 0;
}

