#include "solver/calculix/CalculiXCaseWriter.h"

#include "solver/calculix/CalculiXCaseDataBuilder.h"
#include "solver/calculix/CalculiXCasePaths.h"
#include "solver/calculix/CalculiXInputDeckBuilder.h"

SolverCaseWriterResult CalculiXCaseWriter::write(const SolverCaseContext &context) const
{
    SolverCaseWriterResult result;
    if (!context.isValid()) {
        result.errors.append("CalculiX export failed: invalid solver case context.");
        return result;
    }

    const CalculiXCasePaths paths = CalculiXCasePathsBuilder::fromContext(context);
    const CalculiXCaseDataBuildResult caseDataResult = CalculiXCaseDataBuilder().build(context);
    result.caseRootPath = paths.caseDirectory;
    result.logMessages.append(caseDataResult.logMessages);
    result.warnings.append(caseDataResult.warnings);
    result.errors.append(caseDataResult.errors);
    if (!caseDataResult.success) {
        return result;
    }

    const CalculiXInputDeckBuildResult deckBuildResult =
        CalculiXInputDeckBuilder().build(caseDataResult.caseData);
    result.logMessages.append(deckBuildResult.logMessages);
    result.warnings.append(deckBuildResult.warnings);
    result.errors.append(deckBuildResult.errors);
    if (!deckBuildResult.success) {
        return result;
    }

    QString writeError;
    if (!deckBuildResult.deck.writeToFile(paths.inputFile, &writeError)) {
        result.errors.append(writeError);
        return result;
    }

    result.success = true;
    result.writtenFiles.append(paths.inputFile);
    result.logMessages.append("CalculiX input written: " + paths.inputFile);
    return result;
}
