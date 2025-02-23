#include <featureTests.h>
#ifdef ROSE_ENABLE_CONCOLIC_TESTING
#include <sage3basic.h>
#include <Rose/BinaryAnalysis/Concolic/LinuxExitStatus.h>

#include <Rose/BinaryAnalysis/Concolic/ConcolicExecutor.h>
#include <Rose/BinaryAnalysis/Concolic/Database.h>
#include <Rose/BinaryAnalysis/Concolic/LinuxExecutor.h>
#include <Rose/BinaryAnalysis/Concolic/Specimen.h>
#include <Rose/BinaryAnalysis/Concolic/TestCase.h>

namespace Rose {
namespace BinaryAnalysis {
namespace Concolic {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LinuxExitStatus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LinuxExitStatus::LinuxExitStatus(const Database::Ptr &db)
    : ExecutionManager(db) {}

LinuxExitStatus::~LinuxExitStatus() {}

// class method
LinuxExitStatus::Ptr
LinuxExitStatus::create(const std::string databaseUrl, const boost::filesystem::path &executableName,
                        const std::vector<std::string> &arguments) {
    // Create the initial test case
    std::string name = executableName.filename().string();
    Specimen::Ptr specimen = Specimen::instance(executableName);
    TestCase::Ptr testCase0 = TestCase::instance(specimen);
    testCase0->name(name + " #0");

    // Create the database
    Database::Ptr db = Database::create("sqlite:" + name + ".db", name);
    db->id(testCase0);                                  // save the first test case, side effect of obtaining an ID

    return Ptr(new LinuxExitStatus(db));
}

LinuxExitStatus::Ptr
LinuxExitStatus::instance(const std::string& databaseUri, const std::string &testSuiteName) {
    ASSERT_not_implemented("[Robb Matzke 2019-04-15]");
}

void
LinuxExitStatus::run() {
    LinuxExecutor::Ptr concreteExecutor = LinuxExecutor::instance(database());
    ConcolicExecutor::Ptr concolicExecutor = ConcolicExecutor::instance();

    while (!isFinished()) {
        // Run as many test cases concretely as possible.
        while (TestCaseId testCaseId = pendingConcreteResult()) {
            TestCase::Ptr testCase = database()->object(testCaseId);
            std::unique_ptr<ConcreteExecutorResult> concreteResult(concreteExecutor->execute(testCase));
            ASSERT_not_null(concreteResult);
            insertConcreteResults(testCase, *concreteResult);
        }

        // Now that all the test cases have run concretely, run a few of the "best" ones concolically.  The "best" is defined
        // either by the ranks returned from the concrete executor, or by this class overriding pendingConcolicResult (which we
        // haven't done).
        for (TestCaseId testCaseId: pendingConcolicResults(10 /*arbitrary*/)) {
            TestCase::Ptr testCase = database()->object(testCaseId);
            std::vector<TestCase::Ptr> newTestCases = concolicExecutor->execute(database(), testCase);
            insertConcolicResults(testCase, newTestCases);
        }
    }
}

} // namespace
} // namespace
} // namespace

#endif
