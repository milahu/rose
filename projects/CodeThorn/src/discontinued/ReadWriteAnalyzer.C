#include "ReadWriteAnalyzer.h"
#include "AstUtility.h"
#include "AstNodeInfo.h"
#include "CollectionOperators.h"
#include "CodeThornException.h"
#include "DataRaceDetection.h"
#include "CodeThornCommandLineOptions.h"

#include <omp.h>

using namespace CodeThorn;
using namespace std;
using namespace Sawyer::Message;

Sawyer::Message::Facility ReadWriteAnalyzer::logger;

void ReadWriteAnalyzer::initDiagnostics() {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    logger = Sawyer::Message::Facility("CodeThorn::ReadWriteAnalyzer", Rose::Diagnostics::destination);
    Rose::Diagnostics::mfacilities.insertAndAdjust(logger);
  }
}

void ReadWriteAnalyzer::runAnalysisPhase1Sub1(std::string functionToStartAt,SgProject* root, TimingCollector& tc) {

  ROSE_ASSERT(false);
  IOAnalyzer::runAnalysisPhase1Sub1(functionToStartAt,root,tc);
  /////////////////////////////////////////////////////////////////////

#if 0
  ReadWriteHistory initialRWHistory;
  const ReadWriteHistory* initRWHistoryPointer = rWHistorySet.processNew(initialRWHistory);
  ROSE_ASSERT(initRWHistoryPointer);

  RWState initialRWState(currentEState, initRWHistoryPointer);
  const RWState* initRWStatePointer = rWStateSet.processNew(initialRWState);
  ROSE_ASSERT(initRWStatePointer);

  addToWorkList(initRWStatePointer);

  logger[TRACE]<< "INIT: finished."<<endl;
#endif
}

void ReadWriteAnalyzer::addToWorkList(const RWState* state) {
  ROSE_ASSERT(state);
#pragma omp critical(WORKLIST)
  {
    workList.push_back(state);
  }
}

RWStateSet::ProcessingResult ReadWriteAnalyzer::process(RWState& state) {
  return rWStateSet.process(state);
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
void ReadWriteAnalyzer::runSolver() {
  ROSE_ASSERT(workList.size() == 1);
  ReadWriteAnalyzer* _analyzer = this;  // simplifies a later move to its own class
  size_t prevStateSetSize=0; // force immediate report at start
  int threadNum;
  int workers=_analyzer->_numberOfThreadsToUse;
  vector<bool> workVector(workers);
  set_finished(workVector,true);
  bool terminateEarly=false;
  omp_set_num_threads(workers);

  logger[TRACE]<<"STATUS: Running data race detection solver with "<<workers<<" threads."<<endl;
  printStatusMessage(true);
# pragma omp parallel shared(workVector) private(threadNum)
  {
    threadNum=omp_get_thread_num();
    while(!_analyzer->all_false(workVector)) {
      if(threadNum==0 && _analyzer->_displayDiff && (_analyzer->estateSet.size()>(prevStateSetSize+_analyzer->_displayDiff))) {
        _analyzer->printStatusMessage(true);
        prevStateSetSize=_analyzer->estateSet.size();
      }
      if(_analyzer->isEmptyWorkList()) {
#pragma omp critical
        {
          workVector[threadNum]=false;
        }
        continue;
      } else {
#pragma omp critical
        {
          if(terminateEarly)
            workVector[threadNum]=false;
          else
            workVector[threadNum]=true;
        }
      }
      const RWState* currentStatePtr=popWorkList();
      // if we want to terminate early, we ensure to stop all threads and empty the worklist (e.g. verification error found).
      if(terminateEarly)
        continue;
      if(!currentStatePtr) {
        ROSE_ASSERT(threadNum>=0 && threadNum<=_analyzer->_numberOfThreadsToUse);
      } else {
        ROSE_ASSERT(currentStatePtr);
        Flow edgeSet=getFlow()->outEdges(currentStatePtr->eState()->label());
        for(Flow::iterator i=edgeSet.begin();i!=edgeSet.end();++i) {
          Edge e=*i;
	  list<RWState> newStateList = bigStep(currentStatePtr);
	  for(list<RWState>::iterator nesListIter=newStateList.begin();
              nesListIter!=newStateList.end();
              ++nesListIter) {
            // newState is passed by value (not created yet)
            RWState newState=move(*nesListIter);
            ROSE_ASSERT(newState.eState()->label()!=Labeler::NO_LABEL);
	    RWStateSet::ProcessingResult pres=process(newState);
	    const RWState* newStatePtr=pres.second;
	    if(pres.first==true) {
	      _analyzer->addToWorkList(newStatePtr);
	    }
          } // end of loop on transfer function return-states
        } // edge set iterator
      } // conditional: test if work is available
    } // while
  } // omp parallel
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
list<RWState> ReadWriteAnalyzer::bigStep(const RWState* state) {
  RWStateSet visited; // to detect self loops without outgoing branches in the (imaginary) STG
  RWState tempState = *state;
  const RWState* tempStatePtr = visited.processNew(tempState);
  ROSE_ASSERT(tempStatePtr);
  list<RWState> successors;
  successors = transfer(tempState);
  while (successors.size() == 1) {
    RWStateSet::ProcessingResult pres = visited.process(*successors.begin());
    if(pres.first==false) { // Single successor state already visited locally: Done
      return list<RWState>();
    } else {
      tempState=move(*successors.begin());
      successors = transfer(tempState);
    }
  }
  return successors;
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
list<RWState> ReadWriteAnalyzer::transfer(RWState& state) {
  list<RWState> result;
  Flow edgeSet=getFlow()->outEdges(state.eState()->label());
  for(Flow::iterator i=edgeSet.begin();i!=edgeSet.end();++i) {
    Edge edge = move(*i);
    list<EState> successors = transferEdgeEState(edge, state.eState()); //TODO: return type will change
    for (list<EState>::iterator k=successors.begin(); k!=successors.end(); ++k) { //TODO: different iterator type
      EState succEState = move(*k); //TODO: *k will become a triple, extract appropriately
      MemLocAccessSet reads; //TODO: get from "successors"
      MemLocAccessSet writes; //TODO: get from "successors"
      EStateSet::ProcessingResult presEState = estateSet.process(succEState);
      const EState* succEStatePtr = presEState.second;
      ReadWriteHistory rWHistory = *state.readWriteHistory(); // will be modified, therefore copy by value
      bool dataRaceFound = updateAndCheckForDataRaces(rWHistory, state.eState(), reads, writes);
      if (dataRaceFound) {
	throw CodeThorn::Exception("DATA RACE FOUND!");
      }
      RWHistorySet::ProcessingResult presRWHistory = rWHistorySet.process(rWHistory);
      const ReadWriteHistory* succRWHistoryPtr = presRWHistory.second;
      RWState succRWState(succEStatePtr,succRWHistoryPtr);
      result.push_back(succRWState);
    }
  }
  return result;
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
bool ReadWriteAnalyzer::updateAndCheckForDataRaces(ReadWriteHistory& history, const EState* eState,
						   MemLocAccessSet& reads, MemLocAccessSet& writes) {
  ReadWriteHistory::StackOfRWBlocks* stack = history.stackOfUnorderedBlocks();
  if (isEndOfUnorderedExecutionBlock(eState)) {
    ROSE_ASSERT(!stack->empty());  // must be within an unordered execution block
    // Check of current unordered execution block is now completed.
    // No data race within this block, otherwise this function would have
    // returned "true" already.
    ReadsWritesUnorderedBlock unorderedBlockRWs = stack->top();
    stack->pop();
    // merge reads and writes with ordered execution block 
    // that just the finished unordered block was spawned in (if existing)
    if (!stack->empty()) {
      stack->top().currentReads.insert(unorderedBlockRWs.previousReads.begin(), 
					unorderedBlockRWs.previousReads.end());
      stack->top().currentWrites.insert(unorderedBlockRWs.previousWrites.begin(), 
					unorderedBlockRWs.previousWrites.end());
    }
  }
  if (isBeginningOfUnorderedExecutionBlock(eState)) {
    // Need new write and read sets for the just entered unordered execution block.
    stack->push(ReadsWritesUnorderedBlock());
  }
  if (!stack->empty()) {
    // Check if identical code is executed by multiple workers: 
    // If yes and contains writes, then data race found
    if (!writes.empty() && isExecutedByMultipleWorkers(eState)) {
      return true; // data race found      
    }
    // Within at least one unordered execution block: 
    // Accumulate reads and writes for current ordered execution block.
    stack->top().currentWrites.insert(writes.begin(), writes.end());
    stack->top().currentReads.insert(reads.begin(), reads.end());
  }
  if (isEndOfOrderedExecutionBlock(eState)) {
    // Check for data races within previously explored part of 
    // most recently entered unordered execution block.
    // Note: Order of operands of '*' (a.k.a. intersection) matters for performance reasons
    // TODO: Maybe move namespace of operator '*'? 
    if (!CodeThorn::operator*(stack->top().currentWrites, stack->top().previousWrites).empty() ||
	!CodeThorn::operator*(stack->top().currentWrites, stack->top().previousReads).empty() ||
	!CodeThorn::operator*(stack->top().currentReads, stack->top().previousWrites).empty()) {
      return true; // data race found
    }
    // Accumulate reads and writes from just finished ordered execution block
    // in the respective sets that collect all accesses from previous ordered blocks (implies a merge 
    // with previous accesses within currently checked unordered execution block)
    stack->top().previousReads.insert(stack->top().currentReads.begin(), stack->top().currentReads.end());
    stack->top().currentReads.clear();
    stack->top().previousWrites.insert(stack->top().currentWrites.begin(), stack->top().currentWrites.end());
    stack->top().currentWrites.clear();
  }
  return false;
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
bool ReadWriteAnalyzer::isBeginningOfUnorderedExecutionBlock(const EState* eState) const {
  return isBeginningOfOmpParallel(eState) || isOmpWorkShareBarrier(eState);
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
bool ReadWriteAnalyzer::isEndOfUnorderedExecutionBlock(const EState* eState) const {
  return isEndOfOmpParallel(eState) || isOmpWorkShareBarrier(eState);
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
bool ReadWriteAnalyzer::isEndOfOrderedExecutionBlock(const EState* eState) const {
  return (isEndOfOmpParallelLoopIteration(eState)
	  || isEndOfOmpSingle(eState)
	  || isEndOfOmpSection(eState));
}

/*! 
  * \author Marc Jasper
  * \date 2017
 */
bool ReadWriteAnalyzer::isExecutedByMultipleWorkers(const EState* eState) const {
  // Note: Currently only called when within an unordered execution block and
  //       therefore within an OpenMP parallel region. Otherwise this would have
  //       to be checked when determining the return value.
  return (!isInsideOmpLoop(eState)
	  && !isInsideOmpSingle(eState)
	  && !isInsideOmpSection(eState));
}

bool ReadWriteAnalyzer::isBeginningOfOmpParallel(const EState* eState) const {
  // TODO (MJ, 09/26/2017): Needs to be implemented.
  // label should be sufficient for determining the return value (if explicitly represented in the CFG)
  return false;
}

bool ReadWriteAnalyzer::isEndOfOmpParallel(const EState* eState) const {
  // TODO (MJ, 09/29/2017): Needs to be implemented.
  // label should be sufficient for determining the return value (if explicitly represented in the CFG)
  return false;
}

bool ReadWriteAnalyzer::isOmpWorkShareBarrier(const EState* eState) const {
  // TODO (MJ, 09/29/2017): Needs to be implemented.
  // label should be sufficient for determining the return value (if explicitly represented in the CFG)
  return false;
}

bool ReadWriteAnalyzer::isInsideOmpLoop(const EState* eState) const {
  // TODO (MJ, 10/11/2017): Needs to be implemented.
  // parallel context (current OpenMP scopes) should be sufficient for determining the return value
  return false;
}

bool ReadWriteAnalyzer::isEndOfOmpParallelLoopIteration(const EState* eState) const {
  // TODO (MJ, 09/26/2017): Needs to be implemented.
  return false;
}

bool ReadWriteAnalyzer::isInsideOmpSingle(const EState* eState) const {
  // TODO (MJ, 10/11/2017): Needs to be implemented.
  // parallel context (current OpenMP scopes) should be sufficient for determining the return value
  return false;
}

bool ReadWriteAnalyzer::isEndOfOmpSingle(const EState* eState) const {
  // TODO (MJ, 09/29/2017): Needs to be implemented.
  return false;
}

bool ReadWriteAnalyzer::isInsideOmpSection(const EState* eState) const {
  // TODO (MJ, 10/11/2017): Needs to be implemented.
  // parallel context (current OpenMP scopes) should be sufficient for determining the return value
  return false;
}

bool ReadWriteAnalyzer::isEndOfOmpSection(const EState* eState) const {
  // TODO (MJ, 09/29/2017): Needs to be implemented.
  return false;
}

bool ReadWriteAnalyzer::isEmptyWorkList() {
  bool res;
#pragma omp critical(WORKLIST)
  {
    res=workList.empty();
  }
  return res;
}

const RWState* ReadWriteAnalyzer::topWorkList() {
  const RWState* state=0;
#pragma omp critical(WORKLIST)
  {
    if(!workList.empty())
      state=*workList.begin();
  }
  return state;
}

const RWState* ReadWriteAnalyzer::popWorkList() {
  const RWState* state=0;
#pragma omp critical(WORKLIST)
  {
    if(!workList.empty())
      state=*workList.begin();
    if(state)
      workList.pop_front();
  }
  return state;
}
