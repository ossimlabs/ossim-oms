#include <oms/OssimTools.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/util/ossimHlzUtil.h>
#include <cstdlib>

using namespace oms;
using namespace std;

OssimTools::OssimTools(string name)
:  m_chipProcUtil(0)
{
   if ((name.compare("hlz") == 0) || (name.compare("viewshed") == 0))
      m_chipProcUtil = (ossimChipProcUtil*) ossimUtilityRegistry::instance()->createUtility(name);
}

OssimTools::~OssimTools()
{
	delete m_chipProcUtil;
}

bool OssimTools::initialize(map<string, string> params)
{
   if (m_chipProcUtil == 0)
      return false;

   try
   {
      ossimKeywordlist kwl (params);
      m_chipProcUtil->initialize(kwl);
   }
   catch (...)
   {
      return false;
   }

	return true;
}

bool OssimTools::getChip(char* data, map<string,string> hints)
{
   int status = OSSIM_STATUS_UNKNOWN;
   if ((m_chipProcUtil == 0) || (data == 0))
      return OSSIM_NULL;

   // Expect only geographic bounding rect in hints.
   double min_lat=ossim::nan(), max_lat=ossim::nan(), min_lon=ossim::nan(), max_lon=ossim::nan();
   map<string,string>::iterator value;
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
   if (bbox.hasNans())
      return OSSIM_NULL;

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
   catch ( ... )
   {
   }

   return status;
}
