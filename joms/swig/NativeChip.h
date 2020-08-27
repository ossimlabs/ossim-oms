#ifndef NativeChip_HEADER
#define NativeChip_HEADER 1

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/imaging/ossimImageData.h>

#include <map>
#include <string>

class OSSIM_DLL NativeChip : public ossimReferenced
{
public:
    /** default constructor */
    NativeChip(ossimRefPtr<ossimImageData> chip);

    /** virtual destructor */
    virtual ~NativeChip();

    virtual ossim_uint32 getHeight() const;
    virtual ossim_uint32 getNumberOfBands() const;    
    virtual ossim_uint32 getSizeInBytes() const;
    virtual ossim_uint32 getWidth() const;

    virtual ossimScalarType getScalarType() const;
    virtual ossimIrect getImageRectangle() const;
    virtual void unloadTile(void *dest, const ossimIrect &destRect, ossimInterleaveType ilType) const;

private:
    ossimRefPtr<ossimImageData> m_chip;
};

#endif /* #ifndef NativeChip_HEADER */
