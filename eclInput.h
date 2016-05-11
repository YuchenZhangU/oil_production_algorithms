#pragma once
#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include"read_write.h"


//----------------- generate tVec pVec pairs ------------------
void addFlatIntV(std::vector<double>& tVec, std::vector<double>& pVec,
	double flatP, double start, double end, double intV){
	size_t i = 0;
	double n = 2;
	double invSum = 0;
	while (true){
		i++;
		invSum += intV;
		double time = start + invSum;
		if (time >= end){
			tVec.push_back(end - time + intV);
			pVec.push_back(flatP);
			break;
		}
		tVec.push_back(intV);
		pVec.push_back(flatP);
		//intV = n*intV;
		//intV = intV < 180 ? intV : 180;
	}
}

void addDropIntV(std::vector<double>& tVec, std::vector<double>& pVec,
	double up, double low, double start, double end, double intV){
	double slope = (up - low) / (end - start);
	size_t i = 1;
	while (true){
		double time = start + i*intV;
		double BHP = up - slope*intV*i;
		if (time >= end){
			tVec.push_back(end - time + intV);
			pVec.push_back(low);
			break;
		}
		tVec.push_back(intV);
		pVec.push_back(BHP);
		i++;
	}
}


void drop(std::vector<double>& pVec, std::vector<double>& tVec, double Pi, double Pend, double dropSlope, double dt){
	double p = Pi;
	while (true){
		if (p - dt*dropSlope > Pend){
			p = p - dt*dropSlope;
			tVec.push_back(dt);
			pVec.push_back(p);
		}
		else{
			tVec.push_back((p - Pend) / dropSlope);
			pVec.push_back(Pend);
			break;
		}
	}
}

void hold(std::vector<double>& pVec, std::vector<double>& tVec, double Pi,double Time, double dt){
	double dT = 0;
	while (true){
		if (dT + dt < Time){
			dT += dt;
			pVec.push_back(Pi);
			tVec.push_back(dt);
		}
		else{
			pVec.push_back(Pi);
			tVec.push_back(Time - dT);
			break;
		}
	}
}

void holdIncreaseT(std::vector<double>& pVec, std::vector<double>& tVec, double Pi, double Time, double dt){
	double dT = 0;
	while (true){
		if (dT + dt < Time){
			dT += dt;
			pVec.push_back(Pi);
			tVec.push_back(dt);
			dt = dt*1.5;
		}
		else{
			pVec.push_back(Pi);
			tVec.push_back(Time - dT);
			break;
		}
	}
}



void dropHoldSch(std::vector<double>& pVec, std::vector<double>& tVec,
	double Pi, double Pb, double Plow, size_t Nc, double dropInv,
	double h_d_ratio, double dt, double dropSlope, double last_hold){
	
	if (Nc*dropSlope*dropInv>Pb - Plow){
		std::cerr << "Too many drop-hold circle, the BHP will drop to lower bound!" << std::endl;
	}
	
	pVec.clear();
	tVec.clear();
	pVec.push_back(Pi);
	tVec.push_back(1e-6);
	// generate drop schedule before Pb
	drop(pVec, tVec, Pi, Pb, dropSlope, dt);

	//generate drop-hold schedule
	double pDrop = dropSlope*dropInv;
	double holdInv = dropInv*h_d_ratio;
	for (size_t i=1; i < Nc; ++i){
		drop(pVec, tVec, pVec.back(), pVec.back() - pDrop, dropSlope, dt);
		hold(pVec, tVec, pVec.back(), holdInv, dt);
	}
	//generate last drop-hold schedule, the last hold maybe very short
	drop(pVec, tVec, pVec.back(), pVec.back() - pDrop, dropSlope, dt);
	hold(pVec, tVec, pVec.back(), last_hold, dt);
}


void dropHoldAllSch(std::vector<double>& pVec, std::vector<double>& tVec,
	double Pi, double Pb, double Plow, size_t Nc, double dropInv,
	double h_d_ratio, double dt, double dropSlope, double Time){

	if (Nc*dropSlope*dropInv>Pb - Plow){
		std::cerr << "Too many drop-hold circle, the BHP will drop to lower bound!" << std::endl;
	}

	pVec.clear();
	tVec.clear();
	pVec.push_back(Pi);
	tVec.push_back(1e-4);
	// generate drop schedule before Pb
	drop(pVec, tVec, Pi, Pb, dropSlope, dt);

	//generate drop-hold schedule
	double pDrop = dropSlope*dropInv;
	double holdInv = dropInv*h_d_ratio;
	for (size_t i = 1; i <= Nc; ++i){
		drop(pVec, tVec, pVec.back(), pVec.back() - pDrop, dropSlope, dt);
		hold(pVec, tVec, pVec.back(), holdInv, dt);
	}

	//drop to pressure lower bound
	drop(pVec, tVec, pVec.back(), Plow, dropSlope / (Nc+1), dt);

	//hold until end
	double sumT = 0;
	for (double n : tVec)
		sumT += n;
	holdIncreaseT(pVec, tVec, pVec.back(), Time - sumT, dt);
}




// ------------------- well --------------------------
typedef std::vector<std::vector<double>> doubleVecVec;
typedef std::vector<std::vector<size_t>> size_tVecVec;


class well
{
public:
	std::string wellName;
	bool is_prod;								//initialize in constructor
	std::vector<size_t> loc;					//initialize in cosntructor
	size_tVecVec perf;		//initialize by function
	double depth;

	// in dataVec		is_prod(bool)			loc()					
	//						0 or 1			size_t size_t size_t
	// perf is initialized by a perfFile
	// schedule is initialized by a schFile and given tstep

	well(std::vector<double> dataVec, std::string perfFileName)
	{
		is_prod = bool(dataVec[0]);
		loc.resize(2);
		loc[0] = size_t(dataVec[1]);
		loc[1] = size_t(dataVec[2]);
		depth = dataVec[3];

		// each line represent a completion location --------
		perf = readMultiLines_t(perfFileName);
	}

};



// =================================== struct eclInput ====================================
//#1 updataWelldata(const std::string& fileFold),
//	 read 'sch.dat' and 'perf' file and updata the mWells varaible
//	 mWells will be used when write the COMPAT keyword
//#2 updateSch(std::ifstream& infileT, std::ifstream& infileP)
//	 infileT should be connectted to 'tstep.dat' to updata mTimeVec
//	 infileP should be connectted to 'sch.dat' to updata mPresVec

struct eclInput{
	std::vector<double> mTimeVec;
	std::vector<double> mCumTimeVec;
	std::vector<double> mPresVec;
	std::vector<well> mWells;
	
	// the following four varaible are used when build schedules
	double mPi;
	double mPb;
	double mPlow;	//psi
	double mTime;	//days
	
	/*void generateVec();*/
	void updateAll(const std::string& fileFold);
	void updateWellData(const std::string& fileFold);
	void updateReservoir(const std::string& fileFold);


	void updateSch(std::ifstream& infile);
	void updateSch(size_t Nc, double dropInv, double h_d_ratio, double dropSlope, double dt);
	void updateSch(std::vector<double> P, std::vector<double> T);
	void updateHoldSch(size_t Nc, double dropInv, double h_d_ratio, double dropSlope, double dt, double last_hold);
	

	eclInput(){ ; }
	eclInput(const std::string& filename);
	//eclInput(std::ifstream& infile);
	//eclInput(const std::vector<double>& inVec);
};




//-- define the way to update and initialize eclInput

void eclInput::updateReservoir(const std::string& fileFold){
	std::ifstream infile(fileFold + "/reservoir.dat");
	infile >> mPi;
	infile >> mPb;
	infile >> mPlow;
	infile >> mTime;
	infile.close();
}

void eclInput::updateWellData(const std::string& fileFold)
{
	mWells.clear();
	//initialize mWells
	doubleVecVec wellDataVV = readMultiLines(fileFold + "/wells.dat");
	for (size_t i = 0; i < wellDataVV.size(); ++i)
	{
		std::string perfFileName = fileFold + "/perf/" + std::to_string(i + 1) + ".dat";
		well w(wellDataVV[i], perfFileName);
		mWells.push_back(w);
	}
}

void eclInput::updateSch(size_t Nc, double dropInv, double h_d_ratio,  double dt ,double dropSlope){
	mTimeVec.clear();
	mPresVec.clear();
	mCumTimeVec.clear();
	dropHoldAllSch(mPresVec, mTimeVec, mPi, mPb, mPlow, Nc, dropInv, h_d_ratio, dt, dropSlope, mTime);
	mCumTimeVec.resize(mTimeVec.size());
	std::partial_sum(mTimeVec.begin(), mTimeVec.end(), mCumTimeVec.begin());
}


void eclInput::updateSch(std::ifstream& infile){
	double Nc, dropInv, h_d_ratio, dt, dropSlope;
	infile >> Nc >> dropInv >> h_d_ratio >> dt >> dropSlope;
	updateSch(Nc, dropInv, h_d_ratio, dt, dropSlope);
}

void eclInput::updateHoldSch(size_t Nc, double dropInv, double h_d_ratio, double dropSlope, double dt, double last_hold){
	mTimeVec.clear();
	mPresVec.clear();
	mCumTimeVec.clear();
	dropHoldSch(mPresVec, mTimeVec, mPi, mPb, mPlow, Nc, dropInv, h_d_ratio, dt, dropSlope, last_hold);
	mCumTimeVec.resize(mTimeVec.size());
	std::partial_sum(mTimeVec.begin(), mTimeVec.end(), mCumTimeVec.begin());
}


void eclInput::updateAll(const std::string& inputFold){
	/*std::string schFile = inputFold + "/sch.dat";*/
	updateWellData(inputFold);
	updateReservoir(inputFold);
}

eclInput::eclInput(const std::string& inputFold){
	updateAll(inputFold);
}



//
//void eclInput::generateVec(){
//	mTimeVec.clear();
//	mPresVec.clear();
//	
//	//if (mDropStart > 0){
//	//	addFlatIntV(mTimeVec, mPresVec, mUp, 0, mDropStart, mInvFlat);
//	//}
//	//addDropIntV(mTimeVec, mPresVec, mUp, mLow, mDropStart, mDropEnd, mInvDrop);
//	//if (mSimuTime > mDropEnd){
//	//	addFlatIntV(mTimeVec, mPresVec, mLow, mDropEnd, mSimuTime, mInvFlat);
//	//}
//	mCumTimeVec.resize(mTimeVec.size());
//	std::partial_sum(mTimeVec.begin(), mTimeVec.end(), mCumTimeVec.begin());
//}


