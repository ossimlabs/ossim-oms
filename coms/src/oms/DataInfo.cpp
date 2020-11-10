#include <oms/DataInfo.h>
#include <oms/Util.h>
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRegExp.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimDate.h>
#include <ossim/support_data/ossimInfoFactoryRegistry.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimXmlString.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/base/KwlNodeXmlFormatter.h>
#include <ossim/base/ossimScalarTypeLut.h>


#ifdef OSSIM_VIDEO_ENABLED
#  include <ossimPredator/ossimPredatorVideo.h>
#  include <ossimPredator/ossimPredatorKlvTable.h>
#endif

#include <sstream>
#include <memory>
#include <ctype.h>
#include <geos_c.h>
#if 0
static ossimString monthToNumericString(const ossimString& month)
{
   ossimString result;
   if(month == "JAN")
   {
      result =  "01";
   }
   else if(month == "FEB")
   {
      result =  "02";
   }
   else if(month == "MAR")
   {
      result =  "03";
   }
   else if(month == "APR")
   {
      result =  "04";
   }
   else if(month == "MAY")
   {
      result =  "05";
   }
   else if(month == "JUN")
   {
      result =  "06";
   }
   else if(month == "JUL")
   {
      result =  "07";
   }
   else if(month == "AUG")
   {
      result =  "08";
   }
   else if(month == "SEP")
   {
      result =  "09";
   }
   else if(month == "OCT")
   {
      result =  "10";
   }
   else if(month == "NOV")
   {
      result =  "11";
   }
   else if(month == "DEC")
   {
      result =  "12";
   }

   return result;
}
#endif

static ossimString blankOutBinary(const ossimString& s)
{
   ossimString result;
   ossim_uint32 idx;
   for(idx = 0; idx < s.length(); ++idx)
   {
      int test = (int) s[idx];
      if((test >= 0x20) & (test <= 0x7e))
      {
         result += s[idx];
      }
      else
      {
         result += " ";
      }
   }

   return result;
}

#if 0
class ossimXmlOutputKeywordList : public ossimKeywordlist
{
public:
   ossimXmlOutputKeywordList(bool includeMetadataTagName = false)
   :ossimKeywordlist(),
   m_includeMetadataTagName(includeMetadataTagName)
   {
   }
   bool constainsSpecialXmlCharacters(const ossimString& value)const
   {
      for(ossimString::const_iterator it = value.begin();
         it != value.end();++it)
      {
         switch(*it)
         {
            case '&':
            case '<':
            case '>':
            case '"':
            case '\'':
            {
               return true;
            }
            default:
            {
               break;
            }
         }

      }
      return false;
   }
   virtual bool validTag(const std::string& value)const
   {
      std::string::const_iterator textChars = value.begin();
      bool result = true;
      if(!isalpha(*(textChars) ))
      {
         result = false;
      }
      else if(!value.empty())
      {
         for(++textChars;textChars!=value.end();++textChars)
         {
            bool test = isalnum(*textChars) ||
                        (*textChars == '-') ||
                        (*textChars == '_') ||
                        (*textChars == '.');
            if (!test)
            {
               result = false;
               break;
            }
         }
      }
      else
      {
         result = false;
      }

      return result;
   }
   virtual void writeToStream(std::ostream &out)const
   {
      ossimRefPtr<ossimXmlNode> metadata = new ossimXmlNode;
      metadata->setTag("metadata");
      ossimKeywordlist::KeywordMap::const_iterator iter = m_map.begin();
      while(iter != m_map.end())
      {
         ossimString path = iter->first;
         bool outputValue = true;
         ossimString value = iter->second;
         if(path.contains("unformatted_tag_data"))
         {
            ossimString temp = value.trim();
            if(ossimString(temp.begin(), temp.begin()+5) == "<?xml")
            {
               value = "XML not converted";
               outputValue = false;
            }
         }

         if(outputValue)
         {
            bool tagOk = true;
            path = path.substitute(".", "/", true);
            replaceSpecialCharacters(path);
            std::vector<ossimString> splitValues;
            path.split(splitValues,"/");
            if(splitValues.size())
            {
               splitValues[splitValues.size()-1] = splitValues[splitValues.size()-1].downcase();
               ossim_uint32 idx = 0;
               for(idx = 0; ((idx < splitValues.size()-1)&&tagOk);++idx)
               {
                  if (!validTag(splitValues[idx]))
                  {
                     tagOk = false;
                  }
                  splitValues[idx] = splitValues[idx].upcase();
               }
            }
            if(tagOk)
            {
               path.join(splitValues, "/");
               ossimRefPtr<ossimXmlNode> node = metadata->addNode(path.c_str(), value);
               if(constainsSpecialXmlCharacters(value))
               {
                  node->setCDataFlag(true);
               }
            }
         }
         ++iter;
      }

      if(!m_includeMetadataTagName)
      {
         const ossimXmlNode::ChildListType& children = metadata->getChildNodes();
         ossimXmlNode::ChildListType::const_iterator iter = children.begin();
         while(iter != children.end())
         {
            out << *(iter->get());
            ++iter;
         }
         out << std::endl;
      }
      else
      {
         out << *(metadata.get())<<std::endl;
      }
   }
   void replaceSpecialCharacters(ossimString& value)const
   {
      ossimString::iterator iter = value.begin();

      while(iter!=value.end())
      {
         if(!(isdigit(*iter) ||
              isalpha(*iter)||
              (*iter=='/')))
         {
            *iter = '_';
         }
         ++iter;
      }
   }
   bool m_includeMetadataTagName;
};
#endif
namespace oms
{
   class DataInfoPrivateData
   {
   public:
      ~DataInfoPrivateData()
      {
         clear();
      }
      void clear()
      {
         theFilename = "";
         theImageHandler = 0;
#ifdef OSSIM_VIDEO_ENABLED

         thePredatorVideo = 0;
         thePredatorVideoFrameInfo = 0;
         theExternalVideoGeometryFile = "";
#endif
      }

      std::string formatName() const
      {
#ifdef OSSIM_VIDEO_ENABLED

         if(thePredatorVideo.valid())
         {
            return "mpeg";
         }
         else
#endif
            if (theImageHandler.valid())
            {
               ossimString className = theImageHandler->getClassName();
               className = className.downcase();
               if (className.contains("dted"))
               {
                  return "dted";
               }
               else if (className.contains("tiff"))
               {
                  return "tiff";
               }
               else if (className.contains("generalraster"))
               {
                  return "general_raster";
               }
               else if (className.contains("jpeg"))
               {
                  return "jpeg";
               }
               else if (className.contains("png"))
               {
                  return "png";
               }
               else if (className.contains("nitf"))
               {
                  return "nitf";
               }
               else if (className.contains("srtm"))
               {
                  return "srtm";
               }
               else if (className.contains("nui"))
               {
                  return "nui";
               }
               else if (className.contains("doqq"))
               {
                  return "doqq";
               }
               else if (className.contains("usgsdem"))
               {
                  return "usgsdem";
               }
               else if (className.contains("ccf"))
               {
                  return "ccf";
               }
               else if (className.contains("cibcadrgtile"))
               {
                  if (theImageHandler->getNumberOfOutputBands() == 1)
                  {
                     return "cib";
                  }
                  return "cadrg";
               }
               else if (className.contains("adrgtile"))
               {
                  return "adrg";
               }
               else if (className.contains("erstile"))
               {
                  return "ers";
               }

               else if (className.contains("landsattile"))
               {
                  ossimString file = theImageHandler->getFilename();
                  file = file.downcase();
                  if (file.contains("header"))
                  {
                     return "lansat5";
                  }
                  return "landsat7";
               }
               else if (className.contains("gdaltilesource"))
               {
                  ossimString driverName =
                  theImageHandler->getPropertyValueAsString(
                                                            "driver_short_name");
                  driverName = driverName.downcase();
                  if (driverName.contains("hfa"))
                  {
                     return "imagine_hfa";
                  }
                  else if (driverName.contains("jp2"))
                  {
                     return "j2k";
                  }
                  else if (driverName.contains("jpeg2000"))
                  {
                     return "j2k";
                  }
                  else if (driverName.contains("hdf4"))
                  {
                     return "hdf4";
                  }
                  else if (driverName.contains("hdf5"))
                  {
                     return "hdf5";
                  }
                  else if (driverName.contains("bmp"))
                  {
                     return "bmp";
                  }
                  else if (driverName.contains("gif"))
                  {
                     return "gif";
                  }
                  else if (driverName.contains("aaigrid"))
                  {
                     return "aaigrid";
                  }
                  else if (driverName.contains("aig"))
                  {
                     return "aig";
                  }
                  else if (driverName.contains("doq"))
                  {
                     return "doqq";
                  }
               }
               else if (className.contains("tfrd"))
               {
                  return "tfrd";
               }
               else
               {
                  ossimFilename file(theImageHandler.valid()?theImageHandler->getFilename():"");

                  ossimString ext = file.ext();
                  ext = ext.downcase();
                  if(ext == "jp2")
                  {
                     return "jpeg2000";
                  }
                  else if(ext == "sid")
                  {
                     return "mrsid";
                  }
                  else if(ext == "dem")
                  {
                     return "usgsdem";
                  }
                  else
                  {
                     return ext;
                  }
               }
            }
         return "unspecified";
      }
      ossimFilename theFilename;
      ossimRefPtr<ossimImageHandler> theImageHandler;
#ifdef OSSIM_VIDEO_ENABLED
      ossimRefPtr<ossimPredatorVideo> thePredatorVideo;
      ossimRefPtr<ossimPredatorVideo::FrameInfo> thePredatorVideoFrameInfo;
      ossimFilename                              theExternalVideoGeometryFile;
#endif
   };
}

void replaceSpacesInKeys(ossimKeywordlist& kwl)
{
   ossimKeywordlist newKwl;

   for(auto entry:kwl.getMap())
   {
      newKwl.addPair(ossimString(entry.first).substitute(" ", "_", true).c_str(),
                 entry.second);
   }

   kwl = newKwl;
}

oms::DataInfo::DataInfo() :
thePrivateData(new oms::DataInfoPrivateData)
{
}

oms::DataInfo::~DataInfo()
{
   if (thePrivateData)
   {
      delete thePrivateData;
      thePrivateData = 0;
   }
}
static bool canCheckVideo(const ossimFilename& file)
{
   ossimString ext = file.ext();
   ext = ext.downcase();
   return((ext == "mpg") ||
          (ext == "swf") ||
          (ext == "mpeg") ||
          (ext == "flv")||
          (ext == "ts")||
          (ext == "avi")||
          (ext == "mp4"));
}

std::string oms::DataInfo::readInfo(const std::string& file, bool failIfNoGeometryFlag)
{
   DataInfo dataInfo;
   std::string results = (dataInfo.open(file, failIfNoGeometryFlag)) ? dataInfo.getInfo() : "";

   dataInfo.close();

   return results;
}

bool oms::DataInfo::open(const std::string& file, bool failIfNoGeometryFlag)
{
   bool result = false;
   try
   {
      thePrivateData->clear();

      ossimRefPtr<ossimImageHandler> handler =
         ossimImageHandlerRegistry::instance()->openConnection(file, true);

      if (handler.valid() == true)
      {
         result = setHandler(handler.get(), failIfNoGeometryFlag);
      }
      else
      {
#ifdef OSSIM_VIDEO_ENABLED
         if(canCheckVideo(file) )
         {
            thePrivateData->theFilename = file;
            thePrivateData->thePredatorVideo = new ossimPredatorVideo();
            if(thePrivateData->thePredatorVideo->open(
                  thePrivateData->theFilename))
            {
               ossimFilename externalGeom = thePrivateData->theFilename;
               externalGeom = externalGeom.setExtension("xml");

               if(!externalGeom.exists())
               {
                  // check for the first 10 seconds for a klv.
                  // If no KLV then done
                  //

                  ossim_uint32 frameMax = (thePrivateData->thePredatorVideo->videoFrameRate()*
                                           (ossim::min(10.0, thePrivateData->thePredatorVideo->duration())));
                  ossim_uint32 idx = 0;
                  for(idx=0;((idx < frameMax)&&!(thePrivateData->thePredatorVideoFrameInfo.valid()));++idx)
                  {
                     ossimRefPtr<ossimPredatorVideo::FrameInfo> frameInfo = thePrivateData->thePredatorVideo->nextFrame();
                     if(frameInfo.valid())
                     {
                        if(frameInfo->klvTable())
                        {
                           thePrivateData->thePredatorVideoFrameInfo = frameInfo;
                           // frameInfo->klvTable()->print(std::cout);
                        }
                     }
                     else
                     {
                        break;
                     }
                  }
                  if(thePrivateData->thePredatorVideoFrameInfo.valid())
                  {
                     result = true;
                  }
               }
               else
               {
                  thePrivateData->theExternalVideoGeometryFile = externalGeom;
                  result = true;
               }
            }
         }
#endif

      }

      if (result == false)
      {
         thePrivateData->clear();
      }
   }
   catch ( const std::exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms::DataInfo::open caught exception: " << e.what() << std::endl;
      result = false;
   }
   catch ( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms::DataInfo::open caught exception!" << std::endl;
      result = false;
   }

   //std::cout << "OPEN FILE? " << result << ", " << file << std::endl;

   return result;

} // End: oms::DataInfo::open( ... )

bool oms::DataInfo::setHandler(ossimImageHandler* handler, bool failIfNoGeometryFlag)
{
   bool result = false;
   if(!handler) return result;
   thePrivateData->clear();
   thePrivateData->theFilename = handler->getFilename();
   thePrivateData->theImageHandler= handler;
   // Do not do shapes for now.
   if ( (! thePrivateData->theImageHandler->
         getClassName().contains("Ogr") ) &&
       (! thePrivateData->theImageHandler->
        getClassName().contains("ossimVpf")))
   {
      if(failIfNoGeometryFlag)
      {
         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         if(geom.valid()&&geom->getProjection())
         {
            result = true;
         }
      }
      else
      {
         result = true;
      }
   }

   return result;
}

bool oms::DataInfo::getGroundCorners(std::vector<ossimGpt>& corners, int entry)
{
   bool result = false;
   ossimRefPtr<ossimImageGeometry> geom;
   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      if(entry >=0)
      {
         if(!thePrivateData->theImageHandler->setCurrentEntry(entry))
         {
            return false;
         }
      }
      geom = thePrivateData->theImageHandler->getImageGeometry();

      if(geom.valid())
      {
         corners.resize(4);
         geom->getCornerGpts(corners[0], corners[1], corners[2], corners[3]);
         result = true;
      }
   }

   return result;

}
double oms::DataInfo::getMetersPerPixel( int entry, int resolution)
{
   double result = 0.0;

   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      if(entry >=0)
      {
         if(!thePrivateData->theImageHandler->setCurrentEntry(entry))
         {
            return result;
         }
      }
      ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();

      if(geom.valid())
      {
         result = geom->getMetersPerPixel().y;
         // test if resolution is not 0 and then multiply
         if(resolution>0)
         {
            result *= (1<<resolution);
         }
      }
   }

   return result;
}

int oms::DataInfo::getNumberOfEntries()const
{
   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      return thePrivateData->theImageHandler->getNumberOfEntries();
   }

   return 0;
}

int oms::DataInfo::getNumberOfResolutionLevels(int entry)const
{
   int result = 0;

   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      if(entry >=0)
      {
         if(!thePrivateData->theImageHandler->setCurrentEntry(entry))
         {
            return 0;
         }
      }
   }

   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      result = (int)thePrivateData->theImageHandler->getNumberOfReducedResSets();
   }

   return result;
}
void oms::DataInfo::getWidthHeight(int entry, int /* resolution */, int *w, int *h)const
{
   *w = 0;
   *h = 0;

   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      if(entry >=0)
      {
         if(!thePrivateData->theImageHandler->setCurrentEntry(entry))
         {
            return ;
         }
      }
   }

   if(thePrivateData&&thePrivateData->theImageHandler.valid())
   {
      ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
      if(!rect.hasNans())
      {
         *w = rect.width();
         *h = rect.height();
      }
   }
}

void oms::DataInfo::close()
{
   if (thePrivateData)
   {
      thePrivateData->clear();
   }
}

void appendDateRange(ossimKeywordlist& kwl, 
                     const ossimDate &startDate,
                     const ossimDate &endDate, 
                     const ossimString &prefix)
{
   std::ostringstream outStart, outEnd;
   double roundStart = ((int) ((startDate.getSec()
                                + startDate.getFractionalSecond()) * 1000)) / 1000.0;
   double roundEnd =
   ((int) ((endDate.getSec() + endDate.getFractionalSecond()) * 1000))
   / 1000.0;
   outStart << std::setw(4) << std::setfill('0') << startDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << startDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << startDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << roundStart << "Z";

   outEnd << std::setw(4) << std::setfill('0') << endDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << endDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << endDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << roundEnd << "Z";

   kwl.add(prefix.c_str(), "TimeSpan.begin", outStart.str().c_str());
   kwl.add(prefix.c_str(), "TimeSpan.end", outEnd.str().c_str());

   // outputString += (indentation + "<TimeSpan>" + separator + indentation
   //                  + "   <begin>" + outStart.str() + "</begin>" + separator
   //                  + indentation + "   <end>" + outEnd.str() + "</end>" + separator
   //                  + indentation + "</TimeSpan>" + separator);

}

void appendDateRange(std::string& outputString, const ossimDate& startDate,
                     const ossimDate& endDate, const std::string& indentation,
                     const std::string& separator)
{
   std::ostringstream outStart, outEnd;
   double roundStart = ((int) ((startDate.getSec()
                                + startDate.getFractionalSecond()) * 1000)) / 1000.0;
   double roundEnd =
   ((int) ((endDate.getSec() + endDate.getFractionalSecond()) * 1000))
   / 1000.0;
   outStart << std::setw(4) << std::setfill('0') << startDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << startDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << startDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << roundStart << "Z";

   outEnd << std::setw(4) << std::setfill('0') << endDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << endDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << endDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << roundEnd << "Z";

   outputString += (indentation + "<TimeSpan>" + separator + indentation
                    + "   <begin>" + outStart.str() + "</begin>" + separator
                    + indentation + "   <end>" + outEnd.str() + "</end>" + separator
                    + indentation + "</TimeSpan>" + separator);
}
#ifdef OSSIM_VIDEO_ENABLED

void appendToMultiGeometry(std::string& /*result*/,
                           ossimRefPtr<ossimPredatorKlvTable> /*klvTable*/,
                           const std::string& /*indentation*/,
                           const std::string& /*separator*/)
{

}

std::shared_ptr<ossimPolyArea2d> createGeomFromKlv(ossimRefPtr<ossimPredatorKlvTable> klvTable)
{
   std::shared_ptr<ossimPolyArea2d> result;
   ossimGpt wgs84;
   ossimGpt ul;
   ossimGpt ur;
   ossimGpt lr;
   ossimGpt ll;
   if(!klvTable->getCornerPoints(ul, ur, lr, ll))
   {
#if 1
      ossim_float64 lat, lon, elev;
      if(klvTable->getFrameCenter(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else if(klvTable->getSensorPosition(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else
      {
         return 0;
      }
#else
      return;
#endif
   }
   ul.changeDatum(wgs84.datum());
   ur.changeDatum(wgs84.datum());
   lr.changeDatum(wgs84.datum());
   ll.changeDatum(wgs84.datum());

   // geos::io::WKTReader reader;
   // geos::geom::Geometry* result = 0;

   result = std::make_shared<ossimPolyArea2d>();
   try
   {
      result->setFromWkt("MULTIPOLYGON((("
                           +ossimString::toString(ul.lond())+" "
                           +ossimString::toString(ul.latd())+","
                           +ossimString::toString(ur.lond())+" "
                           +ossimString::toString(ur.latd())+","
                           +ossimString::toString(lr.lond())+" "
                           +ossimString::toString(lr.latd())+","
                           +ossimString::toString(ll.lond())+" "
                           +ossimString::toString(ll.latd())+","
                           +ossimString::toString(ul.lond())+" "
                           +ossimString::toString(ul.latd())+" "
                           +")))");

   }
   catch ( const std::exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms createGeomFromKlv caught ossim exception: " << e.what() << std::endl;
      result = 0;
   }
   catch(...)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms createGeomFromKlv caught ossim exception!" << std::endl;
      result = 0;
   }

   return result;
}

void appendVideoGeom(ossimKeywordlist& kwl,
                     ossimRefPtr<ossimPredatorKlvTable> klvTable,
                     const std::string& klvnumber,
                     const ossimString& prefix)
{
   ossimGpt wgs84;
   ossimGpt ul;
   ossimGpt ur;
   ossimGpt lr;
   ossimGpt ll;
   ossimString groundGeometry;
   ossimString obliquityAngle;
   ossimString sensorPosition;
   ossimString horizontalFOV;
   ossimString verticalFOV;
   ossimString elevation;
   ossimString sensorDistance;

   if (!klvTable->getCornerPoints(ul, ur, lr, ll))
   {
#if 1
      ossim_float64 lat, lon, elev;
      if (klvTable->getFrameCenter(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else if (klvTable->getSensorPosition(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else
      {
         return;
      }
#else
      return;
#endif
   }
   ul.changeDatum(wgs84.datum());
   ur.changeDatum(wgs84.datum());
   lr.changeDatum(wgs84.datum());
   ll.changeDatum(wgs84.datum());
   // RPALKO -modified to the end of appendVideoGeom
   ossimGeoPolygon geoPoly;
   geoPoly.addPoint(ul);
   geoPoly.addPoint(ur);
   geoPoly.addPoint(lr);
   geoPoly.addPoint(ll);
   double degarea = ossim::abs(geoPoly.area());

   groundGeometry += ("MULTIPOLYGON(((" + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + "," + ossimString::toString(ur.lond()) + " " + ossimString::toString(ur.latd()) + "," + ossimString::toString(lr.lond()) + " " + ossimString::toString(lr.latd()) + "," + ossimString::toString(ll.lond()) + " " + ossimString::toString(ll.latd()) + "," + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + " " + ")))");

   ossim_float32 oangle;
   if (klvTable->getObliquityAngle(oangle))
   {
      obliquityAngle += (ossimString::toString(oangle));
   }
   else
   {
      obliquityAngle = "";
   }

   ossim_float32 hfov;
   if (klvTable->getHorizontalFieldOfView(hfov))
   {
      horizontalFOV += (ossimString::toString(hfov));
   }
   else
   {
      horizontalFOV = "";
   }

   ossim_float32 vfov;
   if (klvTable->getVerticalFieldOfView(vfov))
   {
      verticalFOV += (ossimString::toString(vfov));
   }
   else
   {
      verticalFOV = "";
   }

   ossim_float64 latsp, lonsp, elevsp;
   ossimGpt llsp;

   if (klvTable->getSensorPosition(latsp, lonsp, elevsp))
   {
      llsp.latd(latsp);
      llsp.lond(lonsp);
      llsp.changeDatum(wgs84.datum());
      sensorPosition += ("POINT(" + ossimString::toString(llsp.lond()) + " " + ossimString::toString(llsp.latd()) + ")");
      elevation += (ossimString::toString(elevsp));
   }
   // Convert degrees squared area to meters squared based on sensor position
   ossimDpt sensormpd = llsp.metersPerDegree();
   ossim_float64 mtrsperlat = sensormpd.y;
   ossim_float64 mtrsperlon = sensormpd.x;
   double mtrssqdperdegsqd = mtrsperlat * mtrsperlon;
   double area = degarea * mtrssqdperdegsqd;
   ossimString geoArea;
   geoArea += (ossimString::toString(area));

   // using the distance from the sensor to the frame to weed out cases where we have absolutely bogus coordinates in the Predator video
   ossimGpt closept;
   // Use the center of the bottom of the frame as the closest point.
   closept.latd(ll.latd() - ((ll.latd() - lr.latd()) * 0.5));
   closept.lond(ll.lond() - ((ll.lond() - lr.lond()) * 0.5));

   ossimEcefPoint start(llsp);
   ossimEcefPoint end(closept);

   ossim_float64 distance = (end - start).length();
//   std::cout << "*************************** " << distance/elevation.toDouble() << std::endl;
#if 0
   ossim_float64 distance = std::pow(std::pow(((llsp.lond() -
                                                closept.lond()) * mtrsperlon), 2) + std::pow(((llsp.latd() -
                                                                                               closept.latd()) * mtrsperlat), 2), 0.5);
#endif
   sensorDistance += (ossimString::toString(distance));
   ossimString videostartutc;
   if (!klvTable->valueAsString(videostartutc,
                                KLV_KEY_VIDEO_START_DATE_TIME_UTC))
   {
      videostartutc = "";
   }
   ossimString groundGeomPrefix = prefix + "groundGeom" + klvnumber;
   ossimString newPrefix = prefix + "groundGeom" + klvnumber + ".";
   kwl.add(newPrefix.c_str(), "@area", geoArea.c_str());
   kwl.add(newPrefix.c_str(), "@elevation", elevation.c_str());
   kwl.add(newPrefix.c_str(), "@klvnumber", klvnumber.c_str());
   kwl.add(newPrefix.c_str(), "@horizontalFOV", horizontalFOV.c_str());
   kwl.add(newPrefix.c_str(), "@verticalFOV", verticalFOV.c_str());
   kwl.add(newPrefix.c_str(), "@sensorDistance", sensorDistance.c_str());
   kwl.add(newPrefix.c_str(), "@sensorPosition", sensorPosition.c_str());
   kwl.add(newPrefix.c_str(), "@videoStartUTC", videostartutc.c_str());
   kwl.add(newPrefix.c_str(), "@obliquityAngle", obliquityAngle.c_str());
   kwl.add(newPrefix.c_str(), "@srs", "EPSG:4326");
   kwl.add(groundGeomPrefix.c_str(), groundGeometry.c_str());
   // result += indentation+"<groundGeom area=\""+geoArea.string()+"\" elevation=\""
   //    +elevation.string()+"\" klvnumber=\""+klvnumber+"\" horizontalFOV=\""
   //    +horizontalFOV.string()+"\" verticalFOV=\""+verticalFOV.string()+"\" sensorDistance=\""
   //    +sensorDistance.string()+"\" sensorPosition=\""+sensorPosition.string()+"\" videoStartUTC=\""
   //    +videostartutc.string()+"\" obliquityAngle=\""+obliquityAngle.string()
   //    +"\" srs=\"epsg:4326\">"+groundGeometry.string()+"</groundGeom>"+separator;
}

void appendVideoGeom(std::string& result,
                     ossimRefPtr<ossimPredatorKlvTable> klvTable,
                     const std::string& indentation,
                     const std::string& separator,
                     const std::string& klvnumber)
{
   ossimGpt wgs84;
   ossimGpt ul;
   ossimGpt ur;
   ossimGpt lr;
   ossimGpt ll;
   ossimString groundGeometry;
   ossimString obliquityAngle;
   ossimString sensorPosition;
   ossimString horizontalFOV;
   ossimString verticalFOV;
   ossimString elevation;
   ossimString sensorDistance;

   if(!klvTable->getCornerPoints(ul, ur, lr, ll))
   {
#if 1
      ossim_float64 lat, lon, elev;
      if(klvTable->getFrameCenter(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else if(klvTable->getSensorPosition(lat, lon, elev))
      {
         ul.latd(lat);
         ul.lond(lon);
         ur = ul;
         lr = ul;
         ll = ul;
      }
      else
      {
         return;
      }
#else
      return;
#endif
   }
   ul.changeDatum(wgs84.datum());
   ur.changeDatum(wgs84.datum());
   lr.changeDatum(wgs84.datum());
   ll.changeDatum(wgs84.datum());
   // RPALKO -modified to the end of appendVideoGeom
   ossimGeoPolygon geoPoly;
   geoPoly.addPoint(ul);
   geoPoly.addPoint(ur);
   geoPoly.addPoint(lr);
   geoPoly.addPoint(ll);
   double degarea = ossim::abs(geoPoly.area());

   groundGeometry+=("MULTIPOLYGON((("
                    +ossimString::toString(ul.lond())+" "
                    +ossimString::toString(ul.latd())+","
                    +ossimString::toString(ur.lond())+" "
                    +ossimString::toString(ur.latd())+","
                    +ossimString::toString(lr.lond())+" "
                    +ossimString::toString(lr.latd())+","
                    +ossimString::toString(ll.lond())+" "
                    +ossimString::toString(ll.latd())+","
                    +ossimString::toString(ul.lond())+" "
                    +ossimString::toString(ul.latd())+" "
                    +")))");

   ossim_float32 oangle;
   if(klvTable->getObliquityAngle(oangle)) {
      obliquityAngle+=(ossimString::toString(oangle));
   }
   else {
      obliquityAngle = "";
   }

   ossim_float32 hfov;
   if(klvTable->getHorizontalFieldOfView(hfov)) {
      horizontalFOV+=(ossimString::toString(hfov));
   }
   else {
      horizontalFOV = "";
   }

   ossim_float32 vfov;
   if(klvTable->getVerticalFieldOfView(vfov)) {
      verticalFOV+=(ossimString::toString(vfov));
   }
   else {
      verticalFOV = "";
   }

   ossim_float64 latsp, lonsp, elevsp;
   ossimGpt llsp;

   if (klvTable->getSensorPosition(latsp, lonsp, elevsp))
   {
      llsp.latd(latsp);
      llsp.lond(lonsp);
      llsp.changeDatum(wgs84.datum());
      sensorPosition+=("POINT("
                       +ossimString::toString(llsp.lond())+" "
                       +ossimString::toString(llsp.latd())
                       +")");
      elevation+=(ossimString::toString(elevsp));
   }
   // Convert degrees squared area to meters squared based on sensor position
   ossimDpt sensormpd = llsp.metersPerDegree();
   ossim_float64 mtrsperlat = sensormpd.y;
   ossim_float64 mtrsperlon = sensormpd.x;
   double mtrssqdperdegsqd = mtrsperlat * mtrsperlon;
   double area = degarea * mtrssqdperdegsqd;
   ossimString geoArea;
   geoArea+=(ossimString::toString(area));

   // using the distance from the sensor to the frame to weed out cases where we have absolutely bogus coordinates in the Predator video
   ossimGpt closept;
   // Use the center of the bottom of the frame as the closest point.
   closept.latd(ll.latd() - ((ll.latd() - lr.latd()) * 0.5));
   closept.lond(ll.lond() - ((ll.lond() - lr.lond()) * 0.5));

   ossimEcefPoint start(llsp);
   ossimEcefPoint end(closept);

   ossim_float64 distance = (end-start).length();
//   std::cout << "*************************** " << distance/elevation.toDouble() << std::endl;
#if 0
   ossim_float64 distance = std::pow(std::pow(((llsp.lond() -
                                                closept.lond()) * mtrsperlon), 2) + std::pow(((llsp.latd() -
                                                                                               closept.latd()) * mtrsperlat), 2), 0.5);
#endif
   sensorDistance+=(ossimString::toString(distance));
   ossimString videostartutc;
   if (!klvTable->valueAsString(videostartutc,
                                KLV_KEY_VIDEO_START_DATE_TIME_UTC)) {
      videostartutc = "";
   }
   result += indentation+"<groundGeom area=\""+geoArea.string()+"\" elevation=\""
      +elevation.string()+"\" klvnumber=\""+klvnumber+"\" horizontalFOV=\""
      +horizontalFOV.string()+"\" verticalFOV=\""+verticalFOV.string()+"\" sensorDistance=\""
      +sensorDistance.string()+"\" sensorPosition=\""+sensorPosition.string()+"\" videoStartUTC=\""
      +videostartutc.string()+"\" obliquityAngle=\""+obliquityAngle.string()
      +"\" srs=\"epsg:4326\">"+groundGeometry.string()+"</groundGeom>"+separator;
}
#endif

std::string oms::DataInfo::getInfo() const
{
   std::string result = "";
   ossimKeywordlist kwl;
   bool errorsOcurred = false;
   try
   {
      if (!thePrivateData)
         return result;

      if (thePrivateData->theImageHandler.valid())
      {
         appendAssociatedFiles(kwl, "oms.dataSets.RasterDataSet0.fileObjects.");
         appendRasterEntries(kwl, "oms.dataSets.RasterDataSet0.rasterEntries.");
         // appendRasterDataSetMetadata(kwl, "oms.dataSets.RasterDataSet0.");
      }
#ifdef OSSIM_VIDEO_ENABLED
      else if(!thePrivateData->theExternalVideoGeometryFile.empty()&&
              thePrivateData->theExternalVideoGeometryFile.exists())
      {
         std::vector<char> buf;
         ossim_uint64 filesize = thePrivateData->theExternalVideoGeometryFile.fileSize();
         if(filesize>0)
         {
            std::ifstream in(thePrivateData->theExternalVideoGeometryFile.c_str(), std::ios::in|std::ios::binary);
            if(in.good())
            {
               buf.resize(filesize);
               in.read(&buf.front(), filesize);

               result = std::string(buf.begin(), buf.end());
            }
         }
      }
      else if(thePrivateData->thePredatorVideoFrameInfo.valid())
      {
         ossimDate startDate;
         ossimDate endDate;
         if(thePrivateData->thePredatorVideoFrameInfo->klvTable()->getDate(startDate, true))
         {
            endDate = startDate;
            endDate.addSeconds(thePrivateData->thePredatorVideo->duration());
         }
         kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.@type"
                 ,"main");
         kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.@format", thePrivateData->formatName().c_str());
         kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.name", thePrivateData->theFilename.c_str());
         kwl.add("oms.dataSets.VideoDataSet0.width", thePrivateData->thePredatorVideo->imageWidth());
         kwl.add("oms.dataSets.VideoDataSet0.height", thePrivateData->thePredatorVideo->imageHeight());
         ossimString spatialMetaPrefix = "oms.dataSets.VideoDataSet0.spatialMetaData.";

         // // RPALKO - replaced 1 line with everything to END RPALKO
         ossim_uint32 idx = 0;
         // safer to rewind
         std::shared_ptr<ossimPolyArea2d> composite = 0;
         thePrivateData->thePredatorVideo->rewind();
         ossimRefPtr<ossimPredatorVideo::KlvInfo> klvInfo = thePrivateData->thePredatorVideo->nextKlv();

// this code does a union of all the polygons
#if 1
         while(klvInfo.valid() &&klvInfo->table())
         {
            std::shared_ptr<ossimPolyArea2d> geom = createGeomFromKlv(klvInfo->table());
            //std::cout << klvInfo->table()->print(std::cout) << std::endl;
            if(!composite)
            {
               if(geom->isValid())
               {
                  composite = geom;
               }
            }
            else if(geom)
            {
               try
               {
                  if(geom->isValid())
                  {
                     *composite+=*geom;

                     geom = 0;
                  }
               }
               catch ( const std::exception& e )
               {
                  ossimNotify(ossimNotifyLevel_WARN)
                     << "oms::DataInfo::getInfo caught ossim exception: " << e.what() << std::endl;
                  geom = 0;
               }
               catch(...)
               {
                  ossimNotify(ossimNotifyLevel_WARN)
                     << "oms::DataInfo::getInfo caught ossim exception!" << std::endl;
                  geom = 0;
               }
            }
            klvInfo = thePrivateData->thePredatorVideo->nextKlv();
            idx++;
         }
         if(composite)
         {
            try
            {
               kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom.@area", composite->getArea()*ossimGpt().metersPerDegree().y);
               kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom.@srs", "epsg:4326");
               kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom", composite->toString().c_str());
            }
            catch ( const std::exception& e )
            {
               ossimNotify(ossimNotifyLevel_WARN)
                  << "oms::DataInfo::getInfo caught ossim exception: " << e.what() << std::endl;
            }
            catch(...)
            {
               ossimNotify(ossimNotifyLevel_WARN)
                  << "oms::DataInfo::getInfo caught ossim exception!" << std::endl;
            }

            composite = 0;
         }

         // this code outputs each polygon
#else

         while (klvInfo.valid() && klvInfo->table())
         {
            ossimString klvnumber = ossimString::toString(idx);
            appendVideoGeom(kwl, klvInfo->table(),
                            klvnumber.c_str(), spatialMetaPrefix);
            klvInfo = thePrivateData->thePredatorVideo->nextKlv();
            idx++;
         }
#endif
         appendDateRange(kwl, startDate, endDate, "oms.dataSets.VideoDataSet0.");
         appendVideoDataSetMetadata(kwl, "oms.dataSets.VideoDataSet0.");
      }
#endif
   }
   catch ( const std::exception& e )
   {
      errorsOcurred = true;
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms::DataInfo::getInfo caught ossim exception: " << e.what() << std::endl;
   }
   catch ( ... )
   {
      errorsOcurred = true;
      ossimNotify(ossimNotifyLevel_WARN)
         << "oms::DataInfo::getInfo caught exception!" << std::endl;
   }

   if(kwl.getSize()&&!errorsOcurred)
   {
      std::ostringstream out;
      // std::shared_ptr<ossim::KwlNodeXmlFormatter> formatter =
      //     std::make_shared<ossim::KwlNodeXmlFormatter>(kwl, "");
      // ossim::KwlNodeFormatter *baseFormatter = formatter.get();
      // baseFormatter->write(out, ossim::KwlNodeFormatter::FormatHints(3,false, false, false));

      kwl.toXML(out, "");
      result = out.str();
   }

   return result;

} // End: std::string oms::DataInfo::getInfo() const

bool oms::DataInfo::isVideo()const
{
   if(!thePrivateData) return false;
#ifdef OSSIM_VIDEO_ENABLED
   return thePrivateData->thePredatorVideo.valid();
#else
   return false;
#endif
}

bool oms::DataInfo::isImagery()const
{
   if(!thePrivateData) return false;
   return thePrivateData->theImageHandler.valid();
}

std::string oms::DataInfo::getImageInfo(int entry)
{
   std::string result = "";
   if (!thePrivateData)
      return result;
   if(!thePrivateData->theImageHandler.valid())
   {
      return result;
   }
   if(!thePrivateData->theImageHandler->setCurrentEntry(entry)) return result;

   if (thePrivateData->theImageHandler.valid())
   {
      ossimKeywordlist kwl;
      appendAssociatedFiles(kwl, "oms.dataSets.RasterDataSet.fileObjects.");
      if(entry < 0)
      {
         appendRasterEntries(kwl, "oms.dataSets.RasterDataSet.rasterEntries.");
      }
      else
      {
         appendRasterEntry(kwl, "oms.dataSets.RasterDataSet.rasterEntries.RasterEntry0.");
      }

      // appendRasterDataSetMetadata(kwl, "oms.dataSets.RasterDataSet.");
      std::ostringstream out;
      // std::shared_ptr<ossim::KwlNodeXmlFormatter> formatter =
      //     std::make_shared<ossim::KwlNodeXmlFormatter>(kwl, "");
      // ossim::KwlNodeFormatter *baseFormatter = formatter.get();
      // baseFormatter->write(out, ossim::KwlNodeFormatter::FormatHints(3, false, false, false));

      kwl.toXML(out, "");
      result = out.str();
   }

   return result;

}

std::string oms::DataInfo::getVideoInfo()
{
   std::string result;
   ossimKeywordlist kwl;
#ifdef OSSIM_VIDEO_ENABLED
   if(!thePrivateData->theExternalVideoGeometryFile.empty()&&
           thePrivateData->theExternalVideoGeometryFile.exists())
   {
      std::vector<char> buf;
      ossim_uint64 filesize = thePrivateData->theExternalVideoGeometryFile.fileSize();
      if(filesize>0)
      {
         std::ifstream in(thePrivateData->theExternalVideoGeometryFile.c_str(), std::ios::in|std::ios::binary);
         if(in.good())
         {
            buf.resize(filesize);
            in.read(&buf.front(), filesize);

            result = std::string(buf.begin(), buf.end());
         }
      }
   }
   else if(thePrivateData->thePredatorVideoFrameInfo.valid())
   {

      ossimDate startDate;
      ossimDate endDate;
      if(thePrivateData->thePredatorVideoFrameInfo->klvTable()->getDate(startDate, true))
      {
         endDate = startDate;
         endDate.addSeconds(thePrivateData->thePredatorVideo->duration());
      }
      kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.@type", "main");
      kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.@format", thePrivateData->formatName().c_str());
      kwl.add("oms.dataSets.VideoDataSet0.fileObjects.VideoFile0.name", thePrivateData->theFilename.c_str());
      kwl.add("oms.dataSets.VideoDataSet0.width", thePrivateData->thePredatorVideo->imageWidth());
      kwl.add("oms.dataSets.VideoDataSet0.height", thePrivateData->thePredatorVideo->imageHeight());
      ossimString spatialMetaPrefix = "oms.dataSets.VideoDataSet0.spatialMetaData.";

      // RPALKO - replaced 1 line with everything to END RPALKO
      ossim_uint32 idx = 0;
      // safer to rewind
      thePrivateData->thePredatorVideo->rewind();
      ossimRefPtr<ossimPredatorVideo::KlvInfo> klvInfo = thePrivateData->thePredatorVideo->nextKlv();
// this code does a union of all the polygons
#if 1
      std::shared_ptr<ossimPolyArea2d> composite = std::make_shared<ossimPolyArea2d>();
      while (klvInfo.valid() && klvInfo->table())
      {
         std::shared_ptr<ossimPolyArea2d> geom = createGeomFromKlv(klvInfo->table());
         *composite += *geom;
         klvInfo = thePrivateData->thePredatorVideo->nextKlv();
         idx++;
      }
      if(!composite->isEmpty())
      {

         kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom.@area", composite->getArea() * ossimGpt().metersPerDegree().y);
         kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom.@srs", "epsg:4326");
         kwl.add("oms.dataSets.VideoDataSet0.spatialMetadata.groundGeom", composite->toString().c_str());
         composite = 0;
      }

      // this code outputs each polygon
#else

      while (klvInfo.valid() && klvInfo->table())
      {
         ossimString klvnumber = ossimString::toString(idx);
         appendVideoGeom(kwl, klvInfo->table(),
                         klvnumber.c_str(), spatialMetaPrefix);
         klvInfo = thePrivateData->thePredatorVideo->nextKlv();
         idx++;
      }
#endif

      appendDateRange(kwl, startDate, endDate, "oms.dataSets.VideoDataSet0.");
      appendVideoDataSetMetadata(kwl, "oms.dataSets.VideoDataSet0.");
   }
#endif
   if (kwl.getSize())
   {
      std::ostringstream out;
      kwl.toXML(out);
      result = out.str();
   }

   return result;
}

void oms::DataInfo::appendRasterEntry( std::string& outputString,
                                       const std::string& indentation,
                                       const std::string& separator ) const
{
   ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
   outputString += indentation + "<RasterEntry>" + separator;
   appendAssociatedRasterEntryFileObjects(outputString, indentation
                                          + "   ", separator);
   outputString += indentation + "   <entryId>" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()).string() + "</entryId>" + separator;
   outputString += indentation + "   <width>" + ossimString::toString(
      rect.width()).string() + "</width>" + separator;
   outputString += indentation + "   <height>" + ossimString::toString(
      rect.height()).string() + "</height>" + separator;
   outputString
   += indentation + "   <numberOfBands>"
   + ossimString::toString(
      ossim::max(thePrivateData->theImageHandler->getNumberOfOutputBands(),
         thePrivateData->theImageHandler->getNumberOfInputBands())).string()
   + "</numberOfBands>" + separator;
   outputString
   += indentation + "   <numberOfResLevels>"
      + ossimString::toString(
         thePrivateData->theImageHandler->getNumberOfDecimationLevels()).string()
   + "</numberOfResLevels>" + separator;
   appendBitDepthAndDataType(outputString, indentation + "   ", separator);
   appendGeometryInformation(outputString, indentation + "   ", separator);
//   appendRasterEntryDateTime(outputString, indentation + "   ", separator);
   appendRasterEntryMetadata(outputString, indentation + "   ", "\n");

   outputString += indentation + "</RasterEntry>" + separator;

}

void oms::DataInfo::appendRasterEntries(std::string& outputString,
                                        const std::string& indentation, const std::string& separator) const
{
   ossim_uint32 numberOfEntries =
   thePrivateData->theImageHandler->getNumberOfEntries();
   ossim_uint32 idx = 0;

   for (idx = 0; idx < numberOfEntries; ++idx)
   {
      if(thePrivateData->theImageHandler->setCurrentEntry(idx))
      {
         appendRasterEntry(outputString, indentation, separator);
      }
   }
}

void oms::DataInfo::appendRasterEntry(ossimKeywordlist& kwl,
                                      const ossimString& prefix) const
{
   ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
   kwl.add(prefix.c_str(), "entryId", ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()).c_str());
   kwl.add(prefix.c_str(), "width", ossimString::toString(rect.width()).c_str());
   kwl.add(prefix.c_str(), "height", ossimString::toString(rect.height()).c_str());
   kwl.add(prefix.c_str(), "numberOfBands", ossimString::toString(ossim::max(thePrivateData->theImageHandler->getNumberOfOutputBands(), thePrivateData->theImageHandler->getNumberOfInputBands())).c_str());
   kwl.add(prefix.c_str(), "numberOfResLevels", ossimString::toString(ossim::max(thePrivateData->theImageHandler->getNumberOfDecimationLevels(), thePrivateData->theImageHandler->getNumberOfInputBands())).c_str());
   appendAssociatedRasterEntryFileObjects(kwl, prefix);
   appendBitDepthAndDataType(kwl, prefix);
   appendGeometryInformation(kwl, prefix);
      //  appendRasterEntryDateTime(outputString, indentation + "   ", separator);
   appendRasterEntryMetadata(kwl, prefix);

   // outputString += indentation + "</RasterEntry>" + separator;
}

void oms::DataInfo::appendRasterEntries(ossimKeywordlist &kwl,
                                        const ossimString &prefix) const
{
   ossim_uint32 numberOfEntries =
       thePrivateData->theImageHandler->getNumberOfEntries();
   ossim_uint32 idx = 0;

   for (idx = 0; idx < numberOfEntries; ++idx)
   {
      ossimString newPrefix = prefix + "RasterEntry" + ossimString::toString(idx) + ".";
      if (thePrivateData->theImageHandler->setCurrentEntry(idx))
      {
         appendRasterEntry(kwl, newPrefix);
      }
   }
}

std::string oms::DataInfo::checkAndGetThumbnail(const std::string &baseNoExtension) const
{
   ossimFilename thumnailTest = baseNoExtension + "thumb.jpg";
   if (thumnailTest.exists())
   {
      return thumnailTest.string();
   }
   thumnailTest = baseNoExtension + "thumbnail.jpg";
   if (thumnailTest.exists())
   {
         return thumnailTest.string();
   }
   thumnailTest = baseNoExtension + "thumbnail.png";
   if (thumnailTest.exists())
   {
         return thumnailTest.string();
   }
   thumnailTest = baseNoExtension + "thumb.png";
   if (thumnailTest.exists())
   {
         return thumnailTest.string();
   }
   return "";
}
void oms::DataInfo::appendAssociatedFiles(ossimKeywordlist &kwl, 
                                          const ossimString &prefix) const
{
   ossimFilename mainFile = thePrivateData->theImageHandler->getFilename();
   ossim_uint32 startIdx = 1;
   ossimString newPrefix = prefix + "RasterFile";
   kwl.add(prefix.c_str(), "RasterFile0.@type", "main");
   kwl.add(prefix.c_str(), "RasterFile0.@format", thePrivateData->formatName().c_str());
   kwl.add(prefix.c_str(), "RasterFile0.name", thePrivateData->theFilename.c_str());

   if ((mainFile.ext().downcase() == "h5") || (mainFile.contains("_noaa_ops")))
   {
      //---
      // Special case/hack to handle NPP VIIRS hdf5 files that were split by
      // NOAA for customer.
      //
      // Example:
      // GDNBO_npp_d20140113_t0828103_e0833507_b11463_c20140113143351340475_noaa_ops.h5
      // IICMO_npp_d20140113_t0828103_e0833507_b11463_c20140113143351446171_noaa_ops.h5
      // SVDNB_npp_d20140113_t0828103_e0833507_b11463_c20140113143351340692_noaa_ops.h5
      // SVM15_npp_d20140113_t0828103_e0833507_b11463_c20140113143351447600_noaa_ops.h5
      //---
      std::string searchStr = "_npp";
      std::string::size_type pos = mainFile.find(searchStr, 0);
      if ((pos != std::string::npos) && (mainFile.size() > (pos + 5)))
      {
         pos += 5; // Skip past: "_npp_"
         ossimString os = mainFile.substr(pos, mainFile.size() - pos);
         ossimString stringSeparator = "_";
         std::vector<ossimString> list;

         os.split(list, stringSeparator);
         if (list.size() > 5)
         {
            ossimFilename expandedMain = mainFile.expand();
            ossimFilename dirParent = expandedMain.path();
            ossimDirectory dir;
            if (dir.open(dirParent))
            {
               ossimFilename file;
               if (dir.getFirst(file, ossimDirectory::OSSIM_DIR_FILES))
               {
                  do
                  {
                     file = file.expand();
                     if ((file != expandedMain))
                     {
                        ossimString ext = file.ext().downcase();
                        if ((ext != "geom") && (ext != "his") && (ext != "ocg") &&
                            (ext != "omd") && (ext != "ovr") && !file.contains("thumb"))
                        {
                           if (file.contains(list[0]) && // e.g. d20140115
                               file.contains(list[1]) && // e.g. t2337431
                               file.contains(list[2]) && // e.g. e2343235
                               file.contains(list[3]))   // e.g. b11500
                           {
                              ossimString tempPrefix = (newPrefix + ossimString::toString(startIdx++) + ".RasterFile.");
                              kwl.add(tempPrefix + "@type", "support");
                              kwl.add(tempPrefix + "name", file.c_str());
                              //outputString += indentation + "   <RasterFile type=\"support\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(file).string() + "</name>" + separator + indentation + "   </RasterFile>" + separator;
                           }
                        }
                     }

                  } while (dir.getNext(file));
               }
            }
         }
      }
   }
}

void oms::DataInfo::appendAssociatedFiles(std::string& outputString,
                                          const std::string& indentation,
                                          const std::string& separator) const
{
   ossimFilename mainFile = thePrivateData->theImageHandler->getFilename();

   if ( ( mainFile.ext().downcase() == "h5" ) || (  mainFile.contains("_noaa_ops") ) )
   {
      //---
      // Special case/hack to handle NPP VIIRS hdf5 files that were split by
      // NOAA for customer.
      //
      // Example:
      // GDNBO_npp_d20140113_t0828103_e0833507_b11463_c20140113143351340475_noaa_ops.h5
      // IICMO_npp_d20140113_t0828103_e0833507_b11463_c20140113143351446171_noaa_ops.h5
      // SVDNB_npp_d20140113_t0828103_e0833507_b11463_c20140113143351340692_noaa_ops.h5
      // SVM15_npp_d20140113_t0828103_e0833507_b11463_c20140113143351447600_noaa_ops.h5
      //---
      std::string searchStr = "_npp";
      std::string::size_type pos = mainFile.find( searchStr, 0 );
      if ( (pos != std::string::npos) && ( mainFile.size() > (pos+5) ) )
      {
         pos += 5; // Skip past: "_npp_"
         ossimString os = mainFile.substr( pos, mainFile.size()-pos );
         ossimString stringSeparator = "_";
         std::vector<ossimString> list;

         os.split( list, stringSeparator );
         if ( list.size() > 5)
         {
            ossimFilename expandedMain = mainFile.expand();
            ossimFilename dirParent = expandedMain.path();
            ossimDirectory dir;
            if( dir.open(dirParent ))
            {
               ossimFilename file;
               if( dir.getFirst(file, ossimDirectory::OSSIM_DIR_FILES) )
               {
                  do
                  {
                     file = file.expand();
                     if ( (file != expandedMain) )
                     {
                        ossimString ext = file.ext().downcase();
                        if ( ( ext != "geom") && (ext != "his") && ( ext != "ocg") &&
                             (ext != "omd") && ( ext != "ovr") && !file.contains("thumb") )
                        {
                           if ( file.contains( list[0] ) && // e.g. d20140115
                                file.contains( list[1] ) && // e.g. t2337431
                                file.contains( list[2] ) && // e.g. e2343235
                                file.contains( list[3] ) )  // e.g. b11500
                           {
                              outputString += indentation
                                 + "   <RasterFile type=\"support\">" + separator
                                 + indentation + "      <name>"
                                 + ossimXmlString::wrapCDataIfNeeded(file).string() + "</name>"
                                 + separator + indentation + "   </RasterFile>"
                                 + separator;
                           }
                        }
                     }

                  } while ( dir.getNext( file ) );
               }
            }
         }
      }
   }
}

void oms::DataInfo::appendAssociatedRasterEntryFileObjects(
    ossimKeywordlist& kwl, const ossimString& prefix) const
{
   ossim_uint32 idx = 0;
   ossimRegExp dgregex("m1bs|p1bs");
   ossimFilename mainFile = thePrivateData->theImageHandler->getFilename();
   ossimFilename kmlFile(thePrivateData->theImageHandler->getFilename());
   ossimFilename overviewFile =
       thePrivateData->theImageHandler->createDefaultOverviewFilename();
   ossimFilename overview2File =
       thePrivateData->theImageHandler->getFilename() + ".ovr";
   ossimFilename hdrFile =
       thePrivateData->theImageHandler->getFilename() + ".hdr";
   ossimFilename navData =
       thePrivateData->theImageHandler->getFilename().path();
   ossimFilename metaData = navData.dirCat("Metadata");
   navData = navData.dirCat("NavData");
   ossimFilename histogramFile =
       thePrivateData->theImageHandler->createDefaultHistogramFilename();
   ossimFilename
       validVerticesFile =
           thePrivateData->theImageHandler->createDefaultValidVerticesFilename();
   ossimFilename geomFile =
       thePrivateData->theImageHandler->createDefaultGeometryFilename();
   ossimFilename coarseGridFile =
       thePrivateData->theImageHandler->createDefaultGeometryFilename();
   ossimFilename metadataFile =
       thePrivateData->theImageHandler->createDefaultMetadataFilename();
   ossimFilename aDotToc = thePrivateData->theImageHandler->getFilename().file();
   ossimFilename baseNoExtension = thePrivateData->theImageHandler->getFilenameWithThisExtension("");

   coarseGridFile = coarseGridFile.setExtension("ocg");
   bool digitalGlobeFlag = false;
   ossimString drivePart;
   ossimString pathPart;
   ossimString filePart;
   ossimString extPart;
   mainFile.split(drivePart, pathPart,
                  filePart, extPart);
   ossimString downcaseFilePart = filePart.downcase();
   if (dgregex.find(downcaseFilePart.c_str()))
   {
      // it's digital globe naming so lets check for external files
      digitalGlobeFlag = true;
   }
   // we will only support for now kml files associated at the entire file level and
   // not individual entries.
   //
   //std::cout <<"PATH1:::::::::::::::: "<< thePrivateData->theImageHandler->getFilename().path() << "\n";
   //std::cout <<"PATH2:::::::::::::::: "<< thePrivateData->theImageHandler->getFilename().path().path() << "\n";

   kmlFile = kmlFile.setExtension("kml");
   ossimFilename thumbnailFile = checkAndGetThumbnail(baseNoExtension);
   //bool associatedFilesFlag = (thumbnailFile.exists() || overviewFile.exists() || overview2File.exists() || histogramFile.exists()
   //                            || validVerticesFile.exists() || geomFile.exists()
   //                            || metadataFile.exists()||kmlFile.exists()||navData.exists());
   // if (associatedFilesFlag)
   //{
   ossimString fileObjectsPrefix = prefix + "fileObjects.";
   if (digitalGlobeFlag)
   {
      ossimFilename dgFile = mainFile;
      dgFile.setExtension("TIL");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile"+ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix+"@type").c_str(),
                 "dgtil");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("IMD");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgimd");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("ATT");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgatt");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("GEO");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dggeo");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("EPH");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgeph");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("STE");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgste");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("XML");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgxml");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
      dgFile.setExtension("RPB");
      if (dgFile.exists())
      {
         ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "@type").c_str(),
                 "dgrpb");
         kwl.add(fileObjectsPrefix.c_str(),
                 (keyPrefix + "name").c_str(),
                 dgFile.c_str());
         ++idx;
      }
   }
   if (!thumbnailFile.empty())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "thumbnail");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              thumbnailFile.c_str());
      ++idx;
   }
   if (overviewFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "overview");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              overviewFile.c_str());
      ++idx;
   }
   else if (overview2File.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "overview");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              overview2File.c_str());
      ++idx;
   }
   if (histogramFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "histogram");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              histogramFile.c_str());
      ++idx;
   }
   if (validVerticesFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "valid_vertices");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              validVerticesFile.c_str());
      ++idx;
   }
   if (coarseGridFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "coarseGrid");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              coarseGridFile.c_str());
      ++idx;
   }
   if (geomFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "geom");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              geomFile.c_str());
      ++idx;
   }
   if (metadataFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "omd");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              metadataFile.c_str());
      ++idx;
   }
   if (kmlFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "kml");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              kmlFile.c_str());
      ++idx;
   }
   if (hdrFile.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "hdr");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              hdrFile.c_str());
      ++idx;
   }
   if (navData.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "NavData");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              navData.c_str());
      ++idx;
   }
   if (metaData.exists())
   {
      ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "@type").c_str(),
              "Metadata");
      kwl.add(fileObjectsPrefix.c_str(),
              (keyPrefix + "name").c_str(),
              metaData.c_str());
      ++idx;
   }
   if (aDotToc.downcase() == "a.toc")
   {
      ossimFilename dirParent = mainFile.path();
      ossimDirectory dir;
      if (dir.open(dirParent))
      {
         ossimFilename dirPath;
         if (dir.getFirst(dirPath, ossimDirectory::OSSIM_DIR_DIRS))
         {
            do
            {
               ossimString keyPrefix = "RasterEntryFile" + ossimString::toString(idx) + ".";

               kwl.add(fileObjectsPrefix.c_str(),
                       (keyPrefix + "@type").c_str(),
                       "directory");
               kwl.add(fileObjectsPrefix.c_str(),
                       (keyPrefix + "name").c_str(),
                       dirPath.c_str());
               ++idx;
            } while (dir.getNext(dirPath));
         }
      }
   }

   // outputString += indentation + "</fileObjects>" + separator;
   //}
}

void oms::DataInfo::appendAssociatedRasterEntryFileObjects(
       std::string & outputString, const std::string &indentation,
       const std::string &separator) const
   {
      ossimRegExp dgregex("m1bs|p1bs");
      ossimFilename mainFile = thePrivateData->theImageHandler->getFilename();
      ossimFilename kmlFile(thePrivateData->theImageHandler->getFilename());
      ossimFilename overviewFile =
          thePrivateData->theImageHandler->createDefaultOverviewFilename();
      ossimFilename overview2File =
          thePrivateData->theImageHandler->getFilename() + ".ovr";
      ossimFilename hdrFile =
          thePrivateData->theImageHandler->getFilename() + ".hdr";
      ossimFilename navData =
          thePrivateData->theImageHandler->getFilename().path();
      ossimFilename metaData = navData.dirCat("Metadata");
      navData = navData.dirCat("NavData");
      ossimFilename histogramFile =
          thePrivateData->theImageHandler->createDefaultHistogramFilename();
      ossimFilename
          validVerticesFile =
              thePrivateData->theImageHandler->createDefaultValidVerticesFilename();
      ossimFilename geomFile =
          thePrivateData->theImageHandler->createDefaultGeometryFilename();
      ossimFilename coarseGridFile =
          thePrivateData->theImageHandler->createDefaultGeometryFilename();
      ossimFilename metadataFile =
          thePrivateData->theImageHandler->createDefaultMetadataFilename();
      ossimFilename aDotToc = thePrivateData->theImageHandler->getFilename().file();
      ossimFilename baseNoExtension = thePrivateData->theImageHandler->getFilenameWithThisExtension("");

      coarseGridFile = coarseGridFile.setExtension("ocg");
      bool digitalGlobeFlag = false;
      ossimString drivePart;
      ossimString pathPart;
      ossimString filePart;
      ossimString extPart;
      mainFile.split(drivePart, pathPart,
                     filePart, extPart);
      ossimString downcaseFilePart = filePart.downcase();
      if (dgregex.find(downcaseFilePart.c_str()))
      {
         // it's digital globe naming so lets check for external files
         digitalGlobeFlag = true;
      }
      // we will only support for now kml files associated at the entire file level and
      // not individual entries.
      //
      //std::cout <<"PATH1:::::::::::::::: "<< thePrivateData->theImageHandler->getFilename().path() << "\n";
      //std::cout <<"PATH2:::::::::::::::: "<< thePrivateData->theImageHandler->getFilename().path().path() << "\n";

      kmlFile = kmlFile.setExtension("kml");
      ossimFilename thumbnailFile = checkAndGetThumbnail(baseNoExtension);
      //bool associatedFilesFlag = (thumbnailFile.exists() || overviewFile.exists() || overview2File.exists() || histogramFile.exists()
      //                            || validVerticesFile.exists() || geomFile.exists()
      //                            || metadataFile.exists()||kmlFile.exists()||navData.exists());
      // if (associatedFilesFlag)
      //{
      outputString += indentation + "<fileObjects>" + separator;

      if (digitalGlobeFlag)
      {
         ossimFilename dgFile = mainFile;
         dgFile.setExtension("TIL");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgtil\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("IMD");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgimd\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("ATT");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgatt\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("GEO");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dggeo\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("EPH");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgeph\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("STE");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgste\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("TIL");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgtil\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("XML");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgxml\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
         dgFile.setExtension("RPB");
         if (dgFile.exists())
         {
            outputString += indentation + "   <RasterEntryFile type=\"dgrpb\">" +
                            separator + indentation + "      <name>" +
                            ossimXmlString::wrapCDataIfNeeded(dgFile).string() +
                            "</name>" + separator + indentation +
                            "   </RasterEntryFile>" + separator;
         }
      }
      if (!thumbnailFile.empty())
      {
         outputString += indentation + "   <RasterEntryFile type=\"thumbnail\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(thumbnailFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (overviewFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"overview\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(overviewFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      else if (overview2File.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"overview\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(overview2File).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (histogramFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"histogram\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(histogramFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (validVerticesFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"valid_vertices\">" + separator + indentation + "      <name>" + validVerticesFile.string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (coarseGridFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"coarseGrid\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(coarseGridFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (geomFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"geom\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(geomFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (metadataFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"omd\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(metadataFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (kmlFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"kml\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(kmlFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (hdrFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"hdr\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(hdrFile).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (navData.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"NavData\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(navData).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (metaData.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"Metadata\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(metaData).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
      }
      if (aDotToc.downcase() == "a.toc")
      {
         ossimFilename dirParent = mainFile.path();
         ossimDirectory dir;
         if (dir.open(dirParent))
         {
            ossimFilename dirPath;
            if (dir.getFirst(dirPath, ossimDirectory::OSSIM_DIR_DIRS))
            {
               do
               {
                  outputString += indentation + "   <RasterEntryFile type=\"directory\">" + separator + indentation + "      <name>" + ossimXmlString::wrapCDataIfNeeded(dirPath).string() + "</name>" + separator + indentation + "   </RasterEntryFile>" + separator;
               } while (dir.getNext(dirPath));
            }
         }
      }

      outputString += indentation + "</fileObjects>" + separator;
      //}
   }

   void oms::DataInfo::appendBitDepthAndDataType(ossimKeywordlist& kwl,
                                                 const ossimString& prefix) const
   {
      ossim_uint32 bits = 0;
      ossimString dataType;
      ossimScalarType scalarType = thePrivateData->theImageHandler->getOutputScalarType();
      ossimString bitDepth;
   
      //omd overrides
      if (thePrivateData && thePrivateData->theImageHandler){
         ossimFilename omd = thePrivateData->theImageHandler->createDefaultMetadataFilename();
         if(omd.exists())
         {
            ossimKeywordlist omdKwl(omd);
            bitDepth = omdKwl.findKey("bit_depth");
            dataType = omdKwl.findKey("data_type");
         }
      }

      if( ! bitDepth.empty() && ! dataType.empty()){
         bits = bitDepth.toInt32();
      } else {
         switch (scalarType){
            case OSSIM_UINT8:
            {
               bits = 8;
               dataType = "uint";
               break;
            }
            case OSSIM_SINT8:
            {
               bits = 8;
               dataType = "sint";
               break;
            }
            case OSSIM_UINT12:
            {
               bits = 12;
               dataType = "uint";
               break;
            }
            case OSSIM_UINT16:
            {
               bits = 16;
               dataType = "uint";
               break;
            }
            case OSSIM_SINT16:
            {
               bits = 16;
               dataType = "sint";
               break;
            }
            case OSSIM_USHORT11:
            {
               bits = 11;
               dataType = "uint";
               break;
            }
            case OSSIM_SINT32:
            {
               bits = 32;
               dataType = "sint";
               break;
            }
            case OSSIM_UINT32:
            {
               bits = 32;
               dataType = "uint";
               break;
            }
            case OSSIM_FLOAT32:
            {
               bits = 32;
               dataType = "float";
               break;
            }
            case OSSIM_FLOAT64:
            {
               bits = 64;
               dataType = "float";
               break;
            }
            default:
            {
               dataType = "unknown";
               break;
            }
         }
      }

      kwl.add(prefix.c_str(), "bitDepth", ossimString::toString(bits).c_str());
      kwl.add(prefix.c_str(), "dataType", dataType.c_str());
   }
   void oms::DataInfo::appendBitDepthAndDataType(std::string &outputString,
                                                 const std::string &indentation, const std::string &separator) const
   {
      ossim_uint32 bits = 0;
      ossimString dataType;

      ossimScalarType scalarType = thePrivateData->theImageHandler->getOutputScalarType();

      // std::cout << "This is the scalarType: " <<  ossimScalarTypeLut::instance()->getEntryString(scalarType) << std::endl;

      switch (scalarType)
      {
      case OSSIM_UINT8:
      {
         bits = 8;
         dataType = "uint";
         break;
      }
      case OSSIM_SINT8:
      {
         bits = 8;
         dataType = "sint";
         break;
      }
      case OSSIM_UINT12:
      {
         bits = 12;
         dataType = "uint";
         break;
      }
      case OSSIM_UINT16:
      {
         bits = 16;
         dataType = "uint";
         break;
      }
      case OSSIM_SINT16:
      {
         bits = 16;
         dataType = "sint";
         break;
      }
      case OSSIM_USHORT11:
      {
         bits = 11;
         dataType = "uint";
         break;
      }
      case OSSIM_SINT32:
      {
         bits = 32;
         dataType = "sint";
         break;
      }
      case OSSIM_UINT32:
      {
         bits = 32;
         dataType = "uint";
         break;
      }
      case OSSIM_FLOAT32:
      {
         bits = 32;
         dataType = "float";
         break;
      }
      case OSSIM_FLOAT64:
      {
         bits = 64;
         dataType = "float";
         break;
      }
      default:
      {
         dataType = "unknown";
         break;
      }
      }
      outputString += indentation + "<bitDepth>" + ossimString::toString(bits).string() + "</bitDepth>" + separator;
      outputString += indentation + "<dataType>" + dataType.string() + "</dataType>" + separator;
   }

   void oms::DataInfo::appendGeometryInformation(ossimKeywordlist& kwl,
                                                 const ossimString& prefix) const
   {
      ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
      ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
      ossimGpt wgs84;
      ossimString groundGeometry;
      ossimString validGroundGeometry;
      ossimFilename omd;
      ossimString gsdUnit = "meters";
      if (thePrivateData && thePrivateData->theImageHandler){
         omd = thePrivateData->theImageHandler->createDefaultMetadataFilename();
      }
      if (geom.valid() && geom->getProjection())
      {
         ossimDpt gsd = geom->getMetersPerPixel();
         
         if(omd.exists()) //gsd overrides
         {
            ossimKeywordlist omdKwl(omd);
            ossimString gsdX = omdKwl.findKey("gsd_x");
            ossimString gsdY = omdKwl.findKey("gsd_y");
            ossimString gsdUnitOMD = omdKwl.findKey("gsd_unit");
            if( !gsdX.empty()  ){
               gsd.x=  ossimString::toDouble(gsdX);
            }
            if( !gsdY.empty() ){
               gsd.y=  ossimString::toDouble(gsdY);
            }
            if( !gsdUnit.empty() ){
               gsdUnit= gsdUnitOMD;
            }
         }
         ossimGpt ul;
         ossimGpt ur;
         ossimGpt lr;
         ossimGpt ll;
         geom->localToWorld(rect.ul(), ul);
         geom->localToWorld(rect.ur(), ur);
         geom->localToWorld(rect.lr(), lr);
         geom->localToWorld(rect.ll(), ll);

         if (ul.isLatNan() || ul.isLonNan() || ur.isLatNan() || ur.isLonNan() || lr.isLatNan() || lr.isLonNan() || ll.isLatNan() || ll.isLonNan())
         {
            return;
         }
         ul.changeDatum(wgs84.datum());
         ur.changeDatum(wgs84.datum());
         lr.changeDatum(wgs84.datum());
         ll.changeDatum(wgs84.datum());

         std::string polyString;
         //std::cout <<"----------------GETTING FOOTPRINT--------------------\n";

         if (getWktFootprint(geom.get(), polyString))
         {
            groundGeometry += polyString;
         }
         else
         {
            groundGeometry += ("MULTIPOLYGON(((" + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + "," + ossimString::toString(ur.lond()) + " " + ossimString::toString(ur.latd()) + "," + ossimString::toString(lr.lond()) + " " + ossimString::toString(lr.latd()) + "," + ossimString::toString(ll.lond()) + " " + ossimString::toString(ll.latd()) + "," + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + ")))");
         }
         std::ostringstream coordinates;
         std::ostringstream gcoords;
         kwl.add(prefix.c_str(), "gsd.@unit", gsdUnit);
         kwl.add(prefix.c_str(), "gsd.@dx", ossimString::toString(gsd.x, 20).c_str());
         kwl.add(prefix.c_str(), "gsd.@dy", ossimString::toString(gsd.y, 20).c_str());
         kwl.add(prefix.c_str(), "gsd.@mean", ossimString::toString((gsd.y+gsd.x)*0.5, 20).c_str());
         kwl.add(prefix.c_str(), "groundGeom.@srs", "epsg:4326");
         kwl.add(prefix.c_str(), "groundGeom", groundGeometry.c_str());
         kwl.add(prefix.c_str(), "TiePointSet.@version", "1");
         coordinates << ossimString::toString(rect.ul().x).string() << ","
                     << ossimString::toString(rect.ul().y).string() << " "
                     << ossimString::toString(rect.ur().x).string() << ","
                     << ossimString::toString(rect.ur().y).string() << " "
                     << ossimString::toString(rect.lr().x).string() << ","
                     << ossimString::toString(rect.lr().y).string() << " "
                     << ossimString::toString(rect.ll().x).string() << ","
                     << ossimString::toString(rect.ll().y).string();
         gcoords << ossimString::toString(ul.lond()).string() << ","
                 << ossimString::toString(ul.latd()).string() << " "
                 << ossimString::toString(ur.lond()).string() << ","
                 << ossimString::toString(ur.latd()).string() << " "
                 << ossimString::toString(lr.lond()).string() << ","
                 << ossimString::toString(lr.latd()).string() << " "
                 << ossimString::toString(ll.lond()).string() << ","
                 << ossimString::toString(ll.latd()).string();
         kwl.add(prefix.c_str(), "TiePointSet.Image.coordinates", coordinates.str().c_str());
         kwl.add(prefix.c_str(), "TiePointSet.Ground.coordinates", gcoords.str().c_str());
      }
   }
   void oms::DataInfo::appendGeometryInformation(std::string &outputString,
                                                 const std::string &indentation, const std::string &separator) const
   {
      ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
      ossimKeywordlist kwl;
      ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
      ossimGpt wgs84;
      ossimString groundGeometry;
      ossimString validGroundGeometry;

      if (geom.valid() && geom->getProjection())
      {
         ossimDpt gsd = geom->getMetersPerPixel();
         ossimGpt ul;
         ossimGpt ur;
         ossimGpt lr;
         ossimGpt ll;
         geom->localToWorld(rect.ul(), ul);
         geom->localToWorld(rect.ur(), ur);
         geom->localToWorld(rect.lr(), lr);
         geom->localToWorld(rect.ll(), ll);

         if (ul.isLatNan() || ul.isLonNan() || ur.isLatNan() || ur.isLonNan() || lr.isLatNan() || lr.isLonNan() || ll.isLatNan() || ll.isLonNan())
         {
            return;
         }
         ul.changeDatum(wgs84.datum());
         ur.changeDatum(wgs84.datum());
         lr.changeDatum(wgs84.datum());
         ll.changeDatum(wgs84.datum());

         std::string polyString;
         //std::cout <<"----------------GETTING FOOTPRINT--------------------\n";
         outputString += indentation + "<gsd unit=\"meters\" dx=\"" +
                         ossimString::toString(gsd.x, 15).string() + "\" dy=\"" +
                         ossimString::toString(gsd.y, 15).string() + "\"/>" + separator;

         if (getWktFootprint(geom.get(), polyString))
         {
            groundGeometry += polyString;
         }
         else
         {
            groundGeometry += ("MULTIPOLYGON(((" + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + "," + ossimString::toString(ur.lond()) + " " + ossimString::toString(ur.latd()) + "," + ossimString::toString(lr.lond()) + " " + ossimString::toString(lr.latd()) + "," + ossimString::toString(ll.lond()) + " " + ossimString::toString(ll.latd()) + "," + ossimString::toString(ul.lond()) + " " + ossimString::toString(ul.latd()) + ")))");
         }

         outputString += indentation + "<groundGeom srs=\"epsg:4326\">" + groundGeometry.string() + "</groundGeom>" + separator;

         outputString += indentation + "<TiePointSet version='1'>" + separator;
         outputString += indentation + "   " + "<Image>" + separator;
         outputString += indentation + "      " + "<coordinates>";
         outputString += ossimString::toString(rect.ul().x).string() + "," +
                         ossimString::toString(rect.ul().y).string() + " ";
         outputString += ossimString::toString(rect.ur().x).string() + "," +
                         ossimString::toString(rect.ur().y).string() + " ";
         outputString += ossimString::toString(rect.lr().x).string() + "," +
                         ossimString::toString(rect.lr().y).string() + " ";
         outputString += ossimString::toString(rect.ll().x).string() + "," +
                         ossimString::toString(rect.ll().y).string();
         outputString += "</coordinates>" + separator;
         outputString += indentation + "   " + "</Image>" + separator;
         outputString += indentation + "   " + "<Ground>" + separator;
         outputString += indentation + "      " + "<coordinates>";
         outputString += ossimString::toString(ul.lond()).string() + "," +
                         ossimString::toString(ul.latd()).string() + " ";
         outputString += ossimString::toString(ur.lond()).string() + "," +
                         ossimString::toString(ur.latd()).string() + " ";
         outputString += ossimString::toString(lr.lond()).string() + "," +
                         ossimString::toString(lr.latd()).string() + " ";
         outputString += ossimString::toString(ll.lond()).string() + "," +
                         ossimString::toString(ll.latd()).string();
         outputString += "</coordinates>" + separator;
         outputString += indentation + "   " + "</Ground>" + separator;
         outputString += indentation + "</TiePointSet>" + separator;
      }
   }
   void appendMetadataTag(ossimRefPtr<ossimProperty> property,
                          std::string &outputString, 
                          const std::string &indentation,
                          const std::string &separator)
   {
      if (property.valid())
      {
         ossimContainerProperty *container =
             dynamic_cast<ossimContainerProperty *>(property.get());
         if (container)
         {
            ossim_uint32 idx = 0;
            ossim_uint32 n = container->getNumberOfProperties();
            if (container->getName() == "tags")
            {
               for (idx = 0; idx < n; ++idx)
               {
                  ossimRefPtr<ossimProperty> prop = container->getProperty(
                      idx);
                  if (prop.valid())
                  {
                     appendMetadataTag(prop.get(), outputString,
                                       indentation, separator);
                  }
               }
            }
            else
            {
               ossimString containerName = container->getName();
               ossimString newIndentation = indentation + "   ";

               outputString += indentation + "<tag name=\"" + containerName.string() + "\">" + separator;

               ossimKeywordlist coeffs;

               for (idx = 0; idx < n; ++idx)
               {
                  ossimRefPtr<ossimProperty> prop = container->getProperty(idx);
                  if (prop.valid())
                  {
                     ossimString childName = prop->getName();

                     // Add some special handling for NITF coefficients
                     // Consolidate into a single tag
                     if (containerName == "RPC00B" && childName.match(
                                                          ".*COEFF.*") != "")
                     {
                        coeffs.add(childName, prop->valueToString());
                     }
                     else
                     {
                        appendMetadataTag(prop.get(), outputString,
                                          newIndentation, separator);
                     }
                  }
               }

               //  Add the consolidated coeffs, if they exist
               if (coeffs.getSize() > 0)
               {
                  std::vector<ossimString> namedSet;

                  namedSet.push_back("LINE_DEN");
                  namedSet.push_back("LINE_NUM");
                  namedSet.push_back("SAMP_DEN");
                  namedSet.push_back("SAMP_NUM");

                  for (int i = 0, size = namedSet.size(); i < size; i++)
                  {
                     outputString += newIndentation.string();
                     outputString += "<tag name=\"" + namedSet[i].string() + "\">";

                     std::vector<ossimString> namedCoeffs =
                         coeffs.findAllKeysThatContains(namedSet[i]);

                     for (int j = 0, size = namedCoeffs.size(); j < size; j++)
                     {
                        outputString += namedCoeffs[j].afterRegExp(namedSet[i] + "_COEFF_").string();

                        outputString += "=";
                        outputString += coeffs.find(namedCoeffs[j]);

                        if (j < size - 1)
                           outputString += ", ";
                     }

                     outputString += "</tag>";
                     outputString += separator;
                  }
               }

               outputString += indentation + "</tag>" + separator;
            }
         }
         else
         {
            ossimString value = property->valueToString().trim();

            // Only add the tag if it has a value
            if (!value.empty())
            {
               outputString += indentation + "<tag name=\"" + property->getName().string() + "\">" + value.string() + "</tag>" + separator;
            }
         }
      }
   }

   void appendMetadataTag(ossimRefPtr<ossimProperty> property,
                          ossimKeywordlist& kwl, 
                          const ossimString& prefix)
   {
      #if 0
      if (property.valid())
      {
         ossimContainerProperty *container =
             dynamic_cast<ossimContainerProperty *>(property.get());
         if (container)
         {
            ossim_uint32 idx = 0;
            ossim_uint32 n = container->getNumberOfProperties();
            if (container->getName() == "tags")
            {
               ossimString newPrefix = prefix + "tags.";
               for (idx = 0; idx < n; ++idx)
               {
                  ossimRefPtr<ossimProperty> prop = container->getProperty(
                      idx);
                  if (prop.valid())
                  {
                     appendMetadataTag(prop.get(), kwl, newPrefix);
                  }
               }
            }
            else
            {
               ossimString containerName = container->getName();
               ossimString newPrefix = prefix + containerName + ".";

               ossimKeywordlist coeffs;

               for (idx = 0; idx < n; ++idx)
               {
                  ossimRefPtr<ossimProperty> prop = container->getProperty(idx);
                  if (prop.valid())
                  {
                     ossimString childName = prop->getName();

                     // Add some special handling for NITF coefficients
                     // Consolidate into a single tag
                     if (containerName == "RPC00B" && childName.match(
                                                          ".*COEFF.*") != "")
                     {
                        coeffs.add(childName, prop->valueToString());
                     }
                     else
                     {
                        appendMetadataTag(prop.get(), kwl, newPrefix);
                     }
                  }
               }

               //  Add the consolidated coeffs, if they exist
               if (coeffs.getSize() > 0)
               {
                  std::vector<ossimString> namedSet;

                  namedSet.push_back("LINE_DEN");
                  namedSet.push_back("LINE_NUM");
                  namedSet.push_back("SAMP_DEN");
                  namedSet.push_back("SAMP_NUM");

                  for (int i = 0, size = namedSet.size(); i < size; i++)
                  {
                     ossimString newPrefix = prefix + namedSet[i];
                     // outputString += newIndentation.string();
                     // outputString += "<tag name=\"" + namedSet[i].string() + "\">";

                     std::vector<ossimString> namedCoeffs =
                         coeffs.findAllKeysThatContains(namedSet[i]);

                     for (int j = 0, size = namedCoeffs.size(); j < size; j++)
                     {
                        outputString += namedCoeffs[j].afterRegExp(namedSet[i] + "_COEFF_").string();

                        outputString += "=";
                        outputString += coeffs.find(namedCoeffs[j]);

                        if (j < size - 1)
                           outputString += ", ";
                     }

                     outputString += "</tag>";
                     outputString += separator;
                  }
               }

               outputString += indentation + "</tag>" + separator;
            }
         }
         else
         {
            ossimString value = property->valueToString().trim();

            // Only add the tag if it has a value
            if (!value.empty())
            {
               outputString += indentation + "<tag name=\"" + property->getName().string() + "\">" + value.string() + "</tag>" + separator;
            }
         }
      }
      #endif
   }

/*
void oms::DataInfo::appendRasterDataSetMetadata(ossimKeywordlist& kwl,
                                                const ossimString& prefix) const
{
   // #if 0
   std::vector<ossimRefPtr<ossimProperty>> properties;
   thePrivateData->theImageHandler->getPropertyList(properties);
   if (properties.size())
   {
      ossim_uint32 idx = 0;
      bool found = false;
      for (idx = 0; (idx < properties.size())&&!found; ++idx)
      {
         if (properties[idx]->getName() == "file_header")
         {
            found = true;
            ossimString newPrefix = prefix + "metadata.";
            ossim_uint32 idx2 = 0;
            ossimContainerProperty
                *container =
                    dynamic_cast<ossimContainerProperty *>(properties[idx].get());
            if (container)
            {
               ossim_uint32 n = container->getNumberOfProperties();
               for (idx2 = 0; idx2 < n; ++idx2)
               {
                  ossimString propPrefix = newPrefix + "property" + ossimString::toString(idx2) + ".";
                  ossimRefPtr<ossimProperty> prop =
                      container->getProperty(idx2);
                  if (prop.valid())
                  {
                     appendMetadataTag(prop.get(), kwl, propPrefix);
                  }
               }
            }
         }
      }
   }
}
*/
void oms::DataInfo::appendRasterDataSetMetadata(std::string &outputString,
                                                const std::string &indentation,
                                                const std::string &separator) const
{
   std::vector<ossimRefPtr<ossimProperty>> properties;
   thePrivateData->theImageHandler->getPropertyList(properties);
   if (properties.size())
   {
      ossim_uint32 idx = 0;
      for (idx = 0; idx < properties.size(); ++idx)
      {
         if (properties[idx]->getName() == "file_header")
         {
            outputString += indentation + "<metadata>" + separator;
            ossim_uint32 idx2 = 0;
            ossimContainerProperty
                *container =
                    dynamic_cast<ossimContainerProperty *>(properties[idx].get());
            if (container)
            {
               ossim_uint32 n = container->getNumberOfProperties();
               for (idx2 = 0; idx2 < n; ++idx2)
               {
                  ossimRefPtr<ossimProperty> prop =
                      container->getProperty(idx2);
                  if (prop.valid())
                  {
                     appendMetadataTag(prop.get(), outputString,
                                       indentation + "   ", separator);
                  }
               }
            }
            outputString += indentation + "</metadata>" + separator;
         }
      }
   }
}

void oms::DataInfo::appendVideoDataSetMetadata(ossimKeywordlist& kwl,
                                               const ossimString& prefix) const
{
#ifdef OSSIM_VIDEO_ENABLED
   ossimString newPrefix = prefix + "metadata.";
   if (thePrivateData->thePredatorVideoFrameInfo.valid() &&
       thePrivateData->thePredatorVideoFrameInfo->klvTable())
   {
      ossimRefPtr<ossimPredatorKlvTable> klvTable = thePrivateData->thePredatorVideoFrameInfo->klvTable();
      ossimString value;
      ossimString securityClassification;
      //      ossimString grazingAngle;
      //      ossimString azimuthAngle;
      ossimString missionNumber;
      //outputString += indentation + "<metadata>" + separator;
      if (klvTable->valueAsString(value, KLV_KEY_ORGANIZATIONAL_PROGRAM_NUMBER))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "organizationalProgramNumber", blankOutBinary(value).trim().c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_SECURITY_CLASSIFICATION))
      {
         value = value.trim();
         if (!value.empty())
         {
            value = value.downcase();
            if (value.contains("unclas"))
            {
               securityClassification = "UNCLASSIFIED";
            }
            else if (value.contains("top"))
            {
               securityClassification = "TOP SECRET";
            }
            else if (value.contains("secret"))
            {
               securityClassification = "SECRET";
            }
            else
            {
               securityClassification = value;
            }
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_SECURITY_RELEASE_INSTRUCTIONS))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "releaseInstructions", value.c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_SECURITY_CAVEATS))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "securityCaveats", value.c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_CLASSIFICATION_COMMENT))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "classificationComment", value.c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_ORIGINAL_PRODUCER_NAME))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "originalProducerName", value.c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_IMAGE_SOURCE_SENSOR))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "imageSourceSensor", value.c_str());
         }
      }
      if (klvTable->valueAsString(value, KLV_KEY_PLATFORM_DESIGNATION))
      {
         value = value.trim();
         if (!value.empty())
         {
            kwl.add(newPrefix.c_str(), "platformDesignation", value.c_str());
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_INDICATED_AIR_SPEED))
      {
         value = value.trim();
         if(!value.empty())
         {
            kwl.add(newPrefix.c_str(), "indicatedAirSpeed", value.c_str());
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_STATIC_PRESSURE))
      {
         value = value.trim();
         if(!value.empty())
         {
            kwl.add(newPrefix.c_str(), "staticAirPressure", value.c_str());
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_PLATFORM_GROUND_SPEED))
      {
         value = value.trim();
         if(!value.empty())
         {
            kwl.add(newPrefix.c_str(), "groundSpeed", value.c_str());
         }
      }
      //      if(!klvTable->valueAsString(azimuthAngle, KLV_KEY_DEVICE_ABSOLUTE_HEADING))
      //      {
      //         if(!klvTable->valueAsString(azimuthAngle, KLV_KEY_ANGLE_TO_NORTH))
      //         {
      //            klvTable->valueAsString(azimuthAngle, KLV_KEY_PLATFORM_HEADING_ANGLE);
      //         }
      //      }
      //      if(klvTable->valueAsString(grazingAngle,KLV_KEY_OBLIQUITY_ANGLE))
      //      {
      //         grazingAngle = ossimString::toString(90.0 - grazingAngle.toDouble());
      //      }
      klvTable->valueAsString(missionNumber, KLV_KEY_MISSION_NUMBER);

      kwl.add(newPrefix.c_str(), "securityClassification", securityClassification.c_str());
      kwl.add(newPrefix.c_str(), "missionId", missionNumber.c_str());
      kwl.add(newPrefix.c_str(), "fileType", thePrivateData->formatName().c_str());
      kwl.add(newPrefix.c_str(), "filename", thePrivateData->theFilename.c_str());
      // outputString += indentation + "   <securityClassification>" + securityClassification.string() + "</securityClassification>" + separator;
      //      outputString += indentation + "   <azimuthAngle>" + azimuthAngle + "</azimuthAngle>" + separator;
      //      outputString += indentation + "   <grazingAngle>" + grazingAngle + "</grazingAngle>" + separator;
      // outputString += indentation + "   <missionId>" + missionNumber.string() + "</missionId>" +
      //                 separator;
      // outputString += indentation + "   <fileType>" + thePrivateData->formatName() +
      //                 "</fileType>" + separator;
      // outputString += indentation + "   <filename>" + thePrivateData->theFilename.string() +
      //                 "</filename>" + separator;

#if 0
      outputString += indentation + "   <imageId>" + imageId + "</imageId>" + separator;
      outputString += indentation + "   <sensorId>" + sensorId + "</sensorId>" + separator;
      outputString += indentation + "   <countryCode>" + countryCode + "</countryCode>" + separator;
      outputString += indentation + "   <imageCategory>" + imageCategory + "</imageCategory>" + separator;
      outputString += indentation + "   <grazingAngle>" + grazingAngle + "</grazingAngle>" + separator;
      outputString += indentation + "   <title>" + title + "</title>" + separator;
      outputString += indentation + "   <organization>" + organization + "</organization>" + separator;
      outputString += indentation + "   <description>" + description + "</description>" + separator;
      outputString += indentation + "   <niirs>" + niirs + "</niirs>" + separator;
      outputString += indentation + "   <fileType>" + thePrivateData->formatName() + "</fileType>" + separator;
      outputString += indentation + "   <className>" + (thePrivateData->theImageHandler.valid()?thePrivateData->theImageHandler->getClassName():ossimString("")) + "</className>" + separator;
#endif
      // outputString += indentation + "</metadata>" + separator;
   }
#endif
}

void oms::DataInfo::appendVideoDataSetMetadata(std::string & outputString,
                                                  const std::string &indentation, const std::string &separator) const
{
#ifdef OSSIM_VIDEO_ENABLED
   if(thePrivateData->thePredatorVideoFrameInfo.valid()&&
      thePrivateData->thePredatorVideoFrameInfo->klvTable())
   {
      ossimRefPtr<ossimPredatorKlvTable> klvTable = thePrivateData->thePredatorVideoFrameInfo->klvTable();
      ossimString value;
      ossimString securityClassification;
      //      ossimString grazingAngle;
      //      ossimString azimuthAngle;
      ossimString missionNumber;
      outputString += indentation + "<metadata>" + separator;
      if(klvTable->valueAsString(value, KLV_KEY_ORGANIZATIONAL_PROGRAM_NUMBER))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <organizationalProgramNumber>" +
               blankOutBinary(value).trim().string() + "</organizationalProgramNumber>" +
               separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_SECURITY_CLASSIFICATION))
      {
         value = value.trim();
         if(!value.empty())
         {
            value = value.downcase();
            if(value.contains("unclas"))
            {
               securityClassification = "UNCLASSIFIED";
            }
            else if(value.contains("top"))
            {
               securityClassification = "TOP SECRET";
            }
            else if(value.contains("secret"))
            {
               securityClassification = "SECRET";
            }
            else
            {
               securityClassification = value;
            }
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_SECURITY_RELEASE_INSTRUCTIONS))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <releaseInstructions>" + value.string() +
               "</releaseInstructions>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_SECURITY_CAVEATS))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <securityCaveats>" + value.string() +
               "</securityCaveats>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_CLASSIFICATION_COMMENT))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <classificationComment>" + value.string() + "</classificationComment>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_ORIGINAL_PRODUCER_NAME))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <originalProducerName>" + value.string() + "</originalProducerName>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_IMAGE_SOURCE_SENSOR))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <imageSourceSensor>" + value.string() + "</imageSourceSensor>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_PLATFORM_DESIGNATION))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <platformDesignation>" + value.string() + "</platformDesignation>" + separator;
         }
      }
/*
      if(klvTable->valueAsString(value, KLV_KEY_INDICATED_AIR_SPEED))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <indicatedAirSpeed>" + value.string() + "</indicatedAirSpeed>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_STATIC_PRESSURE))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <staticAirPressure>" + value.string() + "</staticAirPressure>" + separator;
         }
      }
      if(klvTable->valueAsString(value, KLV_KEY_PLATFORM_GROUND_SPEED))
      {
         value = value.trim();
         if(!value.empty())
         {
            outputString += indentation + "   <groundSpeed>" + value.string() + "</groundSpeed>" + separator;
         }
      }
*/
      //      if(!klvTable->valueAsString(azimuthAngle, KLV_KEY_DEVICE_ABSOLUTE_HEADING))
      //      {
      //         if(!klvTable->valueAsString(azimuthAngle, KLV_KEY_ANGLE_TO_NORTH))
      //         {
      //            klvTable->valueAsString(azimuthAngle, KLV_KEY_PLATFORM_HEADING_ANGLE);
      //         }
      //      }
      //      if(klvTable->valueAsString(grazingAngle,KLV_KEY_OBLIQUITY_ANGLE))
      //      {
      //         grazingAngle = ossimString::toString(90.0 - grazingAngle.toDouble());
      //      }
      klvTable->valueAsString(missionNumber, KLV_KEY_MISSION_NUMBER);

      outputString += indentation + "   <securityClassification>" + securityClassification.string()
         + "</securityClassification>" + separator;
      //      outputString += indentation + "   <azimuthAngle>" + azimuthAngle + "</azimuthAngle>" + separator;
      //      outputString += indentation + "   <grazingAngle>" + grazingAngle + "</grazingAngle>" + separator;
      outputString += indentation + "   <missionId>" + missionNumber.string() + "</missionId>" +
         separator;
      outputString += indentation + "   <fileType>" + thePrivateData->formatName() +
         "</fileType>" + separator;
      outputString += indentation + "   <filename>" + thePrivateData->theFilename.string() +
         "</filename>" + separator;

#if 0
      outputString += indentation + "   <imageId>" + imageId + "</imageId>" + separator;
      outputString += indentation + "   <sensorId>" + sensorId + "</sensorId>" + separator;
      outputString += indentation + "   <countryCode>" + countryCode + "</countryCode>" + separator;
      outputString += indentation + "   <imageCategory>" + imageCategory + "</imageCategory>" + separator;
      outputString += indentation + "   <grazingAngle>" + grazingAngle + "</grazingAngle>" + separator;
      outputString += indentation + "   <title>" + title + "</title>" + separator;
      outputString += indentation + "   <organization>" + organization + "</organization>" + separator;
      outputString += indentation + "   <description>" + description + "</description>" + separator;
      outputString += indentation + "   <niirs>" + niirs + "</niirs>" + separator;
      outputString += indentation + "   <fileType>" + thePrivateData->formatName() + "</fileType>" + separator;
      outputString += indentation + "   <className>" + (thePrivateData->theImageHandler.valid()?thePrivateData->theImageHandler->getClassName():ossimString("")) + "</className>" + separator;
#endif
      outputString += indentation + "</metadata>" + separator;
   }
#endif
}

static ossimString monthStringToNumberString(const ossimString& monthString)
{
   if (monthString == "jan")
   {
      return "01";
   }
   else if (monthString == "feb")
   {
      return "02";
   }
   else if (monthString == "mar")
   {
      return "03";
   }
   else if (monthString == "apr")
   {
      return "04";
   }
   else if (monthString == "may")
   {
      return "05";
   }
   else if (monthString == "jun")
   {
      return "06";
   }
   else if (monthString == "jul")
   {
      return "07";
   }
   else if (monthString == "aug")
   {
      return "08";
   }
   else if (monthString == "sep")
   {
      return "09";
   }
   else if (monthString == "oct")
   {
      return "10";
   }
   else if (monthString == "nov")
   {
      return "11";
   }
   else if (monthString == "dec")
   {
      return "12";
   }
   return "";
}

std::string oms::DataInfo::convertIdatimToXmlDate(const std::string& idatim) const
{
   ossimString result = "";

   if (idatim.size() == 14)
   {
      if (idatim[8] == 'Z')
      {

         ossimString day(idatim.begin(), idatim.begin() + 2);
         ossimString hour(idatim.begin() + 2, idatim.begin() + 4);
         ossimString m(idatim.begin() + 4, idatim.begin() + 6);
         ossimString sec(idatim.begin() + 6, idatim.begin() + 8);
         // skip one character for Zulu "Z" so go to index 9
         ossimString month(idatim.begin() + 9, idatim.begin() + 12);
         ossimString year(idatim.begin() + 12, idatim.begin() + 14);
         month = monthStringToNumberString(month.downcase());

         if (year.toUInt32() > 60)
         {
            result += "19";
         }
         else
         {
            result += "20";
         }
         result += year + "-";
         result += month + "-";
         result += day + "T";
         result += hour + ":";
         result += m + ":";
         result += sec + "Z";
      }
      else
      {
         ossimString year(idatim.begin(), idatim.begin() + 4);
         ossimString month(idatim.begin() + 4, idatim.begin() + 6);
         ossimString day(idatim.begin() + 6, idatim.begin() + 8);
         ossimString hour(idatim.begin() + 8, idatim.begin() + 10);
         ossimString m(idatim.begin() + 10, idatim.begin() + 12);
         ossimString sec(idatim.begin() + 12, idatim.begin() + 14);
         result += year + "-";
         result += month + "-";
         result += day + "T";
         result += hour + ":";
         result += m + ":";
         result += sec + "Z";
      }
   }

   return result;
}

std::string oms::DataInfo::convertAcqDateToXmlDate(const std::string& value) const
{
   std::string result = "";
   if (value.size() == 8) // assume 4 character year 2 month and 2 day format
   {
      result = ossimString( ossimString(value.begin(), value.begin() + 4) + "-"
                            + ossimString(value.begin() + 4, value.begin() + 6) + "-"
                            + ossimString(value.begin() + 6, value.begin() + 8) ).string();
   }
   if (value.size() == 14) // assume 4 character year 2 month and 2 day 2 hour 2 minute 2 seconds
   {
      result = ossimString( ossimString(value.begin(), value.begin() + 4) + "-"
                            + ossimString(value.begin() + 4, value.begin() + 6) + "-"
                            + ossimString(value.begin() + 6, value.begin() + 8) + "T"
                            + ossimString(value.begin() + 8, value.begin() + 10) + ":"
                            + ossimString(value.begin() + 10, value.begin() + 12) + ":"
                            + ossimString(value.begin() + 12, value.begin() + 14) + "Z" ).string();
   }
   return result;
}

void oms::DataInfo::appendRasterEntryDateTime(std::string &outputString,
                                              const std::string &indentation,
                                              const std::string &separator) const
{
   ossimRefPtr<ossimProperty> prop;
   ossimString dateValue =
       convertIdatimToXmlDate(
           thePrivateData->theImageHandler->getPropertyValueAsString(
               "IDATIM"));

   if (dateValue.empty())
   {
      dateValue = convertAcqDateToXmlDate(
          thePrivateData->theImageHandler->getPropertyValueAsString(
              "acquisition_date"));
   }
   // this part is better done in JAVA land and not here.  I will take this out for now
#if 0
   if(dateValue.empty()&&thePrivateData->theImageHandler.valid())
   {
      ossimString filename = thePrivateData->theImageHandler->getFilename();
      filename = filename.upcase();
      ossimRegExp regexp1("[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]");
      ossimRegExp regexp2("[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][T][0-9][0-9][0-9][0-9][0-9][0-9]");
      ossimRegExp regexp3("[0-9][0-9]-(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)-[0-9][0-9][0-9][0-9]");
      ossimRegExp regexp4("[0-9][0-9][0-9][0-9]-(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)-[0-9][0-9]");

     // lets try a pattern in the file name
      //
      ossimString month, day, year, hours="00", minutes="00", seconds="00";

      if(regexp1.find(filename.c_str()))
      {
         ossimString value(regexp1.match(0));
         year = value.substr(0, 4);
         month = value.substr(4, 2);
         day = value.substr(6, 2);
         hours = value.substr(8, 2);
         minutes = value.substr(10, 2);
         seconds = value.substr(12);
      }
      else if(regexp2.find(filename.c_str()))
      {
         ossimString value(regexp2.match(0));
         year = value.substr(0, 4);
         month = value.substr(4, 2);
         day = value.substr(6, 2);
         hours = value.substr(9, 2);
         minutes = value.substr(11, 2);
         seconds = value.substr(13);
      }
      else if(regexp3.find(filename.c_str()))
      {
         std::vector<ossimString> splitArray;
         ossimString value(regexp3.match(0));
         value.split(splitArray, "-");
         if(splitArray.size() >2)
         {
            month = monthToNumericString(splitArray[1]);
            day   = splitArray[0];
            year  = splitArray[2];
         }

      }
      else if(regexp4.find(filename.c_str()))
      {
         std::vector<ossimString> splitArray;
         ossimString value(regexp4.match(0));
         value.split(splitArray, "-");
         if(splitArray.size() >2)
         {
            month = monthToNumericString(splitArray[1]);
            day   = splitArray[2];
            year  = splitArray[0];
         }
      }

      if(!month.empty()&&!day.empty()&&!year.empty())
      {
         dateValue = year+"-"+month+"-"+day+"T" + hours + ":" + minutes + ":" + seconds +"Z";
      }
   }
#endif
   if (!dateValue.empty())
   {
      outputString += indentation + "<TimeStamp>" + separator;
      outputString += indentation + "   <when>" + dateValue.string() + "</when>" + separator;
      outputString += indentation + "</TimeStamp>" + separator;
   }
}

void oms::DataInfo::appendRasterEntryDateTime(ossimKeywordlist& kwl,
                                              const ossimString& prefix) const
{
   ossimRefPtr<ossimProperty> prop;
   ossimString dateValue =
       convertIdatimToXmlDate(
           thePrivateData->theImageHandler->getPropertyValueAsString(
               "IDATIM"));

   if (dateValue.empty())
   {
      dateValue = convertAcqDateToXmlDate(
          thePrivateData->theImageHandler->getPropertyValueAsString(
              "acquisition_date"));
   }
   // this part is better done in JAVA land and not here.  I will take this out for now
   if (!dateValue.empty())
   {
      ossimString newPrefix = prefix + "TimeStamp.";
      kwl.add(newPrefix, "when", dateValue.c_str());
   }
}

void oms::DataInfo::appendRasterEntryMetadata(std::string &outputString,
                                              const std::string &indentation,
                                              const std::string &separator) const
{
   ossimString dateValue = "";
   outputString += indentation + "<metadata>" + separator;

   std::shared_ptr<ossimInfoBase> info = ossimInfoFactoryRegistry::instance()->create(thePrivateData->theFilename);
   if (info)
   {
      ossimKeywordlist kwl;
      ossimKeywordlist kwl2;
      ossimKeywordlist tempDefaultKwl;
      ossimKeywordlist defaultKwl;
      // ossimXmlOutputKeywordList kwl3;
      ossimKeywordlist kwl3;
      ossimRefPtr<ossimImageHandler> imageHandler = thePrivateData->theImageHandler;
      ossim_uint32 entryId = imageHandler->getCurrentEntry();

      //info->print(std::cout);
      //      kwl.removeKeysThatMatch("[^.*image"+ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) +
      //                              "]");
      info->getKeywordlist(kwl);

      replaceSpacesInKeys(kwl);
      // std::cout << "_________________________\n";
      // std::cout << kwl << "\n";
      // std::cout << "_________________________\n";
      kwl3.getMap() = kwl.getMap();
      kwl3.removeKeysThatMatch(".*\\.image.*\\..*");
      defaultKwl.getMap() = kwl3.getMap();
      kwl.extractKeysThatMatch(kwl2, ".*\\.image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) + "\\..*");
      kwl.extractKeysThatMatch(defaultKwl, ossimString(".*\\.image0") + "\\..*");
      ossimKeywordlist::KeywordMap::iterator iter = kwl2.getMap().begin();
      while (iter != kwl2.getMap().end())
      {
         ossimString k = iter->first;
         k = k.substitute(".image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()), "");
         kwl3.getMap().insert(make_pair(k.string(), iter->second));
         ++iter;
      }
      iter = tempDefaultKwl.getMap().begin();
      while (iter != tempDefaultKwl.getMap().end())
      {
         ossimString k = iter->first;
         k = k.substitute(".image0", "");
         defaultKwl.getMap().insert(make_pair(k.string(), iter->second));
         ++iter;
      }

      if (kwl3.getSize() > 0)
      {

         ossimString imageId;
         ossimString beNumber;
         ossimString cloudCover;
         ossimString imageRepresentation;
         ossimString isorce;
         ossimString offNadirAngle;
         ossimString targetId;
         ossimString productId;
         ossimString sensorId;
         ossimString missionId;
         ossimString countryCode;
         ossimString imageCategory;
         ossimString azimuthAngle;
         ossimString grazingAngle;
         ossimString securityClassification;
         ossimString securityCode;
         ossimString stripId;
         ossimString sunElevation;
         ossimString sunAzimuth;
         ossimString title;
         ossimString organization;
         ossimString description;
         ossimString niirs;

         // Open the dot omd file if present as it can now have metadata in it.
         ossimFilename omdFile;
         getOmdFile(omdFile);
         if (omdFile.exists())
         {
            kwl3.addFile(omdFile);
         }

         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         bool validModel = false;
         if (geom)
         {
            validModel = geom->isAffectedByElevation();
         }

         // This downcases only the keys, not the values.
         kwl.downcaseKeywords();
         kwl3.downcaseKeywords();
         defaultKwl.downcaseKeywords();
         // Azimuth angle:
         getAzimuthAngle(kwl3, azimuthAngle.string());

         // BE number:
         getBeNumber(kwl3, beNumber.string());

         // Cloud cover:
         getCloudCover(kwl3, cloudCover.string());

         // Country code:
         getCountryCode(kwl3, countryCode.string());
         if (countryCode.empty() && entryId > 0)
         {
            getCountryCode(defaultKwl, countryCode.string());
         }

         // Date:
         getDate(kwl3, dateValue.string());
         if (dateValue.empty() && entryId > 0)
         {
            getDate(defaultKwl, dateValue.string());
         }

         // Description:
         getDescription(kwl3, description.string());

         // Grazing angle:
         getGrazingAngle(kwl3, grazingAngle.string());

         // Image ID:
         getImageId(kwl3, imageId.string());

         // Image Category:
         getImageCategory(kwl3, imageCategory.string());

         // Image representation:
         getImageRepresentation(kwl3, imageRepresentation.string());

         // ISOURCE:
         getIsorce(kwl3, isorce.string());

         // Mission:
         getMissionId(kwl3, missionId.string());
         if (missionId.empty() && entryId > 0)
         {
            getMissionId(defaultKwl, missionId.string());
         }

         // NIIRS:
         getNiirs(kwl3, niirs.string());
         if (niirs.empty() && entryId > 0)
         {
            getNiirs(defaultKwl, niirs.string());
         }

         getOffNadirAngle(kwl3, offNadirAngle.string());

         // Organization:
         getOrganization(kwl3, organization.string());

         // Product:
         getProductId(kwl3, productId.string());

         // Security classification:
         getSecurityClassification(kwl3, securityClassification.string());
         if (securityClassification.empty() && entryId > 0)
         {
            getSecurityClassification(defaultKwl, securityClassification.string());
         }

         // Security code
         getSecurityCode(kwl3, securityCode.string());
         if (securityCode.empty() && entryId > 0)
         {
            getSecurityCode(defaultKwl, securityCode.string());
         }
         // Sensor:
         getSensorId(kwl3, sensorId.string());
         if (sensorId.empty() && entryId > 0)
         {
            getSensorId(defaultKwl, sensorId.string());
         }

         // Strip ID:
         getStripId(kwl3, stripId.string());

         // Sun azimuth:
         getSunAzimuth(kwl3, sunAzimuth.string());

         // Sun elevation:
         getSunElevation(kwl3, sunElevation.string());

         // Target ID:
         getTargetId(kwl3, targetId.string());

         // Title:
         getTitle(kwl3, title.string());

         outputString += indentation + "   <filename>" + ossimXmlString::wrapCDataIfNeeded(thePrivateData->theFilename.string()).string() +
                         "</filename>" + separator;
         outputString += indentation + "   <imageId>" + ossimXmlString::wrapCDataIfNeeded(imageId.string()).string() + "</imageId>" +
                         separator;
         outputString += indentation + "   <imageRepresentation>" + imageRepresentation.string() +
                         "</imageRepresentation>" + separator;

         if (isorce.size())
         {
            outputString += indentation + "   <isorce>" + isorce.string() + "</isorce>" +
                            separator;
         }

         outputString += indentation + "   <targetId>" + targetId.string() + "</targetId>" +
                         separator;
         outputString += indentation + "   <productId>" + productId.string() + "</productId>" +
                         separator;
         outputString += indentation + "   <beNumber>" + beNumber.string() + "</beNumber>" +
                         separator;
         outputString += indentation + "   <cloudCover>" + cloudCover.string() + "</cloudCover>" +
                         separator;
         outputString += indentation + "   <sensorId>" + sensorId.string() + "</sensorId>" +
                         separator;
         outputString += indentation + "   <missionId>" + missionId.string() + "</missionId>" +
                         separator;
         outputString += indentation + "   <countryCode>" + countryCode.string() +
                         "</countryCode>" + separator;
         outputString += indentation + "   <imageCategory>" + imageCategory.string() +
                         "</imageCategory>" + separator;
         outputString += indentation + "   <azimuthAngle>" + azimuthAngle.string() +
                         "</azimuthAngle>" + separator;
         outputString += indentation + "   <grazingAngle>" + grazingAngle.string() +
                         "</grazingAngle>" + separator;
         outputString += indentation + "   <offNadirAngle>" + offNadirAngle.string() +
                         "</offNadirAngle>" + separator;
         outputString += indentation + "   <securityClassification>" +
                         securityClassification.string() + "</securityClassification>" + separator;
         outputString += indentation + "   <securityCode>" +
                         securityCode.string() + "</securityCode>" + separator;
         outputString += indentation + "   <title>" + title.string() + "</title>" + separator;
         outputString += indentation + "   <organization>" + organization.string() +
                         "</organization>" + separator;
         outputString += indentation + "   <description>" + description.string() +
                         "</description>" + separator;
         outputString += indentation + "   <niirs>" + niirs.string() + "</niirs>" + separator;
         outputString += indentation + "   <stripId>" + stripId.string() +
                         "</stripId>" + separator;
         outputString += indentation + "   <sunAzimuth>" + sunAzimuth.string() +
                         "</sunAzimuth>" + separator;
         outputString += indentation + "   <sunElevation>" + sunElevation.string() +
                         "</sunElevation>" + separator;
         outputString += indentation + "   <validModel>" + ((validModel) ? "true" : "false") +
                         "</validModel>" + separator;

         std::ostringstream out;
         //out << kwl3;
         //outputString += out.str() + separator;

         std::shared_ptr<ossim::KwlNodeFormatter> formatter =
             std::make_shared<ossim::KwlNodeXmlFormatter>(kwl3);
         // std::cout << " ----------------------------------------- \n";
         formatter->write(out,
                          ossim::KwlNodeFormatter::FormatHints(3, ossim::KwlNodeFormatter::FormatHints::FORMAT_HINTS_PRETTY_PRINT_FLAG));
         outputString += out.str() + separator;

         // std::cout << " ----------------------------------------- \n";
      }
   }
   else
   {
      ossimString azimuthAngle = "0";
      if (thePrivateData->theImageHandler.valid())
      {
         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         ossimDrect rect = thePrivateData->theImageHandler->getBoundingRect();
         if (geom.valid())
         {
            ossimGpt centerGpt;
            ossimGpt upGpt;
            ossimDpt centerDpt(rect.midPoint());
            ossimDpt upDpt = centerDpt + ossimDpt(0, -rect.height() * .5);
            geom->localToWorld(centerDpt, centerGpt);
            geom->localToWorld(upDpt, upGpt);
            azimuthAngle = ossimString::toString(centerGpt.azimuthTo(upGpt));
         }
      }
      outputString += indentation + "   <azimuthAngle>" + azimuthAngle.string() + "</azimuthAngle>" + separator;
   }
   ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
   if (geom.valid())
   {
      bool crossesDateline = geom->getCrossesDateline();
      outputString += indentation + "   <crossesDateline>" + (crossesDateline ? "true" : "false") + "</crossesDateline>" + separator;
   }
   outputString += indentation + "   <fileType>" + thePrivateData->formatName() + "</fileType>" + separator;
   outputString += indentation + "   <className>" + (thePrivateData->theImageHandler.valid() ? thePrivateData->theImageHandler->getClassName().string() : ossimString("").string()) + "</className>" + separator;
   outputString += indentation + "</metadata>" + separator;

   if (!dateValue.empty())
   {
      outputString += indentation + "<TimeStamp>" + separator;
      outputString += indentation + "   <when>" + dateValue.string() + "</when>" + separator;
      outputString += indentation + "</TimeStamp>" + separator;
   }
   else
   {
      ossimString alreadyHasDate;
      appendRasterEntryDateTime(alreadyHasDate,
                                "", "");
      outputString += alreadyHasDate.string();
   }

} // End:  oms::DataInfo::appendRasterEntryMetadata( ... )

void oms::DataInfo::appendRasterEntryMetadata(
   ossimKeywordlist& kwl,
   const ossimString& prefix) const
{
   ossimString newPrefix = prefix+"metadata.";
   ossimString dateValue = "";
   //outputString += indentation + "<metadata>" + separator;

   std::shared_ptr<ossimInfoBase> info = ossimInfoFactoryRegistry::instance()->create(thePrivateData->theFilename);
   if (info)
   {
      ossimKeywordlist tempKwl;
      ossimKeywordlist kwl2;
      ossimKeywordlist tempDefaultKwl;
      ossimKeywordlist defaultKwl;
      // ossimXmlOutputKeywordList kwl3;
      ossimKeywordlist kwl3;
      ossimRefPtr<ossimImageHandler> imageHandler = thePrivateData->theImageHandler;
      ossim_uint32 entryId = imageHandler->getCurrentEntry();

      //info->print(std::cout);
      //      kwl.removeKeysThatMatch("[^.*image"+ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) +
      //                              "]");
      info->getKeywordlist(tempKwl);

      replaceSpacesInKeys(tempKwl);
      ossimKeywordlist desdata;

      tempKwl.extractKeysThatMatch(desdata, ".*DESDATA$");
      tempKwl.removeKeysThatMatch(".*DESDATA$");

      for (auto x : desdata.getMap())
      {
         ossimXmlDocument xml;

         if(ossimString(x.second).startsWith("<"))
         {
            if(xml.readString(x.second))
            {
               xml.toKwl(tempKwl, x.first+".");
            }
            else
            {
               tempKwl.add(x.first.c_str(), x.second.c_str());
            }
         }
      }
         // std::cout << "_________________________\n";
         // std::cout << kwl << "\n";
         // std::cout << "_________________________\n";
         kwl3.getMap() = tempKwl.getMap();
      kwl3.removeKeysThatMatch(".*\\.image.*\\..*");
      defaultKwl.getMap() = kwl3.getMap();
      tempKwl.extractKeysThatMatch(kwl2, ".*\\.image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) + "\\..*");
      tempKwl.extractKeysThatMatch(defaultKwl, ossimString(".*\\.image0") + "\\..*");
      ossimKeywordlist::KeywordMap::iterator iter = kwl2.getMap().begin();

      while (iter != kwl2.getMap().end())
      {
         ossimString k = iter->first;
         k = k.substitute(".image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()), "");
         kwl3.getMap().insert(make_pair(k.string(), iter->second));
         ++iter;
      }
      iter = tempDefaultKwl.getMap().begin();
      while (iter != tempDefaultKwl.getMap().end())
      {
         ossimString k = iter->first;
         k = k.substitute(".image0", "");
         defaultKwl.getMap().insert(make_pair(k.string(), iter->second));
         ++iter;
      }

      if (kwl3.getSize() > 0)
      {

         ossimString imageId;
         ossimString beNumber;
         ossimString cloudCover;
         ossimString imageRepresentation;
         ossimString isorce;
         ossimString offNadirAngle;
         ossimString targetId;
         ossimString productId;
         ossimString sensorId;
         ossimString missionId;
         ossimString countryCode;
         ossimString imageCategory;
         ossimString azimuthAngle;
         ossimString grazingAngle;
         ossimString securityClassification;
         ossimString securityCode;
         ossimString stripId;
         ossimString sunElevation;
         ossimString sunAzimuth;
         ossimString title;
         ossimString organization;
         ossimString description;
         ossimString niirs;

         // Open the dot omd file if present as it can now have metadata in it.
         ossimFilename omdFile;
         getOmdFile(omdFile);
         if (omdFile.exists())
         {
            kwl3.addFile(omdFile);
         }

         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         bool validModel = false;
         if (geom)
         {
            validModel = geom->isAffectedByElevation();
         }
         ossimKeywordlist downcaseKwl(kwl3);
         downcaseKwl.downcaseKeywords();
         // This downcases only the keys, not the values.
         // tempKwl.downcaseKeywords();
         // kwl3.downcaseKeywords();
         defaultKwl.downcaseKeywords();
         // Azimuth angle:
         getAzimuthAngle(downcaseKwl, azimuthAngle.string());

         // BE number:
         getBeNumber(downcaseKwl, beNumber.string());

         // Cloud cover:
         getCloudCover(downcaseKwl, cloudCover.string());

         // Country code:
         getCountryCode(downcaseKwl, countryCode.string());
         if (countryCode.empty() && entryId > 0)
         {
            getCountryCode(defaultKwl, countryCode.string());
         }

         // Date:
         getDate(downcaseKwl, dateValue.string());
         if (dateValue.empty() && entryId > 0)
         {
            getDate(defaultKwl, dateValue.string());
         }

         // Description:
         getDescription(downcaseKwl, description.string());

         // Grazing angle:
         getGrazingAngle(downcaseKwl, grazingAngle.string());

         // Image ID:
         getImageId(downcaseKwl, imageId.string());

         // Image Category:
         getImageCategory(downcaseKwl, imageCategory.string());

         // Image representation:
         getImageRepresentation(downcaseKwl, imageRepresentation.string());

         // ISOURCE:
         getIsorce(downcaseKwl, isorce.string());

         // Mission:
         getMissionId(downcaseKwl, missionId.string());
         if (missionId.empty() && entryId > 0)
         {
            getMissionId(defaultKwl, missionId.string());
         }

         // NIIRS:
         getNiirs(downcaseKwl, niirs.string());
         if (niirs.empty() && entryId > 0)
         {
            getNiirs(defaultKwl, niirs.string());
         }

         getOffNadirAngle(downcaseKwl, offNadirAngle.string());

         // Organization:
         getOrganization(downcaseKwl, organization.string());

         // Product:
         getProductId(downcaseKwl, productId.string());

         // Security classification:
         getSecurityClassification(downcaseKwl, securityClassification.string());
         if (securityClassification.empty() && entryId > 0)
         {
            getSecurityClassification(defaultKwl, securityClassification.string());
         }

         // Security code
         getSecurityCode(downcaseKwl, securityCode.string());
         if (securityCode.empty() && entryId > 0)
         {
            getSecurityCode(defaultKwl, securityCode.string());
         }
         // Sensor:
         getSensorId(downcaseKwl, sensorId.string());
         if (sensorId.empty() && entryId > 0)
         {
            getSensorId(defaultKwl, sensorId.string());
         }

         // Strip ID:
         getStripId(downcaseKwl, stripId.string());

         // Sun azimuth:
         getSunAzimuth(downcaseKwl, sunAzimuth.string());

         // Sun elevation:
         getSunElevation(downcaseKwl, sunElevation.string());

         // Target ID:
         getTargetId(downcaseKwl, targetId.string());

         // Title:
         getTitle(downcaseKwl, title.string());

         kwl.add(newPrefix.c_str(), "filename", thePrivateData->theFilename.c_str());
         kwl.add(newPrefix.c_str(), "imageId", imageId.c_str());
         kwl.add(newPrefix.c_str(), "imageRepresentation", imageRepresentation.c_str());

         if (isorce.size())
         {
            kwl.add(newPrefix.c_str(), "isorce", isorce.c_str());
         }

         kwl.add(newPrefix.c_str(), "targetId", targetId.c_str());
         kwl.add(newPrefix.c_str(), "productId", productId.c_str());
         kwl.add(newPrefix.c_str(), "beNumber", beNumber.c_str());
         kwl.add(newPrefix.c_str(), "cloudCover", cloudCover.c_str());
         kwl.add(newPrefix.c_str(), "sensorId", sensorId.c_str());
         kwl.add(newPrefix.c_str(), "missionId", missionId.c_str());
         kwl.add(newPrefix.c_str(), "countryCode", countryCode.c_str());
         kwl.add(newPrefix.c_str(), "imageCategory", imageCategory.c_str());
         kwl.add(newPrefix.c_str(), "azimuthAngle", azimuthAngle.c_str());
         kwl.add(newPrefix.c_str(), "grazingAngle", grazingAngle.c_str());
         kwl.add(newPrefix.c_str(), "offNadirAngle", offNadirAngle.c_str());
         kwl.add(newPrefix.c_str(), "securityClassification", securityClassification.c_str());
         kwl.add(newPrefix.c_str(), "securityCode", securityCode.c_str());
         kwl.add(newPrefix.c_str(), "title", title.c_str());
         kwl.add(newPrefix.c_str(), "organization", organization.c_str());
         kwl.add(newPrefix.c_str(), "description", description.c_str());
         kwl.add(newPrefix.c_str(), "niirs", niirs.c_str());
         kwl.add(newPrefix.c_str(), "stripId", stripId.c_str());
         kwl.add(newPrefix.c_str(), "sunAzimuth", sunAzimuth.c_str());
         kwl.add(newPrefix.c_str(), "sunElevation", sunElevation.c_str());
         kwl.add(newPrefix.c_str(), "validModel", ((validModel) ? "true" : "false"));

         ossimKeywordlist outKwl(tempKwl);
         outKwl.stripPrefixFromAll(".*\\.image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) + "\\.");
         outKwl.removeKeysThatMatch(".*\\.image.*\\..*");
         kwl.add(newPrefix.c_str(), outKwl);
      }
   }
   else
   {
      ossimString azimuthAngle = "0";
      if (thePrivateData->theImageHandler.valid())
      {
         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         ossimDrect rect = thePrivateData->theImageHandler->getBoundingRect();
         if (geom.valid())
         {
            ossimGpt centerGpt;
            ossimGpt upGpt;
            ossimDpt centerDpt(rect.midPoint());
            ossimDpt upDpt = centerDpt + ossimDpt(0, -rect.height() * .5);
            geom->localToWorld(centerDpt, centerGpt);
            geom->localToWorld(upDpt, upGpt);
            azimuthAngle = ossimString::toString(centerGpt.azimuthTo(upGpt));
         }
      }
      kwl.add(newPrefix.c_str(), "azimuthAngle", azimuthAngle.c_str());
   }
   
   ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
   if (geom.valid())
   {
      bool crossesDateline = geom->getCrossesDateline();
      kwl.add(newPrefix.c_str(), "crossesDateline", (crossesDateline ? "true" : "false"));
   }
   kwl.add(newPrefix.c_str(), "fileType", thePrivateData->formatName().c_str());
   kwl.add(newPrefix.c_str(), "className", (thePrivateData->theImageHandler.valid() ? thePrivateData->theImageHandler->getClassName().c_str() : ossimString("").c_str()));

   if (!dateValue.empty())
   {
      ossimString timePrefix = prefix + "TimeStamp.";

      kwl.add(timePrefix.c_str(), "when", dateValue.c_str());
   }
   else
   {
      appendRasterEntryDateTime(kwl, prefix);
   }

   ossimFilename omdFile = thePrivateData->theImageHandler->createDefaultMetadataFilename();
   if(omdFile.exists())
   {
      kwl.add(newPrefix.c_str(), ossimKeywordlist(omdFile));
   }

} // End:  oms::DataInfo::appendRasterEntryMetadata( ... )

bool oms::DataInfo::getWktFootprint( const ossimImageGeometry* geom, std::string& s ) const
{
   bool status = false;
   if ( geom &&geom->getProjection())
   {
      ossimPolyArea2d polyArea;

      bool mapProjectedFlag = (dynamic_cast<const ossimMapProjection*>(geom->getProjection())!=0);

      if(!mapProjectedFlag)
      {
         geom->calculatePolyBounds(polyArea, 25);
      }
      else
      {
         geom->calculatePolyBounds(polyArea, 1);
      }

      polyArea.toMultiPolygon();
      if(!polyArea.isEmpty())
      {
         s = polyArea.toString();
         status = true;
      }
   }
   return status;
}

void oms::DataInfo::getIsorce( const ossimKeywordlist& kwl,
                               std::string& isorce) const
{
   isorce = kwl.findKey( std::string("isorce") ); // omd file
   if ( isorce.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "\\.isorce$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         isorce = kwl.findKey( keys[0].string() );
      }
   }

   if ( isorce.size() )
   {
      isorce = ossimString( isorce ).trim().string();
   }
}

void oms::DataInfo::getAzimuthAngle( const ossimKeywordlist& kwl,
                                     std::string& azimuthAngle ) const
{
   azimuthAngle = kwl.findKey( std::string("azimuth_angle") ); // omd file
   if ( azimuthAngle.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.azimuth_angle$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         azimuthAngle = kwl.findKey( keys[0].string() );
      }

      if ( azimuthAngle.empty() )
      {
         azimuthAngle = kwl.findKey( std::string("dted.dsi.orientation") );

         if ( azimuthAngle.empty() )
         {
            // std::cout << "CALCULATING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            azimuthAngle = "0";
            if ( thePrivateData )
            {
               if( thePrivateData->theImageHandler.valid() )
               {
                  ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
                  if(geom.valid())
                  {
                     ossimDrect rect = thePrivateData->theImageHandler->getBoundingRect();
                     ossimGpt centerGpt;
                     ossimGpt upGpt;
                     ossimDpt centerDpt(rect.midPoint());
                     ossimDpt upDpt = centerDpt + ossimDpt(0,-rect.height()*.5);
                     geom->localToWorld(centerDpt, centerGpt);
                     geom->localToWorld(upDpt, upGpt);
                     azimuthAngle = ossimString::toString(centerGpt.azimuthTo(upGpt)).string();
                     //     std::cout << "UP: " << upDpt << std::endl;
                     //     std::cout << "CENTER: " << centerDpt << std::endl;
                     //     std::cout << "UP: " << upGpt << std::endl;
                     //     std::cout << "CENTER: " << centerGpt << std::endl;
                     //     std::cout << "======================================== " << centerGpt.azimuthTo(upGpt) << std::endl;
                  }
               }
            }
         }
      }
   }

   if ( azimuthAngle.size() )
   {
      azimuthAngle = ossimString( azimuthAngle ).trim().string();
   }
}

void oms::DataInfo::getBeNumber( const ossimKeywordlist& kwl,
                                 std::string& beNumber ) const
{
   beNumber = kwl.findKey( std::string("be") ); // omd file
   if ( beNumber.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.be$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         beNumber = kwl.findKey( keys[0].string() );
      }
   }

   if ( beNumber.size() )
   {
      beNumber = ossimString( beNumber ).trim().string();
   }
}

void oms::DataInfo::getCloudCover( const ossimKeywordlist& kwl,
                                   std::string& cloudCover ) const
{
   cloudCover = kwl.findKey( std::string("cloud_cover") ); // omd file
   if ( cloudCover.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.cloud_cover$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         cloudCover = kwl.findKey( keys[0].string() );
      }

      if ( cloudCover.empty() )
      {
         // Profile for Imagery Access Image:
         std::string key = "nitf.piaimc.cloudcvr";
         cloudCover = kwl.findKey( key );
         if ( cloudCover.empty() )
         {
            key = "tiff.gdalmetadata.cloud_cover";
            cloudCover = kwl.findKey( key );
         }
      }
   }

} // End: getCloudCover( ... )

void oms::DataInfo::getCountryCode( const ossimKeywordlist& kwl,
                                    std::string& countryCode ) const
{
   countryCode = kwl.findKey( std::string("country_code") ); // omd file
   if ( countryCode.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.cc$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         countryCode = kwl.findKey( keys[0].string() );
      }

      if ( countryCode.empty() )
      {
         keys.clear();
         regExp = "country$";
         kwl.findAllKeysThatMatch( keys, regExp );
         if ( keys.size() )
         {
            // Taking first one...
            countryCode = kwl.findKey( keys[0].string() );
         }

         if ( countryCode.empty() )
         {
            countryCode = kwl.findKey( std::string("nitf.rpf.countrycode") );
            if ( countryCode.empty() )
            {
               // Extract from tgtid, last two characters.:
               std::string tgtid;
               getTargetId( kwl, tgtid );
               if ( tgtid.size() >= 17 )
               {
                  countryCode = tgtid.substr( 15, 2 );
               }
            }
         }
      }
   }

} // End: getCountryCode( ... )

void oms::DataInfo::getDate( const ossimKeywordlist& kwl,
                             std::string& dateValue ) const
{
   dateValue = kwl.findKey( std::string("acquisition_date") ); // omd file
   if ( dateValue.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.acquisition_date$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         dateValue = kwl.findKey( keys[0].string() );
      }

      if ( dateValue.empty() )
      {
         //---
         // STDIDC Acquisition date:
         // Have seen two forms: ACQUISITION_DATE(SPEC) and ACQDATE(DG data):
         // test for both.
         //---
         dateValue = kwl.findKey( std::string("nitf.stdidc.acquisition_date") );
         if ( dateValue.empty() )
         {
            dateValue = kwl.findKey( std::string("nitf.stdidc.acqdate") );
         }
         if(dateValue.empty())
         {
            dateValue = kwl.findKey( std::string("nitf.idatim") );

         }
         if ( dateValue.size() )
         {
            //---
            // Convert from numeric form of: "CCYYMMDDhhmmss" to ISO 8601 form.
            // This is in the same form as the IDATIM image header field.
            // E.g.: 20180316051853 to 2018-03-16T05:18:53Z
            //---
            dateValue = convertIdatimToXmlDate( dateValue );
         }
         else
         {
            ossimString dateString(kwl.findKey("tiff.acquisition_date"));

            if (dateString.empty())
            {
               dateString = kwl.findKey("tiff.date_time");
            }
            if(dateString.empty())
            {
               dateString = kwl.findKey("jp2.acquisition_date");
            }
            if (!dateString.empty())
            {
               std::vector<ossimString> splitArray;

               // strip any decimal based seconds
               dateString = dateString.replaceStrThatMatch("\\..*", "");

               // test first if we are 'T' seaprated
               //
               dateString.split(splitArray, "T");
               if(splitArray.size() != 2)
               {
                  splitArray.clear();
                  dateString.split(splitArray, " ");
               }
               if (splitArray.size() > 0)
               {
                  dateValue = splitArray[0].substitute(":", "-", true).string();
                  if (splitArray.size() > 1)
                  {
                     dateValue += "T";
                     dateValue += splitArray[1].string();
                     if(!splitArray[1].endsWith("Z"))
                     {
                        dateValue += "Z";
                     }
                  }
                  else
                  {
                     dateValue += "T00:00:00Z";
                  }
               }
            }
         }
         if( dateValue.empty() )
         {
            dateValue = kwl.findKey( std::string("envi.collection.start"));
            if ( dateValue.empty() )
            {
               dateValue = kwl.findKey(std::string("envi.collection.end"));
            }
         }
      }
      if(dateValue.empty())
      {
         dateValue = kwl.findKey("nitf.csdida.time");
         if(!dateValue.empty())
         {
            dateValue += "Z";
         }
      }
   }

   if ( dateValue.size() )
   {
      dateValue = ossimString( dateValue ).trim().string();
   }

} // End: oms::DataInfo::getDate( ... )

void oms::DataInfo::getDescription( const ossimKeywordlist& kwl,
                                    std::string& description ) const
{
   description = kwl.findKey( std::string("description") ); // omd file
   if ( description.empty() )
   {
      getIsorce( kwl, description ); // Not sure about this? (drb)
   }

   if(description.empty())
   {
      description = kwl.findKey( std::string("envi.description"));
   }

   if ( description.size() )
   {
      description = ossimString( description ).trim().string();
   }
}

void oms::DataInfo::getGrazingAngle( const ossimKeywordlist& kwl,
                                     std::string& grazingAngle ) const
{
   grazingAngle = kwl.findKey( std::string("grazing_angle") ); // omd file
   if ( grazingAngle.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.grazing_angle$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         grazingAngle = kwl.findKey( keys[0].string() );
      }

      if ( grazingAngle.empty() )
      {
         // Normalized:
         keys.clear();
         regExp = "common\\.elevation_angle$";
         kwl.findAllKeysThatMatch( keys, regExp );
         if ( keys.size() )
         {
            grazingAngle = kwl.findKey( keys[0].string() );
         }

         if ( grazingAngle.empty() )
         {
            grazingAngle = kwl.findKey( std::string("tiff.gdalmetadata.view_angle") );
         }

#if 0 /* Removed 11 Sep 2015 (drb) */
         if ( grazingAngle.empty() )
         {
            keys.clear();
            regExp = "common\\.obliquity_angle$";
            kwl.findAllKeysThatMatch( keys, regExp );
            if ( keys.size() )
            {
               grazingAngle = kwl.findKey( keys[0].string() );
            }


            // From old code that was commented out.
            // ossimString tempAngleString =
            // kwl.findKey( std::string("nitf.use00a.angletonorth") );
            // if(!tempAngleString.empty())
            // {
            //    double tempAngle = tempAngleString.toDouble();
            //    azimuthAngle = ossimString::toString(fmod(tempAngle+90.0, 360.0));
            // }
            grazingAngle = kwl.findKey( std::string("nitf.use00a.oblang") );
            if ( grazingAngle.size() )
            {
               // ???
               double d = 90.0 - ossimString( grazingAngle ).toDouble();
               grazingAngle = ossimString::toString( d ).string();
            }
         }
#endif
      }
   }

   if ( grazingAngle.size() )
   {
      grazingAngle = ossimString( grazingAngle ).trim().string();
   }
}

void oms::DataInfo::getImageId( const ossimKeywordlist& kwl,
                                std::string& imageId ) const
{
   imageId = kwl.findKey( std::string("image_id") ); // omd file
   if ( imageId.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.image_id$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         imageId = kwl.findKey( keys[0].string() );
      }

      if ( imageId.empty() )
      {
         std::string key = "nitf.iid2";
         imageId = kwl.findKey( key );
         if ( imageId.empty() )
         {
            key = "nitf.iid";
            imageId = kwl.findKey( key );
            if ( imageId.empty() )
            {
               key = "nitf.ftitle";
               imageId = kwl.findKey( key );
               if ( imageId.size() )
               {
                  ossimFilename f = imageId;
                  imageId = f.fileNoExtension().string();
               }
               else
               {
                  key = "tiff.gdalmetadata.id";
                  imageId = kwl.findKey( key );
                  if ( imageId.empty() )
                  {
                     // Lastly use the filename:
                     if ( thePrivateData )
                     {
                        imageId = thePrivateData->theFilename.fileNoExtension().string();
                     }
                  }
               }
            }
         }
      }
   }

   if ( imageId.size() )
   {
      imageId = ossimString( imageId ).trim().string();
   }
}

void oms::DataInfo::getImageCategory( const ossimKeywordlist& kwl,
                                std::string& imageCategory ) const
{
   imageCategory = kwl.findKey( std::string("image_category") ); // omd file
   if ( imageCategory.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.image_category$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         imageCategory = kwl.findKey( keys[0].string() );
      }

      if ( imageCategory.empty() )
      {
         imageCategory = kwl.findKey( std::string("nitf.icat") );
      }
   }

   if ( imageCategory.size() )
   {
      imageCategory = ossimString( imageCategory ).trim().string();
   }
}

void oms::DataInfo::getImageRepresentation( const ossimKeywordlist& kwl,
                                            std::string& imageRepresentation ) const
{
   imageRepresentation = kwl.findKey( std::string("irep") ); // omd file
   if ( imageRepresentation.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.irep$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         imageRepresentation = kwl.findKey( keys[0].string() );
      }

      if ( imageRepresentation.empty() )
      {
         imageRepresentation = kwl.findKey( std::string("nitf.irep") );

         if ( imageRepresentation.empty() )
         {
            imageRepresentation = kwl.findKey( std::string("tiff.gdalmetadata.item_type") );
         }
      }
   }

   if ( imageRepresentation.size() )
   {
      imageRepresentation = ossimString( imageRepresentation ).trim().string();
   }

}

void oms::DataInfo::getMissionId( const ossimKeywordlist& kwl,
                                  std::string& missionId ) const
{
	missionId = kwl.findKey( std::string("mission_id") ); // omd file
   
   if ( missionId.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "mission_id$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         missionId = kwl.findKey( keys[0].string() );
      }

      if ( missionId.empty() )
      {
         // Search anything ending with mission.  In various tags.
         keys.clear();
         regExp = "\\.mission$";
         kwl.findAllKeysThatMatch( keys, regExp );
         if ( keys.size() )
         {
            // Taking first one...
            missionId = kwl.findKey( keys[0].string() );
         }

         if(missionId.empty())
         {
            ossimString platformCode = kwl.findKey("nitf.csdida.platform_code");
            ossimString vehicleId = kwl.findKey("nitf.csdida.vehicle_id");
            if (!platformCode.empty() && !vehicleId.empty())
            {
               missionId = (platformCode + vehicleId).c_str();
            }
         }

         if ( missionId.empty() )
         {
            //---
            // If we get here, see if we should pull from the iid2 or the more
            // generic isorce field.
            //---
            std::string iid2 = kwl.findKey( std::string("nitf.iid2") );

            ossimString isorce;
            getIsorce( kwl, isorce );

            if ( iid2.size() >= 11 )
            {
               // Given: "17APR151234..." return 1234:
               std::string iid2Mission;
               iid2Mission = iid2.substr( 7, 4 );
               bool allDigits = true;
               for ( ossim_uint32 i = 0; i < 4; ++i )
               {
                  if ( isdigit( iid2Mission[i] ) == false )
                  {
                     allDigits = false;
                     break;
                  }
               }
               if ( allDigits )
               {
                  int intMission = atoi( iid2Mission.c_str() );
                  if ( intMission > 100 )
                  {
                     int baseMission = (intMission / 100) * 100;
                     ossimString os = ossimString::toString( baseMission );
                     if ( isorce.contains( os ) )
                     {
                        //---
                        // If the (iid2/100)*100 is in the isorce, use the iid2.
                        // Else use the generic isorce.
                        //---
                        missionId = iid2Mission;
                     }
                  }
               }
            }

            if ( missionId.empty() && isorce.size() )
            {
               missionId = isorce.string();
            }

            if ( missionId.empty() )
            {
              missionId = kwl.findKey( std::string("tiff.gdalmetadata.satellite_id") );
            }
         }
      }
   }

   if ( missionId.size() )
   {
      missionId = ossimString( missionId ).trim().string();
   }

} // End: getMissionId( ... )

void oms::DataInfo::getNiirs( const ossimKeywordlist& kwl,
                                  std::string& niirs ) const
{
   niirs = kwl.findKey( std::string("niirs") ); // omd file
   if ( niirs.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.niirs$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         niirs = kwl.findKey( keys[0].string() );
      }

      if ( niirs.empty())
      {
         // NITF Exploitation Reference Data(CSEXRA):
         niirs = kwl.findKey( std::string("nitf.csexra.predicted_niirs") );

         if ( niirs.empty())
         {
            keys.clear();
            ossimString regExp = "\\.niirs$"; // any tag ending in .niirs.
            kwl.findAllKeysThatMatch( keys, regExp );
            if ( keys.size() )
            {
               // Taking first one...
               niirs = kwl.findKey( keys[0].string() );
            }
         }
      }
   }

   if ( niirs.size() )
   {
      niirs = ossimString( niirs ).trim().string();
   }

} // End: getNiirs( ... )

void oms::DataInfo::getOrganization( const ossimKeywordlist& kwl,
                                  std::string& organization ) const
{
   organization = kwl.findKey( std::string("organization") ); // omd file
   if ( organization.empty())
   {
      organization = kwl.findKey( std::string("nitf.oname") );

      if ( organization.empty())
      {
         organization = kwl.findKey( std::string("tiff.gdalmetadata.provider") );
      }
   }

   if ( organization.size() )
   {
      organization = ossimString( organization ).trim().string();
   }

} // End: getOrganization( ... )

void oms::DataInfo::getOffNadirAngle( const ossimKeywordlist& kwl,
                                      std::string& offNadirAngle ) const
{
   offNadirAngle = kwl.findKey( std::string("off_nadir_angle") ); // omd file
   if ( offNadirAngle.empty() )
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.off_nadir_angle$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         offNadirAngle = kwl.findKey( keys[0].string() );
      }
   }

   if ( offNadirAngle.size() )
   {
      offNadirAngle = ossimString( offNadirAngle ).trim().string();
   }

} // End: getOffNadiAngle(...)


void oms::DataInfo::getProductId( const ossimKeywordlist& kwl,
                                  std::string& productId ) const
{
   productId = kwl.findKey( std::string("product_id") ); // omd file
   if ( productId.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.product_id$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         productId = kwl.findKey( keys[0].string() );
      }

      if ( productId.empty() )
      {
         productId = kwl.findKey( std::string("nitf.rpf.ProductDataType") );
         if ( productId.empty())
         {
            productId = kwl.findKey( std::string("dted.dsi.product_level") );

            if( ossimString( productId ).downcase().contains("rted") )
            {
               // Special case for generated dted:
               std::string points = kwl.findKey( std::string("dted.uhl.number_of_lat_points") );
               if ( points.size() )
               {
                  if(points == "3601")
                  {
                     productId = "DTED2";
                  }
                  else if(points == "1201")
                  {
                     productId = "DTED1";
                  }
                  else if(points == "0121")
                  {
                     productId = "DTED0";
                  }
                  else
                  {
                     productId = "DTED";
                  }
               }
            }
         }
      }
   }

   if ( productId.size() )
   {
      productId = ossimString( productId ).trim().string();
   }

} // End: getProductId( ... )

void oms::DataInfo::getSecurityClassification( const ossimKeywordlist& kwl,
                                               std::string& securityClassification ) const
{
   // Method assumes keys have been downcased.

   securityClassification.clear();

   // Look for normalized key first, could be from omd file:
   std::string key = "security_classification";
   securityClassification = kwl.findKey( key );

   if ( securityClassification.empty() )
   {
      // nitf common:
      key = "nitf.common.classification";
      securityClassification = kwl.findKey( key );
      if ( securityClassification.empty() )
      {
         // tfrd common:
         key = "tfrd.common.classification";
         securityClassification = kwl.findKey( key );
         if ( securityClassification.empty() )
         {
            // nitf Image Security Classification tag:
            key = "nitf.isclas";
            securityClassification = kwl.findKey( key );
            if ( securityClassification.empty() )
            {
               // rpf
               key = "nitf.rpf.SecurityClassification";
               securityClassification = kwl.findKey( key );
               if ( securityClassification.empty() )
               {
                  // dted:
                  key = "dted.dsi.security_code";
                  securityClassification = kwl.findKey( key );
               }
            }
         }
      }

   } // Matches: if ( securityClassification.empty() )

   if ( securityClassification.size() )
   {
      if(securityClassification == "R")
      {
         securityClassification = "RESTRICTED";
      }
      else if(securityClassification == "U")
      {
         securityClassification = "UNCLASSIFIED";
      }
      else if(securityClassification == "S")
      {
         securityClassification = "SECRET";
      }
      else if(securityClassification == "T")
      {
         securityClassification = "TOP SECRET";
      }
      else if(securityClassification == "TS")
      {
         securityClassification = "TOP SECRET";
      }
   }
}
void oms::DataInfo::getSecurityCode(const ossimKeywordlist& kwl,
                                    std::string& securityCode)const
{

   securityCode.clear();
   // Look for normalized key first, could be from omd file:
   std::string key = "security_code";
   securityCode = kwl.findKey( key );
   if ( securityCode.empty() )
   {
      key = "nitf.fscltx";
      securityCode = kwl.findKey( key );
      if(securityCode.empty())
      {
         // tfrd common:
         key = "tfrd.common.securityCode";
         securityCode = kwl.findKey( key );
         if ( securityCode.empty() )
         {
            // rpf
            key = "nitf.rpf.SecurityCode";
            securityCode = kwl.findKey( key );
            if ( securityCode.empty() )
            {
               // dted:
               key = "dted.dsi.security_code";
               securityCode = kwl.findKey( key );
            }
         }
      }
   }
   ossimString tempSecurityCode =  securityCode;
   tempSecurityCode = tempSecurityCode.downcase();
   // test if it was some kind of SIC code
   ossimKeywordlist tempKwl;
   tempKwl.parseString(tempSecurityCode);
   ossimString sicCode;
   sicCode = tempKwl.findKey("sic");
   if(!sicCode.empty())
   {
      securityCode = sicCode.string();
   }
}


void oms::DataInfo::getSensorId( const ossimKeywordlist& kwl,
                                 std::string& sensorId ) const
{
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "sensor_id$";
      kwl.findAllKeysThatMatch(keys, regExp);
      if (keys.size())
      {
         // Taking first one...
         sensorId = kwl.findKey(keys[0].string());
      }
//      sensorId = kwl.findKey(std::string("sensor_id")); // omd file
      // if (sensorId.empty())
      // {
      //    sensorId = kwl.findKey("nitf.csdida.sensor_id");
      //    if (sensorId.empty())
      //    {
      //       sensorId = kwl.findKey(std::string("nitf.common.sensor_id"));
      //       if (sensorId.empty())
      //       {
      //          sensorId = kwl.findKey(std::string("tiff.sensor_id"));
      //          if (sensorId.empty())
      //          {
      //             sensorId = kwl.findKey(std::string("tfrd.common.sensor_id"));
                  if (sensorId.empty())
                  {
                     sensorId = kwl.findKey(std::string("tiff.gdalmetadata.instrument"));
                  }
      //          }
      //       }
      //    }
      // }
      if (sensorId.empty())
      {
         sensorId = kwl.findKey(std::string("envi.sensor_type"));
      }
      if (!sensorId.empty())
      {
         sensorId = ossimString(sensorId).trim().string();
      }
   }

void oms::DataInfo::getStripId( const ossimKeywordlist& kwl,
                       std::string& stripId ) const
{
   stripId = kwl.findKey( std::string("strip_id") ); // omd file
   if ( stripId.empty())
   {
      stripId = kwl.findKey( std::string("nitf.common.strip_id") );
      if ( stripId.empty() )
      {
         stripId = kwl.findKey( std::string("tiff.gdalmetadata.strip_id") );
      }
   }

   if ( stripId.size() )
   {
      stripId = ossimString( stripId ).trim().string();
   }
}

void oms::DataInfo::getSunAzimuth( const ossimKeywordlist& kwl,
                                   std::string& sunAzimuth ) const
{
   sunAzimuth = kwl.findKey( std::string("sun_azimuth") ); // omd file
   if ( sunAzimuth.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.sunaz$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         sunAzimuth = kwl.findKey( keys[0].string() );
      }

      if ( sunAzimuth.empty())
      {
			regExp = "\\.sun_azimuth$";
			kwl.findAllKeysThatMatch(keys, regExp);

			if (keys.size())
			{
				// Taking first one...
				sunAzimuth = kwl.findKey(keys[0].string());
			}

			if ( sunAzimuth.empty() )
         {
            keys.clear();
            ossimString regExp = "\\.sun_az$"; // any tag ending in: ".sun_az"
            kwl.findAllKeysThatMatch( keys, regExp );
            if ( keys.size() )
            {
               // Taking first one...
               sunAzimuth = kwl.findKey( keys[0].string() );
            }
         }
      }
   }

   if ( sunAzimuth.size() )
   {
      sunAzimuth = ossimString( sunAzimuth ).trim().string();
   }

} // End: getSunAzimuth( ... )

void oms::DataInfo::getSunElevation( const ossimKeywordlist& kwl,
                                     std::string& sunElevation ) const
{
   sunElevation = kwl.findKey( std::string("sun_elevation") ); // omd file
   if ( sunElevation.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.sunel$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         sunElevation = kwl.findKey( keys[0].string() );
      }

      if ( sunElevation.empty())
      {
			regExp = "\\.sun_elevation$";
			kwl.findAllKeysThatMatch(keys, regExp);

			if (keys.size())
			{
				// Taking first one...
				sunElevation = kwl.findKey(keys[0].string());
			}

			if ( sunElevation.empty() )
         {
            keys.clear();
            ossimString regExp = "\\.sun_el$"; // any tag ending in ".sun_el"
            kwl.findAllKeysThatMatch( keys, regExp );
            if ( keys.size() )
            {
               // Taking first one...
               sunElevation = kwl.findKey( keys[0].string() );
            }
         }
      }
   }

   if ( sunElevation.size() )
   {
      sunElevation = ossimString( sunElevation ).trim().string();
   }

} // End: getSunElevation( ... )

void oms::DataInfo::getTargetId( const ossimKeywordlist& kwl,
                                 std::string& targetid ) const
{
   targetid = kwl.findKey( std::string("target_id") ); // omd file
   if ( targetid.empty())
   {
      targetid = kwl.findKey( std::string("nitf.common.target_id") );
      if ( targetid.empty() )
      {
         targetid = kwl.findKey( std::string("tfrd.common.target_id") );
         if ( targetid.empty() )
         {
            targetid = kwl.findKey( std::string("nitf.tgtid") );
            if ( targetid.empty() )
            {
               // Search anything ending in tgtid:
               std::vector<ossimString> keys;
               ossimString regExp = "tgtid$";
               kwl.findAllKeysThatMatch( keys, regExp );
               if ( keys.size() )
               {
                  // Taking first one...
                  targetid = kwl.findKey( keys[0].string() );
               }
            }
         }
      }
   }

   if ( targetid.size() )
   {
      targetid = ossimString( targetid ).trim().string();
   }
}

void oms::DataInfo::getTitle( const ossimKeywordlist& kwl,
                              std::string& title ) const
{
   title = kwl.findKey( std::string("title") ); // omd file
   if ( title.empty())
   {
      // Normalized:
      std::vector<ossimString> keys;
      ossimString regExp = "common\\.ititle$";
      kwl.findAllKeysThatMatch( keys, regExp );
      if ( keys.size() )
      {
         // Taking first one...
         title = kwl.findKey( keys[0].string() );
      }

      if ( title.empty() )
      {
         title = kwl.findKey( std::string("nitf.iid2") );
         if ( title.empty() )
         {
            title = kwl.findKey( std::string("nitf.ititle") );
         }
      }
   }

   if ( title.size() )
   {
      title = ossimString( title ).trim().string();
   }
}

void oms::DataInfo::getOmdFile( ossimFilename& omdFile ) const
{
   if ( thePrivateData )
   {
      if ( thePrivateData->theImageHandler.valid() )
      {
         omdFile = thePrivateData->theImageHandler->getFilename().expand().noExtension();
         if ( thePrivateData->theImageHandler->getNumberOfEntries() > 1 )
         {
            omdFile += std::string("_e") +
               ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()).string() +
               std::string(".omd");
         }
         else
         {
            omdFile.setExtension( ossimString("omd") );
         }
      }
   }
}
