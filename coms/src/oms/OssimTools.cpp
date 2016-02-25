#include <oms/OssimTools.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/util/ossimHlzUtil.h>
#include <cstdlib>

using namespace oms;
using namespace std;

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
      ossimKeywordlist kwl (params);
      m_chipProcUtil->initialize(kwl);
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
   int status = OSSIM_STATUS_UNKNOWN;
   if ((m_chipProcUtil == 0) || (data == 0))
      return OSSIM_NULL;

   // Expect only geographic bounding rect in hints.
   double min_lat=ossim::nan(), max_lat=ossim::nan(), min_lon=ossim::nan(), max_lon=ossim::nan();
   map<string,string>::const_iterator value;
   value = hints.find("min_lat");
   if (value != hints.end())
      min_lat = atof(value->second.c_str());
   value = hints.find("max_lat");
   if (value != hints.end())
      max_lat = atof(value->second.c_str());
   value = hints.find("min_lon");
   if (value != hints.end())
      min_lon = atof(value->second.c_str());
   value = hints.find("max_lon");
   if (value != hints.end())
      max_lon = atof(value->second.c_str());

   ossimGrect bbox (max_lat, min_lon, min_lat, max_lat);

   // Need the ossimImageData buffer returned from native call as a char buffer:
   try
   {
      ossimRefPtr<ossimImageData> chip = m_chipProcUtil->getChip(bbox);
      if ( chip.valid() )
      {
         status = chip->getDataObjectStatus();
         ossimIrect rect = chip->getImageRectangle();
         if ( !rect.hasNans() && (status != (int) OSSIM_NULL))
            chip->unloadTile( (void*)data, rect, OSSIM_BIP );
      }
   }
   catch (ossimException& e)
   {
      cerr<<"Caught OSSIM exception in OssimTools::getChip():\n"<<e.what()<<endl;
      return false;
   }
   catch ( ... )
   {
      cerr<<"Caught exception in OssimTools::getChip(). Operation aborted."<<endl;
   }

   return status;
}
