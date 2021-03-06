#ifndef omsCoordinateUtility_HEADER
#define omsCoordinateUtility_HEADER
#include <oms/Constants.h>
#include <ossim/base/ossimGpt.h>
#include <string>
namespace oms 
{
   class OMSDLL CoordinateUtility
   {
   public:
      CoordinateUtility();
      ~CoordinateUtility();
      std::string lastError()const;
      /**
       * Max precision is 5.
       */
      std::string convertLatLonToMgrs(double lat,
                                      double lon,
                                      int precision)const;
      ossimGpt convertMgrsToLatLon(const std::string& mgrsString)const;
      
      /**
       * setDegrees(char*).  Will allow you to set the internal
       * paramters through a string.  examples:
       *
       * "45 5 6"  will assume degrees minutes seconds
       * "45"      will assume degrees.
       * "-45"
       * "45W"
       *
       * @return true on success, false if unhandled characters are found.
       */
      double dmsToDegrees(const std::string& dmsString)const;
      
      /**
       *  You can specify a number of different formats.
       *  special characters:
       *
       *      @    is replaced with a degree character.
       *      '    minutes character
       *      "    seconds character
       *      d    replaced with degree portion
       *      m    replaced with minute portion
       *      s    replaced with second portion
       *      c or C replaces with N/S or E/W
       *
       *  examples for format string
       *
       *
       */
      std::string degreesToDms(double value, const std::string& format, bool latFlag)const;
      
      /**
       * @param datumCode Datum code to use. The default is a wgs84 datum code
       */
      void setDatum(const std::string& datumCode="WGE");
      std::string getDatumCode()const;

      /**
       * Expects a 3 coordinateArray of lat lon height values and expects to
       * write to a 3 coordinate array ecef using the current datum for the ellipsoidal 
       * definition
       */
      bool latLonHeightToEcef(double latLonHeight[],
                              double ecef[]) const;
      /**
       * Expects a 3 coordinateArray of lat lon height values and expects to
       * write to a 3 coordinate array ecef using the current datum for the ellipsoidal 
       * definition
       */
      bool ecefToLatLonHeight(double ecef[],
                              double latLonHeight[]) const;

    protected:
      class PrivateData;
      PrivateData *theData;
   };
}
#endif
