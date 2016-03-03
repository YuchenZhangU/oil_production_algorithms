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
	
	/*void generateVec();*/
	void updateAll(const std::string& fileFold);
	void updateSch(std::ifstream& infileT, std::ifstream& infileP);
	void updateWellData(const std::string& fileFold);

	eclInput(){ ; }
	eclInput(const std::string& filename);
	//eclInput(std::ifstream& infile);
	//eclInput(const std::vector<double>& inVec);
};




//-- define the way to update and initialize eclInput

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

void eclInput::updateSch(std::ifstream& infileT, std::ifstream& infileP){
	mTimeVec.clear();
	mPresVec.clear();
	mTimeVec = readSingleLine(infileT);
	mPresVec = readSingleLine(infileP);
	mCumTimeVec.resize(mTimeVec.size());
	std::partial_sum(mTimeVec.begin(), mTimeVec.end(), mCumTimeVec.begin());
}


void eclInput::updateAll(const std::string& inputFold){
	/*std::string schFile = inputFold + "/sch.dat";*/
	updateWellData(inputFold);
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


