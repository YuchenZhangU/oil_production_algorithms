#pragma once
#include "eclInput.h"


//================ 1. Basic Ecl write and the overload of basic write ==============
void writeTimeStep(std::ofstream& outfile, const double& time){
	outfile << "TSTEP\n";
	outfile << time << " /\n\n";
}

//------ two overload of WCONPROD
void writeWCONPROD_BHP(const std::string& fName, const double& pressure, const std::string& wName= "PROD"){
	std::ofstream outfile(fName, std::ios_base::app);
	outfile << "WCONPROD\n";
	outfile << "'" << wName << "'" << " 'OPEN' 'BHP' 5* " << pressure << " /\n";
	outfile << "/\n\n";
	outfile.close();
}
//overloading the last function, input ofstream instead of output file name
//remmeber to close the outfile or flush the outfile before use the another overload of this function
void writeWCONPROD_BHP(std::ofstream& outfile, const double& pressure,const std::string& wName = "PROD"){
	outfile << "WCONPROD\n";
	outfile << "'" << wName << "'" << " 'OPEN' 'BHP' 5* " << pressure << " /\n";
	outfile << "/\n\n";
}


//----- two overload of WELSPECS
void writeWELSPECS_Prod(const std::string& fName, const double& depth, const std::string& wName = "PROD"){
	std::ofstream outfile(fName, std::ios_base::app);
	outfile << "WELSPECS\n";
	outfile << "'" << wName << "'" << " 'WGRP' 1 1 " << depth <<" 'OIL' 9* 'STD' /\n";
	outfile << "/\n\n";
	outfile.close();
}

void writeWELSPECS_Prod(std::ofstream& outfile, const double& depth, const std::string& wName = "PROD"){
	outfile << "WELSPECS\n";
	outfile << "'" << wName << "'" << " 'WGRP' 1 1 " << depth << " 'OIL' 9* 'STD' /\n";
	outfile << "/\n\n";
}

//----- two overload of COMPDAT
void writeCOMPDAT(const std::string& fName, std::vector<size_t> perfVec, 
	const double& D = 0.4, const std::string& wName = "PROD"){
	if (perfVec.size() != 4){
		std::cerr << "Wrong Perf input\n";
	}
	std::ofstream outfile(fName, std::ios_base::app);
	outfile << "COMPDAT\n";
	outfile << "'" << wName << "' " << perfVec[0]<<" "<<perfVec[1] <<" "<<perfVec[2]<<" "
		<<perfVec[3]<<"'OPEN' 2* "<<D<<" 3* 'X' 1* /\n";
	outfile << "/\n\n";
	outfile.close();
}

void writeCOMPDAT(std::ofstream& outfile, std::vector<size_t> perfVec,
	const double& D = 0.4, const std::string& wName = "PROD"){
	if (perfVec.size() != 4){
		std::cerr << "Wrong Perf input\n";
	}
	outfile << "COMPDAT\n";
	outfile << "'" << wName << "' " << perfVec[0] << " " << perfVec[1] << " " << perfVec[2] << " "
		<< perfVec[3] << " 'OPEN' 2* " << D << " 3* 'X' 1* /\n";
	outfile << "/\n\n";
}


//================ 2. multi or compound write based on basic write function ==============
//----- write COMPDAT for a well
void writeCOMPDATs(std::ofstream& outfile, const well& w, 
	const double& D = 0.4, const std::string& wName = "PROD"){
	for (size_t i = 0; i < w.perf.size(); ++i){
		writeCOMPDAT(outfile, w.perf[i], D, wName);
	}
}



//---- two overload of ProdSch
void writeProdSch(const std::string& fName, const eclInput& eclIn){
	if (eclIn.mTimeVec.size() != eclIn.mPresVec.size()){
		std::cerr << "time vector and pressure vector have different size!\n";
	}
	std::ofstream outfile(fName, std::ios_base::app);
	for (size_t i = 0; i < eclIn.mTimeVec.size(); ++i){
		writeWCONPROD_BHP(outfile, eclIn.mPresVec[i]);
		writeTimeStep(outfile, eclIn.mTimeVec[i]);
	}
}


void writeProdSch(std::ofstream& outfile, const eclInput& eclIn){
	if (eclIn.mTimeVec.size() != eclIn.mPresVec.size()){
		std::cerr << "time vector and pressure vector have different size!\n";
	}

	for (size_t i = 0; i < eclIn.mTimeVec.size(); ++i){
		writeWCONPROD_BHP(outfile, eclIn.mPresVec[i]);
		writeTimeStep(outfile, eclIn.mTimeVec[i]);
	}
}




//void writeUniformDrawDown(const std::string& fName, const double& up, const double low, const double& tan, const double& intv = 30){
//	size_t i = 1;
//	while (true){
//		double BHP = up - tan*intv*i;
//		if (BHP <= low){
//			writeWCONPROD_BHP(fName, low);
//			break;
//		}
//		writeWCONPROD_BHP(fName, BHP);
//		i++;
//	}
//}
//
//
//void writeUniformDrawDown(std::ofstream& outfile, const double& up, const double low, const double& tan, const double& intv = 30){
//	size_t i = 1;
//	while (true){
//		double BHP = up - tan*intv*i;
//		if (BHP <= low){
//			writeWCONPROD_BHP(outfile, low);
//			break;
//		}
//		writeWCONPROD_BHP(outfile, BHP);
//		i++;
//	}
//}//