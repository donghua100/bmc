#include "smt-switch/include/smt.h"
#include "smt-switch/include/boolector_factory.h"
#include "smt-switch/include/solver_enums.h"
#include "smt-switch/include/z3_factory.h"
#include "smt-switch/include/cvc5_factory.h"
#include "smt-switch/include/msat_factory.h"
#include "smt-switch/include/yices2_factory.h"
#include "frontends/btor2_encoder.h"
#include "trans/ts.h"
#include "trans/unroller.h"
#include "bmc/bmc.h"
#include "printer/btor2_witness_printer.h"
#include "printer/vcd_printer.h"
#include "utils/logger.h"
#include "clipp.h"
#include <cstddef>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;
using namespace smt;
using namespace clipp;

// -DCMAKE_EXPORT_COMPILE_COMMANDS=ON



void test_one_file(string file,int k,int skip, bool inv,string slv,string vcdpath)
{
    //cout<<"file name: "<<file<<endl;
	SmtSolver s = NULL;
	if (slv == "btor") s = smt::BoolectorSolverFactory::create(false);
	else if (slv == "z3") s = smt::Z3SolverFactory::create(false);
	else if (slv == "cvc5") s = smt::Cvc5SolverFactory::create(false);
	else if (slv == "msat") s = smt::MsatSolverFactory::create(false);
	else if (slv == "yices") s = smt::Yices2SolverFactory::create(false);
	else{
		cout<<"not support solver!"<<endl;
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
        Bmc bmc(p,ts,s,inv,skip);
        ProverResult r = bmc.check_until(k);
        if (r == ProverResult::FALSE)
        {
            cout<<"find counter-example"<<endl;
            cout << "sat" << endl;
            cout << "b" <<0<< endl;
            vector<smt::UnorderedTermMap> cex = bmc.witness();
            print_witness_btor(be, cex, ts);
			VCDWitnessPrinter vcdprinter(ts,cex);
			vcdprinter.dump_trace_to_file(vcdpath);
        }
        else if (r == ProverResult::UNKNOWN)
        {
			cout<<"unknow"<<endl;
        }
        else if (r == ProverResult::ERROR)
        {
            cout <<"error"<<endl; 
        }
        else if (r == ProverResult::TRUE)
        {
            cout <<"verified"<<endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}


int main(int argc,char *argv[])
{
	int k = 10000;
	int skip = 1;
	string file = "";
	bool inv = false;
	string vcdpath = "dump.vcd";
	string slv = "btor";
	int v = 0;
	auto cli = (
		option("-k").doc("bmc run k steps") & value("step",k),
		option("--skip").doc("skip steps") & value("skip",skip),
		option("--inv").set(inv).doc("inverse bmc"),
		option("-s","--solver") & value("solver",slv) % "btor(by default), z3, cvc5, msat, yices",
		option("--vcd") & value("vcdpath",vcdpath) % ("vcd file output path if find conterexample"),
		option("-v", "--verbose") & value("verbose", v) % ("verbose level(default 0)"),
		value("input file",file)
	);

    if(!parse(argc, argv, cli)){
		cout << make_man_page(cli, argv[0]);
		exit(-1);
	}
	logger.set_verbosity(v);
    test_one_file(file, k, skip, inv, slv, vcdpath);
}

