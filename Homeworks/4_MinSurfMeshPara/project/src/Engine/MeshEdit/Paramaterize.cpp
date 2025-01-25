#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <numeric>
using namespace Ubpa;

using namespace std;
using namespace Eigen;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void Paramaterize::Clear()
{
	heMesh->Clear();
	triMesh = nullptr;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh)
{
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

bool Paramaterize::Run(size_t n) {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::Paramaterize::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	paramaterize(n);

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);

	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());

	////old version
	//for (auto f : heMesh->Polygons()) { // f is triangle
	//	for (auto v : f->BoundaryVertice()) { // vertices of the triangle
	//		indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	//		cout << static_cast<unsigned>(heMesh->Index(v) )<< endl;
	//	}
	//}
	
	//new version
	//to avoid a triangle'points is all boundary
	//our aim is to change indice
	vector<THalfEdge<V, E, P>*>He;
	for (auto f : heMesh->Polygons())
	{
		auto boundary = f->BoundaryVertice();
		auto v0 = boundary[0];
		auto v1 = boundary[1];
		auto v2 = boundary[2];
		if (v0->IsBoundary() && v1->IsBoundary() && v2->IsBoundary())
		{
			for (auto he : f->BoundaryHEs())
			{
				if (!he->Pair()->IsBoundary())
				{
					He.push_back(he);
				}
			}
		}
	}
	for (auto f : heMesh->Polygons()) 
	{
		int mark = 0;
		for (auto he1 : f->BoundaryHEs())
		{
			for (auto he2 : He)
			{
				if (he1 == he2)mark = 1;
				if (he1 == he2->Pair())mark = 1;
			}
		}
		if (mark == 1)continue;
		else
		{
			auto boundary = f->BoundaryVertice();
			for (size_t i = 0; i < 3; i++)
			{
				indice.push_back(static_cast<unsigned>(heMesh->Index(boundary[i])));
			}
		}
	}
	for (auto he : He)
	{
		auto v0 = static_cast<unsigned>(heMesh->Index(he->Origin()));
		auto v1 = static_cast<unsigned>(heMesh->Index(he->End()));
		auto v2 = static_cast<unsigned>(heMesh->Index(he->Next()->End()));
		auto v3 = static_cast<unsigned>(heMesh->Index(he->Pair()->Next()->End()));
		indice.push_back(v1);
		indice.push_back(v2);
		indice.push_back(v3);

		indice.push_back(v0);
		indice.push_back(v3);
		indice.push_back(v2);
	}
	triMesh->Init(indice, positions);
	return true;
}

bool Paramaterize::SetTexcoords(size_t n) {

	if (n == 1)
	{
		Set_LaplaceMatrix_Uniform();
		Set_BoundaryMatrix_Circle();
	}
	else if (n == 2)
	{
		Set_LaplaceMatrix_Cotangent();
		Set_BoundaryMatrix_Circle();
	}
	else if (n == 3)
	{
		Set_LaplaceMatrix_Uniform();
		Set_BoundaryMatrix_Square();
	}
	else if (n == 4)
	{
		Set_LaplaceMatrix_Cotangent();
		Set_BoundaryMatrix_Square();
	}

	SparseLU<SparseMatrix<float>> solver;
	solver.compute(LaplaceMatrix);
	MatrixXf new_pos = solver.solve(BoundaryMatrix);

	size_t nV = heMesh->NumVertices();
	vector<pointf2> texcoords;
	texcoords.reserve(nV);
	for (int i = 0; i < nV; i++)
	{
		vecf2 v1;
		v1[0] = new_pos(i, 0);
		v1[1] = new_pos(i, 1);
		texcoords.push_back(v1.cast_to<pointf2>());
	}

	triMesh->Update(texcoords);
	return true;
}

void Paramaterize::paramaterize(size_t n)
{
	if (n == 1)
	{
		Set_LaplaceMatrix_Uniform();
		Set_BoundaryMatrix_Circle();
	}
	else if (n == 2)
	{
		Set_LaplaceMatrix_Cotangent();
		Set_BoundaryMatrix_Circle();
	}
	else if (n == 3)
	{
		Set_LaplaceMatrix_Uniform();
		Set_BoundaryMatrix_Square();
	}
	else if (n == 4)
	{
		Set_LaplaceMatrix_Cotangent();
		Set_BoundaryMatrix_Square();
	}

	SparseLU<SparseMatrix<float>> solver;
	solver.compute(LaplaceMatrix);

	MatrixXf new_pos = solver.solve(BoundaryMatrix);

	for (auto v : heMesh->Vertices())
	{
		size_t id = heMesh->Index(v);
		for (size_t i = 0; i < 3; i++)
		{
			v->pos[i] = new_pos(id, i);
		}
	}
	
	cout << "WARNING::Paramaterize::paramaterize:" << endl
		<< "\t" << "Success!" << endl;
}

void Paramaterize::Set_LaplaceMatrix_Uniform()
{
	size_t nV = heMesh->NumVertices();
	vector<Triplet<float>> tripletList;
	tripletList.reserve(heMesh->NumHalfEdges() + nV);
	for (auto v : heMesh->Vertices())
	{
		size_t id = heMesh->Index(v);
		tripletList.push_back(Triplet<float>(id, id, 1));
		if (!v->IsBoundary())
		{
			for (auto v_adjvex : v->AdjVertices())
			{
				tripletList.push_back(Triplet<float>(id, heMesh->Index(v_adjvex), -1 / static_cast<float>(v->Degree())));
			}
		}
	}
	LaplaceMatrix = SparseMatrix<float>(nV, nV);
	LaplaceMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
}
void Paramaterize::Set_LaplaceMatrix_Cotangent()
{
	size_t nV = heMesh->NumVertices();

	vector<Triplet<float>> tripletList;
	tripletList.reserve(heMesh->NumHalfEdges() + nV);
	for (auto v : heMesh->Vertices())
	{
		size_t id = heMesh->Index(v);
		tripletList.push_back(Triplet<float>(id, id, 1));
		if (!v->IsBoundary())
		{
			size_t d = v->Degree();
			vector<float>cotvalue=CotValue(v);
			for (size_t i = 0; i < d; i++)
			{
				auto v_adjvex = v->AdjVertices()[i];
				tripletList.push_back(Triplet<float>(id, heMesh->Index(v_adjvex), cotvalue[i]));
			}
		}
	}
	LaplaceMatrix = Eigen::SparseMatrix<float>(nV, nV);
	LaplaceMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
}

void Paramaterize::Set_BoundaryMatrix_Circle()
{
	size_t nV = heMesh->NumVertices();

	BoundaryMatrix = MatrixXf::Zero(nV, 3);
	auto pre_boundary = heMesh->Boundaries()[0];//只考虑一个边界的情况

	//为了参数网格和原网格相比不旋转
	float max_x = pre_boundary[0]->Origin()->pos[0];
	size_t max_i = 0;
	for (size_t i = 0; i < pre_boundary.size(); i++)
	{
		auto v = pre_boundary[i]->Origin();
		size_t id = heMesh->Index(v);
		if (v->pos[0] > max_x)
		{
			max_x = v->pos[0];
			max_i = i;
		}
	}
	auto boundary = pre_boundary[max_i]->NextLoop();

	size_t nB = boundary.size();
	size_t k = nB;
	float f = 2 * PI<float> / nB;

	for (auto he : boundary)
	{
		auto v = he->Origin();
		size_t id = heMesh->Index(v);
		BoundaryMatrix(id, 0) = cos(f * k) / 2 + 0.5;
		BoundaryMatrix(id, 1) = sin(f * k) / 2 + 0.5;
		k--;
	}
}
void Paramaterize::Set_BoundaryMatrix_Square()
{
	size_t nV = heMesh->NumVertices();

	BoundaryMatrix = MatrixXf::Zero(nV, 3);
	auto pre_boundary = heMesh->Boundaries()[0];//只考虑一个边界的情况

	//为了参数网格和原网格相比不旋转
	float max_x = pre_boundary[0]->Origin()->pos[0];
	size_t max_i = 0;
	for (size_t i = 0; i < pre_boundary.size(); i++)
	{
		auto v = pre_boundary[i]->Origin();
		size_t id = heMesh->Index(v);
		if (v->pos[0] > max_x)
		{
			max_x = v->pos[0];
			max_i = i;
		}
	}
	auto boundary = pre_boundary[max_i]->NextLoop();

	size_t nB = boundary.size();
	float a1, a2, a3, a4;
	a1 = nB / 8;
	a2 = 3 * nB / 8;
	a3 = 5 * nB / 8;
	a4 = 7 * nB / 8;
	for (size_t i = 0; i < nB; i++)
	{
		auto v = boundary[i]->Origin();
		size_t id = heMesh->Index(v);
		if (0 <= i && i < a1)
		{
			BoundaryMatrix(id, 0) = 1;
			BoundaryMatrix(id, 1) = -static_cast<float>(i) / a1 / 2 + 0.5;
		}
		else if (a1 <= i && i < a2)
		{
			BoundaryMatrix(id, 0) = (a1 + a2 - 2 * i) / (a2 - a1) / 2 + 0.5;
			BoundaryMatrix(id, 1) = 0;
		}
		else if (a2 <= i && i < a3)
		{
			BoundaryMatrix(id, 0) = 0;
			BoundaryMatrix(id, 1) = (a2 + a3 - 2 * i) / (a2 - a3) / 2 + 0.5;
		}
		else if (a3 <= i && i < a4)
		{
			BoundaryMatrix(id, 0) = (a3 + a4 - 2 * i) / (a3 - a4) / 2 + 0.5;
			BoundaryMatrix(id, 1) = 1;
		}
		else
		{
			BoundaryMatrix(id, 0) = 1;
			BoundaryMatrix(id, 1) = static_cast<float>(nB - i) / (static_cast<float>(nB) - a4) / 2 + 0.5;
		}
	}
}

std::vector<float> Paramaterize::CotValue(V* v)
{
	size_t d = v->Degree();
	vector<Vector3f>adjpos(d);
	for (size_t i = 0; i < d; i++)
	{
		for (size_t k = 0; k < 3; k++)
		{
			adjpos[i][k] = v->AdjVertices()[i]->pos[k];
		}
	}
	Vector3f vpos(v->pos[0], v->pos[1], v->pos[2]);
	vector<float>cotvalue(d);
	cotvalue[0] = Cotan(vpos, adjpos[d - 1], adjpos[0], adjpos[1]);
	for (size_t i = 1; i < d - 1; i++)
	{
		cotvalue[i] = Cotan(vpos, adjpos[i - 1], adjpos[i], adjpos[i + 1]);
	}
	cotvalue[d - 1] = Cotan(vpos, adjpos[d - 2], adjpos[d - 1], adjpos[0]);
	float sum = accumulate(begin(cotvalue), end(cotvalue), 0.0);
	for (size_t i = 0; i < d; i++)
	{
		cotvalue[i] = -cotvalue[i] / sum;
	}
	return cotvalue;
}
float Paramaterize::Cotan(Eigen::Vector3f v, Eigen::Vector3f a1, Eigen::Vector3f a2, Eigen::Vector3f a3)
{
	float Cos_value1 = (v - a1).dot(a2 - a1) / (v - a1).norm() / (a2 - a1).norm();
	float Cos_value2 = (v - a3).dot(a2 - a3) / (v - a3).norm() / (a2 - a3).norm();
	return sqrt(1 / (1 / (Cos_value1 * Cos_value1) - 1)) + sqrt(1 / (1 / (Cos_value2 * Cos_value2) - 1));
}