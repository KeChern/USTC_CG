#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
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
		printf("ERROR::MinSurf::Init:\n"
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

bool MinSurf::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Minimize();

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
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

void MinSurf::Minimize()
{
	Init_LaplaceMatrix();

	compute();
	
	cout << "WARNING::MinSurf::Minimize:" << endl
		<< "\t" << "Success!" << endl;
}

void MinSurf::Init_LaplaceMatrix()
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
	LaplaceMatrix = Eigen::SparseMatrix<float>(nV, nV);
	LaplaceMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
}
void MinSurf::compute()
{
	size_t nV = heMesh->NumVertices();
	SparseLU<SparseMatrix<float>> solver;
	solver.compute(LaplaceMatrix);
	MatrixXf b = MatrixXf::Zero(nV, 3);
	for (auto v : heMesh->Vertices())
	{
		size_t id = heMesh->Index(v);
		if (v->IsBoundary())
		{
			for (size_t i = 0; i < 3; i++)
			{
				b(id, i) = v->pos[i];
			}
		}
	}
	MatrixXf new_pos = solver.solve(b);
	for (auto v : heMesh->Vertices())
	{
		size_t id = heMesh->Index(v);
		for (size_t i = 0; i < 3; i++)
		{
			v->pos[i] = new_pos(id, i);
		}
	}
}
