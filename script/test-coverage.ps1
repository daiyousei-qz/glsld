$ScriptFolder = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptFolder
$CoverageBuildDir = Join-Path $ProjectRoot "build-coverage"
$CoverageReportDir = Join-Path $CoverageBuildDir "coverage-report"

Push-Location $ProjectRoot

# Build the project with coverage enabled
cmake --preset DebugWithCoverage
cmake --build build-coverage

# Run the tests with coverage
$env:LLVM_PROFILE_FILE="$CoverageReportDir/coverage.profraw"
& "$CoverageBuildDir/glsld-test.exe"
llvm-profdata merge -sparse $CoverageReportDir/coverage.profraw -o $CoverageReportDir/coverage.profdata

# Generate the coverage report
llvm-cov report $CoverageBuildDir/glsld-test.exe -instr-profile="$CoverageReportDir/coverage.profdata" > $CoverageReportDir/coverage-report.txt
llvm-cov show $CoverageBuildDir/glsld-test.exe -instr-profile="$CoverageReportDir/coverage.profdata" -format=html > $CoverageReportDir/coverage-report.html

Pop-Location