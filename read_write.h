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

//*********************** 1.READ ECLIPSE FILE ***************************

double readKeyWordValue(const std::string& filename, const int& n);

std::string io4st(const size_t& n);

int whereIsKeyWord(const std::string& filename, const std::string& keyWord);

std::vector<double> readMultiTimeStepValue(const std::string&
	projectName, const std::string& keyWord);

std::vector<std::ios_base::streampos> getStreamPosition(const
	std::string& filename, const std::string& keyWordtoFind);

std::vector<double> getNextValue(const std::string& filename, const
	std::string& keyWord);

double readNPV_OW(const std::string& projName);


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

/*keyWordValue for each timestep

*/
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

//Used to get the keyword "TSTEP" position
std::vector<std::ios_base::streampos> getStreamPosition(const std::string& filename, const std::string& keyWordtoFind)
{
	std::ifstream infile(filename);
	std::string buff;
	std::vector<std::ios_base::streampos> pos;
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

// This function is read to read a single line and convert this into a double type vector
std::vector<double> readSingleLine(std::ifstream& infile){
	std::string buffstr;
	getline(infile, buffstr);
	std::istringstream is(buffstr);
	std::vector<double> vec((std::istream_iterator<double>(is)), std::istream_iterator<double>());
	return vec;
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

//matrix readMultiLines(std::string filename)
//{
//	std::ifstream infile(filename);
//	std::string buffStr;
//	std::vector<std::vector<double>> valueVV;
//	while (true){
//		if (!getline(infile, buffStr))
//			break;
//		std::istringstream is(buffStr);
//		std::vector<double> v((std::istream_iterator<double>(is)), std::istream_iterator<double>());
//		valueVV.push_back(v);
//	}
//	return matirx;
//}


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


//***************************** 3.Function of Write ***********************
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




////		/*outfile << "wCtrl_inj" << it << " = [" << uMat_inj << "];\n";
//outfile << "figure;\n";
//outfile << "imagesc(" << "wCtrl_inj" << it << ");\n";
//outfile << "colorbar;\n";
//outfile << "colormap(jet);\n"; */
/*outfile << "wCtrl_prod" << it << " = [" << uMat_prod << "];\n";
outfile << "figure;\n";
outfile << "imagesc(" << "wCtrl_prod" << it << ");\n";
outfile << "colorbar;\n";
outfile << "colormap(jet);\n\n";*/