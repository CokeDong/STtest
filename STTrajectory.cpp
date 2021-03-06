#include "STTrajectory.h"

//using namespace std;

void STTrajectory::GettingSTPointOnpointID(std::vector<STPoint> &pointdb) {

	//for_each(traj_of_stpoint_id.cbegin(), traj_of_stpoint_id.cend(), ?);
	for (std::vector<int>::iterator it = traj_of_stpoint_id.begin(); it != traj_of_stpoint_id.end(); it++) {
		traj_of_stpoint.push_back(pointdb.at(*it));
	}
	
}

float STTrajectory::CalcTTSTSim(const STTrajectory &stt) const {

	// Spacial + Textual
	float stsim12 = 0;
	for (size_t i = 0; i < this->traj_length; i++) {
		float tmpmax = 0;
		for (size_t j = 0; j < stt.traj_length; j++) {
			//float tmpmax = 0;
			float ppsim = this->traj_of_stpoint[i].CalcPPSTSim(stt.traj_of_stpoint[j]);
			if (ppsim > tmpmax) tmpmax = ppsim; 
		}
		stsim12 += tmpmax;
	}
	stsim12 /= this->traj_length;
	
	float stsim21 = 0;
	for (size_t j = 0; j < stt.traj_length; j++) {
		float tmpmax = 0;
		for (size_t i = 0; i < this->traj_length; i++) {
			//float tmpmax = 0;
			float ppsim = stt.traj_of_stpoint[j].CalcPPSTSim(this->traj_of_stpoint[i]);
			if (ppsim > tmpmax) tmpmax = ppsim;
		}
		stsim21 += tmpmax;
	}
	stsim21 /= stt.traj_length;
#ifdef DIS_SIM_RESULT
	printf("\n**** sim12= %.5f, sim21=%.5f *****\n\n\n", stsim12, stsim21);
#endif
	//cout << stsim12 << '\t' << stsim21 << '\n';
	return(stsim12 + stsim21);
}
