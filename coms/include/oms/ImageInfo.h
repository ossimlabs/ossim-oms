//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  
//
// Description:  Simple container class to encapsulate image info.
//
//----------------------------------------------------------------------------
// $Id: ImageInfo.h 12991 2008-06-04 19:14:59Z gpotts $
#ifndef imageInfo_HEADER
#define imageInfo_HEADER 1

#include <vector>
#include <string>
#include <oms/Constants.h>
#include <oms/Object.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordlist.h>

class ossimImageHandler;
namespace oms
{
   class  ImageInfoPrivateData;
   class OMSDLL ImageInfo : public oms::Object
   {
   public:
      /** default constructor */
      ImageInfo();
   
      /** destructor */
      virtual ~ImageInfo();


      /**
       * @brief Open method.
       * @param file File to open.
       * @return true on success, false on error.
       */
      bool open(const std::string& file, bool failIfNoGeometryFlag=true);
      bool setHandler(ossimImageHandler* handler, bool failIfNoGeometryFlag=true);
      void close();
      bool isImagery()const;
      std::string getInfo()const;
      bool getKwl(ossimKeywordlist& kwl)const;

      void setJson(bool json){this->json = json;}
      void setPretty(bool pretty){this->pretty = pretty;}
      void setMulti(bool multi){this->multi = multi;}
      void setOmardb(bool omardb){this->omardb = omardb;}
      void setShowsummary(bool showsummary){this->showsummary = showsummary;}
      void setShowdetails(bool showdetails){this->showdetails = showdetails;}


   protected:
      bool json;
      bool pretty;
      bool multi;
      bool omardb;
      bool showsummary;
      bool showdetails;
      
   private:
      std::string convertIdatimToXmlDate(const std::string& idatim)const;
      std::string convertAcqDateToXmlDate(const std::string& idatim)const;
      void appendRasterEntry(std::string& outputString, const ossimKeywordlist& kwl,
                             ossim_uint32 entry=0, ossimString root="")const;
      void writeJsonKVP(std::string& outputString, ossimString key, ossimString value, bool comma, ossim_uint32 tabcount, ossimString path="")const;
      void appendRasterEntries(std::string& outputString, const ossimKeywordlist& kwl,
                               ossimString root="")const;
      void appendAssociatedRasterEntryFileObjects(std::string& outputString,
                                                  const std::string& indentation,
                                                  const std::string& separator)const;
      void appendBitDepthAndDataType(std::string& outputString,
                                     ossim_uint32 indent=0, ossimString path="")const;
      void appendGeometryInformation(std::string& outputString,
                                     ossim_uint32 indent=0, ossimString path="")const;
      void appendRasterDataSetMetadata(std::string& outputString,
                                       const std::string& indentation,
                                       const std::string& separator)const;
      void appendRasterEntryDateTime(std::string& outputString,
                                     ossim_uint32 indent=0, ossimString path="")const;
      void appendMetadataDetails(std::string& outputString, const ossimKeywordlist& kwl)const;

      void appendNormalizedMetadata(std::string& outputString, const ossimKeywordlist& kwl,
                                     ossim_uint32 indent=0, ossimString path="")const;

      ImageInfoPrivateData* thePrivateData;
    };

} // end of namespace oms

#endif 
