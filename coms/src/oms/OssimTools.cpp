#include <oms/OssimTools.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/util/ossimHlzTool.h>
#include <ossim/util/ossimToolRegistry.h>
#include <ossim/util/ossimViewshedTool.h>
#include <cstdlib>

using namespace oms;
using namespace std;

bool OssimTools::m_locked = false;

OssimTools::OssimTools(string name)
:  m_utility(0)
{
   m_utility = ossimToolRegistry::instance()->createTool(name);
   if (m_utility == 0)
      cerr<<"OssimTools() Bad opeation requested: <"<<name<<">. Ignoring."<<endl;
}

OssimTools::~OssimTools()
{
	delete m_utility;
}

bool OssimTools::initialize(const map<string, string>& params)
{
   if (m_utility == 0)
      return false;

   // Accept map from service and translate to KWL expected by native code:
   ossimKeywordlist kwl;

   try
   {
      if (!m_locked)
      {
         m_locked = true;
         ossimKeywordlist kwl (params);
         //cout<<"\nOssimTools::initialize() -- KWL:\n"<<kwl<<endl;//TODO:remove debug
         m_utility->initialize(kwl);
         m_locked = false;

      }
   }
   catch (ossimException& e)
   {
      cerr<<"Caught OSSIM exception in OssimTools::initialize():\n"<<e.what()<<endl;
      return false;
   }
   catch (...)
   {
      cerr<<"Caught unknown exception in OssimTools::initialize()."<<endl;
      return false;
   }

	return true;
}

bool OssimTools::execute(char* outstreambuf)
{
   if (m_utility == 0)
      return false;

   ostringstream outputStream;
   m_utility->setOutputStream(&outputStream);

   bool status = m_utility->execute();

   // Copy the output stream to string array:
   size_t bufsize = outputStream.width();
   outstreambuf = new char[bufsize+1];

   memcpy(outstreambuf, outputStream.str().c_str(), bufsize);

   return status;
}

bool OssimTools::getChip(ossim_int8* data, const map<string,string>& hints)
{
   if ((m_utility == 0) || !m_utility->isChipProcessor() || (data == 0))
      return false;

   ossimChipProcTool* chipper = (ossimChipProcTool*) m_utility;

   // Expect only geographic bounding rect in hints.
   double min_x, max_x, min_y, max_y;
   int width=0, height=0;
   map<string,string>::const_iterator value;
   do
   {
      value = hints.find("min_x");
      if (value == hints.end())
         break;
      min_x = atof(value->second.c_str());

      value = hints.find("max_x");
      if (value == hints.end())
         break;
      max_x = atof(value->second.c_str());

      value = hints.find("min_y");
      if (value == hints.end())
         break;
      min_y = atof(value->second.c_str());

      value = hints.find("max_y");
      if (value == hints.end())
         break;
      max_y = atof(value->second.c_str());

      value = hints.find("width");
      if (value == hints.end())
         break;
      width = atoi(value->second.c_str());

      value = hints.find("height");
      if (value == hints.end())
         break;
      height = atoi(value->second.c_str());

   } while (0);

   if (width == 0)
   {
      cerr<<"OssimTools ["<<__LINE__<<"] -- Bad parse of hints map."<<endl;
      return false;
   }

   ossimDrect map_bbox (min_x, max_y, max_x, min_y);
   double gsd_x = fabs(max_x - min_x)/(double) (width-1);
   double gsd_y = fabs(max_y - min_y)/(double) (height-1);
   ossimDpt gsd (gsd_x, gsd_y);

   // Need the ossimImageData buffer returned from native call as a char buffer:
   try
   {
      ossimRefPtr<ossimImageData> chip = chipper->getChip(map_bbox, gsd);
      if ( chip.valid() )
      {
         ossimDataObjectStatus status = chip->getDataObjectStatus();
         ossimIrect rect = chip->getImageRectangle();
         if ( !rect.hasNans() && (status != (int) OSSIM_NULL))
         {
            chip->unloadTile((void*)data, rect, OSSIM_BIP);
         }
         else
            throw ossimException("Bad chip returned from native getChip call.");
      }
      else
         throw ossimException("Null chip returned from native getChip call.");
   }
   catch (ossimException& e)
   {
      cerr<<"Caught OSSIM exception in OssimTools::getChip():\n"<<e.what()<<endl;
      return false;
   }
   catch ( ... )
   {
      cerr<<"Caught exception in OssimTools::getChip(). Operation aborted."<<endl;
      return false;
   }

   return true;
}
