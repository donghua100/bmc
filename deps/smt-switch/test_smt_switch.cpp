#include<iostream>
#include<smt-switch/smt.h>
#include<smt-switch/z3_factory.h>

using namespace std;
using namespace smt;

int main()
{
    smt::SmtSolver s = smt::Z3SolverFactory::create(false);
    s->set_logic("QF_UFBV");                // Quantifier-Free Uninterpreted Functions and BitVector
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true");  // get unsat core
    // Term trans = s->make_term(true);

    Sort bvs = s->make_sort(BV, 32);
    Sort funs =
    s->make_sort(FUNCTION, {bvs, bvs});

    Term x = s->make_symbol("x", bvs);
    Term y = s->make_symbol("y", bvs);
    Term f = s->make_symbol("f", funs);

    Op ext = Op(Extract, 15, 0);
    Term x0 = s->make_term(ext, x);
    Term y0 = s->make_term(ext, y);

    Term fx = s->make_term(Apply, f, x);
    Term fy = s->make_term(Apply, f, y);
    s->assert_formula(
    s->make_term(Distinct, fx, fy));

    s->push(1);
    s->assert_formula(
    s->make_term(Equal, x0, y0));
    cout <<  s->check_sat() << endl;

    cout << s->get_value(x) << endl;
    s->pop(1);

    Term xy = s->make_term(BVAnd, x, y);
    Term a1 = s->make_term(BVUge, x0, y0);
    Term a2 = s->make_term(BVUge, xy, x);
    Term a3 = s->make_term(BVUge, xy, y);
    cout <<
    s->check_sat_assuming({a1, a2, a3})
    << endl;
    UnorderedTermSet ua;
    s->get_unsat_assumptions(ua);
    for (Term t : ua) { cout << t << endl; }
    return 0;
}