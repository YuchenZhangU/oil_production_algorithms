#include<dlib/matrix.h>
#include<vector>

typedef dlib::matrix<double> matrix;


//********************** 1. vector and matrix convert *****************
//TESTED!
template <typename T>
//convert 1-D vector to a column matrix(vec)
dlib::matrix<T> vec2mat(const std::vector<T>& vec)
{
	dlib::matrix<T, 0, 1> mat;
	mat.set_size(vec.size());
	for (size_t i = 0; i < vec.size(); i++)
	{
		mat(i) = vec[i];
	}
	return mat;
}

//TESTED!
template <typename T>
// convert 1-D matrix to vector
std::vector<T> mat2vec(const dlib::matrix<T>& mat)
{
	std::vector<T> vec;
	vec.resize(mat.size());
	for (size_t i = 0; i < mat.size(); i++)
	{
		vec[i] = mat(i);
	}
	return vec;
}

template <typename T>
// convert N-D vector to a matrix
dlib::matrix<T> vec2mat(const std::vector<std::vector<T>>& vecVec){
	dlib::matrix<T> M(vecVec.size(), vecVec[0].size());
	size_t Nc = vecVec[0].size();
	for (size_t i = 0; i < vecVec.size(); ++i){
		if (vecVec[i].size() != Nc)
			std::cerr << "the vector<vector> is not a square matrix!" << std::endl;
		dlib::set_rowm(M, i) = dlib::trans(vec2mat(vecVec[i]));
	}
	return M;
}

// vecVec'
template<typename T>
std::vector<std::vector<T>> transVec(std::vector<std::vector<T>> vecVec){
	std::vector<std::vector<T>> vV;
	vV.resize(vecVec[0].size());
	for (auto& it : vV){
		it.resize(vecVec.size());
	}
	for (size_t i = 0; i < vecVec.size(); ++i){
		for (size_t j = 0; j < vecVec[i].size(); ++j){
			vV[j][i] = vecVec[i][j];
		}
	}
	return vV;
}


//************************ 2. Scaling ******************************
//scale the vector to [0,1] domain using one lower and upper bound
dlib::matrix<double> forwardScale(dlib::matrix<double> X, double x_lower, double x_upper)
{
	dlib::matrix<double> X_scaled = 1 / (x_upper - x_lower) * (X - x_lower);
	return X_scaled;
}


dlib::matrix<double> backwardScale(dlib::matrix<double> X_scaled, double x_lower, double x_upper)
{
	dlib::matrix<double> X = (x_upper - x_lower) * X_scaled + x_lower;
	return X;
}



//scale the vector to [0,1] domain using multiple lower and upper bounds
matrix scaleWellControl(const matrix& X, const std::vector<double>& lowerV,
	const std::vector<double>& upperV, int Nc, int Nw)
{
	matrix X_scaled(X.nr(), X.nc());

	// each well have different upper and lower so that may have differnt scaled criterion
	for (int i = 0; i < Nw; ++i)
	{
		dlib::set_rowm(X_scaled, dlib::range(i*Nc, (i + 1)*Nc - 1)) =
			forwardScale(dlib::rowm(X, dlib::range(i*Nc, (i + 1)*Nc - 1)), lowerV[i], upperV[i]);
	}
	return X_scaled;
}


matrix backScaleWellControl(const matrix& X_scaled, const std::vector<double>& lowerV,
	const std::vector<double>& upperV, int Nc, int Nw)
{
	matrix X(X_scaled.nr(), X_scaled.nc());

	for (int i = 0; i < Nw; ++i)
	{
		dlib::set_rowm(X, dlib::range(i*Nc, (i + 1)*Nc - 1)) =
			backwardScale(dlib::rowm(X_scaled, dlib::range(i*Nc, (i + 1)*Nc - 1)), lowerV[i], upperV[i]);
	}
	return X;
}



//********************* 3. matrix vector operation ***************
//double normMatVec(const matrix& X){
//	return sqrt(dlib::sum(dlib::trans(X)*X));
//}


// compute X_bar, the average of the generated ensembles
matrix colSum(const matrix& X) {
	matrix xColSum = dlib::zeros_matrix<double>(X.nr(), 1);
	for (int i = 0; i < X.nc(); ++i) {
		xColSum += dlib::colm(X, i);
	}
	return xColSum;
}

void matZeroBound(matrix& X){
	for (size_t i = 0; i < X.nr(); ++i){
		for (size_t j = 0; j < X.nc(); ++j){
			if (X(i, j)<0)
				X(i, j) = 0;
		}
	}
}

void matOneBound(matrix& X) {
	for (size_t i = 0; i < X.nr(); ++i){
		for (size_t j = 0; j < X.nc(); ++j){
			if (X(i, j)>1)
				X(i, j) = 1;
		}
	}
}

template<typename T>
size_t vecMaxInd(const std::vector<T>& vec){
	size_t ind = 0;
	T max = vec[0];
	for (size_t i = 1; i < vec.size(); ++i){
		if (max < vec[i]){
			ind = i;
			max = vec[i];
		}
			
	}

	return ind;
}

double matMax(const matrix& M) {
	std::vector<double> vec = mat2vec(M);
	double max = *std::max_element(vec.begin(), vec.end());
	return max;
}


void printMatlabPlot(std::ofstream& outfile, const matrix& x1, const matrix& x2, const std::string& str1="X", const std::string& str2 = "Y"){
	outfile << str1 <<" = [" << x1 << "];\n";
	outfile << str2 << " = [" << x2 << "];\n";
	outfile << "plot("<<str1<<","<<str2<<",'LineWidth',2)";
}

void printMatlabPlot(std::ofstream& outfile, const std::vector<double>& x1,
	const std::vector<double>& x2, std::string str1 = "X", std::string str2 = "Y"){
	outfile << str1 << " = [" << dlib::trans(vec2mat(x1)) << "];\n";
	outfile << str2 << " = [" << dlib::trans(vec2mat(x2)) << "];\n";
	outfile << "plot(" << str1 << "," << str2 << ",'LineWidth',2);\n\n";
}

//********************* 4. time ***************
std::string getCurrentTimeStr(){
	time_t now = time(0);
	// convert now to string form
	std::string dt = ctime(&now);
	std::string curTime;
	curTime = dt ;
	return curTime;
}


