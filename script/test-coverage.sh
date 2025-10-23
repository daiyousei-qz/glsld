#!/usr/bin/env bash

set -euo pipefail

scriptFolder="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
projectRoot="$(dirname "$scriptFolder")"
coverageBuildDir="$projectRoot/build-coverage"
coverageReportDir="$coverageBuildDir/coverage-report"

pushd "$projectRoot" >/dev/null

# Build the project with coverage enabled
cmake --preset DebugWithCoverage
cmake --build "$coverageBuildDir"

# Run the tests with coverage
mkdir -p "$coverageReportDir"
export LLVM_PROFILE_FILE="$coverageReportDir/coverage.profraw"
$coverageBuildDir/glsld-test
llvm-profdata merge -sparse "$coverageReportDir/coverage.profraw" -o "$coverageReportDir/coverage.profdata"

# Generate the coverage report
llvm-cov report "$coverageBuildDir/glsld-test" -instr-profile="$coverageReportDir/coverage.profdata" >"$coverageReportDir/coverage-report.txt"
llvm-cov show "$coverageBuildDir/glsld-test" -instr-profile="$coverageReportDir/coverage.profdata" -format=html >"$coverageReportDir/coverage-report.html"
llvm-cov export "$coverageBuildDir/glsld-test" -instr-profile="$coverageReportDir/coverage.profdata" -format=lcov >"$coverageReportDir/coverage.lcov"

popd >/dev/null