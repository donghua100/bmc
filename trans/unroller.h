#pragma once

#include "smt-switch/include/smt.h"
#include "ts.h"




class Unroller
{
 public:
  Unroller(const TransitionSystem & ts,
           const std::string &time_identifier = "@");

  virtual ~Unroller();

  virtual smt::Term at_time(const smt::Term & t, unsigned int k);
  smt::Term untime(const smt::Term & t) const;
  size_t get_var_time(const smt::Term & v) const;

  size_t get_curr_time(const smt::Term & t) const;

 protected:
  smt::Term var_at_time(const smt::Term & v, unsigned int k);
  virtual smt::UnorderedTermMap & var_cache_at_time(unsigned int k);

  const TransitionSystem & ts_;
  const smt::SmtSolver solver_;
  const std::string time_id_;

  typedef std::vector<smt::UnorderedTermMap> TimeCache;
  TimeCache time_cache_;
  TimeCache time_var_map_;
  smt::UnorderedTermMap untime_cache_;
  std::unordered_map<smt::Term, size_t> var_times_;

  size_t num_vars_;  

};
