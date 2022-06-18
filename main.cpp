#include <cstddef>
#include<smt-switch/smt.h>
#include<smt-switch/boolector_factory.h>
#include <smt-switch/z3_factory.h>
#include <smt-switch/cvc5_factory.h>
#include"frontends/btor2_encoder.h"
#include"trans/ts.h"
#include"trans/unroller.h"
#include"bmc/bmc.h"
#include"printer/btor2_witness._printer.h"
#include "printer/vcd_printer.h"
#include "clipp.h"
#include<vector>
#include<string>
#include<cstring>
#include<dirent.h>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;
using namespace smt;
using namespace clipp;

// -DCMAKE_EXPORT_COMPILE_COMMANDS=ON



void test_one_file(string file,int k,bool inv = false,int Stype = 0,bool vcd = false)
{
    //cout<<"file name: "<<file<<endl;
	SmtSolver s = NULL;
	if (Stype == 0) s = smt::BoolectorSolverFactory::create(false);
	else if (Stype == 1) s = smt::Z3SolverFactory::create(false);
	else if (Stype == 2) s = smt::Cvc5SolverFactory::create(false);
	else{
		cout<<"do not support solver!"<<endl;
		exit(-1);
	} 
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true"); 
    try
    {   
        TransitionSystem ts(s);
        BTOR2Encoder be(file, ts);
        TermVec propvec = be.propvec();
        assert(propvec.size()>0);
        Property p(s,propvec[0]);
        cout << "start bmc.."<<endl;
        Bmc bmc(p,ts,s,inv);
        ProverResult r = bmc.check_until(k);
        if (r == ProverResult::FALSE)
        {
            cout << "sat" << endl;
            cout << "b" <<0<< endl;
            // cout<<"find counter-example!!!"<<endl;
            vector<smt::UnorderedTermMap> cex = bmc.witness();
			// cout<<"witness: "<<endl;
            print_witness_btor(be, cex, ts);
			VCDWitnessPrinter vcdprinter(ts,cex);
			if (vcd) vcdprinter.dump_trace_to_file("dump.vcd");
        }
        else if (r == ProverResult::UNKNOWN)
        {
            cout << "the circuit in safe in "<<k<<" steps"<<endl;
        }
        else if (r == ProverResult::ERROR)
        {
            cout <<"something error in checking!"<<endl; 
        }
        else if (r == ProverResult::TRUE)
        {
            cout <<"the circuit is verified!"<<endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}


int main(int argc,char *argv[])
{
	int k = 10;
	string file = "";
	bool inv = false;
	bool vcd = false;
	int Stype = 0;
	auto cli = (
		value("input file",file),
		option("-k").doc("bmc run k steps") & value("step",k),
		option("-inv").set(inv).doc("inverse bmc"),
		option("-s","--solver").set(Stype) & opt_value("Stype=0",Stype) % "0:btor,1:z3,2:cvc5(default: 0)",
		option("-vcd").set(vcd).doc("generagte vcd files if find conterexample")
	);

    if(!parse(argc, argv, cli)){
		cout << make_man_page(cli, argv[0]);
		exit(-1);
	} 
    test_one_file(file,k,inv,Stype,vcd);
	//using namespace std::chrono;
	//steady_clock::time_point t1 = steady_clock::now();
	//steady_clock::time_point t2 = steady_clock::now();
	//duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	//std::cout << "It took " << time_span.count() << " seconds.";
	//std::cout << std::endl;
}

