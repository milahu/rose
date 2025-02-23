
#ifndef ROSE_CODEGEN_API_TXX
#define ROSE_CODEGEN_API_TXX

#include "sage3basic.h"

#include <iostream>

namespace Rose { namespace CodeGen {

template <typename CRT>
void API<CRT>::load(Driver & driver) {
  load_headers(driver);
  load_api(driver);
}

template <typename CRT>
void API<CRT>::display(std::ostream & out) const {
  out << "API of " << name << " :" << std::endl;
  out << "  Namespaces:" << std::endl;
  for (auto p: namespaces) {
    if (((CRT*)this)->*(p.second)) {
      out << "    " << p.first << " = (" << (((CRT*)this)->*(p.second))->class_name() << " *)" << std::hex << ((CRT*)this)->*(p.second) << std::endl;
    } else {
      out << "    " << p.first << " = Not Found" << std::endl;
    }
  }

  out << "  Classes:" << std::endl;
  for (auto p: classes) {
    if (((CRT*)this)->*(p.second)) {
      out << "    " << p.first << " = (" << (((CRT*)this)->*(p.second))->class_name() << " *)" << std::hex << ((CRT*)this)->*(p.second) << std::endl;
    } else {
      out << "    " << p.first << " = Not Found" << std::endl;
    }
  }

  out << "  Typedefs:" << std::endl;
  for (auto p: typedefs) {
    if (((CRT*)this)->*(p.second)) {
      out << "    " << p.first << " = (" << (((CRT*)this)->*(p.second))->class_name() << " *)" << std::hex << ((CRT*)this)->*(p.second) << std::endl;
    } else {
      out << "    " << p.first << " = Not Found" << std::endl;
    }
  }

  out << "  Variables:" << std::endl;
  for (auto p: variables) {
    if (((CRT*)this)->*(p.second)) {
      out << "    " << p.first << " = (" << (((CRT*)this)->*(p.second))->class_name() << " *)" << std::hex << ((CRT*)this)->*(p.second) << std::endl;
    } else {
      out << "    " << p.first << " = Not Found" << std::endl;
    }
  }

  out << "  Functions:" << std::endl;
  for (auto p: functions) {
    if (((CRT*)this)->*(p.second)) {
      out << "    " << p.first << " = (" << (((CRT*)this)->*(p.second))->class_name() << " *)" << std::hex << ((CRT*)this)->*(p.second) << std::endl;
    } else {
      out << "    " << p.first << " = Not Found" << std::endl;
    }
  }
}

template <typename CRT>
void API<CRT>::build_command_line(std::vector<std::string> & cmdline) const {
  cmdline.push_back("rose-cxx");
  cmdline.push_back("-c");

  for (auto path: paths) {
    if (!boost::filesystem::exists( path )) {
      std::cerr << "[WARN] Path to API header files does not exist: " << path << std::endl;
    } else {
      cmdline.push_back("-I" + path);
    }
  }

  for (auto flag: flags) {
    cmdline.push_back(flag);
  }
}

template <typename CRT>
void API<CRT>::add_nodes_for_namequal(Driver & driver, SgSourceFile * srcfile) const {
  auto & extra_nodes_for_namequal_init = srcfile->get_extra_nodes_for_namequal_init();
  for (auto fid: file_ids) {
    extra_nodes_for_namequal_init.push_back(driver.getGlobalScope(fid));
  }
}

template <typename CRT>
void API<CRT>::load_headers(Driver & driver) {
  if (!CommandlineProcessing::isCppFileNameSuffix("hxx")) {
    CommandlineProcessing::extraCppSourceFileSuffixes.push_back("hxx");
  }

  std::vector<std::string> old_cmdline = driver.project->get_originalCommandLineArgumentList();
  std::vector<std::string> cmdline;
  build_command_line(cmdline);
  driver.project->set_originalCommandLineArgumentList(cmdline);

  for (auto file: files) {
    bool found = false;
    if (!cache.empty()) {
      // TODO Try to get AST file from cache directory else build AST file
      // TODO Need "driver.readAST(fp);" which could return a set of file_id...
    }

    if (!found) {
      for (auto path: paths) {
        auto fp = path + "/" + file;
        if (boost::filesystem::exists(fp)) {
          file_ids.insert(driver.add(fp));
          found = true;
          break;
        }
      }
    }

    if (!found) {
      std::cerr << "[WARN] API header file not found: " << file << std::endl;
    }
  }

  driver.project->set_originalCommandLineArgumentList(old_cmdline);

  ROSE_ASSERT(SageBuilder::topScopeStack() == NULL); // Sanity check
}

template <typename API>
struct SymbolScanner : public ROSE_VisitTraversal {
  API & api;

  SymbolScanner(API & api_) : api(api_) {}

  template <typename SymT>
  void visit(SymT * sym, std::map<std::string, SymT * API::* > const & objmap) {
    auto str = sym->get_declaration()->get_qualified_name().getString();
    auto it = objmap.find(str);
    if (it != objmap.end()) {
      api.*(it->second) = sym;
    }
  }

  void visit(SgNode * node) {
    switch (node->variantT()) {
      case V_SgNamespaceSymbol:              visit<SgNamespaceSymbol> ( (SgNamespaceSymbol*)node, API::namespaces); break;

      case V_SgClassSymbol:                  visit<SgClassSymbol>     ( (SgClassSymbol*)node,     API::classes);    break;
      case V_SgTemplateClassSymbol:          visit<SgClassSymbol>     ( (SgClassSymbol*)node,     API::classes);    break;

      case V_SgTypedefSymbol:                visit<SgTypedefSymbol>   ( (SgTypedefSymbol*)node,   API::typedefs);   break;
      case V_SgTemplateTypedefSymbol:        visit<SgTypedefSymbol>   ( (SgTypedefSymbol*)node,   API::typedefs);   break;

      case V_SgVariableSymbol:               visit<SgVariableSymbol>  ( (SgVariableSymbol*)node,  API::variables);  break;
      case V_SgTemplateVariableSymbol:       visit<SgVariableSymbol>  ( (SgVariableSymbol*)node,  API::variables);  break;

      case V_SgFunctionSymbol:               visit<SgFunctionSymbol>  ( (SgFunctionSymbol*)node,  API::functions);  break;
      case V_SgMemberFunctionSymbol:         visit<SgFunctionSymbol>  ( (SgFunctionSymbol*)node,  API::functions);  break;
      case V_SgTemplateFunctionSymbol:       visit<SgFunctionSymbol>  ( (SgFunctionSymbol*)node,  API::functions);  break;
      case V_SgTemplateMemberFunctionSymbol: visit<SgFunctionSymbol>  ( (SgFunctionSymbol*)node,  API::functions);  break;

      default:
	      ROSE_ABORT();
    }
  }
};

template <typename CRT>
void API<CRT>::load_api(Driver & driver) {
  SymbolScanner<CRT> scanner(*(CRT*)this);

  SgNamespaceSymbol::traverseMemoryPoolNodes(scanner);

  SgClassSymbol::traverseMemoryPoolNodes(scanner);
  SgTemplateClassSymbol::traverseMemoryPoolNodes(scanner);

  SgTypedefSymbol::traverseMemoryPoolNodes(scanner);
  SgTemplateTypedefSymbol::traverseMemoryPoolNodes(scanner);

  SgVariableSymbol::traverseMemoryPoolNodes(scanner);
  SgTemplateVariableSymbol::traverseMemoryPoolNodes(scanner);

  SgFunctionSymbol::traverseMemoryPoolNodes(scanner);
  SgMemberFunctionSymbol::traverseMemoryPoolNodes(scanner);
  SgTemplateFunctionSymbol::traverseMemoryPoolNodes(scanner);
  SgTemplateMemberFunctionSymbol::traverseMemoryPoolNodes(scanner);
}

} }

#endif /* ROSE_CODEGEN_API_TXX */

