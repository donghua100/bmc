import subprocess as sp
import argparse
import os
import signal


DEFAULT_STEP = 10000
DEFAULT_SOLVER = 'btor'
DEFAULT_THREAD_NUM = os.cpu_count()
DEFAULT_VCDPATH = 'dump.vcd'

header='''RUN BMC PARALLEL 
'''


procidx = 0
procs = {}
procIdx = []


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=str(header))
    p.add_argument('file', help='btor file name', type=str)
    p.add_argument('-t', '--thread', help='thread num', type=int, default=DEFAULT_THREAD_NUM)
    p.add_argument('-k', '--kstep', help='bmc steps', type=int, default= DEFAULT_STEP)
    p.add_argument('-s', '--solver', help='backend solver(default btor)',type=str, default=DEFAULT_SOLVER)
    p.add_argument('--vcd', help='vcd path', type=str, default=DEFAULT_VCDPATH)
    args = p.parse_args()
    file            = args.file
    thread_num      = args.thread
    kstep           = args.kstep
    solver          = args.solver
    vcdpath         = args.vcd
    BMC = './build/bmc'
    def preexec_fn():
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        os.setpgrp()

    for i in range(thread_num):
        cmd = f"{BMC} -v2  --start {i+1} --skip {thread_num} --solver {solver} -k {kstep} --vcd {vcdpath} {file}"
        print(cmd)
        proc = sp.Popen(['sh', '-c', cmd], stdout=sp.PIPE, stderr=sp.STDOUT,preexec_fn=preexec_fn)
        procs[procidx] = proc
        procIdx.append(procidx)
        procidx += 1
    for idx in procIdx:
        proc = procs[idx]
        print(f"proc:{proc.pid} start")
    while True:
        # print(len(procIdx))
        if len(procIdx)==0:
            break
        for idx in procIdx:
            proc = procs[idx]
            if proc.poll() is not None:
                procIdx.remove(idx)
                print(f"proc:{proc.pid} find conterexample")
                outs = proc.stdout.read()
                print(outs.decode('utf-8'))
                for k in procs.keys():
                    if k in procIdx:
                        procIdx.remove(k)
                        proc = procs[k]
                        os.killpg(proc.pid,signal.SIGTERM)
                        print(f"proc:{proc.pid} killed")






