#include<assert.h>
#include"ts.h"
#include"utils/exceptions.h"

using namespace smt;
using namespace std;

void TransitionSystem::name_term(const std::string name, const smt::Term & t)
{
  auto it = named_terms_.find(name);
  if (it != named_terms_.end() && t != it->second) {
    throw PanguException("Name " + name + " has already been used.");
  }
  named_terms_[name] = t;
  term_to_name_[t] = name;
}

void TransitionSystem::add_statevar(const Term & cv, const Term & nv)
{
  if (statevars_.find(cv) != statevars_.end()) {
    throw PanguException("Cannot redeclare a state variable");
  }

  if (next_statevars_.find(nv) != next_statevars_.end()) {
    throw PanguException("Cannot redeclare a state variable");
  }

  if (next_statevars_.find(cv) != next_statevars_.end()) {
    throw PanguException(
        "Cannot use an existing next state variable as a current state var");
  }

  if (statevars_.find(nv) != statevars_.end()) {
    throw PanguException(
        "Cannot use an existing state variable as a next state var");
  }

  if (inputvars_.find(cv) != inputvars_.end()) {
    bool success = inputvars_.erase(cv);
    assert(success);
  }

  if (inputvars_.find(nv) != inputvars_.end()) {
    bool success = inputvars_.erase(nv);
    assert(success);
  }

  statevars_.insert(cv);
  next_statevars_.insert(nv);
  next_map_[cv] = nv;
  curr_map_[nv] = cv;
  name_term(cv->to_string(), cv);
  name_term(nv->to_string(), nv);
}

void TransitionSystem::add_inputvar(const Term & v)
{

  if (statevars_.find(v) != statevars_.end()
      || next_statevars_.find(v) != next_statevars_.end()
      || inputvars_.find(v) != inputvars_.end()) {
    throw PanguException(
        "Cannot reuse an existing variable as an input variable");
  }

  inputvars_.insert(v);
  name_term(v->to_string(), v);
}


Term TransitionSystem::make_inputvar(const string name, const Sort & sort)
{
  Term input = solver_->make_symbol(name, sort);
  add_inputvar(input);
  return input;
}

Term TransitionSystem::make_statevar(const string name, const Sort & sort)
{
  Term state = solver_->make_symbol(name, sort);
  Term next_state = solver_->make_symbol(name + ".next", sort);
  add_statevar(state, next_state);
  return state;
}


void TransitionSystem::assign_next(const Term & state, const Term & val)
{
  // TODO: only do this check in debug mode
  if (statevars_.find(state) == statevars_.end()) {
    throw PanguException("Unknown state variable");
  }

  if (!no_next(val)) {
    throw PanguException(
        "Got a symbolic that is not a current state or input variable in RHS "
        "of functional assignment");
  }

  if (state_updates_.find(state) != state_updates_.end()) {
    throw PanguException("State variable " + state->to_string()
                        + " already has next-state logic assigned.");
  }

  state_updates_[state] = val;
  trans_ = solver_->make_term(
      And, trans_, solver_->make_term(Equal, next_map_.at(state), val));
}

bool TransitionSystem::contains(const Term & term,
                                UnorderedTermSetPtrVec term_sets) const
{
  UnorderedTermSet visited;
  TermVec to_visit{ term };
  Term t;
  while (to_visit.size()) {
    t = to_visit.back();
    to_visit.pop_back();

    if (visited.find(t) != visited.end()) {
      // cache hit
      continue;
    }

    if (t->is_symbolic_const()) {
      bool in_atleast_one = false;
      for (const auto & ts : term_sets) {
        if (ts->find(t) != ts->end()) {
          in_atleast_one = true;
          break;
        }
      }

      if (!in_atleast_one) {
        return false;
      }
    }

    visited.insert(t);
    for (const auto & c : t) {
      to_visit.push_back(c);
    }
  }

  return true;
}

bool TransitionSystem::only_curr(const Term & term) const
{
  return contains(term, UnorderedTermSetPtrVec{ &statevars_ });
}

bool TransitionSystem::no_next(const Term & term) const
{
  return contains(term, UnorderedTermSetPtrVec{ &statevars_, &inputvars_ });
}

bool TransitionSystem::is_input_var(const Term & sv) const
{
  return (inputvars_.find(sv) != inputvars_.end());
}

void TransitionSystem::promote_inputvar(const Term & iv)
{
  size_t num_erased = inputvars_.erase(iv);
  if (!num_erased) {
    throw PanguException("Tried to promote non-input to state variable: "
                        + iv->to_string());
  }
  Term next_state_var =
      solver_->make_symbol(iv->to_string() + ".next", iv->get_sort());
  add_statevar(iv, next_state_var);
}

void TransitionSystem::constrain_init(const Term & constraint)
{
  // TODO: Only do this check in debug mode
  if (!only_curr(constraint)) {
    throw PanguException(
        "Initial state constraints should only use current state variables");
  }
  init_ = solver_->make_term(And, init_, constraint);
}

void TransitionSystem::add_constraint(const Term & constraint,
                                      bool to_init_and_next)
{

  if (only_curr(constraint)) {
    trans_ = solver_->make_term(And, trans_, constraint);

    if (to_init_and_next) {
      init_ = solver_->make_term(And, init_, constraint);
      Term next_constraint = solver_->substitute(constraint, next_map_);
      trans_ = solver_->make_term(And, trans_, next_constraint);
    }
    constraints_.push_back({ constraint, to_init_and_next });
  } else if (no_next(constraint)) {
    trans_ = solver_->make_term(And, trans_, constraint);
    constraints_.push_back({ constraint, to_init_and_next });
  } else {
    throw PanguException("Constraint cannot have next states");
  }
}