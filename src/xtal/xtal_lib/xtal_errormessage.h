
#pragma once

namespace xtal{

inline void bind_error_message(){
	const char_t* messages[] = {
		XTAL_STRING("Xtal Compile Error 1001"), XTAL_STRING("構文エラーです。"),
		XTAL_STRING("Xtal Compile Error 1002"), XTAL_STRING("予期せぬ文字 '%(char)s' が検出されました。"),
		XTAL_STRING("Xtal Compile Error 1003"), XTAL_STRING("';' がありません。"),
		XTAL_STRING("Xtal Compile Error 1004"), XTAL_STRING("不正な関数の仮引数です。"),
		XTAL_STRING("Xtal Compile Error 1005"), XTAL_STRING("非名前付き引数が名前付き引数の後にあります。"),
		XTAL_STRING("Xtal Compile Error 1006"), XTAL_STRING("不正なbreak文、またはcontinue文です。"),
		XTAL_STRING("Xtal Compile Error 1008"), XTAL_STRING("不正な多重代入文です。"),
		XTAL_STRING("Xtal Compile Error 1009"), XTAL_STRING("定義されていない変数 '%(name)s' に代入しようとしました 。"),
		XTAL_STRING("Xtal Compile Error 1010"), XTAL_STRING("不正な数字リテラルのサフィックスです。"),
		XTAL_STRING("Xtal Compile Error 1011"), XTAL_STRING("文字列リテラルの途中でファイルが終わりました。"),
		XTAL_STRING("Xtal Compile Error 1012"), XTAL_STRING("不正な代入文の左辺です。"),
		XTAL_STRING("Xtal Compile Error 1013"), XTAL_STRING("比較演算式の結果を演算しようとしています。"),
		XTAL_STRING("Xtal Compile Error 1014"), XTAL_STRING("不正な浮動小数点数リテラルです。"),
		XTAL_STRING("Xtal Compile Error 1015"), XTAL_STRING("不正な%(n)d進数値リテラルのサフィックスです。"),
		XTAL_STRING("Xtal Compile Error 1016"), XTAL_STRING("assert文の引数の数が不正です。"),
		XTAL_STRING("Xtal Compile Error 1017"), XTAL_STRING("不正な%%記法リテラルです。"),
		XTAL_STRING("Xtal Compile Error 1018"), XTAL_STRING("default節が重複定義されました。"),
		XTAL_STRING("Xtal Compile Error 1019"), XTAL_STRING("'%(name)s'は代入不可能です。"),
		XTAL_STRING("Xtal Compile Error 1021"), XTAL_STRING("コメントの途中でファイルが終わりました。"),
		XTAL_STRING("Xtal Compile Error 1022"), XTAL_STRING("関数から返せる多値の最大は255個です。"),
		XTAL_STRING("Xtal Compile Error 1023"), XTAL_STRING("定義されていないインスタンス変数名 '%(name)s' を参照しています。"),
		XTAL_STRING("Xtal Compile Error 1024"), XTAL_STRING("同名のインスタンス変数名 '%(name)s' が既に定義されています。"),
		XTAL_STRING("Xtal Compile Error 1025"), XTAL_STRING("比較演算式の結果を最比較しようとしています。"),
		XTAL_STRING("Xtal Compile Error 1026"), XTAL_STRING("同じスコープ内で、同じ変数名 '%(name)s' が既に定義されています。"),
		XTAL_STRING("Xtal Compile Error 1027"), XTAL_STRING("コードが大きすぎて、バイトコードの生成に失敗しました。"),
		XTAL_STRING("Xtal Compile Error 1028"), XTAL_STRING("演算子の前後の空白と演算子の優先順位が一致していません。想定している優先順位と異なっている可能性があります。"),
		
		XTAL_STRING("Xtal Runtime Error 1001"), XTAL_STRING("'%(object)s' 関数呼び出しの '%(no)s'番目の引数の型が不正です。'%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。"),
		XTAL_STRING("Xtal Runtime Error 1002"), XTAL_STRING("evalに渡されたソースのコンパイル中、コンパイルエラーが発生しました。"),
		XTAL_STRING("Xtal Runtime Error 1003"), XTAL_STRING("不正なインスタンス変数の参照です。"),
		XTAL_STRING("Xtal Runtime Error 1004"), XTAL_STRING("型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。"),
		XTAL_STRING("Xtal Runtime Error 1005"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。'%(min)s'以上の引数を受け取る関数に、%(value)s個の引数を渡しました。"),
		XTAL_STRING("Xtal Runtime Error 1006"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。'%(min)s'以上、'%(max)s'以下の引数を受け取る関数に、'%(value)s'個の引数を渡しました。"),
		XTAL_STRING("Xtal Runtime Error 1007"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。引数を取らない関数に、'%(value)s'個の引数を渡しました。"),
		XTAL_STRING("Xtal Runtime Error 1008"), XTAL_STRING("'%(object)s'はシリアライズできません。"),
		XTAL_STRING("Xtal Runtime Error 1009"), XTAL_STRING("不正なコンパイル済みXtalファイルです。"),
		XTAL_STRING("Xtal Runtime Error 1010"), XTAL_STRING("コンパイルエラーが発生しました。"),
		XTAL_STRING("Xtal Runtime Error 1011"), XTAL_STRING("%(object)s :: '%(name)s' は既に定義されています。"),
		XTAL_STRING("Xtal Runtime Error 1012"), XTAL_STRING("yieldがfiberの非実行中に実行されました。"),
		XTAL_STRING("Xtal Runtime Error 1013"), XTAL_STRING("%(object)s :: new 関数が登録されていないため、インスタンスを生成できません。"),
		XTAL_STRING("Xtal Runtime Error 1014"), XTAL_STRING("ファイル '%(name)s' を開けません。"),
		XTAL_STRING("Xtal Runtime Error 1015"), XTAL_STRING("%(object)s は定義されていません。"),
		XTAL_STRING("Xtal Runtime Error 1016"), XTAL_STRING("ファイル '%(name)s' のコンパイル中、コンパイルエラーが発生しました。"),
		XTAL_STRING("Xtal Runtime Error 1017"), XTAL_STRING("%(object)s :: %(primary_key)s # %(secondary_key)sは %(accessibility)s です。"),
		XTAL_STRING("Xtal Runtime Error 1018"), XTAL_STRING("既に閉じられたストリームです。"),
		XTAL_STRING("Xtal Runtime Error 1019"), XTAL_STRING("C++で定義されたクラスの多重継承は出来ません。"),
		XTAL_STRING("Xtal Runtime Error 1020"), XTAL_STRING("配列の範囲外アクセスです。"),
		XTAL_STRING("Xtal Runtime Error 1021"), XTAL_STRING("%(object)s は定義されていません。'%(pick)s'と間違えている可能性があります。"),
		XTAL_STRING("Xtal Runtime Error 1023"), XTAL_STRING("1より長い文字列は範囲演算子に指定できません。"),
		XTAL_STRING("Xtal Runtime Error 1024"), XTAL_STRING("0除算エラーです。"),
		XTAL_STRING("Xtal Runtime Error 1025"), XTAL_STRING("ChRangeは閉区間である必要があります。"),
		XTAL_STRING("Xtal Runtime Error 1026"), XTAL_STRING("Xeg要素に変換できません。"),
		XTAL_STRING("Xtal Runtime Error 1027"), XTAL_STRING("cap関数の引数が不正です。cap(name: value)というように名前付き引数にするか、cap(\"name\"), value)というように呼んでください。"),
		XTAL_STRING("Xtal Runtime Error 1028"), XTAL_STRING("finalマークが付けられたクラス'%(name)s'を継承しようとしました。"),
		XTAL_STRING("Xtal Runtime Error 1029"), XTAL_STRING("C++で定義されたクラス'%(name)s'は、クラス生成時のみ継承が可能です。"),
		XTAL_STRING("Xtal Runtime Error 1030"), XTAL_STRING("暗黙の変数参照があります。%(name)s"),
	};
	
	for(int i=0; i<sizeof(messages)/sizeof(*messages)/2; ++i){
		text_map()->set_at(messages[i*2+0], messages[i*2+1]);
	}
}
	
}
