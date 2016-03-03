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
	eclipse(){ ; }
	eclipse(std::string fName, std::string pName, std::string iName);
	void updateAll(std::string fName, std::string pName, std::string iName);
	void updatesch(std::ifstream& infile);

	//--- functions
	void writeSchFile();
	void run();
	//---------- read functions
	std::vector<double> readFOPT();
	std::vector<double> readFOPR();
	std::vector<double> readVec(std::string keyword);

	double readFinalFOPT();
	double readNPV();

};


// <1>---------------- update and constructor --------------
void eclipse::updatesch(std::ifstream& infile){
	mInput.updateSch(infile);
}

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

void eclipse::updateAll(std::string fName, std::string pName, std::string iName){
	mProjFold = fName;
	mProjName = pName;
	mInputFold = iName;
	mInput.updateAll(mInputFold);		//initilize completion & schedule data
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
	writeSchFile();
	writeLocalBatchFile();

}

eclipse::eclipse(std::string fName, std::string pName, std::string iName){
	updateAll(fName, pName, iName);
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



// <3> -----------------Read Functions -------------------
std::vector<double> eclipse::readFOPT(){
	std::string proj = mProjFold + "/" + mProjName;
	std::vector<double> FOPTvec = readMultiTimeStepValue(proj, "FOPT");
	return FOPTvec;
}



std::vector<double> eclipse::readFOPR() {
	std::string proj = mProjFold + "/" + mProjName;
	std::vector<double> FOPRvec = readMultiTimeStepValue(proj, "FOPR");
	return FOPRvec;
}

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




//==================== 2. a class to manage multiple eclipse run , input ,output and visulaize ===============

class Eclipses
{
	public:
	size_t mNecl;
	std::vector<eclipse> mEclVec;

	//--- constructors and updates
	void updateAll(size_t Necl, std::string modelFold, std::string projName, std::string iName);
	Eclipses(size_t Necl, std::string modelFold, std::string projName, std::string iName);

	//--- functions 
	void run();

	void compBHP(std::string outFileName, std::string title);
	void compFOPR(std::string outFileName,std::string title);
	void compFOPT(std::string outFileName,  std::string title);
	void compFGPR(std::string outFileName, std::string title );
	void compAll(std::string foldNameInOutput);
	void outputFOPT(std::string outFileName);

	void compKeywordValue(std::string keyword, std::string outFileName,
		std::vector<size_t> caseNums, std::string title);
	void compSomeBHP(std::vector<size_t> caseNums, std::string outFileName, std::string title);
	void outputSomeFOPT(std::vector<size_t> caseNums, std::string outFileName);
	void compSomeCases(std::vector<size_t> caseNums, std::string foldNameInOutput);

	void outputRawFOPR(std::string outFileName);
	void outputRawFOPT(std::string outFileName);
	void outputNPV(std::string outFileName);
	void outputBHP(std::string outFileName);
	void outputRaw(std::string foldNameInOutput);
};


// <1>-------------------- updates and constructors ---------------------
//!!!!!!!!!!! PS:  output file route did not set as a variable
void Eclipses::updateAll(size_t Necl, std::string modelFold, std::string projName, std::string iName){
	mNecl = Necl;
	mEclVec.resize(Necl);


	std::string schName = iName + "/sch.dat";
	std::ifstream schIn(schName);
	for (size_t i = 0; i < Necl; ++i){
		mEclVec[i].updatesch(schIn);
	}


	for (size_t i = 0; i < Necl;++i){
		std::string projFold = modelFold + "/case (" + std::to_string(i + 1)+")";
		mEclVec[i].updateAll(projFold, projName, iName);
	}
	
}


Eclipses::Eclipses(size_t Necl, std::string modelFold, std::string projName, std::string iName){
	updateAll(Necl, modelFold, projName, iName);
}


// <2>----------------------------- FUNCTIONS ----------------------------
void Eclipses::run(){
	for (size_t i = 0; i < mNecl; ++i){
		mEclVec[i].run();
	}
}



// <3>------------------- FUNCTIONS for comparison -----------------------

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

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].readFOPR(),str1,str2);
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

		printMatlabPlot(outfile, mEclVec[i].mInput.mCumTimeVec, mEclVec[i].readFOPT(),  str1, str2);
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



// <5>--------------- function for raw data output ----------------
void Eclipses::outputRawFOPR(std::string outFileName) {
	std::ofstream outfile("../Output/" + outFileName);
	for (size_t i = 0; i < mNecl; ++i){
		std::vector<double> timeVec = mEclVec[i].mInput.mCumTimeVec;
		std::vector<double> FOPRvec = mEclVec[i].readVec("FOPR");
		outfile <<"0 "<< dlib::trans(vec2mat(timeVec));
		outfile <<"0 "<< dlib::trans(vec2mat(FOPRvec));
	}
}

void Eclipses::outputRawFOPT(std::string outFileName) {
	std::ofstream outfile("../Output/" + outFileName);
	for (size_t i = 0; i < mNecl; ++i){
		std::vector<double> timeVec = mEclVec[i].mInput.mCumTimeVec;
		std::vector<double> FOPTvec = mEclVec[i].readVec("FOPT");
		outfile << "\t0 "<<dlib::trans(vec2mat(timeVec));
		outfile <<"\t0 "<< dlib::trans(vec2mat(FOPTvec));
	}
}

void Eclipses::outputNPV(std::string outFileName){
	std::ofstream outfile("../Output/" + outFileName);
	for (size_t i = 0; i < mNecl; ++i){
		double NPV = mEclVec[i].readNPV();
		outfile << NPV << std::endl;
	}
}


void Eclipses::outputBHP(std::string outFileName){
	std::ofstream outfile("../Output/" + outFileName);
	for (size_t i = 0; i < mNecl; ++i){
		std::vector<double> timeVec = mEclVec[i].mInput.mCumTimeVec;
		std::vector<double> BHPvec = mEclVec[i].mInput.mPresVec;
		outfile << "\t0 "<<dlib::trans(vec2mat(timeVec));
		outfile << "\t" << mEclVec[i].mInput.mUp<<" "
			<< dlib::trans(vec2mat(BHPvec));
	}
}

void Eclipses::outputRaw(std::string foldNameInOutput){
	/*outputNPV(foldNameInOutput + "/rawNPV.out");*/
	outputBHP(foldNameInOutput + "/rawBHP.out");
	outputRawFOPT(foldNameInOutput + "/rawFOPT.out");
	outputRawFOPR(foldNameInOutput + "/rawFOPR.out");
	
}