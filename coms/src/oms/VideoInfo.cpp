#include <oms/VideoInfo.h>
#include <oms/Util.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRegExp.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>
#include <ossim/base/ossimDate.h>
#include <ossim/projection/ossimBilinearProjection.h>
#include <ossimPredator/ossimPredatorVideo.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/GEOSException.h>
#include <ossimPredator/ossimVMTITrack.h>

#include <sstream>

namespace oms
{
   class VideoInfoPrivateData
   {
   public:
      ~VideoInfoPrivateData()
      {
         clear();
      }
      void clear()
      {
         thePredatorVideo = 0;
         thePredatorVideoFrameInfo = 0;
         theExternalVideoGeometryFile = "";
      }
      std::string formatName() const
      {
         if(thePredatorVideo.valid())
         {
            return "mpeg";
         }
         return "unspecified";
      }

      ossimFilename theFilename;
      ossimRefPtr<ossimPredatorVideo> thePredatorVideo;
      ossimRefPtr<ossimPredatorVideo::FrameInfo> thePredatorVideoFrameInfo;
      ossimFilename                              theExternalVideoGeometryFile;
   };
}
      

oms::VideoInfo::VideoInfo() :
thePrivateData(new oms::VideoInfoPrivateData),
pretty(true),
showsummary(true),
showdetails(true),
coredata(false),
omardb(true),
multi(true),
json(true),
skipSeconds(1.0)
{
}

oms::VideoInfo::~VideoInfo()
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

bool oms::VideoInfo::open(const std::string& file, bool failIfNoGeometryFlag)
{
   bool result = false;
     thePrivateData->clear();
   
     if(canCheckVideo(file) )
         {
            thePrivateData->theFilename = file;
            thePrivateData->thePredatorVideo = new ossimPredatorVideo();
            if(thePrivateData->thePredatorVideo->open(
                  thePrivateData->theFilename))
            {
	       result = true;
		/*
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
		*/
            }
         }

   if (result == false)
   {
      thePrivateData->clear();
   }
   //   std::cout << "OPEN FILE? " << result << ", " << file << std::endl;
   
   return result;
}

void oms::VideoInfo::close()
{
   if (thePrivateData)
   {
      thePrivateData->clear();
   }
}

void geomUnion(geos::geom::Geometry* geom, geos::geom::Geometry** composite)
{
            if(!*composite)
            {
               if(geom->isValid())
               {
                  *composite = geom;
               }
            }
            else if(geom)
            {
               try
               {
                  if(geom->isValid())
                  {
                     geos::geom::Geometry* newGeom = geom->Union(*composite);

                     if(geom) delete geom;
                     delete *composite;
                     *composite = newGeom;

                  }
               }
               catch(...)
               {
                  if(geom) delete geom;
                  geom = 0;
               }
            }
}

bool oms::VideoInfo::updateTrackInfo(ossimRefPtr<ossimVMTITrack> track, ossimRefPtr<ossimPredatorKlvTable> klvTable, ossimRefPtr<ossimImageGeometry> geom, ossim_uint32 index) const
{
  ossim_uint32 frameNumber, frameWidth, frameHeight, targetIntensity;
  ossim_uint64 timestamp;

  // Update ground geometry information
  ossimDpt centroid, ul, ur, ll, lr;
  ossimGpt gul, gur, gll, glr, gcentroid;
  klvTable->getVMTIFrameNumber(frameNumber);
  klvTable->getVMTIPointFromPixel(centroid, index, 0);
  klvTable->getVMTIPointFromPixel(ul, index, 1);
  klvTable->getVMTIPointFromPixel(lr, index, 2);
  ur.x=lr.x;
  ur.y=ul.y;
  ll.x=ul.x;
  ll.y=lr.y;
  geom->localToWorld(ul, gul);
  geom->localToWorld(ur, gur);
  geom->localToWorld(ll, gll);
  geom->localToWorld(lr, glr);
  geom->localToWorld(centroid, gcentroid);
  ossimString polygon = "POLYGON(("
                       +ossimString::toString(gul.lond())+" "
                       +ossimString::toString(gul.latd())+","
                       +ossimString::toString(gur.lond())+" "
                       +ossimString::toString(gur.latd())+","
                       +ossimString::toString(glr.lond())+" "
                       +ossimString::toString(glr.latd())+","
                       +ossimString::toString(gll.lond())+" "
                       +ossimString::toString(gll.latd())+","
                       +ossimString::toString(gul.lond())+" "
                       +ossimString::toString(gul.latd())+" "
                       +"))";

  geos::io::WKTReader reader;
  geos::geom::Geometry* groundTrack = 0;
  try{
      groundTrack = reader.read(polygon);
  }
  catch(...)
  {
  }
  geos::geom::Geometry* composite = track->getGroundTrack();
  geomUnion(groundTrack, &composite);
  track->setGroundTrack(composite);
  ossimString point = "POINT("
                       +ossimString::toString(gcentroid.lond())+" "
                       +ossimString::toString(gcentroid.latd())
                       +")";

  geos::geom::Geometry* centroidTrack = 0;
  try{
      centroidTrack = reader.read(point);
  }
  catch(...)
  {
  }
  geos::geom::Geometry* centroidComposite = track->getCentroidTrack();
  geomUnion(centroidTrack, &centroidComposite);  
  track->setCentroidTrack(centroidComposite);


  klvTable->getVMTIUnixMicroTime(timestamp);
  klvTable->getVMTITargetIntensity(targetIntensity, index);
  track->updateIntensities(targetIntensity);
  track->updateTimes(timestamp);
  track->updateFrames(frameNumber);
  ossimString klvTrackMetadata = "{";
  klvTrackMetadata += "\"TIME\": \"" + track->getDate(timestamp) + "\", ";
  klvTrackMetadata += "\"FRAME_NUMBER\": \"" + ossimString::toString(frameNumber) + "\", "; 
  klvTrackMetadata += "\"TARGET_CENTROID_TRACK\": \"" + point + "\", ";
  klvTrackMetadata += "\"TARGET_FOOTPRINT\": \"" + polygon + "\"";
  klvTrackMetadata += "}";

  track->updateKlvTrackMetadata(klvTrackMetadata);

  return true;
}

void multiGeometry(std::string& /*result*/,
                           ossimRefPtr<ossimPredatorKlvTable> /*klvTable*/,
                           const std::string& /*indentation*/,
                           const std::string& /*separator*/)
{

}

geos::geom::Geometry* geomFromSensorPosition(ossimRefPtr<ossimPredatorKlvTable> klvTable, ossim_float64& lat, ossim_float64& lon)
{
   ossim_float64 elev;
   geos::geom::Geometry* result = 0;
   if(klvTable->getSensorPosition(lat, lon, elev))
   {
     geos::io::WKTReader reader;

     try{
        result = reader.read("POINT("
                       +ossimString::toString(lon)+" "
                       +ossimString::toString(lat)
                       +")");

     }
     catch(...)
     {

     }

   }
   return result;
}


geos::geom::Geometry*  geomFromKlv(ossimRefPtr<ossimPredatorKlvTable> klvTable)
{
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

   geos::io::WKTReader reader;
   geos::geom::Geometry* result = 0;


   try{
      result = reader.read("MULTIPOLYGON((("
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
   catch(...)
   {

   }

   return result;
}

void videoGeom(std::string& result,
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
   // RPALKO -modified to the end of videoGeom
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

std::string oms::VideoInfo::getInfo() const
{
   std::string result = "";
     /*
       if(!thePrivateData->theExternalVideoGeometryFile.empty()&&
              thePrivateData->theExternalVideoGeometryFile.exists())
      {
         std::vector<char> buf;
         ossim_uint64 filesize = thePrivateData->theExternalVideoGeometryFile.fileSize();
         if(filesize>0)
         {
            ifstream in(thePrivateData->theExternalVideoGeometryFile.c_str(), std::ios::in|std::ios::binary);
            if(in.good())
            {
               buf.resize(filesize);
               in.read(&buf.front(), filesize);

               result = std::string(buf.begin(), buf.end());
            }
         }
      }
	*/
      //if(thePrivateData->thePredatorVideoFrameInfo.valid())
      //{
	 if (json) result += "{";

         //thePrivateData->thePredatorVideo->rewind();
         ossim_uint32 numberOfFrames = 0;
         ossim_uint32 numberOfKlv = 0;
  	 thePrivateData->thePredatorVideo->countFrames(numberOfFrames, numberOfKlv);
	 // Close and re-open as PTS/DTS discontinuities are causing rewind/seek to work unreliably for MPEG-TS dumps
	 thePrivateData->thePredatorVideo->close();
	 thePrivateData->thePredatorVideo->open(thePrivateData->theFilename);
         //ossim_uint32 idx = 0;
         // safer to rewind
         geos::geom::Geometry* composite = 0;
	 geos::geom::Geometry* sensorComposite = 0;
         std::vector<geos::geom::Geometry*> vmtiGroundTrack;
	 std::vector<geos::geom::Geometry*> vmtiCentroidTrack;
         //thePrivateData->thePredatorVideo->rewind();

	 //ossim_uint32 numberOfKlv = 0;
         trackMap tracks;
         ossimRefPtr<ossimPredatorVideo::KlvInfo> klvInfo = thePrivateData->thePredatorVideo->nextKlv();
	 ossim_uint32 klvProcessed=0;
         ossimDate startDate;
         ossimDate endDate;
         klvInfo->table()->getDate(startDate, true);
         klvInfo->table()->getDate(endDate, true);

 	 bool needFileClassification = true, needNumberOfFrames = true, needCountryCode = true, needDissemination = true, needCompartment = true, needSensorFormat = true, needSensor = true;
	 ossimString fileClassification, sensor, sensorFormat, countryCode, compartment, dissemination;
	 ossimString unionGeom = "", sensorUnionGeom = ""; 
	 ossim_float64 lastlat=0.0, lastlon=0.0;
	 ossim_float64 mingsd=1000000.0, maxgsd=0.0;
	 // Plan to deprecate the next two in favor of a combined field
         //ossimString klvFootprintMetadata = "[\n\t\t";
	 //ossimString klvSensorMetadata = "[\n\t\t";
	 ossimString klvMetadata = "[\n\t\t";
	 ossim_int64 initialepoc; 
     	 klvInfo->table()->getUnixEpocTimestamp(initialepoc);

	 bool first = true;

	 // Loop through all KLV frames, grabbing the metadata
	 // this code does a union of all the polygons
         while(klvInfo.valid() &&klvInfo->table())
         {
	    ++klvProcessed;
	    if (!first) 
	    {
	      //klvFootprintMetadata += ",\n\t\t";
	      //klvSensorMetadata += ",\n\t\t";
	      klvMetadata += ",\n\t\t";
	    }
 	    //klvFootprintMetadata+=klvInfo->table()->getFootprintMetadata();
	    //klvSensorMetadata+=klvInfo->table()->getSensorMetadata();
	    klvMetadata+=klvInfo->table()->getMetadata(initialepoc);
	    //++numberOfKlv;           
	    // Grab some fields on first occurrence, as they should not change
	    if (needFileClassification && klvInfo->table()->getClassification(fileClassification)) needFileClassification = false;
	    if (needSensor && klvInfo->table()->getSensor(sensor)) needSensor = false;
	    if (needSensorFormat && klvInfo->table()->getSensorFormat(sensorFormat)) needSensorFormat = false;
	    if (needCountryCode && klvInfo->table()->getCountryCode(countryCode)) needCountryCode = false; 
	    if (needCompartment && klvInfo->table()->getSecurityInformation(compartment)) needCompartment = false;
 	    if (needDissemination && klvInfo->table()->getReleaseInstructions(dissemination)) needDissemination = false;

            ossimRefPtr<ossimImageGeometry> imagegeom = new ossimImageGeometry();
            ossimDpt ul(0,0);
            ossimDpt ur(0,thePrivateData->thePredatorVideo->imageWidth());
            ossimDpt lr(thePrivateData->thePredatorVideo->imageHeight(),thePrivateData->thePredatorVideo->imageWidth());
            ossimDpt ll(thePrivateData->thePredatorVideo->imageHeight(),0);
            ossimGpt llg, ulg, urg, lrg;
            if(klvInfo->table()->getCornerPoints(ulg, urg, lrg, llg))
            {
              ossimRefPtr<ossimProjection> proj = new ossimBilinearProjection( ul, ur, lr, ll, ulg, urg, lrg, llg );
              if (proj.valid())
              {
                 imagegeom->setProjection( proj.get() );
              }
            }
            ossimDpt gsdPt = imagegeom->getMetersPerPixel();
            ossim_float64 gsd = (gsdPt.x + gsdPt.y) * 0.5 * FT_PER_MTRS * 12.0;
            if (gsd < mingsd) mingsd = gsd;
            if (gsd > maxgsd) maxgsd = gsd;

	    ossim_uint32 numberOfReportedTargets = 0;
	    klvInfo->table()->getNumberOfReportedTargets(numberOfReportedTargets); 
            for (int i=0; i<numberOfReportedTargets; ++i)
	    {
	      //klvInfo->table()->
	      ossim_uint32 targetid;
	      klvInfo->table()->getTargetID(targetid, i);
	      //std::cout << "TARGET ID: " << ossimString::toString(targetid) << std::endl;
	      trackMap::iterator i2 = tracks.find(targetid); 
	      ossimRefPtr<ossimVMTITrack> track = 0;
	      if(i2 == tracks.end())
   	      {
	        track = new ossimVMTITrack;
	        tracks.insert(std::make_pair(targetid, track));
		track->setTargetID(targetid);
	      }
	      else
 	      {
		track = i2->second;
	      }
	      track->updateGsds(gsd);
	      updateTrackInfo(track, klvInfo->table(), imagegeom, i);
	    }

	    // footprints
	    ossim_float64 lat, lon;
	
            geos::geom::Geometry* geom = geomFromKlv(klvInfo->table());
	    geos::geom::Geometry* sensorGeom = geomFromSensorPosition(klvInfo->table(), lat, lon);
	    geomUnion(geom, &composite);
	    if (!ossim::almostEqual(lastlat, lat) || !ossim::almostEqual(lastlon, lon)) 
	    {
		geomUnion(sensorGeom, &sensorComposite);
	        lastlat = lat;
                lastlon = lon;
	    }
            if (skipSeconds > 0.0)
            {
	      ossim_uint32 skipKlvs = skipSeconds * thePrivateData->thePredatorVideo->videoFrameRate();
	      for (int i=0; i<skipKlvs; ++i)
              {
		++klvProcessed;
		if (klvProcessed >= numberOfKlv) 
		{
	          klvProcessed--;
                  break;
		}
                thePrivateData->thePredatorVideo->skipKlv();
              }

	      // Seek not working properly due to PTS/DTS rollbacks in stream	
              //thePrivateData->thePredatorVideo->seek(skipSeconds, ossimPredatorVideo::SEEK_RELATIVE);
              //thePrivateData->thePredatorVideo->adjustSeek();

	      /*
              ossim_uint32 skipFrames = skipSeconds * thePrivateData->thePredatorVideo->videoFrameRate();
              for (int i=0; i<skipFrames; ++i)
              {
                thePrivateData->thePredatorVideo->nextFrame();
              }
	      */
            }
	    klvInfo->table()->getDate(endDate, true);
            klvInfo = thePrivateData->thePredatorVideo->nextKlv();
	    if (first) first = false;
            //idx++;
         }
	 //std::cout << "KLV PROCESSED: " << ossimString::toString(klvProcessed) << std::endl;
         if(composite)
         {
            geos::io::WKTWriter writer;
            try
            {
               unionGeom = writer.write(composite);
            }
            catch(...)
            {

            }
            delete composite;
            composite = 0;
         }
         if(sensorComposite)
         {
            geos::io::WKTWriter writer;
            try
            {
               sensorUnionGeom = writer.write(sensorComposite);
            }
            catch(...)
            {

            }
            delete sensorComposite;
            sensorComposite = 0;
         }
         //ossim_uint32 numberOfFrames = (thePrivateData->thePredatorVideo->videoFrameRate()*thePrivateData->thePredatorVideo->duration());
	 ossim_uint32 numberOfTracks = tracks.size(); 
         //klvFootprintMetadata+="\n\t]";
         //klvSensorMetadata+="\n\t]";
	 klvMetadata+="\n\t]";
  
         writeJsonKVP(result, "DIR_ID", thePrivateData->theFilename.path().file().string(), true, 1);
         writeJsonKVP(result, "FILE_ID", thePrivateData->theFilename.fileNoExtension().string(), true, 1);
         writeJsonKVP(result, "FILE_FORMAT", thePrivateData->formatName(), true, 1);
         writeJsonKVP(result, "WIDTH", ossimString::toString(thePrivateData->thePredatorVideo->imageWidth()).string(), true, 1);
         writeJsonKVP(result, "HEIGHT", ossimString::toString(thePrivateData->thePredatorVideo->imageHeight()).string(), true, 1);
         writeJsonKVP(result, "FILE_SIZE", ossimString::toString(thePrivateData->theFilename.fileSize()), true, 1);
         appendDateRange(result, startDate, endDate, true, 1);
	 writeJsonKVP(result, "NUMBER_OF_FRAMES", ossimString::toString(numberOfFrames), true, 1);
         writeJsonKVP(result, "NUMBER_OF_KLV_FRAMES", ossimString::toString(numberOfKlv), true, 1);
	 writeJsonKVP(result, "NUMBER_OF_TRACKS", ossimString::toString(numberOfTracks), true, 1);
	 writeJsonKVP(result, "MIN_GSD", ossimString::toString(mingsd), true, 1);
	 writeJsonKVP(result, "MAX_GSD", ossimString::toString(maxgsd), true, 1);
	 writeJsonKVP(result, "SENSOR", sensor, true, 1);
	 writeJsonKVP(result, "SENSOR_FORMAT", sensorFormat, true, 1);
	 writeJsonKVP(result, "COUNTRY_CODE_3DIGIT", countryCode, true, 1);
	 writeJsonKVP(result, "FILE_CLASSIFICATION", fileClassification, true, 1);
         writeJsonKVP(result, "FILENAME", thePrivateData->theFilename.file().string(), true, 1);
         writeJsonKVP(result, "PATH", thePrivateData->theFilename.path().string(), true, 1);
         writeJsonKVP(result, "SENSOR_TRACK", sensorUnionGeom.gsub("MULTIPOINT", "LINESTRING"), true, 1);
	 writeJsonKVP(result, "FOOTPRINT", unionGeom, true, 1);
	 writeJsonKVP(result, "COMPARTMENT", compartment, true, 1);
	 writeJsonKVP(result, "DISSEMINATION", dissemination, true, 1);
	 writeJsonKVP(result, "KLV_METADATA", klvMetadata, true, 1, false);
	 result+="\n\t\"TRACKS\": [";
        
         trackMap::iterator iter = tracks.begin();
	 bool comma = false;
         while(iter!=tracks.end())
         {
	    iter->second->updateKlvTrackMetadata(ossimString("\n\t\t\t]"), false);
	    result += iter->second->getJson(3, comma).c_str();
	    comma = true;
	    ++iter;
 	 }
	 result += "\n\t]\n}\n";
   //}
   return result;
}

bool oms::VideoInfo::getKwl(ossimKeywordlist& kwl) const
{
   
   // Do a print to a memory stream.
   std::ostringstream out;
   out.str( getInfo() );

   std::istringstream in(out.str());
   // Give the result to the keyword list.
   return kwl.parseStream(in);
}

void oms::VideoInfo::writeJsonKVP(std::string& outputString, ossimString key, ossimString value, bool comma, ossim_uint32 tabcount, bool quote, ossimString path) const
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
    if (quote) outputString += "\"" + key.string() + "\": \"" + value.string() + "\"";	 
    else outputString += "\"" + key.string() + "\": " + value.string();
    if (comma) outputString += ",";
  }
  else
  {
    outputString += "\n" + path.string() + key.string() + ": " + value.string();
  }
}

void oms::VideoInfo::appendDateRange(std::string& outputString, const ossimDate& startDate,
                     const ossimDate& endDate, bool comma, ossim_uint32 tabcount) const
{
   std::ostringstream outStart, outEnd;
   //double roundStart = ((int) ((startDate.getSec()
   //                             + startDate.getFractionalSecond()) * 1000)) / 1000.0;
   //double roundEnd =
   //((int) ((endDate.getSec() + endDate.getFractionalSecond()) * 1000))
   ossim_uint32 roundStart = startDate.getFractionalSecond() * 1000;
   ossim_uint32 roundEnd = endDate.getFractionalSecond() * 1000;
   /// 1000.0;
   outStart << std::setw(4) << std::setfill('0') << startDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << startDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << startDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << startDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << startDate.getSec() << "."
   << std::setw(3) << std::setfill('0') << roundStart;
  
   outEnd << std::setw(4) << std::setfill('0') << endDate.getYear() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getMonth() << "-"
   << std::setw(2) << std::setfill('0') << endDate.getDay() << "T"
   << std::setw(2) << std::setfill('0') << endDate.getHour() << ":"
   << std::setw(2) << std::setfill('0') << endDate.getMin() << ":"
   << std::setw(2) << std::setfill('0') << endDate.getSec() << "."
   << std::setw(3) << std::setfill('0') << roundEnd;

   writeJsonKVP(outputString, "VIDEO_START_TIME", outStart.str(), true, 1);
   writeJsonKVP(outputString, "VIDEO_END_TIME", outEnd.str(), true, 1);
}

