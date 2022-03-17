#include<smt-switch/smt.h>
#include<smt-switch/z3_factory.h>
#include"frontends/btor2_encoder.h"
#include"trans/ts.h"
#include"trans/unroller.h"
#include"bmc/bmc.h"


using namespace std;
using namespace smt;

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
        string ss = t->to_string();
        cout<<ss<<endl;
        // if (ss.length() < 10 ) cout<<ss<<endl;
        // else cout << "term[0:10] = "<<ss.substr(10)<<endl;
        for (const auto &c:t) to_visit.push_back(c);
    }
}

int main()
{

    string filename = "../tests/encoder/input/btor2/p-counter-false.btor2";
    SmtSolver s = smt::Z3SolverFactory::create(false);
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true");  // get unsat core
    cout<<"start encoder..."<<endl;
    TransitionSystem ts(s);
    BTOR2Encoder be(filename, ts);
    // cout<<ts.trans_->to_string()<<endl;
    print_all_term(ts.trans_);

    cout<<"-------------------\n"<<"property :"<<endl;
    for (const auto & t:be.propvec())
    {
        cout<<t->to_string()<<endl;
    }
    // cout<<"end encoder..."<<endl;
    // cout<<"start encoder.."<<endl;
    // Unroller u(ts);
    // UnorderedTermSet states = ts.statevars();
    // for (const auto &s:states)
    // {
    //     cout <<"state : "<<s->to_string()<<endl;
    //     cout <<"next state :"<<ts.next(s)->to_string()<<endl;
    //     cout <<"state at time 1 :"<<u.at_time(s,1)->to_string()<<endl;
    // }
    // TermVec prop = be.propvec();
    // for (const auto &b : prop)
    // {
    //     cout <<"prop state : "<<b->to_string()<<endl;
    // }
    Property p(s,be.propvec()[0]);
    Bmc bmc(p,ts,s);
    ProverResult r = bmc.check_until(10);
    if (r == FALSE)
    {
        cout<<"find counter-example!!!"<<endl;
    }
    return 0;
}