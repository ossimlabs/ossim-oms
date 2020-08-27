#ifndef NativeChipper_HEADER
#define NativeChipper_HEADER 1

#include <ossim/base/ossimReferenced.h>
#include <ossim/util/ossimChipperUtil.h>
#include <map>
#include <string>
#include "NativeChip.h"

class OSSIM_DLL NativeChipper : public ossimReferenced
{
public:
   /** default constructor */
   NativeChipper();

   /** virtual destructor */
   virtual ~NativeChipper();

    NativeChip* run(std::map<std::string,std::string>& opts);
};

#endif /* #ifndef NativeChipper_HEADER */
