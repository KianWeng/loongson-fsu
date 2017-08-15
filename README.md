# loongson-fsu
Set up env:
1.GCC tool 
    #tar zxvf gcc-4.3-ls232-softfloat.tar.gz
    #sudo cp -rf gcc-4.3-ls232-softfloat/ /opt/
2.pmon depend lib
    #sudo apt-get install bison
    #sudo apt-get install flex
    #sudo apt-get install xutils-dev

Compile
    #cp ls1b-hangdian.config .config
    #make
