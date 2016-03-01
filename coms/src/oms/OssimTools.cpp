#include <oms/OssimTools.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/util/ossimHlzUtil.h>
#include <ossim/projection/ossimMapProjection.h>
#include <cstdlib>

using namespace oms;
using namespace std;

bool OssimTools::m_locked = false;

OssimTools::OssimTools(string name)
:  m_chipProcUtil(0)
{
   m_chipProcUtil = (ossimChipProcUtil*) ossimUtilityRegistry::instance()->createUtility(name);
   if (m_chipProcUtil == 0)
      cerr<<"OssimTools() Bad opeation requested: <"<<name<<">. Ignoring."<<endl;
}

OssimTools::~OssimTools()
{
	delete m_chipProcUtil;
}

bool OssimTools::initialize(const map<string, string>& params)
{
   if (m_chipProcUtil == 0)
      return false;

   // Accept map from service and translate to KWL expected by native code:
   ossimKeywordlist kwl;

   try
   {
      if (!m_locked)
      {
         m_locked = true;
         ossimKeywordlist kwl (params);
         cout<<"\nOssimTools::initialize() -- KWL:\n"<<kwl<<endl;//TODO:remove debug
         m_chipProcUtil->initialize(kwl);
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

bool OssimTools::getChip(ossim_int8* data, const map<string,string>& hints)
{
   if ((m_chipProcUtil == 0) || (data == 0))
      return false;

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
   cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
   cerr<<"\nOssimTools: map_bbox"<<map_bbox<<endl;//TODO:remove debug
   cerr<<"\nOssimTools: gsd"<<gsd<<endl;//TODO:remove debug
   try
   {
      ossimRefPtr<ossimImageData> chip = m_chipProcUtil->getChip(map_bbox, gsd);
      cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
      if ( chip.valid() )
      {
         cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
         ossimDataObjectStatus status = chip->getDataObjectStatus();
         ossimIrect rect = chip->getImageRectangle();
         if ( !rect.hasNans() && (status != (int) OSSIM_NULL))
         {
            cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
            //chip->computeAlphaChannel();
            cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
            chip->unloadTile((void*)data, rect, OSSIM_BIP);
            cerr<<"\nOssimTools:"<<__LINE__<<endl;//TODO:remove debug
            chip->write("/tmp/getChip.ras");//TODO:remove debug
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
