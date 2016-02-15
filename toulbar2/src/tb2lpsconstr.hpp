/** \file tb2lpsconstr.hpp
 *  \brief Linear programming based global cost function : ssum, samong, sgcc, ssame, segcc, sdisjunction
 */

#ifndef TB2LPSCONSTR_HPP_
#define TB2LPSCONSTR_HPP_

#include "stddef.h"
#include "tb2linearconstr.hpp"

//#define upper_bound first
//#define lower_bound second

class LPSConstraint : public LinearConstraint 
{
private:
    int nwindows, nrows, nslacks;
    vector<int> sumlow, sumhigh, windowSize, subdef;
    vector<string> windowType;
    int** windowVars;
    int** group;
    int count2;
    int *wcspconstrcounter;
    void buildIndex();
    Cost buildMIP(MIP &mip);
    Cost solveMIP(MIP &mip);

public:
    static const int VALUE = 1;
    static const int VAR = 0;
    LPSConstraint(WCSP *wcsp, EnumeratedVariable** scope_in, int
            arity_in, int *constrcounter);

    ~LPSConstraint() {

    }

    string getName() {return "LPS constraint";}
    Cost evalOriginal (string s);
    virtual void read(istream &file);

    void print(ostream& os);
    void dump(ostream& os, bool original = true);
};


#endif /*TB2LPSCONSTR_HPP_*/


/* Local Variables: */
/* c-basic-offset: 4 */
/* tab-width: 4 */
/* indent-tabs-mode: nil */
/* c-default-style: "k&r" */
/* End: */

