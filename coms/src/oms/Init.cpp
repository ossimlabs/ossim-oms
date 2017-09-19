//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Singleton class for queries about things like writer strings
// that should stay the same throughout the session.
//
// $Id: Init.cpp 22314 2013-07-17 19:50:48Z dburken $
//----------------------------------------------------------------------------

#include <iostream>

#include <oms/Init.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimEnvironmentUtility.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <mutex>

oms::Init* oms::Init::theInstance = 0;
static std::mutex initMutex;
oms::Init::~Init()
{
}

oms::Init* oms::Init::instance()
{
   static std::mutex m;
   std::lock_guard<std::mutex> lock(m);

   if (!theInstance)
   {
      theInstance = new oms::Init;
   }
   return theInstance;
}

int oms::Init::initialize(int argc, char* argv[])
{
   std::lock_guard<std::mutex> lock(initMutex);

   int result = argc;
   if(!theInitCalledFlag)
   {
      theInitCalledFlag = true;
      for (int i=0; i<argc; ++i)
      {
         // std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
      }
      //ossimSetLogFilename(ossimFilename("/tmp/oms.log"));
      
      ossimArgumentParser argumentParser(&argc, argv);
      ossimInit::instance()->addOptions(argumentParser);
      ossimInit::instance()->initialize(argumentParser);
      result = argumentParser.argc();
   }
   return result;
}

void oms::Init::initialize()
{
   std::lock_guard<std::mutex> lock(initMutex);
   if(!theInitCalledFlag)
   {
      theInitCalledFlag = true;
      ossimInit::instance()->initialize();
   }
}

oms::Init::Init()
:theInitCalledFlag(false)
{
}

oms::Init::Init(const oms::Init& /* obj */)
:theInitCalledFlag(false)
{
}

const oms::Init& oms::Init::operator=(const oms::Init& /* obj */)
{
   return *this;
}

