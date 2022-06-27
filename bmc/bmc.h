#pragma once

#include<smt-switch/smt.h>
#include <unordered_map>
#include"trans/ts.h"
#include"trans/unroller.h"

typedef enum
{
	UNKNOWN = -1,
	FALSE = 0,
	TRUE = 1,
	ERROR = 2
} ProverResult;

class Property
{
	public:
		Property(const smt::SmtSolver & s, const smt::Term & p,
				std::string name="")
			: solver_(s), prop_(p), name_(name)
			{};

		~Property() {};

		const smt::Term & prop() const { return prop_; }

		const smt::SmtSolver & solver() const { return solver_; }

		std::string name() { return name_; };

	private:
		smt::SmtSolver solver_;

		smt::Term prop_;

		std::string name_; 
};  

class Bmc{
	public:
		Bmc(const Property & p, const TransitionSystem & ts,
				const smt::SmtSolver & solver,bool inv = false);

		~Bmc();
		ProverResult check_until(int k);
		void set_inv(){inv_ = true;}
		void initialize();
		std::vector<smt::UnorderedTermMap> witness_;
		const std::vector<smt::UnorderedTermMap> witness(){return witness_;}
	protected:
		bool step_0();
		bool step(int i);
		bool compute_witness();
		void task(int idx,int k);
		smt::SmtSolver solver_;

		TransitionSystem ts_;

		Unroller unroller_;

		int reached_k_;  ///< the last bound reached with no counterexamples  
		smt::Term bad_;
		bool initialized_;
		bool inv_;
		int cur_max_t;
};
