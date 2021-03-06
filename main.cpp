#include "ConstDefine.h"
#include "preprocess.h"
#include "STPoint.h"
#include "STTrajectory.h"
#include "STInvertedList.h"
#include "STGrid.h"
#include "test.h"
#include "util.h"


// using namespace std; // cuda cusparse库 not recommended


//extern void split2(std::string s, std::string delim, std::vector<std::string>* ret); // for test
//extern void GetKeywords(std::vector<std::string>* keywords, std::vector<std::string>* corpus);


// global really good ?? why global? not global 跨文件使用 poor coding ? how to improve really have meaning ?
// global is a poor choice!!

//// std::vector push_back 全部比较慢
//
//std::vector<STPoint> pointDB; // only points in ram
//std::vector<STTrajectory> trajDB; // still need
//
//// for filtering 
//std::vector<STInvertedList> invertedlist;

void CheckSimResult(std::vector<trajPair> paircpu, std::vector<float> valuecpu, std::vector<trajPair> pairgpu, std::vector<float> valuegpu);

std::vector<float> cpuonethreadtimes;
std::vector<float> cpumthreadtimes;
std::vector<float> gpucoarsetimes;
std::vector<float> gpufinetimes;
std::vector<float> gpuonefinetimes;

std::vector<float> gpufinenoFliptimes;
std::vector<float> gpufinenoSortingtimes;

int timeseeds;


float ave(std::vector<float> a) {
	float sum = 0;
	for (int i = 0; i < a.size(); ++i) {
		sum += a.at(i);
	}
	sum /= a.size();
	return sum;
}


int main() {

	using namespace std;

	std::cout << "hello world" << endl;
	bool first = 0; // buggy, not recommended but I.
	Preprocess pp;
	std::string sfilename = "LA";
	if(first){
		// can be done by python. -> 见 python 预处理  to do
		// fooolly extraction
		pp.VenuesExtraction("./"+ sfilename+"/Venues.txt","./" + sfilename + "/VenuesExtc.txt");
		pp.TipsExtraction("./" + sfilename + "/Tips.txt");
	}

	//test test;
	//test.testfunc();

	// std::vector push_back 全部比较慢 all into ram!!  & or * 传递
	std::vector<STPoint> pointDB; // only points in ram
	std::vector<STTrajectory> trajDB; // still need

	// for filtering 
	std::vector<STInvertedList> invertedlist;

	// in this way only because of 预处理 2次 PointDB not good enough
	// slow time-consuming for large # of points and traj
	// 引用传递 比指针传递 更简单

	// 已验证 无问题！！but too slow!! -> stringstream v.s. std::string, no big difference!!

	pp.ReadPointDBLL(pointDB, "./" + sfilename + "/VenuesExtc.txt");
	pp.ReadPointDBKeyword(pointDB, "./" + sfilename + "/tfidf.txt");
	pp.ReadTrajDBPointID(trajDB, "./" + sfilename + "/TrajExtc.txt", pointDB);

	pp.ReadTrajDBPoint(trajDB, pointDB);

	//std::map<trajPair, float> result;
	std::vector<trajPair> resultpair;
	std::vector<float> resultvalue;



	// aborted not accurate. as 1 and extreme data exists
	//// adding status info.
	size_t lenavg = 0;
	size_t keyavg = 0;
	
	size_t trajcnt = 0;
	size_t pointcnt = 0;
	for (int i = 0; i < trajDB.size(); ++i) {
		if(trajDB.at(i).traj_length > 1 && trajDB.at(i).traj_length < 25){
			lenavg += trajDB.at(i).traj_length;
			++trajcnt;
			pointcnt += trajDB.at(i).traj_of_stpoint.size();
			for (int j = 0; j < trajDB.at(i).traj_of_stpoint.size(); ++j) {
				for (int k = 0; k < trajDB.at(i).traj_of_stpoint.at(j).keywords.size(); ++k) {
					if (trajDB.at(i).traj_of_stpoint.at(j).keywords.at(k).keywordvalue > 0.1)
						keyavg++;
				}
				
			}
		}
	}
	lenavg /= trajcnt;
	keyavg /= pointcnt;
	printf("trajcnt = %zu pointcnt= %zu lenavg = %zu  keyavg = %zu\n", trajcnt, pointcnt,lenavg, keyavg);



	// 频繁调参使用变量！！不需要重新make -> figure of scalability
	int SIZE = 128; // this is good or gloabal parameter not #define? maybe more convenient


	int times = 10;
	
	for (int tlooptimes = 0; tlooptimes < times; ++tlooptimes) {
		
		timeseeds = tlooptimes;
		
		STGrid grid;
		grid.init(trajDB); // clever！！






		/********************  CPU version here. ********************/


		// 单线程
		printf("*************** 1-cpu ***************\n");
		std::vector<trajPair> resultpaircpu;
		std::vector<float> resultvaluecpu;
		// for equality, we have to padding for CPU?? -----> no need!!
		grid.joinExhaustedCPUonethread(SIZE, SIZE, resultpaircpu, resultvaluecpu,3);
		

		/*
		// 多线程版本
		printf("***** mul-cpu full *****\n");
		std::vector<trajPair> resultpairmcpu;
		std::vector<float> resultvaluemcpu;
		//grid.joinExhaustedCPU(SIZE,SIZE, resultpairmcpu, resultvaluemcpu,3);

		// if SIZE*SIZE too big, we get Resource temporarily unavailabl
		printf("***** cpu  %d *****\n", MAX_CPU_THREAD);
		grid.joinExhaustedCPUconfigurablethread(SIZE, SIZE, resultpairmcpu, resultvaluemcpu, MAX_CPU_THREAD, 3); // not that accurate!!
		*/







		 /********************  GPU version here. ********************/


		/*
		//// baseline的算法 很慢
		printf("*************** 1-gpu coarse(baseline) ****************\n");
		std::vector<trajPair> resultpaircoarsegpu;
		std::vector<float> resultvaluecoarsegpu;
		//grid.joinExhaustedGPU(SIZE, SIZE, resultpaircoarsegpu, resultvaluecoarsegpu);
		grid.joinExhaustedGPU_Final(SIZE, SIZE, resultpaircoarsegpu, resultvaluecoarsegpu, 0, 3);
		//CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpaircoarsegpu, resultvaluecoarsegpu);
		*/


		//// this is worse than Zero Copy, but very very tiny
		//printf("***** 1-gpu coarse No Zero Copy*****\n");
		//std::vector<trajPair> resultpaircoarsegpunzc;
		//std::vector<float> resultvaluecoarsegpunzc;
		//grid.joinExhaustedGPUNZC(SIZE, SIZE, resultpaircoarsegpunzc, resultvaluecoarsegpunzc);
		////CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpaircoarsegpu, resultvaluecoarsegpu);




		//// 不能太长轨迹 否则显存不足 -> 数据预处理 自定义三个矩阵尤其第一个占用过大显存 不要太极端 已写 assert
		//printf("*************** 1-gpu fine no-budget-schedular ***************\n");
		//std::vector<trajPair> resultpairfinegpu;
		//std::vector<float> resultvaluefinegpu;
		////grid.joinExhaustedGPUV2(SIZE, SIZE, resultpairfinegpu, resultvaluefinegpu);
		//grid.joinExhaustedGPU_Final(SIZE, SIZE, resultpairfinegpu, resultvaluefinegpu, 1,3);
		////CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpairfinegpu, resultvaluefinegpu);



		//// not that important for performance improvement  -> 合并多个kernel
		//printf("***** 1-gpu V2p1 fine *****\n");
		//std::vector<trajPair> resultpairfinegpu2;
		//std::vector<float> resultvaluefinegpu2;
		//grid.joinExhaustedGPUV2p1(SIZE, SIZE, resultpairfinegpu2, resultvaluefinegpu2);
		////CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpairfinegpu2, resultvaluefinegpu2);



		//// 不能太长轨迹 否则显存不足 -> 数据预处理 用的是cusparse库 中间结果需要显存 不要太极端 已写 assert
		//printf("*************** 1-gpu V3 fine cusparse nobatch ***************\n");
		//std::vector<trajPair> resultpairfinegpu4;
		//std::vector<float> resultvaluefinegpu4;
		//grid.joinExhaustedGPUV4(SIZE, SIZE, resultpairfinegpu4, resultvaluefinegpu4);
		////CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpairfinegpu4, resultvaluefinegpu4);




		bool noB = 0;
		if(!noB){
			// sorting
			// 可以任意长轨迹 这里构成batch很重要
			printf("*************** 1-gpu V3 fine sorting ***************\n");
			std::vector<trajPair> resultpairfinegpu3;
			std::vector<float> resultvaluefinegpu3;
			//grid.joinExhaustedGPUV3(SIZE, SIZE, resultpairfinegpu3, resultvaluefinegpu3);
			grid.joinExhaustedGPU_Final(SIZE, SIZE, resultpairfinegpu3, resultvaluefinegpu3, 2, 3);
			//CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpairfinegpu3, resultvaluefinegpu3);
		}else
		{		
			printf("*************** 1-gpu V3 fine sorting ***************\n");
			std::vector<trajPair> resultpairfinegpu3;
			std::vector<float> resultvaluefinegpu3;
			//grid.joinExhaustedGPUV3(SIZE, SIZE, resultpairfinegpu3, resultvaluefinegpu3);
			grid.joinExhaustedGPU_Final(SIZE, SIZE, resultpairfinegpu3, resultvaluefinegpu3, 2, 4);
			//CheckSimResult(resultpairmcpu, resultvaluemcpu, resultpairfinegpu3, resultvaluefinegpu3);
			
		}
		


	
	}

	printf("\n\n********** time average here: ***********\n");
	cout << "1cpu:   " << ave(cpuonethreadtimes) << endl;
	cout << "1gpu:   " << ave(gpuonefinetimes) << endl;
	cout << "mcpu:   " << ave(cpumthreadtimes) << endl;
	cout << "gpu-c:   " << ave(gpucoarsetimes) << endl;
	cout << "gpu-f   " << ave(gpufinetimes) << endl;





	//sleep(10);
	std::cout << "finished" << endl;
#ifdef DIS_RESULT
	getchar();
	getchar();
	getchar();
#endif
	return 0;
}


void CheckSimResult(std::vector<trajPair> paircpu, std::vector<float> valuecpu, std::vector<trajPair> pairgpu, std::vector<float> valuegpu) {
	
	// fully output: #define EPSILON -1

	for (size_t i = 0; i < paircpu.size(); i++) {
		if (i < pairgpu.size()) {
			printf("%zu\n(%zu,%zu):%.5f\n(%zu,%zu):%.5f\n\n", i, paircpu[i].first, paircpu[i].second, valuecpu[i], pairgpu[i].first, pairgpu[i].second, valuegpu[i]);
		}
		else {
			printf("%zu\n(%zu,%zu):%.5f\n(%zu,%zu):%.5f\n\n", i, paircpu[i].first, paircpu[i].second, valuecpu[i], 0, 0, 0);
		}
	}
	if (paircpu.size() < pairgpu.size()) {
		for (size_t i = paircpu.size(); i < pairgpu.size(); i++) {
			printf("%zu\n(%zu,%zu):%.5f\n(%zu,%zu):%.5f\n\n", i, 0, 0, 0, pairgpu[i].first, pairgpu[i].second, valuegpu[i]);
		}
	}
	

	//int falsecnt = 0;
	//// check pair
	//for (size_t i = 0; i < paircpu.size(); i++) {
	//		if (i < pairgpu.size()) {
	//			if (paircpu[i] != pairgpu[i]) {
	//				printf("False %zu\n(%zu,%zu):%.5f\n(%zu,%zu):%.5f\n\n", i, paircpu[i].first, paircpu[i].second, valuecpu[i], pairgpu[i].first, pairgpu[i].second, valuegpu[i]);
	//				falsecnt++;
	//			}
	//		}
	//	}
	//if (!falsecnt) printf("True!\n");

}