#include"bmc.h"
#include"utils/exceptions.h"

using namespace smt;


Bmc::Bmc(const Property & p, const TransitionSystem & ts,
         const SmtSolver & solver)
         :solver_(solver),
         ts_(ts),
         unroller_(ts_),
         bad_(solver_->make_term(smt::PrimOp::Not,
                p.prop())),
         initialized_ (false)
{
}

Bmc::~Bmc() {}

void Bmc::initialize()
{
  if (initialized_) {
    return;
  }

  reached_k_ = -1;

  if (!ts_.only_curr(bad_)) {
    throw PanguException("Property should not contain inputs or next state variables");
  }

  initialized_ = true;

  solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
}

ProverResult Bmc::check_until(int k)
{
  initialize();

  for (int i = reached_k_ + 1; i <= k; ++i) {
    if (!step(i)) {
      // compute_witness();
      return ProverResult::FALSE;
    }
  }
  return ProverResult::UNKNOWN;
}

bool Bmc::step(int i)
{
  if (i <= reached_k_) {
    return true;
  }

  bool res = true;
  if (i > 0) {
    solver_->assert_formula(unroller_.at_time(ts_.trans(), i - 1));
  }

  solver_->push();
  std::cout<< "Checking bmc at bound: "<<i<<std::endl;
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

