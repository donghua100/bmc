#include"bmc.h"
#include "utils/logger.h"
#include "trans/ts.h"
#include"utils/exceptions.h"
#include "smt-switch/include/ops.h"
#include "smt-switch/include/result.h"
#include "smt-switch/include/smt_defs.h"
#include <bits/chrono.h>
#include <cstring>
#include <cstdio>
#include <chrono>
using namespace smt;
using namespace std;


Bmc::Bmc(const Property & p, const TransitionSystem & ts,
		const SmtSolver & solver,bool inv, int start_k, int skip)
	:solver_(solver),
	ts_(ts),
	unroller_(ts_),
	bad_(solver_->make_term(smt::PrimOp::Not,
				p.prop())),
	initialized_ (false),
	inv_(inv),
	start_k_(start_k),
	skip_(skip)
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
	for (int i =start_k_; i <= k; i+=skip_) {
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
	logger.log(1, "Checking steps at {}",i);
	// std::cout<<"Checking bmc at bound: "<<i<<std::endl;
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	if (inv_) solver_->assert_formula(unroller_.at_time(ts_.init(),i));
	else
		solver_->assert_formula(unroller_.at_time(bad_, i));
	Result r = solver_->check_sat();
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2-t1);
	logger.log(2, "Checking step {} takes time: {} sec", i , round(time_span.count()));
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

