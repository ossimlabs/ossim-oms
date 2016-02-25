#ifndef ossimTools_HEADER
#define ossimTools_HEADER 1

#include <oms/Constants.h>
#include <string>
#include <map>

class ossimChipProcUtil;

namespace oms
{

class OMSDLL OssimTools
{
public:
   OssimTools(std::string name);
   ~OssimTools();
   bool initialize(const std::map<std::string,std::string> params);
   bool getChip(char* buffer, const std::map<std::string, std::string> hints);

private:
   ossimChipProcUtil* m_chipProcUtil;
};

}

#endif
