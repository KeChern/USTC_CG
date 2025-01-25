#include <Engine/MeshEdit/ASAP.h>

#include <Engine/Primitive/TriMesh.h>

using namespace Ubpa;
using namespace std;
using namespace Eigen;

ASAP::ASAP(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}
void ASAP::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}
bool ASAP::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::ASAP::Init:\n"
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
		printf("ERROR::ASAP::Init:\n"
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

bool ASAP::ShowASAPPara() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::ASAP::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	compute();

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
bool ASAP::SetASAPTexcoords()
{
	compute();

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

void ASAP::compute()
{
	Flatten();

	Set_LaplaceMatrix();

	Set_BoundaryMatrix();

	SparseLU<SparseMatrix<float>> solver;
	solver.compute(LaplaceMatrix);
	MatrixXf result = solver.solve(BoundaryMatrix);

	size_t nV = heMesh->NumVertices();
	parapos.reserve(nV);
	for (int i = 0; i < nV; i++)
	{
		vecf2 v1;
		v1[0] = result(i, 0);
		v1[1] = result(i + nV, 0);
		parapos.push_back(v1);
	}
}
void ASAP::Set_LaplaceMatrix()
{
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	auto MeshBoundary = heMesh->Boundaries()[0];
	auto anchor1 = MeshBoundary[0]->Origin();
	auto anchor2 = MeshBoundary[MeshBoundary.size() / 2]->Origin();//两个锚点
	size_t id1 = heMesh->Index(anchor1);
	size_t id2 = heMesh->Index(anchor2);

	vector<Triplet<float>> tripletList;
	tripletList.reserve(nF * 78);

	for (size_t t = 0; t < nF; t++)
	{
		auto Boundary = heMesh->Polygons()[t]->BoundaryVertice();
		vector<size_t>v = { heMesh->Index(Boundary[0]), heMesh->Index(Boundary[1]), heMesh->Index(Boundary[2]) };
		size_t a_t = 2 * nV + t;
		size_t b_t = 2 * nV + nF + t;

		for (size_t i = 0; i < 3; i++)
		{
			size_t n = (i + 1) % 3;
			float c = Cot(t, i);
			float d0 = Delta(t, i, 0);
			float d1 = Delta(t, i, 1);
			tripletList.push_back(Triplet<float>(a_t, a_t, c * d0 * d0 + c * d1 * d1));

			tripletList.push_back(Triplet<float>(a_t, v[i], -c * d0));
			tripletList.push_back(Triplet<float>(a_t, v[n], c * d0));
			tripletList.push_back(Triplet<float>(a_t, v[i] + nV, -c * d1));
			tripletList.push_back(Triplet<float>(a_t, v[n] + nV, c * d1));

			tripletList.push_back(Triplet<float>(b_t, b_t, c * d0 * d0 + c * d1 * d1));

			tripletList.push_back(Triplet<float>(b_t, v[i], -c * d1));
			tripletList.push_back(Triplet<float>(b_t, v[n], c * d1));
			tripletList.push_back(Triplet<float>(b_t, v[i] + nV, c * d0));
			tripletList.push_back(Triplet<float>(b_t, v[n] + nV, -c * d0));
		}
	}
	for (size_t k = 0; k < nV; k++)
	{
		size_t k1 = k + nV;
		if (k == id1 || k == id2)
		{
			tripletList.push_back(Triplet<float>(k, k, 1));
			tripletList.push_back(Triplet<float>(k1, k1, 1));
		}
		else
		{
			auto q = heMesh->Vertices()[k];
			for (auto adjP : q->AdjPolygons())
			{
				if (adjP == nullptr)continue;

				size_t i = find(k, adjP);
				size_t ipre = (i + 2) % 3;
				size_t inext = (i + 1) % 3;
				size_t t = heMesh->Index(adjP);
				auto Boundary = adjP->BoundaryVertice();
				vector<size_t>v = { heMesh->Index(Boundary[ipre]), heMesh->Index(Boundary[i]), heMesh->Index(Boundary[inext]) };
				size_t a_t = 2 * nV + t;
				size_t b_t = 2 * nV + nF + t;

				tripletList.push_back(Triplet<float>(k, k, Cot(t, i)));
				tripletList.push_back(Triplet<float>(k, v[2], -Cot(t, i)));
				tripletList.push_back(Triplet<float>(k, a_t, -Cot(t, i) * Delta(t, i, 0)));
				tripletList.push_back(Triplet<float>(k, b_t, -Cot(t, i) * Delta(t, i, 1)));

				tripletList.push_back(Triplet<float>(k, v[0], -Cot(t, ipre)));
				tripletList.push_back(Triplet<float>(k, k, Cot(t, ipre)));
				tripletList.push_back(Triplet<float>(k, a_t, Cot(t, ipre) * Delta(t, ipre, 0)));
				tripletList.push_back(Triplet<float>(k, b_t, Cot(t, ipre) * Delta(t, ipre, 1)));

				tripletList.push_back(Triplet<float>(k1, k1, Cot(t, i)));
				tripletList.push_back(Triplet<float>(k1, v[2] + nV, -Cot(t, i)));
				tripletList.push_back(Triplet<float>(k1, b_t, Cot(t, i) * Delta(t, i, 0)));
				tripletList.push_back(Triplet<float>(k1, a_t, -Cot(t, i) * Delta(t, i, 1)));

				tripletList.push_back(Triplet<float>(k1, v[0] + nV, -Cot(t, ipre)));
				tripletList.push_back(Triplet<float>(k1, k1, Cot(t, ipre)));
				tripletList.push_back(Triplet<float>(k1, b_t, -Cot(t, ipre) * Delta(t, ipre, 0)));
				tripletList.push_back(Triplet<float>(k1, a_t, Cot(t, ipre) * Delta(t, ipre, 1)));
			}
		}
	}
	LaplaceMatrix = Eigen::SparseMatrix<float>(2 * (nV + nF), 2 * (nV + nF));
	LaplaceMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
}
void ASAP::Set_BoundaryMatrix()
{
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();

	BoundaryMatrix = MatrixXf::Zero(2 * (nV + nF), 1);
	auto boundary = heMesh->Boundaries()[0];

	auto anchor2 = boundary[boundary.size() / 2]->Origin();//两个锚点
	size_t id2 = heMesh->Index(anchor2);

	BoundaryMatrix(id2, 0) = 1;
	BoundaryMatrix(id2 + nV, 0) = 1;
}


void ASAP::Flatten()
{
	size_t nF = heMesh->NumPolygons();
	
	FlattenedMatrix = MatrixXf::Zero(3 * nF, 3);
	for (size_t i = 0; i < nF; i++)
	{
		auto Boundary = heMesh->Polygons()[i]->BoundaryVertice();
		auto v0 = Boundary[0]->pos;
		auto v1 = Boundary[1]->pos;
		auto v2 = Boundary[2]->pos;
		//被压平后的二维坐标
		FlattenedMatrix(3 * i, 0) = 0;
		FlattenedMatrix(3 * i, 1) = 0;
		FlattenedMatrix(3 * i + 1, 0) = (v1 - v0).norm();
		FlattenedMatrix(3 * i + 1, 1) = 0;
		FlattenedMatrix(3 * i + 2, 0) = (v2 - v0).norm() * (v2 - v0).cos_theta(v1 - v0);
		FlattenedMatrix(3 * i + 2, 1) = (v2 - v0).norm() * (v2 - v0).sin_theta(v1 - v0);
		//每个角的cot值
		FlattenedMatrix(3 * i, 2) = (v0 - v2).cos_theta(v1 - v2) / (v0 - v2).sin_theta(v1 - v2);
		FlattenedMatrix(3 * i + 1, 2) = (v2 - v0).cos_theta(v1 - v0) / (v2 - v0).sin_theta(v1 - v0);
		FlattenedMatrix(3 * i + 2, 2) = (v0 - v1).cos_theta(v2 - v1) / (v0 - v1).sin_theta(v2 - v1);
	}
}
float ASAP::Delta(size_t t, size_t i, size_t k)
{
	return FlattenedMatrix(3 * t + i % 3, k) - FlattenedMatrix(3 * t + (i + 1) % 3, k);
}
float ASAP::Cot(size_t t, size_t i)
{
	return FlattenedMatrix(3 * t + i % 3, 2);
}
size_t ASAP::find(size_t i, P* adjP)
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