#include"bmc.h"
#include "trans/ts.h"
#include"utils/exceptions.h"
#include <cstdio>
#include <sched.h>
#include <smt-switch/ops.h>
#include <smt-switch/result.h>
#include <smt-switch/smt_defs.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <cstring>
using namespace smt;
using namespace std;

//int Bmc::thread_nums = 1;
//ProverResult Bmc::result = ProverResult::UNKNOWN;
const int bufsize = 1024*1024*1024;
struct Buf{
	ProverResult r; 
	vector<UnorderedTermMap> witness_;
	SmtSolver solver_;
	TransitionSystem ts_;
	pid_t pid;
};
typedef struct Buf buf;

void *get_shm(){
	key_t key = ftok(".",2022);
	if (key == -1){
		perror("ftok");
		exit(-1);
	}
	int shmid = shmget(key,bufsize,IPC_CREAT|0666);
	if (shmid == -1){
		perror("shmget");
		exit(-1);
	}
	return shmat(shmid,NULL,0);
}


Bmc::Bmc(const Property & p, const TransitionSystem & ts,
         const SmtSolver & solver,int thread_nums_,bool inv)
         :solver_(solver),
         ts_(ts),
         unroller_(ts_),
         bad_(solver_->make_term(smt::PrimOp::Not,
                p.prop())),
         initialized_ (false),
         inv_(inv)
{
	thread_nums = thread_nums_;
	result = ProverResult::UNKNOWN;
}

Bmc::~Bmc() {}

void Bmc::initialize()
{
  if (initialized_) {
    return;
  }
  reached_k_ = -1;
  initialized_ = true;
  //if (inv_) solver_->assert_formula(unroller_.at_time(bad_,0));
  //else{
	//solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
	//}
}

ProverResult Bmc::task(int idx,int k){
	buf *shmaddr = (buf *)get_shm();
	for(int i = idx; i <= k; i+=thread_nums){
		if(!step(i)) {
			compute_witness();
			printf("pid = %u,find counterexample at bound %d\n",getpid(),i);
			//shmaddr->pid = getpid();
			//shmaddr->solver_ = solver_;
			//shmaddr->ts_ = ts_;
			//shmaddr->r = ProverResult::FALSE;
			return ProverResult::FALSE;
		}
	}
	return ProverResult::UNKNOWN;
}

ProverResult Bmc::check_until(int k)
{
  initialize();
	if (!step_0()){
		compute_witness();
		return ProverResult::FALSE;
	}
	reached_k_++;
	vector<pid_t> pids;
	buf *shmaddr = (buf *)get_shm();
	bzero(shmaddr,bufsize);
	printf("start %d process run bmc...\n",thread_nums);
  for (int idx =1; idx <= thread_nums; idx++) {
		pid_t pid = fork();
		if(pid == 0) {
			ProverResult r = task(idx, k);
			if (r!=ProverResult::FALSE) exit(0);
			else {
				return ProverResult::FALSE;
			}
		}
		else if (pid < 0) cout<<"fork error!"<<endl;
		else pids.push_back(pid);
	}
	// while(shmaddr->r==ProverResult::INIT_STATE){};
	for (const auto &pid:pids) waitpid(pid,NULL,0);
	//for (const auto &pid:pids) {
	//	kill(pid, SIGKILL);
	//}
	//if (shmaddr->r == ProverResult::FALSE) {
	//	solver_ = shmaddr->solver_;
	//	// 此处出现段错误，原因在于子进程中的对象随着子进程退出销毁，
	//	// 而赋值给共享内存的ts_传递是指针。而不是重新构造了新的对象 
	//	ts_ = shmaddr->ts_;
	//	compute_witness();
	//	cout<<"witness size: "<<witness_.size()<<endl;
	//	shmctl(shmid, IPC_RMID, NULL);
	//	return ProverResult::FALSE;
	//}
	//shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}

bool Bmc::step_0(){	
  std::cout<<"pid = "<<getpid()<<",Checking bmc at bound: "<<0<<std::endl;
	cur_max_t = 0;
  if (inv_){ 
		solver_->assert_formula(unroller_.at_time(bad_,0));
		solver_->push();
		solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
		Result r = solver_->check_sat();
		if (r.is_sat()) return false;
		else {
			solver_->pop();
			return true;
		}
	}
  else{
		solver_->assert_formula(unroller_.at_time(ts_.init(), 0));
		solver_->push();
		solver_->assert_formula(unroller_.at_time(bad_, 0));
		Result r = solver_->check_sat();
		if (r.is_sat()) return false;
		else {
			solver_->pop();
			return true;
		}
	}
	return true;
}

bool Bmc::step(int i)
{
  if (i <= reached_k_) {
    return true;
  }
  bool res = true;
	if (cur_max_t<i){
		for (int j = cur_max_t+1;j < i; j++) {
			solver_->assert_formula(unroller_.at_time(ts_.trans(),j-1));
			reached_k_++;
		}
	}
  cur_max_t = i;
  solver_->assert_formula(unroller_.at_time(ts_.trans(), i - 1));
  solver_->push();
  std::cout<<"pid = "<<getpid()<<",Checking bmc at bound: "<<i<<std::endl;
  if (inv_) solver_->assert_formula(unroller_.at_time(ts_.init(),i));
  else
  solver_->assert_formula(unroller_.at_time(bad_, i));
  Result r = solver_->check_sat();
  if (r.is_sat()) {
    res = false;
  } else {
    solver_->pop();
    ++reached_k_;
  }

  return res;
}

bool Bmc::compute_witness()
{
  // TODO: make sure the solver state is SAT

  for (int i = 0; i <= reached_k_ + 1; ++i) {
    witness_.push_back(UnorderedTermMap());
    UnorderedTermMap & map = witness_.back();

    for (const auto &v : ts_.statevars()) {
      const Term &vi = unroller_.at_time(v, i);
      const Term &r = solver_->get_value(vi);
      map[v] = r;
    }

    for (const auto &v : ts_.inputvars()) {
      const Term &vi = unroller_.at_time(v, i);
      const Term &r = solver_->get_value(vi);
      map[v] = r;
    }

    for (const auto &elem : ts_.named_terms()) {
      const Term &ti = unroller_.at_time(elem.second, i);
      map[elem.second] = solver_->get_value(ti);
    }
  }

  return true;
}

