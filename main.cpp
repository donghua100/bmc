#include<smt-switch/smt.h>
#include<smt-switch/cvc5_factory.h>
#include"frontends/btor2_encoder.h"
#include"trans/ts.h"
#include"trans/unroller.h"
#include"bmc/bmc.h"
#include"printer/btor2_witness._printer.h"
#include<vector>
#include<string>
#include<cstring>
#include<dirent.h>


using namespace std;
using namespace smt;

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


int main()
{
    string filepath = "../tests/encoder/input/benchmark";
    vector<string> filevec;
    getFileNames(filepath,filevec);
    cout<<"tot files: "<<filevec.size()<<endl;
    SmtSolver s = smt::Cvc5SolverFactory::create(false);
    s->set_opt("incremental", "true");    
    s->set_opt("produce-models", "true");  // get value
    s->set_opt("produce-unsat-assumptions","true");  // get unsat core

    // string testfile = "../tests/encoder/input/btor2/array/2019/mann/unsafe/ridecore_array_unsafe.btor";
    // string testfile = "../tests/encoder/input/mybtor2/p-counter-false.btor2"; //  inv 2 步
    // string testfile = "../tests/encoder/input/mybtor2/memory.btor2";
    // string testfile = "../tests/encoder/input/btor2/bv/2019/beem/anderson.3.prop1-back-serstep.btor2";
    // try
    // {
    //     TransitionSystem ts(s);
    //     BTOR2Encoder be(testfile, ts);
    //     TermVec propvec = be.propvec();
    //     assert(propvec.size()>0);
    //     Property p(s,propvec[0]);
    //     cout << "start bmc.."<<endl;
    //     Bmc bmc(p,ts,s);
    //     // bmc.set_inv();
    //     ProverResult r = bmc.check_until(200);
    //     if (r == ProverResult::FALSE)
    //     {
    //         cout << "sat" << endl;
    //         cout << "b" <<0<< endl;
    //         // cout<<"find counter-example!!!"<<endl;
    //         vector<smt::UnorderedTermMap> cex = bmc.witness();
    //         print_witness_btor(be, cex, ts);
    //     }
    //     else if (r == ProverResult::UNKNOWN)
    //     {
    //         cout << "the circuit in safe in "<<200<<"steps"<<endl;
    //     }
    //     else if (r == ProverResult::ERROR)
    //     {
    //         cout <<"something error in checking!"<<endl; 
    //     }
    //     else if (r == ProverResult::TRUE)
    //     {
    //         cout <<"the circuit is verified!"<<endl;
    //     }
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    // int skip = 0;
    for (const auto & file:filevec)
    {
        SmtSolver s = smt::Cvc5SolverFactory::create(false);
        s->set_opt("incremental", "true");    
        s->set_opt("produce-models", "true");  // get value
        s->set_opt("produce-unsat-assumptions","true"); 
        // skip++;
        // if (skip < 500) continue;
        // cout << "test file: "<<file<<endl;
        try
        {   
            TransitionSystem ts(s);
            BTOR2Encoder be(file, ts);
            TermVec propvec = be.propvec();
            assert(propvec.size()>0);
            Property p(s,propvec[0]);
            cout << "start bmc.."<<endl;
            Bmc bmc(p,ts,s);
            ProverResult r = bmc.check_until(200);
            if (r == ProverResult::FALSE)
            {
                cout << "sat" << endl;
                cout << "b" <<0<< endl;
                // cout<<"find counter-example!!!"<<endl;
                vector<smt::UnorderedTermMap> cex = bmc.witness();
                print_witness_btor(be, cex, ts);
            }
            else if (r == ProverResult::UNKNOWN)
            {
                cout << "the circuit in safe in "<<300<<"steps"<<endl;
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
    return 0;
}