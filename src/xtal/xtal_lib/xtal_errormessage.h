
#pragma once

namespace xtal{

inline void bind_error_message(){
	const StringLiteral messages[] = {
		XTAL_STRING("XCE1001"), XTAL_STRING("構文エラーです。"),
		XTAL_STRING("XCE1002"), XTAL_STRING("予期せぬ文字 '%(char)s' が検出されました。"),
		XTAL_STRING("XCE1003"), XTAL_STRING("';' がありません。"),
		XTAL_STRING("XCE1004"), XTAL_STRING("不正な関数の仮引数です。"),
		XTAL_STRING("XCE1005"), XTAL_STRING("非名前付き引数が名前付き引数の後にあります。"),
		XTAL_STRING("XCE1006"), XTAL_STRING("不正なbreak文、またはcontinue文です。"),
		XTAL_STRING("XCE1008"), XTAL_STRING("不正な多重代入文です。"),
		XTAL_STRING("XCE1009"), XTAL_STRING("定義されていない変数 '%(name)s' に代入しようとしました 。"),
		XTAL_STRING("XCE1010"), XTAL_STRING("不正な数字リテラルのサフィックスです。"),
		XTAL_STRING("XCE1011"), XTAL_STRING("文字列リテラルの途中でファイルが終わりました。"),
		XTAL_STRING("XCE1012"), XTAL_STRING("不正な代入文の左辺です。"),
		XTAL_STRING("XCE1013"), XTAL_STRING("比較演算式の結果を演算しようとしています。"),
		XTAL_STRING("XCE1014"), XTAL_STRING("不正な浮動小数点数リテラルです。"),
		XTAL_STRING("XCE1015"), XTAL_STRING("不正な%(n)d進数値リテラルのサフィックスです。"),
		XTAL_STRING("XCE1016"), XTAL_STRING("assert文の引数の数が不正です。"),
		XTAL_STRING("XCE1017"), XTAL_STRING("不正な%%記法リテラルです。"),
		XTAL_STRING("XCE1018"), XTAL_STRING("default節が重複定義されました。"),
		XTAL_STRING("XCE1019"), XTAL_STRING("'%(name)s'は代入不可能です。"),
		XTAL_STRING("XCE1021"), XTAL_STRING("コメントの途中でファイルが終わりました。"),
		XTAL_STRING("XCE1022"), XTAL_STRING("関数から返せる多値の最大は255個です。"),
		XTAL_STRING("XCE1023"), XTAL_STRING("定義されていないインスタンス変数名 '%(name)s' を参照しています。"),
		XTAL_STRING("XCE1024"), XTAL_STRING("同名のインスタンス変数名 '%(name)s' が既に定義されています。"),
		XTAL_STRING("XCE1025"), XTAL_STRING("比較演算式の結果を再比較しようとしています。"),
		XTAL_STRING("XCE1026"), XTAL_STRING("同じスコープ内で、同じ変数名 '%(name)s' が既に定義されています。"),
		XTAL_STRING("XCE1027"), XTAL_STRING("コードが大きすぎて、バイトコードの生成に失敗しました。"),
		XTAL_STRING("XCE1028"), XTAL_STRING("演算子の前後の空白と演算子の優先順位が一致していません。想定している優先順位と異なっている可能性があります。"),
		
		XTAL_STRING("XRE1001"), XTAL_STRING("'%(object)s' 関数呼び出しの '%(no)s'番目の引数の型が不正です。'%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。"),
		XTAL_STRING("XRE1002"), XTAL_STRING("ソースのコンパイル中、コンパイルエラーが発生しました。"),
		XTAL_STRING("XRE1003"), XTAL_STRING("不正なインスタンス変数の参照です。"),
		XTAL_STRING("XRE1004"), XTAL_STRING("型エラーです。 '%(required)s'型を要求していますが、'%(type)s'型の値が渡されました。"),
		XTAL_STRING("XRE1005"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。'%(min)s'以上の引数を受け取る関数に、%(value)s個の引数を渡しました。"),
		XTAL_STRING("XRE1006"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。'%(min)s'以上、'%(max)s'以下の引数を受け取る関数に、'%(value)s'個の引数を渡しました。"),
		XTAL_STRING("XRE1007"), XTAL_STRING("'%(object)s' 関数呼び出しの引数の数が不正です。引数を取らない関数に、'%(value)s'個の引数を渡しました。"),
		XTAL_STRING("XRE1008"), XTAL_STRING("'%(object)s'はシリアライズできません。"),
		XTAL_STRING("XRE1009"), XTAL_STRING("不正なコンパイル済みXtalファイルです。"),
		XTAL_STRING("XRE1010"), XTAL_STRING("コンパイルエラーが発生しました。"),
		XTAL_STRING("XRE1011"), XTAL_STRING("%(object)s :: '%(name)s' は既に定義されています。"),
		XTAL_STRING("XRE1012"), XTAL_STRING("yieldがfiberの非実行中に実行されました。"),
		XTAL_STRING("XRE1013"), XTAL_STRING("%(object)s にコンストラクタが登録されていないため、インスタンスを生成できません。"),
		XTAL_STRING("XRE1014"), XTAL_STRING("ファイル '%(name)s' を開けません。"),
		XTAL_STRING("XRE1015"), XTAL_STRING("%(object)s は定義されていません。"),
		XTAL_STRING("XRE1016"), XTAL_STRING("ファイル '%(name)s' のコンパイル中、コンパイルエラーが発生しました。"),
		XTAL_STRING("XRE1017"), XTAL_STRING("%(object)s :: %(primary_key)s # %(secondary_key)sは %(accessibility)s です。"),
		XTAL_STRING("XRE1018"), XTAL_STRING("既に閉じられたストリームです。"),
		XTAL_STRING("XRE1019"), XTAL_STRING("C++で定義されたクラスの多重継承は出来ません。"),
		XTAL_STRING("XRE1020"), XTAL_STRING("配列の範囲外アクセスです。"),
		XTAL_STRING("XRE1021"), XTAL_STRING("%(object)s は定義されていません。'%(pick)s'と間違えている可能性があります。"),
		XTAL_STRING("XRE1023"), XTAL_STRING("1より長い文字列は範囲演算子に指定できません。"),
		XTAL_STRING("XRE1024"), XTAL_STRING("0除算エラーです。"),
		XTAL_STRING("XRE1025"), XTAL_STRING("ChRangeは閉区間である必要があります。"),
		XTAL_STRING("XRE1026"), XTAL_STRING("Xeg要素に変換できません。"),
		XTAL_STRING("XRE1027"), XTAL_STRING("cap関数の引数が不正です。cap(name: value)というように名前付き引数にするか、cap(\"name\"), value)というように呼んでください。"),
		XTAL_STRING("XRE1028"), XTAL_STRING("finalマークが付けられたクラス'%(name)s'を継承しようとしました。"),
		XTAL_STRING("XRE1029"), XTAL_STRING("C++で定義されたクラス'%(name)s'は、クラス生成時のみ継承が可能です。"),
		XTAL_STRING("XRE1030"), XTAL_STRING("暗黙の変数参照があります。%(name)s"),
		XTAL_STRING("XRE1031"), XTAL_STRING("シングルトンクラスはシングルトンクラスでないと継承できません。"),
		XTAL_STRING("XRE1032"), XTAL_STRING("ファイル'%(name)s'が開けません。"),
		XTAL_STRING("XRE1033"), XTAL_STRING("ストリームの終端以降を読み取ろうとしました。"),
	};
	
	for(int i=0; i<sizeof(messages)/sizeof(*messages)/2; ++i){
		IDPtr key(messages[i*2+0]);
		StringPtr value(messages[i*2+1]);
		text_map()->set_at(key, value);
	}
}
	
}
