#pragma once

#include <utils/enum_utils.hpp>

#if !defined(USE_WEBGL) && defined(EMSCRIPTEN)
#define USE_WEBGL
#endif

#if !defined(USE_OPENGL_ES) && (defined(EMSCRIPTEN) || defined(USE_WEBGL))
#define USE_OPENGL_ES
#endif

namespace xen {
enum class Capability : uint32_t {
    CULL = 2884 /* GL_CULL_FACE      */,   ///<
    DITHER = 3024 /* GL_DITHER         */, ///<
    BLEND = 3042 /* GL_BLEND          */,  ///<
#if !defined(USE_OPENGL_ES)
    COLOR_LOGIC_OP = 3058 /* GL_COLOR_LOGIC_OP */, ///<
#endif

#if !defined(USE_OPENGL_ES)
    DEPTH_CLAMP = 34383 /* GL_DEPTH_CLAMP  */, ///<
#endif
    DEPTH_TEST = 2929 /* GL_DEPTH_TEST   */,   ///<
    STENCIL_TEST = 2960 /* GL_STENCIL_TEST */, ///<
    SCISSOR_TEST = 3089 /* GL_SCISSOR_TEST */, ///<

#if !defined(USE_OPENGL_ES)
    LINE_SMOOTH = 2848 /* GL_LINE_SMOOTH          */,           ///<
    POLYGON_SMOOTH = 2881 /* GL_POLYGON_SMOOTH       */,        ///<
    POLYGON_OFFSET_POINT = 10753 /* GL_POLYGON_OFFSET_POINT */, ///<
    POLYGON_OFFSET_LINE = 10754 /* GL_POLYGON_OFFSET_LINE  */,  ///<
#endif
    POLYGON_OFFSET_FILL = 32823 /* GL_POLYGON_OFFSET_FILL  */, ///<

#if !defined(USE_OPENGL_ES)
    MULTISAMPLE = 32925 /* GL_MULTISAMPLE              */,         ///<
    SAMPLE_SHADING = 35894 /* GL_SAMPLE_SHADING           */,      ///<
    SAMPLE_ALPHA_TO_ONE = 32927 /* GL_SAMPLE_ALPHA_TO_ONE      */, ///<
#endif
    SAMPLE_ALPHA_TO_COVERAGE = 32926 /* GL_SAMPLE_ALPHA_TO_COVERAGE */, ///<
    SAMPLE_COVERAGE = 32928 /* GL_SAMPLE_COVERAGE          */,          ///<
    SAMPLE_MASK = 36433 /* GL_SAMPLE_MASK              */,              ///<

#if !defined(USE_OPENGL_ES)
    POINT_SIZE = 34370 /* GL_PROGRAM_POINT_SIZE        */,        ///<
    CUBEMAP_SEAMLESS = 34895 /* GL_TEXTURE_CUBE_MAP_SEAMLESS */,  ///<
    FRAMEBUFFER_SRGB = 36281 /* GL_FRAMEBUFFER_SRGB          */,  ///<
    PRIMITIVE_RESTART = 36765 /* GL_PRIMITIVE_RESTART         */, ///<

    DEBUG_OUTPUT = 37600 /* GL_DEBUG_OUTPUT             */,            ///<
    DEBUG_OUTPUT_SYNCHRONOUS = 33346 /* GL_DEBUG_OUTPUT_SYNCHRONOUS */ ///<
#endif
};

enum class ContextInfo : uint32_t {
    VENDOR = 7936 /* GL_VENDOR                   */,                   ///< Graphics card vendor.
    RENDERER = 7937 /* GL_RENDERER                 */,                 ///< Graphics card model.
    VERSION = 7938 /* GL_VERSION                  */,                  ///< OpenGL version.
    SHADING_LANGUAGE_VERSION = 35724 /* GL_SHADING_LANGUAGE_VERSION */ ///< GLSL version.
};

enum class StateParameter : uint32_t {
    MAJOR_VERSION = 33307 /* GL_MAJOR_VERSION  */,   ///< OpenGL major version (in the form major.minor).
    MINOR_VERSION = 33308 /* GL_MINOR_VERSION  */,   ///< OpenGL minor version (in the form major.minor).
    EXTENSION_COUNT = 33309 /* GL_NUM_EXTENSIONS */, ///< Number of extensions supported for the current context.

    ACTIVE_TEXTURE = 34016 /* GL_ACTIVE_TEXTURE  */,  ///< Currently active texture.
    CURRENT_PROGRAM = 35725 /* GL_CURRENT_PROGRAM */, ///< Currently used program.

    DEPTH_CLEAR_VALUE = 2931 /* GL_DEPTH_CLEAR_VALUE */,                                   ///< Depth clear value.
    DEPTH_FUNC = 2932 /* GL_DEPTH_FUNC        */,                                          ///< Depth function.
    DEPTH_RANGE = 2928 /* GL_DEPTH_RANGE       */,                                         ///< Depth range.
    DEPTH_TEST = static_cast<uint32_t>(Capability::DEPTH_TEST) /* GL_DEPTH_TEST        */, ///< Depth testing.
    DEPTH_WRITEMASK = 2930 /* GL_DEPTH_WRITEMASK   */,                                     ///< Depth write mask.

    COLOR_CLEAR_VALUE = 3106 /* GL_COLOR_CLEAR_VALUE */, ///< Clear color.
#if !defined(USE_OPENGL_ES)
    COLOR_LOGIC_OP = static_cast<uint32_t>(Capability::COLOR_LOGIC_OP) /* GL_COLOR_LOGIC_OP    */, ///<
#endif
    COLOR_WRITEMASK = 3107 /* GL_COLOR_WRITEMASK   */, ///< Color write mask.

    BLEND = static_cast<uint32_t>(Capability::BLEND) /* GL_BLEND                */, ///< Blending.
    BLEND_COLOR = 32773 /* GL_BLEND_COLOR          */,                              ///<
    BLEND_DST_RGB = 32968 /* GL_BLEND_DST_RGB        */,                            ///<
    BLEND_DST_ALPHA = 32970 /* GL_BLEND_DST_ALPHA      */,                          ///<
    BLEND_SRC_RGB = 32969 /* GL_BLEND_SRC_RGB        */,                            ///<
    BLEND_SRC_ALPHA = 32971 /* GL_BLEND_SRC_ALPHA      */,                          ///<
    BLEND_EQUATION_RGB = 32777 /* GL_BLEND_EQUATION_RGB   */,                       ///<
    BLEND_EQUATION_ALPHA = 34877 /* GL_BLEND_EQUATION_ALPHA */,                     ///<

    ALIASED_LINE_WIDTH_RANGE = 33902 /* GL_ALIASED_LINE_WIDTH_RANGE      */, ///<
#if !defined(USE_OPENGL_ES)
    SMOOTH_LINE_WIDTH_RANGE = 2850 /* GL_SMOOTH_LINE_WIDTH_RANGE       */,       ///<
    SMOOTH_LINE_WIDTH_GRANULARITY = 2851 /* GL_SMOOTH_LINE_WIDTH_GRANULARITY */, ///<
#endif

    DITHER = static_cast<uint32_t>(Capability::DITHER) /* GL_DITHER                     */,  ///< Dithering.
    CULL_FACE = static_cast<uint32_t>(Capability::CULL) /* GL_CULL_FACE                  */, ///< Polygon culling.
#if !defined(USE_OPENGL_ES)
    POINT_SIZE = static_cast<uint32_t>(Capability::POINT_SIZE) /* GL_POINT_SIZE                 */, ///< Point size.
#endif
    COMPRESSED_TEXTURE_FORMATS = 34467 /* GL_COMPRESSED_TEXTURE_FORMATS */, ///<
    ARRAY_BUFFER_BINDING = 34964 /* GL_ARRAY_BUFFER_BINDING       */,       ///<

#if !defined(USE_OPENGL_ES)
    UNPACK_SWAP_BYTES = 3312 /* GL_UNPACK_SWAP_BYTES  */, ///<
    UNPACK_LSB_FIRST = 3313 /* GL_UNPACK_LSB_FIRST   */,  ///<
#endif
    UNPACK_ROW_LENGTH = 3314 /* GL_UNPACK_ROW_LENGTH  */,    ///<
    UNPACK_SKIP_ROWS = 3315 /* GL_UNPACK_SKIP_ROWS   */,     ///<
    UNPACK_SKIP_PIXELS = 3316 /* GL_UNPACK_SKIP_PIXELS */,   ///<
    UNPACK_SKIP_IMAGES = 32877 /* GL_PACK_SKIP_IMAGES   */,  ///<
    UNPACK_IMAGE_HEIGHT = 32878 /* GL_PACK_IMAGE_HEIGHT  */, ///<
    UNPACK_ALIGNMENT = 3317 /* GL_UNPACK_ALIGNMENT   */,     ///<

#if !defined(USE_OPENGL_ES)
    PACK_SWAP_BYTES = 3328 /* GL_PACK_SWAP_BYTES   */, ///<
    PACK_LSB_FIRST = 3329 /* GL_PACK_LSB_FIRST    */,  ///<
#endif
    PACK_ROW_LENGTH = 3330 /* GL_PACK_ROW_LENGTH   */,    ///<
    PACK_SKIP_ROWS = 3331 /* GL_PACK_SKIP_ROWS    */,     ///<
    PACK_SKIP_PIXELS = 3332 /* GL_PACK_SKIP_PIXELS  */,   ///<
    PACK_SKIP_IMAGES = 32875 /* GL_PACK_SKIP_IMAGES  */,  ///<
    PACK_IMAGE_HEIGHT = 32876 /* GL_PACK_IMAGE_HEIGHT */, ///<
    PACK_ALIGNMENT = 3333 /* GL_PACK_ALIGNMENT    */,     ///<
};

enum class MaskType : uint32_t {
    COLOR = 16384 /* GL_COLOR_BUFFER_BIT   */, ///<
    DEPTH = 256 /* GL_DEPTH_BUFFER_BIT   */,   ///<
    STENCIL = 1024 /* GL_STENCIL_BUFFER_BIT */ ///<
};
MAKE_ENUM_FLAG(MaskType)

enum class DepthStencilFunction : uint32_t {
    NEVER = 512 /* GL_NEVER    */,         ///<
    EQUAL = 514 /* GL_EQUAL    */,         ///<
    NOT_EQUAL = 517 /* GL_NOTEQUAL */,     ///<
    LESS = 513 /* GL_LESS     */,          ///<
    LESS_EQUAL = 515 /* GL_LEQUAL   */,    ///<
    GREATER = 516 /* GL_GREATER  */,       ///<
    GREATER_EQUAL = 518 /* GL_GEQUAL   */, ///<
    ALWAYS = 519 /* GL_ALWAYS   */         ///<
};

enum class StencilOperation : uint32_t {
    ZERO = 0 /* GL_ZERO      */,               ///<
    KEEP = 7680 /* GL_KEEP      */,            ///<
    REPLACE = 7681 /* GL_REPLACE   */,         ///<
    INCREMENT = 7682 /* GL_INCR      */,       ///<
    INCREMENT_WRAP = 34055 /* GL_INCR_WRAP */, ///<
    DECREMENT = 7683 /* GL_DECR      */,       ///<
    DECREMENT_WRAP = 34056 /* GL_DECR_WRAP */, ///<
    INVERT = 5386 /* GL_INVERT    */           ///<
};

enum class FaceOrientation : uint32_t {
    FRONT = 1028 /* GL_FRONT          */,     ///<
    BACK = 1029 /* GL_BACK           */,      ///<
    FRONT_BACK = 1032 /* GL_FRONT_AND_BACK */ ///<
};

enum class BlendFactor : uint32_t {
    ZERO = 0 /* GL_ZERO                     */,                         ///<
    ONE = 1 /* GL_ONE                      */,                          ///<
    SRC_COLOR = 768 /* GL_SRC_COLOR                */,                  ///<
    ONE_MINUS_SRC_COLOR = 769 /* GL_ONE_MINUS_SRC_COLOR      */,        ///<
    SRC_ALPHA = 770 /* GL_SRC_ALPHA                */,                  ///<
    ONE_MINUS_SRC_ALPHA = 771 /* GL_ONE_MINUS_SRC_ALPHA      */,        ///<
    DST_ALPHA = 772 /* GL_DST_ALPHA                */,                  ///<
    ONE_MINUS_DST_ALPHA = 773 /* GL_ONE_MINUS_DST_ALPHA      */,        ///<
    DST_COLOR = 774 /* GL_DST_COLOR                */,                  ///<
    ONE_MINUS_DST_COLOR = 775 /* GL_ONE_MINUS_DST_COLOR      */,        ///<
    SRC_ALPHA_SATURATE = 776 /* GL_SRC_ALPHA_SATURATE       */,         ///<
    CONSTANT_COLOR = 32769 /* GL_CONSTANT_COLOR           */,           ///<
    ONE_MINUS_CONSTANT_COLOR = 32770 /* GL_ONE_MINUS_CONSTANT_COLOR */, ///<
    CONSTANT_ALPHA = 32771 /* GL_CONSTANT_ALPHA           */,           ///<
    ONE_MINUS_CONSTANT_ALPHA = 32772 /* GL_ONE_MINUS_CONSTANT_ALPHA */, ///<
#if !defined(USE_OPENGL_ES)
    SRC1_COLOR = 35065 /* GL_SRC1_COLOR               */,           ///<
    ONE_MINUS_SRC1_COLOR = 35066 /* GL_ONE_MINUS_SRC1_COLOR     */, ///<
    SRC1_ALPHA = 34185 /* GL_SRC1_ALPHA               */,           ///<
    ONE_MINUS_SRC1_ALPHA = 35067 /* GL_ONE_MINUS_SRC1_ALPHA     */  ///<
#endif
};

enum class PolygonMode : uint32_t {
    POINT = 6912 /* GL_POINT */, ///<
    LINE = 6913 /* GL_LINE  */,  ///<
    FILL = 6914 /* GL_FILL  */   ///<
};

enum class ClipOrigin : uint32_t {
    LOWER_LEFT = 36001 /* GL_LOWER_LEFT */, ///<
    UPPER_LEFT = 36002 /* GL_UPPER_LEFT */  ///<
};

enum class ClipDepth : uint32_t {
    NEG_ONE_TO_ONE = 37726 /* GL_NEGATIVE_ONE_TO_ONE */, ///<
    ZERO_TO_ONE = 37727 /* GL_ZERO_TO_ONE         */     ///<
};

enum class PatchParameter : uint32_t {
    DEFAULT_OUTER_LEVEL = 36468 /* GL_PATCH_DEFAULT_OUTER_LEVEL */, ///< Default outer level used if no tessellation
                                                                    ///< control shader exists.
    DEFAULT_INNER_LEVEL =
        36467 /* GL_PATCH_DEFAULT_INNER_LEVEL */ ///< Default inner level used if no tessellation control shader exists.
};

enum class PixelStorage : uint32_t {
#if !defined(USE_OPENGL_ES)
    UNPACK_SWAP_BYTES = static_cast<uint32_t>(StateParameter::UNPACK_SWAP_BYTES), ///<
    UNPACK_LSB_FIRST = static_cast<uint32_t>(StateParameter::UNPACK_LSB_FIRST),   ///<
#endif
    UNPACK_ROW_LENGTH = static_cast<uint32_t>(StateParameter::UNPACK_ROW_LENGTH),     ///<
    UNPACK_SKIP_ROWS = static_cast<uint32_t>(StateParameter::UNPACK_SKIP_ROWS),       ///<
    UNPACK_SKIP_PIXELS = static_cast<uint32_t>(StateParameter::UNPACK_SKIP_PIXELS),   ///<
    UNPACK_SKIP_IMAGES = static_cast<uint32_t>(StateParameter::UNPACK_SKIP_IMAGES),   ///<
    UNPACK_IMAGE_HEIGHT = static_cast<uint32_t>(StateParameter::UNPACK_IMAGE_HEIGHT), ///<
    UNPACK_ALIGNMENT = static_cast<uint32_t>(StateParameter::UNPACK_ALIGNMENT),       ///<

#if !defined(USE_OPENGL_ES)
    PACK_SWAP_BYTES = static_cast<uint32_t>(StateParameter::PACK_SWAP_BYTES), ///<
    PACK_LSB_FIRST = static_cast<uint32_t>(StateParameter::PACK_LSB_FIRST),   ///<
#endif
    PACK_ROW_LENGTH = static_cast<uint32_t>(StateParameter::PACK_ROW_LENGTH),     ///<
    PACK_SKIP_ROWS = static_cast<uint32_t>(StateParameter::PACK_SKIP_ROWS),       ///<
    PACK_SKIP_PIXELS = static_cast<uint32_t>(StateParameter::PACK_SKIP_PIXELS),   ///<
    PACK_SKIP_IMAGES = static_cast<uint32_t>(StateParameter::PACK_SKIP_IMAGES),   ///<
    PACK_IMAGE_HEIGHT = static_cast<uint32_t>(StateParameter::PACK_IMAGE_HEIGHT), ///<
    PACK_ALIGNMENT = static_cast<uint32_t>(StateParameter::PACK_ALIGNMENT)        ///<
};

enum class AttribDataType : uint32_t {
    BYTE = 5120 /* GL_BYTE                         */,             ///< Byte.
    UBYTE = 5121 /* GL_UNSIGNED_BYTE                */,            ///< Unsigned byte.
    SHORT = 5122 /* GL_SHORT                        */,            ///< Short.
    USHORT = 5123 /* GL_UNSIGNED_SHORT               */,           ///< Unsigned short.
    INT = 5124 /* GL_INT                          */,              ///< Integer.
    INT_2_10_10_10 = 36255 /* GL_INT_2_10_10_10_REV           */,  ///< Packed integer stored in an ABGR format (2 alpha
                                                                   ///< bits & 10 BGR bits each).
    UINT = 5125 /* GL_UNSIGNED_INT                 */,             ///< Unsigned integer.
    UINT_2_10_10_10 = 33640 /* GL_UNSIGNED_INT_2_10_10_10_REV  */, ///< Packed uinteger stored in an ABGR format
                                                                   ///< (2 alpha bits & 10 BGR bits each).
    HALF_FLOAT = 5131 /* GL_HALF_FLOAT                   */,       ///< Half-precision floating-point.
    FLOAT = 5126 /* GL_FLOAT                        */,            ///< Simple-precision floating-point.
    FIXED = 5132 /* GL_FIXED                        */,            ///< Fixed-precision.
    UINT_10F_11F_11F =
        35899 /* GL_UNSIGNED_INT_10F_11F_11F_REV */, ///< Packed floating-point stored in a BGR format (10 blue bits &
                                                     ///< 11 green & red bits each). Requires OpenGL 4.4+.
#if !defined(USE_OPENGL_ES)
    DOUBLE = 5130 /* GL_DOUBLE                       */ ///< Double-precision floating-point.
#endif
};

enum class BufferType : uint32_t {
    ARRAY_BUFFER = 34962 /* GL_ARRAY_BUFFER         */,   ///<
    ELEMENT_BUFFER = 34963 /* GL_ELEMENT_ARRAY_BUFFER */, ///<
    UNIFORM_BUFFER = 35345 /* GL_UNIFORM_BUFFER       */  ///<
};

enum class BufferDataUsage : uint32_t {
    STREAM_DRAW = 35040 /* GL_STREAM_DRAW */, ///<
    STREAM_READ = 35041 /* GL_STREAM_READ */, ///<
    STREAM_COPY = 35042 /* GL_STREAM_COPY */, ///<

    STATIC_DRAW = 35044 /* GL_STATIC_DRAW */, ///<
    STATIC_READ = 35045 /* GL_STATIC_READ */, ///<
    STATIC_COPY = 35046 /* GL_STATIC_COPY */, ///<

    DYNAMIC_DRAW = 35048 /* GL_DYNAMIC_DRAW */, ///<
    DYNAMIC_READ = 35049 /* GL_DYNAMIC_READ */, ///<
    DYNAMIC_COPY = 35050 /* GL_DYNAMIC_COPY */  ///<
};

enum class TextureType : uint32_t {
#if !defined(USE_OPENGL_ES)
    TEXTURE_1D = 3552 /* GL_TEXTURE_1D                  */, ///<
#endif
    TEXTURE_2D = 3553 /* GL_TEXTURE_2D                  */,  ///<
    TEXTURE_3D = 32879 /* GL_TEXTURE_3D                  */, ///<
#if !defined(USE_OPENGL_ES)
    TEXTURE_1D_ARRAY = 35864 /* GL_TEXTURE_1D_ARRAY            */, ///<
#endif
    TEXTURE_2D_ARRAY = 35866 /* GL_TEXTURE_2D_ARRAY            */, ///<
    CUBEMAP = 34067 /* GL_TEXTURE_CUBE_MAP            */,          ///<
    CUBEMAP_POS_X = 34069 /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */,    ///<
    CUBEMAP_NEG_X = 34070 /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */,    ///<
    CUBEMAP_POS_Y = 34071 /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */,    ///<
    CUBEMAP_NEG_Y = 34072 /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */,    ///<
    CUBEMAP_POS_Z = 34073 /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */,    ///<
    CUBEMAP_NEG_Z = 34074 /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */,    ///<
#if !defined(USE_OPENGL_ES)
    CUBEMAP_ARRAY = 36873 /* GL_TEXTURE_CUBE_MAP_ARRAY      */ ///<
#endif
};

enum class TextureParam : uint32_t {
    MINIFY_FILTER = 10241 /* GL_TEXTURE_MIN_FILTER   */,  ///<
    MAGNIFY_FILTER = 10240 /* GL_TEXTURE_MAG_FILTER   */, ///<
    WRAP_S = 10242 /* GL_TEXTURE_WRAP_S       */,         ///<
    WRAP_T = 10243 /* GL_TEXTURE_WRAP_T       */,         ///<
    WRAP_R = 32882 /* GL_TEXTURE_WRAP_R       */,         ///<
    SWIZZLE_R = 36418 /* GL_TEXTURE_SWIZZLE_R    */,      ///<
    SWIZZLE_G = 36419 /* GL_TEXTURE_SWIZZLE_G    */,      ///<
    SWIZZLE_B = 36420 /* GL_TEXTURE_SWIZZLE_B    */,      ///<
    SWIZZLE_A = 36421 /* GL_TEXTURE_SWIZZLE_A    */,      ///<
#if !defined(USE_OPENGL_ES)
    SWIZZLE_RGBA = 36422 /* GL_TEXTURE_SWIZZLE_RGBA */ ///<
#endif
};

enum class TextureParamValue : uint32_t {
    NEAREST = 9728 /* GL_NEAREST                */,                ///<
    LINEAR = 9729 /* GL_LINEAR                 */,                 ///<
    NEAREST_MIPMAP_NEAREST = 9984 /* GL_NEAREST_MIPMAP_NEAREST */, ///<
    LINEAR_MIPMAP_NEAREST = 9985 /* GL_LINEAR_MIPMAP_NEAREST  */,  ///<
    NEAREST_MIPMAP_LINEAR = 9986 /* GL_NEAREST_MIPMAP_LINEAR  */,  ///<
    LINEAR_MIPMAP_LINEAR = 9987 /* GL_LINEAR_MIPMAP_LINEAR   */,   ///<
    REPEAT = 10497 /* GL_REPEAT                 */,                ///<
    CLAMP_TO_EDGE = 33071 /* GL_CLAMP_TO_EDGE          */          ///<
};

enum class TextureAttribute : uint32_t {
    WIDTH = 4096 /* GL_TEXTURE_WIDTH                 */,                 ///<
    HEIGHT = 4097 /* GL_TEXTURE_HEIGHT                */,                ///<
    DEPTH = 32881 /* GL_TEXTURE_DEPTH                 */,                ///<
    INTERNAL_FORMAT = 4099 /* GL_TEXTURE_INTERNAL_FORMAT       */,       ///<
    RED_SIZE = 32860 /* GL_TEXTURE_RED_SIZE              */,             ///<
    GREEN_SIZE = 32861 /* GL_TEXTURE_GREEN_SIZE            */,           ///<
    BLUE_SIZE = 32862 /* GL_TEXTURE_BLUE_SIZE             */,            ///<
    ALPHA_SIZE = 32863 /* GL_TEXTURE_ALPHA_SIZE            */,           ///<
    DEPTH_SIZE = 34890 /* GL_TEXTURE_DEPTH_SIZE            */,           ///<
    COMPRESSED = 34465 /* GL_TEXTURE_COMPRESSED            */,           ///<
    COMPRESSED_IMAGE_SIZE = 34464 /* GL_TEXTURE_COMPRESSED_IMAGE_SIZE */ ///<
};

enum class TextureFormat : uint32_t {
    RED = 6403 /* GL_RED             */,           ///<
    GREEN = 6404 /* GL_GREEN           */,         ///<
    BLUE = 6405 /* GL_BLUE            */,          ///<
    RG = 33319 /* GL_RG              */,           ///<
    RGB = 6407 /* GL_RGB             */,           ///<
    BGR = 32992 /* GL_BGR             */,          ///<
    RGBA = 6408 /* GL_RGBA            */,          ///<
    BGRA = 32993 /* GL_BGRA            */,         ///<
    SRGB = 35904 /* GL_SRGB            */,         ///<
    DEPTH = 6402 /* GL_DEPTH_COMPONENT */,         ///<
    STENCIL = 6401 /* GL_STENCIL_INDEX   */,       ///<
    DEPTH_STENCIL = 34041 /* GL_DEPTH_STENCIL   */ ///<
};

enum class TextureInternalFormat : uint32_t {
    // Base formats
    RED = static_cast<uint32_t>(TextureFormat::RED),
    RG = static_cast<uint32_t>(TextureFormat::RG),
    RGB = static_cast<uint32_t>(TextureFormat::RGB),
    BGR = static_cast<uint32_t>(TextureFormat::BGR),
    RGBA = static_cast<uint32_t>(TextureFormat::RGBA),
    BGRA = static_cast<uint32_t>(TextureFormat::BGRA),
    SRGB = static_cast<uint32_t>(TextureFormat::SRGB),
    DEPTH = static_cast<uint32_t>(TextureFormat::DEPTH),
    DEPTH_STENCIL = static_cast<uint32_t>(TextureFormat::DEPTH_STENCIL),

    // Sized formats
    R8 = 33321 /* GL_R8           */,          ///<
    RG8 = 33323 /* GL_RG8          */,         ///<
    RGB8 = 32849 /* GL_RGB8         */,        ///<
    RGBA8 = 32856 /* GL_RGBA8        */,       ///<
    SRGB8 = 35905 /* GL_SRGB8        */,       ///<
    SRGBA8 = 35907 /* GL_SRGB8_ALPHA8 */,      ///<
    R8I = 33329 /* GL_R8I          */,         ///<
    RG8I = 33335 /* GL_RG8I         */,        ///<
    RGB8I = 36239 /* GL_RGB8I        */,       ///<
    RGBA8I = 36238 /* GL_RGBA8I       */,      ///<
    R8UI = 33330 /* GL_R8UI         */,        ///<
    RG8UI = 33336 /* GL_RG8UI        */,       ///<
    RGB8UI = 36221 /* GL_RGB8UI       */,      ///<
    RGBA8UI = 36220 /* GL_RGBA8UI      */,     ///<
    R8_SNORM = 36756 /* GL_R8_SNORM     */,    ///<
    RG8_SNORM = 36757 /* GL_RG8_SNORM    */,   ///<
    RGB8_SNORM = 36758 /* GL_RGB8_SNORM   */,  ///<
    RGBA8_SNORM = 36759 /* GL_RGBA8_SNORM  */, ///<

    R16 = 33322 /* GL_R16          */,          ///<
    RG16 = 33324 /* GL_RG16         */,         ///<
    RGBA16 = 32859 /* GL_RGBA16       */,       ///<
    R16I = 33331 /* GL_R16I         */,         ///<
    RG16I = 33337 /* GL_RG16I        */,        ///<
    RGB16I = 36233 /* GL_RGB16I       */,       ///<
    RGBA16I = 36232 /* GL_RGBA16I      */,      ///<
    R16UI = 33332 /* GL_R16UI        */,        ///<
    RG16UI = 33338 /* GL_RG16UI       */,       ///<
    RGB16UI = 36215 /* GL_RGB16UI      */,      ///<
    RGBA16UI = 36214 /* GL_RGBA16UI     */,     ///<
    R16F = 33325 /* GL_R16F         */,         ///<
    RG16F = 33327 /* GL_RG16F        */,        ///<
    RGB16F = 34843 /* GL_RGB16F       */,       ///<
    RGBA16F = 34842 /* GL_RGBA16F      */,      ///<
    R16_SNORM = 36760 /* GL_R16_SNORM    */,    ///<
    RG16_SNORM = 36761 /* GL_RG16_SNORM   */,   ///<
    RGB16_SNORM = 36762 /* GL_RGB16_SNORM  */,  ///<
    RGBA16_SNORM = 36763 /* GL_RGBA16_SNORM */, ///<

    R32I = 33333 /* GL_R32I     */,     ///<
    RG32I = 33339 /* GL_RG32I    */,    ///<
    RGB32I = 36227 /* GL_RGB32I   */,   ///<
    RGBA32I = 36226 /* GL_RGBA32I  */,  ///<
    R32UI = 33334 /* GL_R32UI    */,    ///<
    RG32UI = 33340 /* GL_RG32UI   */,   ///<
    RGB32UI = 36209 /* GL_RGB32UI  */,  ///<
    RGBA32UI = 36208 /* GL_RGBA32UI */, ///<
    R32F = 33326 /* GL_R32F     */,     ///<
    RG32F = 33328 /* GL_RG32F    */,    ///<
    RGB32F = 34837 /* GL_RGB32F   */,   ///<
    RGBA32F = 34836 /* GL_RGBA32F  */,  ///<

    RGB10_A2 = 32857 /* GL_RGB10_A2       */,       ///<
    RGB10_A2UI = 36975 /* GL_RGB10_A2UI     */,     ///<
    R11F_G11F_B10F = 35898 /* GL_R11F_G11F_B10F */, ///<

    DEPTH16 = 33189 /* GL_DEPTH_COMPONENT16  */,          ///<
    DEPTH24 = 33190 /* GL_DEPTH_COMPONENT24  */,          ///<
    DEPTH24_STENCIL8 = 35056 /* GL_DEPTH24_STENCIL8   */, ///<
    DEPTH32 = 33191 /* GL_DEPTH_COMPONENT32  */,          ///<
    DEPTH32F = 36012 /* GL_DEPTH_COMPONENT32F */,         ///<
    DEPTH32F_STENCIL8 = 36013 /* GL_DEPTH32F_STENCIL8  */ ///<
};

enum class PixelDataType : uint32_t {
    UBYTE = 5121 /* GL_UNSIGNED_BYTE */, ///< Unsigned byte data type.
    FLOAT = 5126 /* GL_FLOAT         */  ///< Single precision floating-point data type.
};

enum class ImageAccess : uint32_t {
    READ = 35000 /* GL_READ_ONLY  */,      ///< Read-only image access.
    WRITE = 35001 /* GL_WRITE_ONLY */,     ///< Write-only image access.
    READ_WRITE = 35002 /* GL_READ_WRITE */ ///< Both read & write image access.
};

enum class ImageInternalFormat : uint32_t {
#if !defined(USE_OPENGL_ES)
    R8 = static_cast<uint32_t>(TextureInternalFormat::R8),   ///<
    RG8 = static_cast<uint32_t>(TextureInternalFormat::RG8), ///<
#endif
    RGBA8 = static_cast<uint32_t>(TextureInternalFormat::RGBA8), ///<
#if !defined(USE_OPENGL_ES)
    R8I = static_cast<uint32_t>(TextureInternalFormat::R8I),   ///<
    RG8I = static_cast<uint32_t>(TextureInternalFormat::RG8I), ///<
#endif
    RGBA8I = static_cast<uint32_t>(TextureInternalFormat::RGBA8I), ///<
#if !defined(USE_OPENGL_ES)
    R8UI = static_cast<uint32_t>(TextureInternalFormat::R8UI),   ///<
    RG8UI = static_cast<uint32_t>(TextureInternalFormat::RG8UI), ///<
#endif
    RGBA8UI = static_cast<uint32_t>(TextureInternalFormat::RGBA8UI), ///<
#if !defined(USE_OPENGL_ES)
    R8_SNORM = static_cast<uint32_t>(TextureInternalFormat::R8_SNORM),   ///<
    RG8_SNORM = static_cast<uint32_t>(TextureInternalFormat::RG8_SNORM), ///<
#endif
    RGBA8_SNORM = static_cast<uint32_t>(TextureInternalFormat::RGBA8_SNORM), ///<

#if !defined(USE_OPENGL_ES)
    R16 = static_cast<uint32_t>(TextureInternalFormat::R16),       ///<
    RG16 = static_cast<uint32_t>(TextureInternalFormat::RG16),     ///<
    RGBA16 = static_cast<uint32_t>(TextureInternalFormat::RGBA16), ///<
    R16I = static_cast<uint32_t>(TextureInternalFormat::R16I),     ///<
    RG16I = static_cast<uint32_t>(TextureInternalFormat::RG16I),   ///<
#endif
    RGBA16I = static_cast<uint32_t>(TextureInternalFormat::RGBA16I), ///<
#if !defined(USE_OPENGL_ES)
    R16UI = static_cast<uint32_t>(TextureInternalFormat::R16UI),   ///<
    RG16UI = static_cast<uint32_t>(TextureInternalFormat::RG16UI), ///<
#endif
    RGBA16UI = static_cast<uint32_t>(TextureInternalFormat::RGBA16UI), ///<
#if !defined(USE_OPENGL_ES)
    R16F = static_cast<uint32_t>(TextureInternalFormat::R16F),   ///<
    RG16F = static_cast<uint32_t>(TextureInternalFormat::RG16F), ///<
#endif
    RGBA16F = static_cast<uint32_t>(TextureInternalFormat::RGBA16F), ///<
#if !defined(USE_OPENGL_ES)
    R16_SNORM = static_cast<uint32_t>(TextureInternalFormat::R16_SNORM),       ///<
    RG16_SNORM = static_cast<uint32_t>(TextureInternalFormat::RG16_SNORM),     ///<
    RGBA16_SNORM = static_cast<uint32_t>(TextureInternalFormat::RGBA16_SNORM), ///<
#endif

    R32I = static_cast<uint32_t>(TextureInternalFormat::R32I), ///<
#if !defined(USE_OPENGL_ES)
    RG32I = static_cast<uint32_t>(TextureInternalFormat::RG32I), ///<
#endif
    RGBA32I = static_cast<uint32_t>(TextureInternalFormat::RGBA32I), ///<
    R32UI = static_cast<uint32_t>(TextureInternalFormat::R32UI),     ///<
#if !defined(USE_OPENGL_ES)
    RG32UI = static_cast<uint32_t>(TextureInternalFormat::RG32UI), ///<
#endif
    RGBA32UI = static_cast<uint32_t>(TextureInternalFormat::RGBA32UI), ///<
    R32F = static_cast<uint32_t>(TextureInternalFormat::R32F),         ///<
#if !defined(USE_OPENGL_ES)
    RG32F = static_cast<uint32_t>(TextureInternalFormat::RG32F), ///<
#endif
    RGBA32F = static_cast<uint32_t>(TextureInternalFormat::RGBA32F), ///<

#if !defined(USE_OPENGL_ES)
    RGB10_A2 = static_cast<uint32_t>(TextureInternalFormat::RGB10_A2),            ///<
    RGB10_A2UI = static_cast<uint32_t>(TextureInternalFormat::RGB10_A2UI),        ///<
    R11F_G11F_B10F = static_cast<uint32_t>(TextureInternalFormat::R11F_G11F_B10F) ///<
#endif
};

enum class ProgramParameter : uint32_t {
    DELETE_STATUS = 35712 /* GL_DELETE_STATUS                         */,                         ///<
    LINK_STATUS = 35714 /* GL_LINK_STATUS                           */,                           ///<
    VALIDATE_STATUS = 35715 /* GL_VALIDATE_STATUS                       */,                       ///<
    INFO_LOG_LENGTH = 35716 /* GL_INFO_LOG_LENGTH                       */,                       ///<
    ATTACHED_SHADERS = 35717 /* GL_ATTACHED_SHADERS                      */,                      ///<
    ACTIVE_ATTRIBUTES = 35721 /* GL_ACTIVE_ATTRIBUTES                     */,                     ///<
    ACTIVE_ATTRIBUTE_MAX_LENGTH = 35722 /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH           */,           ///<
    ACTIVE_UNIFORMS = 35718 /* GL_ACTIVE_UNIFORMS                       */,                       ///<
    ACTIVE_UNIFORM_MAX_LENGTH = 35719 /* GL_ACTIVE_UNIFORM_MAX_LENGTH             */,             ///<
    ACTIVE_UNIFORM_BLOCKS = 35382 /* GL_ACTIVE_UNIFORM_BLOCKS                 */,                 ///<
    ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 35381 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH  */,  ///<
    TRANSFORM_FEEDBACK_BUFFER_MODE = 35967 /* GL_TRANSFORM_FEEDBACK_BUFFER_MODE        */,        ///<
    TRANSFORM_FEEDBACK_VARYINGS = 35971 /* GL_TRANSFORM_FEEDBACK_VARYINGS           */,           ///<
    TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 35958 /* GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH */, ///<
    GEOMETRY_VERTICES_OUT = 35094 /* GL_GEOMETRY_VERTICES_OUT                 */,                 ///<
    GEOMETRY_INPUT_TYPE = 35095 /* GL_GEOMETRY_INPUT_TYPE                   */,                   ///<
    GEOMETRY_OUTPUT_TYPE = 35096 /* GL_GEOMETRY_OUTPUT_TYPE                  */                   ///<
};

enum class ShaderType : uint32_t {
    VERTEX = 35633 /* GL_VERTEX_SHADER          */, ///< Vertex shader.
    TESSELLATION_CONTROL =
        36488 /* GL_TESS_CONTROL_SHADER    */, ///< Tessellation control shader. Requires OpenGL 4.0+.
    TESSELLATION_EVALUATION =
        36487 /* GL_TESS_EVALUATION_SHADER */,        ///< Tessellation evaluation shader. Requires OpenGL 4.0+.
    FRAGMENT = 35632 /* GL_FRAGMENT_SHADER        */, ///< Fragment shader.
    GEOMETRY = 36313 /* GL_GEOMETRY_SHADER        */, ///< Geometry shader.
    COMPUTE = 37305 /* GL_COMPUTE_SHADER         */   ///< Compute shader. Requires OpenGL 4.3+.
};

enum class ShaderInfo : uint32_t {
    TYPE = 35663 /* GL_SHADER_TYPE          */,            ///<
    DELETE_STATUS = 35712 /* GL_DELETE_STATUS        */,   ///<
    COMPILE_STATUS = 35713 /* GL_COMPILE_STATUS       */,  ///<
    INFO_LOG_LENGTH = 35716 /* GL_INFO_LOG_LENGTH      */, ///<
    SOURCE_LENGTH = 35720 /* GL_SHADER_SOURCE_LENGTH */    ///<
};

enum class UniformType : uint32_t {
    // Primitive types
    FLOAT = 5126 /* GL_FLOAT        */, ///< Single precision floating-point value.
#if !defined(USE_OPENGL_ES)
    DOUBLE = 5130 /* GL_DOUBLE       */, ///< Double precision floating-point value. Requires OpenGL 4.1+.
#endif
    INT = 5124 /* GL_INT          */,   ///< Integer value.
    UINT = 5125 /* GL_UNSIGNED_INT */,  ///< Unsigned integer value.
    BOOL = 35670 /* GL_BOOL         */, ///< Boolean value.

    // Vectors
    VEC2 = 35664 /* GL_FLOAT_VEC2        */, ///< Single precision floating-point 2D vector.
    VEC3 = 35665 /* GL_FLOAT_VEC3        */, ///< Single precision floating-point 3D vector.
    VEC4 = 35666 /* GL_FLOAT_VEC4        */, ///< Single precision floating-point 4D vector.
#if !defined(USE_OPENGL_ES)
    DVEC2 = 36860 /* GL_DOUBLE_VEC2       */, ///< Double precision floating-point 2D vector. Requires OpenGL 4.1+.
    DVEC3 = 36861 /* GL_DOUBLE_VEC3       */, ///< Double precision floating-point 3D vector. Requires OpenGL 4.1+.
    DVEC4 = 36862 /* GL_DOUBLE_VEC4       */, ///< Double precision floating-point 4D vector. Requires OpenGL 4.1+.
#endif
    IVEC2 = 35667 /* GL_INT_VEC2          */, ///<
    IVEC3 = 35668 /* GL_INT_VEC3          */, ///<
    IVEC4 = 35669 /* GL_INT_VEC4          */, ///<
    UVEC2 = 36294 /* GL_UNSIGNED_INT_VEC2 */, ///<
    UVEC3 = 36295 /* GL_UNSIGNED_INT_VEC3 */, ///<
    UVEC4 = 36296 /* GL_UNSIGNED_INT_VEC4 */, ///<
    BVEC2 = 35671 /* GL_BOOL_VEC2         */, ///<
    BVEC3 = 35672 /* GL_BOOL_VEC3         */, ///<
    BVEC4 = 35673 /* GL_BOOL_VEC4         */, ///<

    // Matrices
    MAT2 = 35674 /* GL_FLOAT_MAT2    */,   ///< Single precision floating-point 2x2 matrix.
    MAT3 = 35675 /* GL_FLOAT_MAT3    */,   ///< Single precision floating-point 3x3 matrix.
    MAT4 = 35676 /* GL_FLOAT_MAT4    */,   ///< Single precision floating-point 4x4 matrix.
    MAT2x3 = 35685 /* GL_FLOAT_MAT2x3  */, ///< Single precision floating-point 2x3 matrix.
    MAT2x4 = 35686 /* GL_FLOAT_MAT2x4  */, ///< Single precision floating-point 2x4 matrix.
    MAT3x2 = 35687 /* GL_FLOAT_MAT3x2  */, ///< Single precision floating-point 3x2 matrix.
    MAT3x4 = 35688 /* GL_FLOAT_MAT3x4  */, ///< Single precision floating-point 3x4 matrix.
    MAT4x2 = 35689 /* GL_FLOAT_MAT4x2  */, ///< Single precision floating-point 4x2 matrix.
    MAT4x3 = 35690 /* GL_FLOAT_MAT4x3  */, ///< Single precision floating-point 4x3 matrix.
#if !defined(USE_OPENGL_ES)
    DMAT2 = 36678 /* GL_DOUBLE_MAT2   */,   ///< Double precision floating-point 2x2 matrix. Requires OpenGL 4.1+.
    DMAT3 = 36679 /* GL_DOUBLE_MAT3   */,   ///< Double precision floating-point 3x3 matrix. Requires OpenGL 4.1+.
    DMAT4 = 36680 /* GL_DOUBLE_MAT4   */,   ///< Double precision floating-point 4x4 matrix. Requires OpenGL 4.1+.
    DMAT2x3 = 36681 /* GL_DOUBLE_MAT2x3 */, ///< Double precision floating-point 2x3 matrix. Requires OpenGL 4.1+.
    DMAT2x4 = 36682 /* GL_DOUBLE_MAT2x4 */, ///< Double precision floating-point 2x4 matrix. Requires OpenGL 4.1+.
    DMAT3x2 = 36683 /* GL_DOUBLE_MAT3x2 */, ///< Double precision floating-point 3x2 matrix. Requires OpenGL 4.1+.
    DMAT3x4 = 36684 /* GL_DOUBLE_MAT3x4 */, ///< Double precision floating-point 3x4 matrix. Requires OpenGL 4.1+.
    DMAT4x2 = 36685 /* GL_DOUBLE_MAT4x2 */, ///< Double precision floating-point 4x2 matrix. Requires OpenGL 4.1+.
    DMAT4x3 = 36686 /* GL_DOUBLE_MAT4x3 */, ///< Double precision floating-point 4x3 matrix. Requires OpenGL 4.1+.
#endif

// Samplers
#if !defined(USE_OPENGL_ES)
    SAMPLER_1D = 35677 /* GL_SAMPLER_1D                                */, ///<
#endif
    SAMPLER_2D = 35678 /* GL_SAMPLER_2D                                */,   ///<
    SAMPLER_3D = 35679 /* GL_SAMPLER_3D                                */,   ///<
    SAMPLER_CUBE = 35680 /* GL_SAMPLER_CUBE                              */, ///<
#if !defined(USE_OPENGL_ES)
    SAMPLER_1D_SHADOW = 35681 /* GL_SAMPLER_1D_SHADOW                         */, ///<
#endif
    SAMPLER_2D_SHADOW = 35682 /* GL_SAMPLER_2D_SHADOW                         */, ///<
#if !defined(USE_OPENGL_ES)
    SAMPLER_1D_ARRAY = 36288 /* GL_SAMPLER_1D_ARRAY                          */, ///<
#endif
    SAMPLER_2D_ARRAY = 36289 /* GL_SAMPLER_2D_ARRAY                          */, ///<
#if !defined(USE_OPENGL_ES)
    SAMPLER_1D_ARRAY_SHADOW = 36291 /* GL_SAMPLER_1D_ARRAY_SHADOW                   */, ///<
#endif
    SAMPLER_2D_ARRAY_SHADOW = 36292 /* GL_SAMPLER_2D_ARRAY_SHADOW                   */, ///<
#if !defined(USE_OPENGL_ES)
    SAMPLER_2D_MULTISAMPLE = 37128 /* GL_SAMPLER_2D_MULTISAMPLE                    */,       ///<
    SAMPLER_2D_MULTISAMPLE_ARRAY = 37131 /* GL_SAMPLER_2D_MULTISAMPLE_ARRAY              */, ///<
#endif
    SAMPLER_CUBE_SHADOW = 36293 /* GL_SAMPLER_CUBE_SHADOW                       */, ///<
#if !defined(USE_OPENGL_ES)
    SAMPLER_BUFFER = 36290 /* GL_SAMPLER_BUFFER                            */,         ///<
    SAMPLER_2D_RECT = 35683 /* GL_SAMPLER_2D_RECT                           */,        ///<
    SAMPLER_2D_RECT_SHADOW = 35684 /* GL_SAMPLER_2D_RECT_SHADOW                    */, ///<
    INT_SAMPLER_1D = 36297 /* GL_INT_SAMPLER_1D                            */,         ///<
#endif
    INT_SAMPLER_2D = 36298 /* GL_INT_SAMPLER_2D                            */,   ///<
    INT_SAMPLER_3D = 36299 /* GL_INT_SAMPLER_3D                            */,   ///<
    INT_SAMPLER_CUBE = 36300 /* GL_INT_SAMPLER_CUBE                          */, ///<
#if !defined(USE_OPENGL_ES)
    INT_SAMPLER_1D_ARRAY = 36302 /* GL_INT_SAMPLER_1D_ARRAY                      */, ///<
#endif
    INT_SAMPLER_2D_ARRAY = 36303 /* GL_INT_SAMPLER_2D_ARRAY                      */, ///<
#if !defined(USE_OPENGL_ES)
    INT_SAMPLER_2D_MULTISAMPLE = 37129 /* GL_INT_SAMPLER_2D_MULTISAMPLE                */,       ///<
    INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 37132 /* GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY          */, ///<
    INT_SAMPLER_BUFFER = 36304 /* GL_INT_SAMPLER_BUFFER                        */,               ///<
    INT_SAMPLER_2D_RECT = 36301 /* GL_INT_SAMPLER_2D_RECT                       */,              ///<
    UINT_SAMPLER_1D = 36305 /* GL_UNSIGNED_INT_SAMPLER_1D                   */,                  ///<
#endif
    UINT_SAMPLER_2D = 36306 /* GL_UNSIGNED_INT_SAMPLER_2D                   */,   ///<
    UINT_SAMPLER_3D = 36307 /* GL_UNSIGNED_INT_SAMPLER_3D                   */,   ///<
    UINT_SAMPLER_CUBE = 36308 /* GL_UNSIGNED_INT_SAMPLER_CUBE                 */, ///<
#if !defined(USE_OPENGL_ES)
    UINT_SAMPLER_1D_ARRAY = 36310 /* GL_UNSIGNED_INT_SAMPLER_1D_ARRAY             */, ///<
#endif
    UINT_SAMPLER_2D_ARRAY = 36311 /* GL_UNSIGNED_INT_SAMPLER_2D_ARRAY             */, ///<
#if !defined(USE_OPENGL_ES)
    UINT_SAMPLER_2D_MULTISAMPLE = 37130 /* GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE       */,       ///<
    UINT_SAMPLER_2D_MULTISAMPLE_ARRAY = 37133 /* GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY */, ///<
    UINT_SAMPLER_BUFFER = 36312 /* GL_UNSIGNED_INT_SAMPLER_BUFFER               */,               ///<
    UINT_SAMPLER_2D_RECT = 36309 /* GL_UNSIGNED_INT_SAMPLER_2D_RECT              */,              ///<
#endif

// Images
#if !defined(USE_OPENGL_ES)
    IMAGE_1D = 36940 /* GL_IMAGE_1D                                */, ///< 1D image. Requires OpenGL 4.2+.
    IMAGE_2D = 36941 /* GL_IMAGE_2D                                */, ///< 2D image. Requires OpenGL 4.2+.
    IMAGE_3D = 36942 /* GL_IMAGE_3D                                */, ///< 3D image. Requires OpenGL 4.2+.
    IMAGE_2D_RECT =
        36943 /* GL_IMAGE_2D_RECT                           */,          ///< 2D rectangle image. Requires OpenGL 4.2+.
    IMAGE_CUBE = 36944 /* GL_IMAGE_CUBE                              */, ///< Cube image. Requires OpenGL 4.2+.
    IMAGE_BUFFER = 36945 /* GL_IMAGE_BUFFER                            */,   ///< Buffer image. Requires OpenGL 4.2+.
    IMAGE_1D_ARRAY = 36946 /* GL_IMAGE_1D_ARRAY                          */, ///< 1D array image. Requires OpenGL 4.2+.
    IMAGE_2D_ARRAY = 36947 /* GL_IMAGE_2D_ARRAY                          */, ///< 2D array image. Requires OpenGL 4.2+.
    IMAGE_2D_MULTISAMPLE =
        36949 /* GL_IMAGE_2D_MULTISAMPLE                    */, ///< 2D multisample image. Requires OpenGL 4.2+.
    IMAGE_2D_MULTISAMPLE_ARRAY =
        36950 /* GL_IMAGE_2D_MULTISAMPLE_ARRAY              */, ///< 2D array multisample image. Requires OpenGL 4.2+.
    INT_IMAGE_1D = 36951 /* GL_INT_IMAGE_1D                            */, ///< 1D integer image. Requires OpenGL 4.2+.
    INT_IMAGE_2D = 36952 /* GL_INT_IMAGE_2D                            */, ///< 2D integer image. Requires OpenGL 4.2+.
    INT_IMAGE_3D = 36953 /* GL_INT_IMAGE_3D                            */, ///< 3D integer image. Requires OpenGL 4.2+.
    INT_IMAGE_2D_RECT =
        36954 /* GL_INT_IMAGE_2D_RECT                       */, ///< 2D rectangle integer image. Requires OpenGL 4.2+.
    INT_IMAGE_CUBE =
        36955 /* GL_INT_IMAGE_CUBE                          */, ///< Cube integer image. Requires OpenGL 4.2+.
    INT_IMAGE_BUFFER =
        36956 /* GL_INT_IMAGE_BUFFER                        */, ///< Integer buffer image. Requires OpenGL 4.2+.
    INT_IMAGE_1D_ARRAY =
        36957 /* GL_INT_IMAGE_1D_ARRAY                      */, ///< 1D array integer image. Requires OpenGL 4.2+.
    INT_IMAGE_2D_ARRAY =
        36958 /* GL_INT_IMAGE_2D_ARRAY                      */, ///< 2D array integer image. Requires OpenGL 4.2+.
    INT_IMAGE_2D_MULTISAMPLE =
        36960 /* GL_INT_IMAGE_2D_MULTISAMPLE                */, ///< 2D multisample integer image. Requires OpenGL 4.2+.
    INT_IMAGE_2D_MULTISAMPLE_ARRAY =
        36961 /* GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY          */, ///< 2D array multisample integer image. Requires
                                                                ///< OpenGL 4.2+.
    UINT_IMAGE_1D =
        36962 /* GL_UNSIGNED_INT_IMAGE_1D                   */, ///< 1D uinteger image. Requires OpenGL 4.2+.
    UINT_IMAGE_2D =
        36963 /* GL_UNSIGNED_INT_IMAGE_2D                   */, ///< 2D uinteger image. Requires OpenGL 4.2+.
    UINT_IMAGE_3D =
        36964 /* GL_UNSIGNED_INT_IMAGE_3D                   */, ///< 3D uinteger image. Requires OpenGL 4.2+.
    UINT_IMAGE_2D_RECT = 36965 /* GL_UNSIGNED_INT_IMAGE_2D_RECT              */, ///< 2D rectangle uint image.
                                                                                 ///< Requires OpenGL 4.2+.
    UINT_IMAGE_CUBE =
        36966 /* GL_UNSIGNED_INT_IMAGE_CUBE                 */, ///< Cube uinteger image. Requires OpenGL 4.2+.
    UINT_IMAGE_BUFFER = 36967 /* GL_UNSIGNED_INT_IMAGE_BUFFER               */,   ///< Unsigned integer buffer image.
                                                                                  ///< Requires OpenGL 4.2+.
    UINT_IMAGE_1D_ARRAY = 36968 /* GL_UNSIGNED_INT_IMAGE_1D_ARRAY             */, ///< 1D array uinteger image.
                                                                                  ///< Requires OpenGL 4.2+.
    UINT_IMAGE_2D_ARRAY = 36969 /* GL_UNSIGNED_INT_IMAGE_2D_ARRAY             */, ///< 2D array uinteger image.
                                                                                  ///< Requires OpenGL 4.2+.
    UINT_IMAGE_2D_MULTISAMPLE =
        36971 /* GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE       */, ///< 2D multisample uinteger image. Requires
                                                                ///< OpenGL 4.2+.
    UINT_IMAGE_2D_MULTISAMPLE_ARRAY =
        36972 /* GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY */, ///< 2D array multisample uinteger image.
                                                                ///< Requires OpenGL 4.2+.
#endif

// Misc
#if !defined(USE_OPENGL_ES)
    UINT_ATOMIC_COUNTER = 37595 /* GL_UNSIGNED_INT_ATOMIC_COUNTER */ ///< . Requires OpenGL 4.2+.
#endif
};

enum class FramebufferType : uint32_t {
    READ_FRAMEBUFFER = 36008 /* GL_READ_FRAMEBUFFER */, ///<
    DRAW_FRAMEBUFFER = 36009 /* GL_DRAW_FRAMEBUFFER */, ///<
    FRAMEBUFFER = 36160 /* GL_FRAMEBUFFER      */       ///<
};

enum class FramebufferStatus : uint32_t {
    COMPLETE = 36053 /* GL_FRAMEBUFFER_COMPLETE                      */,                      ///<
    UNDEFINED = 33305 /* GL_FRAMEBUFFER_UNDEFINED                     */,                     ///<
    INCOMPLETE_ATTACHMENT = 36054 /* GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         */,         ///<
    INCOMPLETE_MISSING_ATTACHMENT = 36055 /* GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT */, ///<
    INCOMPLETE_DRAW_BUFFER = 36059 /* GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER        */,        ///<
    INCOMPLETE_READ_BUFFER = 36060 /* GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER        */,        ///<
    UNSUPPORTED = 36061 /* GL_FRAMEBUFFER_UNSUPPORTED                   */,                   ///<
    INCOMPLETE_MULTISAMPLE = 36182 /* GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE        */,        ///<
    INCOMPLETE_LAYER_TARGETS = 36264 /* GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS      */       ///<
};

enum class FramebufferAttachment : uint32_t {
    DEPTH = 36096 /* GL_DEPTH_ATTACHMENT         */,         ///< Depth attachment.
    STENCIL = 36128 /* GL_STENCIL_ATTACHMENT       */,       ///< Stencil attachment.
    DEPTH_STENCIL = 33306 /* GL_DEPTH_STENCIL_ATTACHMENT */, ///< Depth & stencil attachment.

    COLOR0 = 36064 /* GL_COLOR_ATTACHMENT0 */, ///< Color attachment 0.
    COLOR1 = 36065 /* GL_COLOR_ATTACHMENT1 */, ///< Color attachment 1.
    COLOR2 = 36066 /* GL_COLOR_ATTACHMENT2 */, ///< Color attachment 2.
    COLOR3 = 36067 /* GL_COLOR_ATTACHMENT3 */, ///< Color attachment 3.
    COLOR4 = 36068 /* GL_COLOR_ATTACHMENT4 */, ///< Color attachment 4.
    COLOR5 = 36069 /* GL_COLOR_ATTACHMENT5 */, ///< Color attachment 5.
    COLOR6 = 36070 /* GL_COLOR_ATTACHMENT6 */, ///< Color attachment 6.
    COLOR7 = 36071 /* GL_COLOR_ATTACHMENT7 */, ///< Color attachment 7.

// Default framebuffer attachments
#if !defined(USE_WEBGL)
#if !defined(USE_OPENGL_ES)
    DEFAULT_FRONT_LEFT = 1024 /* GL_FRONT_LEFT  */,  ///<
    DEFAULT_FRONT_RIGHT = 1025 /* GL_FRONT_RIGHT */, ///<
    DEFAULT_BACK_LEFT = 1026 /* GL_BACK_LEFT   */,   ///<
    DEFAULT_BACK_RIGHT = 1027 /* GL_BACK_RIGHT  */,  ///<
#else
    DEFAULT_BACK = 1029 /* GL_BACK        */, ///<
#endif
    DEFAULT_DEPTH = 6145 /* GL_DEPTH       */,  ///<
    DEFAULT_STENCIL = 6146 /* GL_STENCIL     */ ///<
#endif
};

enum class FramebufferAttachmentParam : uint32_t {
    RED_SIZE = 33298 /* GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE              */,              ///<
    GREEN_SIZE = 33299 /* GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE            */,            ///<
    BLUE_SIZE = 33300 /* GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE             */,             ///<
    ALPHA_SIZE = 33301 /* GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE            */,            ///<
    DEPTH_SIZE = 33302 /* GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE            */,            ///<
    STENCIL_SIZE = 33303 /* GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE          */,          ///<
    COMPONENT_TYPE = 33297 /* GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE        */,        ///<
    COLOR_ENCODING = 33296 /* GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING        */,        ///<
    OBJECT_TYPE = 36048 /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           */,           ///<
    OBJECT_NAME = 36049 /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           */,           ///<
    TEXTURE_LEVEL = 36050 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         */,         ///<
    TEXTURE_CUBE_MAP_FACE = 36051 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE */, ///<
    TEXTURE_LAYER = 36052 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER         */,         ///<
    LAYERED = 36263 /* GL_FRAMEBUFFER_ATTACHMENT_LAYERED               */                ///<
};

enum class ReadBuffer : uint32_t {
    NONE = 0 /* GL_NONE */,    ///<
    BACK = 1029 /* GL_BACK */, ///<

#if !defined(USE_OPENGL_ES)
    FRONT = 1028 /* GL_FRONT       */, ///<
    LEFT = 1030 /* GL_LEFT        */,  ///<
    RIGHT = 1031 /* GL_RIGHT       */, ///<

    FRONT_LEFT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_FRONT_LEFT),   ///<
    FRONT_RIGHT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_FRONT_RIGHT), ///<
    BACK_LEFT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_BACK_LEFT),     ///<
    BACK_RIGHT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_BACK_RIGHT),   ///<
#endif

    COLOR_ATTACHMENT0 = static_cast<uint32_t>(FramebufferAttachment::COLOR0), ///< Color attachment 0.
    COLOR_ATTACHMENT1 = static_cast<uint32_t>(FramebufferAttachment::COLOR1), ///< Color attachment 1.
    COLOR_ATTACHMENT2 = static_cast<uint32_t>(FramebufferAttachment::COLOR2), ///< Color attachment 2.
    COLOR_ATTACHMENT3 = static_cast<uint32_t>(FramebufferAttachment::COLOR3), ///< Color attachment 3.
    COLOR_ATTACHMENT4 = static_cast<uint32_t>(FramebufferAttachment::COLOR4), ///< Color attachment 4.
    COLOR_ATTACHMENT5 = static_cast<uint32_t>(FramebufferAttachment::COLOR5), ///< Color attachment 5.
    COLOR_ATTACHMENT6 = static_cast<uint32_t>(FramebufferAttachment::COLOR6), ///< Color attachment 6.
    COLOR_ATTACHMENT7 = static_cast<uint32_t>(FramebufferAttachment::COLOR7)  ///< Color attachment 7.
};

enum class DrawBuffer : uint32_t {
    NONE = static_cast<uint32_t>(ReadBuffer::NONE), ///<

#if !defined(USE_OPENGL_ES)
    FRONT_LEFT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_FRONT_LEFT),   ///<
    FRONT_RIGHT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_FRONT_RIGHT), ///<
    BACK_LEFT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_BACK_LEFT),     ///<
    BACK_RIGHT = static_cast<uint32_t>(FramebufferAttachment::DEFAULT_BACK_RIGHT),   ///<
#else
    BACK = static_cast<uint32_t>(ReadBuffer::BACK), ///<
#endif

    COLOR_ATTACHMENT0 = static_cast<uint32_t>(FramebufferAttachment::COLOR0), ///< Color attachment 0.
    COLOR_ATTACHMENT1 = static_cast<uint32_t>(FramebufferAttachment::COLOR1), ///< Color attachment 1.
    COLOR_ATTACHMENT2 = static_cast<uint32_t>(FramebufferAttachment::COLOR2), ///< Color attachment 2.
    COLOR_ATTACHMENT3 = static_cast<uint32_t>(FramebufferAttachment::COLOR3), ///< Color attachment 3.
    COLOR_ATTACHMENT4 = static_cast<uint32_t>(FramebufferAttachment::COLOR4), ///< Color attachment 4.
    COLOR_ATTACHMENT5 = static_cast<uint32_t>(FramebufferAttachment::COLOR5), ///< Color attachment 5.
    COLOR_ATTACHMENT6 = static_cast<uint32_t>(FramebufferAttachment::COLOR6), ///< Color attachment 6.
    COLOR_ATTACHMENT7 = static_cast<uint32_t>(FramebufferAttachment::COLOR7)  ///< Color attachment 7.
};

enum class BlitFilter : uint32_t {
    NEAREST = static_cast<uint32_t>(TextureParamValue::NEAREST), ///<
    LINEAR = static_cast<uint32_t>(TextureParamValue::LINEAR)    ///<
};

enum class PrimitiveType : uint32_t {
    POINTS = 0 /* GL_POINTS                   */,     ///<
    LINES = 1 /* GL_LINES                    */,      ///<
    LINE_LOOP = 2 /* GL_LINE_LOOP                */,  ///<
    LINE_STRIP = 3 /* GL_LINE_STRIP               */, ///<
#if !defined(USE_OPENGL_ES)
    LINES_ADJACENCY = 10 /* GL_LINES_ADJACENCY          */,      ///<
    LINE_STRIP_ADJACENCY = 11 /* GL_LINE_STRIP_ADJACENCY     */, ///<
#endif
    TRIANGLES = 4 /* GL_TRIANGLES                */,      ///<
    TRIANGLE_STRIP = 5 /* GL_TRIANGLE_STRIP           */, ///<
    TRIANGLE_FAN = 6 /* GL_TRIANGLE_FAN             */,   ///<
#if !defined(USE_OPENGL_ES)
    TRIANGLES_ADJACENCY = 12 /* GL_TRIANGLES_ADJACENCY      */,      ///<
    TRIANGLE_STRIP_ADJACENCY = 13 /* GL_TRIANGLE_STRIP_ADJACENCY */, ///<
    PATCHES = 14 /* GL_PATCHES                  */                   ///<
#endif
};

enum class ElementDataType : uint32_t {
    UBYTE = 5121 /* GL_UNSIGNED_BYTE  */,  ///<
    USHORT = 5123 /* GL_UNSIGNED_SHORT */, ///<
    UINT = 5125 /* GL_UNSIGNED_INT   */    ///<
};

enum class BarrierType : uint32_t {
    VERTEX_ATTRIB_ARRAY = 1 /* GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT */,   ///<
    ELEMENT_ARRAY = 2 /* GL_ELEMENT_ARRAY_BARRIER_BIT       */,         ///<
    UNIFORM = 4 /* GL_UNIFORM_BARRIER_BIT             */,               ///<
    TEXTURE_FETCH = 8 /* GL_TEXTURE_FETCH_BARRIER_BIT       */,         ///<
    SHADER_IMAGE_ACCESS = 32 /* GL_SHADER_IMAGE_ACCESS_BARRIER_BIT */,  ///<
    COMMAND = 64 /* GL_COMMAND_BARRIER_BIT             */,              ///<
    PIXEL_BUFFER = 128 /* GL_PIXEL_BUFFER_BARRIER_BIT        */,        ///<
    TEXTURE_UPDATE = 256 /* GL_TEXTURE_UPDATE_BARRIER_BIT      */,      ///<
    BUFFER_UPDATE = 512 /* GL_BUFFER_UPDATE_BARRIER_BIT       */,       ///<
    FRAMEBUFFER = 1024 /* GL_FRAMEBUFFER_BARRIER_BIT         */,        ///<
    TRANSFORM_FEEDBACK = 2048 /* GL_TRANSFORM_FEEDBACK_BARRIER_BIT  */, ///<
    ATOMIC_COUNTER = 4096 /* GL_ATOMIC_COUNTER_BARRIER_BIT      */,     ///<
    SHADER_STORAGE = 8192 /* GL_SHADER_STORAGE_BARRIER_BIT      */,     ///<
    ALL = 4294967295 /* GL_ALL_BARRIER_BITS                */           ///<
};
MAKE_ENUM_FLAG(BarrierType)

enum class RegionBarrierType : uint32_t {
    ATOMIC_COUNTER = static_cast<uint32_t>(BarrierType::ATOMIC_COUNTER),           ///<
    FRAMEBUFFER = static_cast<uint32_t>(BarrierType::FRAMEBUFFER),                 ///<
    SHADER_IMAGE_ACCESS = static_cast<uint32_t>(BarrierType::SHADER_IMAGE_ACCESS), ///<
    SHADER_STORAGE = static_cast<uint32_t>(BarrierType::SHADER_STORAGE),           ///<
    TEXTURE_FETCH = static_cast<uint32_t>(BarrierType::TEXTURE_FETCH),             ///<
    UNIFORM = static_cast<uint32_t>(BarrierType::UNIFORM),                         ///<
    ALL = static_cast<uint32_t>(BarrierType::ALL)                                  ///<
};
MAKE_ENUM_FLAG(RegionBarrierType)

enum class QueryType : uint32_t {
#if !defined(USE_OPENGL_ES)
    SAMPLES = 35092 /* GL_SAMPLES_PASSED                        */, ///<
#endif
    ANY_SAMPLES = 35887 /* GL_ANY_SAMPLES_PASSED                    */,              ///<
    ANY_SAMPLES_CONSERVATIVE = 36202 /* GL_ANY_SAMPLES_PASSED_CONSERVATIVE       */, ///<
#if !defined(USE_OPENGL_ES)
    PRIMITIVES = 35975 /* GL_PRIMITIVES_GENERATED                  */, ///<
#endif
    TRANSFORM_FEEDBACK_PRIMITIVES = 35976 /* GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN */, ///<
#if !defined(USE_OPENGL_ES)
    TIME_ELAPSED = 35007 /* GL_TIME_ELAPSED                          */ ///<
#endif
};

enum class RenderObjectType : uint32_t {
    BUFFER = 33504 /* GL_BUFFER             */,                                                    ///<
    TEXTURE = 5890 /* GL_TEXTURE            */,                                                    ///<
    SAMPLER = 33510 /* GL_SAMPLER            */,                                                   ///<
    SHADER = 33505 /* GL_SHADER             */,                                                    ///<
    PROGRAM = 33506 /* GL_PROGRAM            */,                                                   ///<
    QUERY = 33507 /* GL_QUERY              */,                                                     ///<
    FRAMEBUFFER = static_cast<uint32_t>(FramebufferType::FRAMEBUFFER) /* GL_FRAMEBUFFER        */, ///<
    RENDERBUFFER = 36161 /* GL_RENDERBUFFER       */,                                              ///<
    VERTEX_ARRAY = 32884 /* GL_VERTEX_ARRAY       */,                                              ///<
    PROGRAM_PIPELINE = 33508 /* GL_PROGRAM_PIPELINE   */,                                          ///<
    TRANSFORM_FEEDBACK = 36386 /* GL_TRANSFORM_FEEDBACK */                                         ///<
};

/// Possible errors obtained after any renderer call; see
/// https://www.khronos.org/opengl/wiki/OpenGL_Error#Meaning_of_errors for a more detailed description.
/// \see Renderer::recover_errors()
enum class ErrorCode : uint32_t {
    NONE = 0 /* GL_NO_ERROR                      */,                 ///< No error.
    INVALID_ENUM = 1280 /* GL_INVALID_ENUM                  */,      ///< Invalid enumeration value given.
    INVALID_VALUE = 1281 /* GL_INVALID_VALUE                 */,     ///< Invalid value given.
    INVALID_OPERATION = 1282 /* GL_INVALID_OPERATION             */, ///< Invalid parameters combination given.
    STACK_OVERFLOW = 1283 /* GL_STACK_OVERFLOW                */,    ///< Stack overflow.
    STACK_UNDERFLOW = 1284 /* GL_STACK_UNDERFLOW               */,   ///< Stack underflow.
    OUT_OF_MEMORY = 1285 /* GL_OUT_OF_MEMORY                 */,     ///< Out of memory; the result of the required
                                                                     ///< operation is undefined.
    INVALID_FRAMEBUFFER_OPERATION =
        1286 /* GL_INVALID_FRAMEBUFFER_OPERATION */, ///< Operation asked on an incomplete framebuffer.
    CONTEXT_LOST =
        1287 /* GL_CONTEXT_LOST                  */ ///< Context lost due to a GPU reset. Requires OpenGL 4.5+.
};

/// ErrorCodes structure, holding codes of errors that may have happened on prior Renderer calls.
struct ErrorCodes {
    std::bitset<8> codes{};

    /// Checks if no error code has been set.
    /// \return True if no code has been set, false otherwise.
    [[nodiscard]] bool empty() const { return codes.none(); }

    /// Checks if the given error code has been set.
    /// \param code Error code to be checked.
    /// \return True if the code has been set, false otherwise.
    [[nodiscard]] constexpr bool get(ErrorCode code) const
    {
        return codes[static_cast<uint32_t>(code) - static_cast<uint32_t>(ErrorCode::INVALID_ENUM)];
    }

    /// Checks if the given error code has been set.
    /// \param code Error code to be checked.
    /// \return True if the code has been set, false otherwise.
    [[nodiscard]] constexpr bool operator[](ErrorCode code) const { return get(code); }
};

class Renderer {
public:
    Renderer() = delete;
    Renderer(Renderer const&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer& operator=(Renderer const&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    ~Renderer() = delete;

    static void init();

    static bool is_initialized() { return initialized; }

    static int get_major_version() { return major_version; }

    static int get_minor_version() { return minor_version; }

    /// Checks that the current version is higher than or equal to the given one.
    /// \param major Major version.
    /// \param minor Minor version.
    /// \return True if the given version is higher than or equal to the current one, false otherwise.
    static bool check_version(int major, int minor)
    {
        return (major_version > major || (major_version == major && minor_version >= minor));
    }

    static bool is_extension_supported(std::string const& extension)
    {
        return (extensions.find(extension) != extensions.cend());
    }

    static TextureInternalFormat get_default_framebuffer_color_format() { return default_framebuffer_color; }

    static TextureInternalFormat get_default_framebuffer_depth_format() { return default_framebuffer_depth; }

    static void enable(Capability capability);

    static void disable(Capability capability);

    static bool is_enabled(Capability capability);

    static std::string get_context_info(ContextInfo info);

    /// Gets the name of the extension located at the given index.
    /// \see get_parameter(StateParameter::EXTENSION_COUNT)
    /// \param ext_index Index of the extension. Must be less than the total number of extensions.
    /// \return Name of the extension.
    static std::string get_extension(uint ext_index);

    static void get_parameter(StateParameter parameter, unsigned char* values);
    static void get_parameter(StateParameter parameter, int* values);
    static void get_parameter(StateParameter parameter, int64_t* values);
    static void get_parameter(StateParameter parameter, float* values);
    static void get_parameter(StateParameter parameter, double* values);
    static void get_parameter(StateParameter parameter, uint32_t index, unsigned char* values);
    static void get_parameter(StateParameter parameter, uint32_t index, int* values);
    static void get_parameter(StateParameter parameter, uint32_t index, int64_t* values);

    /// Gets the active texture's index.
    /// \note This returns an index starting from 0, not from GL_TEXTURE0.
    /// \return Index of the currently active texture.
    static uint32_t get_active_texture();

    /// Gets the current program's index.
    /// \return Index of the currently used program.
    static uint32_t get_current_program();

    static void clear_color(Color const& color);

    static void clear(MaskType mask);

    static void set_depth_function(DepthStencilFunction func);

    /// Sets the function to evaluate for stencil testing.
    /// \param func Function to be evaluated.
    /// \param ref Reference value to compare the stencil with.
    /// \param mask Bitmask to compare the reference & the stencil with.
    /// \param orientation Face orientation for which to evaluate the function.
    static void set_stencil_function(
        DepthStencilFunction func, int ref, uint32_t mask, FaceOrientation orientation = FaceOrientation::FRONT_BACK
    );

    /// Sets operations to perform on stencil tests.
    /// \param stencil_fail_op Action to be performed if the stencil test fails.
    /// \param depth_fail_op Action to be performed if the stencil test succeeds, but the depth test fails.
    /// \param success_op Action to be performed if both stencil & depth tests succeed, or if only the former does and
    /// there is no depth testing or no depth buffer.
    /// \param orientation Face orientation for which to set the operations.
    static void set_stencil_operations(
        StencilOperation stencil_fail_op, StencilOperation depth_fail_op, StencilOperation success_op,
        FaceOrientation orientation = FaceOrientation::FRONT_BACK
    );
    /// Enables overwriting stencil values to the bits represented by the given mask.
    /// \param mask Bitmask defining which stencil bits can be written.
    /// \param orientation Face orientation for which to set the mask.
    static void set_stencil_mask(uint32_t mask, FaceOrientation orientation = FaceOrientation::FRONT_BACK);
    static void set_blend_function(BlendFactor source, BlendFactor destination);
    static void set_face_cull(FaceOrientation orientation);
#if !defined(USE_OPENGL_ES)
    static void set_polygon_mode(FaceOrientation orientation, PolygonMode mode);
    static void set_clip_control(ClipOrigin origin, ClipDepth depth);
    static void set_patch_vertex_count(int value);
    static void set_patch_parameter(PatchParameter param, float const* values);
#endif
    static void set_pixel_storage(PixelStorage storage, uint32_t value);
    static void recover_frame(Vector2ui const& size, TextureFormat format, PixelDataType data_type, void* data);
    static void generate_vertex_arrays(uint32_t count, uint32_t* indices);
    static void generate_vertex_array(uint32_t& index) { generate_vertex_arrays(1, &index); }
    static void bind_vertex_array(uint32_t index);
    static void unbind_vertex_array() { bind_vertex_array(0); }
    static void enable_vertex_attrib_array(uint32_t index);
    static void set_vertex_attrib(
        uint32_t index, AttribDataType data_type, uint8_t size, uint32_t stride, uint32_t offset, bool normalize = false
    );
    static void set_vertex_attribDivisor(uint32_t index, uint32_t divisor);
    static void delete_vertex_arrays(uint32_t count, uint32_t* indices);
    static void delete_vertex_array(uint32_t& index) { delete_vertex_arrays(1, &index); }
    static void generate_buffers(uint32_t count, uint32_t* indices);
    template <size_t N>
    static void generate_buffers(uint32_t (&indices)[N])
    {
        generate_buffers(N, indices);
    }
    static void generate_buffer(uint32_t& index) { generate_buffers(1, &index); }
    static void bind_buffer(BufferType type, uint32_t index);
    static void unbind_buffer(BufferType type) { bind_buffer(type, 0); }
    static void bind_buffer_base(BufferType type, uint32_t binding_index, uint32_t buffer_index);
    static void bind_buffer_range(
        BufferType type, uint32_t binding_index, uint32_t buffer_index, std::ptrdiff_t offset, std::ptrdiff_t size
    );
    static void send_buffer_data(BufferType type, std::ptrdiff_t size, void const* data, BufferDataUsage usage);
    static void
    send_buffer_sub_data(BufferType type, std::ptrdiff_t offset, std::ptrdiff_t data_size, void const* data);
    template <typename T>
    static void send_buffer_sub_data(BufferType type, std::ptrdiff_t offset, T const& data)
    {
        send_buffer_sub_data(type, offset, sizeof(T), &data);
    }
    static void delete_buffers(uint32_t count, uint32_t* indices);
    template <size_t N>
    static void delete_buffers(uint32_t (&indices)[N])
    {
        delete_buffers(N, indices);
    }
    static void delete_buffer(uint32_t& index) { delete_buffers(1, &index); }
    static bool is_texture(uint32_t index);
    static void generate_textures(uint32_t count, uint32_t* indices);
    template <size_t N>
    static void generate_textures(uint32_t (&indices)[N])
    {
        generate_textures(N, indices);
    }
    static void generate_texture(uint32_t& index) { generate_textures(1, &index); }
    static void bind_texture(TextureType type, uint32_t index);
    static void unbind_texture(TextureType type) { bind_texture(type, 0); }
#if !defined(USE_WEBGL)
    static void bind_image_texture(
        uint32_t image_unit_index, uint32_t texture_index, int texture_level, bool is_layered, int layer,
        ImageAccess image_access, ImageInternalFormat image_format
    );
#endif
    static void activate_texture(uint32_t index);
    /// Sets a parameter to the currently bound texture.
    /// \param type Type of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(TextureType type, TextureParam param, int value);
    /// Sets a parameter to the currently bound texture.
    /// \param type Type of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(TextureType type, TextureParam param, float value);
    /// Sets a parameter to the currently bound texture.
    /// \param type Type of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param values Values to be set.
    static void set_texture_parameter(TextureType type, TextureParam param, int const* values);
    /// Sets a parameter to the currently bound texture.
    /// \param type Type of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param values Values to be set.
    static void set_texture_parameter(TextureType type, TextureParam param, float const* values);
    /// Sets a parameter to the currently bound texture.
    /// \param type Type of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(TextureType type, TextureParam param, TextureParamValue value)
    {
        set_texture_parameter(type, param, static_cast<int>(value));
    }
#if !defined(USE_OPENGL_ES)
    /// Sets a parameter to the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(uint32_t texture_index, TextureParam param, int value);
    /// Sets a parameter to the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(uint32_t texture_index, TextureParam param, float value);
    /// Sets a parameter to the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param values Values to be set.
    static void set_texture_parameter(uint32_t texture_index, TextureParam param, int const* values);
    /// Sets a parameter to the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param values Values to be set.
    static void set_texture_parameter(uint32_t texture_index, TextureParam param, float const* values);
    /// Sets a parameter to the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to set the parameter to.
    /// \param param Parameter to set.
    /// \param value Value to be set.
    static void set_texture_parameter(uint32_t texture_index, TextureParam param, TextureParamValue value)
    {
        set_texture_parameter(texture_index, param, static_cast<int>(value));
    }
    /// Sends the image's data corresponding to the currently bound 1D texture.
    /// \note Unavailable with OpenGL ES; use a Nx1 2D texture instead.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param internal_format Image internal format.
    /// \param width Image width.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_data_1d(
        TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, uint32_t width,
        TextureFormat format, PixelDataType data_type, void const* data
    );
    /// Sends the image's sub-data corresponding to the currently bound 1D texture.
    /// \note Unavailable with OpenGL ES; use a Nx1 2D texture instead.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param offsetX Width offset.
    /// \param width Image width.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_sub_data_1d(
        TextureType type, uint32_t mipmap_level, uint32_t offsetX, uint32_t width, TextureFormat format,
        PixelDataType data_type, void const* data
    );
#endif
    /// Sends the image's data corresponding to the currently bound 2D texture.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param internal_format Image internal format.
    /// \param width Image width.
    /// \param height Image height.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_data_2d(
        TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, Vector2ui const& size,
        TextureFormat format, PixelDataType data_type, void const* data
    );
    /// Sends the image's sub-data corresponding to the currently bound 2D texture.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param offsetX Width offset.
    /// \param offsetY Height offset.
    /// \param width Image width.
    /// \param height Image height.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_sub_data_2d(
        TextureType type, uint32_t mipmap_level, Vector2ui const& offset, Vector2ui const& size, TextureFormat format,
        PixelDataType data_type, void const* data
    );
    /// Sends the image's data corresponding to the currently bound 3D texture.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param internal_format Image internal format.
    /// \param width Image width.
    /// \param height Image height.
    /// \param depth Image depth.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_data_3d(
        TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, Vector3ui const& size,
        TextureFormat format, PixelDataType data_type, void const* data
    );
    /// Sends the image's sub-data corresponding to the currently bound 3D texture.
    /// \param type Type of the texture.
    /// \param mipmap_level Mipmap (level of detail) of the texture. 0 is the most detailed.
    /// \param offsetX Width offset.
    /// \param offsetY Height offset.
    /// \param offsetZ Depth offset.
    /// \param width Image width.
    /// \param height Image height.
    /// \param depth Image depth.
    /// \param format Image format.
    /// \param data_type Type of the data to be sent.
    /// \param data Data to be sent.
    static void send_image_sub_data_3d(
        TextureType type, uint32_t mipmap_level, Vector3ui const& offset, Vector3ui const& size, TextureFormat format,
        PixelDataType data_type, void const* data
    );
#if !defined(USE_OPENGL_ES)
    static void
    recover_texture_attribute(TextureType type, uint32_t mipmap_level, TextureAttribute attribute, int* values);
    static void
    recover_texture_attribute(TextureType type, uint32_t mipmap_level, TextureAttribute attribute, float* values);
    static int recover_texture_width(TextureType type, uint32_t mipmap_level = 0);
    static int recover_texture_height(TextureType type, uint32_t mipmap_level = 0);
    static int recover_texture_depth(TextureType type, uint32_t mipmap_level = 0);
    static TextureInternalFormat recover_texture_internal_format(TextureType type, uint32_t mipmap_level = 0);
    static void recover_texture_data(
        TextureType type, uint32_t mipmap_level, TextureFormat format, PixelDataType data_type, void* data
    );
#endif
    /// Generate mipmaps (levels of detail) for the currently bound texture.
    /// \param type Type of the texture to generate mipmaps for.
    static void generate_mipmap(TextureType type);
#if !defined(USE_OPENGL_ES)
    /// Generate mipmaps (levels of detail) for the given texture.
    /// \note Requires OpenGL 4.5+.
    /// \param texture_index Index of the texture to generate mipmaps for.
    static void generate_mipmap(uint32_t texture_index);
#endif
    static void delete_textures(uint32_t count, uint32_t* indices);
    template <size_t N>
    static void delete_textures(uint32_t (&indices)[N])
    {
        delete_textures(N, indices);
    }
    static void delete_texture(uint32_t& index) { delete_textures(1, &index); }
    static void resize_viewport(Vector2ui const& position, Vector2ui const& size);
    static uint32_t create_program();
    static void get_program_parameter(uint32_t index, ProgramParameter parameter, int* parameters);
    static bool is_program_linked(uint32_t index);
    static uint32_t recover_active_uniform_count(uint32_t program_index);
    static std::vector<uint32_t> recover_attached_shaders(uint32_t program_index);
    static void link_program(uint32_t index);
    static void use_program(uint32_t index);
    static void delete_program(uint32_t index);
    static uint32_t create_shader(ShaderType type);
    static int recover_shader_info(uint32_t index, ShaderInfo info);
    static ShaderType recover_shader_type(uint32_t index)
    {
        return static_cast<ShaderType>(recover_shader_info(index, ShaderInfo::TYPE));
    }
    static bool is_shader_compiled(uint32_t index)
    {
        return (recover_shader_info(index, ShaderInfo::COMPILE_STATUS) == 1);
    }
    static void send_shader_source(uint32_t index, char const* source, int length);
    static void send_shader_source(uint32_t index, std::string const& source)
    {
        send_shader_source(index, source.c_str(), static_cast<int>(source.size()));
    }
    static void send_shader_source(uint32_t index, std::string_view source)
    {
        send_shader_source(index, source.data(), static_cast<int>(source.size()));
    }
    static std::string recover_shader_source(uint32_t index);
    static void compile_shader(uint32_t index);
    static void attach_shader(uint32_t program_index, uint32_t shader_index);
    static void detach_shader(uint32_t program_index, uint32_t shader_index);
    static bool is_shader_attached(uint32_t program_index, uint32_t shader_index);
    static void delete_shader(uint32_t index);
    /// Gets the uniform's location (ID) corresponding to the given name.
    /// \note Location will be -1 if the name is incorrect or if the uniform isn't used in the shader(s) (will be
    /// optimized out).
    /// \param program_index Index of the shader program to which is bound the uniform.
    /// \param uniform_name Name of the uniform to recover the location from.
    /// \return Location (ID) of the uniform.
    static int recover_uniform_location(uint32_t program_index, char const* uniform_name);
    /// Gets the uniform's information (type, name & size).
    /// \param program_index Index of the program to recover the uniform from.
    /// \param uniform_index Index of the uniform to recover. This is NOT the uniform's location, rather the actual
    /// index from 0 to the uniform count.
    /// \param type Type of the uniform to recover.
    /// \param name Name of the uniform to recover.
    /// \param size Size of the uniform to recover. Will be 1 for non-array uniforms, greater for arrays.
    /// \see recover_active_uniform_count().
    static void recover_uniform_info(
        uint32_t program_index, uint32_t uniform_index, UniformType& type, std::string& name, int* size = nullptr
    );
    static UniformType recover_uniform_type(uint32_t program_index, uint32_t uniform_index);
    static std::string recover_uniform_name(uint32_t program_index, uint32_t uniform_index);
    static void recover_uniform_data(uint32_t program_index, int uniform_index, int* data);
    static void recover_uniform_data(uint32_t program_index, int uniform_index, uint32_t* data);
    static void recover_uniform_data(uint32_t program_index, int uniform_index, float* data);
#if !defined(USE_OPENGL_ES)
    static void recover_uniform_data(uint32_t program_index, int uniform_index, double* data);
#endif
    static void bind_uniform_block(uint32_t program_index, uint32_t uniform_block_index, uint32_t binding_index);
    static uint32_t recover_uniform_block_index(uint32_t program_index, char const* uniform_name);
    /// Sends an integer as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param value Integer to be sent.
    static void send_uniform(int uniform_index, int value);
    /// Sends an uinteger as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param value Unsigned integer to be sent.
    static void send_uniform(int uniform_index, uint32_t value);
    /// Sends a floating-point value as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param value Floating-point value to be sent.
    static void send_uniform(int uniform_index, float value);
    /// Sends an integer 1D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_vector1fi(int uniform_index, int const* values, int count = 1);
    /// Sends an integer 2D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector2i(int uniform_index, int const* values, int count = 1);
    /// Sends an integer 3D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector3i(int uniform_index, int const* values, int count = 1);
    /// Sends an integer 4D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector4i(int uniform_index, int const* values, int count = 1);
    /// Sends an uinteger 1D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_vector1fui(int uniform_index, uint32_t const* values, int count = 1);
    /// Sends an uinteger 2D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector2ui(int uniform_index, uint32_t const* values, int count = 1);
    /// Sends an uinteger 3D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector3ui(int uniform_index, uint32_t const* values, int count = 1);
    /// Sends an uinteger 4D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector4ui(int uniform_index, uint32_t const* values, int count = 1);
    /// Sends a floating-point 1D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_vector1f(int uniform_index, float const* values, int count = 1);
    /// Sends a floating-point 2D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector2f(int uniform_index, float const* values, int count = 1);
    /// Sends a floating-point 3D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector3f(int uniform_index, float const* values, int count = 1);
    /// Sends a floating-point 4D vector as uniform.
    /// \param uniform_index Index of the uniform to send the data to.
    /// \param values Array of values to be sent.
    /// \param count Number of vectors to be sent.
    static void send_uniform_Vector4f(int uniform_index, float const* values, int count = 1);
    /// Sends a floating-point 2x2 matrix as uniform.
    /// \param uniform_index Index of the uniform to send the matrix's data to.
    /// \param values Array of values to be sent.
    /// \param count Number of matrices to be sent.
    /// \param transpose Defines whether the matrix should be transposed when sent; false if sending it as column-major,
    /// true if row-major.
    static void send_uniform_Matrix2(int uniform_index, float const* values, int count = 1, bool transpose = false);
    /// Sends a floating-point 3x3 matrix as uniform.
    /// \param uniform_index Index of the uniform to send the matrix's data to.
    /// \param values Array of values to be sent.
    /// \param count Number of matrices to be sent.
    /// \param transpose Defines whether the matrix should be transposed when sent; false if sending it as column-major,
    /// true if row-major.
    static void send_uniform_Matrix3(int uniform_index, float const* values, int count = 1, bool transpose = false);
    /// Sends a floating-point 4x4 matrix as uniform.
    /// \param uniform_index Index of the uniform to send the matrix's data to.
    /// \param values Array of values to be sent.
    /// \param count Number of matrices to be sent.
    /// \param transpose Defines whether the matrix should be transposed when sent; false if sending it as column-major,
    /// true if row-major.
    static void send_uniform_Matrix4(int uniform_index, float const* values, int count = 1, bool transpose = false);

    static void send_uniform_color(int uniform_index, float const* values, int count = 1);

    static void generate_framebuffers(int count, uint32_t* indices);
    template <size_t N>
    static void generate_framebuffers(uint32_t (&indices)[N])
    {
        generate_framebuffers(N, indices);
    }
    static void generate_framebuffer(uint32_t& index) { generate_framebuffers(1, &index); }
    static void bind_framebuffer(uint32_t index, FramebufferType type = FramebufferType::FRAMEBUFFER);
    static void unbind_framebuffer(FramebufferType type = FramebufferType::FRAMEBUFFER) { bind_framebuffer(0, type); }
    static FramebufferStatus get_framebuffer_status(FramebufferType type = FramebufferType::FRAMEBUFFER);
    static bool is_framebuffer_complete(FramebufferType type = FramebufferType::FRAMEBUFFER)
    {
        return get_framebuffer_status(type) == FramebufferStatus::COMPLETE;
    }
#if !defined(USE_OPENGL_ES)
    static void set_framebuffer_texture(
        FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level,
        FramebufferType type = FramebufferType::FRAMEBUFFER
    );
    static void set_framebuffer_texture_1d(
        FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level,
        FramebufferType type = FramebufferType::FRAMEBUFFER
    );
#endif
    static void set_framebuffer_texture_2d(
        FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level,
        TextureType textureType = TextureType::TEXTURE_2D, FramebufferType type = FramebufferType::FRAMEBUFFER
    );
#if !defined(USE_OPENGL_ES)
    static void set_framebuffer_texture_3d(
        FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level, uint32_t layer,
        FramebufferType type = FramebufferType::FRAMEBUFFER
    );
#endif
    static void recover_framebuffer_attachment_parameter(
        FramebufferAttachment attachment, FramebufferAttachmentParam param, int* values,
        FramebufferType type = FramebufferType::FRAMEBUFFER
    );
    static void set_read_buffer(ReadBuffer buffer);
    static void set_draw_buffers(uint32_t count, DrawBuffer const* buffers);
    template <size_t N>
    static void set_draw_buffers(DrawBuffer (&buffers)[N])
    {
        set_draw_buffers(N, buffers);
    }
    static void blit_framebuffer(
        int read_min_x, int read_min_y, int read_max_x, int read_max_y, int write_min_x, int write_min_y,
        int write_max_x, int write_max_y, MaskType mask, BlitFilter filter
    );
    static void delete_framebuffers(uint32_t count, uint32_t* indices);
    template <size_t N>
    static void delete_framebuffers(uint32_t (&indices)[N])
    {
        delete_framebuffers(N, indices);
    }
    static void deleteFramebuffer(uint32_t& index) { delete_framebuffers(1, &index); }
    static void draw_arrays(PrimitiveType type, uint32_t first, uint32_t count);
    static void draw_arrays(PrimitiveType type, uint32_t count) { draw_arrays(type, 0, count); }
    static void
    draw_arrays_instanced(PrimitiveType type, uint32_t first, uint32_t primitive_count, uint32_t instance_count);
    static void draw_arrays_instanced(PrimitiveType type, uint32_t primitive_count, uint32_t instance_count)
    {
        draw_arrays_instanced(type, 0, primitive_count, instance_count);
    }
    static void draw_elements(PrimitiveType type, uint32_t count, ElementDataType data_type, void const* indices);
    static void draw_elements(PrimitiveType type, uint32_t count, uint8_t const* indices)
    {
        draw_elements(type, count, ElementDataType::UBYTE, indices);
    }
    static void draw_elements(PrimitiveType type, uint32_t count, unsigned short const* indices)
    {
        draw_elements(type, count, ElementDataType::USHORT, indices);
    }
    static void draw_elements(PrimitiveType type, uint32_t count, uint32_t const* indices)
    {
        draw_elements(type, count, ElementDataType::UINT, indices);
    }
    static void draw_elements(PrimitiveType type, uint32_t count)
    {
        draw_elements(type, count, ElementDataType::UINT, nullptr);
    }
    static void draw_elements_instanced(
        PrimitiveType type, uint32_t primitive_count, ElementDataType data_type, void const* indices,
        uint32_t instance_count
    );
    static void draw_elements_instanced(
        PrimitiveType type, uint32_t primitive_count, uint8_t const* indices, uint32_t instance_count
    )
    {
        draw_elements_instanced(type, primitive_count, ElementDataType::UBYTE, indices, instance_count);
    }
    static void draw_elements_instanced(
        PrimitiveType type, uint32_t primitive_count, unsigned short const* indices, uint32_t instance_count
    )
    {
        draw_elements_instanced(type, primitive_count, ElementDataType::USHORT, indices, instance_count);
    }
    static void draw_elements_instanced(
        PrimitiveType type, uint32_t primitive_count, uint32_t const* indices, uint32_t instance_count
    )
    {
        draw_elements_instanced(type, primitive_count, ElementDataType::UINT, indices, instance_count);
    }
    static void draw_elements_instanced(PrimitiveType type, uint32_t primitive_count, uint32_t instance_count)
    {
        draw_elements_instanced(type, primitive_count, ElementDataType::UINT, nullptr, instance_count);
    }

    static void dispatch_compute(Vector3ui group_content = Vector3ui(1));

    /// Sets a memory synchronization barrier.
    /// \note Requires OpenGL 4.2+ or ES 3.1+.
    /// \param type Type of the barrier to be set.
    static void set_memory_barrier(BarrierType type);

    /// Sets a localized memory synchronization barrier.
    /// \note Requires OpenGL 4.5+ or ES 3.1+.
    /// \param type Type of the barrier to be set.
    static void set_memory_barrier_by_region(RegionBarrierType type);

    static void generate_queries(uint32_t count, uint32_t* indices);

    static void generate_query(uint32_t& index) { generate_queries(1, &index); }

    static void begin_query(QueryType type, uint32_t index);

    static void end_query(QueryType type);

#if !defined(USE_OPENGL_ES)
    static void recover_query_result(uint32_t index, int64_t& result);

    static void recover_query_result(uint32_t index, uint64_t& result);
#endif
    static void delete_queries(uint32_t count, uint32_t* indices);

    static void delete_query(uint32_t& index) { delete_queries(1, &index); }

#if !defined(USE_OPENGL_ES)
    /// Assigns a label to a graphic object.
    /// \note Requires OpenGL 4.3+.
    /// \param type Type of the object to assign the label to.
    /// \param object_index Index of the object to assign the label to.
    /// \param label Label to assign; must be null-terminated.
    static void set_label(RenderObjectType type, uint32_t object_index, char const* label);

    /// Assigns a label to a graphic object.
    /// \note Requires OpenGL 4.3+.
    /// \param type Type of the object to assign the label to.
    /// \param object_index Index of the object to assign the label to.
    /// \param label Label to assign.
    static void set_label(RenderObjectType type, uint32_t object_index, std::string const& label)
    {
        set_label(type, object_index, label.c_str());
    }

    /// Removes label from a graphic object.
    /// \note Requires OpenGL 4.3+.
    /// \param type Type of the object to remove the label from.
    /// \param object_index Index of the object to remove the label from.
    static void reset_label(RenderObjectType type, uint32_t object_index) { set_label(type, object_index, nullptr); }

    /// Retrieves a previously assigned label on a given object.
    /// \note Requires OpenGL 4.3+.
    /// \param type Type of the object to retrieve the label from.
    /// \param object_index Index of the object to retrieve the label from.
    /// \return Label of the object.
    static std::string recover_label(RenderObjectType type, uint32_t object_index);

    /// Adds an entry into the debug group stack. This can be used to define regions which are shown in graphics
    /// debuggers.
    /// \note Requires OpenGL 4.3+.
    /// \param name Name of the group.
    static void push_debug_group(std::string const& name);

    /// Removes the latest pushed group.
    /// \note Requires OpenGL 4.3+.
    static void pop_debug_group();
#endif
    static ErrorCodes recover_errors();

    static bool has_errors() { return !recover_errors().empty(); }

    static void print_errors();

private:
    static void recover_default_framebuffer_color_format();

    static void recover_default_framebuffer_depth_format();

    /// Prints OpenGL errors only in Debug mode and if XEN_SKIP_RENDERER_ERRORS hasn't been defined.
    static void print_conditional_errors()
    {
#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
        print_errors();
#endif
    }

    static inline bool initialized = false;

    static inline int major_version{};
    static inline int minor_version{};
    static inline std::unordered_set<std::string> extensions{};
    static inline TextureInternalFormat default_framebuffer_color{};
    static inline TextureInternalFormat default_framebuffer_depth{};
};
}