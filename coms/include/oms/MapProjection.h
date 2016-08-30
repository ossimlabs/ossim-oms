#ifndef omsMapProjection_HEADER
#define omsMapProjection_HEADER
#include <oms/Constants.h>
#include <string>

class ossimGpt;
class ossimDpt;
namespace oms
{

	class OMSDLL MapProjection{
	public:
		MapProjection();
		~MapProjection();

		void createUtmProjection(const ossimGpt& worldPt);
		void createTransMercProjection(const ossimGpt& worldPt);
		bool createEpsgProjection(const ossimGpt& worldPt,
			                       const std::string& epsgCode);

		bool worldToLocal(const ossimGpt& worldPt, ossimDpt& localPt)const;
		bool localToWorld(const ossimDpt& localPt, ossimGpt& worldPt)const;

	protected:
	   class PrivateData;
	   PrivateData* m_privateData;

	};

}

#endif
