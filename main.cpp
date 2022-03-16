#include<smt-switch/smt.h>
#include<smt-switch/z3_factory.h>
#include"trans/ts.h"
#include"frontends/btor2_encoder.h"


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

    string filename = "../tests/encoder/input/btor2/p-mycounter.btor2";
    SmtSolver s = smt::Z3SolverFactory::create(false);
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true");  // get unsat core
    cout<<"start..."<<endl;
    TransitionSystem ts(s);
    BTOR2Encoder be(filename, ts);
    // cout<<ts.trans_->to_string()<<endl;
    print_all_term(ts.trans_);

    cout<<"-------------------\n"<<"property :"<<endl;
    for (const auto & t:be.propvec())
    {
        cout<<t->to_string()<<endl;
    }
    return 0;
}