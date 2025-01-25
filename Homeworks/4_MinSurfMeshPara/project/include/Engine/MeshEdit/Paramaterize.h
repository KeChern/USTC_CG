#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	// mesh boundary == 1
	class Paramaterize : public HeapObj {
	public:
		Paramaterize(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Paramaterize>(triMesh);
		}
	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);

		bool Run(size_t n);
		bool SetTexcoords(size_t n);

	private:
		void paramaterize(size_t n);

		void Set_LaplaceMatrix_Uniform();
		void Set_LaplaceMatrix_Cotangent();

		void Set_BoundaryMatrix_Circle();
		void Set_BoundaryMatrix_Square();
		
		float Cotan(Eigen::Vector3f v, Eigen::Vector3f a1, Eigen::Vector3f a2, Eigen::Vector3f a3);

	private:
		Eigen::SparseMatrix<float> LaplaceMatrix;
		Eigen::MatrixXf BoundaryMatrix;

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
		friend class Paramaterize;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh;

		std::vector<float> CotValue(V* v);
	};
}