

#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

namespace {
void register_matrix2(sol::state& state, std::string_view name)
{
    using MatrixDataType = float;
    using MatrixType = Matrix2;

    sol::usertype<MatrixType> matrix = state.new_usertype<MatrixType>(
        name, sol::constructors<
                  MatrixType(), MatrixType(MatrixDataType),
                  MatrixType(MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType)>()
    );

    matrix["add"] = &MatrixType::add;
    matrix["subtract"] = &MatrixType::subtract;
    matrix["multiply"] = &MatrixType::multiply;
    matrix["divide"] = &MatrixType::divide;
    matrix["transform"] = &MatrixType::transform;
    matrix["scale"] = &MatrixType::scale;

    matrix["negate"] = &MatrixType::negate;
    matrix["inverse"] = &MatrixType::inverse;
    matrix["transpose"] = &MatrixType::transpose;
    matrix["determinant"] = &MatrixType::determinant;
    matrix["submatrix"] = &MatrixType::submatrix;

    matrix.set_function(sol::meta_function::unary_minus, &MatrixType::subtract);
    matrix.set_function(sol::meta_function::addition, &MatrixType::add);
    matrix.set_function(sol::meta_function::subtraction, &MatrixType::subtract);
    matrix.set_function(sol::meta_function::multiplication, &MatrixType::multiply);
    matrix.set_function(sol::meta_function::division, &MatrixType::divide);
    matrix.set_function(
        sol::meta_function::index, sol::overload(
                                       PickNonConstOverload<uint32_t>(&MatrixType::operator[]),
                                       PickConstOverload<uint32_t>(&MatrixType::operator[])
                                   )
    );
}

void register_matrix3(sol::state& state, std::string_view name)
{
    using MatrixDataType = float;
    using MatrixType = Matrix3;

    sol::usertype<MatrixType> matrix = state.new_usertype<MatrixType>(
        name, sol::constructors<
                  MatrixType(), MatrixType(MatrixDataType),
                  MatrixType(
                      MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType,
                      MatrixDataType, MatrixDataType, MatrixDataType
                  )>()
    );

    matrix["add"] = &MatrixType::add;
    matrix["subtract"] = &MatrixType::subtract;
    matrix["multiply"] = sol::overload(
        PickOverload<MatrixType const&>(&MatrixType::multiply),
        PickOverload<MatrixType::row_t const&>(&MatrixType::multiply)
    );
    matrix["divide"] = &MatrixType::divide;
    matrix["transform"] = &MatrixType::transform;
    matrix["scale"] = &MatrixType::scale;

    matrix["negate"] = &MatrixType::negate;
    matrix["inverse"] = &MatrixType::inverse;
    matrix["transpose"] = &MatrixType::transpose;
    matrix["determinant"] = &MatrixType::determinant;
    matrix["submatrix"] = &MatrixType::submatrix;

    matrix.set_function(sol::meta_function::unary_minus, &MatrixType::subtract);
    matrix.set_function(sol::meta_function::addition, &MatrixType::add);
    matrix.set_function(sol::meta_function::subtraction, &MatrixType::subtract);
    matrix.set_function(
        sol::meta_function::multiplication, sol::overload(
                                                PickOverload<MatrixType const&>(&MatrixType::multiply),
                                                PickOverload<MatrixType::row_t const&>(&MatrixType::multiply)
                                            )
    );
    matrix.set_function(sol::meta_function::division, &MatrixType::divide);
    matrix.set_function(
        sol::meta_function::index, sol::overload(
                                       PickNonConstOverload<uint32_t>(&MatrixType::operator[]),
                                       PickConstOverload<uint32_t>(&MatrixType::operator[])
                                   )
    );
}

void register_matrix4(sol::state& state, std::string_view name)
{
    using MatrixDataType = float;
    using MatrixType = Matrix4;

    sol::usertype<MatrixType> matrix = state.new_usertype<MatrixType>(
        name, sol::constructors<
                  MatrixType(), MatrixType(MatrixDataType),
                  MatrixType(
                      MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType,
                      MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType,
                      MatrixDataType, MatrixDataType, MatrixDataType, MatrixDataType
                  )>()
    );

    matrix["add"] = &MatrixType::add;
    matrix["subtract"] = &MatrixType::subtract;
    matrix["multiply"] = sol::overload(
        PickOverload<MatrixType const&>(&MatrixType::multiply),
        PickOverload<MatrixType::row_t const&>(&MatrixType::multiply)
    );
    matrix["divide"] = &MatrixType::divide;
    matrix["transform"] = &MatrixType::transform;
    matrix["translate"] = sol::overload(
        PickOverload<Vector2f const&>(&MatrixType::translate), PickOverload<Vector3f const&>(&MatrixType::translate)
    );
    matrix["scale"] = sol::overload(
        PickOverload<Vector2f const&>(&MatrixType::scale), PickOverload<Vector3f const&>(&MatrixType::scale),
        PickOverload<MatrixType::row_t const&>(&MatrixType::scale)
    );
    matrix["rotate"] = &MatrixType::rotate;

    matrix["negate"] = &MatrixType::negate;
    matrix["inverse"] = &MatrixType::inverse;
    matrix["transpose"] = &MatrixType::transpose;
    matrix["determinant"] = &MatrixType::determinant;
    matrix["submatrix"] = &MatrixType::submatrix;
    matrix["translation"] = &MatrixType::translation;
    matrix["scaling"] = &MatrixType::scaling;

    matrix["tranform_matrix"] = &MatrixType::tranform_matrix;
    matrix["perspective_matrix"] = sol::overload(
        PickOverload<float, float, float>(&MatrixType::perspective_matrix),
        PickOverload<float, float, float, float>(&MatrixType::perspective_matrix)
    );
    matrix["orthographic_matrix"] = &MatrixType::orthographic_matrix;
    matrix["frustum_matrix"] = &MatrixType::frustum_matrix;
    matrix["view_matrix"] = &MatrixType::view_matrix;
    matrix["project"] = &MatrixType::project;
    matrix["unproject"] = &MatrixType::unproject;
    matrix["look_at"] = &MatrixType::look_at;

    matrix.set_function(sol::meta_function::unary_minus, &MatrixType::subtract);
    matrix.set_function(sol::meta_function::addition, &MatrixType::add);
    matrix.set_function(sol::meta_function::subtraction, &MatrixType::subtract);
    matrix.set_function(
        sol::meta_function::multiplication, sol::overload(
                                                PickOverload<MatrixType const&>(&MatrixType::multiply),
                                                PickOverload<MatrixType::row_t const&>(&MatrixType::multiply)
                                            )
    );
    matrix.set_function(sol::meta_function::division, &MatrixType::divide);
    matrix.set_function(
        sol::meta_function::index, sol::overload(
                                       PickNonConstOverload<uint32_t>(&MatrixType::operator[]),
                                       PickConstOverload<uint32_t>(&MatrixType::operator[])
                                   )
    );
}
}

void LuaWrapper::register_matrix_types()
{
    sol::state& state = get_state();

    register_matrix2(state, "Matrix2");
    register_matrix3(state, "Matrix3");
    register_matrix4(state, "Matrix4");

    // Attempting to hash matrices can give an error: 'integer value will be misrepresented in lua'
    // This could be solved by defining SOL_ALL_INTEGER_VALUES_FIT to 1 (see
    // https://github.com/ThePhD/sol2/issues/1178#issuecomment-833923813),
    //   but this gives signed integers thus can (and does) produce negative values
    // Unsigned 64-bit integers seem to be supported in Lua 5.3 and above, so there may (will) be a better solution
    // Manually hashing a matrix should not be useful on Lua's side anyway
}
}
