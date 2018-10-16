#include <oms/CoordinateUtility.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/projection/ossimMgrs.h>
#include <ossim/base/ossimDatumFactoryRegistry.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimEllipsoidFactory.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimDms.h>
#include <ossim/base/ossimDatum.h>
#include <iostream>
#include <mutex>

class oms::CoordinateUtility::PrivateData
{
public:
   PrivateData()
   {
      
   }
   void setErrorFromMgrs(int error)
   {
      switch(error)
      {
         case OSSIM_MGRS_NO_ERROR:
         {
            theLastError = "";
            break;
         }
         case OSSIM_MGRS_LAT_ERROR:
         {
            theLastError = "Lat Error: Needs to be between -90 and 90";
            break;
         }
         case OSSIM_MGRS_LON_ERROR:
         {
            theLastError = "Lon Error: Needs to be bewteen -180 and 360";
            break;
         }
         case OSSIM_MGRS_STRING_ERROR:
         {
            theLastError = "Wrong format for MGRS string";
            break;
         }
         case OSSIM_MGRS_PRECISION_ERROR:
         {
            theLastError = "Precision paramter needs to be between 0 and 5";
           break;
         }
         case OSSIM_MGRS_A_ERROR:
         {
            theLastError = "Invalid major axis";
            break;
         }
         case OSSIM_MGRS_INV_F_ERROR:
         {
            theLastError = "Invalid inverse flattening.  Needs to be bewteen 250 and 350";
            break; 
         }
         case OSSIM_MGRS_EASTING_ERROR:
         {
            theLastError = "Invalid easting error";
            break;
         }
         case OSSIM_MGRS_NORTHING_ERROR:
         {
            theLastError = "Invalid northing error";
           break;
         }
         case OSSIM_MGRS_ZONE_ERROR:
         {
            theLastError = "Invalid zone error";
           break;
         }
         case OSSIM_MGRS_HEMISPHERE_ERROR:
         {
            theLastError = "Invalid hemisphere error";
            break;
         }
         default:
         {
            theLastError = "";
            break;
         }
      }
   }
   void clearError()
   {
      std::lock_guard<std::mutex> lock(theMutex);
      theLastError = "";
   }
   void setDatum(const std::string& datumCode)
   {
     m_datum = ossimDatumFactoryRegistry::instance()->create(ossimString(datumCode));
     if(!m_datum)
     {
       m_datum = ossimDatumFactory::instance()->wgs84();
     }
   }
   std::mutex theMutex;
   std::string theLastError;
   const ossimDatum* m_datum;
};

oms::CoordinateUtility::CoordinateUtility()
:theData(new PrivateData)
{
  theData->m_datum = ossimDatumFactory::instance()->wgs84();
}

oms::CoordinateUtility::~CoordinateUtility()
{
   if(theData)
   {
      delete theData;
      theData = 0;
   }
}

std::string oms::CoordinateUtility::lastError()const
{
   return theData->theLastError;
}

std::string oms::CoordinateUtility::convertLatLonToMgrs(double lat,
                                                        double lon,
                                                        int precision)const
{
   theData->clearError();
   std::string result;
   std::lock_guard<std::mutex> lock(theData->theMutex);
   char mgrsResult[1024];
   const ossimEllipsoid* wgs84Ellipsoid = ossimEllipsoidFactory::instance()->wgs84();
   Set_OSSIM_MGRS_Parameters(wgs84Ellipsoid->a(),
                             wgs84Ellipsoid->flattening(),
                             "WE");
   int error = Convert_Geodetic_To_OSSIM_MGRS(ossim::degreesToRadians(lat), 
                                               ossim::degreesToRadians(lon), 
                                               precision, 
                                               mgrsResult);
   theData->setErrorFromMgrs(error);
   if(theData->theLastError.empty())
   {
      result = std::string(mgrsResult);
   }
  return result;
}

ossimGpt oms::CoordinateUtility::convertMgrsToLatLon(const std::string& mgrsString)const
{
   theData->clearError();
   std::string result;
   std::lock_guard<std::mutex> lock(theData->theMutex);
   const ossimEllipsoid* wgs84Ellipsoid = ossimEllipsoidFactory::instance()->wgs84();
   Set_OSSIM_MGRS_Parameters(wgs84Ellipsoid->a(),
                             wgs84Ellipsoid->flattening(),
                             "WE");
   double lat=0.0, lon=0.0;
   int error = Convert_OSSIM_MGRS_To_Geodetic(mgrsString.c_str(),
                                              &lat, 
                                              &lon);
   theData->setErrorFromMgrs(error);
   return ossimGpt(ossim::radiansToDegrees(lat),
                   ossim::radiansToDegrees(lon));
}

double oms::CoordinateUtility::dmsToDegrees(const std::string& dmsString)const
{
   ossimDms dms;
   double result = 0.0;
   theData->clearError();
   if(dms.setDegrees(dmsString))
   {
      result = dms.getDegrees();
   }
   else
   {
      theData->theLastError = "Invalid dms string: " +dmsString;
   }
   
   return result;
}

std::string oms::CoordinateUtility::degreesToDms(double value, const std::string& format, bool latFlag)const
{
   theData->clearError();
   ossimDms dms(value, latFlag);
   
   std::string result = "";
   result = dms.toString(format).string();
   return result;
}

bool oms::CoordinateUtility::latLonHeightToEcef(double latLonHeight[],
                                                double ecef[]) const
{
  bool result = false;

  if(theData->m_datum)
  {
    theData->m_datum->ellipsoid()->latLonHeightToXYZ(latLonHeight[0], latLonHeight[1], latLonHeight[2],
                                                     ecef[0], ecef[1], ecef[2]);
    result = true;
  }

  return result;
}

bool oms::CoordinateUtility::ecefToLatLonHeight(double ecef[],
                                                double latLonHeight[]) const
{
  bool result = false;

  if (theData->m_datum)
  {
    theData->m_datum->ellipsoid()->latLonHeightToXYZ(latLonHeight[0], latLonHeight[1], latLonHeight[2],
                                                     ecef[0], ecef[1], ecef[2]);
    result = true;
  }

  return result;
}

void oms::CoordinateUtility::setDatum(const std::string &datumCode)
{
  theData->setDatum(datumCode);
}

std::string oms::CoordinateUtility::getDatumCode() const
{
  std::string result;

  result = theData->m_datum->code().c_str();

  return result;
}
