#include "NativeChipper.h"

NativeChipper::NativeChipper()
    : ossimReferenced()
{

}   

NativeChipper::~NativeChipper()
{
//   clear();
}

NativeChip* NativeChipper::run(std::map<std::string,std::string>& opts)
{
    ossimChipperUtil chipper;

    chipper.initialize(opts);

    return new NativeChip( chipper.getChip(opts) );    
}
