#include "STPoint.h"
#include "util.h"

//extern float calculateDistance(float LatA, float LonA, float LatB, float LonB); 

//using namespace std;

// 数据成员初始化是在进入构造函数之前完成的
//STPoint::STPoint() {
//	lat = 0;
//	lon = 0;
//	stpoint_id = 0;
//
//	std::vector<Keywordtuple> kwtmp;
//	keywords = kwtmp;
//
//	std::vector<int> bjtmp;
//	belongtraj = bjtmp;
//
//}




float STPoint::CalcPPSTSim(const STPoint &p) const{
	//spacial
	float ssim = 0;
	ssim = 1 - calculateDistance(this->lat, this->lon, p.lat, p.lon) / MAX_DIST;
	
	//textual
	float tsim = 0;
	
	for (size_t i = 0; i < this->keywords.size(); i++) {
		for (size_t j = 0; j < p.keywords.size(); j++) {
			if (this->keywords[i].keywordid == p.keywords[j].keywordid) {
				tsim += this->keywords[i].keywordvalue*p.keywords[j].keywordvalue;
				break; // 单个点不会出现重复的keyword
			}
		}
	}

#ifdef DIS_SIM_RESULT	
	printf(" ssim= %.5f, tsim= %.5f \n", ssim, tsim);
#endif

	return(ALPHA * ssim + (1- ALPHA) * tsim);
}