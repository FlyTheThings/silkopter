#pragma once

namespace math
{

///////////////////////////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////////////////////////
template<typename T> struct vec2;
template<typename T> struct vec3;
template<typename T> struct mat3;

//column-major (OpenGL) layout

//	.--------------- column0 - x axis
//	|   .----------- column1 - y axis
//  |	|	.------- column2 - translation
//  0   3   6
//  1   4   7
//  2   5   8

template <typename T>
struct mat3
{
	typedef T value_t;
	static mat3<T> const zero;
	static mat3<T> const one;
	static mat3<T> const identity;
	static uint8_t const row_count = 3;
	static uint8_t const column_count = 3;
	static uint8_t const element_count = 9;


	///////////////////////////////////////////////////////////////////////////////
	// constructors
	///////////////////////////////////////////////////////////////////////////////
	mat3();
	mat3(math::ZUninitialized);
	explicit mat3(T value);
    mat3(mat3<T> const&) = default;
    mat3(mat3<T>&&) = default;
    template<typename U> explicit mat3(mat3<U> const& v);

	//casting
	explicit mat3(mat2<T> const& v);
	explicit mat3(mat4<T> const& v);

	mat3(T const v0, T const v1, T const v2, 
				T const v3, T const v4, T const v5, 
				T const v6, T const v7, T const v8);

	mat3(vec3<T> const& column0, vec3<T> const& column1, vec3<T> const& column2);

    template<class Policy = standard> static mat3<T> look_at(vec3<T> const& front, vec3<T> const& up);

	///////////////////////////////////////////////////////////////////////////////
	// methods
	///////////////////////////////////////////////////////////////////////////////

	void set(T const values[9]);

	void set_identity();
	template<class Policy = standard> bool invert();
	void transpose();

public:
	//Rows are NOT linearly in memory. First row is m[0], m[3], m[6]
	vec3<T> get_row(uint8_t row) const;
	void set_row(uint8_t row, vec3<T> const& v);

	//Columns are linearly in memory. First row is m[0], m[1], m[2] and is the X axis of the matrix
	vec3<T> const& get_column(uint8_t column) const;
	void set_column(uint8_t column, vec3<T> const& v);

	vec3<T> const& get_axis_x() const;
	void set_axis_x(vec3<T> const& axis);

	vec3<T> const& get_axis_y() const;
	void set_axis_y(vec3<T> const& axis);

	vec3<T> const& get_axis_z() const;
	void set_axis_z(vec3<T> const& axis);

	vec3<T> get_scale() const;
	void set_scale(vec3<T> const& scale);

	void post_scale(vec3<T> const& scale);

	///////////////////////////////////////////////////////////////////////////////
	// operators

	bool operator==(mat3<T> const& v) const;
	bool operator!=(mat3<T> const& v) const;

	mat3<T>& operator=(mat2<T> const& m);
	mat3<T>& operator=(mat4<T> const& m);

    mat3<T>& operator=(mat3<T> const&) = default;
    mat3<T>& operator=(mat3<T>&&) = default;

	///////////////////////////////////////////////////////////////////////////////
	// indexing operators
	///////////////////////////////////////////////////////////////////////////////
	T* data();
	T const* data() const;

	T& operator()(uint8_t column, uint8_t row);
	T const& operator()(uint8_t column, uint8_t row) const;

	///////////////////////////////////////////////////////////////////////////////
	// arithmetic operators
	///////////////////////////////////////////////////////////////////////////////

	mat3<T> operator*(mat3<T> const& other) const;
	mat3<T> operator+(mat3<T> const& other) const;
	mat3<T> operator-(mat3<T> const& other) const;
	mat3<T> operator*(T scalar) const;
	mat3<T> operator+(T scalar) const;
	mat3<T> operator-(T scalar) const;
	mat3<T>& operator*=(mat3<T> const& other);
	mat3<T>& operator+=(mat3<T> const& other);
	mat3<T>& operator-=(mat3<T> const& other);
	mat3<T>& operator*=(T scalar);
	mat3<T>& operator+=(T scalar);
	mat3<T>& operator-=(T scalar);

	///////////////////////////////////////////////////////////////////////////////
	//members
	///////////////////////////////////////////////////////////////////////////////
	union
	{
		T m[element_count];
		struct  
		{
			vec3<T> column0;
			vec3<T> column1;
			vec3<T> column2;
		};
	};
};

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////


}//namespace math
