from genericpath import exists
import os
import subprocess as sp
import timeit
from multiprocessing.dummy import Pool as ThreadPool
from functools import reduce
import time
def getFiles(dir_path: str):
    files = []
    for root,dirs,fs in os.walk(dir_path):
        for f in fs:
            files.append(os.path.join(root,f))
    return files

def BMCAFile(file_name: str,Stype: int,outpath: str):
    if file_name.endswith('.btor') or file_name.endswith('.btor2'):
        mod_name = file_name.split('/')[-1]
        mod_name = reduce(lambda x, y: x + y, mod_name.split('.')[:-1])
        # print(mod_name)
        file = open(outpath + '/' + mod_name + '.log', 'w')
        file.write(time.strftime("%a %b %d %H:%M:%S %Y", time.localtime()))
        file.write('\n')
        file.write("Checking file: " + file_name + '\n')
        proc = sp.Popen(['./build/bmc', file_name,'-k', '100', '-s', str(Stype)], 
                        stdout=sp.PIPE, stderr=sp.STDOUT,encoding='utf-8')
        try:
            outs,_ = proc.communicate(timeout=3600)
        except sp.TimeoutExpired:
            proc.kill()
            outs,_ = proc.communicate()
            outs += 'TIMEOUT\n'
        file.write(outs)
        file.write(time.strftime("%a %b %d %H:%M:%S %Y", time.localtime()))
        file.write('\n')
        file.close()


def runBMCDir(Stype: int,outpath:str):
    pool = ThreadPool(os.cpu_count())
    pool.map(lambda file_name: BMCAFile(file_name,Stype,outpath), files)
    pool.close()
    pool.join()


if __name__ == '__main__':
    files_dir = "/home/guishuo/linux/hwmcc20"
    if not os.path.exists('outs'):
        os.mkdir("outs")
    files = getFiles(files_dir)
    d = {0:'btor',1:'z3',2:'cvc5'}
    for i in range(3):
        outpath = 'outs/'+d[i]
        if not os.path.exists(outpath):
            os.mkdir(outpath)
        print('test ',d[i])
        runBMCDir(i,outpath)
        print()
    # print((os.listdir(dir_path)))
    #cost = timeit.timeit(runBMCDir,number=1)
    #print(str(cost))
