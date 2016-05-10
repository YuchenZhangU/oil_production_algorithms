#pragma once
#include "read_write.h"
#include "eclipse.h"

void compMultiCases(Eclipses& ecls, std::string caseToComp = "../Input/caseToComp.in"){
	std::vector<std::vector<size_t>> caseNums;
	caseNums = readMultiLines_t(caseToComp);
	for (size_t i = 0; i < caseNums.size(); ++i){
		std::string foldName = "comp (" + std::to_string(i + 1) + ")";
		ecls.compSomeCases(caseNums[i], foldName);
	}
}

int main(){
	// --------------- used to build drop hold schudles---------------
	size_t Necl=32;
	Eclipses ecls_ALL(Necl, "../Model/32_comp_ng_Nc_ratio", "11", "../Input/32_comp_ng_Nc_ratio");
	ecls_ALL.updateSch(Necl, "../Input/32_comp_ng_Nc_ratio");
	ecls_ALL.run();


	////---------------- ouput all data ----------------------
	//std::ifstream infile("caseNo.dat");
	//size_t n;
	//infile >> n;
	//Eclipses eclVec(n, ".", "11");
	//eclVec.ouputAll("outputAll.out", "econPara.dat");


	return 0;
}


//-- test writeWCONPROD
//char* ch = "abb";
//std::cout << ch;
//std::ofstream outfile("test.out");
//writeWCONPROD_BHP(outfile, 6000);
//std::string fName = "test.out";
//outfile.flush();		//or use outfile.close();
//writeWCONPROD_BHP(fName, 5000.5);
//writeWCONPROD_BHP(fName, 5000);

////-- test writeProdSch
//std::vector<double> tVec(10, 1);
//std::vector<double> pVec = tVec;
//writeProdSch("test.out", tVec, pVec);


////-- test writeUniformDrawDown
//std::ofstream outfile("test.out");
//writeUniformDrawDown(outfile, 6000, 1000, double(5000)/360, 30);




////---------- case a (compare tan)----------------
//Eclipses ecls1(4, "../Model/a", "11", "../Input/a");
///*ecls1.run();*/
//ecls1.outputFOPT("a/finalFOPT.out");
//ecls1.compBHP("a/compBHP.m");
//ecls1.compFOPR("a/compFOPR.m");
//ecls1.compFOPT("a/compFOPT.m");
//ecls1.compFGPR("a/compFGPR.m");
// -----------case b (get q -> t_start) ----------------
//Eclipses ecls1(3, "../Model/b", "11", "../Input/b");
//ecls1.run();
//ecls1.outputFOPT("b/finalFOPT.out");
//ecls1.compBHP("b/compBHP.m");
//ecls1.compFOPR("b/compFOPR.m");
//ecls1.compFOPT("b/compFOPT.m");
//ecls1.compFGPR("b/compFGPR.m");
////-- test eclInput
//eclipse ecl1("../Model/1", "11", "../Input");
//eclipse ecl2("../Model/2", "11", "../Input");
//eclipse ecl3("../Model/3", "11", "../Input");
//eclipse ecl4("../Model/4", "11", "../Input");
//std::ifstream schIn("../Input/sch.dat");
//ecl1.mInput.updateSch(schIn);
//ecl2.mInput.updateSch(schIn);
//ecl3.mInput.updateSch(schIn);
//ecl4.mInput.updateSch(schIn);


//double FOPT1 = ecl1.eclFOPT();
//double FOPT2 = ecl2.eclFOPT();
//double FOPT3 = ecl3.eclFOPT();
//double FOPT4 = ecl4.eclFOPT();
//
//std::ofstream foptOut("../Output/FOPT.out");
//foptOut << FOPT1 << "\n";
//foptOut << FOPT2 << "\n";
//foptOut << FOPT3 << "\n";
//foptOut << FOPT4 << "\n";

//std::ofstream foprPlotOut("../Output/FOPR.m");
//foprPlotOut << "figure;\n";
//printMatlabPlot(foprPlotOut, ecl1.mInput.mCumTimeVec, ecl1.readFOPR());
//foprPlotOut << "hold on;\n";
//printMatlabPlot(foprPlotOut, ecl2.mInput.mCumTimeVec, ecl2.readFOPR());
//printMatlabPlot(foprPlotOut, ecl3.mInput.mCumTimeVec, ecl3.readFOPR());
//printMatlabPlot(foprPlotOut, ecl4.mInput.mCumTimeVec, ecl4.readFOPR());
//eclInput input("input");
//std::ofstream outfile("test.out");
//writeWELSPECS_Prod(outfile, input.mWells[0].depth);
//writeCOMPDATs(outfile, input.mWells[0]);
//writeProdSch(outfile, input);
////-- test writes
//std::ofstream outfile("test.out");
//writeWELSPECS_Prod(outfile, 10500);
//std::vector<double> perfVec(4, 1);
//writeCOMPDAT(outfile, perfVec);
//writeWCONPROD_BHP(outfile, 5000);