#include <Tree.h>

#include "comm.h"

static bool testTree()
{
    typedef CTree<int> __Tree;
    __Tree tree;

    return true;
}

int main()
{
    if(!testTree())
        return 1;
    cout<<"Tree test succ\n";
    return 0;
}
