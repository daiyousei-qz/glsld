
from dataclasses import dataclass, field
from typing import Literal

@dataclass
class BuiltinFunction:
    signature: str
    documentation: str
    guardMacros: list[str]

type GenTypePrefix = Literal["", "d", "i", "u", "b", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32"]

def prefixToScalarType(prefix: GenTypePrefix) -> str:
    match prefix:
        case "":
            return "float"
        case "i":
            return "int"
        case "u":
            return "uint"
        case "d":
            return "double"
        case "b":
            return "bool"
        case "i8":
            return "int8_t"
        case "i16":
            return "int16_t"
        case "i32":
            return "int32_t"
        case "i64":
            return "int64_t"
        case "u8":
            return "uint8_t"
        case "u16":
            return "uint16_t"
        case "u32":
            return "uint32_t"
        case "u64":
            return "uint64_t"
        case "f16":
            return "float16_t"
        case "f32":
            return "float32_t"
        case _:
            raise ValueError(f"Unknown prefix: {prefix}")
    
def prefixToGuardMacros(prefix: GenTypePrefix) -> list[str]:
    match prefix:
        case "i8":
            return ["__GLSLD_FEATURE_ENABLE_INT8_TYPE"]
        case "i16":
            return ["__GLSLD_FEATURE_ENABLE_INT16_TYPE"]
        case "i32":
            return ["__GLSLD_FEATURE_ENABLE_INT32_TYPE"]
        case "i64":
            return ["__GLSLD_FEATURE_ENABLE_INT64_TYPE"]
        case "u8":
            return ["__GLSLD_FEATURE_ENABLE_UINT8_TYPE"]
        case "u16":
            return ["__GLSLD_FEATURE_ENABLE_UINT16_TYPE"]
        case "u32":
            return ["__GLSLD_FEATURE_ENABLE_UINT32_TYPE"]
        case "u64":
            return ["__GLSLD_FEATURE_ENABLE_UINT64_TYPE"]
        case "f16":
            return ["__GLSLD_FEATURE_ENABLE_FLOAT16_TYPE"]
        case "f32":
            return ["__GLSLD_FEATURE_ENABLE_FLOAT32_TYPE"]
        case _:
            return []

@dataclass
class VectorGenTypeInfo:
    prefix: GenTypePrefix
    dim: int
    scalarType: str
    guardMacros: list[str] = field(default_factory=list)

    def __init__(self, prefix: GenTypePrefix, dim: int):
        assert dim >= 1 and dim <= 4
        if dim == 1:
            self.prefix = ""
            self.dim = 1
        else:
            self.prefix = prefix
            self.dim = dim
        self.scalarType = prefixToScalarType(prefix)
        self.guardMacros = prefixToGuardMacros(prefix)

    def getBType(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("b", int(self.dim))
    
    def getIType(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("i", int(self.dim))

    def getUType(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("u", int(self.dim))
    
    def getITypeForBitcast(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("i", int(self.dim))
    
    def getUTypeForBitcast(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("u", int(self.dim))
    
    def getFTypeForBitcast(self) -> 'VectorGenTypeInfo':
        return VectorGenTypeInfo("", int(self.dim))

    def __str__(self):
        if self.dim == 1:
            return self.scalarType
        else:
            return f"{self.prefix}vec{self.dim}"

@dataclass
class MatrixGenTypeInfo:
    prefix: GenTypePrefix
    dimX: int
    dimY: int
    scalarType: str
    guardMacros: list[str] = field(default_factory=list)

    def __init__(self, prefix: GenTypePrefix, dimX: int, dimY: int):
        assert dimX >= 2 and dimX <=4
        assert dimY >= 2 and dimY <=4
        self.prefix = prefix
        self.dimX = dimX
        self.dimY = dimY
        self.scalarType = prefixToScalarType(prefix)
        self.guardMacros = prefixToGuardMacros(prefix)

    def getRowVectorType(self) -> VectorGenTypeInfo:
        return VectorGenTypeInfo(self.prefix, self.dimY)
    
    def getColumnVectorType(self) -> VectorGenTypeInfo:
        return VectorGenTypeInfo(self.prefix, self.dimX)

    def getTransposedMatrixType(self) -> 'MatrixGenTypeInfo':
        return MatrixGenTypeInfo(self.prefix, self.dimY, self.dimX)

    def __str__(self):
        if self.dimX == self.dimY:
            return f"{self.prefix}mat{self.dimX}"
        else:
            return f"{self.prefix}mat{self.dimX}x{self.dimY}"

def allScalarGenTypes(allPrefixes: list[GenTypePrefix]):
    for prefix in allPrefixes:
        yield VectorGenTypeInfo(prefix, 1)

def allVectorGenTypes(allPrefixes: list[GenTypePrefix]):
    for dim in [1, 2, 3, 4]:
        for prefix in allPrefixes:
            yield VectorGenTypeInfo(prefix, dim)

def allMatrixGenTypes(allPrefixes: list[GenTypePrefix]):
    for dimX in [2, 3, 4]:
        for dimY in [2, 3, 4]:
            for prefix in allPrefixes:
                yield MatrixGenTypeInfo(prefix, dimX, dimY)

class StdlibBuilder:
    data: list[BuiltinFunction] = []
    snippets: list[str] = []

    def addFunction(self, signature: str, documentation: str, guardMacros: list[str] = []):
        self.data.append(BuiltinFunction(signature, documentation, guardMacros))

    def addVariable(self, declaration: str, documentation: str, guardMacros: list[str] = []):
        pass

    def addSnippet(self, snippet: str):
        self.snippets.append(snippet)

    def build(self):
        buffer = ""
        for snippet in self.snippets:
            buffer += snippet
            buffer += "\n"

        for func in self.data:
            if len(func.guardMacros) > 0:
                for guardMacro in func.guardMacros:
                    buffer += f"#if {guardMacro}\n"
            for documentationLine in func.documentation.strip().splitlines():
                buffer += f"// {documentationLine.strip()}\n"
            buffer += func.signature.strip()
            buffer += "\n"
            if len(func.guardMacros) > 0:
                buffer += "#endif\n"

        return buffer