#ifndef ossimTools_HEADER
#define ossimTools_HEADER 1

#include <oms/Constants.h>
#include <string>
#include <iostream>
#include <map>

class ossimTool;

namespace oms
{

class OMSDLL OssimTools
{
public:
   OssimTools(std::string name);
   ~OssimTools();
   bool initialize(const std::map<std::string,std::string>& params);
   bool execute(char* outstreambuf);
   bool getChip(ossim_int8* buffer, const std::map<std::string, std::string>& hints);

private:
   ossimTool* m_utility;
   static bool m_locked;

};

}

#endif
