/** \file tb2solver.hpp
 *  \brief Generic solver.
 * 
 */
 
#ifndef TB2SOLVER_HPP_
#define TB2SOLVER_HPP_

#include "toulbar2.hpp"

class Solver
{
    static Solver *currentSolver;

    Store *store;
    long long nbNodes;
    long long nbBacktracks;
    WeightedCSP *wcsp;
    Domain *unassignedVars;
        
    // Heuristics and search methods
    int getVarMinDomainDivMaxDegree();
    int getNextUnassignedVar();
    void binaryChoicePoint(int xIndex, Value value);
    void narySortedChoicePoint(int xIndex);
    void recursiveSolve();
    
public:
    Solver(int storeSize, Cost initUpperBound);
    ~Solver();
    
    void read_wcsp(const char *fileName);
    
    bool solve();
    
    friend void setvalue(int wcspId, int varIndex, Value value);
};

#endif /*TB2SOLVER_HPP_*/
