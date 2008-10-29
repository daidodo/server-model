#!/bin/sh

EXE=Server.out

rm ${EXE}
scp daizhao@essh.sandai.net:~/${EXE} .
chmod +x ${EXE}

VER=`./${EXE} -v | awk '{print $3}'`
EXE_VER=${EXE}.${VER}

mv ${EXE} ${EXE_VER}
ln -s ${EXE_VER} ${EXE}