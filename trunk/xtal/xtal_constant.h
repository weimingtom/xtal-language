
#pragma once

#include "xtal_inst.h"

namespace xtal{
	
enum{
	KIND_BLOCK,
	KIND_CLASS
};

enum{
	KIND_FUN,
	KIND_LAMBDA,
	KIND_METHOD,
	KIND_FIBER,
};

enum{
	KIND_STRING,
	KIND_TEXT,
	KIND_FORMAT
};

enum{
	KIND_PUBLIC = 0,
	KIND_PROTECTED = 1<<0,
	KIND_PRIVATE = 1<<1,
};

enum{
	BREAKPOINT_LINE,
	BREAKPOINT_CALL,
	BREAKPOINT_RETURN
};

/** @addtogroup code */
/*@{*/

#if 0
/// バイトコードの識別ID
enum CodeType{

/**
* @brief 何もしない。
*
* format op
* stack [] -> []
*/
CODE_NOP,

/**
* @brief スタックにnull値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_NULL,

/**
* @brief スタックにtrue値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_TRUE,

/**
* @brief スタックにfalse値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FALSE,

/**
* @brief スタックにnop値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_NOP,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_0,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_1,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_2,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_3,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_4,

/**
* @brief スタックにint値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_INT_5,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_0,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_0_25,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_0_5,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_1,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_2,

/**
* @brief スタックにfloat値をプッシュする。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_FLOAT_3,

/**
* @brief スタックにint値をプッシュする。
*
* format op, value
* stack [] -> [value]
*/
CODE_PUSH_INT_1BYTE,

/**
* @brief スタックにint値をプッシュする。
*
* format op, value1, value2
* stack [] -> [value]
*/
CODE_PUSH_INT_2BYTE,

/**
* @brief 現在のthisをスタックに積む。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_THIS,

/**
* @brief 引数のチェック
*
* format op, goto1, goto2, argid
* stack [] -> []
*/
CODE_IF_ARG_IS_NULL,

/**
* @brief スタックトップの値を一つ前に入れる。
*
* format op
* stack [value1, value2] -> [value2, value1]
*/
CODE_INSERT_1,

/**
* @brief スタックトップの値を二つ前に入れる。
*
* format op
* stack [value1, value2, value3] -> [value3, value1, value2]
*/
CODE_INSERT_2,

/**
* @brief スタックトップの値を三つ前に入れる。
*
* format op
* stack [value1, value2, value3, value4] -> [value4, value1, value2, value3]
*/
CODE_INSERT_3,

/**
* @brief スタックトップの値をポップする。
*
* format op
* stack [value] -> []
*/
CODE_POP,

/**
* @brief スタックトップの値をプッシュする。
*
* format op
* stack [value] -> [value, value]
*/
CODE_DUP,

/**
* @brief 値の数を調整する
*
* format op, retult_count, need_result_count, flags
* stack [] -> []
*/
CODE_ADJUST_RESULT,

/**
* @brief 条件分岐
*
* format op, goto1, goto2
* stack [value] -> []
*/
CODE_IF,

/**
* @brief 条件分岐
*
* format op, goto1, goto2
* stack [value] -> []
*/
CODE_UNLESS,

/**
* @brief 無条件分岐
*
* format op, goto1, goto2
* stack [] -> []
*/
CODE_GOTO,

/**
* @brief 関数呼び出し
*
* format op, ...
* stack [target] -> [result 0..num]
*/
CODE_CALL,

/**
* @brief 関数呼び出し
*
* format op, ...
* stack [target] -> [result 0..num]
*/
CODE_CALLEE,

/**
* @brief メソッド呼び出し
*
* format op, sym, ...
* stack [target] -> [result 0..num]
*/
CODE_SEND,

/**
* @brief メソッド呼び出し
*
* format op, sym, ...
* stack [target] -> [result 0..num]
*/
CODE_SEND_IF_DEFINED,

/**
* @brief 関数から抜ける
*
* format op
* stack [] -> []
*/
CODE_RETURN_0,

/**
* @brief 関数から抜ける
*
* format op
* stack [value1] -> []
*/
CODE_RETURN_1,

/**
* @brief 関数から抜ける
*
* format op
* stack [value1, value2] -> []
*/
CODE_RETURN_2,

/**
* @brief 関数から抜ける
*
* format op, N
* stack [value1, value2, ... valueN] -> []
*/
CODE_RETURN_N,

/**
* @brief 関数呼び出しの後始末をする
*
* format op
* stack [] -> []
*/
CODE_CLEANUP_CALL,

/**
* @brief fiberの実行を一時中断する
format; op, N
* stack [value1, value2, ... valueN] -> []
*/
CODE_YIELD,

/**
* @brief 例外フレームを開始
*
* format op, catch1, catch2, finally1, finally2, end1, end2
* stack [] -> []
*/
CODE_TRY_BEGIN,

/**
* @brief 例外フレームを終了
*
* format op
* stack [] -> []
*/
CODE_TRY_END,

/**
* @brief ブロックを開始する。
*
* format op, size1, size2
* stack [] -> []
*/
CODE_BLOCK_BEGIN,

/**
* @brief ブロックを閉じる。
*
* format op
* stack [] -> []
*/
CODE_BLOCK_END,

/**
* @brief ヒープに乗らないと静的に確定しているブロックを閉じる。
*
* format op
* stack [] -> []
*/
CODE_BLOCK_END_NOT_ON_HEAP,

/**
* @brief インスタンス変数を取り出す。
*
* format op, symbol1, symbol2
* stack [object] -> [result]
*/
CODE_INSTANCE_VARIABLE,

/**
* @brief インスタンス変数に値を設定する。
*
* format op, symbol1, symbol2
* stack [object, value] -> []
*/
CODE_SET_INSTANCE_VARIABLE,

/**
* @brief ローカル変数を取り出す。
*
* format op
* stack [] -> [value]
*/
CODE_LOCAL_0,

/**
* @brief ローカル変数を取り出す。
*
* format op
* stack [] -> [value]
*/
CODE_LOCAL_1,

/**
* @brief ローカル変数を取り出す。
*
* format op
* stack [] -> [value]
*/
CODE_LOCAL_2,

/**
* @brief ローカル変数を取り出す。
*
* format op
* stack [] -> [value]
*/
CODE_LOCAL_3,

/**
* @brief ローカル変数を取り出す。
*
* format op, pos
* stack [] -> [value]
*/
CODE_LOCAL,

/**
* @brief ヒープに乗らないと静的に確定しているローカル変数を取り出す。
*
* format op, pos
* stack [] -> [value]
*/
CODE_LOCAL_NOT_ON_HEAP,

/**
* @brief ローカル変数を取り出す。
*
* format op, pos1, pos2
* stack [] -> [value]
*/
CODE_LOCAL_W,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL_0,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL_1,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL_2,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL_3,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL,

/**
* @brief ヒープに乗らないと静的に確定しているローカル変数に値を設定する。
*
* format op, pos
* stack [] -> [value]
*/
CODE_SET_LOCAL_NOT_ON_HEAP,

/**
* @brief ローカル変数に値を設定する。
*
* format op, pos1, pos2
* stack [] -> [value]
*/
CODE_SET_LOCAL_W,

/**
* @brief グローバル変数を取り出す。
*
* format op, symbol1, symbol2
* stack [] -> [value]
*/
CODE_GLOBAL,

/**
* @brief グローバル変数に設定する。
*
* format op, symbol1, symbol2
* stack [value] -> []
*/
CODE_SET_GLOBAL,

/**
* @brief グローバル変数を定義する。
*
* format op, symbol1, symbol2
* stack [value] -> []
*/
CODE_DEFINE_GLOBAL,

/**
* @brief オブジェクトのメンバを取り出す。
*
* format op, symbol1, symbol2
* stack [object] -> [result]
*/
CODE_MEMBER,

/**
* @brief オブジェクトのメンバを取り出す。
*
* format op, symbol1, symbol2
* stack [object] -> [result]
*/
CODE_MEMBER_IF_DEFINED,

/**
* @brief オブジェクトのメンバを定義する。
*
* format op, symbol1, symbol2
* stack [object, value] -> []
*/
CODE_DEFINE_MEMBER,

/**
* @brief 配列の要素を取り出す。
*
* format op
* stack [object, index] -> [result]
*/
CODE_AT,

/**
* @brief 配列の要素を設定する。
*
* format op
* stack [object, index, value] -> []
*/
CODE_SET_AT,

/**
* @brief once演算子
*
* format op, goto1, goto2, value1, value2
* stack [] -> []
*/
CODE_ONCE,

/**
* @brief オブジェクトテーブルから値を取り出す。
*
* format op, pos1, pos2
* stack [] -> [result]
*/
CODE_GET_VALUE,

/**
* @brief オブジェクトテーブルに値を設定する。
*
* format op, pos1, pos2
* stack [value] -> []
*/
CODE_SET_VALUE,

/**
* @brief スタックにジャンプ位置を埋め込む
*
* format op, goto1, goto2
* stack [] -> [address]
*/
CODE_PUSH_GOTO,

/**
* @brief スタックからポップしたアドレスにジャンプする
*
* format op
* stack [] -> []
*/
CODE_POP_GOTO,

/**
* @brief 単項!の演算を行う。
*
* format op
* stack [value] -> [result]
*/
CODE_NOT,

/**
* @brief 単項+の演算を行う。
*
* format op
* stack [value] -> [result]
*/
CODE_POS,

/**
* @brief 単項-の演算を行う。
*
* format op
* stack [value] -> [result]
*/
CODE_NEG,

/**
* @brief 単項~の演算を行う。
*
* format op
* stack [value] -> [result]
*/
CODE_COM,

/**
* @brief クローンを生成する
*
* format op
* stack [value] -> [result]
*/
CODE_CLONE,

/**
* @brief 二項+の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_ADD,

/**
* @brief 二項-の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SUB,

/**
* @brief 二項~の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_CAT,

/**
* @brief 二項*の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_MUL,

/**
* @brief 二項/の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_DIV,

/**
* @brief 二項%の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_MOD,

/**
* @brief 二項&の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_AND,

/**
* @brief 二項&の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_OR,

/**
* @brief 二項^の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_XOR,

/**
* @brief 二項>>の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SHR,

/**
* @brief 二項>>>の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_USHR,

/**
* @brief 二項<<の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SHL,

/**
* @brief 二項==の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_EQ,

/**
* @brief 二項!=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_NE,

/**
* @brief 二項<の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_LT,

/**
* @brief 二項>の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_GT,

/**
* @brief 二項<=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_LE,

/**
* @brief 二項>=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_GE,

/**
* @brief 二項===の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_RAW_EQ,

/**
* @brief 二項!==の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_RAW_NE,

/**
* @brief 二項isの演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_IS,

/**
* @brief 二項!isの演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_NIS,

/**
* @brief 二項==の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_EQ_IF,

/**
* @brief 二項!=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_NE_IF,

/**
* @brief 二項<の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_LT_IF,

/**
* @brief 二項>の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_GT_IF,

/**
* @brief 二項<=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_LE_IF,

/**
* @brief 二項>=の演算を行い、条件分岐する。
*
* format op, goto1, goto2
* stack [value1, value2] -> [result]
*/
CODE_GE_IF,

/**
* @brief ローカル変数をインクリメントする
*
* format op
* stack [value] -> [result]
*/
CODE_INC,

/**
* @brief ローカル変数をデクリメントする
*
* format op
* stack [value] -> [result]
*/
CODE_DEC,

/**
* @brief ローカル変数をインクリメントする
*
* format op, variable1, variable2
* stack [] -> []
*/
CODE_LOCAL_NOT_ON_HEAP_INC,

/**
* @brief ローカル変数をデクリメントする
*
* format op, variable1, variable2
* stack [] -> []
*/
CODE_LOCAL_NOT_ON_HEAP_DEC,

/**
* @brief 二項+の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_ADD_ASSIGN,

/**
* @brief 二項-の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SUB_ASSIGN,

/**
* @brief 二項~の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_CAT_ASSIGN,

/**
* @brief 二項*の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_MUL_ASSIGN,

/**
* @brief 二項/の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_DIV_ASSIGN,

/**
* @brief 二項%の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_MOD_ASSIGN,

/**
* @brief 二項&の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_AND_ASSIGN,

/**
* @brief 二項&の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_OR_ASSIGN,

/**
* @brief 二項^の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_XOR_ASSIGN,

/**
* @brief 二項>>の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SHR_ASSIGN,

/**
* @brief 二項>>>の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_USHR_ASSIGN,

/**
* @brief 二項<<の演算を行う。
*
* format op
* stack [value1, value2] -> [result]
*/
CODE_SHL_ASSIGN,

/**
* @brief 配列を作る。
*
* format op, count
* stack [] -> [value]
*/
CODE_PUSH_ARRAY,

/**
* @brief 連想配列を作る。
*
* format op, count
* stack [] -> [value]
*/
CODE_PUSH_MAP,

/**
* @brief 現在の呼ばれている関数自身をスタックに積む。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_CALLEE,

/**
* @brief 関数を作る。
*
* format op, type, num1, num2
* stack [] -> [value]
*/
CODE_PUSH_FUN,

/**
* @brief 現在のフレームをスタックに積む。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_CURRENT_CONTEXT,

/**
* @brief 現在の残り引数オブジェクトをスタックに積む。
*
* format op
* stack [] -> [value]
*/
CODE_PUSH_ARGS,

/**
* @brief フレームの開始
*
* format op, type, core1, core2
* stack [] -> []
*/
CODE_CLASS_BEGIN,

/**
* @brief フレームを閉じる
*
* format op
* stack [] -> [value]
*/
CODE_CLASS_END,

/**
* @brief 名前をつける
*
* format op, size1, size2
* stack [value] -> [value]
*/
CODE_SET_NAME,

/**
* @brief 直前のメンバ参照が有効かチェックする。
*
* format op
* stack [value] -> [result]
*/
CODE_CHECK_UNSUPPORTED,

/**
* @brief 表明。
*
* format op
* stack [expr, expr_String, message] -> []
*/
CODE_ASSERT,

/**
* @brief 可触性を設定する
*
* format op, type
*/
CODE_SET_ACCESSIBILITY,

/**
* @brief 配列に要素を追加する。
*
* format op
* stack [arrya, value] -> [array]
*/
CODE_ARRAY_APPEND,

/**
* @brief 連想配列に要素を追加する。
*
* format op
* stack [map, key, value] -> [map]
*/
CODE_MAP_APPEND,

/**
* @brief 仮想マシンのループから脱出する。
*
* format op
* stack [] -> []
*/
CODE_EXIT,

/**
* @brief ブレークポイント
*
* format op, kind
* stack [] -> []
*/
CODE_BREAKPOINT,

/**
* @brief 例外を投げる
*
* format op
* stack [value] -> []
*/
CODE_THROW,

/**
* @brief 例外UnsupportedErrorを投げる。
*
* format op
* stack [] -> []
*/
CODE_THROW_UNSUPPORTED_ERROR,

/**
* @brief nullを投げる。
*
* format op
* stack [] -> []
*/
CODE_THROW_NULL,



CODE_MAX

};

#endif

/*@}*/

}

