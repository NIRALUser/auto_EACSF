#include <iostream>

#include "surfacecorrespondence.h"

using namespace std;

int main(int argc, char* argv[]) {
    //cout<<"inputs: "<<endl<<inputObj1<<endl<<inputObj2<<endl<<prefix<<endl<<dims<<endl<<endl;

    if (argc != 5) {
        cout << "-surfaceCorrespondence option needs two inputs, one prefix and the number of dims" << endl;
    }

    string inputObj1 = argv[1];
    string inputObj2 = argv[2];
    string prefix = argv[3];
    int dims = atoi(argv[4]);

    SurfaceCorrespondance sCorr(inputObj1,inputObj2,prefix,dims);
    sCorr.run();
}
