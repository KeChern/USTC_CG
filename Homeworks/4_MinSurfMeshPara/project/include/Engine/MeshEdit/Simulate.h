#pragma once

#include <Basic/HeapObj.h>
//#include <Engine/Primitive/MassSpring.h>
#include <UGM/UGM>
#include <Eigen/Sparse>
namespace Ubpa {
	enum class SimulateType
	{
		ImplicitEuler,
		ProjectiveDynamic,
	};

	class Simulate : public HeapObj {
	public:
		Simulate(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist)
		{
			edgelist = elist;
			this->positions.resize(plist.size());
			for (int i = 0; i < plist.size(); i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->positions[i][j] = plist[i][j];
				}
			}
		};
	public:
		static const Ptr<Simulate> New(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			return Ubpa::New<Simulate>(plist, elist);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init();
		//bool Init();

		// call it after Init()
		bool Run();

		const std::vector<pointf3>& GetPositions() const { return positions; };

		const float GetStiff() { return stiff; };
		void SetStiff(float k) { stiff = k; Init(); };

		const float GetMass() { return Mass; };
		void SetMass(float k) { Mass = k; Init(); };

		const float GetTimeStep() { return h; };
		void SetTimeStep(float k) { h = k; Init(); };

		std::vector<unsigned>& GetFix() { return this->fixed_id; };
		void SetFix(const std::vector<unsigned>& f) { this->fixed_id = f; Init(); };

		const std::vector<pointf3>& GetVelocity() { return velocity; };
		//void SetVelocity(const std::vector<pointf3>& v) { velocity = v; };

		//two methods to fix points
		void SetLeftFix();
		void Set2PointFix();
		bool isfixed(unsigned id);

		// kernel part of the algorithm, two methods
		void SimulateOnce_ImplicitEuler();
		void SimulateOnce_ProjectiveDynamic();
		void SetSimulateType(const SimulateType type) { simulatetype = type; };

	private:
		float h = 0.03f;  //步长
		float stiff = 6000;
		float Mass = 1;//每个质点的质量都为1
		float g = 9.8;
		SimulateType simulatetype = SimulateType::ImplicitEuler;//模拟的方法

		//mesh data
		std::vector<unsigned> edgelist;

		//simulation data
		std::vector<pointf3> positions;
		std::vector<pointf3> velocity;
		std::vector<pointf3> externalF;
		std::vector<float> mass;
		std::vector<float> length;
		std::vector<unsigned> fixed_id;  //fixed point id

		//auxiliary data
		Eigen::SparseMatrix<float> L;
		Eigen::SparseMatrix<float> J;
		Eigen::SparseMatrix<float> K;
		Eigen::SparseMatrix<float> M;
		Eigen::SparseMatrix<float> b;
		Eigen::SimplicialLLT<Eigen::SparseMatrix<float>> solver_acc;

		Eigen::SparseMatrix<float> K_M_h2_L_Kt;
		Eigen::SparseMatrix<float> K_h2_J;
		Eigen::SparseMatrix<float> K_M;
		Eigen::SparseMatrix<float> _K_M_h2_L_b;
	};
}
