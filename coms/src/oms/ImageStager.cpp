#include <oms/ImageStager.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimProcessListener.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/imaging/ossimOverviewBuilderFactoryRegistry.h>
#include <ossim/imaging/ossimOverviewBuilderBase.h>
#include <ossim/imaging/ossimHistogramWriter.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimScaleFilter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimHistogramRemapper.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <iostream>
#include <mutex>
#include <oms/DataInfo.h>

    class oms::ImageStager::PrivateData : public ossimProcessListener

{
public:
  PrivateData()
      : m_quietFlag(false),
        m_useFastHistogramStagingFlag(false),
        m_stageHistogramFlag(false),
        m_overviewType("ossim_tiff_box"),
        m_compressionType("NONE"),
        m_compressionQuality(100),
        m_stageOverviewFlag(false),
        m_stageThumbnailFlag(false),
        m_currentProcessInterface(0),
        m_cancelFlag(false),
        m_entryId(-1),
        m_thumbnailSize(512),
        m_thumbnailStretchType(ossimHistogramRemapper::LINEAR_AUTO_MIN_MAX),
        m_thumbnailType("jpeg")
  {
  }

  void initHandler()
  {
    m_handler = ossimImageHandlerRegistry::instance()->open(m_filename);
  }

  void cancel()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cancelFlag = true;
  }

  bool isCancelled() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cancelFlag;
  }

  void setCancelFlag(bool flag)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cancelFlag = flag;
  }

  void setDefaults()
  {
    if (!m_handler.valid())
    {
      initHandler();
      initEntry();
    }
    ossim::defaultTileSize(m_overviewTileSize);
    m_stageHistogramFlag   = false;
    m_stageOverviewFlag    = false;
    m_histogramFilename    = "";
    m_overviewFilename     = "";
    m_thumbnailFilename    = "";
    if (m_handler.valid())
    {
      m_stageOverviewFlag = !(m_handler->hasOverviews());
      m_overviewFilename = m_handler->createDefaultOverviewFilename();
      m_histogramFilename = m_handler->createDefaultHistogramFilename();
      m_thumbnailFilename = m_handler->getFilenameWithThisExtension(""); 
      m_stageHistogramFlag = !(m_histogramFilename.exists());
      m_stageThumbnailFlag = !(m_thumbnailFilename.exists());
    }
  }

  ossim_uint32 getNumberOfEntries() const
  {
    ossim_uint32 result = 0;

    if (m_handler.valid())
    {
      result = m_handler->getNumberOfEntries();
    }

    return result;
  }

  bool changeEntry(ossim_uint32 entryId)
  {
    bool result = false;
    if (m_handler.valid())
    {
      result = m_handler->setCurrentEntry(entryId);
      m_overviewFilename = m_handler->createDefaultOverviewFilename();
      m_histogramFilename = m_handler->createDefaultHistogramFilename();
      m_stageOverviewFlag = !(m_handler->hasOverviews());
      m_stageHistogramFlag = !(m_histogramFilename.exists());
    }
    if (result)
    {
      m_entryId = entryId;
    }

    return result;
  }

  int getCurrentEntry() const
  {
    int result = -1;

    if (m_handler.valid())
    {
      result = m_handler->getCurrentEntry();
    }

    return result;
  }

  void setOverviewTileSize(int width, int height)
  {
    m_overviewTileSize = ossimIpt(width, height);
  }

  bool buildOverviews()
  {
    bool result = false;
    if (!initEntry())
      return result;
    ossimRefPtr<ossimOverviewBuilderBase> builder = ossimOverviewBuilderFactoryRegistry::instance()->createBuilder(m_overviewType);
    if (!builder.valid())
    {
      return result;
    }

    ossimPropertyInterface *builderProp = (ossimPropertyInterface *)builder.get();
    builderProp->setProperty(ossimKeywordNames::OUTPUT_TILE_SIZE_KW, m_overviewTileSize.toString());
    builderProp->setProperty(ossimKeywordNames::COMPRESSION_TYPE_KW, m_compressionType);
    builderProp->setProperty(ossimKeywordNames::COMPRESSION_QUALITY_KW, ossimString::toString(m_compressionQuality));
    builder->setInputSource(m_handler.get());

    m_currentProcessInterface = builder.get();
    builder->addListener((ossimProcessListener *)this);
    if (m_quietFlag)
    {
      //    ossimPushNotifyFlags();
      //   ossimDisableNotify();
    }

    if (m_useFastHistogramStagingFlag)
    {
      builder->setHistogramMode(OSSIM_HISTO_MODE_FAST);
    }
    else
    {
      builder->setHistogramMode(OSSIM_HISTO_MODE_NORMAL);
    }
    if (m_overviewFilename.path().isWriteable())
    {
      result = true;
      builder->setOutputFile(m_overviewFilename);
      builder->execute();
    }

    builder->removeListener((ossimProcessListener *)this);
    if (m_quietFlag)
    {
      //  ossimPopNotifyFlags();
    }
    builder = 0;
    m_currentProcessInterface = 0;

    if(m_handler)
    {
      std::shared_ptr<ossim::ImageHandlerState> state = m_handler->getState();

      if(state) state->setOverviewState(0);
      
      m_handler->openOverview();
    }
    return result;
  }

  bool buildHistograms()
  {
    if (!initEntry())
      return false;
    ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
    ossimRefPtr<ossimHistogramWriter> writer = new ossimHistogramWriter;
    histoSource->connectMyInputTo(0, m_handler.get());
    histoSource->enableSource();
    if (m_useFastHistogramStagingFlag)
    {
      histoSource->setComputationMode(OSSIM_HISTO_MODE_FAST);
    }
    writer->connectMyInputTo(0, histoSource.get());

    writer->setFilename(m_histogramFilename);
    writer->addListener((ossimProcessListener *)this);
    if (m_quietFlag)
    {
      //ossimPushNotifyFlags();
      //ossimDisableNotify();
    }
    m_currentProcessInterface = writer.get();
    writer->execute();
    writer->removeListener((ossimProcessListener *)this);
    if (m_quietFlag)
    {
      //ossimPopNotifyFlags();
    }
    writer->disconnect();
    histoSource->disconnect();
    m_currentProcessInterface = 0;

    return true;
  }

  bool buildThumbnail()
  {
    if (!initEntry())
      return false;
    ossimKeywordlist bandsKeywordList;
    bandsKeywordList.add("type", "ossimBandSelector");
    bandsKeywordList.add(ossimKeywordNames::BANDS_KW, "default");
    ossimRefPtr<ossimImageSource> bs = ossimImageSourceFactoryRegistry::instance()->createImageSource(bandsKeywordList);
    if(!bs) return false;
    ossimIrect cutRect;
    ossimRefPtr<ossimImageRenderer> renderer = new ossimImageRenderer();
    ossimRefPtr<ossimScalarRemapper> scalarRemapper = new ossimScalarRemapper();
    ossimRefPtr<ossimRectangleCutFilter> cutFilter = new ossimRectangleCutFilter();
    ossimRefPtr<ossimImageViewAffineTransform> trans = new ossimImageViewAffineTransform();

    ossimDrect bounds = m_handler->getBoundingRect();
    ossim_float64 maxSize = ossim::max(bounds.width(), bounds.height());
    ossim_float64 scale = m_thumbnailSize/maxSize;

    if(scale>1.0){
      scale = 1.0;
      cutRect = bounds;
    } 
    else
    {
      cutRect = bounds*scale;
    }
    cutFilter->setRectangle(cutRect);
    if ((scale < .5) && (m_handler->getNumberOfDecimationLevels() < 2))
    {
      return false;
    }
    trans->scale(scale,scale);
    renderer->setImageViewTransform(trans.get());
    if (!bs.valid()) return false;
    bs->connectMyInputTo(m_handler.get());
    bs->initialize();
    ossim_uint32 bandCount = bs->getNumberOfInputBands();
    if((bandCount == 2) || (bandCount > 3))
    {
      ossimBandSelector *tempBs = dynamic_cast<ossimBandSelector *>(bs.get());
      if (tempBs)
      {
        tempBs->setThreeBandRgb();
      }
    }

    ossimRefPtr<ossimHistogramRemapper> stretch = new ossimHistogramRemapper();
    stretch->setStretchMode(m_thumbnailStretchType);
    ossimFilename histogramFile = m_handler->createDefaultHistogramFilename();
    if(!m_histogramFilename.empty()) 
    {
      histogramFile = m_histogramFilename;
    }
    stretch->connectMyInputTo(bs.get());
    stretch->openHistogram(histogramFile);
    renderer->connectMyInputTo(stretch.get());
   
    scalarRemapper->connectMyInputTo(renderer.get());
    cutFilter->connectMyInputTo(scalarRemapper.get());
    ossimKeywordlist writerKwl;
    ossimFilename thumbnailFilename;
    if (m_thumbnailType == "png")
    {
      thumbnailFilename = ossimFilename(m_thumbnailFilename + "thumb.png");
    }
    else
    {
      thumbnailFilename = ossimFilename(m_thumbnailFilename + "thumb.jpg");
    }
    ossimString ext = thumbnailFilename.ext();
    ext = ext.downcase();
    writerKwl.add("type", "image/" + ext);
    writerKwl.add("filename", thumbnailFilename.c_str());
    writerKwl.add("create_external_geometry","false");
    if(ext=="png")
    {
      writerKwl.add("add_alpha_channel", "true");
    }
    ossimRefPtr<ossimImageFileWriter> writer = ossimImageWriterFactoryRegistry::instance()->createWriter(writerKwl);

    if(writer)
    {
      writer->connectMyInputTo(cutFilter.get());

      writer->execute();

      writer->disconnect();
      writer = 0;
    }
    scalarRemapper->disconnect();
    scalarRemapper = 0;
    renderer->disconnect();
    renderer = 0;
    stretch->disconnect();
    stretch = 0;
    bs->disconnect();
    bs = 0;

    return true;
  }
  virtual void processProgressEvent(ossimProcessProgressEvent & /* event */)
  {
    if (isCancelled())
    {
      if (m_currentProcessInterface)
      {
        m_currentProcessInterface->abort();
      }
    }
  }

  bool initEntry()
  {
    if (!m_handler.valid())
    {
      return false;
    }
    if (m_handler->getCurrentEntry() != (ossim_uint32)m_entryId)
    {
      if (m_entryId >= 0)
      {
        if (!m_handler->setCurrentEntry(m_entryId))
        {
          return false;
        }
      }
    }

    return true;
  }

  ossimRefPtr<ossimImageHandler> m_handler;
  ossimFilename m_filename;
  ossimFilename m_histogramFilename;
  ossimFilename m_thumbnailFilename;
  bool m_quietFlag;
  bool m_useFastHistogramStagingFlag;
  bool m_stageHistogramFlag;
  bool m_stageOverviewFlag;
  ossimFilename m_overviewFilename;
  ossimString m_overviewType;
  ossimString m_compressionType;
  int m_compressionQuality;
  bool m_stageThumbnailFlag;
  ossimProcessInterface *m_currentProcessInterface;
  bool m_cancelFlag;
  mutable std::mutex m_mutex;
  ossimIpt m_overviewTileSize;
  ossim_int32 m_entryId;
  ossim_uint32 m_thumbnailSize;
  ossimHistogramRemapper::StretchMode m_thumbnailStretchType;
  ossimString m_thumbnailType;

}; // End: class oms::ImageStager::PrivateData

oms::ImageStager::ImageStager() //const std::string& filename, ossim_int32 entryId)
    : m_privateData(new PrivateData)
{
  //	m_privateData->m_filename = ossimFilename(filename);
  //   m_privateData->m_entryId  = entryId;
  //   m_privateData->initHandler();
  //   m_privateData->initEntry();
}

oms::ImageStager::~ImageStager()
{
  if (m_privateData)
  {
    delete m_privateData;
    m_privateData = 0;
  }
}

bool oms::ImageStager::open(const std::string &filename, bool failIfNoGeom)
{
  bool result = true;
  m_privateData->m_filename = ossimFilename(filename);
  m_privateData->m_handler = 0;
  m_privateData->m_entryId = -1;

  m_privateData->setDefaults();
  if(m_privateData->m_handler.valid())
  {
    if(failIfNoGeom)
    {
      ossimRefPtr<ossimImageGeometry> geom = m_privateData->m_handler->getImageGeometry();
      if (!(geom && geom->hasProjection()))
      {
        result = false;
      }
    }
  }
  else
  {
    result = false;
  }

  return result;
}

unsigned int oms::ImageStager::getNumberOfEntries() const
{
  return m_privateData->getNumberOfEntries();
}

bool oms::ImageStager::setEntry(ossim_uint32 entryId)
{
  return m_privateData->changeEntry(entryId);
}
int oms::ImageStager::getCurrentEntry() const
{
  return m_privateData->getCurrentEntry();
}

void oms::ImageStager::setQuietFlag(bool flag)
{
  m_privateData->m_quietFlag = flag;
}

bool oms::ImageStager::getQuietFlag() const
{
  return m_privateData->m_quietFlag;
}

std::string oms::ImageStager::getInfo(bool failIfNoGeometryFlag) const
{
  std::string result = "";
  if (m_privateData->m_handler.valid())
  {
    ossimRefPtr<DataInfo> info = new DataInfo();
    if (info->setHandler(m_privateData->m_handler.get(), failIfNoGeometryFlag))
    {
      result = info->getImageInfo(m_privateData->m_handler->getCurrentEntry());
    }
  }

  return result;
}

std::string oms::ImageStager::getAllInfo(bool failIfNoGeometryFlag) const
{
  std::string result = "";
  if (m_privateData->m_handler.valid())
  {
    ossimRefPtr<DataInfo> info = new DataInfo();
    if (info->setHandler(m_privateData->m_handler.get(), failIfNoGeometryFlag))
    {
      result = info->getInfo();
    }
  }
  return result;
}

std::string oms::ImageStager::getFilename() const
{
  return m_privateData->m_filename;
}

void oms::ImageStager::setUseFastHistogramStagingFlag(bool flag)
{
  m_privateData->m_useFastHistogramStagingFlag = flag;
}

void oms::ImageStager::setHistogramStagingFlag(bool flag)
{
  m_privateData->m_stageHistogramFlag = flag;
}

void oms::ImageStager::setOverviewStagingFlag(bool flag)
{
  m_privateData->m_stageOverviewFlag = flag;
}

void oms::ImageStager::setThumbnailStagingFlag(bool flag, int size)
{
  m_privateData->m_stageThumbnailFlag = flag;
  m_privateData->m_thumbnailSize = size;
}

void oms::ImageStager::setThumbnailType(const std::string &type)
{
  ossimString typeString = ossimString(type).downcase();

  if((typeString != "jpeg") &&(typeString!="png"))
  {
    typeString = "jpeg";
  }
  m_privateData->m_thumbnailType = typeString;
}

void oms::ImageStager::setThumbnailStretchType(const std::string &type)
{
  ossimString typeString = type;

  typeString = typeString.downcase();

  if ((typeString == "auto-minmax"))
  {
    m_privateData->m_thumbnailStretchType = ossimHistogramRemapper::LINEAR_AUTO_MIN_MAX;
  }
  else if ((typeString == "auto-percentile"))
  {
    m_privateData->m_thumbnailStretchType = ossimHistogramRemapper::LINEAR_AUTO_PERCENTILE;
  }
  else if ((typeString == "std-stretch-1") || (typeString == "std-stretch 1"))
  {
    m_privateData->m_thumbnailStretchType = ossimHistogramRemapper::LINEAR_1STD_FROM_MEAN;
  }
  else if ((typeString == "std-stretch-2") || (typeString == "std-stretch 2"))
  {
    m_privateData->m_thumbnailStretchType = ossimHistogramRemapper::LINEAR_2STD_FROM_MEAN;
  }
  else if ((typeString == "std-stretch-3") || (typeString == "std-stretch 3"))
  {
    m_privateData->m_thumbnailStretchType = ossimHistogramRemapper::LINEAR_3STD_FROM_MEAN;
  }
}

void oms::ImageStager::setOverviewType(const std::string &overviewType)
{
  m_privateData->m_overviewType = overviewType;
}

void oms::ImageStager::setCompressionType(const std::string &compressionType)
{
  m_privateData->m_compressionType = compressionType;
}

void oms::ImageStager::setCompressionQuality(int compressionQuality)
{
  m_privateData->m_compressionQuality = compressionQuality;
}

void oms::ImageStager::setHistogramFilename(const std::string &file)
{
  m_privateData->m_histogramFilename = ossimFilename(file);
}

std::string oms::ImageStager::getHistogramFilename() const
{
  return m_privateData->m_histogramFilename;
}

void oms::ImageStager::setOverviewFilename(const std::string &file)
{
  m_privateData->m_overviewFilename = ossimFilename(file);
}

std::string oms::ImageStager::getOverviewFilename() const
{
  return m_privateData->m_overviewFilename;
}

void oms::ImageStager::setThumbnailFilename(const std::string &file)
{
  m_privateData->m_thumbnailFilename = ossimFilename(file);
}

std::string oms::ImageStager::getThumbnailFilename() const
{
  return m_privateData->m_thumbnailFilename;
}

void oms::ImageStager::disableAll()
{
  m_privateData->m_stageHistogramFlag = false;
  m_privateData->m_stageOverviewFlag = false;
}

void oms::ImageStager::setDefaults()
{
  m_privateData->setDefaults();
}

void oms::ImageStager::setOverviewTileSize(int width, int height)
{
  m_privateData->setOverviewTileSize(width, height);
}

bool oms::ImageStager::stage()
{
  bool status = false;

  m_privateData->setCancelFlag(false);

  if (m_privateData->m_stageOverviewFlag)
  {
    status = m_privateData->buildOverviews();
  }
  if (!m_privateData->m_stageOverviewFlag &&
      m_privateData->m_stageHistogramFlag &&
      !m_privateData->isCancelled())
  {
    status = m_privateData->buildHistograms();
  }
  if (m_privateData->m_stageThumbnailFlag)
  {
    if (m_privateData->m_handler->hasOverviews())
    {
      status = buildThumbnail();
    }
  } 
  return status;
}

bool oms::ImageStager::buildOverviews()
{
  bool status = false;

  if (m_privateData->m_stageOverviewFlag)
  {
    status = m_privateData->buildOverviews();
  }

  return status;
}

bool oms::ImageStager::buildAllOverviews()
{
  bool status = false;

  for (ossim_uint32 i = 0, numEntries = m_privateData->m_handler->getNumberOfEntries(); i < numEntries; i++)
  {
    this->setDefaults();
    this->setEntry(i);
    status |= buildOverviews();
  }

  return status;
}

bool oms::ImageStager::buildHistograms()
{
  bool status = false;

  if (m_privateData->m_stageHistogramFlag && !m_privateData->isCancelled())
  {
    status = m_privateData->buildHistograms();
  }

  return status;
}

bool oms::ImageStager::buildAllHistograms()
{
  bool status = false;

  for (ossim_uint32 i = 0, numEntries = m_privateData->m_handler->getNumberOfEntries(); i < numEntries; i++)
  {
    this->setDefaults();
    this->setEntry(i);
    status |= buildHistograms();
  }

  return status;
}

bool oms::ImageStager::buildThumbnail()
{
  bool status = false;

  if (m_privateData->m_stageHistogramFlag && !m_privateData->isCancelled())
  {
    status = m_privateData->buildThumbnail();
  }

  return status;
}

bool oms::ImageStager::buildAllThumbnails()
{
  bool status = false;

  for (ossim_uint32 i = 0, numEntries = m_privateData->m_handler->getNumberOfEntries(); i < numEntries; i++)
  {
    this->setDefaults();
    this->setEntry(i);
    status |= buildThumbnail();
  }

  return status;
}

bool oms::ImageStager::hasOverviews() const
{
  if (!m_privateData->m_handler.valid())
    return false;

  return m_privateData->m_handler->hasOverviews();
}

bool oms::ImageStager::hasHistograms() const
{
  return m_privateData->m_histogramFilename.exists();
}

void oms::ImageStager::cancel()
{
  m_privateData->cancel();
}

bool oms::ImageStager::isCancelled()const
{
  return m_privateData->isCancelled();
}

bool oms::ImageStager::stageAll()
{
  bool status = false;

  m_privateData->initEntry();

  if (m_privateData->m_handler.valid())
  {

    for (ossim_uint32 i = 0, numEntries = m_privateData->m_handler->getNumberOfEntries(); i < numEntries; i++)
    {
      this->setDefaults();
      this->setEntry(i);
      status |= this->stage();
    }
  }

  return status;
}
