/*
 * **************** Backtrack and Russian Doll Search with Tree Decomposition *******************
 *
 */

#include "tb2solver.hpp"
#include "tb2domain.hpp"
#include "tb2clusters.hpp"

/*
 * Variable ordering heuristics
 *
 */

int Solver::getNextUnassignedVar(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;
  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) return *iter;
  }
  return -1;
}

int Solver::getVarMinDomainDivMaxDegree(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;
  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    int deg = wcsp->getDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        }
	}
  }
  return varIndex;
}

int Solver::getVarMinDomainDivMaxDegreeRandomized(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;
  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;
  int ties[cluster->getNbVars()];
  int nbties = 0;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    int deg = wcsp->getDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
			nbties = 1;
			ties[0] = varIndex;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        } else if (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) == worstUnaryCost) {
		 ties[nbties] = *iter;
		 nbties++;
		}
	}
  }
  if (nbties>1) return ties[myrand()%nbties];
  else return varIndex;
}

int Solver::getVarMinDomainDivMaxDegreeLastConflict(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;

  if (lastConflictVar != -1 && wcsp->unassigned(lastConflictVar) && cluster->isVar(lastConflictVar)) return lastConflictVar;

  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    int deg = wcsp->getDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        }
    }
  }
  return varIndex;
}

int Solver::getVarMinDomainDivMaxDegreeLastConflictRandomized(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;

  if (lastConflictVar != -1 && wcsp->unassigned(lastConflictVar) && cluster->isVar(lastConflictVar)) return lastConflictVar;

  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;
  int ties[cluster->getNbVars()];
  int nbties = 0;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    int deg = wcsp->getDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
			nbties = 1;
			ties[0] = varIndex;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        } else if (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) == worstUnaryCost) {
		 ties[nbties] = *iter;
		 nbties++;
		}
    }
  }
  if (nbties>1) return ties[myrand()%nbties];
  else return varIndex;
}

int Solver::getVarMinDomainDivMaxWeightedDegree(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;
  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    Cost unarymediancost = MIN_COST;
	   	int domsize = wcsp->getDomainSize(*iter);
	   	if (ToulBar2::weightedTightness) {
	   	   ValueCost array[domsize];
	   	   wcsp->getEnumDomainAndCost(*iter, array);
	   	   unarymediancost = stochastic_selection<ValueCost>(array, 0, domsize-1, domsize/2).cost;
	   	}
	    Long deg = wcsp->getWeightedDegree(*iter) + 1 + unarymediancost; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) domsize / (double) max(deg,(Long)1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        }
	}
  }
  return varIndex;
}

int Solver::getVarMinDomainDivMaxWeightedDegreeRandomized(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;
  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;
  int ties[cluster->getNbVars()];
  int nbties = 0;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
	    Cost unarymediancost = MIN_COST;
	   	int domsize = wcsp->getDomainSize(*iter);
	   	if (ToulBar2::weightedTightness) {
	   	   ValueCost array[domsize];
	   	   wcsp->getEnumDomainAndCost(*iter, array);
	   	   unarymediancost = stochastic_selection<ValueCost>(array, 0, domsize-1, domsize/2).cost;
	   	}
	    Long deg = wcsp->getWeightedDegree(*iter) + 1 + unarymediancost; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
        double heuristic = (double) domsize / (double) max(deg,(Long)1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
			nbties = 1;
			ties[0] = varIndex;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        } else if (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) == worstUnaryCost) {
		 ties[nbties] = *iter;
		 nbties++;
		}
	}
  }
  if (nbties>1) return ties[myrand()%nbties];
  else return varIndex;
}

int Solver::getVarMinDomainDivMaxWeightedDegreeLastConflict(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;

  if (lastConflictVar != -1 && wcsp->unassigned(lastConflictVar) && cluster->isVar(lastConflictVar)) return lastConflictVar;

  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
		Cost unarymediancost = MIN_COST;
		int domsize = wcsp->getDomainSize(*iter);
		if (ToulBar2::weightedTightness) {
		   ValueCost array[domsize];
		   wcsp->getEnumDomainAndCost(*iter, array);
		   unarymediancost = stochastic_selection<ValueCost>(array, 0, domsize-1, domsize/2).cost;
		}
        double heuristic = (double) domsize / (double) (wcsp->getWeightedDegree(*iter) + 1 + unarymediancost);
//	      Long deg = wcsp->getWeightedDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
//        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,(Long)1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        }
    }
  }
  return varIndex;
}

int Solver::getVarMinDomainDivMaxWeightedDegreeLastConflictRandomized(Cluster *cluster)
{
  if (unassignedVars->empty()) return -1;

  if (lastConflictVar != -1 && wcsp->unassigned(lastConflictVar) && cluster->isVar(lastConflictVar)) return lastConflictVar;

  int varIndex = -1;
  Cost worstUnaryCost = MIN_COST;
  double best = MAX_VAL - MIN_VAL;
  int ties[cluster->getNbVars()];
  int nbties = 0;

  for (TVars::iterator iter = cluster->beginVars(); iter!= cluster->endVars(); ++iter) {
	if (wcsp->unassigned(*iter)) {
		Cost unarymediancost = MIN_COST;
		int domsize = wcsp->getDomainSize(*iter);
		if (ToulBar2::weightedTightness) {
		   ValueCost array[domsize];
		   wcsp->getEnumDomainAndCost(*iter, array);
		   unarymediancost = stochastic_selection<ValueCost>(array, 0, domsize-1, domsize/2).cost;
		}
        double heuristic = (double) domsize / (double) (wcsp->getWeightedDegree(*iter) + 1 + unarymediancost);
//	      Long deg = wcsp->getWeightedDegree(*iter) + 1; // - ((WCSP *)wcsp)->getVar(*iter)->nbSeparators();
//        double heuristic = (double) wcsp->getDomainSize(*iter) / (double) max(deg,(Long)1);
        if (varIndex < 0 || heuristic < best - epsilon * best
            || (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) > worstUnaryCost)) {
            best = heuristic;
            varIndex = *iter;
			nbties = 1;
			ties[0] = varIndex;
            worstUnaryCost = wcsp->getMaxUnaryCost(*iter);
        } else if (heuristic < best + epsilon * best && wcsp->getMaxUnaryCost(*iter) == worstUnaryCost) {
		  ties[nbties] = *iter;
		  nbties++;
		}
    }
  }
  if (nbties>1) return ties[myrand()%nbties];
  else return varIndex;
}


/*
 * Choice points
 *
 */

pair<Cost,Cost> Solver::binaryChoicePoint(Cluster *cluster, Cost lbgood, Cost cub, int varIndex, Value value)
{
    assert(lbgood < cub);
	if (ToulBar2::interrupted) throw TimeOut();
	Cost clb = cub;
	TreeDecomposition* td = wcsp->getTreeDec();
    assert(wcsp->unassigned(varIndex));
    assert(wcsp->canbe(varIndex,value));
    bool dichotomic = (ToulBar2::dichotomicBranching && ToulBar2::dichotomicBranchingSize < wcsp->getDomainSize(varIndex));
    Value middle = value;
    bool increasing = true;
    if (dichotomic) {
      middle = (wcsp->getInf(varIndex) + wcsp->getSup(varIndex)) / 2;
      if (value <= middle) increasing = true;
      else increasing = false;
    }
    try {
        store->store();
		assert(td->getCurrentCluster() == cluster);
		assert(wcsp->getLb() == cluster->getLbRec());
		wcsp->setUb(cub);
		Cost bestlb = lbgood;
		if (CUT(bestlb, cub)) THROWCONTRADICTION;
		if(ToulBar2::btdMode >= 2) {
			Cost rds = td->getLbRecRDS();
			bestlb = MAX(bestlb, rds);
			if(CUT(bestlb, cub)) THROWCONTRADICTION;
		}
        lastConflictVar = varIndex;
        if (dichotomic) {
    	  if (increasing) decrease(varIndex, middle); else increase(varIndex, middle+1);
    	} else assign(varIndex, value);
        lastConflictVar = -1;
        bestlb = MAX(bestlb, wcsp->getLb());
        pair<Cost,Cost> res = recursiveSolve(cluster, bestlb, cub);
		clb = MIN(res.first, clb);
		cub = MIN(res.second,cub);
    } catch (Contradiction) {
        wcsp->whenContradiction();
    }
    store->restore();
    nbBacktracks++;
	if (ToulBar2::restart>0 && nbBacktracks > nbBacktracksLimit) throw NbBacktracksOut();
    cluster->nbBacktracks++;
    try {
        store->store();
		assert(wcsp->getTreeDec()->getCurrentCluster() == cluster);
		assert(wcsp->getLb() == cluster->getLbRec());
		wcsp->setUb(cub);
		Cost bestlb = lbgood;
		if (CUT(bestlb, cub)) THROWCONTRADICTION;
		if(ToulBar2::btdMode >= 2) {
			Cost rds = td->getLbRecRDS();
			bestlb = MAX(bestlb, rds);
			if(CUT(bestlb, cub)) THROWCONTRADICTION;
		}
		if (dichotomic) {
		  if (increasing) increase(varIndex, middle+1, cluster->nbBacktracks >= cluster->hybridBFSLimit); else decrease(varIndex, middle, cluster->nbBacktracks >= cluster->hybridBFSLimit);
		} else remove(varIndex, value, cluster->nbBacktracks >= cluster->hybridBFSLimit);
		bestlb = MAX(bestlb, wcsp->getLb());
		if (cluster->nbBacktracks >= cluster->hybridBFSLimit) {
		    addOpenNode(*(cluster->cp), *(cluster->open), bestlb, cluster->getCurrentDelta());
		} else {
		    pair<Cost,Cost> res = recursiveSolve(cluster, bestlb, cub);
	        clb = MIN(res.first, clb);
		    cub = MIN(res.second,cub);
		}
    } catch (Contradiction) {
	  wcsp->whenContradiction();
    }
    store->restore();
    assert(lbgood <= clb);
    assert(clb <= cub);
	return make_pair(clb,cub);
}

/*
 * Choice points for solution counting
 *
 */

BigInteger Solver::binaryChoicePointSBTD(Cluster *cluster, int varIndex, Value value)
{
	if (ToulBar2::interrupted) throw TimeOut();
	Cost cub = 1;
	Cost lbgood = 0;
	BigInteger nbSol = 0, nb = 0;
	assert(wcsp->unassigned(varIndex));
	assert(wcsp->canbe(varIndex,value));
	bool dichotomic = (ToulBar2::dichotomicBranching && ToulBar2::dichotomicBranchingSize < wcsp->getDomainSize(varIndex));
	Value middle = value;
	bool increasing = true;
	if (dichotomic) {
		middle = (wcsp->getInf(varIndex) + wcsp->getSup(varIndex)) / 2;
		if (value <= middle) increasing = true;
		else increasing = false;
	}
	try {
		store->store();
		assert(wcsp->getTreeDec()->getCurrentCluster() == cluster);

		wcsp->setUb(cub);
		if (CUT(lbgood, cub)) THROWCONTRADICTION;
		lastConflictVar = varIndex;
		if (dichotomic) {
			if (increasing) decrease(varIndex, middle); else increase(varIndex, middle+1);
		} else assign(varIndex, value);
		lastConflictVar = -1;
		nb = sharpBTD(cluster);
		nbSol += nb;
	} catch (Contradiction) {
		wcsp->whenContradiction();
	}
	store->restore();
	nbBacktracks++;
	if (ToulBar2::restart>0 && nbBacktracks > nbBacktracksLimit) throw NbBacktracksOut();
	try {
		store->store();
		assert(wcsp->getTreeDec()->getCurrentCluster() == cluster);
//		assert(wcsp->getLb() == cluster->getLbRec());
		wcsp->setUb(cub);
		if (CUT(lbgood, cub)) THROWCONTRADICTION;
		if (dichotomic) {
			if (increasing) increase(varIndex, middle+1); else decrease(varIndex, middle);
		} else remove(varIndex, value);

		nb = sharpBTD(cluster);
		nbSol += nb;
	} catch (Contradiction) {
		wcsp->whenContradiction();
	}
	store->restore();
	return nbSol;
}

/*
 * Backtrack with Tree Decomposition
 *
 */

// Maintains the best (monotonically increasing) lower bound of the cluster in parameter lbgood

pair<Cost,Cost> Solver::recursiveSolve(Cluster *cluster, Cost lbgood, Cost cub)
{
  if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] recursive solve     cluster: " << cluster->getId() << "     clb: " << lbgood << "     cub: " << cub << "     clb0: " << cluster->getLb() << "     wcsp->lb: " << wcsp->getLb() << "     wcsp->ub: " << wcsp->getUb() << endl;
  assert(lbgood <= cub);
  TreeDecomposition* td = wcsp->getTreeDec();
  int varIndex = -1;
  if(ToulBar2::Static_variable_ordering) varIndex = getNextUnassignedVar(cluster);
  else if(ToulBar2::weightedDegree && ToulBar2::lastConflict) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxWeightedDegreeLastConflictRandomized(cluster):getVarMinDomainDivMaxWeightedDegreeLastConflict(cluster));
  else if(ToulBar2::lastConflict) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxDegreeLastConflictRandomized(cluster):getVarMinDomainDivMaxDegreeLastConflict(cluster));
  else if(ToulBar2::weightedDegree) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxWeightedDegreeRandomized(cluster):getVarMinDomainDivMaxWeightedDegree(cluster));
  else varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxDegreeRandomized(cluster):getVarMinDomainDivMaxDegree(cluster));

  if (varIndex < 0) {
	// Current cluster is completely assigned
	Cost clb = wcsp->getLb();
	assert(clb <= cub);
	Cost csol = clb;
	for (TClusters::iterator iter = cluster->beginEdges(); clb < cub && iter!= cluster->endEdges(); ) {
	  // Solves each cluster son with local lower and upper bounds
	  Cluster* c = *iter;
	  ++iter;
	  Cost lbSon = MIN_COST;
      Cost ubSon = MAX_COST;
	  bool good = false;
	  if (!c->isActive()) {
		c->reactivate();
		c->nogoodGet(lbSon, ubSon, &c->open);
		good = true;
	  } else {
		lbSon = c->getLbRec();
		ubSon = c->getUb();
#ifndef NDEBUG
		Cost dummylb = -MAX_COST;
		Cost tmpub = -MAX_COST;
		c->nogoodGet(dummylb, tmpub, &c->open);
        assert(tmpub == ubSon);
#endif
	  }
//	  if (ToulBar2::verbose >= 2) cout << "lbson: " << lbSon << " ubson: " << ubSon << " lbgood:" << lbgood << " clb: " << clb << " csol: " << csol << " cub: " << cub << endl;
	  if (lbSon < ubSon) { // we do not have an optimality proof
	      if (clb <= lbgood || (csol < MAX_COST && ubSon >= cub - csol + lbSon)) { // we do not know a good enough son's solution or the currently reconstructed father's solution is not working or the currently reconstructed father's lower bound is not increasing
	          ubSon = MIN(ubSon, cub - clb + lbSon);
	          td->setCurrentCluster(c);
	          wcsp->setUb(ubSon);
	          wcsp->setLb((good)?c->getLbRec():lbSon);
	          try {
	              store->store();
	              wcsp->enforceUb();
	              wcsp->propagate();
	              Cost bestlb = MAX(wcsp->getLb(),lbSon);
	              if (csol < MAX_COST && iter == cluster->endEdges()) bestlb = MAX(bestlb, lbgood-csol+lbSon); // simple trick to provide a better initial lower bound for the last son
	              if(ToulBar2::btdMode >= 2) {
	                  Cost rds = td->getLbRecRDS();
	                  bestlb = MAX(bestlb, rds);
	                  if (CUT(bestlb, ubSon)) THROWCONTRADICTION;
	              }
	              pair<Cost,Cost> res = hybridSolve(c, bestlb, ubSon);
                  assert(res.first >= bestlb && res.second <= ubSon);
	              c->nogoodRec(res.first, ((res.second<ubSon)?res.second:MAX_COST), &c->open);
	              clb += res.first - lbSon;
	              if (csol < MAX_COST) {
	                  if (res.second < ubSon) csol += res.second - lbSon;
	                  else csol = MAX_COST;
	              }
	          } catch (Contradiction) {
	              wcsp->whenContradiction();
	              c->nogoodRec(ubSon, MAX_COST, &c->open);
	              clb = cub;
	              csol = MAX_COST;
	          }
	          store->restore();
	      } else {
	          if (csol < MAX_COST) {
	              assert(ubSon < MAX_COST);
	              csol += ubSon - lbSon;
	          }
	      }
	  }
	}
	assert(csol >= clb);
	if (csol < cub) {
	    // A new solution has been found for the current cluster
	    cub = csol;
		cluster->solutionRec(csol);
		if(cluster == td->getRoot() || cluster == td->getRootRDS()) {
			if (ToulBar2::verbose>=0 || ToulBar2::showSolutions) {
				if(!ToulBar2::bayesian) cout << "New solution: " <<  csol << " (" << nbBacktracks << " backtracks, " << nbNodes << " nodes, depth " << store->getDepth() << ")" << endl;
				else cout << "New solution: " << csol << " log10like: " << wcsp->Cost2LogLike(csol) + ToulBar2::markov_log << " prob: " << wcsp->Cost2Prob(csol) * Exp10(ToulBar2::markov_log) << " (" << nbBacktracks << " backtracks, " << nbNodes << " nodes, depth " << store->getDepth() << ")" << endl;
			}
			if(cluster == td->getRoot()) td->newSolution(csol);
			else {
			  assert(cluster == td->getRootRDS());
			  // Remember current solution for value ordering heuristic
			  wcsp->restoreSolution(cluster);
			  TAssign a;
			  cluster->getSolution( a );
			  if (ToulBar2::showSolutions) {
				TAssign::iterator it = a.begin();
				while(it != a.end()) {
				  Value v = it->second;
				  cout << it->first << ":" << v << " ";
				  ++it;
				}
				cout << endl;
			  }
			}
		}
	}
	if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] C" << cluster->getId() << " return " << MAX(lbgood,clb) << " " << cub << endl;
	assert(MAX(lbgood,clb) <= cub);
	return make_pair(MAX(lbgood,clb),cub);
  }
  else {
	// Enumerates cluster proper variables
  	*((StoreCost *) searchSize) += ((Cost) (10e6 * log(wcsp->getDomainSize(varIndex))));
  	pair<Cost,Cost> res = make_pair(MIN_COST,MAX_COST);
	if (wcsp->enumerated(varIndex)) {
	  assert(wcsp->canbe(varIndex, wcsp->getSupport(varIndex)));
	  // Reuse last solution found if available
	  Value bestval = ((ToulBar2::verifyOpt)?(wcsp->getSup(varIndex)+1):wcsp->getBestValue(varIndex));
	  res = binaryChoicePoint(cluster, lbgood, cub, varIndex, (wcsp->canbe(varIndex, bestval))?bestval:wcsp->getSupport(varIndex));
	} else {
	  res = binaryChoicePoint(cluster, lbgood, cub, varIndex, wcsp->getInf(varIndex));
	}
	if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] C" << cluster->getId() << " return " << res.first << " " << res.second << endl;
	assert(res.first >= lbgood);
	assert(res.second <= cub);
	assert(res.first <= res.second);
	return res;
  }
}


/*
 * Russian Doll Search with Tree Decomposition
 *
 */

void Solver::russianDollSearch(Cluster *c, Cost cub)
{
	TreeDecomposition* td = wcsp->getTreeDec();

	TClusters::iterator it = c->beginEdges();
	while(it != c->endEdges()) {
		russianDollSearch(*it, cub);
		++it;
	}

	try {
	  store->store();

	  if(c != td->getRoot()) {
	      c->deconnectSep();
		  assert(c->getLbRec() == MIN_COST);
		  c->setLb(MIN_COST);
		  wcsp->setLb(MIN_COST);
		  td->setCurrentCluster(td->getRoot());
		  Cost lbroot = td->getLbRecRDS();
		  td->setCurrentCluster(c);
		  Cost lbc = td->getLbRecRDS();
		  cub = cub - lbroot + lbc;
	  }
	  wcsp->setUb(cub);
	  td->setCurrentCluster(c);
	  td->setRootRDS(c);
	  lastConflictVar = -1;

	  if(ToulBar2::verbose>=0) cout << "--- Solving cluster subtree " << c->getId() << " ..." << endl;

	  if(c == td->getRoot()) wcsp->propagate(); // needed if there are connected components
	  Cost rdslb = td->getLbRecRDS();
	  pair<Cost, Cost> res = hybridSolve(c, rdslb, cub);
	  assert(res.first >= rdslb);
	  c->setLbRDS(res.first);
	  if (c->sepSize() == 0)  {
	      c->nogoodRec(res.first, ((res.second<cub)?res.second:MAX_COST), &c->open);
	      if (c->open) c->open->clear();
	  }

	  if (ToulBar2::debug || ToulBar2::verbose >= 1) c->printStatsRec();
	  if(ToulBar2::verbose>=0) cout << "---  done  cost = " << res.second << " ("    << nbBacktracks << " backtracks, " << nbNodes << " nodes, depth " << store->getDepth() << ")" << endl << endl;

	} catch (Contradiction) {
	  wcsp->whenContradiction();
	  c->setLbRDS(cub);
	  if (c->sepSize() == 0) {
	      c->nogoodRec(cub, MAX_COST, &c->open);
	      if (c->open) c->open->clear();
	  }
	}
	store->restore();
    c->resetOptRec(c);
}


/*
 * Backtrack with Tree Decomposition for counting in CSP
 *
 */

BigInteger Solver::sharpBTD(Cluster *cluster){

	TreeDecomposition* td = wcsp->getTreeDec();
	BigInteger NbSol = 0,nb = 0;
	TCtrs totalList;
	if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] recursive solve     cluster: " << cluster->getId() << " **************************************************************"<< endl;

	int varIndex = -1;
	if(ToulBar2::Static_variable_ordering) varIndex = getNextUnassignedVar(cluster);
	else if(ToulBar2::weightedDegree && ToulBar2::lastConflict) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxWeightedDegreeLastConflictRandomized(cluster):getVarMinDomainDivMaxWeightedDegreeLastConflict(cluster));
	else if(ToulBar2::lastConflict) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxDegreeLastConflictRandomized(cluster):getVarMinDomainDivMaxDegreeLastConflict(cluster));
	else if(ToulBar2::weightedDegree) varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxWeightedDegreeRandomized(cluster):getVarMinDomainDivMaxWeightedDegree(cluster));
	else varIndex = ((ToulBar2::restart>0)?getVarMinDomainDivMaxDegreeRandomized(cluster):getVarMinDomainDivMaxDegree(cluster));

	if (varIndex < 0) {
		// Current cluster is completely assigned
		Cost lb = wcsp->getLb();
		if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] C" << cluster->getId() << " lb= " << lb << endl;
		NbSol = 1*cluster->getCount();

		if(ToulBar2::approximateCountingBTD && cluster->getParent()==NULL)
			totalList = cluster->getCtrsTree();

		for (TClusters::iterator iter = cluster->beginEdges(); NbSol>0 && iter!= cluster->endEdges(); ++iter) {
			// Solves each cluster son
			nb = 0;
			Cluster* c = *iter;
			if((nb = c->sgoodGet()) !=-1){
				nbSGoodsUse++;
			}
			else
			{
				nb = 0;
				td->setCurrentCluster(c);
				try{
					store->store();
					if(ToulBar2::approximateCountingBTD){
						if(c->getParent() != NULL && c->getParent()->getParent() == NULL && c->getNbVars()>1){
							// for this son of root, we disconnect the constraints which isn't in intersection
							double time= cpuTime();
							TCtrs usefulCtrsList = c->getCtrsTree();
							c->deconnectDiff(totalList, usefulCtrsList);

							timeDeconnect+=cpuTime()-time;
						}
					}
					wcsp->propagate();
					nb = sharpBTD(c);
					c->sgoodRec(0,nb);
					nbSGoods++;
				}
				catch(Contradiction){
					wcsp->whenContradiction();
					c->sgoodRec(0,0);// no solution
					nbSGoods++;
				}
				store->restore();

			}
			if(cluster->getParent()==NULL && ToulBar2::approximateCountingBTD){
				// computation of upper bound of solutions number for each part
				if(ubSol.find(c->getPart()) == ubSol.end()){
					ubSol[c->getPart()] = 1;
				}
				ubSol[c->getPart()]*=nb;

			}
			NbSol *= nb;

		}
		return NbSol;
	}
	else{
		// Enumerates cluster proper variables
		if (wcsp->enumerated(varIndex)) {
			assert(wcsp->canbe(varIndex, wcsp->getSupport(varIndex)));
			// Reuse last solution found if available
			Value bestval = wcsp->getBestValue(varIndex);

			NbSol = binaryChoicePointSBTD(cluster, varIndex, (wcsp->canbe(varIndex, bestval))?bestval:wcsp->getSupport(varIndex));
		}
		else {
			NbSol = binaryChoicePointSBTD(cluster, varIndex, wcsp->getInf(varIndex));
		}
		if (ToulBar2::verbose >= 1) cout << "[" << store->getDepth() << "] C" << cluster->getId() << " return " << NbSol << endl;
		return NbSol;
	}
}
