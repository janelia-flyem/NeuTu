import sys
import os
import pgm2tiff
import subprocess as sp
if __name__=='__main__':
    if(len(sys.argv)<5):
        print('invalid arguments number')
    cd=os.path.split(os.path.realpath(__file__))[0]
    input_file=sys.argv[1]
    seed_file=sys.argv[2]
    output_file=sys.argv[3]
    al=sys.argv[4]
    n_i=input_file[:input_file.rfind('.')]+'.pgm'
    n_s=seed_file[:seed_file.rfind('.')]+'.pgm'
    pgm2tiff.tiff2pgm(input_file,n_i)
    pgm2tiff.tiff2pgm(seed_file,n_s)
    child=sp.Popen([os.path.join(cd,'powerwatsegm.exe'),'-a',al,'-i',n_i,'-m',n_s,'-o',os.path.join(cd,'result.pgm')])
    child.wait()
    pgm2tiff.pgm2tiff(os.path.join(cd,'result.pgm'),output_file)

