#include"bmc.h"
#include "trans/ts.h"
#include"utils/exceptions.h"
#include <cstdio>
#include <smt-switch/ops.h>
#include <smt-switch/result.h>
#include <smt-switch/smt_defs.h>
#include <cstring>
using namespace smt;
using namespace std;


Bmc::Bmc(const Property & p, const TransitionSystem & ts,
         const SmtSolver & solver,bool inv)
         :solver_(solver),
         ts_(ts),
         unroller_(ts_),
         bad_(solver_->make_term(smt::PrimOp::Not,
                p.prop())),
         initialized_ (false),
         inv_(inv)
{
}

Bmc::~Bmc() {}

void Bmc::initialize()
{
  if (initialized_) {
    return;
  }
  reached_k_ = -1;
  initialized_ = true;
}


ProverResult Bmc::check_until(int k)
{
  initialize();
	if (!step_0()){
		compute_witness();
		return ProverResult::FALSE;
	}
	reached_k_++;
  for (int i =1; i <= k; i++) {
		if (!step(i)){
			compute_witness();
			return ProverResult::FALSE;
		}
	}
	return ProverResult::UNKNOWN;

}

bool Bmc::step_0(){	
	cur_max_t = 0;
  if (inv_){ 
		solver_->assert_formula(unroller_.at_time(bad_,0));
		solver_->push();
		solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
		Result r = solver_->check_sat();
		if (r.is_sat()) return false;
		else {
			solver_->pop();
			return true;
		}
	}
  else{
		solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
		solver_->push();
		solver_->assert_formula(unroller_.at_time(bad_, 0));
		Result r = solver_->check_sat();
		if (r.is_sat()) return false;
		else {
			solver_->pop();
			return true;
		}
	}
	return true;
}

bool Bmc::step(int i)
{
  if (i <= reached_k_) {
    return true;
  }
  bool res = true;
	if (cur_max_t<i){
		for (int j = cur_max_t+1;j < i; j++) {
			solver_->assert_formula(unroller_.at_time(ts_.trans(),j-1));
			reached_k_++;
		}
	}
  cur_max_t = i;
  solver_->assert_formula(unroller_.at_time(ts_.trans(), i - 1));
  solver_->push();
  std::cout<<"Checking bmc at bound: "<<i<<std::endl;
  if (inv_) solver_->assert_formula(unroller_.at_time(ts_.init(),i));
  else
  solver_->assert_formula(unroller_.at_time(bad_, i));
  Result r = solver_->check_sat();
  if (r.is_sat()) {
    res = false;
  } else {
    solver_->pop();
    ++reached_k_;
  }

  return res;
}

bool Bmc::compute_witness()
{
  // TODO: make sure the solver state is SAT

  for (int i = 0; i <= reached_k_ + 1; ++i) {
    witness_.push_back(UnorderedTermMap());
    UnorderedTermMap & map = witness_.back();

    for (const auto &v : ts_.statevars()) {
      const Term &vi = unroller_.at_time(v, i);
      const Term &r = solver_->get_value(vi);
      map[v] = r;
    }

    for (const auto &v : ts_.inputvars()) {
      const Term &vi = unroller_.at_time(v, i);
      const Term &r = solver_->get_value(vi);
      map[v] = r;
    }

    for (const auto &elem : ts_.named_terms()) {
      const Term &ti = unroller_.at_time(elem.second, i);
      map[elem.second] = solver_->get_value(ti);
    }
  }

  return true;
}

