#include <Engine/MeshEdit/ARAP.h>
#include <Engine/MeshEdit/ASAP.h>
#include <Engine/Primitive/TriMesh.h>

using namespace Ubpa;
using namespace std;
using namespace Eigen;

ARAP::ARAP(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}
void ARAP::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}
bool ARAP::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::ARAP::Init:\n"
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
		printf("ERROR::ARAP::Init:\n"
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

bool ARAP::ShowARAPPara(int IterNum)
{
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::ASAP::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	compute(IterNum);
	
	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	for (int i = 0; i < nV; i++)
	{
		auto v = heMesh->Vertices()[i];
		v->pos[0] = parapos[i][0];
		v->pos[1] = parapos[i][1];
		v->pos[2] = 0;
	}

	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);
	return true;
}
bool ARAP::SetARAPTexcoords(int IterNum)
{
	
	compute(IterNum);

	size_t nV = heMesh->NumVertices();

	vector<pointf2> texcoords;
	texcoords.reserve(nV);
	for (int i = 0; i < nV; i++)
	{
		texcoords.push_back(parapos[i].cast_to<pointf2>());
	}

	triMesh->Update(texcoords);
	return true;
}

//主要的几个函数
void ARAP::compute(int IterNum)
{
	Init_Paramaterization();

	InitLaplaceMatrix();

	for (int i = 0; i < IterNum; i++)
	{
		Local();
		Global();
	}
}
void ARAP::Init_Paramaterization()
{
	auto asap = ASAP::New(triMesh);
	asap->compute();
	parapos = asap->parapos;
	FlattenedMatrix = asap->FlattenedMatrix;
}
void ARAP::InitLaplaceMatrix()
{
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	vector<Triplet<float>> tripletList;
	tripletList.reserve(nF * 78);
	tripletList.push_back(Triplet<float>(0, 0, 1));//第一个点作为固定点
	tripletList.push_back(Triplet<float>(nV, nV, 1));

	for (size_t kx = 1; kx < nV; kx++)
	{
		auto q = heMesh->Vertices()[kx];
		size_t ky = kx + nV;
		for (auto adjP : q->AdjPolygons())
		{
			if (adjP == nullptr)continue;

			size_t i = find(kx, adjP);
			size_t ipre = (i + 2) % 3;
			size_t inext = (i + 1) % 3;
			size_t t = heMesh->Index(adjP);
			auto Boundary = adjP->BoundaryVertice();
			vector<size_t>v = { heMesh->Index(Boundary[ipre]), heMesh->Index(Boundary[i]), heMesh->Index(Boundary[inext]) };

			tripletList.push_back(Triplet<float>(kx, kx, Cot(t, i)));
			tripletList.push_back(Triplet<float>(kx, v[2], -Cot(t, i)));

			tripletList.push_back(Triplet<float>(kx, v[0], -Cot(t, ipre)));
			tripletList.push_back(Triplet<float>(kx, kx, Cot(t, ipre)));

			tripletList.push_back(Triplet<float>(ky, ky, Cot(t, i)));
			tripletList.push_back(Triplet<float>(ky, v[2] + nV, -Cot(t, i)));

			tripletList.push_back(Triplet<float>(ky, v[0] + nV, -Cot(t, ipre)));
			tripletList.push_back(Triplet<float>(ky, ky, Cot(t, ipre)));
		}
	}
	LaplaceMatrix = Eigen::SparseMatrix<float>(2 * nV, 2 * nV);
	LaplaceMatrix.setFromTriplets(tripletList.begin(), tripletList.end());

	solver.compute(LaplaceMatrix);
}
void ARAP::Local()
{
	size_t nF = heMesh->NumPolygons();
	L.clear();
	L.reserve(nF);

	for (size_t t = 0; t < nF; t++)
	{
		auto Boundary = heMesh->Polygons()[t]->BoundaryVertice();
		vector<size_t>v = { heMesh->Index(Boundary[0]), heMesh->Index(Boundary[1]), heMesh->Index(Boundary[2]) };
		Matrix2f St = Matrix2f::Zero();
		for (size_t i = 0; i < 3; i++)
		{
			size_t inext = (i + 1) % 3;
			St(0, 0) = St(0, 0) + Cot(t, i) * Delta_u(v[i], v[inext], 0) * Delta_x(t, i, 0);
			St(0, 1) = St(0, 1) + Cot(t, i) * Delta_u(v[i], v[inext], 0) * Delta_x(t, i, 1);
			St(1, 0) = St(1, 0) + Cot(t, i) * Delta_u(v[i], v[inext], 1) * Delta_x(t, i, 0);
			St(1, 1) = St(1, 1) + Cot(t, i) * Delta_u(v[i], v[inext], 1) * Delta_x(t, i, 1);
		}
		JacobiSVD<MatrixXf> svd(St, ComputeThinU | ComputeThinV);
		auto Lt = svd.matrixU() * svd.matrixV().transpose();
		vecf2 w;
		w[0] = Lt(0, 0);
		w[1] = Lt(0, 1);
		L.push_back(w);
	}
}
void ARAP::Global()
{
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	MatrixXf BoundaryMatrix = MatrixXf::Zero(2 * nV, 1);

	for (size_t kx = 1; kx < nV; kx++)
	{
		auto q = heMesh->Vertices()[kx];
		size_t ky = kx + nV;
		for (auto adjP : q->AdjPolygons())
		{
			if (adjP == nullptr)continue;

			size_t i = find(kx, adjP);
			size_t ipre = (i + 2) % 3;
			size_t inext = (i + 1) % 3;
			size_t t = heMesh->Index(adjP);
			auto Boundary = adjP->BoundaryVertice();
			vector<size_t>v = { heMesh->Index(Boundary[ipre]), heMesh->Index(Boundary[i]), heMesh->Index(Boundary[inext]) };

			BoundaryMatrix(kx, 0) = BoundaryMatrix(kx, 0) + Cot(t, i) * Delta_x(t, i, 0) * L[t][0];
			BoundaryMatrix(kx, 0) = BoundaryMatrix(kx, 0) + Cot(t, i) * Delta_x(t, i, 1) * L[t][1];

			BoundaryMatrix(kx, 0) = BoundaryMatrix(kx, 0) - Cot(t, ipre) * Delta_x(t, ipre, 0) * L[t][0];
			BoundaryMatrix(kx, 0) = BoundaryMatrix(kx, 0) - Cot(t, ipre) * Delta_x(t, ipre, 1) * L[t][1];

			BoundaryMatrix(ky, 0) = BoundaryMatrix(ky, 0) - Cot(t, i) * Delta_x(t, i, 0) * L[t][1];
			BoundaryMatrix(ky, 0) = BoundaryMatrix(ky, 0) + Cot(t, i) * Delta_x(t, i, 1) * L[t][0];

			BoundaryMatrix(ky, 0) = BoundaryMatrix(ky, 0) + Cot(t, ipre) * Delta_x(t, ipre, 0) * L[t][1];
			BoundaryMatrix(ky, 0) = BoundaryMatrix(ky, 0) - Cot(t, ipre) * Delta_x(t, ipre, 1) * L[t][0];
		}
	}

	MatrixXf result = solver.solve(BoundaryMatrix);

	parapos.clear();
	parapos.reserve(nV);
	for (size_t i = 0; i < nV; i++)
	{
		vecf2 w;
		w[0] = result(i, 0);
		w[1] = result(i + nV, 0);
		parapos.push_back(w);
	}
}

//辅助函数
float ARAP::Cot(size_t t, size_t i)
{
	return FlattenedMatrix(3 * t + i % 3, 2);
}
float ARAP::Delta_x(size_t t, size_t i, size_t k)
{
	return FlattenedMatrix(3 * t + i % 3, k) - FlattenedMatrix(3 * t + (i + 1) % 3, k);
}
float ARAP::Delta_u(size_t i, size_t i_next, size_t k)
{
	return parapos[i][k] - parapos[i_next][k];
}
size_t ARAP::find(size_t i, P* adjP)
{
	auto Boundary = adjP->BoundaryVertice();
	if (heMesh->Index(Boundary[0]) == i)
	{
		return 0;
	}
	else if (heMesh->Index(Boundary[1]) == i)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}





