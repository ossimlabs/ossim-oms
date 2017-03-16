//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author: RP - borrowed from DataInfo for Video specific JSON dump capability 
//
// Description:  Simple container class to encapsulate video info.
//
//----------------------------------------------------------------------------
// $Id: VideoInfo.h 12991 2008-06-04 19:14:59Z gpotts $
#ifndef videoInfo_HEADER
#define videoInfo_HEADER 1

#include <vector>
#include <string>
#include <oms/Constants.h>
#include <oms/Object.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimDate.h>
#include <ossimPredator/ossimVMTITrack.h>
#include <ossimPredator/ossimPredatorKlvTable.h>
#include <ossim/imaging/ossimImageGeometry.h>


namespace oms
{
   class  VideoInfoPrivateData;
   class OMSDLL VideoInfo : public oms::Object
   {
   public:
      /** default constructor */
      VideoInfo();
   
      /** destructor */
      virtual ~VideoInfo();

      typedef std::map<ossim_uint32, ossimRefPtr<ossimVMTITrack> > trackMap;

      /**
       * @brief Open method.
       * @param file File to open.
       * @return true on success, false on error.
       */
      bool open(const std::string& file, bool failIfNoGeometryFlag=true);
      void close();
      std::string getInfo()const;
      bool getKwl(ossimKeywordlist& kwl)const;

      void setJson(bool json){this->json = json;}
      void setPretty(bool pretty){this->pretty = pretty;}
      void setMulti(bool multi){this->multi = multi;}
      void setOmardb(bool omardb){this->omardb = omardb;}
      void setShowsummary(bool showsummary){this->showsummary = showsummary;}
      void setShowdetails(bool showdetails){this->showdetails = showdetails;}
      void setCoredata(bool coredata){this->coredata = coredata;}
      void setSkipSeconds(ossim_float64 skip){this->skipSeconds = skip;}


   protected:
      bool json;
      bool pretty;
      bool multi;
      bool omardb;
      bool showsummary;
      bool showdetails;
      bool coredata;
      ossim_float64 skipSeconds;
      
   private:
      void writeJsonKVP(std::string& outputString, ossimString key, ossimString value, bool comma, ossim_uint32 tabcount, bool quote=true, ossimString path="")const;
      void appendDateRange(std::string& outputString, const ossimDate& startDate, const ossimDate& endDate, bool comma, ossim_uint32 tabcount)const;
      bool updateTrackInfo(ossimRefPtr<ossimVMTITrack> track, ossimRefPtr<ossimPredatorKlvTable> klvTable, ossimRefPtr<ossimImageGeometry> geom, ossim_uint32 index=0)const;

      void appendVideoDataSetMetadata(std::string& outputString,
                                       const std::string& indentation,
                                       const std::string& separator)const;
      
      VideoInfoPrivateData* thePrivateData;
    };

} // end of namespace oms

#endif /* End of #ifndef omsVideoInfo_HEADER */
