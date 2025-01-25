#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace Ubpa {
	class TriMesh;

	class ARAP : public HeapObj {
	public:
		ARAP(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<ARAP> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<ARAP>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		bool ShowARAPPara(int IterNum);

		bool SetARAPTexcoords(int IterNum);

	private:
		void Local();
		void Global();
		void compute(int IterNum);
		void InitLaplaceMatrix();
		void Init_Paramaterization();
		float Delta_x(size_t t, size_t i, size_t k);
		float Delta_u(size_t i, size_t i_next, size_t k);
		float Cot(size_t t, size_t i);

	private:
		Eigen::SparseMatrix<float> LaplaceMatrix;
		Eigen::MatrixXf FlattenedMatrix;
		Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
		std::vector<vecf2>parapos;
		std::vector<vecf2>L;

	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };
	private:

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		size_t find(size_t i, P* adjp);
	};
}
