#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <sage3basic.h>
#include <Rose/BinaryAnalysis/ModelChecker/SemanticCallbacks.h>

#include <Rose/BinaryAnalysis/ModelChecker/ExecutionUnit.h>
#include <Rose/BinaryAnalysis/ModelChecker/Settings.h>
#include <Rose/BinaryAnalysis/ModelChecker/Tag.h>
#include <Rose/BinaryAnalysis/InstructionSemantics2/BaseSemantics/State.h>
#include <Rose/BinaryAnalysis/InstructionSemantics2/SymbolicSemantics.h>

using namespace Sawyer::Message::Common;
namespace BS = Rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics;
namespace IS = Rose::BinaryAnalysis::InstructionSemantics2;
namespace P2 = Rose::BinaryAnalysis::Partitioner2;

namespace Rose {
namespace BinaryAnalysis {
namespace ModelChecker {

SemanticCallbacks::SemanticCallbacks(const Settings::Ptr &settings)
    : mcSettings_(settings) {
    ASSERT_not_null(settings);
}

SemanticCallbacks::~SemanticCallbacks() {}

Settings::Ptr
SemanticCallbacks::mcSettings() const {
    // no lock necessary since the pointer is const
    return mcSettings_;
}

BS::SValue::Ptr
SemanticCallbacks::protoval() {
    return IS::SymbolicSemantics::SValue::instance();
}

BS::State::Ptr
SemanticCallbacks::createInitialState() {
    BS::RegisterState::Ptr registers = createInitialRegisters();
    BS::MemoryState::Ptr memory = createInitialMemory();
    return BS::State::instance(registers, memory);
}

void
SemanticCallbacks::initializeState(const BS::RiscOperators::Ptr&) {}

std::vector<Tag::Ptr>
SemanticCallbacks::preExecute(const ExecutionUnit::Ptr&, const BS::RiscOperators::Ptr&) {
    return {};
}

std::vector<Tag::Ptr>
SemanticCallbacks::postExecute(const ExecutionUnit::Ptr&, const BS::RiscOperators::Ptr&) {
    return {};
}

SemanticCallbacks::CodeAddresses
SemanticCallbacks::nextCodeAddresses(const BS::RiscOperators::Ptr &ops) {
    ASSERT_not_null(ops);
    CodeAddresses retval;
    retval.ip = instructionPointer(ops);
    retval.isComplete = true;
    SAWYER_MESG(mlog[DEBUG]) <<"    finding next code addresses from ip = " <<*retval.ip <<"\n";

    if (auto va = retval.ip->toUnsigned()) {
        retval.addresses.insert(*va);
    } else if (IS::SymbolicSemantics::SValue::Ptr ipSymbolic = retval.ip.dynamicCast<IS::SymbolicSemantics::SValue>()) {
        SymbolicExpr::Ptr expr = ipSymbolic->get_expression();
        if (SymbolicExpr::OP_ITE == expr->getOperator()) {
            if (auto va = expr->child(1)->toUnsigned()) {
                retval.addresses.insert(*va);
            } else {
                retval.isComplete = false;
            }
            if (auto va = expr->child(2)->toUnsigned()) {
                retval.addresses.insert(*va);
            } else {
                retval.isComplete = false;
            }
        } else {
            struct T1: SymbolicExpr::Visitor {
                typedef std::set<const SymbolicExpr::Node*> SeenNodes;
                SeenNodes seen;
                CodeAddresses &retval;
                T1(CodeAddresses &retval): retval(retval) {}
                SymbolicExpr::VisitAction preVisit(const SymbolicExpr::Ptr &node) {
                    if (seen.insert(getRawPointer(node)).second) {
                        if (auto va = node->toUnsigned())
                            retval.addresses.insert(*va);
                        return SymbolicExpr::CONTINUE;
                    } else {
                        return SymbolicExpr::TRUNCATE;
                    }
                }
                SymbolicExpr::VisitAction postVisit(const SymbolicExpr::Ptr&) {
                    return SymbolicExpr::CONTINUE;
                }
            } visitor(retval);
            expr->depthFirstTraversal(visitor);
            retval.isComplete = false;
        }
    } else {
        retval.isComplete = false;
    }

    if (mlog[DEBUG]) {
        for (rose_addr_t va: retval.addresses)
            mlog[DEBUG] <<"      address: " <<StringUtility::addrToString(va) <<"\n";
        if (!retval.isComplete)
            mlog[DEBUG] <<"      address: undetermined\n";
    }

    return retval;
}

} // namespace
} // namespace
} // namespace

#endif
