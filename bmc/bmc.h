#pragma once

#include<smt-switch/smt.h>
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
      const smt::SmtSolver & solver);

  ~Bmc();
  ProverResult check_until(int k);
  void initialize();

 protected:
  bool step(int i);
  smt::SmtSolver solver_;

  TransitionSystem ts_;

  Unroller unroller_;

  int reached_k_;  ///< the last bound reached with no counterexamples

  smt::Term bad_;
  bool initialized_;

};