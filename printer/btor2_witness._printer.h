
#include <iterator>
#include <map>
#include <sstream>
#include <vector>
#include<string>

#include "gmpxx.h"
#include "utils/exceptions.h"

#include "smt-switch/smt.h"
#include "trans/ts.h"

using namespace std;

std::string as_bits(std::string val)
{
  // TODO: this makes assumptions on format of value from boolector
  //       to support other solvers, we need to be more general
  std::string res = val;

  if (val.length() < 2) {
    throw PanguException("Don't know how to interpret value: " + val);
  }

  if (res.substr(0, 2) == "#b") {
    // remove the #b prefix
    res = res.substr(2, val.length() - 2);
  } else if (res.substr(0, 2) == "#x") {
    throw PanguException("Not supporting hexadecimal format yet.");
  } else {
    res = res.substr(5, res.length() - 5);
    std::istringstream iss(res);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{ iss },
                                    std::istream_iterator<std::string>());

    if (tokens.size() != 2) {
      throw PanguException("Failed to interpret " + val);
    }

    res = tokens[0];
    // get rid of ")"
    std::string width_str = tokens[1].substr(0, tokens[1].length() - 1);
    size_t width = std::stoull(width_str);
    mpz_class cval(res);
    res = cval.get_str(2);
    size_t strlen = res.length();

    if (strlen < width) {
      // pad with zeros
      res = std::string(width - strlen, '0') + res;
    } else if (strlen > width) {
      // remove prepended zeros
      res = res.erase(0, strlen - width);
    }
    return res;
  }
  return res;
}

// Returns true iff 'term' appears in either the state variables or in
// the input variables of 'ts'. If COI is applied, then 'ts' might
// have different state and input variables than the original
// transition system that was parsed by the BTOR encoder.
// NOTE: we call this check also when COI is not applied, hence some
// overhead will occur in witness printing.
bool appears_in_ts_coi (const smt::Term &term,
                        const TransitionSystem & ts)
{
  const auto & it_states = ts.statevars().find(term);
  if (it_states != ts.statevars().end())
    return true;

  const auto & it_inputs = ts.inputvars().find(term);
  if (it_inputs != ts.inputvars().end())
    return true;

  return false;
}

void print_btor_vals_at_time(const smt::TermVec & vec,
                             const smt::UnorderedTermMap & valmap,
                             unsigned int time, const TransitionSystem & ts)
{
  smt::SortKind sk;
  smt::TermVec store_children(3);
  for (size_t i = 0, size = vec.size(); i < size; ++i) {
    // Do not print if term 'vec[i]' does not appear in COI. When not
    // using COI, this check always returns true.
    if (!appears_in_ts_coi(vec[i], ts))
      continue;
    sk = vec[i]->get_sort()->get_sort_kind();
    if (sk == smt::BV) {
      // TODO: this makes assumptions on format of value from boolector
      //       to support other solvers, we need to be more general
    //   logger.log(0,
    //              "{} {} {}@{}",
    //              i,
    //              as_bits(valmap.at(vec[i])->to_string()),
    //              vec[i],
    //              time);
	cout<<valmap.at(vec[i])->to_string()<<endl;
      cout<<i<<" "<<as_bits(valmap.at(vec[i])->to_string())<<" "<<vec[i]<<"@"<<time<<endl;
    } else if (sk == smt::ARRAY) {
      smt::Term tmp = valmap.at(vec[i]);
      while (tmp->get_op() == smt::Store) {
        int num = 0;
        for (auto c : tmp) {
          store_children[num] = c;
          num++;
        }

        // logger.log(0,
        //            "{} [{}] {} {}@{}",
        //            i,
        //            as_bits(store_children[1]->to_string()),
        //            as_bits(store_children[2]->to_string()),
        //            vec[i],
        //            time);
        cout<<i<<" "<<"["<<as_bits(store_children[1]->to_string())<<"]"<<" "
         <<as_bits(store_children[2]->to_string())<<" "<<vec[i]<<"@"<<time<<endl;
        tmp = store_children[0];
      }

      if (tmp->get_op().is_null()
          && tmp->get_sort()->get_sort_kind() == smt::ARRAY) {
        smt::Term const_val = *(tmp->begin());
        // logger.log(
        //     0, "{} {} {}@{}", i, as_bits(const_val->to_string()), vec[i], time);
        cout<<i<<" "<<as_bits(const_val->to_string())<<" "<<vec[i]<<"@"<<time<<endl;
      }

    } else {
      throw PanguException("Unhandled sort kind: " + ::smt::to_string(sk));
    }
  }
}

void print_btor_vals_at_time(const std::map<uint64_t, smt::Term> m,
                             const smt::UnorderedTermMap & valmap,
                             unsigned int time, const TransitionSystem & ts)
{
  smt::SortKind sk;
  smt::TermVec store_children(3);
  for (auto entry : m) {
    // Do not print if term 'entry.second' does not appear in COI. When not
    // using COI, this check always returns true.
    if (!appears_in_ts_coi(entry.second, ts))
      continue;
    sk = entry.second->get_sort()->get_sort_kind();
    if (sk == smt::BV) {
      // TODO: this makes assumptions on format of value from boolector
      //       to support other solvers, we need to be more general
      // Remove the #b prefix
    //   logger.log(0,
    //              "{} {} {}@{}",
    //              entry.first,
    //              as_bits(valmap.at(entry.second)->to_string()),
    //              entry.second,
    //              time);
      cout<<entry.first<<" "<<as_bits(valmap.at(entry.second)->to_string())<<" "<<entry.second<<"@"<<time<<endl;
    } else if (sk == smt::ARRAY) {
      smt::Term tmp = valmap.at(entry.second);
      while (tmp->get_op() == smt::Store) {
        int num = 0;
        for (auto c : tmp) {
          store_children[num] = c;
          num++;
        }

        // logger.log(0,
        //            "{} [{}] {} {}@{}",
        //            entry.first,
        //            as_bits(store_children[1]->to_string()),
        //            as_bits(store_children[2]->to_string()),
        //            entry.second,
        //            time);
        cout<<entry.first<<" "<<"["<<as_bits(store_children[1]->to_string())<<"]"<<" "
         <<as_bits(store_children[2]->to_string())<<" "<<entry.second<<"@"<<time<<endl;
        tmp = store_children[0];
      }

      if (tmp->get_op().is_null()
          && tmp->get_sort()->get_sort_kind() == smt::ARRAY) {
        smt::Term const_val = *(tmp->begin());
        // logger.log(0,
        //            "{} {} {}@{}",
        //            entry.first,
        //            as_bits(const_val->to_string()),
        //            entry.second,
        //            time);
        cout<<entry.first<<" "<<as_bits(const_val->to_string())<<" "<<entry.second<<"@"<<time<<endl;
      }

    } else {
      throw PanguException("Unhandled sort kind: " + ::smt::to_string(sk));
    }
  }
}

void print_witness_btor(const BTOR2Encoder & btor_enc,
                        const std::vector<smt::UnorderedTermMap> & cex,
			const TransitionSystem & ts)
{
  const smt::TermVec inputs = btor_enc.inputsvec();
  const smt::TermVec states = btor_enc.statesvec();
  const std::map<uint64_t, smt::Term> no_next_states =
      btor_enc.no_next_statevars();
  bool has_states_without_next = no_next_states.size();
  // logger.log(0, "#0");
  cout<<"#0"<<endl;
  print_btor_vals_at_time(states, cex.at(0), 0, ts);

  for (size_t k = 0, cex_size = cex.size(); k < cex_size; ++k) {
    // states without next
    if (k && has_states_without_next) {
      // logger.log(0, "#{}", k);
      cout<<"#"<<k<<endl;
      print_btor_vals_at_time(no_next_states, cex.at(k), k, ts);
    }

    // inputs
    if (k < cex_size) {
      // logger.log(0, "@{}", k);
      cout<<"@"<<k<<endl;
      print_btor_vals_at_time(inputs, cex.at(k), k, ts);
    }
  }
  // logger.log(0, ".");
  cout<<"."<<endl;
}

