#ifndef ossimTools_HEADER
#define ossimTools_HEADER

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
	bool initialize(std::map<std::string,std::string> params);
	bool getChip(char* buffer, std::map<std::string, std::string> hints);

private:
	ossimChipProcUtil* m_chipProcUtil;
};
}

#endif
