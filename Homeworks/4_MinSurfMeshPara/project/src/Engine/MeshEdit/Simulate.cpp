#include <Engine/MeshEdit/Simulate.h>
#include <Eigen/Sparse>

using namespace Ubpa;
using namespace std;
using namespace Eigen;

void Simulate::Clear() {
	this->positions.clear();
	this->velocity.clear();
	this->externalF.clear();
	this->mass.clear();
	this->length.clear();
}
bool Simulate::Run()
{
	switch (simulatetype)
	{
	case SimulateType::ImplicitEuler:
		SimulateOnce_ImplicitEuler();
		break;

	case SimulateType::ProjectiveDynamic:
		SimulateOnce_ProjectiveDynamic();
		break;
	}

	return true;
}
bool Simulate::Init()
{
	//Clear();

	//ImplicitEuler
	size_t m = positions.size();//质点个数
	size_t s = edgelist.size() / 2;//弹簧个数
	this->velocity.resize(m);
	this->externalF.resize(m);
	this->mass.resize(m);
	this->length.resize(s);

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			this->velocity[i][j] = 0;
		}
		this->mass[i] = Mass;
		this->externalF[i][0] = 0;
		this->externalF[i][1] = -mass[i] * g;
		this->externalF[i][2] = 0;
	}
	for (size_t i = 0; i < s; i++)
	{
		length[i] = (positions[edgelist[2 * i]] - positions[edgelist[2 * i + 1]]).norm();
	}

	//Accelerate
	// 
	//K
	size_t m_free = m - fixed_id.size();
	vector<Triplet<float>> tripletList_K;
	tripletList_K.reserve(m_free * 3);
	size_t num = 0;
	for (size_t i = 0; i < m; i++)
	{
		if (!isfixed(i))
		{
			tripletList_K.push_back(Triplet<float>(num, 3 * i, 1));
			tripletList_K.push_back(Triplet<float>(num + 1, 3 * i + 1, 1));
			tripletList_K.push_back(Triplet<float>(num + 2, 3 * i + 2, 1));
			num += 3;
		}
	}
	K = Eigen::SparseMatrix<float>(3 * m_free, 3 * m);
	K.setFromTriplets(tripletList_K.begin(), tripletList_K.end());

	//M
	vector<Triplet<float>> tripletList_M;
	tripletList_M.reserve(m * 3);
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			tripletList_M.push_back(Triplet<float>(3 * i + j, 3 * i + j, mass[i]));
		}
	}
	M = Eigen::SparseMatrix<float>(3 * m, 3 * m);
	M.setFromTriplets(tripletList_M.begin(), tripletList_M.end());

	//L
	vector<Triplet<float>> tripletList_L;
	tripletList_L.reserve(s * 12);
	for (size_t i = 0; i < s; i++)
	{
		size_t x1 = edgelist[2 * i];
		size_t x2 = edgelist[2 * i + 1];
		for (size_t j = 0; j < 3; j++)
		{
			tripletList_L.push_back(Triplet<float>(3 * x1 + j, 3 * x1 + j, stiff));
			tripletList_L.push_back(Triplet<float>(3 * x1 + j, 3 * x2 + j, -stiff));
			tripletList_L.push_back(Triplet<float>(3 * x2 + j, 3 * x1 + j, -stiff));
			tripletList_L.push_back(Triplet<float>(3 * x2 + j, 3 * x2 + j, stiff));
		}
	}
	L = Eigen::SparseMatrix<float>(3 * m, 3 * m);
	L.setFromTriplets(tripletList_L.begin(), tripletList_L.end());

	//J
	vector<Triplet<float>> tripletList_J;
	tripletList_J.reserve(s * 36);
	for (size_t i = 0; i < s; i++)
	{
		size_t x1 = edgelist[2 * i];
		size_t x2 = edgelist[2 * i + 1];
		for (size_t j = 0; j < 3; j++)
		{
			tripletList_J.push_back(Triplet<float>(3 * x1 + j, 3 * i + j, stiff));
			tripletList_J.push_back(Triplet<float>(3 * x2 + j, 3 * i + j, -stiff));
		}
	}
	J = Eigen::SparseMatrix<float>(3 * m, 3 * s);
	J.setFromTriplets(tripletList_J.begin(), tripletList_J.end());

	//b
	vector<Triplet<float>> tripletList_b;
	tripletList_b.reserve(m * 3);
	for (size_t i = 0; i < m; i++)
	{
		if (isfixed(i))
		{
			for (size_t j = 0; j < 3; j++)
			{
				tripletList_b.push_back(Triplet<float>(3 * i + j, 0, positions[i][j]));
			}
		}
	}
	b = Eigen::SparseMatrix<float>(3 * m, 1);
	b.setFromTriplets(tripletList_b.begin(), tripletList_b.end());

	K_M_h2_L_Kt = K * (M + h * h * L) * K.transpose();
	K_h2_J = K * h * h * J;
	K_M = K * M;
	_K_M_h2_L_b = -K * (M + h * h * L) * b;

	K_h2_J = K_M_h2_L_Kt.transpose() * K_h2_J;
	K_M = K_M_h2_L_Kt.transpose() * K_M;
	_K_M_h2_L_b = K_M_h2_L_Kt.transpose() * _K_M_h2_L_b;
	solver_acc.compute(K_M_h2_L_Kt.transpose()* K_M_h2_L_Kt);

	return true;
}
void Simulate::SimulateOnce_ImplicitEuler()
{
	size_t m = positions.size();
	size_t s = length.size();
	vector<pointf3> y;
	vector<pointf3> X;
	y.resize(m);
	X.resize(m);
	for (size_t i = 0; i < m; i++)
	{
		if (isfixed(i))
		{
			for (size_t j = 0; j < 3; j++)
			{
				y[i][j] = positions[i][j];
				X[i][j] = y[i][j];
			}
		}
		else
		{
			for (size_t j = 0; j < 3; j++)
			{
				y[i][j] = positions[i][j] + h * velocity[i][j] + h * h * externalF[i][j] / mass[i];
				X[i][j] = y[i][j];
			}
		}
	}
	int num = 5;
	float error = 0;
	do {
		Eigen::SparseMatrix<float> Ggrad(3 * m, 3 * m);
		vector<Triplet<float>> tripletList;
		tripletList.reserve(s * 48 + m * 3);
		MatrixXf G(3 * m, 1);
		for (size_t i = 0; i < m; i++)
		{
			tripletList.push_back(Triplet<float>(3 * i, 3 * i, mass[i]));
			tripletList.push_back(Triplet<float>(3 * i + 1, 3 * i + 1, mass[i]));
			tripletList.push_back(Triplet<float>(3 * i + 2, 3 * i + 2, mass[i]));
			for (size_t j = 0; j < 3; j++)
			{
				G(3 * i + j, 0) = mass[i] * (X[i][j] - y[i][j]);
			}
		}
		for (size_t i = 0; i < s; i++)
		{
			size_t x1 = edgelist[2 * i];
			size_t x2 = edgelist[2 * i + 1];
			float initL = length[i];
			float nowL = (X[x1] - X[x2]).norm();
			float temp1 = stiff * h * h * (1 - initL / nowL);
			float temp2 = stiff * h * h * initL / nowL / nowL / nowL;
			pointf3 r = (X[x1] - X[x2]).cast_to<pointf3>();
			if (!isfixed(x1))
			{
				for (size_t j = 0; j < 3; j++)
				{
					tripletList.push_back(Triplet<float>(3 * x1 + j, 3 * x1 + j, temp1));
					tripletList.push_back(Triplet<float>(3 * x1 + j, 3 * x2 + j, -temp1));
					for (size_t k = 0; k < 3; k++)
					{
						tripletList.push_back(Triplet<float>(3 * x1 + j, 3 * x1 + k, temp2 * r[j] * r[k]));
						tripletList.push_back(Triplet<float>(3 * x1 + j, 3 * x2 + k, -temp2 * r[j] * r[k]));
					}
					G(3 * x1 + j, 0) = G(3 * x1 + j, 0) + temp1 * r[j];
				}
			}
			if (!isfixed(x2))
			{
				for (size_t j = 0; j < 3; j++)
				{
					tripletList.push_back(Triplet<float>(3 * x2 + j, 3 * x2 + j, temp1));
					tripletList.push_back(Triplet<float>(3 * x2 + j, 3 * x1 + j, -temp1));
					for (size_t k = 0; k < 3; k++)
					{
						tripletList.push_back(Triplet<float>(3 * x2 + j, 3 * x2 + k, temp2 * r[j] * r[k]));
						tripletList.push_back(Triplet<float>(3 * x2 + j, 3 * x1 + k, -temp2 * r[j] * r[k]));
					}
					G(3 * x2 + j, 0) = G(3 * x2 + j, 0) - temp1 * r[j];
				}
			}
		}
		SimplicialLLT<SparseMatrix<float>> solver;
		Ggrad.setFromTriplets(tripletList.begin(), tripletList.end());
		solver.compute(Ggrad.transpose() * Ggrad);
		MatrixXf result = solver.solve(Ggrad.transpose() * G);
		for (size_t i = 0; i < m; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				X[i][j] = X[i][j] - result(3 * i + j);
			}
		}
		error = fabs(result.maxCoeff()) > fabs(result.minCoeff()) ? fabs(result.maxCoeff()) : fabs(result.minCoeff());
		num--;
	} while (error > 1e-5 && num > 0);
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			this->velocity[i][j] = (X[i][j] - this->positions[i][j]) / h;
			this->positions[i][j] = X[i][j];
		}
	}
	cout << "Simulate::SimulateOnce_ImplicitEuler() Done!" << endl;
}
void Simulate::SimulateOnce_ProjectiveDynamic()
{
	size_t m = positions.size();
	size_t s = length.size();
	MatrixXf X(3 * m, 1);
	MatrixXf y(3 * m, 1);
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			y(3 * i + j, 0) = positions[i][j] + h * velocity[i][j] + h * h * externalF[i][j] / mass[i];
			X(3 * i + j, 0) = positions[i][j];
		}
	}

	int num = 5;
	float error = 0;
	do {
		//更新d
		//Update d
		MatrixXf d(3 * s, 1);
		for (size_t i = 0; i < s; i++)
		{
			size_t x1 = edgelist[2 * i];
			size_t x2 = edgelist[2 * i + 1];
			vecf3 r(X(3 * x1, 0) - X(3 * x2, 0), X(3 * x1 + 1, 0) - X(3 * x2 + 1, 0), X(3 * x1 + 2, 0) - X(3 * x2 + 2, 0));
			r = r / r.norm() * length[i];
			for (size_t j = 0; j < 3; j++)
			{
				d(3 * i + j, 0) = r[j];
			}
		}
		//更新X
		MatrixXf Xf = solver_acc.solve(K_h2_J * d + K_M * y + _K_M_h2_L_b);
		MatrixXf diff = K * X - Xf;
		error = fabs(diff.maxCoeff()) > fabs(diff.minCoeff()) ? fabs(diff.maxCoeff()) : fabs(diff.minCoeff());
		num--;
		X = K.transpose() * Xf + b;
	} while (error > 1e-5 && num > 0); 
	
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			this->velocity[i][j] = (X(3 * i + j, 0) - this->positions[i][j]) / h;
			this->positions[i][j] = X(3 * i + j, 0);
		}
	}
	cout << "Simulate::SimulateOnce_Accelerate() Done!" << endl;
}
void Simulate::Set2PointFix()
{
	fixed_id.clear();
	double minz = positions[0][2];
	for (int i = 0; i < positions.size(); i++)
	{
		if (positions[i][2] < minz)
		{
			minz = positions[i][2];
		}
	}
	vector<unsigned> temp;
	for (int i = 0; i < positions.size(); i++)
	{
		if (positions[i][2] == minz)
		{
			temp.push_back(i);
		}
	}
	double maxx = positions[temp[0]][0];
	double minx = positions[temp[0]][0];
	unsigned maxxid = temp[0];
	unsigned minxid = temp[0];
	for (int i = 0; i < temp.size(); i++)
	{
		if (positions[temp[i]][0] > maxx)
		{
			maxx = positions[i][0];
			maxxid = temp[i];
		}
		if (positions[temp[i]][0] < minx)
		{
			minx = positions[i][0];
			minxid = temp[i];
		}
	}
	fixed_id.push_back(maxxid);
	fixed_id.push_back(minxid);
	Init();
}
void Simulate::SetLeftFix()
{
	//固定网格x坐标最小点
	fixed_id.clear();
	double x = 100000;
	for (int i = 0; i < positions.size(); i++)
	{
		if (positions[i][0] < x)
		{
			x = positions[i][0];
		}
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - x) < 1e-5)
		{
			fixed_id.push_back(i);
		}
	}
	Init();
}
bool Simulate::isfixed(unsigned id)
{
	for (size_t i = 0; i < fixed_id.size(); i++)
	{
		if (id == fixed_id[i])
		{
			return true;
		}
	}
	return false;
}
