#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace Ubpa {
	class TriMesh;

	class ASAP : public HeapObj {
	public:
		ASAP(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<ASAP> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<ASAP>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool ShowASAPPara();

		bool SetASAPTexcoords();

		std::vector<vecf2>parapos;
	
		void compute();
	private:
		void Set_LaplaceMatrix();
		void Set_BoundaryMatrix();
		void Flatten();
		float Delta(size_t t, size_t i, size_t k);
		float Cot(size_t t, size_t i);
		
	public:
		Eigen::SparseMatrix<float> LaplaceMatrix;
		Eigen::MatrixXf BoundaryMatrix;
		Eigen::MatrixXf FlattenedMatrix;
		
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
