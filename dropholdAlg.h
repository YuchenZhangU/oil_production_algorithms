#pragma once
#include "eclipse.h"


class dropHoldAlg
{
public:
	eclipse mModel;
	size_t mNc; //DHC (BHP drop hold circle)
	double mDropInv;
	double mH_d_ratio;
	double mMrr;
	double mDt;
	double mDropSlope;
	//------ constructors --------------
	dropHoldAlg(std::string fName, std::string pName, std::string iName);
	dropHoldAlg(){ ; }
	//--------functions------------------
	void setPath(std::string fName, std::string pName, std::string iName);
	void setParameters(double dropInv, double h_d_ratio, double mrr, double dt , double dropSlope );
	double getResponseRate();
	void run();  //drop times (D), hold drop ratio (R) AND minimum response rate (E)

	//-------output functions-----------
};

dropHoldAlg::dropHoldAlg(std::string fName, std::string pName="11", std::string iName="input"){
	mModel.updateStatic(fName, pName, iName);
	mNc = 0;
}


void dropHoldAlg::setPath(std::string fName, std::string pName, std::string iName){
	mModel.updateStatic(fName, pName, iName);
}

void dropHoldAlg::setParameters(double dropInv, double h_d_ratio, double mrr=1e-4, double dt = 1, double dropSlope = 80){
	mDropInv = dropInv;
	mH_d_ratio = h_d_ratio;
	mMrr = mrr;
	mDt = dt;
	mDropSlope = dropSlope;
}

void dropHoldAlg::run(){
	mNc = 1;
	while (true){
		mModel.runHoldAlg(mNc, mDropInv, mH_d_ratio, mDt, mDropSlope, mDropInv*mH_d_ratio);
		if (getResponseRate() < mMrr){
			break;
		}
		mNc++;
	}
	mModel.runAlg(mNc, mDropInv, mH_d_ratio, mDt, mDropSlope);
	mNc = 0;
}

double dropHoldAlg::getResponseRate(){
	std::vector<double> qoVec = mModel.readVec("FOPR");
	std::vector<double> qgVec = mModel.readVec("FGPR");
	std::vector<double> ogrVec;
	if (qoVec.size() != qgVec.size()){
		std::cout << "in getResonseRate() function, qoVec and qgVec are not of same size!\n";
	}
	for (size_t i=0; i < qoVec.size(); ++i){
		ogrVec.push_back(qoVec[i] / qgVec[i]);
	}

	double holdInv = mDropInv*mH_d_ratio;
	size_t Nsteps =  holdInv/ mDt;
	double ogrStartHold = *(ogrVec.end() - Nsteps);
	double ogrEndHold = ogrVec.back();
	double RR = (ogrEndHold - ogrStartHold) / holdInv;
	return(RR);
}

