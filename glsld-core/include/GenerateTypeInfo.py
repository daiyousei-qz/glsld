import itertools


def GenScalarType():
    return ['int', 'uint', 'float', 'double']


def GenVectorType():
    prefix = ['b', 'i', 'u', '', 'd']
    size = ['2', '3', '4']
    for p, s in itertools.product(prefix, size):
        print(f'{p}vec{s}')


def GenMatrixType():
    prefix = ['', 'd']
    size = ['2', '3', '4']
    for p, s1, s2 in itertools.product(prefix, size, size):
        if s1 == s2:
            print(f'{p}mat{s1}')
        else:
            print(f'{p}mat{s1}x{s2}')


def GenSamplerType():
    samplerTypes = [
        # Combined Sampler
        "sampler1D",
        "sampler1DShoadow",
        "sampler1DArray",
        "sampler1DArrayShadow",
        "isampler1D",
        "isampler1DArray",
        "usampler1D",
        "usampler1DArray",
        "sampler2D",
        "sampler2DShoadow",
        "sampler2DArray",
        "sampler2DArrayShadow",
        "isampler2D",
        "isampler2DArray",
        "usampler2D",
        "usampler2DArray",
        "sampler2DRect",
        "sampler2DRectShadow",
        "isampler2DRect",
        "usampler2DRect",
        "sampler2DMS",
        "isampler2DMS",
        "usampler2DMS",
        "sampler2DMSArray",
        "isampler2DMSArray",
        "usampler2DMSArray",
        "sampler3D",
        "isampler3D",
        "usampler3D",
        "samplerCube",
        "samplerCubeShadow",
        "isamplerCube",
        "usamplerCube",
        "samplerCubeArray",
        "samplerCubeArrayShadow",
        "isamplerCubeArray",
        "usamplerCubeArray",
        "samplerBuffer",
        "isamplerBuffer",
        "usamplerBuffer",
    ]

    imageTypes = [
        # Image
        "image1D",
        "image1DArray",
        "iimage1D",
        "iimage1DArray",
        "uimage1D",
        "uimage1DArray",
        "image2D",
        "image2DArray",
        "iimage2D",
        "iimage2DArray",
        "uimage2D",
        "uimage2DArray",
        "image2DRect",
        "iimage2DRect",
        "uimage2DRect",
        "image2DMS",
        "iimage2DMS",
        "uimage2DMS",
        "image2DMSArray",
        "iimage2DMSArray",
        "uimage2DMSArray",
        "image3D",
        "iimage3D",
        "uimage3D",
        "imageCube",
        "iimageCube",
        "uimageCube",
        "imageCubeArray",
        "iimageCubeArray",
        "uimageCubeArray",
        "imageBuffer",
        "iimageBuffer",
        "uimageBuffer",
    ]

    sepSamplerTypes = [
        # Separate Sampler(Vulkan-only)
        "sampler",
        "samplerShadow",
    ]

    textureTypes = [
        # Separate Texture(Vulkan-only)
        "texture1D",
        "texture1DArray",
        "itexture1D",
        "itexture1DArray",
        "utexture1D",
        "utexture1DArray",
        "texture2D",
        "texture2DArray",
        "itexture2D",
        "itexture2DArray",
        "utexture2D",
        "utexture2DArray",
        "texture2DRect",
        "itexture2DRect",
        "utexture2DRect",
        "texture2DMS",
        "itexture2DMS",
        "utexture2DMS",
        "texture2DMSArray",
        "itexture2DMSArray",
        "utexture2DMSArray",
        "texture3D",
        "itexture3D",
        "utexture3D",
        "textureCube",
        "itextureCube",
        "utextureCube",
        "textureCubeArray",
        "itextureCubeArray",
        "utextureCubeArray",
        "textureBuffer",
        "itextureBuffer",
        "utextureBuffer",
    ]

    for tyName in [*samplerTypes, *imageTypes, *textureTypes]:
        dimText = ''
        for dim in ['1D', '2D', '3D', 'Cube', 'Buffer']:
            if dim in tyName:
                dimText = dim
                break

        scalarType = 'float'
        uprefix = ['usampler', 'uimage', 'utexture']
        iprefix = ['isampler', 'iimage', 'itexture']
        if any([tyName.startswith(prefix) for prefix in uprefix]):
            scalarType = 'uint'
        if any([tyName.startswith(prefix) for prefix in iprefix]):
            scalarType = 'int'

        shadow = 'Shadow' in tyName
        ms = 'MS' in tyName
        arrayed = 'Array' in tyName

        print(
            f'{tyName}: dim={dimText}, type={scalarType}, shadow={shadow}, ms={ms}, arrayed={arrayed}')


GenSamplerType()
