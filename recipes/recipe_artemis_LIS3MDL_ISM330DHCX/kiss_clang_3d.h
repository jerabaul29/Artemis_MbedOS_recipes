#ifndef KISS_CLANG_3D_UTILS_H
#define KISS_CLANG_3D_UTILS_H

#include <cmath>

// TODO
// 1
// rather than having an annoying (axis, angle) format for rotations,
// define a custom struct for rotation definition
// 2
// add a function to determine if 2 rotations are identical but written
// in different ways
// 3 
// add a function to reduce a rotation to a canonical form
// 4
// check for more optimal ways to compute quaternion product, rotation by
// quaternion, and both of these using specifically unit quaternions; for this,
// write funcs to work with specifically unit quaternions
// 5
// add function variations without the boolean checks

// TODO
// think if should remove some of the checks if things are unitary or not etc

// ------------------------------------------------------------
// MACROS
// ------------------------------------------------------------

#define XSTR(x) STR(x)
#define STR(x) #x

// what fundamental type do we want to use?
// if the F_TYPE_SWITCH is set (by defining the macro earlier, either before #includ-ing, or
// by defining the compilation flag -DF_TYPE_SWITCH="'X'" where X is the type flag wanted),
// use it, otherwise, use the value set under.
#ifndef F_TYPE_SWITCH
  // double is 'D', float is 'F'
  #define F_TYPE_SWITCH 'F'
#endif

// this ic C, no overloading, we need to adapt a bit to what sort of fundamental type we want to use
#if (F_TYPE_SWITCH == 'F')
    #define F_TYPE float
    #define F_CAST (float)

    #define F_SFX f

    #define F_TYPE_2 (2.0f)
    #define F_TYPE_1 (1.0f)
    #define F_TYPE_0 (0.0f)
    #define F_TYPE_05 (0.5f)
    #define F_TYPE_PI (3.14159265358979323846f)
    // TODO: is there a way to write a float in scientific notation?
    #define DEFAULT_TOL (0.0001f)

    #define F_TYPE_ABS(x) fabsf(x)
    #define F_TYPE_SQRT(x) sqrtf(x)
    #define F_TYPE_COS(x) cosf(x)
    #define F_TYPE_SIN(x) sinf(x)
    #define F_TYPE_ACOS(x) acosf(x)

#elif (F_TYPE_SWITCH == 'D')
    #define F_TYPE double
    #define F_CAST (double)

    #define F_SFX

    #define F_TYPE_2 (2.0)
    #define F_TYPE_1 (1.0)
    #define F_TYPE_0 (0.0)
    #define F_TYPE_05 (0.5)
    #define F_TYPE_PI (3.14159265358979323846)
    #define DEFAULT_TOL (1.0e-6)

    #define F_TYPE_ABS(x) fabs(x)
    #define F_TYPE_SQRT(x) sqrt(x)
    #define F_TYPE_COS(x) cos(x)
    #define F_TYPE_SIN(x) sin(x)
    #define F_TYPE_ACOS(x) acos(x)
#else
    #pragma message "The value of F_TYPE_SWITCH: " XSTR(F_TYPE_SWITCH)
    #error "invalid F_TYPE_SWITCH admissible switches are F (float) and D (double)"
#endif


// ------------------------------------------------------------
// STRUCTS
// ------------------------------------------------------------

// TODO
// change struct names to Maj first

// --------------------------------------------------
// 3D vector, components i, j, k
struct vec3 {
    F_TYPE i;
    F_TYPE j;
    F_TYPE k;
};

// --------------------------------------------------
// quaternion, with real part (r), and components (i, i, k)
struct quat {
    F_TYPE r;
    F_TYPE i;
    F_TYPE j;
    F_TYPE k;
};

// --------------------------------------------------
// rotation, provided as a vector (v_i,j,k) and an angle in rads (a)
struct varot {
    vec3 v;
    F_TYPE a;
};


// ------------------------------------------------------------
// FUNCTIONS DECLARATIONS
// ------------------------------------------------------------

// ---------------------------------------------
// VEC3 functions
// ---------------------------------------------

/*
Setter, in the right order
*/
void vec3_setter(vec3 * v, F_TYPE vi, F_TYPE vj, F_TYPE vk);

/*
Copy, 'deep'.
*/
void vec3_copy(vec3 const * v_in, vec3 * v_out);

/*
Check if a vector is the null vector, up to a tolerance
*/
bool vec3_is_null(vec3 const * v1, F_TYPE tolerance=DEFAULT_TOL);

/*
Check if 2 vectors are equal, at a tolerance precision
*/
bool vec3_equal(vec3 const * v1, vec3 const * v2, F_TYPE tolerance=DEFAULT_TOL);

/*
Compute the square norm of a vector
*/
F_TYPE vec3_norm_square(vec3 const * v);

/*
Compute the norm of a vector
*/
F_TYPE vec3_norm(vec3 const * v);

/*
Scale a vector in place
*/
void vec3_scale(vec3 * v, F_TYPE scale);

/*
Add a vector v_add to an already existing vector v_acc
*/
void vec3_add(vec3 * v_acc, vec3 const * v_add);

/*
Subtract a vector v_subs to an already existing vector v_acc
*/
void vec3_sub(vec3 * v_acc, vec3 const * v_sub);

/*
Take the scalar product of 2 vectors
*/
F_TYPE vec3_scalar(vec3 const * v1, vec3 const * v2);

/*
Take the cross product of 2 vectors v1 and v2 and put the result in v_res
*/
void vec3_cross(vec3 const * v1, vec3 const * v2, vec3 * v_res);

/*
Normalize a vector in place; of course this does not work for the null vector, so also
return a bool if was able to normalize or not
*/
bool vec3_normalize(vec3 * v);

/*
Return wether 2 vectors are colinear
*/
bool vec3_colinear(vec3 const * v, vec3 const * w, F_TYPE tolerance=DEFAULT_TOL);

// ---------------------------------------------
// QUAT functions
// ---------------------------------------------

/*
Setter for quaternion
*/
void quat_setter(quat * q, F_TYPE qr, F_TYPE qi, F_TYPE qj, F_TYPE qk);

/*
Copy, deep
*/
void quat_copy(quat const * q_in, quat * q_out);

/*
Norm of a quaternion
*/
F_TYPE quat_norm(quat const * q);

/*
Square norm of a quaternion
*/

F_TYPE quat_norm_square(quat const * q);

/*
Whether or not 2 quaternions are equal up to tolerance
*/
bool quat_equal(quat const * q_1, quat const * q_2, F_TYPE tolerance=DEFAULT_TOL);

/*
Conjugate of a quaternion, in-place
*/
void quat_conj(quat * q);

/*
Whether a quaternion is unitary, i.e. has norm 1
*/
bool quat_is_unitary(quat const * q, F_TYPE tolerance=DEFAULT_TOL);

/*
Multiply 2 quaternions, and write the result in a third one
*/
void quat_prod(quat const * q_left, quat const * q_right, quat * q_result);

/*
Add one quaternion to another, in place, inside an accumulator
*/
void quat_add(quat * q_acc, quat const * q_add);

/*
Subtract one quaternion to another, in place, inside an accumulator
*/
void quat_sub(quat * q_acc, quat const * q_sub);

/*
Inverse of a quaternion, in place. This works only for non zero quat,
so return a boolean flag (true if success).
*/
bool quat_inv(quat * q, F_TYPE tolerance=DEFAULT_TOL);

// ---------------------------------------------
// QUAT and VECT functions
// --------------------------------------------

/*
Get a vector from a quaternion. This makes sense only if the quaternion is a "pure vector",
return a bool indicating if this is the case.
*/
bool quat_to_vec3(quat const * q, vec3 * v_out, F_TYPE tolerance=DEFAULT_TOL);

/*
Write the vector part into a pure vector quaternion
*/
void vec3_to_quat(vec3 const * v, quat * q_out);

/*
Write a "rotation quaternion" given the rotation axis and angle in rad.
This works only for non null axis vector, except if the transformation is
the identity.
*/
bool rotation_to_quat(quat * q, vec3 const * rotation_axis, F_TYPE const rotation_angle_rad, F_TYPE tolerance=DEFAULT_TOL);

/*
Extract the rotation axis and angle from a unit quaternion; this works
only for unit quaternions, so return bool if is unit. We are polite and we
return a rotation axis that has unit norm.
*/
bool quat_to_rotation(vec3 * rotation_axis, F_TYPE * rotation_angle_rad, quat const * q_rotation, F_TYPE tolerance=DEFAULT_TOL);

/*
Rotate a vector by a given quaternion, using the direct method:
[0, R(v)] = q x [0, v] x q*
This is the simplest method, but quite slow.
Only unit quaternions are pure rotations; provide a bool flag indicating if this is valid.
*/
bool rotate_by_quat(vec3 * v, quat const * q, F_TYPE tolerance=DEFAULT_TOL) __attribute__((deprecated("prefer using rotate_by_quat_R with is faster")));

/*
Rotate a vector by a unit quaternion, using the "Rodriguez" formula:
https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
q = [s, u], s the scalar part, u the vector part
R(v) = 2.0 ( (u . v) u + (s * s - 0.5) v + s (u x v) )
This is quite a bit faster. This assumes that a unit quaternion is provided
(but not checked, for speed; providing a unit quaternion is the caller's
responsibility).
*/
void rotate_by_quat_R(vec3 const * v, quat const * q, vec3 * Rv);

// ------------------------------------------------------------
// DEFINITIONS
// ------------------------------------------------------------

void vec3_setter(vec3 * v, F_TYPE vi, F_TYPE vj, F_TYPE vk){
    v->i = vi;
    v->j = vj;
    v->k = vk;
}

void vec3_copy(vec3 const * v_in, vec3 * v_out){
    v_out->i = v_in->i;
    v_out->j = v_in->j;
    v_out->k = v_in->k;
}

bool vec3_is_null(vec3 const * v1, F_TYPE tolerance){
    return(
        F_TYPE_ABS(v1->i) <= tolerance &&
        F_TYPE_ABS(v1->j) <= tolerance &&
        F_TYPE_ABS(v1->k) <= tolerance
    );
}

bool vec3_equal(vec3 const * v1, vec3 const * v2, F_TYPE tolerance){
    return(
        F_TYPE_ABS(v1->i - v2->i) <= tolerance &&
        F_TYPE_ABS(v1->j - v2->j) <= tolerance &&
        F_TYPE_ABS(v1->k - v2->k) <= tolerance
    );
}

F_TYPE vec3_norm_square(vec3 const * v){
    return (
            (v->i * v->i) + (v->j * v->j) + (v->k * v->k)
    );
}

F_TYPE vec3_norm(vec3 const * v){
    return (
        F_TYPE_SQRT(
                (v->i * v->i) + (v->j * v->j) + (v->k * v->k)
        )
    );
}

void vec3_scale(vec3 * v, F_TYPE scale){
    v->i *= scale;
    v->j *= scale;
    v->k *= scale;
}

void vec3_add(vec3 * v_acc, vec3 const * v_add){
    v_acc->i += v_add->i;
    v_acc->j += v_add->j;
    v_acc->k += v_add->k;
}

void vec3_sub(vec3 * v_acc, vec3 const * v_sub){
    v_acc->i -= v_sub->i;
    v_acc->j -= v_sub->j;
    v_acc->k -= v_sub->k;
}

F_TYPE vec3_scalar(vec3 const * v1, vec3 const * v2){
    return(
        v1->i * v2->i +
        v1->j * v2->j +
        v1->k * v2->k
    );
}

void vec3_cross(vec3 const * v1, vec3 const * v2, vec3 * v_res){
    v_res->i =  v1->j * v2->k - v1->k * v2->j;
    v_res->j = -v1->i * v2->k + v1->k * v2->i;
    v_res->k =  v1->i * v2->j - v1->j * v2->i;
}

bool vec3_normalize(vec3 * v){
    if (vec3_is_null(v)){
        return false;
    }
    else{
        F_TYPE norm = vec3_norm(v);
        vec3_scale(v, F_TYPE_1 / norm);
        return true;
    }
}

bool vec3_colinear(vec3 const * v, vec3 const * w, F_TYPE tolerance){
    vec3 result_cross_product;
    vec3_cross(v, w, &result_cross_product);
    return vec3_is_null(&result_cross_product, tolerance);
}

void quat_setter(quat * q, F_TYPE qr, F_TYPE qi, F_TYPE qj, F_TYPE qk){
    q->r = qr;
    q->i = qi;
    q->j = qj;
    q->k = qk;
}

void quat_copy(quat const * q_in, quat * q_out){
    q_out->r = q_in->r;
    q_out->i = q_in->i;
    q_out->j = q_in->j;
    q_out->k = q_in->k;
}

F_TYPE quat_norm(quat const * q){
    return(
        F_TYPE_SQRT(
            q->r * q->r + q->i * q->i + q->j * q->j + q->k * q->k
        )
    );
}

F_TYPE quat_norm_square(quat const * q){
    return(
        q->r * q->r + q->i * q->i + q->j * q->j + q->k * q->k
    );
}

bool quat_equal(quat const * q_1, quat const * q_2, F_TYPE tolerance){
    return(
        F_TYPE_ABS(q_1->r - q_2->r) <= tolerance &&
        F_TYPE_ABS(q_1->i - q_2->i) <= tolerance &&
        F_TYPE_ABS(q_1->j - q_2->j) <= tolerance &&
        F_TYPE_ABS(q_1->k - q_2->k) <= tolerance
    );
}

void quat_conj(quat * q){
    q->i = -q->i;
    q->j = -q->j;
    q->k = -q->k;
}

bool quat_is_unitary(quat const * q, F_TYPE tolerance){
    return(
        F_TYPE_ABS(quat_norm_square(q) - F_TYPE_1) < tolerance
    );
}

// TODO: is there a more computationally efficient way to take quat product? To take quat product for unit quats?
void quat_prod(quat const * q_left, quat const * q_right, quat * q_result){
    q_result->r = q_left->r * q_right->r  -  q_left->i * q_right->i  -  q_left->j * q_right->j  -  q_left->k * q_right->k;
    q_result->i = q_left->r * q_right->i  +  q_left->i * q_right->r  +  q_left->j * q_right->k  -  q_left->k * q_right->j;
    q_result->j = q_left->r * q_right->j  -  q_left->i * q_right->k  +  q_left->j * q_right->r  +  q_left->k * q_right->i;
    q_result->k = q_left->r * q_right->k  +  q_left->i * q_right->j  -  q_left->j * q_right->i  +  q_left->k * q_right->r;
}

void quat_add(quat * q_acc, quat const * q_add){
    q_acc->r += q_add->r;
    q_acc->i += q_add->i;
    q_acc->j += q_add->j;
    q_acc->k += q_add->k;
}

void quat_sub(quat * q_acc, quat const * q_sub){
    q_acc->r -= q_sub->r;
    q_acc->i -= q_sub->i;
    q_acc->j -= q_sub->j;
    q_acc->k -= q_sub->k;
}

bool quat_inv(quat * q, F_TYPE tolerance){
    F_TYPE norm_square = quat_norm_square(q);

    if (norm_square < tolerance){
        return false;
    }
    else{
        q->r /= norm_square;
        q->i = -q->i / norm_square;
        q->j = -q->j / norm_square;
        q->k = -q->k / norm_square;

        return true;
    }
}

bool quat_to_vec3(quat const * q, vec3 * v_out, F_TYPE tolerance){
    v_out->i = q->i;
    v_out->j = q->j;
    v_out->k = q->k;

    if (F_TYPE_ABS(q->r) > tolerance){
        return false;
    }
    else{
        return true;
    }
}

void vec3_to_quat(vec3 const * v, quat * q_out){
    q_out->r = F_TYPE_0;
    q_out->i = v->i;
    q_out->j = v->j;
    q_out->k = v->k;
}

bool rotation_to_quat(quat * q, vec3 const * rotation_axis, F_TYPE const rotation_angle_rad, F_TYPE tolerance){
    if (vec3_is_null(rotation_axis)){
        if (F_TYPE_ABS(rotation_angle_rad) <= tolerance){
            q->r = F_TYPE_1;
            q->i = F_TYPE_0;
            q->j = F_TYPE_0;
            q->k = F_TYPE_0;

            return true;
        }
        else{
            return false;
        }
    }

    F_TYPE half_rotation_angle = rotation_angle_rad / F_TYPE_2;
    F_TYPE cos_of_half = F_TYPE_COS(half_rotation_angle);
    F_TYPE sin_of_half = F_TYPE_SIN(half_rotation_angle);
    F_TYPE norm_of_axis = vec3_norm(rotation_axis);

    q->r = cos_of_half;
    q->i = rotation_axis->i / norm_of_axis * sin_of_half;
    q->j = rotation_axis->j / norm_of_axis * sin_of_half;
    q->k = rotation_axis->k / norm_of_axis * sin_of_half;

    return true;
}

bool quat_to_rotation(vec3 * rotation_axis, F_TYPE * rotation_angle_rad, quat const * q_rotation, F_TYPE tolerance){
    if (!quat_is_unitary(q_rotation, tolerance)){
        return false;
    }
    else{
        *rotation_angle_rad = F_TYPE_2 * F_TYPE_ACOS(q_rotation->r);
        F_TYPE sin_half_angle = F_TYPE_SQRT(F_TYPE_1 - q_rotation->r * q_rotation->r);
        rotation_axis->i = q_rotation->i / sin_half_angle;
        rotation_axis->j = q_rotation->j / sin_half_angle;
        rotation_axis->k = q_rotation->k / sin_half_angle;

        return true;
    }
}

// the naive way, applying the definition; this is quite inefficient though
bool rotate_by_quat(vec3 * v, quat const * q, F_TYPE tolerance){
    if (!quat_is_unitary(q)){
        return false;
    }

    quat q_1;
    quat q_2;
    quat q_3;

    // q_1 is the quat out of v
    vec3_to_quat(v, &q_1);

    // q_2 is the rotation quat conjugate
    quat_copy(q, &q_2);
    quat_conj(&q_2);

    // q_3 contains the right part of the product
    quat_prod(&q_1, &q_2, &q_3);

    // q_2 is the rotation quat
    quat_conj(&q_2);

    // q_1 contains the full quaternion
    quat_prod(&q_2, &q_3, &q_1);

    // make the result available
    quat_to_vec3(&q_1, v);

    return true;
}

void rotate_by_quat_R(vec3 const * v, quat const * q, vec3 * Rv){
    // reminder of the formula:
    // q = [s, u]
    // R(v) = 2.0 ( (u . v) u + (s * s - 0.5) v + s (u x v) )

    F_TYPE u_dot_v = q->i * v->i + q->j * v->j + q->k * v->k;
    F_TYPE s2m05 = q->r * q->r - F_TYPE_05;
    Rv->i = F_TYPE_2 * ( u_dot_v * q->i + s2m05 * v->i + q->r * ( q->j * v->k - q->k * v->j ) );
    Rv->j = F_TYPE_2 * ( u_dot_v * q->j + s2m05 * v->j + q->r * ( q->k * v->i - q->i * v->k ) );
    Rv->k = F_TYPE_2 * ( u_dot_v * q->k + s2m05 * v->k + q->r * ( q->i * v->j - q->j * v->i ) );
}

// TODO: implement the other way to do, and test which is faster :)

#endif
