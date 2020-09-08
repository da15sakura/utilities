/*
GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
Copyright (C) 2020 Takuji Asada
Date: 2020-08-01
*/
//! @file
//! @brief	倍精度浮動小数点対応 座標・大きさ・矩形 宣言
//! @author	T.A.
//! @date	2011/05/30
#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <valarray>
#include <cassert>
//#include <stdexcept>
#define NOMINMAX	// min/maxの使用
#include <windows.h>	// tagPOINT, tagSIZE, tagRECTの定義（windows.hをincludeしたくなければ，左記定義の使用箇所をコメントアウトすること）

static constexpr double	st_zero = 1.0e-6;	// 必ず正値または0とする

template<class T> class CMatrix_;

//! 座標クラス
template<class T>
class CPoint_
{
public:
	CPoint_(): m_value(static_cast<T>(0),2) {}
	constexpr explicit CPoint_(const T& x, const T& y): m_value({x,y}) {}
	constexpr explicit CPoint_(const std::valarray<T>& val): m_value(val) {}
	constexpr explicit CPoint_(const POINT& crPoint): m_value({crPoint.x,crPoint.y}) {}

protected:
	std::valarray<T>	m_value;

public:
	constexpr const T& operator[](size_t pos) const noexcept {return m_value[2<=pos?1:pos];}
	constexpr const T& operator()(size_t pos) const noexcept {return this->operator[](pos);}
	constexpr const T& X() const noexcept {return m_value[0];}
	constexpr const T& Y() const noexcept {return m_value[1];}
	constexpr const std::valarray<T>& valarray() const noexcept {return m_value;}
	constexpr T& operator[](size_t pos) noexcept {return m_value[2<=pos?1:pos];}
	constexpr T& operator()(size_t pos) noexcept {return this->operator[](pos);}
	constexpr T& X() noexcept {return m_value[0];}
	constexpr T& Y() noexcept {return m_value[1];}
	constexpr void setXY(const T& x, const T& y) noexcept {m_value = {x,y};}
public:
	constexpr CPoint_<T> operator-() const noexcept {return std::move(CPoint_<T>(-m_value));}
	constexpr CPoint_<T> operator+(const CPoint_<T>& crPoint) const noexcept {return std::move(CPoint_<T>(this->m_value+crPoint.m_value));}
	constexpr CPoint_<T> operator-(const CPoint_<T>& crPoint) const noexcept {return std::move(this->operator+(-crPoint));}
	constexpr CPoint_<T> operator*(T factor) const noexcept {return std::move(CPoint_<T>(this->m_value*factor));}
	constexpr CPoint_<T> operator/(T factor) const noexcept {if(factor==0) {assert(false); return *this;} else {return std::move(this->operator*(static_cast<T>(1.0/factor)));}}
	constexpr CPoint_<T>& operator+=(const CPoint_<T>& crPoint) noexcept {return (this->operator=(this->operator+(crPoint)));}
	constexpr CPoint_<T>& operator-=(const CPoint_<T>& crPoint) noexcept {return (this->operator=(this->operator-(crPoint)));}
	constexpr CPoint_<T>& operator*=(T factor) noexcept {return (this->operator=(this->operator*(factor)));}
	constexpr CPoint_<T>& operator/=(T factor) noexcept {return (this->operator=(this->operator/(factor)));}
	constexpr bool operator==(const CPoint_<T>& p) const noexcept;
	constexpr bool operator!=(const CPoint_<T>& p) const noexcept {return !operator==(p);}
//	constexpr friend CPoint_<T> operator*(T factor, const CPoint_<T>& crPoint) noexcept;	// 内部構造にアクセスする必要がないので，friend定義の必要がない
public:
	constexpr void scale(T scale, const CPoint_<T>& center=CPoint_<T>(0,0)) noexcept {this->offset(-center); this->operator*=(scale); this->offset(center);}
	constexpr void offset(const CPoint_<T>& crOffset) noexcept {this->operator+=(crOffset);}
	constexpr void rotate(T angle, const CPoint_<T>& center=CPoint_<T>(0,0)) noexcept;
	constexpr T getDistance(const CPoint_<T>& crPoint) const noexcept {return static_cast<T>(std::sqrt((crPoint-*this).getSquareSum()));}
	constexpr operator POINT() const noexcept {return std::move(POINT{std::lround(this->operator[](0)),std::lround(this->operator[](1))});}
protected:
	constexpr T getSquareSum() const noexcept {T sum = m_value.apply([](const T& value){return value*value;}).sum(); if(sum<0) {assert(false); sum = 0;} return sum;}	// 戻り値は0以上
};
template<class T>
constexpr bool CPoint_<T>::operator==(const CPoint_<T>& p) const noexcept {
	std::valarray<T>	diff(p.m_value-this->m_value), diffabs(diff.apply(std::fabs));
	auto	isZero = [](T value){return value <= st_zero;};
	if (isZero(diffabs[0])==false || isZero(diffabs[1])==false) return false;	// □の外
	if (isZero(diffabs[0]+diffabs[1])) return true;	// ◇の中
	if ((diff[0]*diff[0]+diff[1]*diff[1]) <= st_zero*st_zero) return true;	// ○の中
	else                                                      return false;
}
template<class T>
constexpr void CPoint_<T>::rotate(T angle, const CPoint_<T>& center/*=CPoint_<T>(0,0)*/) noexcept {
	this->offset(-center);
#if 0
	double	sin_ = sin(angle), cos_ = cos(angle);
	double	x = this->operator[](0)*cos_ - this->operator[](1)*sin_, y = this->operator[](0)*sin_ + this->operator[](1)*cos_;
	this->m_value = {x,y};
#else
	*this = CMatrix_<T>(angle)*(*this);
#endif
	this->offset(center);
}
template<class T>
constexpr CPoint_<T> operator*(T factor, const CPoint_<T>& crPoint) noexcept {return std::move(crPoint*factor);}
using CDoublePoint = CPoint_<double>;

//! ベクトルクラス
template<class T>
class CVector_ : public CPoint_<T>
{
public:
	CVector_():CPoint_<T>() {};
	CVector_(const T& x, const T& y): CPoint_<T>(x,y) {};
	CVector_(const POINT& crPoint): CPoint_<T>(crPoint) {};
	CVector_(const CPoint_<T>& crPoint): CPoint_<T>(crPoint) {};
public:
	T dot(const CVector_& v) const {return (this->operator[](0)*v[0] + this->operator[](1)*v[1]);}
	T crs(const CVector_& v) const {return (this->operator[](0)*v[1] - this->operator[](1)*v[0]);}
};
using CDoubleVector = CVector_<double>;

//! 行列クラス
//! note これを直接的に使うことはあまりない
// 同次行列ではない
template <class T>
class CMatrix_
{
public:
	CMatrix_(): m_elem({1,0,0,1}) {};
	constexpr explicit CMatrix_(const T& angle) {this->setRotate(angle);}
	constexpr explicit CMatrix_(const T& scale, const void* const dummy) {this->setScale(scale);}	// dummyはangleと区別するため
	constexpr explicit CMatrix_(const std::valarray<T>& val): m_elem(val) {}
private:
	std::valarray<T>	m_elem;
	static constexpr size_t	m_dim = 2;
private:
	constexpr size_t correctElemNo(size_t row, size_t col) const noexcept {size_t row_=(row<m_dim)?(row):(m_dim-1), col_=(col<m_dim)?(col):(m_dim-1); return (row_*m_dim+col_);}
public:
	constexpr const T& operator()(unsigned int row, unsigned int col) const noexcept {return this->m_elem[correctElemNo(row,col)];}
	constexpr T& operator()(unsigned int row, unsigned int col) noexcept {return this->m_elem[correctElemNo(row,col)];}
	constexpr void setElem(size_t row, size_t col, T val) noexcept {this->operator()(row,col) = val;}
	constexpr T getElem(size_t row, size_t col, T val) const noexcept {return this->operator()(row,col);}
	constexpr void setElem(const std::valarray<T>& val) noexcept {m_elem = val;}
	constexpr const std::valarray<T>& valarray() const noexcept {return m_elem;}
public:
	constexpr CMatrix_<T> operator-() const noexcept {return std::move(CMatrix_<T>(-m_elem));}
	constexpr CMatrix_<T> operator+(const CMatrix_<T>& crMatrix) const noexcept {return std::move(CMatrix_<T>(this->m_elem+crMatrix.m_elem));}
	constexpr CMatrix_<T> operator-(const CMatrix_<T>& crMatrix) const noexcept {return std::move(this->operator+(-crMatrix));}
	constexpr CMatrix_<T> operator*(T factor) const noexcept {return std::move(CMatrix_<T>(this->m_elem*factor));}
	constexpr CMatrix_<T> operator/(T factor) const noexcept {if(factor==0) {assert(false); return *this;} else {return std::move(this->operator*(static_cast<T>(1.0/factor)));}}
	constexpr CMatrix_<T>& operator+=(const CMatrix_<T>& crMatrix) noexcept {return (this->operator=(this->operator+(crMatrix)));}
	constexpr CMatrix_<T>& operator-=(const CMatrix_<T>& crMatrix) noexcept {return (this->operator=(this->operator-(crMatrix)));}
	constexpr CMatrix_<T>& operator*=(T factor) noexcept {return (this->operator=(this->operator*(factor)));}
	constexpr CMatrix_<T>& operator/=(T factor) noexcept {return (this->operator=(this->operator/(factor)));}
public:
	constexpr CMatrix_<T> operator*(const CMatrix_<T> crMatrix) const noexcept;
	constexpr CMatrix_<T>& operator*=(const CMatrix_<T> crMatrix) noexcept {return (this->operator=(this->operator*(crMatrix)));}
	constexpr CPoint_<T> operator*(const CPoint_<T>& crPoint) const noexcept;
//	constexpr friend CMatrix_<T> operator*(T factor, const CMatrix_<T>& crMatrix) noexcept;	// 内部構造にアクセスする必要がないので，friend定義の必要がない
public:
	constexpr void setRotate(T angle) noexcept {T sc[2] = {sin(angle),cos(angle)}; m_elem = {sc[1],-sc[0],sc[0],sc[1]};}
	constexpr void setScale(T scale) noexcept {m_elem = {scale,0.0,0.0,scale};}
public:
	constexpr bool isRegular() const noexcept {double det=this->getDeterminant(); return st_zero*st_zero<det*det;}
	constexpr double getDeterminant() const noexcept {return this->m_elem[0]*this->m_elem[3]-this->m_elem[1]*this->m_elem[2];}
	constexpr CMatrix_<T> getInverse() const noexcept {double det=this->getDeterminant(); if(det==0.0){assert(false); return *this;} return std::move(CMatrix_({this->m_elem[3],-this->m_elem[1],-this->m_elem[2],this->m_elem[0]})/det);}
	constexpr CMatrix_<T>& inverse() noexcept {return (this->operator=(this->getInverse()));}
};
template<class T>
constexpr CMatrix_<T> CMatrix_<T>::operator*(const CMatrix_<T> crMatrix) const noexcept {
	// sliceは(先頭要素,要素数,間隔)
	// 正方行列同士の掛け算の一般式で書いてみた
	std::valarray<T>	multi(m_dim*m_dim);
	for (size_t row = 0; row < m_dim; row++) {
		for (size_t col = 0; col < m_dim; col++) {
			multi[row*m_dim+col] = (this->m_elem[std::slice(row*m_dim,m_dim,1)]*crMatrix.m_elem[std::slice(col,m_dim,m_dim)]).sum();
		}
	}
	return std::move(CMatrix_<T>(multi));
}
template<class T>
constexpr CPoint_<T> CMatrix_<T>::operator*(const CPoint_<T>& crPoint) const noexcept {
	// sliceは(先頭要素,要素数,間隔)
	// 掛け算の一般式で書いてみた
	std::valarray<T>	multi(m_dim);
	for (size_t row = 0; row < m_dim; row++) {
		multi[row] = (this->m_elem[std::slice(row*m_dim,m_dim,1)]*crPoint.valarray()).sum();
	}
	return std::move(CPoint_<T>(multi));
}
template<class T>
constexpr CMatrix_<T> operator*(T factor, const CMatrix_<T>& crMatrix) noexcept {return std::move(crMatrix*factor);}
using CDoubleMatrix = CMatrix_<double>;
