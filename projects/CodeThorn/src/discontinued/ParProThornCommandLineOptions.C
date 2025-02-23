#include "sage3basic.h"
#include "CodeThornException.h"
#include "ParProThornCommandLineOptions.h"
#include "CppStdUtilities.h"

#include <string>
#include <sstream>
#include <iostream>

// required for checking of: HAVE_SPOT, HAVE_Z3
#include "rose_config.h"

#include "Rose/Diagnostics.h"
using namespace Sawyer::Message;
using namespace std;
using namespace CodeThorn;

CodeThorn::CommandLineOptions& parseCommandLine(int argc, char* argv[], Sawyer::Message::Facility logger, std::string version,
                                                CodeThornOptions& ctOpt, LTLOptions& ltlOpt, ParProOptions& parProOpt) {

  // Command line option handling.
  po::options_description visibleOptions("Supported options");
  po::options_description hiddenOptions("Hidden options");
  po::options_description passOnToRoseOptions("Options passed on to ROSE frontend");
  po::options_description parallelProgramOptions("Analysis options for parallel programs");
  po::options_description experimentalOptions("Experimental options");
  po::options_description visualizationOptions("Visualization options");
  po::options_description infoOptions("Program information options");

  hiddenOptions.add_options()
    ("max-transitions-forced-top1",po::value< int >(&ctOpt.maxTransitionsForcedTop1)->default_value(-1),"Performs approximation after <arg> transitions (only exact for input,output).")
    ("max-transitions-forced-top2",po::value< int >(&ctOpt.maxTransitionsForcedTop2)->default_value(-1),"Performs approximation after <arg> transitions (only exact for input,output,df).")
    ("max-transitions-forced-top3",po::value< int >(&ctOpt.maxTransitionsForcedTop3)->default_value(-1),"Performs approximation after <arg> transitions (only exact for input,output,df,ptr-vars).")
    ("max-transitions-forced-top4",po::value< int >(&ctOpt.maxTransitionsForcedTop4)->default_value(-1),"Performs approximation after <arg> transitions (exact for all but inc-vars).")
    ("max-transitions-forced-top5",po::value< int >(&ctOpt.maxTransitionsForcedTop5)->default_value(-1),"Performs approximation after <arg> transitions (exact for input,output,df and vars with 0 to 2 assigned values)).")
    ("solver",po::value< int >(&ctOpt.solver)->default_value(5),"Set solver <arg> to use (one of 1,2,3,...).")
    ;

  passOnToRoseOptions.add_options()
    (",I", po::value< vector<string> >(&ctOpt.includeDirs),"Include directories.")
    (",D", po::value< vector<string> >(&ctOpt.preProcessorDefines),"Define constants for preprocessor.")
    ("edg:no_warnings", po::bool_switch(&ctOpt.edgNoWarningsFlag),"EDG frontend flag.")
    ("rose:ast:read", po::value<std::string>(&ctOpt.roseAstReadFileName),"read in binary AST from comma separated list (no spaces)")
    ;

  visualizationOptions.add_options()
    ("rw-clusters", po::value< bool >(&ctOpt.visualization.rwClusters)->default_value(false)->implicit_value(true), "Draw boxes around data elements from the same array (read/write-set graphs).")      
    ("rw-data", po::value< bool >(&ctOpt.visualization.rwData)->default_value(false)->implicit_value(true), "Display names of data elements (read/write-set graphs).") 
    ("rw-highlight-races", po::value< bool >(&ctOpt.visualization.rwHighlightRaces)->default_value(false)->implicit_value(true), "Highlight data races as large red dots (read/write-set graphs).") 
    ("dot-io-stg", po::value< string >(&ctOpt.visualization. dotIOStg), "Output STG with explicit I/O node information in dot file <arg>.")
    ("dot-io-stg-forced-top", po::value< string >(&ctOpt.visualization.dotIOStgForcedTop), "Output STG with explicit I/O node information in dot file <arg>. Groups abstract states together.")
    ("tg1-estate-address", po::value< bool >(&ctOpt.visualization.tg1EStateAddress)->default_value(false)->implicit_value(true), "Transition graph 1: Visualize address.")
    ("tg1-estate-id", po::value< bool >(&ctOpt.visualization.tg1EStateId)->default_value(true)->implicit_value(true), "Transition graph 1: Visualize estate-id.")
    ("tg1-estate-properties", po::value< bool >(&ctOpt.visualization.tg1EStateProperties)->default_value(true)->implicit_value(true), "Transition graph 1: Visualize all estate-properties.")
    ("tg1-estate-predicate", po::value< bool >(&ctOpt.visualization.tg1EStatePredicate)->default_value(false)->implicit_value(true), "Transition graph 1: Show estate as predicate.")
    ("tg1-estate-memory-subgraphs", po::value< bool >(&ctOpt.visualization.tg1EStateMemorySubgraphs)->default_value(false)->implicit_value(true), "Transition graph 1: Show estate as memory graphs.")
    ("tg2-estate-address", po::value< bool >(&ctOpt.visualization.tg2EStateAddress)->default_value(false)->implicit_value(true), "Transition graph 2: Visualize address.")
    ("tg2-estate-id", po::value< bool >(&ctOpt.visualization.tg2EStateId)->default_value(true)->implicit_value(true), "Transition graph 2: Visualize estate-id.")
    ("tg2-estate-properties", po::value< bool >(&ctOpt.visualization.tg2EStateProperties)->default_value(false)->implicit_value(true),"Transition graph 2: Visualize all estate-properties.")
    ("tg2-estate-predicate", po::value< bool >(&ctOpt.visualization.tg2EStatePredicate)->default_value(false)->implicit_value(true), "Transition graph 2: Show estate as predicate.")
    ("visualize-read-write-sets", po::value< bool >(&ctOpt.visualization.visualizeRWSets)->default_value(false)->implicit_value(true), "Generate a read/write-set graph that illustrates the read and write accesses of the involved threads.")
    ("vis", po::value< bool >(&ctOpt.visualization.vis)->default_value(false)->implicit_value(true),"Generate visualizations of AST, CFG, and transition system as dot files (ast.dot, cfg.dot, transitiongraph1/2.dot.")
    ("vis-tg2", po::value< bool >(&ctOpt.visualization.visTg2)->default_value(false)->implicit_value(true),"Generate transition graph 2 (.dot).")
    ("cfg", po::value< string >(&ctOpt.visualization.icfgFileName), "Generate inter-procedural cfg as dot file. Each function is visualized as one dot cluster.")
    ;

  parallelProgramOptions.add_options()
    ("seed",po::value< int >(&parProOpt.seed)->default_value(-1),"Seed value for randomly selected integers (concurrency-related non-determinism might still affect results).")
    ("generate-automata",po::value< string >(&parProOpt.generateAutomata),"Generate random control flow automata (file <arg>) that can be interpreted and analyzed as a parallel program.")
    ("num-automata",po::value< int >(&parProOpt.numAutomata)->default_value(-1),"Select the number of parallel automata to generate.")
    ("num-syncs-range",po::value< string >(&parProOpt.numSyncsRange),"Select a range for the number of random synchronizations between the generated automata (csv pair of integers).")
    ("num-circles-range",po::value< string >(&parProOpt.numCirclesRange),"Select a range for the number of circles that a randomly generated automaton consists of (csv pair of integers).")
    ("circle-length-range",po::value< string >(&parProOpt.circlesLengthRange),"Select a range for the length of circles that are used to construct an automaton (csv pair of integers).")
    ("num-intersections-range",po::value< string >(&parProOpt.numIntersectionsRange),"Select a range for the number of intersections of a newly added circle with existing circles in the automaton (csv pair of integers).")
    ("automata-dot-input",po::value< string >(&parProOpt.automataDotInput),"Reads in parallel automata with synchronized transitions from a given .dot file.")
    ("keep-systems", po::value< bool >(&parProOpt.keepSystems)->default_value(false)->implicit_value(true),"Store computed parallel systems (over- and under-approximated STGs) during exploration  so that they do not need to be recomputed.")
    ("use-components",po::value< string >(&parProOpt. useComponents),"Selects which parallel components are chosen for analyzing the (approximated) state space ([all] | subsets-fixed | subsets-random).")
    ("fixed-subsets",po::value< string >(&parProOpt.fixedSubsets),"A list of sets of parallel component IDs used for analysis (e.g. \"{1,2},{4,7}\"). Use only with \"--use-components=subsets-fixed\".")
    ("num-random-components",po::value< int >(&parProOpt.numRandomComponents)->default_value(-1),"Number of different random components used for the analysis. Use only with \"--use-components=subsets-random\". Default: min(3, <num-parallel-components>)")
    ("parallel-composition-only", po::value< bool >(&parProOpt.parallelCompositionOnly)->default_value(false)->implicit_value(true),"If set to \"yes\", then no approximation will take place. Instead, the parallel compositions of the respective sub-systems will be expanded (sequentialized). Skips any LTL analysis. ([yes|no])")
    ("num-components-ltl",po::value< int >(&parProOpt.numComponentsLtl)->default_value(-1),"Number of different random components used to generate a random LTL property. Default: value of option --num-random-components (a.k.a. all analyzed components)")
    ("minimum-components",po::value< int >(&parProOpt.minimumComponents)->default_value(-1),"Number of different parallel components that need to be explored together in order to be able to analyze the mined properties. (Default: 3).")
    ("different-component-subsets",po::value< int >(&parProOpt.differentComponentSubsets)->default_value(-1),"Number of random component subsets. The solver will be run for each of the random subsets. Use only with \"--use-components=subsets-random\" (Default: no termination).")
    ("ltl-mode",po::value< string >(&parProOpt.ltlMode),"\"check\" checks the properties passed to option \"--check-ltl=<filename>\". \"mine\" searches for automatically generated properties that adhere to certain criteria. \"none\" means no LTL analysis (default).")
    ("mine-num-verifiable",po::value< int >(&parProOpt.mineNumVerifiable)->default_value(-1),"Number of verifiable properties satisfying given requirements that should be collected (Default: 10).")
    ("mine-num-falsifiable",po::value< int >(&parProOpt.mineNumFalsifiable)->default_value(-1),"Number of falsifiable properties satisfying given requirements that should be collected (Default: 10).")
    ("minings-per-subsets",po::value< int >(&parProOpt.miningsPerSubset)->default_value(-1),"Number of randomly generated properties that are evaluated based on one subset of parallel components (Default: 50).")
    ("ltl-properties-output",po::value< string >(&parProOpt.ltlPropertiesOutput),"Writes the analyzed LTL properties to file <arg>.")
    ("promela-output",po::value< string >(&parProOpt.promelaOutput),"Writes a promela program reflecting the synchronized automata of option \"--automata-dot-input\" to file <arg>. Includes LTL properties if analyzed.")
    ("promela-output-only", po::value< bool >(&parProOpt.promelaOutputOnly)->default_value(false)->implicit_value(true),"Only generate Promela code, skip analysis of the input .dot graphs.")
    ("output-with-results", po::value< bool >(&parProOpt.outputWithResults)->default_value(false)->implicit_value(true),"Include results for the LTL properties in generated promela code and LTL property files .")
    ("output-with-annotations", po::value< bool >(&parProOpt.outputWithAnnotations)->default_value(false)->implicit_value(true),"Include annotations for the LTL properties in generated promela code and LTL property files .")
    ("verification-engine",po::value< string >(&parProOpt.verificationEngine),"Choose which backend verification engine is used (ltsmin|[spot]).")
    ;

  experimentalOptions.add_options()
    ("omp-ast", po::value< bool >(&ctOpt.ompAst)->default_value(false)->implicit_value(true),"Flag for using the OpenMP AST - useful when visualizing the ICFG.")
    ("normalize-level", po::value< int >(&ctOpt.normalizeLevel),"Normalize all expressions (2), only fcalls (1), turn off (0).")
    ("inline", po::value< bool >(&ctOpt.inlineFunctions)->default_value(false)->implicit_value(false),"inline functions before analysis .")
    ("inlinedepth",po::value< int >(&ctOpt.inlineFunctionsDepth)->default_value(10),"Default value is 10. A higher value inlines more levels of function calls.")
    ("annotate-terms", po::value< bool >(&ctOpt.annotateTerms)->default_value(false)->implicit_value(true),"Annotate term representation of expressions in unparsed program.")
    ("eliminate-stg-back-edges", po::value< bool >(&ctOpt.eliminateSTGBackEdges)->default_value(false)->implicit_value(true), "Eliminate STG back-edges (STG becomes a tree).")
    ("generate-assertions", po::value< bool >(&ctOpt.generateAssertions)->default_value(false)->implicit_value(true),"Generate assertions (pre-conditions) in program and output program (using ROSE unparser).")
    ("precision-exact-constraints", po::value< bool >(&ctOpt.precisionExactConstraints)->default_value(false)->implicit_value(true),"Use precise constraint extraction.")
    ("stg-trace-file", po::value< string >(&ctOpt.stgTraceFileName), "Generate STG computation trace and write to file <arg>.")

    ("arrays-not-in-state", po::value< bool >(&ctOpt.arraysNotInState)->default_value(false)->implicit_value(true),"Arrays are not represented in state. Only correct if all arrays are read-only (manual optimization - to be eliminated).")
    ("z3", po::value< bool >(&ctOpt.z3BasedReachabilityAnalysis)->default_value(false)->implicit_value(true), "RERS specific reachability analysis using z3.")	
    ("rers-upper-input-bound", po::value< int >(&ctOpt.z3UpperInputBound)->default_value(-1), "RERS specific parameter for z3.")
    ("rers-verifier-error-number",po::value< int >(&ctOpt.z3VerifierErrorNumber)->default_value(-1), "RERS specific parameter for z3.")
    ("ssa",  po::value< bool >(&ctOpt.ssa)->default_value(false)->implicit_value(true), "Generate SSA form (only works for programs without function calls, loops, jumps, pointers and returns).")
    ("program-stats-only",po::value< bool >(&ctOpt.programStatsOnly)->default_value(false)->implicit_value(true),"print some basic program statistics about used language constructs and exit.")
    ("in-state-string-literals",po::value< bool >(&ctOpt.inStateStringLiterals)->default_value(false)->implicit_value(true),"create string literals in initial state.")
    ("std-functions",po::value< bool >(&ctOpt.stdFunctions)->default_value(true)->implicit_value(true),"model std function semantics (malloc, memcpy, etc). Must be turned off explicitly.")
    ("ignore-function-pointers",po::value< bool >(&ctOpt.ignoreFunctionPointers)->default_value(false)->implicit_value(true), "Unknown functions are assumed to be side-effect free.")
    ("ignore-undefined-dereference",po::value< bool >(&ctOpt.ignoreUndefinedDereference)->default_value(false)->implicit_value(true), "Ignore pointer dereference of uninitalized value (assume data exists).")
    ("ignore-unknown-functions",po::value< bool >(&ctOpt.ignoreUnknownFunctions)->default_value(true)->implicit_value(true), "Ignore function pointers (functions are not called).")
    ("function-resolution-mode",po::value< int >(&ctOpt.functionResolutionMode)->default_value(4),"1:Translation unit only, 2:slow lookup, 3: -, 4: complete resolution (including function pointers)")
    ("context-sensitive",po::value< bool >(&ctOpt.contextSensitive)->default_value(true)->implicit_value(true),"Perform context sensitive analysis. Uses call strings with arbitrary length, recursion is not supported yet.")
    ("abstraction-mode",po::value< int >(&ctOpt.abstractionMode)->default_value(0),"Select abstraction mode (0: equality merge (explicit model checking), 1: approximating merge (abstract model checking).")
    ("print-warnings",po::value< bool >(&ctOpt.printWarnings)->default_value(false)->implicit_value(true),"Print warnings on stdout during analysis (this can slow down the analysis significantly)")
    ("callstring-length",po::value< int >(&ctOpt.callStringLength)->default_value(10),"Set the length of the callstring for context-sensitive analysis. Default value is 10.")
    ("unit-test-expr-analyzer", po::value< bool >(&ctOpt.exprEvalTest)->default_value(false)->implicit_value(true), "Run expr eval test (with input program).")
    ;

  visibleOptions.add_options()            
    ("config,c", po::value< string >(&ctOpt.configFileName), "Use the configuration specified in file <arg>.")
    ("colors", po::value< bool >(&ctOpt.colors)->default_value(true)->implicit_value(true),"Use colors in output.")
    ("csv-stats",po::value< string >(&ctOpt.csvStatsFileName),"Output statistics into a CSV file <arg>.")
    ("display-diff",po::value< int >(&ctOpt.displayDiff)->default_value(-1),"Print statistics every <arg> computed estates.")
    ("exploration-mode",po::value< string >(&ctOpt.explorationMode), "Set mode in which state space is explored. ([breadth-first]|depth-first|loop-aware|loop-aware-sync)")
    ("quiet", po::value< bool >(&ctOpt.quiet)->default_value(false)->implicit_value(true), "Produce no output on screen.")
    ("help,h", "Produce this help message.")
    ("help-all", "Show all help options.")
    ("help-rose", "Show options that can be passed to ROSE.")
    ("help-cegpra", "Show options for CEGRPA.")
    ("help-eq", "Show options for program equivalence checking.")
    ("help-exp", "Show options for experimental features.")
    ("help-pat", "Show options for pattern search mode.")
    ("help-svcomp", "Show options for SV-Comp specific features.")
    ("help-rers", "Show options for RERS specific features")
    ("help-ltl", "Show options for LTL verification.")
    ("help-par", "Show options for analyzing parallel programs.")
    ("help-vis", "Show options for visualization output files.")
    ("help-data-race", "Show options for data race detection.")
    ("help-info", "Show options for program info.")
    ("start-function", po::value< string >(&ctOpt.startFunctionName), "Name of function to start the analysis from.")
    ("status", po::value< bool >(&ctOpt.status)->default_value(false)->implicit_value(true), "Show status messages.")
    ("reduce-cfg", po::value< bool >(&ctOpt.reduceCfg)->default_value(true)->implicit_value(true), "Reduce CFG nodes that are irrelevant for the analysis.")
    ("internal-checks", po::value< bool >(&ctOpt.internalChecks)->default_value(false)->implicit_value(true), "Run internal consistency checks (without input program).")
    ("cl-args",po::value< string >(&ctOpt.analyzedProgramCLArgs),"Specify command line options for the analyzed program (as one quoted string).")
    ("input-values",po::value< string >(&ctOpt.inputValues),"Specify a set of input values. (e.g. \"{1,2,3}\")")
    ("input-values-as-constraints", po::value< bool >(&ctOpt.inputValuesAsConstraints)->default_value(false)->implicit_value(true),"Represent input var values as constraints (otherwise as constants in PState).")
    ("input-sequence",po::value< string >(&ctOpt.inputSequence),"Specify a sequence of input values. (e.g. \"[1,2,3]\")")
    ("log-level",po::value< string >(&ctOpt.logLevel)->default_value("none,>=error"),"Set the log level (\"x,>=y\" with x,y in: (none|info|warn|trace|error|fatal|debug)).")
    ("max-transitions",po::value< int >(&ctOpt.maxTransitions)->default_value(-1),"Passes (possibly) incomplete STG to verifier after <arg> transitions have been computed.")
    ("max-iterations",po::value< int >(&ctOpt.maxIterations)->default_value(-1),"Passes (possibly) incomplete STG to verifier after <arg> loop iterations have been explored. Currently requires --exploration-mode=loop-aware[-sync].")
    ("max-time",po::value< long int >(&ctOpt.maxTime)->default_value(-1),"Stop computing the STG after an analysis time of approximately <arg> seconds has been reached.")
    ("max-transitions-forced-top",po::value< int >(&ctOpt.maxTransitionsForcedTop)->default_value(-1),"Performs approximation after <arg> transitions.")
    ("max-iterations-forced-top",po::value< int >(&ctOpt.maxIterationsForcedTop)->default_value(-1),"Performs approximation after <arg> loop iterations. Currently requires --exploration-mode=loop-aware[-sync].")
    ("max-time-forced-top",po::value< long int >(&ctOpt.maxTimeForcedTop)->default_value(-1),"Performs approximation after an analysis time of approximately <arg> seconds has been reached.")
    ("resource-limit-diff",po::value< int >(&ctOpt. resourceLimitDiff)->default_value(-1),"Check if the resource limit is reached every <arg> computed estates.")
    ("rewrite",po::value< bool >(&ctOpt.rewrite)->default_value(false)->implicit_value(true),"Rewrite AST applying all rewrite system rules.")
    ("run-rose-tests",po::value< bool >(&ctOpt.runRoseAstChecks)->default_value(false)->implicit_value(true), "Run ROSE AST tests.")
    ("analyzed-functions-csv",po::value<std::string>(&ctOpt.analyzedFunctionsCSVFileName),"Write list of analyzed functions to CSV file [arg].")
    ("analyzed-files-csv",po::value<std::string>(&ctOpt.analyzedFilesCSVFileName),"Write list of analyzed files (with analyzed functions) to CSV file [arg].")
    ("threads",po::value< int >(&ctOpt.threads)->default_value(1),"(experimental) Run analyzer in parallel using <arg> threads.")
    ("unparse",po::value< bool >(&ctOpt.unparse)->default_value(false)->implicit_value(true),"unpare code (only relevant for inlining, normalization, and lowering)")
    ("version,v",po::value< bool >(&ctOpt.displayVersion)->default_value(false)->implicit_value(true), "Display the version of CodeThorn.")
    ;

  infoOptions.add_options()
    ("print-variable-id-mapping",po::value< bool >(&ctOpt.info.printVariableIdMapping)->default_value(false)->implicit_value(true),"Print variable-id-mapping on stdout.")
    ("ast-stats-print",po::value< bool >(&ctOpt.info.printAstNodeStats)->default_value(false)->implicit_value(true),"Print ast node statistics on stdout.")
    ("ast-stats-csv",po::value< string >(&ctOpt.info.astNodeStatsCSVFileName),"Write ast node statistics to CSV file [arg].")
    ("type-size-mapping-print",po::value< bool >(&ctOpt.info.printTypeSizeMapping)->default_value(false)->implicit_value(true),"Print type-size mapping on stdout.")
    ("type-size-mapping-csv",po::value<std::string>(&ctOpt.info.typeSizeMappingCSVFileName),"Write type-size mapping to CSV file [arg].")
    ;

  po::options_description all("All supported options");
  all.add(visibleOptions)
    .add(hiddenOptions)
    .add(passOnToRoseOptions)
    .add(parallelProgramOptions)
    .add(experimentalOptions)
    .add(visualizationOptions)
    .add(infoOptions)
    ;

  po::options_description configFileOptions("Configuration file options");
  configFileOptions.add(visibleOptions)
    .add(hiddenOptions)
    //    .add(passOnToRoseOptions) [cannot be used in config file]
    .add(parallelProgramOptions)
    .add(experimentalOptions)
    .add(visualizationOptions)
    .add(infoOptions)
    ;

  args.parse(argc,argv,all,configFileOptions);

  if (args.isUserProvided("help")) {
    cout << visibleOptions << "\n";
    exit(0);
  } else if(args.isUserProvided("help-all")) {
    cout << all << "\n";
    exit(0);
  } else if(args.isUserProvided("help-exp")) {
    cout << experimentalOptions << "\n";
    exit(0);
  } else if(args.isUserProvided("help-par")) {
    cout << parallelProgramOptions << "\n";
    exit(0);
  } else if(args.isUserProvided("help-rose")) {
    cout << passOnToRoseOptions << "\n";
    exit(0);
  } else if(args.isUserProvided("help-vis")) {
    cout << visualizationOptions << "\n";
    exit(0);
  } else if(args.isUserProvided("help-info")) {
    cout << infoOptions << "\n";
    exit(0);
  } else if(ctOpt.displayVersion) {
    cout << "CodeThorn version "<<version<<endl;
    cout << "Written by Markus Schordan, Marc Jasper, Simon Schroder, Maximilan Fecke, Joshua Asplund, Adrian Prantl\n";
    exit(0);
  }

  // Additional checks for options passed on to the ROSE frontend.
  // "-std" is a short option with long name. Check that it still has an argument if used.
  // deactivated  // "-I" should either be followed by a whitespace or by a slash
  for (int i=1; i < argc; ++i) {
    string currentArg(argv[i]);
    if (currentArg == "-std") {
      logger[ERROR] << "Option \"-std\" requires an argument." << endl;
      ROSE_ASSERT(0);
    }
  }

  // Remove all CodeThorn-specific elements of argv (do not confuse ROSE frontend)
  for (int i=1; i < argc; ++i) {
    string currentArg(argv[i]);
    if (currentArg == "--rose:ast:read"){
      argv[i] = strdup("");
      ROSE_ASSERT(i+1<argc);
      argv[i+1]= strdup("");
      continue;
    }
    if (currentArg[0] != '-' ){
      continue;  // not an option      
    }
    // explicitly keep options relevant to the ROSE frontend (white list) 
    else if (currentArg == "-I") {
      assert(i+1<argc);
      ++i;
      continue;
    } else if (currentArg == "--edg:no_warnings") {
      continue;
    } else {
      string iPrefix = "-I/";
      string dPrefix = "-D"; // special case, cannot contain separating space
      if(currentArg.substr(0, iPrefix.size()) == iPrefix) {
        continue;
      }
      if(currentArg.substr(0, dPrefix.size()) == dPrefix) {
        continue;
      }
    }
    // No match with elements in the white list above. 
    // Must be a CodeThorn option, therefore remove it from argv.
    argv[i] = strdup("");
  }

  return args;
}

