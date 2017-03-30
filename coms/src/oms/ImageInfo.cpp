#include <oms/ImageInfo.h>
#include <oms/Util.h>
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
#include <ossim/projection/ossimSarModel.h>

#include <sstream>
class jsonOutputKeywordList : public ossimKeywordlist
{
public:
   jsonOutputKeywordList(bool pretty = true, bool json = true)
   :ossimKeywordlist(),
   m_pretty(pretty),
   m_json(json)
   {
   }

   void writePretty(ossim_uint32 tabcount, std::ostream &out)const
   {
	if (m_pretty)
	{
	  out << "\n";
	  for (ossim_uint32 i = 0; i < tabcount; ++i)
	  {
	    out << "\t";
	  }
	}
   }

   virtual void writeImageHeaderFields(ossimString imagenum, std::ostream &out)const
   {
     
      bool first = true;
      ossimKeywordlist::KeywordMap::const_iterator iter = m_map.begin();
      while(iter != m_map.end())
      {
	 ossimString path = iter->first;
         path = path.trim();
         ossimString value = iter->second;
         replaceSpecialCharacters(path);
         std::vector<ossimString> splitValues;
	 path.split(splitValues,".");
	 if (splitValues.size() == 3 && splitValues[1].contains(imagenum)) 
	 {
		if (first) first = false;
	 	else out << ",";	
		writePretty(5, out);
	 	ossimString lineout = "\"" + splitValues[2] + "\": \"" + value.trim() + "\"";
                out << lineout;
	 }
	 ++iter;
      }
   }

   virtual void writeToStream(std::ostream &out)const
   {
    if (m_json)
    {
      bool first = true;
      bool second = false;
      bool third = false;
      bool firstheader = true;
      bool firstheadertre = true;
      bool firstheadertrename = true;
      bool firstheadertrevalue = true;
      bool firstimage = true;
      bool firstimageheader = true;
      bool firstimagetre = true;
      bool firstimagetrename = true;
      bool firstimagetrevalue = true;
      bool hasImageHeaders = false;
      bool hasFileHeaders = false;
      ossimString currentTRE = "";
      ossimString currentImage = "";
      writePretty(1, out);
      out << "\"details\": {";
      ossimKeywordlist::KeywordMap::const_iterator iter = m_map.begin();
      while(iter != m_map.end())
      {
         ossimString path = iter->first;
	 path = path.trim();
         ossimString value = iter->second;
         
         replaceSpecialCharacters(path);
         std::vector<ossimString> splitValues;
         path.split(splitValues,".");
	 if (first && splitValues.size() == 2)
	 {
		hasFileHeaders = true;
		if (firstheader)
		{
		   writePretty(2, out);
		   out << "\"fileheader\": {";
		}
		if (!firstheader) out << ",";
		writePretty(3, out);
		ossimString lineout = "\"" + splitValues[1] + "\": \"" + value.trim() + "\"";
		out << lineout;
		firstheader = false;
	 }
	 if (second && splitValues.size() == 3 && !splitValues[1].contains("image") && !splitValues[1].contains("COMMON")) // File Header TREs
	 {
		if (firstheadertre) 
		{
		        if (!hasFileHeaders)
                	{
                      	  hasFileHeaders = true;
                   	  writePretty(2, out);
                          out << "\"fileheader\": {";
                	}	
			else
			{
			  out << ",";
			}
			writePretty(3, out);
			out << "\"tres\": [";
			firstheadertre = false;
		}
		if (splitValues[1] != currentTRE)
		{
			if (!firstheadertrename) 
			{	
				writePretty(5,out);
				out << "}";
				writePretty(4, out);
				out << "},";
			}
			writePretty(4, out);
			out << "{";
			writePretty(5, out);
			out << "\"" << splitValues[1] << "\": {";
			currentTRE = splitValues[1];
			firstheadertrename = false;
			firstheadertrevalue = true;
		}
		if (!firstheadertrevalue) out << ",";
		else firstheadertrevalue = false;
		writePretty(5, out);
		ossimString lineout = "\"" + splitValues[2] + "\": \"" + value.trim() + "\"";
		out << lineout;
	 }
	 if (third && splitValues.size() == 4 && splitValues[1].contains("image") && !splitValues[2].contains("COMMON"))
	 {
		if (hasFileHeaders) 
		{
			out << ",";
			hasFileHeaders = false;
		}
		hasImageHeaders = true;
		ossimString imagenum = splitValues[1];
		if (firstimage) {
			writePretty(2, out);
			out << "\"imageheaders\": [";
			firstimage = false;
		} 
		if (imagenum != currentImage )
		{
			if (!firstimageheader)
			{
				writePretty(7, out);
				out << "}";
				writePretty(6, out);
				out << "}";
				writePretty(5, out);
				out << "]";
				writePretty(4, out);
				out << "}";
				writePretty(3, out);
				out << "},";
			}
			else
			{
				firstimageheader = false;
			}
			writePretty(3, out);
			out << "{";
			writePretty(4, out);
			out << "\"imageheader\": {"; 
			writeImageHeaderFields(imagenum, out);
			currentImage = imagenum;
			firstimagetre = true;
			firstimagetrename = true;
			firstimagetrevalue = true;
			currentTRE = "";
		}
		if (firstimagetre)
                {
			//if (!firstimagetrename)
                        //{
                        //  out << ",";
                        //}
                        out << ",";
			writePretty(5, out);
			out << "\"tres\": [";
                        firstimagetre = false;
                }
                if (splitValues[2] != currentTRE)
                {
			firstimagetrevalue = true;
			if (!firstimagetrename)
			{
			  writePretty(7, out);
			  out << "}";
			  writePretty(6, out);
			  out << "},";
			}
			else
			{
			  firstimagetrename = false;
			}
			writePretty(6, out);
			out << "{";
			writePretty(7, out);
			out << "\"" << splitValues[2] << "\": {";
                        currentTRE = splitValues[2];
                }
		if (!firstimagetrevalue) out << ",";
                else firstimagetrevalue = false;
		writePretty(8, out);
		ossimString lineout = "\"" + splitValues[3] + "\": \"" + value.trim() + "\"";
	 	out << lineout;	
	 }
         ++iter;
	 if (iter == m_map.end() && (first || second || third))
         {
	   if (third && hasImageHeaders)
           {
	     writePretty(7, out);
	     out << "}";
	     writePretty(6, out);
	     out << "}";
	     writePretty(5, out);
	     out << "]";
	     writePretty(4, out);
	     out << "}";
	     writePretty(3, out);
	     out << "}";
	     writePretty(2, out);
	     out << "]";
           }
	   if (second) 
	   {
		if (!firstheadertre)
		{
		   writePretty(5, out);
		   out << "}";
		   writePretty(4, out);
		   out << "}";
		   writePretty(3, out);
		   out << "]";
		}
		writePretty(2, out);
		if (hasFileHeaders) out << "}"; // END fileheader
	   }
	   if (!third) iter = m_map.begin();
           first = false;
	   if (second) 
	   {
		second = false;
		third = true;
		currentTRE = "";
	   }
	   else second = true;
         }
      }
      
      writePretty(1, out);
      out << "}";
    }
    else ossimKeywordlist::writeToStream(out);
   }
   void replaceSpecialCharacters(ossimString& value)const
   {
      ossimString::iterator iter = value.begin();
      
      while(iter!=value.end())
      {
         if(!(isdigit(*iter) ||
              isalpha(*iter)||
              (*iter=='.')))
         {
            *iter = '_';
         }
         ++iter;
      }
   }
   bool testSpecialCharacters(ossimString& value)const
   {
      ossimString::iterator iter = value.begin();

      while(iter!=value.end())
      {
         if(!(isdigit(*iter) ||
              isalpha(*iter)||
              (*iter=='/')))
         {
           return true;
         }
         ++iter;
      }
      return false;
   }
   bool m_pretty;
   bool m_json;
};

namespace oms
{
   class ImageInfoPrivateData
   {
   public:
      ~ImageInfoPrivateData()
      {
         clear();
      }
      void clear()
      {
         theFilename = "";
         theImageHandler = 0;
      }
      
      std::string formatName() const
      {
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
               }
            }
         return "unspecified";
      }
      ossimFilename theFilename;
      ossimRefPtr<ossimImageHandler> theImageHandler;
   };
}

oms::ImageInfo::ImageInfo() :
thePrivateData(new oms::ImageInfoPrivateData),
pretty(true),
showsummary(true),
showdetails(true),
omardb(true),
multi(true),
json(true)
{
}

oms::ImageInfo::~ImageInfo()
{
   if (thePrivateData)
   {
      delete thePrivateData;
      thePrivateData = 0;
   }
}

bool oms::ImageInfo::open(const std::string& file, bool failIfNoGeometryFlag)
{
   bool result = false;
   thePrivateData->clear();
   
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(ossimFilename(file));
   
   
   if (handler.valid() == true)
   {
      result = setHandler(handler.get(), failIfNoGeometryFlag);
   }
   else
   {
   }
   
   if (result == false)
   {
      thePrivateData->clear();
   }
   //   std::cout << "OPEN FILE? " << result << ", " << file << std::endl;
   
   return result;
}

bool oms::ImageInfo::setHandler(ossimImageHandler* handler, bool failIfNoGeometryFlag)
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

void oms::ImageInfo::close()
{
   if (thePrivateData)
   {
      thePrivateData->clear();
   }
}

std::string oms::ImageInfo::getInfo() const
{
   std::string result = "";
   ossimString root = "";
   ossimString path = "";
   if (!thePrivateData)
      return result;
   if (thePrivateData->theImageHandler.valid())
   {
      root += thePrivateData->formatName() + ".";
      path += root + "RASTER_FILE.";
      ossimKeywordlist detailsKWL;
      bool hasKWL = false;

      std::shared_ptr<ossimInfoBase> info = ossimInfoFactoryRegistry::instance()->create(thePrivateData->theFilename);
      if (info)
      {
	info->getKeywordlist(detailsKWL);
	hasKWL = true;
      }

      if (json && !omardb) result += "{";
      if (showsummary || !showdetails)
      {
	
        if (pretty && json) result += "\n\t";
        if (showdetails && json) result += "\"summary\": {";
        if (1 && !omardb)
        {
          writeJsonKVP(result, "name", thePrivateData->theFilename, true, 2, path);
          writeJsonKVP(result, "format", thePrivateData->formatName(), true, 2, path);
        }
        if (omardb) appendRasterEntries(result, detailsKWL, root);
        if (pretty && json) result += "\n\t";
        if (showdetails && json) result += "}";
      }
      if (showdetails && hasKWL) 
      {
	 if (showsummary && json) result += ",";
         appendMetadataDetails(result, detailsKWL);
      }
      if (pretty && json) result += "\n";
      if (json && !omardb) result += "}";
      if (pretty && json) result += "\n";
   }
   return result;
}

bool oms::ImageInfo::getKwl(ossimKeywordlist& kwl) const
{
   
   // Do a print to a memory stream.
   std::ostringstream out;
   out.str( getInfo() );

   std::istringstream in(out.str());
   // Give the result to the keyword list.
   return kwl.parseStream(in);
}

bool oms::ImageInfo::isImagery()const
{
   if(!thePrivateData) return false;
   return thePrivateData->theImageHandler.valid();
}

void oms::ImageInfo::writeJsonKVP(std::string& outputString, ossimString key, ossimString value, bool comma, ossim_uint32 tabcount, ossimString path) const
{
  if (json)
  {
    if (pretty) 
    {
	outputString += "\n";
	for (ossim_uint32 i = 0; i < tabcount; ++i)
	{
	  outputString += "\t";
	}
    }
    outputString += "\"" + key.string() + "\": \"" + value.string() + "\"";	 
    if (comma) outputString += ",";
  }
  else
  {
    outputString += "\n" + path.string() + key.string() + ": " + value.string();
  }
}


void oms::ImageInfo::appendRasterEntry(std::string& outputString, const ossimKeywordlist& kwl,
                                        ossim_uint32 entry, ossimString root) const
{
   ossimString path = root + "image" + ossimString::toString(entry) + ".RASTER_ENTRY.";
   if (multi || !entry)
   {
     if (entry && json) outputString += ",";
     if (multi && pretty && json) outputString += "\n\t\t\t";
     if (multi && json) outputString += "{";
     //if (multi && pretty && json) outputString += "\n\t\t\t\t";
     //if (multi && json) outputString += "{";
   
     ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
   //outputString += indentation + "<RasterEntry>" + separator;
   //appendAssociatedRasterEntryFileObjects(outputString, indentation
   //                                       + "   ", separator);
     ossim_uint32 indent = 5;
     if (!multi) indent = 2;
     writeJsonKVP(outputString, "entryId", ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()), true, indent, path);
     std::string entryName;
     thePrivateData->theImageHandler->getEntryName(thePrivateData->theImageHandler->getCurrentEntry(), entryName);
     writeJsonKVP(outputString, "entryName", ossimString(entryName), true, indent, path);

     writeJsonKVP(outputString, "width", ossimString::toString(rect.width()), true, indent, path);
     writeJsonKVP(outputString, "height", ossimString::toString(rect.height()), true, indent,path);
     writeJsonKVP(outputString, "numberOfBands", ossimString::toString(thePrivateData->theImageHandler->getNumberOfOutputBands()), true, indent, path);
     writeJsonKVP(outputString, "numberOfResLevels", ossimString::toString(thePrivateData->theImageHandler->getNumberOfDecimationLevels()), true, indent, path);

     appendBitDepthAndDataType(outputString, indent, path);
     appendGeometryInformation(outputString, indent, path);
     appendRasterEntryDateTime(outputString, indent,  path);
     appendNormalizedMetadata(outputString, kwl, indent, path);
     //if (multi && pretty && json) outputString += "\n\t\t\t\t";
     //if (multi && json) outputString += "}";
     if (multi && pretty && json) outputString += "\n\t\t\t";
     if (multi && json) outputString += "}";
   }
}

void oms::ImageInfo::appendRasterEntries(std::string& outputString, const ossimKeywordlist& kwl,
                                        ossimString root) const
{
   ossim_uint32 numberOfEntries =
   thePrivateData->theImageHandler->getNumberOfEntries();
   ossim_uint32 idx = 0;
   if (multi && pretty && json) outputString += "\n\t\t";
   if (multi && json) outputString += "[";
   for (idx = 0; idx < numberOfEntries; ++idx)
   {
      if(thePrivateData->theImageHandler->setCurrentEntry(idx))
      {
         appendRasterEntry(outputString, kwl, idx, root);
      }
   }
   if (multi && pretty && json) outputString += "\n\t\t";
   if (multi && json) outputString += "]";
}

void oms::ImageInfo::appendAssociatedRasterEntryFileObjects(
                                                           std::string& outputString, const std::string& indentation,
                                                           const std::string& separator) const
{
   ossimFilename kmlFile(thePrivateData->theImageHandler->getFilename());
   ossimFilename overviewFile =
   thePrivateData->theImageHandler->createDefaultOverviewFilename();
   ossimFilename overview2File =
   thePrivateData->theImageHandler->getFilename() + ".ovr";
   ossimFilename histogramFile =
   thePrivateData->theImageHandler->createDefaultHistogramFilename();
   ossimFilename
   validVerticesFile =
   thePrivateData->theImageHandler->createDefaultValidVerticesFilename();
   ossimFilename geomFile =
   thePrivateData->theImageHandler->createDefaultGeometryFilename();
   ossimFilename metadataFile =
   thePrivateData->theImageHandler->createDefaultMetadataFilename();
   
   // we will only support for now kml files associated at the entire file level and
   // not individual entries.
   //
   kmlFile = kmlFile.setExtension("kml");
   
   bool associatedFilesFlag = (overviewFile.exists() || overview2File.exists() || histogramFile.exists()
                               || validVerticesFile.exists() || geomFile.exists()
                               || metadataFile.exists()||kmlFile.exists());
   if (associatedFilesFlag)
   {
      outputString += indentation + "<fileObjects>" + separator;
      
      if (overviewFile.exists())
      {
         outputString += indentation
            + "   <RasterEntryFile type=\"overview\">" + separator
            + indentation + "      <name>" + overviewFile.string() + "</name>"
            + separator + indentation + "   </RasterEntryFile>"
            + separator;
      }
      else if (overview2File.exists())
      {
         outputString += indentation
            + "   <RasterEntryFile type=\"overview\">" + separator
            + indentation + "      <name>" + overview2File.string() + "</name>"
            + separator + indentation + "   </RasterEntryFile>"
            + separator;
      }
      if (histogramFile.exists())
         if (histogramFile.exists())
         {
            outputString += indentation
               + "   <RasterEntryFile type=\"histogram\">" + separator
               + indentation + "      <name>" + histogramFile.string() + "</name>"
               + separator + indentation + "   </RasterEntryFile>"
               + separator;
         }
      if (validVerticesFile.exists())
      {
         outputString += indentation
            + "   <RasterEntryFile type=\"valid_vertices\">"
            + separator + indentation + "      <name>"
            + validVerticesFile.string() + "</name>" + separator + indentation
            + "   </RasterEntryFile>" + separator;
      }
      if (geomFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"geom\">"
            + separator + indentation + "      <name>" + geomFile.string()
            + "</name>" + separator + indentation
            + "   </RasterEntryFile>" + separator;
      }
      if (metadataFile.exists())
      {
         outputString += indentation + "   <RasterEntryFile type=\"omd\">"
            + separator + indentation + "      <name>" + metadataFile.string()
            + "</name>" + separator + indentation
            + "   </RasterEntryFile>" + separator;
      }
      if (kmlFile.exists())
      {
         outputString += indentation
            + "   <RasterEntryFile type=\"kml\">" + separator
            + indentation + "      <name>" + kmlFile.string() + "</name>"
            + separator + indentation + "   </RasterEntryFile>"
            + separator;
      }
      outputString += indentation + "</fileObjects>" + separator;
   }
}

void oms::ImageInfo::appendBitDepthAndDataType(std::string& outputString,
                                              ossim_uint32 indent,ossimString path) const
{
   ossim_uint32 bits = 0;
   ossimString dataType;
   switch (thePrivateData->theImageHandler->getOutputScalarType())
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
   writeJsonKVP(outputString, "bitDepth", ossimString::toString(bits), true, indent, path);
   writeJsonKVP(outputString, "dataType", ossimString::toString(bits), true, indent, path);
}

void oms::ImageInfo::appendGeometryInformation(std::string& outputString,
                                              ossim_uint32 indent, ossimString path) const
{
   ossimIrect rect = thePrivateData->theImageHandler->getBoundingRect();
   ossimKeywordlist kwl;
   ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
   ossimGpt wgs84;
   ossimString groundGeometry;
   ossimString groundGeometryCounterClockwise;
   ossimString groundShape;
   ossimString validGroundGeometry;
   
   if (geom.valid()&&geom->getProjection())
   {
      ossimDpt gsd = geom->getMetersPerPixel();
      //outputString += indentation + "<gsd unit=\"meters\" dx=\"" +
      //   ossimString::toString(gsd.x,15).string() + "\" dy=\"" +
      //   ossimString::toString(gsd.y,15).string() + "\"/>" + separator;
      ossim_float64 gsdavg = (gsd.x + gsd.y) * 0.5;
      if (1)
      {
      	writeJsonKVP(outputString, "gsdx", ossimString::toString(gsd.x,15), true, indent, path);
      	writeJsonKVP(outputString, "gsdy", ossimString::toString(gsd.y,15), true, indent, path);
      	writeJsonKVP(outputString, "gsd", ossimString::toString(gsdavg), true, indent, path);
	writeJsonKVP(outputString, "gsdInches", ossimString::toString(gsdavg * FT_PER_MTRS * 12.0), true, indent, path);
      }
      else
      {
      	writeJsonKVP(outputString, "GSD", ossimString::toString(gsdavg * FT_PER_MTRS * 12.0), true, indent, path);
      }
      
      ossimGpt ul;
      ossimGpt ur;
      ossimGpt lr;
      ossimGpt ll;
      geom->localToWorld(rect.ul(), ul);
      geom->localToWorld(rect.ur(), ur);
      geom->localToWorld(rect.lr(), lr);
      geom->localToWorld(rect.ll(), ll);
      
      if (ul.isLatNan() || ul.isLonNan() || ur.isLatNan() || ur.isLonNan()
          || lr.isLatNan() || lr.isLonNan() || ll.isLatNan()
          || ll.isLonNan())
      {
         return;
      }
      ul.changeDatum(wgs84.datum());
      ur.changeDatum(wgs84.datum());
      lr.changeDatum(wgs84.datum());
      ll.changeDatum(wgs84.datum());
      
      groundGeometry += ("POLYGON((" + ossimString::toString(ul.lond()) + " "
                         + ossimString::toString(ul.latd()) + ","
                         + ossimString::toString(ur.lond()) + " "
                         + ossimString::toString(ur.latd()) + ","
                         + ossimString::toString(lr.lond()) + " "
                         + ossimString::toString(lr.latd()) + ","
                         + ossimString::toString(ll.lond()) + " "
                         + ossimString::toString(ll.latd()) + ","
                         + ossimString::toString(ul.lond()) + " "
                         + ossimString::toString(ul.latd()) + "))");

      groundGeometryCounterClockwise += ("POLYGON((" + ossimString::toString(ul.lond()) + " "
                         + ossimString::toString(ul.latd()) + ","
                         + ossimString::toString(ll.lond()) + " "
                         + ossimString::toString(ll.latd()) + ","
                         + ossimString::toString(lr.lond()) + " "
                         + ossimString::toString(lr.latd()) + ","
                         + ossimString::toString(ur.lond()) + " "
                         + ossimString::toString(ur.latd()) + ","
                         + ossimString::toString(ul.lond()) + " "
                         + ossimString::toString(ul.latd()) + "))");

      groundShape += "\n\t\t\"groundShape\" : { \"type\" : \"polygon\", \"coordinates\" : [ [ [" + ossimString::toString(ul.lond()) + ", " + ossimString::toString(ul.latd()) + "], ["
			+ ossimString::toString(ur.lond()) + ", " + ossimString::toString(ur.latd()) + "], ["
			+ ossimString::toString(lr.lond()) + ", " + ossimString::toString(lr.latd()) + "], ["
			+ ossimString::toString(ll.lond()) + ", " + ossimString::toString(ll.latd()) + "], ["
			+ ossimString::toString(ul.lond()) + ", " + ossimString::toString(ul.latd()) + "] ] ] },";


      if (1)
      {
        writeJsonKVP(outputString, "groundGeom", groundGeometry, true, indent, path);
	writeJsonKVP(outputString, "groundGeomCounterClockwise", groundGeometryCounterClockwise, true, indent, path);
 	// This is only for ElasticSearch, which implies json
 	if (json) outputString += groundShape.string();	
      }
      else
      {
	writeJsonKVP(outputString, "FOOTPRINT", groundGeometryCounterClockwise, true, indent, path);
      } 

      if (1) 
      {
        ossimString tiePointsPoly = "";
        tiePointsPoly += ("POLYGON((" + ossimString::toString(rect.ul().x) + " "
                         + ossimString::toString(rect.ul().y) + ","
                         + ossimString::toString(rect.ur().x) + " "
                         + ossimString::toString(rect.ur().y) + ","
                         + ossimString::toString(rect.lr().x) + " "
                         + ossimString::toString(rect.lr().y) + ","
                         + ossimString::toString(rect.ll().x) + " "
                         + ossimString::toString(rect.ll().y) + ","
                         + ossimString::toString(rect.ul().y) + " "
                         + ossimString::toString(rect.ul().x) + "))");
        ossimString tiePointsCounterClockwisePoly = "";
        tiePointsCounterClockwisePoly += ("POLYGON((" + ossimString::toString(rect.ul().x) + " "
                         + ossimString::toString(rect.ul().y) + ","
                         + ossimString::toString(rect.ll().x) + " "
                         + ossimString::toString(rect.ll().y) + ","
                         + ossimString::toString(rect.lr().x) + " "
                         + ossimString::toString(rect.lr().y) + ","
                         + ossimString::toString(rect.ur().x) + " "
                         + ossimString::toString(rect.ur().y) + ","
                         + ossimString::toString(rect.ul().y) + " "
                         + ossimString::toString(rect.ul().x) + "))");

	ossimString tiePoints = "";
        tiePoints += "<TiePointSet version=''1''>";
        tiePoints += "<Image>";
        tiePoints += "<coordinates>";
        tiePoints += ossimString::toString(rect.ul().x).string() + "," + ossimString::toString(rect.ul().y).string() + " ";
        tiePoints += ossimString::toString(rect.ur().x).string() + "," + ossimString::toString(rect.ur().y).string() + " ";
        tiePoints += ossimString::toString(rect.lr().x).string() + "," + ossimString::toString(rect.lr().y).string() + " ";
        tiePoints += ossimString::toString(rect.ll().x).string() + "," + ossimString::toString(rect.ll().y).string();
        tiePoints += "</coordinates>";
        tiePoints += "</Image>";
        tiePoints += "<Ground>";
        tiePoints += "<coordinates>";
        tiePoints += ossimString::toString(ul.lond()).string() + "," + ossimString::toString(ul.latd()).string() + " ";
        tiePoints += ossimString::toString(ur.lond()).string() + "," + ossimString::toString(ur.latd()).string() + " ";
        tiePoints += ossimString::toString(lr.lond()).string() + "," + ossimString::toString(lr.latd()).string() + " ";
        tiePoints += ossimString::toString(ll.lond()).string() + "," + ossimString::toString(ll.latd()).string();
        tiePoints += "</coordinates>";
        tiePoints += "</Ground>";
        tiePoints += "</TiePointSet>";

	writeJsonKVP(outputString, "tiePoints", tiePoints, true, indent, path);
        writeJsonKVP(outputString, "tiePointsPoly", tiePointsPoly, true, indent, path);
	writeJsonKVP(outputString, "tiePointsCounterClockwisePoly", tiePointsCounterClockwisePoly, true, indent, path);	
      }
   }
}
void jsonAppend(ossimRefPtr<ossimProperty> property,
                       std::string& outputString, const std::string& indentation,
                       const std::string& separator)
{
   if (property.valid())
   {
      ossimContainerProperty* container =
      dynamic_cast<ossimContainerProperty*> (property.get());
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
                  jsonAppend(prop.get(), outputString,
                                    indentation, separator);
               }
            }
         }
         else
         {
            ossimString containerName = container->getName();
            ossimString newIndentation = indentation + "   ";
            
            outputString += indentation + "<tag name=\"" + containerName.string()
               + "\">" + separator;
            
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
                     jsonAppend(prop.get(), outputString,
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
            outputString += indentation + "<tag name=\""
               + property->getName().string() + "\">" + value.string() + "</tag>"
               + separator;
         }
      }
   }
}

void oms::ImageInfo::appendRasterDataSetMetadata(std::string& outputString,
                                                const std::string& indentation,
                                                const std::string& separator) const
{
   std::vector<ossimRefPtr<ossimProperty> > properties;
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
            * container =
            dynamic_cast<ossimContainerProperty*> (properties[idx].get());
            if (container)
            {
               ossim_uint32 n = container->getNumberOfProperties();
               for (idx2 = 0; idx2 < n; ++idx2)
               {
                  ossimRefPtr<ossimProperty> prop =
                  container->getProperty(idx2);
                  if (prop.valid())
                  {
                     jsonAppend(prop.get(), outputString,
                                       indentation + "   ", separator);
                  }
               }
            }
            outputString += indentation + "</metadata>" + separator;
         }
      }
   }
}

static ossimString jsonMonthStringToNumberString(const ossimString& monthString)
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

std::string oms::ImageInfo::convertIdatimToXmlDate(const std::string& idatim) const
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
         month = jsonMonthStringToNumberString(month.downcase());
         
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

std::string oms::ImageInfo::convertAcqDateToXmlDate(const std::string& value) const
{
   if (value.size() == 8) // assume 4 character year 2 month and 2 day format
   {
      return (ossimString(value.begin(), value.begin() + 4) + "-"
              + ossimString(value.begin() + 4, value.begin() + 6) + "-"
              + ossimString(value.begin() + 6, value.begin() + 8));
   }
   if (value.size() == 14) // assume 4 character year 2 month and 2 day 2 hour 2 minute 2 seconds
   {
      return (ossimString(value.begin(), value.begin() + 4) + "-"
              + ossimString(value.begin() + 4, value.begin() + 6) + "-"
              + ossimString(value.begin() + 6, value.begin() + 8) + "T"
              + ossimString(value.begin() + 8, value.begin() + 10) + ":"
              + ossimString(value.begin() + 10, value.begin() + 12) + ":"
              + ossimString(value.begin() + 12, value.begin() + 14) + "Z");
   }
   return "";
}

void oms::ImageInfo::appendRasterEntryDateTime(std::string& outputString,
                                              ossim_uint32 indent, ossimString path) const
{
   ossimRefPtr<ossimProperty> prop;
   ossimString dateValue = "";
   dateValue = convertIdatimToXmlDate(
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
      if (1) writeJsonKVP(outputString, "acquisitionDate", dateValue, true, indent, path); 
      else writeJsonKVP(outputString, "IMAGE_DATE_TIME", dateValue, true, indent, path);
   }
}

void oms::ImageInfo::appendMetadataDetails(std::string& outputString, const ossimKeywordlist& kwl) const
{
      jsonOutputKeywordList jsonkwl(pretty, json);
      jsonkwl.getMap() =  kwl.getMap();
      std::ostringstream out;
      out << jsonkwl;
      outputString += out.str();
}

void oms::ImageInfo::appendNormalizedMetadata(std::string& outputString, const ossimKeywordlist& kwl,
                                              ossim_uint32 indent, ossimString path) const
{
   ossimString dateValue = "";
   ossimString beNumber;
   ossimString imageRepresentation;
   ossimString targetId;
   ossimString productId;
   //ossimString sensorId;
   ossimString missionId;
   ossimString countryCode;
   ossimString imageCategory;
   ossimString azimuthAngle;
   ossimString grazingAngle;
   ossimString securityClassification;
   ossimString title;
   ossimString organization;
   ossimString description;
   ossimString niirs;
   ossimString sunElevation;
   ossimString sunAzimuth;
   ossimString cloudCover;
   ossimString isctlh;
   ossimString squintAngle;
   ossimString slopeAngle;
   ossimString shadowAngle;
   ossimString layoverAngle;
   ossimString rollAngle;

   jsonOutputKeywordList kwl3;
   kwl3.getMap() =  kwl.getMap();
   kwl3.downcaseKeywords();
      
   grazingAngle ="";
   azimuthAngle ="";
   ossimString imagePrefix = "image" + ossimString::toString(thePrivateData->theImageHandler->getCurrentEntry()) + ".";
      
   if(kwl3.find("nitf.fhdr")) // are we nitf
   {
      if(!kwl3.find("nitf.rpf.byte_order")) // if we are not a cib/cadrg a.toc file
      {
         ossimString nitfPrefix = ossimString("nitf.")+imagePrefix;
         if(ossimString(kwl3.find("nitf.fhdr")).contains("NITF02.0"))
         {
            title = ossimString(kwl3.find(nitfPrefix+"ititle"));
         }
         else 
         {
            title = ossimString(kwl3.find(nitfPrefix+"iid2"));
         }
         // parse nitf information
         securityClassification = ossimString(kwl3.find(nitfPrefix+"isclas"));
         targetId = ossimString(kwl3.find(nitfPrefix+"tgtid"));
         if(!targetId.empty())
         {
            if(targetId.size() >=10)
            {
               // should be a 17 character ID
               //
               beNumber    = ossimString(targetId.begin(), targetId.begin()+10);
               if(targetId.size() >=17)
               {
                  countryCode = ossimString(targetId.begin()+15, targetId.end());
               }
            }
         }
         targetId = targetId.trim();
         missionId = ossimString(kwl3.find(nitfPrefix+"isorce"));
         imageCategory = ossimString(kwl3.find(nitfPrefix+"icat"));
         imageRepresentation = ossimString(kwl3.find(nitfPrefix+"irep"));
         countryCode = ossimString(kwl3.find(nitfPrefix+"stdidc.country"));
         if(missionId.empty())
         {
            countryCode = ossimString(kwl3.find(nitfPrefix+"stdidc.mission"));
         }
         // I want it relative to Y axis not x for angle to north.
         //
         
         grazingAngle = ossimString(kwl3.find(nitfPrefix+"use00a.oblang"));
         if (grazingAngle.empty()) grazingAngle = ossimString(kwl3.find(nitfPrefix+"use26a.obl_ang"));

         if(!grazingAngle.empty())
         {
            grazingAngle = ossimString::toString(90.0 - grazingAngle.toDouble());
         }
         organization = ossimString(kwl3.find("nitf.oname"));
      }
      else 
      {
         ossimString nitfPrefix = ossimString("nitf.rpf.")+imagePrefix;
         securityClassification = ossimString(kwl3.find(nitfPrefix+"SecurityClassification"));
         productId = ossimString(kwl3.find(nitfPrefix+"ProductDataType"))+(ossimString(kwl.find(nitfPrefix+"Scale")));
         countryCode = ossimString(kwl3.find(nitfPrefix+"CountryCode"));
         organization = ossimString(kwl3.find("nitf.oname"));
      }
   }
   else if(kwl3.find("dted.uhl.recognition_sentinel"))
   {
      grazingAngle = "90.0";
      azimuthAngle = ossimString(kwl3.find("dted.dsi.orientation"));
      securityClassification = ossimString(kwl3.find("dted.dsi.security_code"));
      productId = kwl3.find("dted.dsi.product_level");
      
      if(productId.downcase().contains("rted"))
      {
         ossimString numberOfLatPoints(kwl3.find("dted.uhl.number_of_lat_points"));
         if(!numberOfLatPoints.empty())
         {
            if(numberOfLatPoints == "3601")
            {
               productId = "DTED2";
            }
            else if(numberOfLatPoints == "1201")
            {
               productId = "DTED1";
            }
            else if(numberOfLatPoints == "0121")
            {
               productId = "DTED0";
            }
            else 
            {
               productId = "DTED";
            }
            
         }
      }
      organization = ossimString(kwl3.find("dted.dsi.producer_code"));
   }
   else if(kwl3.find("tiff." + imagePrefix + "date_time") || kwl3.find("jp2.tiff." + imagePrefix + "date_time"))
   {
      ossimString tiffDate(kwl3.find("tiff." + imagePrefix + "date_time"));
      if (tiffDate.empty()) tiffDate = ossimString(kwl3.find("jp2.tiff." + imagePrefix + "date_time"));
      std::vector<ossimString> splitArray;
      tiffDate.split(splitArray, " ");
      if(splitArray.size() > 0)
      {
         dateValue = splitArray[0].substitute(":", "-", true);
         if(splitArray.size() > 1)
         {
            dateValue += "T";
            dateValue += splitArray[1];
            dateValue += "Z";
         }
         else 
         {
            dateValue += "T00:00:00Z";
         }
	 writeJsonKVP(outputString, "acquisitionDate", dateValue, true, indent);
      }
   }

   ossimString format = thePrivateData->formatName();
   bool hasCommon = format.contains("tfrd") || format.contains("nitf");
   if (hasCommon)
   {
	if(dateValue.empty())
	{
          if (kwl3.find(format + "." + imagePrefix + "common.acquisition_date"))
          {
             dateValue = ossimString(kwl3.find(format + "." + imagePrefix + "common.acquisition_date"));
	     writeJsonKVP(outputString, "acquisitionDate", dateValue, true, indent);
          } 
        }
   }

   if(azimuthAngle.empty())
   {
      azimuthAngle = "0";
      
      if(thePrivateData->theImageHandler.valid())
      {
         ossimRefPtr<ossimImageGeometry> geom = thePrivateData->theImageHandler->getImageGeometry();
         ossimDrect rect = thePrivateData->theImageHandler->getBoundingRect();
         if(geom.valid())
         {
            ossimGpt centerGpt;
            ossimGpt upGpt;
            ossimDpt centerDpt(rect.midPoint());
            ossimDpt upDpt = centerDpt + ossimDpt(0,-rect.height()*.5);
            geom->localToWorld(centerDpt, centerGpt);
            geom->localToWorld(upDpt, upGpt);
            azimuthAngle = ossimString::toString(centerGpt.azimuthTo(upGpt));
         }
      }
   }

   if (hasCommon)
   {
     if(countryCode.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.cc"))
       {
	  countryCode = ossimString(kwl3.find(format + "." + imagePrefix + "common.cc"));
       } 
     }
     if(niirs.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.niirs"))
       {
          niirs = ossimString(kwl3.find(format + "." + imagePrefix + "common.niirs"));
       }
     }
     if (kwl3.find(format + "." + imagePrefix + "common.sunel"))
     {
        sunElevation = ossimString(kwl3.find(format + "." + imagePrefix + "common.sunel"));
     }
     if (kwl3.find(format + "." + imagePrefix + "common.sunaz"))
     {
        sunAzimuth = ossimString(kwl3.find(format + "." + imagePrefix + "common.sunaz"));
     }
     if (kwl3.find(format + "." + imagePrefix + "common.mission"))
     {
          missionId = ossimString(kwl3.find(format + "." + imagePrefix + "common.mission"));
     }
     if (kwl3.find(format + "." + imagePrefix + "common.mission_id"))
     {
          missionId = ossimString(kwl3.find(format + "." + imagePrefix + "common.mission_id"));
     }
     if(cloudCover.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.cloud_cover"))
       {
          cloudCover = ossimString(kwl3.find(format + "." + imagePrefix + "common.cloud_cover"));
       }
     }
     if(isctlh.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.isctlh"))
       {
          isctlh = ossimString(kwl3.find(format + "." + imagePrefix + "common.isctlh"));
       }
     }
     if(isctlh.empty())
     {
       if (kwl3.find(format + ".common.isctlh"))
       {
          isctlh = ossimString(kwl3.find(format + ".common.isctlh"));
       }
     }
     if(squintAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.squint_angle"))
       {
          squintAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.squint_angle"));
       }
     }
     if(grazingAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.grazing_angle"))
       {
          grazingAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.grazing_angle"));
       }
     }
     if(grazingAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.elevation_angle"))
       {
          grazingAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.elevation_angle"));
       }
     }
     if(slopeAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.slope_angle"))
       {
          slopeAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.slope_angle"));
       }
     }
     if(shadowAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.shadow_angle"))
       {
          shadowAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.shadow_angle"));
       }
     }
     if(rollAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.roll_angle"))
       {
          rollAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.roll_angle"));
       }
     }
     if(layoverAngle.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.layover_angle"))
       {
          layoverAngle = ossimString(kwl3.find(format + "." + imagePrefix + "common.layover_angle"));
       }
     }
     if(beNumber.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.be"))
       {
          beNumber = ossimString(kwl3.find(format + "." + imagePrefix + "common.be"));
       }
     }
     if(beNumber.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.secondary_be_0"))
       {
          beNumber = ossimString(kwl3.find(format + "." + imagePrefix + "common.secondary_be_0"));
       }
     }
     if(securityClassification.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.classification"))
       {
          securityClassification = ossimString(kwl3.find(format + "." + imagePrefix + "common.classification"));
	  if (!securityClassification.empty()) securityClassification = ossimString(securityClassification.substr(0,1)).trim();
       }
     }
     if(imageCategory.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.sensor_id"))
       {
          imageCategory = ossimString(kwl3.find(format + "." + imagePrefix + "common.sensor_id"));
       }
     }
     if(title.empty())
     {
       if (kwl3.find(format + "." + imagePrefix + "common.ititle"))
       {
          title = ossimString(kwl3.find(format + "." + imagePrefix + "common.ititle"));
       }
       // Override from iid2 tag
       if (kwl3.find(format + "." + imagePrefix + "iid2"))
       {
          title = ossimString(kwl3.find(format + "." + imagePrefix + "iid2"));
       }
     }
   }

   writeJsonKVP(outputString, "filename", thePrivateData->theFilename.trim(), true, indent, path);
   writeJsonKVP(outputString, "imageId", title.trim(), true, indent, path);
   writeJsonKVP(outputString, "imageRepresentation", imageRepresentation.trim(), true, indent, path);
//   writeJsonKVP(outputString, "targetId", targetId.trim(), true, indent, path);
   writeJsonKVP(outputString, "productId", productId.trim(), true, indent, path);
   writeJsonKVP(outputString, "beNumber", beNumber.trim(), true, indent, path);
   //writeJsonKVP(outputString, "sensorId", sensorId.trim(), true, indent, path);
   writeJsonKVP(outputString, "missionId", missionId.trim(), true, indent, path);
   writeJsonKVP(outputString, "countryCode", countryCode.trim(), true, indent, path);
   writeJsonKVP(outputString, "imageCategory", imageCategory.trim(), true, indent, path);
   writeJsonKVP(outputString, "azimuthAngle", azimuthAngle.trim(), true, indent, path);
   writeJsonKVP(outputString, "securityClassification", securityClassification.trim(), true, indent, path);
   writeJsonKVP(outputString, "title", title.trim(), true, indent, path);
   writeJsonKVP(outputString, "organization", organization.trim(), true, indent, path);
   writeJsonKVP(outputString, "description", description.trim(), true, indent, path);
   writeJsonKVP(outputString, "niirs", niirs.trim(), true, indent, path);
   writeJsonKVP(outputString, "sunElevation", sunElevation.trim(), true, indent, path);
   writeJsonKVP(outputString, "sunAzimuth", sunAzimuth.trim(), true, indent, path);
   writeJsonKVP(outputString, "cloudCover", cloudCover.trim(), true, indent, path);
   writeJsonKVP(outputString, "isctlh", isctlh.trim(), true, indent, path);
   //writeJsonKVP(outputString, "squintAngle", squintAngle.trim(), true, indent, path);
   writeJsonKVP(outputString, "grazingAngle", grazingAngle.trim(), true, indent, path);
   //writeJsonKVP(outputString, "slopeAngle", slopeAngle.trim(), true, indent, path);
   writeJsonKVP(outputString, "shadowAngle", shadowAngle.trim(), true, indent, path);
   writeJsonKVP(outputString, "className", thePrivateData->theImageHandler.valid()?thePrivateData->theImageHandler->getClassName():ossimString(""), false, indent, path);
   if (!json) outputString += "\n";
}
