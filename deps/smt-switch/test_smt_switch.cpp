#include<iostream>
#include<string>
#include<cassert>
#include<smt-switch/smt.h>
#include<smt-switch/z3_factory.h>

using namespace std;
using namespace smt;

class Property
{   SmtSolver solver_;
    Term prop_;
    string name_;
    public:
      Property(const SmtSolver &s,const Term &p,string name = ""):solver_(s),prop_(p),name_(name){}
      virtual ~Property(){}
      const Term & prop() const {return prop_;}
      const SmtSolver & solver() const {return solver_;}
      string name(){return name_;}    
};



void conuter(SmtSolver &s,const Term &max_val)
{
    assert(max_val);
    Sort sort = max_val->get_sort();
    Term x = s->make_symbol("x", sort);
    SortKind sk = sort->get_sort_kind();
    PrimOp plus_op = (sk == BV) ? BVAdd : Plus;
    PrimOp lt_op = (sk == BV) ? BVUlt : Lt;
    Term inc_term = s->make_term(plus_op, x, s->make_term(1, sort));
    Term zero = s->make_term(0, sort);

}


void print_all_term(Term term)
{
    TermVec to_visit{term};
    Term t;
    while(to_visit.size())
    {
        t = to_visit.back();
        to_visit.pop_back();
        if (t->is_symbolic_const())
        {
            cout<<"this is a symbolic_const: ";
        }
        cout<<t->to_string()<<endl;
        for (const auto &c:t) to_visit.push_back(c);
    }
}




int main()
{
    smt::SmtSolver s = smt::Z3SolverFactory::create(false);
    s->set_logic("QF_UFBV");                // Quantifier-Free Uninterpreted Functions and BitVector
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true");  // get unsat core
    // Term trans = s->make_term(true);

    // Sort bvs = s->make_sort(BV, 32);
    // Sort funs =
    // s->make_sort(FUNCTION, {bvs, bvs});

    // Term x = s->make_symbol("xxxxxxxxxxxxx", bvs);
    // Term y = s->make_symbol("y", bvs);
    // Term f = s->make_symbol("f", funs);

    // Op ext = Op(Extract, 15, 0);
    // Term x0 = s->make_term(ext, x);
    // Term y0 = s->make_term(ext, y);

    // Term fx = s->make_term(Apply, f, x);
    // Term fy = s->make_term(Apply, f, y);

    // cout<<fx->to_string()<<"---------"<<fx->get_sort()<<endl;
    // if (fx->is_symbolic_const())
    // {
    //     cout<<fx->to_string()<<" is a symbolic_const"<<endl;
    // }
    // if (fx->is_symbol())
    // {
    //     cout<<fx->to_string()<<" is a symbolic"<<endl;
    // }
    // for (const auto & t:fx)
    // {
    //     if (t->is_symbolic_const())
    //     {
    //         cout<<t->to_string()<<" is a symbolic_const"<<endl;
    //     }
    //     if (t->is_symbol())
    //     {
    //         cout<<t->to_string()<<" is a symbolic"<<endl;
    //     }
    // }
    Sort bvsort8 = s->make_sort(BV,8);
    Term max_val = s->make_term(7,bvsort8);
    Sort sort = max_val->get_sort();
    Term x = s->make_symbol("x", sort);
    SortKind sk = sort->get_sort_kind();
    PrimOp plus_op = (sk == BV) ? BVAdd : Plus;
    PrimOp lt_op = (sk == BV) ? BVUlt : Lt;
    Term inc_term = s->make_term(plus_op, x, s->make_term(1, sort));
    Term zero = s->make_term(0, sort);
    Term next_state = s->make_term(Ite, s->make_term(lt_op, x, max_val), inc_term, zero);


    for (const auto & c : next_state) 
    {
        cout<<c->to_string()<<"---------"<<c->get_sort()<<endl;
        if (c->is_symbolic_const())
        {
            cout<<"is a symbolic_const"<<endl;
        }
        if (c->is_symbol())
        {
            cout<<"is a symbolic"<<endl;
        }
    }

    cout<<"------------------------------------"<<endl;
    print_all_term(next_state);




    // Term trans = s->make_term(true);
    // trans = s->make_term(And,trans,fx);
    // trans = s->make_term(And,trans,fy);

    // for (const auto & c : trans) 
    // {
    //     cout<<c->to_string()<<"---------"<<c->get_sort()<<endl;
    //     if (c->is_symbolic_const())
    //     {
    //         cout<<"is a symbolic_const"<<endl;
    //     }
    //     if (c->is_symbol())
    //     {
    //         cout<<"is a symbolic"<<endl;
    //     }
    // }



    // s->assert_formula(
    // s->make_term(Distinct, fx, fy));

    // s->push(1);
    // s->assert_formula(
    // s->make_term(Equal, x0, y0));
    // cout <<  s->check_sat() << endl;

    // cout << s->get_value(x) << endl;
    // s->pop(1);

    // Term xy = s->make_term(BVAnd, x, y);
    // Term a1 = s->make_term(BVUge, x0, y0);
    // Term a2 = s->make_term(BVUge, xy, x);
    // Term a3 = s->make_term(BVUge, xy, y);
    // cout <<
    // s->check_sat_assuming({a1, a2, a3})
    // << endl;
    // UnorderedTermSet ua;
    // s->get_unsat_assumptions(ua);
    // for (Term t : ua) { cout << t << endl; }
    return 0;
}