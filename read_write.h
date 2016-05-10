#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>
#include <regex>
#include <numeric>      // std::partial_sum in readNPV_OW function
#include "generalFuc.h"

typedef dlib::matrix<double> matrix;

//************************************************************ 1.1 READ SCH ECLIPSE FILE ******************************************

// filename is the file name of file to be read
//n = No. of position of keywrod from the bottom of summary
double readKeyWordValue(const std::string& filename, const int& n)
{
	std::ifstream infile(filename);

	std::ios_base::streamoff off = -17 * n - n / 4 - 1;
	std::string valueString;
	double dig;

	infile.seekg(off, std::ios_base::end);
	infile >> valueString;
	dig = std::stod(valueString);
	return dig;
}

// convet a number to 4 digit string, say 4 to '0004'
std::string io4st(const size_t& n)
{
	if (n <= 9)
		return "000" + std::to_string(n);
	if (9 < n && n <= 99)
		return "00" + std::to_string(n);
	if (99 < n && n <= 999)
		return "0" + std::to_string(n);
	if (n > 999)
		return std::to_string(n);
	std::cerr << "fail to convet from integer to string" << std::endl;
}


/*find the Keyword position from the end of summary file (XX_SUM.INC), for example:
--
A
B
C
--
B is 2;C is 1
usually the file to be searched is "***_SUM.INC"
------------
filename is the name of the file to be read
keyWord is key word whose value we want to read
*/
int whereIsKeyWord(const std::string& filename, const std::string& keyWord)
{
	std::ifstream  infile(filename);
	int posN = 0;
	std::string buff;
	std::regex r(keyWord);
	std::regex r_end("--");
	while (true)
	{
		if (!getline(infile, buff))
		{
			std::cout << filename << '\t' << keyWord << std::endl;
			/*std::cerr << " Can't find the key word! in function whereIsKeyWord" << std::endl;*/
			return 0;
		}
		if (std::regex_match(buff, r))
			break;
	}
	while (true)
	{
		posN++;
		getline(infile, buff);
		if (std::regex_match(buff, r_end))
			break;
	}
	return posN;

}


int whereIsKeyWordBeg(const std::string& filename, const std::string& keyWord)
{
	std::ifstream  infile(filename);
	int posN = 0;
	std::string buff;
	std::regex r(keyWord);
	std::regex r_commet("--.*");
	while (true)
	{
		if (!getline(infile, buff))
		{
			std::cout << filename << '\t' << keyWord << std::endl;
			/*std::cerr << " Can't find the key word! in function whereIsKeyWord" << std::endl;*/
			return 0;
		}
		if (!std::regex_match(buff, r_commet)){
			posN++;
		}

		if (std::regex_match(buff, r))
			break;
	}
	return posN;

}



//Used to get the keyword "TSTEP" position
std::vector<std::ios_base::streampos> getStreamPosition(const std::string& filename, const std::string& keyWordtoFind)
{
	std::ifstream infile(filename);
	std::string buff;
	std::vector<std::ios_base::streampos> pos;
	std::string matchStr = ".*" + keyWordtoFind + ".*'REAL'";
	std::regex r(keyWordtoFind);
	while (true)
	{
		if (!getline(infile, buff))
			break;
		if (std::regex_match(buff, r))
		{
			pos.push_back(infile.tellg());
		}

	}
	return pos;
}



// getSinglePos  function is used in the Read Ecl Rst file class to read eclipse F file
std::ios_base::streampos getSinglePos(const std::string& filename, const std::string& keyWordtoFind)
{
	std::ifstream infile(filename);
	std::string buff;
	std::ios_base::streampos pos;

	while (true)
	{
		if (!getline(infile, buff))
			break;
		if (buff.find(keyWordtoFind, 0) != std::string::npos)
		{
			pos = infile.tellg();
		}
	}
	infile.close();
	return pos;
}


//double getNextSingleValue(const std::string& filename, const std::string& keyWordtoFind){
//	std::ifstream infile(filename);
//	std::string buff;
//	std::ios_base::streampos pos;
//
//	while (true)
//	{
//		if (!getline(infile, buff))
//			break;
//		if (buff.find(keyWordtoFind, 0) != std::string::npos)
//		{
//			pos = infile.tellg();
//			break;
//		}
//	}
//	double var;
//	infile >> var;
//	infile.close();
//	return var;
//}


// Used to get the next value of "TSTEP"
std::vector<double> getNextValue(const std::string& filename, const std::string& keyWord)
{
	std::vector<double> nextValue;
	std::vector<std::ios_base::streampos> Pos;
	std::ifstream infile(filename);
	Pos = getStreamPosition(filename, keyWord);
	for (size_t i = 0; i < Pos.size(); i++)
	{
		infile.seekg(Pos[i]);
		double buff;
		infile >> buff;
		nextValue.push_back(buff);
	}
	return nextValue;
}



// read last line keyword value of each .AXXX eclipse output file
std::vector<double> readMultiTimeStepValue(const std::string& projectName, const std::string& keyWord)
{
	std::vector<double> keyWordValue;
	std::string filename;
	std::string filenameBase;
	std::ifstream infile;
	int n = whereIsKeyWord(projectName + "_SUM.INC", keyWord);
	if (n == 0){
		return keyWordValue;
	}
	size_t timeStep = 1;
	filenameBase = projectName + "_E100.A";
	filename = filenameBase + "0001";

	// get time step s
	std::vector<std::ios_base::streampos> stPosV =
		getStreamPosition(projectName + "_SCH.INC", "TSTEP");
	size_t Ntstep = stPosV.size();
	while (true)
	{
		infile.open(filename);
		if (!infile || timeStep>Ntstep)
			break;
		keyWordValue.push_back(readKeyWordValue(filename, n));
		timeStep++;
		filename = filenameBase + io4st(timeStep);
		infile.close();
	}

	return keyWordValue;
}

//usd to read keyword value(given it's postion) in one .AXXX eclipse output file
void readMiniStep(std::vector<double>& vec, std::string filename, int nBeg){
	std::vector<std::ios_base::streampos> posVec = getStreamPosition(filename, ".* 'PARAMS  '.*\\d+.*'REAL'.*");
	std::ifstream infile(filename);
	size_t Nsteps = posVec.size();
	for (size_t i = 0; i < Nsteps; i++)
	{
		std::ios_base::streamoff off = 17 * (nBeg + 1) + (nBeg + 1) / 4 + 1;
		infile.seekg(posVec[i]);
		infile.seekg(off, std::ios_base::cur);
		double buff;
		infile >> buff;
		vec.push_back(buff);
	}
}

// used only to read all time steps in all .AXXX eclipse output file
std::vector<double> readMiniTimeStep(const std::string& projectName){
	//--- get how many timestep we have ( how many file we should read?)
	std::vector<double> minitVec;

	std::vector<std::ios_base::streampos> stPosV =
		getStreamPosition(projectName + "_SCH.INC", "TSTEP");
	size_t Ntstep = stPosV.size();

	size_t timeStep = 1;
	std::string filenameBase = projectName + "_E100.A";
	std::string  filename = filenameBase + "0001";

	while (true)
	{
		std::ifstream infile(filename);
		if (!infile || timeStep>Ntstep)
			break;
		readMiniStep(minitVec, filename, -1);
		timeStep++;
		filename = filenameBase + io4st(timeStep);
		infile.close();
	}

	return minitVec;
}

//  used only to read all keyword value for all mini timesteps in all .AXXX eclipse output file
std::vector<double> readMultiMiniStep(const std::string& projectName, const std::string& keyWord){
	//--- get how the keyword ranking from the beg
	std::vector<double> eclVec;
	int nBeg = whereIsKeyWordBeg(projectName + "_SUM.INC", keyWord);
	if (nBeg == 0){
		return eclVec;
	}

	//--- get how many timestep we have ( how many file we should read?)
	std::vector<std::ios_base::streampos> stPosV =
		getStreamPosition(projectName + "_SCH.INC", "TSTEP");
	size_t Ntstep = stPosV.size();

	size_t timeStep = 1;
	std::string filenameBase = projectName + "_E100.A";
	std::string  filename = filenameBase + "0001";

	while (true)
	{
		std::ifstream infile(filename);
		if (!infile || timeStep>Ntstep)
			break;
		readMiniStep(eclVec, filename, nBeg);
		timeStep++;
		filename = filenameBase + io4st(timeStep);
		infile.close();
	}

	return eclVec;
}

// read NPV from the runed Eclipse file
double readNPV_OW(const std::string& projName)
{
	double NPV = 0;
	std::ifstream infile("../Input/econPara.dat");
	double b = 0.12;	//year inflation rate
	double RO = 35;	//oil price $/stb
	double RG = 2.4;	//gas price $/MMBtu
	double RW = 0;	//water disposal cost $/stb
	double RWINJ = 0;	//water injection cost %/stb
	infile >> b;
	infile >> RO;
	infile >> RG;
	infile >> RW;
	infile >> RWINJ;

	//read time & compute cummulative time
	std::vector<double> Time;
	std::vector<double> cumTime;
	Time = getNextValue(projName + "_SCH.INC", "TSTEP");
	cumTime.resize(Time.size());
	std::partial_sum(Time.begin(), Time.end(), cumTime.begin());

	// read FOPT FWPT FWIT
	std::vector<double> FOPT = readMultiTimeStepValue(projName, "FOPT");
	std::vector<double> FGPT = readMultiTimeStepValue(projName, "FGPT");
	std::vector<double> FWPT = readMultiTimeStepValue(projName, "FWPT");
	std::vector<double> FWIT = readMultiTimeStepValue(projName, "FWIT");
	if (FGPT.size() == 0 ){
		std::vector<double> buff(FOPT.size(), 0);
		FGPT = buff;
	}
	if (FWPT.size() == 0 || FWIT.size() == 0){
		std::vector<double> buff(FOPT.size(), 0);
		FWPT = buff;
		FWIT = buff;
	}
	// -------- calculate NPV -----------
	NPV = (FOPT[0] * RO + FGPT[0] * RG - FWPT[0] * RW - FWIT[0] * RWINJ)
		/ pow((1 + b), cumTime[0] / 365);
	for (size_t i = 1; i < cumTime.size(); i++)
	{
		double NPVi = ((FOPT[i] - FOPT[i - 1])*RO + (FGPT[i] - FGPT[i - 1])*RG
			- (FWPT[i] - FWPT[i - 1])*RW - (FWIT[i] - FWIT[i - 1])*RWINJ);
		NPVi = NPVi / pow((1 + b), cumTime[i] / 365);
		NPV += NPVi;
	}

	return NPV;
}
//========== Example: ====================
//const std::string projName("../test1/1");
//double NPV = readNPV_OW(projName);
//std::cout << NPV << std::endl;




//*********************************************************** 1.2 READ ECLIPSE RST FILES ************************************************

//read grid properties for each time step
std::vector<matrix> readGrid(const std::string& projectName, const std::string& keyWord){
	//--- read dimensions
	size_t I;
	size_t J;
	size_t K;
	std::string dataFile = projectName + "_E100.DATA";
	std::ios_base::streampos Pos = getSinglePos(dataFile,"DIMENS");
	std::ifstream infile(dataFile);
	infile.seekg(Pos);
	infile >> I >> J >> K;
	infile.close();

	//--- get time step s
	std::vector<std::ios_base::streampos> stPosV =
		getStreamPosition(projectName + "_SCH.INC", "TSTEP");
	size_t Ntstep = stPosV.size();

	//--- read property matrices
	std::vector<matrix> matVec;
	matrix mat(J,I);
	std::string  filenameBase = projectName + "_E100.F";
	for (size_t t = 1; t <= Ntstep; ++t){
		std::string filename = filenameBase + io4st(t);
		Pos = getSinglePos(filename, keyWord);
		infile.open(filename);
		infile.seekg(Pos);
		for (size_t j = 0; j < J; ++j){
			for (size_t i = 0; i < I; ++i){
				infile >> mat(j, i);
			}
		}
		matVec.push_back(mat);
		infile.close();
	}

	return matVec;	
}

class rstData{
public:
	std::string mProjName;
	std::vector<matrix> mPoMatVec;
	std::vector<matrix> mSgMatVec;
	std::vector<double> mTVec;
	std::vector<double> mTCumVec;
	std::vector<size_t> mDim;
	
	
	rstData(){ ; }
	rstData(std::string projName);

	void updateDim();
	void updateTVec();
	void updateMatVec(std::vector<matrix>& matVec, std::string keyword);
	void updateAll( std::string projName);
	
	// outpust Po Sg So
	void outputMat(std::ofstream& outfile);
	void outputMatByText(std::ofstream& outfile);
	
	void outputPlot();
	void outputPlot(std::ofstream& outfile);
	void outputSo(std::ofstream& outfile, size_t caseNum);
	


	void output1D(std::ofstream& outfile);
	void outputSingleCellSo(std::ofstream& outilfe);
};


// output So of fracture vs time
// output Fo of fracture vs time
void rstData::outputSingleCellSo(std::ofstream& outfile){
	outfile << "So_rel = [0.2	0.3	0.4	0.5	0.6	0.7	0.8	0.9	1];\n";
	outfile << "Fo_rel = [0	0.00092969	0.004323816	0.011589118	0.025393903	0.05148754	0.104945178	0.241953654	1];\n";

	outfile << "T = [" << dlib::trans(vec2mat(mTCumVec)) << "];\n";
	outfile << "So = [";
	for (size_t i = 0; i < mSgMatVec.size(); ++i){
		outfile << 1 - mSgMatVec[i](31)<<" ";
	}
	outfile << "];\n";
	outfile << "fo = interp1(So_rel,Fo_rel,So);\n";
	outfile << "[hAx, hLine1, hLine2] = plotyy(T,So,T,fo)\n";
	outfile << "hLine1.LineWidth = 2;\nhLine2.LineWidth = 2;\n";
	outfile << "ylabel(hAx(1), 'Oil Saturation');\n";
	outfile << "ylabel(hAx(2), 'Oil fractional flow');\n";
	outfile << "title('fracture So and fo');\n";

	outfile << "outSoFo = [So',fo'];\n";
}

//void rstData::outputSingleCellSo(std::ofstream& outfile){
//	
//}

void rstData::output1D(std::ofstream& outfile){
	size_t N = 20;
	outfile << "L = 0.1*[1:" << N << "] - 0.05;";
	/*outfile << "So_rel = [0.2	0.3	0.4	0.5	0.6	0.7	0.8	0.9	1];\n";
	outfile << "Fo_rel = [0	0.00092969	0.004323816	0.011589118	0.025393903	0.05148754	0.104945178	0.241953654	1];\n";
*/
	for (size_t i = 0; i < mPoMatVec.size(); ++i){
		outfile << "Po" << i + 1 << "=[" << dlib::colm(mPoMatVec[i], dlib::range(0, N-1)) << "];\n";
		outfile << "So" << i + 1 << "=[" << 1 - dlib::colm(mSgMatVec[i], dlib::range(0, N-1)) << "];\n";
		/*outfile << "dPo" << i + 1 << " = (Po" << i + 1 << "(2:end)-Po" << i + 1 << "(1:end-1))./dL;\n";
		outfile << "Fo" << i + 1 << "= interp1(So_rel,Fo_rel,So" << i + 1 << ");\n";*/
		outfile << "[hAx, hLine1, hLine2] = plotyy(L, So" << i + 1 << ", L, Po" << i + 1 << ")\n";
		outfile << "hLine1.LineWidth = 2;\nhLine2.LineWidth = 2;\n";
		outfile << "ylim(hAx(1),[0.85,1.01])\n";
		outfile << "ylim(hAx(2),[4000,6000])\n";
		//outfile << "xlim(hAx(1),[0, 5])\n";
		//outfile << "xlim(hAx(2),[0, 5])\n";
		outfile << "xlabel('distance from fracture/ft');\n";
		outfile << "ylabel(hAx(1), 'Oil Saturation');\n";
		outfile << "ylabel(hAx(2), 'Pressure(psi)');\n";
		outfile << "title('So / Po vs L  t = " << i + 1 << "');\n";
		outfile << "saveas(gcf,'plot/So_Po" << i + 1 << ".jpg');\n\n\n";


		//outfile << "Po" << i + 1 << "=[" << dlib::colm(mPoMatVec[i], dlib::range(0, 20)) << "];\n";
		//outfile << "So" << i + 1 << "=[" << 1 - dlib::colm(mSgMatVec[i], dlib::range(31, 62)) << "];\n";
		//outfile << "dPo" << i + 1 << " = (Po" << i+1 << "(2:end)-Po" << i+1 << "(1:end-1))./dL;\n";
		//outfile << "Fo" << i + 1 << "= interp1(So_rel,Fo_rel,So" << i + 1 << ");\n";
		//outfile << "[hAx, hLine1, hLine2] = plotyy(L, Fo"<<i+1<<", L2, dPo"<<i+1<<")\n";
		//outfile << "hLine1.LineWidth = 2;\nhLine2.LineWidth = 2;\n";
		//outfile << "ylim(hAx(1),[0,1.01])\n";
		//outfile << "ylim(hAx(2),[0,400])\n";
		//outfile << "xlim(hAx(1),[0, 5])\n";
		//outfile << "xlim(hAx(2),[0, 5])\n";
		//outfile << "xlabel('distance from fracture/ft');\n";
		//outfile << "ylabel(hAx(1), 'Oil Fractional Flow');\n";
		//outfile << "ylabel(hAx(2), 'Pressure Gradient(psi/ft)');\n";
		//outfile << "title('Fo / Grad(P) vs L  t = "<<i+1<<"');\n";
		//outfile << "saveas(gcf,'plot3/dP_Fo" << i +1<<".jpg');\n\n\n";
		
	}
	
}

void rstData::updateDim(){
	std::string dataFile = mProjName + "_E100.DATA";
	std::ios_base::streampos Pos = getSinglePos(dataFile, "DIMENS");
	std::ifstream infile(dataFile);
	infile.seekg(Pos);
	mDim.resize(3);
	infile >> mDim[0] >> mDim[1] >> mDim[2];
	infile.close();
}

void rstData::updateTVec(){
	mTVec = getNextValue(mProjName + "_SCH.INC", "TSTEP");
	mTCumVec.resize(mTVec.size());
	std::partial_sum(mTVec.begin(), mTVec.end(), mTCumVec.begin());
}
void rstData::updateMatVec(std::vector<matrix>& matVec, std::string keyword){
	matVec.clear();
	matrix mat(mDim[1], mDim[0]);
	std::ios_base::streampos Pos;
	std::string  filenameBase = mProjName + "_E100.F";
	for (size_t t = 1; t <= mTVec.size(); ++t){
		std::string filename = filenameBase + io4st(t);
		Pos = getSinglePos(filename, keyword);
		std::ifstream infile(filename);
		infile.seekg(Pos);
		for (size_t j = 0; j < mDim[1]; ++j){
			for (size_t i = 0; i < mDim[0]; ++i){
				infile >> mat(j, i);
			}
		}
		matVec.push_back(mat);
		infile.close();
	}

}


void rstData::updateAll(std::string projName){
	mProjName = projName;
	updateDim();
	updateTVec();
	updateMatVec(mPoMatVec, "PRESSURE");
	updateMatVec(mSgMatVec, "SGAS");
}

rstData::rstData(std::string projName){
	updateAll(projName);
}


void rstData::outputMat(std::ofstream& outfile){
	for (size_t i = 0; i < mTVec.size(); ++i){
		outfile << "t = " << mTCumVec[i] << std::endl;
		outfile << "PoMat" << i + 1 << "=[" << dlib::subm(mPoMatVec[i], dlib::range(0, 0), dlib::range(0, 10)) << "];" << std::endl;
		outfile << "SgMat" << i + 1 << "=[" << dlib::subm(mSgMatVec[i], dlib::range(0, 0), dlib::range(0, 10)) << "];" << std::endl;
		outfile << "SoMat" << i + 1 << "= 1-SgMat" << i + 1 << ";" << std::endl;
	}
}

//void rstData::outputMatByText(std::ofstream& outfile){
//	for (size_t i = 0; i < mTCumVec.size(); ++i){
//		outfile << "t = " << mTCumVec[i] << std::endl;
//		outfile << "Po\tSo\n";
//		matrix posoMat()
//		outfile << 
//	}
//}

void rstData::outputPlot(){
	std::ofstream outfile("_plotSoPo.m");
	for (size_t i = 0; i < mTVec.size(); ++i){
		outfile << "PoMat" << i + 1 << "=[" << dlib::subm(mPoMatVec[i], dlib::range(0, 0), dlib::range(59, 69)) << "];" << std::endl;
		outfile << "SgMat" << i + 1 << "=[" << dlib::subm(mSgMatVec[i], dlib::range(0, 0), dlib::range(59, 69)) << "];" << std::endl;
		outfile << "SoMat" << i + 1 << "= 1-SgMat" << i + 1 << ";" << std::endl;
	}
		
		outfile << "wt =[ 0.2429	0.2034	0.1703	0.1426	0.1194	0.1	0.1194	0.1426	0.1703	0.2034	0.2429]" << std::endl;
		outfile << "wt = wt';" << std::endl;
		outfile << "wt_sum = sum(wt);" << std::endl;
		outfile << "Sg= [];" << std::endl;
		for (size_t i = 0; i < mTVec.size(); ++i){
			outfile << "Sg=[Sg SgMat" << i + 1 << "*wt/wt_sum];" << std::endl;
		}
		outfile << "T = [" << dlib::trans(vec2mat(mTCumVec)) << "];" << std::endl;
		outfile << "plot(T,Sg,'LineWidth',2);" << std::endl;
		outfile << "xlabel('time/days');" << std::endl;
		outfile << "ylabel('gas saturation');" << std::endl;
		outfile << "title('saturation change of perforated area');" << std::endl;
}

void rstData::outputPlot(std::ofstream& outfile){
	for (size_t i = 0; i < mTVec.size(); ++i){
		outfile << "PoMat" << i + 1 << "=[" << dlib::subm(mPoMatVec[i], dlib::range(0, 0), dlib::range(59, 69)) << "];" << std::endl;
		outfile << "SgMat" << i + 1 << "=[" << dlib::subm(mSgMatVec[i], dlib::range(0, 0), dlib::range(59, 69)) << "];" << std::endl;
		outfile << "SoMat" << i + 1 << "= 1-SgMat" << i + 1 << ";" << std::endl;
	}

	outfile << "wt =[ 0.2429	0.2034	0.1703	0.1426	0.1194	0.1	0.1194	0.1426	0.1703	0.2034	0.2429]" << std::endl;
	outfile << "wt = wt';" << std::endl;
	outfile << "wt_sum = sum(wt);" << std::endl;
	outfile << "Sg= [];" << std::endl;
	outfile << "Po = [];\n\n";
	for (size_t i = 0; i < mTVec.size(); ++i){
		outfile << "Sg=[Sg SgMat" << i + 1 << "*wt/wt_sum];\n" << std::endl;
	}
	for (size_t i = 0; i < mTVec.size(); ++i){
		outfile << "Po=[Po PoMat" << i + 1 << "*wt/wt_sum];\n" << std::endl;
	}

	outfile << "T = [" << dlib::trans(vec2mat(mTCumVec)) << "];" << std::endl;
	outfile << "plot(T,Sg,'LineWidth',2);" << std::endl;
	outfile << "xlabel('time/days');" << std::endl;
	outfile << "ylabel('gas saturation');" << std::endl;
	outfile << "title('saturation change of perforated area');\n" << std::endl;

	outfile << "figure;\n";
	outfile << "plot(T,Po,'LineWidth',2);" << std::endl;
	outfile << "xlabel('time/days');" << std::endl;
	outfile << "ylabel('pressure');" << std::endl;
	outfile << "title('pressure change of perforated area');\n" << std::endl;
	
}

// averaging the so of near fracture blocks
void rstData::outputSo(std::ofstream& outfile, size_t caseNum){
	matrix wt(1, 11);
	wt = 0.2429, 0.2034, 0.1703, 0.1426, 0.1194, 0.1, 0.1194, 0.1426, 0.1703, 0.2034, 0.2429;
	double wt_sum = dlib::sum(wt);
	wt = dlib::trans(wt);
	matrix poMat(mTVec.size(), 1);
	matrix sgMat(mTVec.size(), 1);
	matrix soMat(mTVec.size(), 1);
	for (size_t i = 0; i < mTVec.size(); ++i){
		matrix PoPerated = dlib::subm(mPoMatVec[i], dlib::range(0, 0), dlib::range(59, 69));
		matrix SgPerated = dlib::subm(mSgMatVec[i], dlib::range(0, 0), dlib::range(59, 69));
		double PoAve = PoPerated*wt/wt_sum;
		double SgAve = SgPerated*wt / wt_sum;
		poMat(i) = PoAve;
		sgMat(i) = SgAve;
	}
	soMat = 1 - sgMat;

	matrix caseNMat = caseNum *	dlib::ones_matrix<double>(mTVec.size(), 1);
	matrix outMat(mTVec.size(), 5);
	dlib::set_colm(outMat, 0) = caseNMat;
	dlib::set_colm(outMat, 1) = vec2mat(mTCumVec);
	dlib::set_colm(outMat, 2) = poMat;
	dlib::set_colm(outMat, 3) = sgMat;
	dlib::set_colm(outMat, 4) = soMat;
	outfile << outMat;
}


//************************* 2.READ INPUT DATA ********************************
// This function is simply for reading text file with one value each line
std::vector<double> readSingleValue(std::string filename)
{
	std::ifstream infile(filename);
	std::string buff;
	std::vector<double> valueVec;	//the vector of well control
	while (true){
		if (!getline(infile, buff))
			break;
		valueVec.push_back(std::stod(buff));
	}
	return valueVec;
}

// read multi line double type data
// each line of the text become a vector in the two dimension vector
std::vector<std::vector<double>> readMultiLines(std::string filename)
{
	std::ifstream infile(filename);
	std::string buffStr;
	std::vector<std::vector<double>> valueVV;
	while (true){
		if (!getline(infile, buffStr))
			break;
		std::istringstream is(buffStr);
		std::vector<double> v((std::istream_iterator<double>(is)), std::istream_iterator<double>());
		valueVV.push_back(v);
	}
	return valueVV;
}

std::vector<std::vector<size_t>> readMultiLines_t(std::string filename)
{
	std::ifstream infile(filename);
	std::string buffStr;
	std::vector<std::vector<size_t>> valueVV;
	while (true){
		if (!getline(infile, buffStr))
			break;
		std::istringstream is(buffStr);
		std::vector<size_t> v((std::istream_iterator<size_t>(is)), std::istream_iterator<size_t>());
		valueVV.push_back(v);
	}
	return valueVV;
}






std::vector<double> initialWellControl(const std::string& filename, const int& tStep)
{
	std::ifstream infile(filename);
	std::string buff;
	std::vector<double> xVec;	//the vector of well control
	while (true){
		if (!getline(infile, buff))
			break;
		for (int i = 0; i < tStep; ++i)
			xVec.push_back(std::stod(buff));
	}
	return xVec;
}

//***************************** 3.Write ***********************
template<typename T>
void writeVec(std::ofstream& fout, std::vector<T> vec){
	for (size_t i = 0; i < vec.size(); ++i){
		fout << vec[i] << " ";
	}
	fout << std::endl;
}



//***************************** 3.Function of Write (for use in opt) ***********************
// TESTED!
void write2End(const double& x1, const double&x2, const std::string& filename){
	std::ofstream outfile(filename, std::ios_base::app);
	outfile << x1 << "\t" << x2 << std::endl;
	outfile.close();
}


//TESTED!
void writeNPV2M(const std::vector<double>& npvVec,
	const std::string& filename, const size_t& runNo, const double& it2sim)
{
	matrix npvMatVec = vec2mat(npvVec);
	npvMatVec = trans(npvMatVec);

	std::ofstream outfile(filename);
	std::string npvName = "npv" + std::to_string(runNo);
	outfile << npvName << "= [ " << npvMatVec << "];\n";
	outfile << "searchIt = "<<it2sim<<"*"<<"[0:" << npvVec.size() - 1 << "];\n";
	outfile << "plot(searchIt," << npvName << ",'LineWidth',2);";
}


void printMat(const matrix& Mat, std::ofstream& os){
	os << Mat;
}

//TESTED!
void writeMat2M(const matrix& Mat, const std::string& filename,
	const size_t nameSuf=1, const std::string& namePre = "x")
{
	std::ofstream outfile(filename, std::ios_base::app);
	outfile << namePre << nameSuf << " = [" << Mat << "];\n";
	outfile << "figure;\n";
	outfile << "imagesc(" << namePre << nameSuf << ");\n";
	outfile << "colorbar;\n";
	outfile << "colormap(jet);\n";
	outfile << "title('" << namePre << ", it = " << nameSuf << "');\n";
	outfile << "saveas(gcf,'" << "plots/" << namePre << nameSuf << ".jpg"<<"');\n";
	outfile.close();
}

//TESTED!
void writeCtrl2M(const matrix& uVec, const std::string& filename_w,const std::string& filename_o,
	const size_t& Nt, const size_t Nw, const size_t Nprod, const size_t& it)
{
	matrix uMat = dlib::reshape(uVec, Nw, Nt);
	matrix uMat_inj = dlib::rowm(uMat, dlib::range(0, Nw - Nprod - 1));
	matrix uMat_prod = dlib::rowm(uMat, dlib::range(Nw - Nprod, Nw-1));

	if ((Nw - Nprod) != 0){
		writeMat2M(uMat_inj, filename_w, it,"injCtrl");
	}

	if (Nprod != 0){
		writeMat2M(uMat_prod, filename_o, it,"proCtrl");
		
	}
}

