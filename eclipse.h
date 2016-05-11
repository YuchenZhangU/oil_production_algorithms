#pragma once
#include "ecl_write.h"

class eclipse
{
	/*std::vector<size_t> mDim;*/
	std::string mProjFold;
	std::string mProjName;
	std::string mInputFold;

	// for static data read in constructor
	void writeBatchFile();
	void writeLocalBatchFile();

public:
	size_t nOfRun;
	size_t nOfProd;
	size_t nOfFail;
	eclInput mInput;

	//--- updates and constructors
	// * updateStatic function update the filename(for model, input and output)
	//   and all mInput data(well, reservoir data and schedule), but do not write schulde to data
	// * updateSch function update the well schedule vector in mInput according to input parametter
	//   and write the schudule to eclipse model file. The updateSch function can accept parameters and ifstream as input
	// * updateAll do both 
	eclipse(){ ; }
	eclipse(std::string fName, std::string pName, std::string iName);
	void updateAll(std::string fName, std::string pName, std::string iName);
	void updateSch(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope);
	void updateSch(std::ifstream& infile);
	void updateHoldSch(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope, double last_hold);
	void updateStatic(std::string fName, std::string pName, std::string iName);

	//--- functions
	void writeSchFile();
	void run();
	void runHoldAlg(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope, double last_hold);
	void runAlg(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope);

	//---------- read functions
	std::vector<double> readVec(std::string keyword);
	double readFinalFOPT();
	double readNPV();

	//--------- output functions
	void outputAll(std::string filename, size_t caseNo, std::string eco_filename);

};


// <1>---------------- update and constructor --------------
void eclipse::writeSchFile(){
	std::ofstream outfile(mProjFold + "/" + mProjName + "_SCH.INC");
	writeWELSPECS_Prod(outfile, mInput.mWells[0].depth);
	writeCOMPDATs(outfile, mInput.mWells[0]);
	writeProdSch(outfile, mInput);
}

void eclipse::writeLocalBatchFile(){
	std::ofstream batch(mProjFold + "/_RunEclipse.bat");
	batch << "@echo off";
	batch << "set ECLVER=2013.1\n";
	batch << "set TEMP_PATH=C:\\ecl\\home\\$eclrc.bat\n";
	batch << "echo *\n";
	batch << "echo * ECLIPSE VERSION 2013.1\n";
	batch << "echo *\n";
	batch << "C:\\ecl\\2013.1\\bin\\pc_x86_64\\eclipse.exe " << mProjName << "_E100" << std::endl;
	batch << "set PATH=%TEMP_PATH%\n";
	batch.close();
}

void eclipse::updateSch(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope){
	mInput.updateSch(Nc, dropInv, h_d_ratio, dt, dropSlope);
	writeSchFile();
}

void eclipse::updateSch(std::ifstream& infile){
	mInput.updateSch(infile);
	writeSchFile();
}

void eclipse::updateHoldSch(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope, double last_hold){
	mInput.updateHoldSch(Nc, dropInv, h_d_ratio, dropSlope, dt,last_hold);
	writeSchFile();
}




void eclipse::updateStatic(std::string fName, std::string pName, std::string iName){
	mProjFold = fName;
	mProjName = pName;
	mInputFold = iName;
	mInput.updateAll(mInputFold);		//initilize completion & reservoir data, but not sch file
	nOfProd = 0;
	nOfRun = 0;
	nOfFail = 0;
	//Assign wellName & count nOfProd-----------
	for (size_t i = 0; i < mInput.mWells.size(); ++i)
	{
		if (mInput.mWells[i].is_prod){
			++nOfProd;
			mInput.mWells[i].wellName = "PROD" + std::to_string(i + 1);
		}

		else
			mInput.mWells[i].wellName = "INJ" + std::to_string(i + 1);
	}
	writeLocalBatchFile();
}


void eclipse::updateAll(std::string fName, std::string pName, std::string iName){
	updateStatic(fName,pName,iName);
	writeSchFile();
}

eclipse::eclipse(std::string fName, std::string pName, std::string iName){
	updateStatic(fName, pName, iName);
}


// <2> --------------- Batch and Run --------------------
void eclipse::writeBatchFile(){
	std::ofstream batch("RunEclipse.bat");
	//batch << "cd.\\model\\" + to_string(i) + "\\";
	batch << "cd./" << mProjFold << "/";
	batch << "\n@echo off";
	batch << "set ECLVER=2013.1\n";
	batch << "set TEMP_PATH=C:\\ecl\\home\\$eclrc.bat\n";
	batch << "echo *\n";
	batch << "echo * ECLIPSE VERSION 2013.1\n";
	batch << "echo *\n";
	batch << "C:\\ecl\\2013.1\\bin\\pc_x86_64\\eclipse.exe " << mProjName << "_E100" << std::endl;
	batch << "set PATH=%TEMP_PATH%\n";
	batch.close();
}

void eclipse::run(){
	writeBatchFile();
	system("RunEclipse.bat");

	std::string str = mProjFold + "/" + mProjName + "_E100.A0001";
	std::ifstream infile(str);
	std::ofstream outfile;
	while (!infile.is_open()){
		++nOfFail;
		/*outfile.open("../output/error.dat", std::ios_base::app);
		outfile << "At time " << getCurrentTimeStr();
		outfile << nOfFail << " Eclipse Run fail, run again!-----------" << std::endl;*/
		system("RunEclipse.bat");
		infile.open(str);
	}
	infile.close();
	++nOfRun;
	//outfile.open("../output/runNums.dat", std::ios_base::app);
	//outfile << getCurrentTimeStr();
	//outfile << nOfRun << "\n";
}

void eclipse::runAlg(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope){
	updateSch(Nc, dropInv,h_d_ratio, dt,dropSlope);
	run();
}

void eclipse::runHoldAlg(size_t Nc, double dropInv, double h_d_ratio, double dt, double dropSlope, double last_hold){
	updateHoldSch(Nc, dropInv, h_d_ratio, dt, dropSlope, last_hold);
	run();
}




// <3> -----------------Read Functions -------------------


std::vector<double> eclipse::readVec(std::string keyword){
	std::string proj = mProjFold + "/" + mProjName;
	std::vector<double> vec = readMultiTimeStepValue(proj, keyword);
	return vec;
}

double eclipse::readFinalFOPT(){
	std::string proj = mProjFold + "/" + mProjName;
	std::vector<double> FOPTvec = readMultiTimeStepValue(proj, "FOPT");
	return *(FOPTvec.end() - 1);
}


double eclipse::readNPV(){
	return readNPV_OW(mProjFold + "/" + mProjName);

}

// <4> ---------------------- ouput Functions --------------------------
void eclipse::outputAll(std::string filename, size_t caseNo, std::string eco_filename){
	std::string projName;
	if (mProjFold == ""){
		projName = mProjName;
	}
	else{
		projName = mProjFold + "/" + mProjName;
	}

	std::vector<double> tVec = readMiniTimeStep(projName);
	std::vector<double> FGPR = readMultiMiniStep(projName, "FGPR");
	std::vector<double> FGPT = readMultiMiniStep(projName, "FGPT");
	std::vector<double> FOPR = readMultiMiniStep(projName, "FOPR");
	std::vector<double> FOPT = readMultiMiniStep(projName, "FOPT");
	std::vector<double> WBHP = readMultiMiniStep(projName, "WBHP");
	matrix caseMat = caseNo * dlib::ones_matrix<double>(tVec.size(), 1);
	matrix outMat(tVec.size(), 10);
	dlib::set_colm(outMat, 0) = caseMat;
	dlib::set_colm(outMat, 1) = vec2mat(tVec);
	dlib::set_colm(outMat, 2) = vec2mat(FGPR);
	dlib::set_colm(outMat, 3) = vec2mat(FGPT);
	dlib::set_colm(outMat, 4) = vec2mat(FOPR);
	dlib::set_colm(outMat, 5) = vec2mat(FOPT);
	dlib::set_colm(outMat, 6) = vec2mat(WBHP);

	//-------- get NPV --------------
	std::ifstream infile(eco_filename);
	double b = 0.12;	//year inflation rate
	double RO = 35;	//oil price $/stb
	double RG = 2.0;	//gas price $/MMBtu
	double RW = 0;	//water disposal cost $/stb
	double RWINJ = 0;	//water injection cost %/stb
	infile >> b;
	infile >> RO;
	infile >> RG;
	infile >> RW;
	infile >> RWINJ;

	matrix gasNPV(tVec.size(), 1);
	matrix oilNPV(tVec.size(), 1);
	matrix tNPV(tVec.size(), 1);
	gasNPV(0) = 0;
	oilNPV(0) = 0;
	tNPV(0) = 0;
	for (size_t i = 1; i < tVec.size(); ++i){
		gasNPV(i) = gasNPV(i - 1) + (FGPT[i] - FGPT[i - 1]) / 1.028*RG / pow((1 + b), tVec[i] / 365);
		oilNPV(i) = oilNPV(i - 1) + (FOPT[i] - FOPT[i - 1]) *RO / pow((1 + b), tVec[i] / 365);
		tNPV(i) = gasNPV(i) + oilNPV(i);
	}

	dlib::set_colm(outMat, 7) = gasNPV;
	dlib::set_colm(outMat, 8) = oilNPV;
	dlib::set_colm(outMat, 9) = tNPV;

	//---- output
	std::ofstream outfile(filename, std::ios_base::app);
	outfile << outMat;
}

//==================== 2. a class to manage multiple eclipse run , input ,output and visulaize ===============

class Eclipses
{
	public:
	size_t mNecl;
	std::vector<eclipse> mEclVec;

	//--- constructors and updates
	void updateAll(size_t Necl, std::string modelFold, std::string projName, std::string iName);
	void updatePath(size_t Necl, std::string modelFold, std::string projName, std::string iName);
	void updateSch(size_t Necl, std::string iName);
	Eclipses(size_t Necl, std::string modelFold, std::string projName, std::string iName);

	//--- functions 
	void run();

	//--- output functions ---
	void ouputAll(std::string outputFile, std::string eco_filename);


	//----------------------------------------- comp fuctions ---------------------------------------
	void outputSomeFOPT(std::vector<size_t> caseNums, std::string outFileName);
	void compBHP(std::string outFileName, std::string title);
	void compFOPR(std::string outFileName, std::string title);
	void compFOPT(std::string outFileName, std::string title);
	void compFGPR(std::string outFileName, std::string title);
	void compAll(std::string foldNameInOutput);
	void outputFOPT(std::string outFileName);

	void compKeywordValue(std::string keyword, std::string outFileName,
		std::vector<size_t> caseNums, std::string title);
	void compSomeBHP(std::vector<size_t> caseNums, std::string outFileName, std::string title);
	void compSomeCases(std::vector<size_t> caseNums, std::string foldNameInOutput);
};


// <1>-------------------- updates and constructors ---------------------
//!!!!!!!!!!! PS:  output file route did not set as a variable
void Eclipses::updateAll(size_t Necl, std::string modelFold, std::string projName, std::string iName){
	mNecl = Necl;
	mEclVec.resize(Necl);


	std::string inFileName = iName + "/sch.dat";
	std::ifstream infile(inFileName);


	for (size_t i = 0; i < Necl;++i){
		std::string projFold = modelFold + "/case (" + std::to_string(i + 1)+")";
		mEclVec[i].updateAll(projFold, projName, iName);
	}


	for (size_t i = 0; i < Necl; ++i){
		mEclVec[i].updateSch(infile);
	}

	infile.close();
}

void Eclipses::updatePath(size_t Necl, std::string modelFold, std::string projName, std::string iName){
	mNecl = Necl;
	mEclVec.resize(Necl);

	for (size_t i = 0; i < Necl; ++i){
		std::string projFold = modelFold + "/case (" + std::to_string(i + 1) + ")";
		mEclVec[i].updateStatic(projFold, projName, iName);
	}
}

void Eclipses::updateSch(size_t Necl, std::string iName){

	std::string inFileName = iName + "/sch.dat";
	std::ifstream infile(inFileName);

	for (size_t i = 0; i < Necl; ++i){
		mEclVec[i].updateSch(infile);
	}

	infile.close();
}



Eclipses::Eclipses(size_t Necl, std::string modelFold, std::string projName, std::string iName="./input"){
	updatePath(Necl, modelFold, projName, iName);
}


// <2>----------------------------- FUNCTIONS ----------------------------
void Eclipses::run(){
	for (size_t i = 0; i < mNecl; ++i){
		mEclVec[i].run();
	}
}

// <3> -------------------------------ouput Functions ----------------------------------
void Eclipses::ouputAll(std::string outputFile, std::string eco_filename){
	std::ofstream outfile(outputFile);
	//outfile << "\tcase no."<<"\tTIME (DAYS)" << "\tFGPR (MSCF/DAY)" << "\tFGPT (MSCF)" << "\tFOPR(STB/DAY)" << "\tFOPT(STB)" << "\tBHP(PSIA)"
	//	<< "\tNPV Gas($)" << "\tNPV Oil($)" << "\tNPV($)\n\t";
	for (size_t i = 0; i < mNecl; ++i){
		mEclVec[i].outputAll(outputFile, i + 1, eco_filename);
	}

}


//=================================================================================================

// <4>------------------- FUNCTIONS for comparison -----------------------

void Eclipses::compBHP(std::string outFileName = "compBHP.m", std::string title = "Comparison of BHP"){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (size_t i = 0; i < mNecl; ++i){
		std::string str1 = "T" + std::to_string(i + 1);
		std::string str2 = "BHP" + std::to_string(i + 1);

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].mInput.mPresVec, str1, str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('pressure(psi)');\n";
	outfile << "title('" << title << "');\n";
}


void Eclipses::compFOPR(std::string outFileName = "compFOPR.m", std::string title = "Comparison of Oil Rate"){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (size_t i = 0; i < mNecl; ++i){
		std::string str1 = "T" + std::to_string(i + 1);
		std::string str2 = "Ro" + std::to_string(i + 1);

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].readVec("FOPR"),str1,str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('oil rate(stb/day)');\n";
	outfile << "title('" << title << "');\n";

}


void Eclipses::compFOPT(std::string outFileName = "compFOPT.m", std::string title = "Comparison of Total Oil Production"){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (size_t i = 0; i < mNecl; ++i){
		std::string str1 = "T" + std::to_string(i + 1);
		std::string str2 = "OilProd" + std::to_string(i + 1);

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].readVec("FOPT"),  str1, str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('total oil production(stb)');\n";
	outfile << "title('" << title << "');\n";
}


void Eclipses::compFGPR(std::string outFileName = "compFGPR.m", std::string title = "Comparison of Gas Production Rate"){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (size_t i = 0; i < mNecl; ++i){
		std::string str1 = "T" + std::to_string(i + 1);
		std::string str2 = "Rg" + std::to_string(i + 1);

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].readVec("FGPR"),  str1, str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('gas production(MSCF/day)');\n";
	outfile << "title('" << title << "');\n";
}

void Eclipses::compAll(std::string foldNameInOutput){
	compBHP(foldNameInOutput + "/compBHP.m");
	compFGPR(foldNameInOutput + "/compFGPR.m");
	compFOPR(foldNameInOutput + "/compFOPR.m");
	compFOPT(foldNameInOutput + "/compFOPT.m");
}

void Eclipses::outputFOPT(std::string outFileName = "finalFOPT.m"){
	std::ofstream outfile("../Output/" + outFileName);
	for (size_t i = 0; i < mNecl; ++i){
		outfile << mEclVec[i].readFinalFOPT() << std::endl;
	}
}

// <4>------------------- FUNCTIONS for "some" comparison -----------------------
void Eclipses::compKeywordValue(std::string keyword, std::string outFileName,
	std::vector<size_t> caseNums, std::string title){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (auto iter : caseNums){
		std::string str1 = "T" + std::to_string(iter);
		std::string str2 = keyword + std::to_string(iter);

		printMatlabPlot(outfile, mEclVec[iter - 1].mInput.mCumTimeVec, mEclVec[iter - 1].readVec(keyword), str1, str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('" << keyword << "(stb/day)');\n";
	outfile << "title('" << title << "');\n";
	outfile << "grid on;\n";
	outfile << "grid minor ;\n";
	outfile << "xlim([0,500])";
}


void Eclipses::outputSomeFOPT(std::vector<size_t> caseNums, std::string outFileName){
	std::ofstream outfile("../Output/" + outFileName);
	for (auto iter:caseNums){
		outfile << mEclVec[iter-1].readFinalFOPT() << std::endl;
	}
}

void Eclipses::compSomeBHP(std::vector<size_t> caseNums, std::string outFileName, std::string title = "Comparison of BHP"){
	std::ofstream outfile("../Output/" + outFileName);
	outfile << "figure;\n";
	outfile << "hold on;\n";
	for (auto iter : caseNums){
		std::string str1 = "T" + std::to_string(iter);
		std::string str2 = "BHP" + std::to_string(iter);

		printMatlabPlot(outfile, mEclVec[iter - 1].mInput.mCumTimeVec, mEclVec[iter - 1].mInput.mPresVec, str1, str2);
	}
	outfile << "xlabel('time(days)');\n";
	outfile << "ylabel('pressure(psi)');\n";
	outfile << "title('" << title << "');\n";
	outfile << "grid on;\n";
	outfile << "grid minor ;\n";
	outfile << "xlim([0,500])";
}

void Eclipses::compSomeCases(std::vector<size_t> caseNums, std::string foldNameInOutput){
	outputSomeFOPT(caseNums, foldNameInOutput + "/finalFOPT.out");
	compSomeBHP(caseNums, foldNameInOutput + "/compBHP.m");
	compKeywordValue("FGPR", foldNameInOutput + "/compFGPR.m", caseNums, "Comparison of Gas Production Rate");
	compKeywordValue("FOPR", foldNameInOutput + "/compFOPR.m", caseNums, "Comparison of Oil Rate");
	compKeywordValue("FOPT", foldNameInOutput + "/compFOPT.m", caseNums, "Comparison of Total Oil Production");
}

