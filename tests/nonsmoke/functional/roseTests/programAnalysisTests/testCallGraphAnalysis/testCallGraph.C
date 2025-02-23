// DQ (10/14/2010):  This should only be included by source files that require it.
// This fixed a reported bug which caused conflicts with autoconf macros (e.g. PACKAGE_BUGREPORT).
// Interestingly it must be at the top of the list of include files.
#include "rose_config.h"
#undef CONFIG_ROSE /* prevent error about including both private and public headers; must be between rose_config.h and rose.h */

// DQ (8/15/2009): This is a call graph test program that I have 
// modified from CG.C to generate tests more specific my own 
// requirements for testing.

#include "rose.h"
#include <GraphAccess.h>
#include <GraphUpdate.h>
#include <CallGraph.h>

using namespace std;

std::string edgeLabel(SgDirectedGraphEdge* edge)
{
    return "some label";
};

std::string nodeLabel(SgGraphNode* node)
{
    return "some label";
};

/**
 *  getTrueFilePath
 *
 *  Get the path of the file containing the provided Declaration
 *  This is a special case to deal with when a function is defined as a 
 *  template.  Instantiated templates are always compilerGenerated, so have
 *  no file.  So we have to get the associated template and get its file.
 *
 * \param[in] funcDecl: A function declaration to get the filename of
 *
 **/
std::string getTrueFilePath(SgFunctionDeclaration* inFuncDecl) {
  //We need to figure out where the function originates, but that's different if it's a template so handle both cases
  std::string filepathStr;
  SgTemplateInstantiationFunctionDecl* tempIDecl = isSgTemplateInstantiationFunctionDecl(inFuncDecl);
  SgTemplateInstantiationMemberFunctionDecl* tempIMDecl = isSgTemplateInstantiationMemberFunctionDecl(inFuncDecl);
  if(tempIDecl) {  //is template
    SgTemplateFunctionDeclaration* tempDecl = isSgTemplateFunctionDeclaration(tempIDecl->get_templateDeclaration());
    SgFunctionDeclaration* defFuncDecl = isSgTemplateFunctionDeclaration(tempDecl->get_definingDeclaration());
    filepathStr = defFuncDecl->get_file_info()->get_filename();
  } else if(tempIMDecl) { //is template member
    SgTemplateMemberFunctionDeclaration* tempDecl = isSgTemplateMemberFunctionDeclaration(tempIMDecl->get_templateDeclaration());
    SgFunctionDeclaration* defFuncDecl = isSgTemplateMemberFunctionDeclaration(tempDecl->get_definingDeclaration());
    filepathStr = defFuncDecl->get_file_info()->get_filename();
  } else { //is NOT template
      SgFunctionDeclaration* defDecl = isSgFunctionDeclaration(inFuncDecl->get_definingDeclaration());
      if(defDecl == NULL) {
          return "";
      }
      filepathStr = defDecl->get_file_info()->get_filename();
  }
  boost::filesystem::path filePath = boost::filesystem::weakly_canonical(filepathStr);  
  return filePath.native();

}


struct OnlyCurrentDirectory : public std::unary_function<bool, SgFunctionDeclaration*>
{

    bool operator() (SgFunctionDeclaration * node) const
    {
        std::string stringToFilter = ROSE_COMPILE_TREE_PATH + std::string("/tests");
        std::string srcDir = ROSE_AUTOMAKE_TOP_SRCDIR;

//#if 0
        printf("stringToFilter = %s %d\n", stringToFilter.c_str(), stringToFilter.size());
        printf("srcDir         = %s \n", srcDir.c_str());
//#endif

        string sourceFilename = getTrueFilePath(node);
        string sourceFilenameSubstring = sourceFilename.substr(0, stringToFilter.size());
        string sourceFilenameSrcdirSubstring = sourceFilename.substr(0, srcDir.size());
//#if 0
        printf("functionName                  = %s \n", node->get_qualified_name().getString().c_str());
        printf("sourceFilename                = %s \n", sourceFilename.c_str());
        printf("sourceFilenameSubstring       = %s \n", sourceFilenameSubstring.c_str());
        printf("sourceFilenameSrcdirSubstring = %s \n", sourceFilenameSrcdirSubstring.c_str());
//#endif
        // if (string(node->get_file_info()->get_filename()).substr(0,stringToFilter.size()) == stringToFilter  )
        if (sourceFilenameSubstring == stringToFilter)
            return true;
//        else
            // if ( string(node->get_file_info()->get_filename()).substr(0,srcDir.size()) == srcDir )
//            if (sourceFilenameSrcdirSubstring == srcDir)
//            return true;
        else
            return false;
    }
};

struct OnlyNonCompilerGenerated : public std::unary_function<bool, SgFunctionDeclaration*>
{

    bool operator() (SgFunctionDeclaration * node) const
    {
        // false will filter out ALL nodes
        bool filterNode = true;

        SgFunctionDeclaration *fct = isSgFunctionDeclaration(node);
        if (fct != NULL)
        {
            bool ignoreFunction = (fct->get_file_info()->isCompilerGenerated() == true);
            if (ignoreFunction == true)
                filterNode = false;
        }

        return filterNode;
    }
};

class SecurityVulnerabilityAttribute : public AstAttribute
{
public:
    SgDirectedGraphEdge* edge;
    SgGraphNode* node;

    SecurityVulnerabilityAttribute(SgGraphNode*);
    SecurityVulnerabilityAttribute(SgDirectedGraphEdge*);
    virtual std::vector<AstAttribute::AttributeEdgeInfo> additionalEdgeInfo();
    virtual std::vector<AstAttribute::AttributeNodeInfo> additionalNodeInfo();
};

SecurityVulnerabilityAttribute::SecurityVulnerabilityAttribute(SgGraphNode* n)
: edge(NULL), node(n) { }

SecurityVulnerabilityAttribute::SecurityVulnerabilityAttribute(SgDirectedGraphEdge* e)
: edge(e), node(NULL) { }

vector<AstAttribute::AttributeEdgeInfo>
SecurityVulnerabilityAttribute::additionalEdgeInfo()
{
    vector<AstAttribute::AttributeEdgeInfo> v;

    if (edge != NULL)
    {
        string vulnerabilityName = edge->get_name();
        string vulnerabilityColor = "green";
        string vulnerabilityOptions = " arrowsize=2.0 style=\"setlinewidth(7)\" constraint=false fillcolor=" + vulnerabilityColor + ",style=filled ";

        // AstAttribute::AttributeNodeInfo vulnerabilityNode ( (SgNode*) vulnerabilityPointer, "SecurityVulnerabilityAttribute"," fillcolor=\"red\",style=filled ");
        AstAttribute::AttributeEdgeInfo vulnerabilityNode(edge->get_from(), edge->get_to(), vulnerabilityName, vulnerabilityOptions);
        v.push_back(vulnerabilityNode);
    }

    return v;
}

std::vector<AstAttribute::AttributeNodeInfo>
SecurityVulnerabilityAttribute::additionalNodeInfo()
{
    vector<AstAttribute::AttributeNodeInfo> v;
    if (node != NULL)
    {
        string vulnerabilityName = node->get_name();
        string vulnerabilityColor = "blue";
        string vulnerabilityOptions = " arrowsize=2.0 style=\"setlinewidth(7)\" constraint=false fillcolor=" + vulnerabilityColor + ",style=filled ";

        // AstAttribute::AttributeNodeInfo vulnerabilityNode ( (SgNode*) vulnerabilityPointer, "SecurityVulnerabilityAttribute"," fillcolor=\"red\",style=filled ");
        AstAttribute::AttributeNodeInfo vulnerabilityNode(node, vulnerabilityName, vulnerabilityOptions);
        v.push_back(vulnerabilityNode);
    }

    return v;
}

int
main(int argc, char * argv[])
{
    std::cout << "Analyzing outside DATABASE" << std::endl;

    SgProject* project = new SgProject(argc, argv);

    CallGraphBuilder CGBuilder(project);
    CGBuilder.buildCallGraph(OnlyCurrentDirectory());
    SgIncidenceDirectedGraph *newGraph = CGBuilder.getGraph();
    
    ClassHierarchyWrapper hier(project);

 
    // Use the information in the graph to output a dot file for the call graph
    // CallGraphDotOutput output( *(CGBuilder.getGraph()) );
    
    // Generate a filename for this whole project (even if it has more than one file)
    string generatedProjectName = SageInterface::generateProjectName(project);

    printf("generatedProjectName            = %s \n", generatedProjectName.c_str());
    printf("project->get_outputFileName()   = %s \n", project->get_outputFileName().c_str());
    printf("project->get_dataBaseFilename() = %s \n", project->get_dataBaseFilename().c_str());

    // DQ (7/12/2009): Modified to use string instead of ostringstream.
    string uncoloredFileName = generatedProjectName + "_callgraph.dot";
    string coloredFileName = generatedProjectName + "_colored_callgraph.dot";

    // Example of how to generate a modified Call Graph
    //  OutputDot::writeToDOTFile(newGraph, coloredFileName.c_str(),"Incidence Graph", nodeLabel,edgeLabel );

    // This generated the dot file for the AST.
    generateDOT(*project);

    // This generates the dot file for the Call Graph
    {
        AstDOTGeneration dotgen;
        dotgen.writeIncidenceGraphToDOTFile(newGraph, uncoloredFileName.c_str());
    }

    // Generate colored graph
    rose_graph_integer_node_hash_map & nodes = newGraph->get_node_index_to_node_map();

    for (rose_graph_integer_node_hash_map::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        SgGraphNode* node = it->second;
        AstAttribute* newAttribute = new SecurityVulnerabilityAttribute(node);
        ROSE_ASSERT(newAttribute != NULL);

        node->addNewAttribute("SecurityVulnerabilityAttribute", newAttribute);
    }
    {
        AstDOTGeneration dotgen;
        dotgen.writeIncidenceGraphToDOTFile(newGraph, coloredFileName.c_str());
    }

    cout << "Done with DOT\n";

    printf("\nLeaving main program ... \n");

    return 0;
}
