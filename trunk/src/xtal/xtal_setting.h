/** \file src/xtal/xtal_setting.h
* \brief src/xtal/xtal_setting.h
*/

#ifndef XTAL_SETTING_H_INCLUDE_GUARD
#define XTAL_SETTING_H_INCLUDE_GUARD

#pragma once

/* \defgroup settingmacro Xtalの挙動を変更するためのマクロ */
/*@{*/

/** \def XTAL_NO_PARSER
* \brief パーサ使用off
*/
//#define XTAL_NO_PARSER

/** \def XTAL_USE_WCHAR
* \brief wchar_t使用on
*/
//#define XTAL_USE_WCHAR

/** \def XTAL_NO_THREAD
* \brief スレッド使用off
*/
//#define XTAL_NO_THREAD

/** \def XTAL_USE_PTHREAD_TLS
* \brief pthreadのスレッドローカルストレージを使う
*/
//#define XTAL_USE_PTHREAD_TLS

/*@}*/

#endif // XTAL_SETTING_H_INCLUDE_GUARD
