#include<smt-switch/smt.h>
#include<smt-switch/boolector_factory.h>
#include <smt-switch/z3_factory.h>
#include <smt-switch/cvc5_factory.h>>
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



void test_one_file(string file,int k,bool inv = false)
{
    cout<<"file name: "<<file<<endl;
    SmtSolver s = smt::BoolectorSolverFactory::create(false);
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
			vcdprinter.dump_trace_to_file("dump.vcd");
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
	auto cli = (
		value("input file",file),
		option("-k").set(k).doc("bmc run k steps"),
		option("-inv").set(inv).doc("inverse bmc")
	);

    if(!parse(argc, argv, cli)){
		cout << make_man_page(cli, argv[0]);
		exit(-1);
	} 
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
    test_one_file(file,k,inv);
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "It took " << time_span.count() << " seconds.";
	std::cout << std::endl;
}






void getFileNames(string path,vector<string> &filesvec)
{
    DIR *dr;
    struct dirent *en;
    dr = opendir(path.c_str());
    if (!dr) cout<<"opean file "<<path<<"error!"<<endl;
    while((en = readdir(dr))!=NULL)
    {
        if (strcmp(en->d_name,".")==0 || strcmp(en->d_name,"..") == 0) continue;
        else if (en->d_type == 8) 
        {
            // cout <<en->d_name<<endl;
            filesvec.push_back(path + "/" + en->d_name);
        }
        else if (en->d_type == 4) 
        {
            string subpath = path + "/" + en->d_name;
            getFileNames(subpath,filesvec);
        }
    }
    closedir(dr);
}


void test_all_file(string filepath)
{
    vector<string> filevec;
    getFileNames(filepath,filevec);
    cout<<"tot files : "<<filevec.size()<<endl;
    for (const auto & file:filevec) 
    {
        test_one_file(file,100,false);
        cout<<"-------------------------"<<endl;
    }
}

// int main(int argc,char *argv[])
// {
//     // string filepath = "../tests/encoder/input/btor2/array";
//     // vector<string> filevec;
//     // getFileNames(filepath,filevec);
//     // cout<<"tot files: "<<filevec.size()<<endl;
//     SmtSolver s = smt::Cvc5SolverFactory::create(false);
//     s->set_opt("incremental", "true");    
//     s->set_opt("produce-models", "true");  // get value
//     s->set_opt("produce-unsat-assumptions","true");  // get unsat core

//     // string testfile = "../tests/encoder/input/btor2/array/2019/mann/unsafe/ridecore_array_unsafe.btor";
//     // string testfile = "../tests/encoder/input/mybtor2/p-counter-false.btor2"; //  inv 2 步
//     // string testfile = "../tests/encoder/input/mybtor2/memory.btor2";
//     // string testfile = "../tests/encoder/input/btor2/bv/2019/beem/anderson.3.prop1-back-serstep.btor2";
    // string filepath = "../tests/encoder/input/btor2/array";
    // string file = "../tests/encoder/input/btor2/array/2019/wolf/2019B/marlann_compute_fail2-p1.btor"; // 12 vs 90+
    // string file1 = "../tests/encoder/input/btor2/bv/2020/mann/rast-p19.btor";  // 0 vs 0
    // string file2 = "../tests/encoder/input/btor2/bv/2019/beem/anderson.3.prop1-back-serstep.btor2";// 3 vs 112+
    // string file3 = "../tests/encoder/input/mybtor2/memory.btor2"; // 3 vs 100+
    // string file4 = "../tests/encoder/input/mybtor2/p-counter-false.btor2"; // 2 vs 1
    // string file5 = "../tests/encoder/input/btor2/array/2019/wolf/2019C/dblclockfft_butterfly_ck1-p006.btor";//UNSAT 10+ vs30+
    // string file6 = "../tests/encoder/input/btor2/array/2019/mann/unsafe/ridecore_array_unsafe.btor";
    // string file7 = "../tests/encoder/input/mybtor2/p-counter-false-10.btor2"; // 90 vs 2
//     int k = 0;
//     string testfile;
//     if(argc != 3) 
//     {
//         printf("usage: bmc ksteps filename\n");
//         return 0;
//     }
//     k = stoi(argv[1]);
//     testfile = argv[2];
//     try
//     {
//         TransitionSystem ts(s);
//         BTOR2Encoder be(testfile, ts);
//         TermVec propvec = be.propvec();
//         assert(propvec.size()>0);
//         Property p(s,propvec[0]);
//         cout << "start bmc.."<<endl;
//         Bmc bmc(p,ts,s);
//         // bmc.set_inv();
//         ProverResult r = bmc.check_until(k);
//         if (r == ProverResult::FALSE)
//         {
//             cout << "sat" << endl;
//             cout << "b" <<0<< endl;
//             // cout<<"find counter-example!!!"<<endl;
//             vector<smt::UnorderedTermMap> cex = bmc.witness();
//             print_witness_btor(be, cex, ts);
//         }
//         else if (r == ProverResult::UNKNOWN)
//         {
//             cout << "the circuit in safe in "<<k<<"steps"<<endl;
//         }
//         else if (r == ProverResult::ERROR)
//         {
//             cout <<"something error in checking!"<<endl; 
//         }
//         else if (r == ProverResult::TRUE)
//         {
//             cout <<"the circuit is verified!"<<endl;
//         }
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << '\n';
//     }
//     return 0;
// }
