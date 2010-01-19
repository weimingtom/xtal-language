/** \file src/xtal/xtal_setting.h
* \brief src/xtal/xtal_setting.h
*/

#ifndef XTAL_SETTING_H_INCLUDE_GUARD
#define XTAL_SETTING_H_INCLUDE_GUARD

#pragma once

/**
* \brief パーサ使用off
*/
//#define XTAL_NO_PARSER

/**
* \brief wchar_t使用on
*/
//#define XTAL_USE_WCHAR

/**
* \brief スレッド使用off
*/
//#define XTAL_NO_THREAD

/**
* \brief pthreadのスレッドローカルストレージを使う
*/
//#define XTAL_USE_PTHREAD_TLS

/**
* \brief 小さいサイズのメモリ確保にXtal独自のアロケータを使わない
*/
//#define XTAL_NO_SMALL_ALLOCATOR

/**
* \brief 可能な限り整数の範囲を大きくする
*/
//#define XTAL_USE_LARGE_INT

#endif // XTAL_SETTING_H_INCLUDE_GUARD
