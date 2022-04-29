#include<smt-switch/smt.h>
#include<smt-switch/cvc5_factory.h>
#include<smt-switch/boolector_factory.h>
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


// -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

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



void test_one_file(string file,int k,bool inv = false)
{
    cout<<"file name: "<<file<<endl;
	cout<<"1111";
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
            print_witness_btor(be, cex, ts);
        }
        else if (r == ProverResult::UNKNOWN)
        {
            cout << "the circuit in safe in "<<k<<"steps"<<endl;
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

int main(int argc,char *argv[])
{
    // string filepath = "../tests/encoder/input/btor2/array";
    // string file = "../tests/encoder/input/btor2/array/2019/wolf/2019B/marlann_compute_fail2-p1.btor"; // 12 vs 90+
    // string file1 = "../tests/encoder/input/btor2/bv/2020/mann/rast-p19.btor";  // 0 vs 0
    // string file2 = "../tests/encoder/input/btor2/bv/2019/beem/anderson.3.prop1-back-serstep.btor2";// 3 vs 112+
    // string file3 = "../tests/encoder/input/mybtor2/memory.btor2"; // 3 vs 100+
    // string file4 = "../tests/encoder/input/mybtor2/p-counter-false.btor2"; // 2 vs 1
    // string file5 = "../tests/encoder/input/btor2/array/2019/wolf/2019C/dblclockfft_butterfly_ck1-p006.btor";//UNSAT 10+ vs30+
    // string file6 = "../tests/encoder/input/btor2/array/2019/mann/unsafe/ridecore_array_unsafe.btor";
    // string file7 = "../tests/encoder/input/mybtor2/p-counter-false-10.btor2"; // 90 vs 2


    if (argc != 4)
    {
        printf("usesage:bmc ksteps filename inv(0 or 1)\n");
        exit(-1);
    }
    
    int k = stoi(argv[1]);
    string file = argv[2];
    string inv = argv[3];
    bool flag = false;
    if(inv == "1") flag = true;
    test_one_file(file,k,flag);
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
