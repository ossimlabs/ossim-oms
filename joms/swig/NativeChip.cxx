#include "NativeChip.h"

NativeChip::NativeChip(ossimRefPtr<ossimImageData> chip)  
    : ossimReferenced()
{
    m_chip = chip;
}

NativeChip::~NativeChip()  {}

ossim_uint32 NativeChip::getHeight() const {
    return m_chip->getHeight();
}

ossim_uint32 NativeChip::getNumberOfBands() const {
    return m_chip->getNumberOfBands();
}

ossim_uint32 NativeChip::getSizeInBytes() const {
    return m_chip->getSizeInBytes();
}

ossim_uint32 NativeChip::getWidth() const {
    return m_chip->getWidth();
}

ossimScalarType NativeChip::getScalarType() const {
    return m_chip->getScalarType();
}

void NativeChip::unloadTile(void *dest, const ossimIrect &destRect, ossimInterleaveType ilType) const {
    return m_chip->unloadTile(dest, destRect,  ilType);
}

ossimIrect NativeChip::getImageRectangle() const {
    return m_chip->getImageRectangle();
}
