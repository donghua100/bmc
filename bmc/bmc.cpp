#include"bmc.h"
#include"utils/exceptions.h"

using namespace smt;


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

  if (!ts_.only_curr(bad_)) {
    throw PanguException("Property should not contain inputs or next state variables");
  }

  initialized_ = true;
  if (inv_) solver_->assert_formula(unroller_.at_time(bad_,0));
  else
  solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
}

ProverResult Bmc::check_until(int k)
{
  initialize();

  for (int i = reached_k_ + 1; i <= k; ++i) {
    if (!step(i)) {
      compute_witness();
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

