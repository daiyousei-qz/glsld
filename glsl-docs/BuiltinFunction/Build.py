
from glob import glob
from os import linesep, path
import sys


def Build() -> None:
    docDirPath = path.dirname(path.abspath(__file__))

    print(f'// clang-format off')
    for docPath in glob(path.join(docDirPath, '**/*.md'), recursive=True):
        docFileName = path.basename(docPath)
        functionName = docFileName.removesuffix('.md')
        with open(docPath, 'rb') as docFile:
            documentation = str(
                docFile.read(), encoding='utf-8').replace('\\', '\\\\').replace(linesep, '\\n')

        print(f'{{')
        print(f'    "{functionName}",')
        print(f'    "{documentation}",')
        print(f'}},')
    print(f'// clang-format on')


def main() -> None:
    with open('output.txt', 'w', encoding='utf-8') as sys.stdout:
        Build()


if __name__ == '__main__':
    main()
